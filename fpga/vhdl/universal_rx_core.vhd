-----------------------------------------------------------------
-- This file is part of GIAnt, the Generic Implementation ANalysis Toolkit
--
-- Visit www.sourceforge.net/projects/giant/
--
-- Copyright (C) 2010 - 2012 David Oswald <david.oswald@rub.de>
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License version 3 as
-- published by the Free Software Foundation.
--
-- This program is distributed in the hope that it will be useful, but
-- WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
-- General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, see http://www.gnu.org/licenses/.
-----------------------------------------------------------------

-----------------------------------------------------------------
-- 
-- Component name: universal_rx_core
-- Author: David Oswald <david.oswald@rub.de>
-- Date: 10:54 28.03.2012
--
-- Description: Universal receiver core
--
-- Notes:
-- The receive process starts/stops with one clock delay from rising edge
-- of start/stop pin
--  
-- Dependencies:
-- universal_rx_fifo
-- shift_register
-----------------------------------------------------------------


library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.defaults.all;

library UNISIM;
use UNISIM.vcomponents.all;

entity universal_rx_core is
	generic (
		-- width of data_in_delay
		INPUT_DELAY_WIDTH : positive := 4
	);
	port (
		-- standard inputs
		clk : in std_logic;
		reset : in std_logic; 
		
		-- dedicated RX clock
		clk_rx : in std_logic;
		
		-- enable dedicated RX clock
		use_clk_rx : in std_logic;
		
		-- delay of input pin in clk cycles
		-- e.g. to compensate for trigger delays etc.
		data_in_delay : in unsigned(INPUT_DELAY_WIDTH-1 downto 0);
		
		-- coarse delay at divided clock
		data_in_divide_delay : in unsigned(INPUT_DELAY_WIDTH-1 downto 0);
		
		-- resync sample points to rising clock edges?
		resync_to_rising_edges : in std_logic;
		
		-- resync sample points to falling clock edges?
		resync_to_falling_edges : in std_logic;
		
		-- data input pin, connect to top-level IO
		data_in : in std_logic;
			
		-- start recording on rising edge
		start : in std_logic;
		
		-- stop recording on rising edge
		stop : in std_logic;
		
		-- clear all contents on rising edge
		clear : in std_logic;
		
		-- divider for receiver clock
		rx_clk_divide : in byte;
		
		-- write enable for rx_clk_divide (edge triggered)
		rx_clk_divide_we : in std_logic;
		
		-- Ready to accept commands and data
		ready : out std_logic;
		
		-- currently receiving data
		receiving : out std_logic;
		
		-- sampling point
		data_in_sample : out std_logic;
		
		-- number of received data packets (number of assertions of start)
		packet_count : out byte;
		
		-- data output (received data)
		data_out : out byte;

		-- read enable for data_out (edge triggered)
		data_out_re : in std_logic;
		
		-- FIFO for packet sizes output
		packet_size_out_low : out byte;
		packet_size_out_high : out byte;
		
		-- read enable for packet_size_out_low (edge triggered)
		packet_size_out_low_re : in std_logic;		
		packet_size_out_high_re : in std_logic		
	);
end universal_rx_core;

architecture behavioral of universal_rx_core is
	
	-- constants
	-- trigger state machine
	type state_type is (
		S_IDLE,
		S_PAD_TO_8,
		S_RECORDING
	);
	
	-- components
	component universal_rx_fifo
		port 
		(
			rst : in std_logic;
			wr_clk : in std_logic;
			rd_clk : in std_logic;
			din : in std_logic_vector(0 downto 0);
			wr_en : in std_logic;
			rd_en : in std_logic;
			dout : out std_logic_vector(7 downto 0);
			full : out std_logic;
			empty : out std_logic;
			rd_data_count : out std_logic_vector(9 downto 0)
		);
	end component;
	
	component shift_register is
		generic
		(	
			WIDTH : positive
		);
		port
		( 
			clk : in std_logic;
			reset : in std_logic;
			d_in : in byte;
			w_en : in std_logic;
			count : out unsigned(log2_ceil(WIDTH)-1 downto 0);
			d_out : out byte;
			r_en : in std_logic
		);
	end component;
	
	component u8_to_parallel is
		generic(
			WIDTH : positive
		);
		port( 
			clk : in std_logic;
			reset : in std_logic;
			d_in : in byte;
			w_en : in std_logic;
			clear : in std_logic;
			count : out unsigned(log2_ceil(WIDTH)-1 downto 0);
			d_out : out std_logic_vector(WIDTH*8-1 downto 0)
		);
	end component;

	-- signals
	signal state, state_next : state_type;
	
	-- FIFO control
	signal fifo_rst : std_logic;
	signal fifo_wr_clk: std_logic;
	signal fifo_rd_clk: std_logic;
	signal fifo_din: std_logic_vector(0 downto 0);
	signal fifo_wr_en: std_logic;
	signal fifo_din_prev : std_logic;
	signal fifo_rd_en: std_logic;
	signal fifo_dout: std_logic_vector(7 downto 0);
	signal fifo_full: std_logic;
	signal fifo_empty: std_logic;
	signal fifo_rd_data_count : std_logic_vector(9 downto 0);
	
	-- receiver clock divider
	constant CLK_DIV_WIDTH : positive := 16;
	constant CLK_RESYNC: unsigned(CLK_DIV_WIDTH-1 downto 0) := to_unsigned(10, CLK_DIV_WIDTH);
	signal rx_clk_divide_int : std_logic_vector(CLK_DIV_WIDTH-1 downto 0);
	signal rx_clk_div_counter : unsigned(CLK_DIV_WIDTH-1 downto 0);
	signal rx_clk_div_resync_counter : unsigned(CLK_DIV_WIDTH-1 downto 0);
	signal rx_do_resync : std_logic;
	signal rx_do_resync_next : std_logic;

	signal sync_rx : std_logic;
	signal reset_rx : std_logic;
	
	-- internal rx clock
	signal clk_rx_int : std_logic;
	
	-- input delay shift register
	signal data_in_sr : std_logic_vector(2**INPUT_DELAY_WIDTH-1 downto 0);
	signal data_in_divide_sr : std_logic_vector(2**INPUT_DELAY_WIDTH-1 downto 0);
	signal data_in_fine : std_logic;
	signal data_in_fine_prev : std_logic;

	-- status bits
	signal ready_int : std_logic;
	signal ready_next : std_logic;
	signal receiving_int : std_logic;
	signal receiving_next : std_logic;
	
	-- FIFO for packet bit counts
	constant FIFO_BC_WIDTH : positive := 32;
	signal fifo_bc_count : unsigned(log2_ceil(FIFO_BC_WIDTH)-1 downto 0);
	signal fifo_bc_wr_en : std_logic;
	signal fifo_bc_wr_en_next : std_logic;
	signal fifo_bc_d_in : std_logic_vector(15 downto 0);
	signal fifo_bc_d_in_next : std_logic_vector(15 downto 0);
	signal fifo_bc_reset : std_logic;
	
	-- current input bit counter
	signal current_bit_count : unsigned(15 downto 0);	
	
	-- Edge detection
	signal start_prev : std_logic;
	signal stop_prev : std_logic;
	signal clear_prev : std_logic;

begin
	-- Outputs
	packet_count <= "000" & std_logic_vector(fifo_bc_count);
	data_in_sample <= fifo_wr_en;
	--data_in_sample <= fifo_din(0);
	ready <= ready_int;
	receiving <= receiving_int;
	
	-- Components
	
	-- Clk divider FIFO
	CLKDIV_FIFO : u8_to_parallel
	generic map(
		WIDTH => 2
	)
	port map(
		clk => clk,
		reset => reset,
		d_in => rx_clk_divide,
		w_en => rx_clk_divide_we,
		clear => '0', 
		count => open, 
		d_out => rx_clk_divide_int
	);
	
	-- Packet bit count FIFO
	SHIFT_REGISTER_LSB_inst : shift_register
	generic map
	(
		WIDTH => FIFO_BC_WIDTH
	)
	port map
	(
		clk   => clk,
		reset => fifo_bc_reset or reset,
		d_in  => fifo_bc_d_in(7 downto 0),
		w_en  => fifo_bc_wr_en,
		count => fifo_bc_count,
		d_out => packet_size_out_low,
		r_en  => packet_size_out_low_re
	);
	
	SHIFT_REGISTER_MSB_inst : shift_register
	generic map
	(
		WIDTH => FIFO_BC_WIDTH
	)
	port map
	(
		clk   => clk,
		reset => fifo_bc_reset or reset,
		d_in  => fifo_bc_d_in(15 downto 8),
		w_en  => fifo_bc_wr_en,
		count => open,
		d_out => packet_size_out_high,
		r_en  => packet_size_out_high_re
	);
	
	-- Data FIFO
	UNIVERSAL_RX_FIFO_inst: universal_rx_fifo
	port map
	(
		rst 		  => fifo_rst or reset,
		wr_clk        => fifo_wr_clk,
		rd_clk        => fifo_rd_clk,
		din           => fifo_din,
		wr_en         => fifo_wr_en,
		rd_en         => fifo_rd_en,
		dout          => fifo_dout,
		full          => fifo_full,
		empty         => fifo_empty,
		rd_data_count => fifo_rd_data_count
	);
	
	fifo_rd_clk <= clk;
	fifo_wr_clk <= clk_rx_int;
	fifo_rd_en <= data_out_re;
	data_out <= fifo_dout;
	
	fifo_din(0) <= data_in_fine when data_in_divide_delay = to_unsigned(0, data_in_divide_delay'length) else
				   data_in_divide_sr(to_integer(data_in_divide_delay));
				   
	data_in_fine <= data_in when data_in_delay = to_unsigned(0, data_in_delay'length) else
				    data_in_sr(to_integer(data_in_delay)-1);
	
	clk_rx_int <= clk_rx when use_clk_rx = '1' else
			        clk;

	-- FSM next state decoding
	NEXT_STATE_DECODE : process(state, start, start_prev,
		stop, stop_prev, clear, ready_int, receiving_int,
		current_bit_count, fifo_bc_d_in)
	begin
		-- default is to stay in current state
		state_next <= state;
		
		-- default values
		fifo_rst <= '0';
		fifo_bc_reset <= '0';
		fifo_bc_wr_en_next <= '0';
		fifo_bc_d_in_next <= fifo_bc_d_in; --(others => '0');
		ready_next <= ready_int;
		receiving_next <= receiving_int;
		sync_rx <= '0';
		reset_rx <= '0';
		
		case state is
			when S_IDLE =>
				
				ready_next <= '1';
				receiving_next <= '0';
				fifo_bc_d_in_next <= (others => '0');
				
				if clear = '1' and clear_prev = '0' then
					-- reset RX fifo
					fifo_rst <= '1';
					
					-- reset packet boundary fifo
					fifo_bc_reset <= '1';
					
					-- reset delay fifo
					reset_rx <= '1';
					
				elsif start = '1' and start_prev = '0' then
					state_next <= S_RECORDING;
					
					sync_rx <= '1';
					
					-- set recording signal
					ready_next <= '0';
					receiving_next <= '1';
				end if;
			
			when S_RECORDING =>
				if stop = '1' and stop_prev = '0' then
					
					fifo_bc_d_in_next <= std_logic_vector(current_bit_count);
					
					if current_bit_count(2 downto 0) = 0 then
						-- stop recording
						state_next <= S_IDLE;
						
						receiving_next <= '0';
				
						-- store length of current packet
						fifo_bc_wr_en_next <= '1';
					else
						-- continue to write until bit count divisible by 8
						state_next <= S_PAD_TO_8;
					end if;		
				end if;
				
			when S_PAD_TO_8 =>
			
				-- stop when bit count divisible by 8
				if current_bit_count(2 downto 0) = 0 then
					-- stop recording
					state_next <= S_IDLE;
					
					receiving_next <= '0';
			
					-- store length of current packet (unpadded)
					fifo_bc_wr_en_next <= '1';
					--fifo_bc_d_in_next <= byte(current_bit_count);
				end if;		
		end case;
	end process;
	
	-- FIFO rx enable generation
	FIFO_RX_CONTROL : process (clk_rx_int)
	begin
		if rising_edge(clk_rx_int) then
			rx_do_resync <= rx_do_resync_next;
			
			if reset_rx = '1' then
				fifo_wr_en <= '0';
				rx_clk_div_counter <= (others => '0');
				rx_clk_div_resync_counter <= (others => '0');
				current_bit_count <= (others => '0');
				data_in_divide_sr <= (others => '0');
				rx_do_resync_next <= '0';
				
				
			elsif sync_rx = '1' then
				fifo_wr_en <= '0';
				rx_clk_div_counter <= (others => '0');
				rx_clk_div_resync_counter <= (others => '0');
				current_bit_count <= (others => '0');
				rx_do_resync_next <= '0';
				
			else
				rx_do_resync_next <= rx_do_resync;
				
				-- sample
				if rx_clk_div_counter = 0 and (state = S_PAD_TO_8 or state = S_RECORDING) then
					fifo_wr_en <= '1';
					rx_clk_div_counter <= unsigned(rx_clk_divide_int);
					current_bit_count <= current_bit_count + 1;
					data_in_divide_sr <= data_in_divide_sr(data_in_divide_sr'length-2 downto 0) & data_in_fine; 
					rx_do_resync_next <= '0';
					
				-- sample if resync
				elsif rx_do_resync = '1' and rx_clk_div_resync_counter = 0 
					and (state = S_PAD_TO_8 or state = S_RECORDING) then
					
					fifo_wr_en <= '1';
					rx_clk_div_counter <= unsigned(rx_clk_divide_int);
					rx_clk_div_resync_counter <= (others => '0');
					current_bit_count <= current_bit_count + 1;
					data_in_divide_sr <= data_in_divide_sr(data_in_divide_sr'length-2 downto 0) & data_in_fine; 
					rx_do_resync_next <= '0';
				
				-- resync to edges
				elsif (fifo_din(0) = '1' and fifo_din_prev = '0') 
					and (state = S_PAD_TO_8 or state = S_RECORDING) 
					and resync_to_rising_edges = '1' and rx_do_resync = '0' then
					
					fifo_wr_en <=  '0';
					rx_clk_div_resync_counter <= CLK_RESYNC;
					rx_do_resync_next <= '1';
					
					--rx_clk_div_counter <= unsigned(rx_clk_divide_int);
					--data_in_divide_sr <= data_in_divide_sr(data_in_divide_sr'length-2 downto 0) & data_in_fine; 
				
				elsif (fifo_din(0) = '0' and fifo_din_prev = '1')  
					and (state = S_PAD_TO_8 or state = S_RECORDING) 
					and resync_to_falling_edges = '1' and rx_do_resync = '0' then
					
					fifo_wr_en <= '0';
					rx_clk_div_resync_counter <= CLK_RESYNC;
					rx_do_resync_next <= '1';
					
					--rx_clk_div_counter <= unsigned(rx_clk_divide_int);
					--data_in_divide_sr <= data_in_divide_sr(data_in_divide_sr'length-2 downto 0) & data_in_fine; 
				
				elsif (rx_clk_div_counter = 0) then
					fifo_wr_en <= '0';
					rx_clk_div_counter <= unsigned(rx_clk_divide_int);
					rx_clk_div_resync_counter <= (others => '0');
					rx_do_resync_next <= '0';
					
					--if (state = S_PAD_TO_8 or state = S_RECORDING) then
					data_in_divide_sr <= data_in_divide_sr(data_in_divide_sr'length-2 downto 0) & data_in_fine; 
				else
					fifo_wr_en <= '0';
					rx_clk_div_counter <= rx_clk_div_counter - 1;
					
					if rx_do_resync = '1' then
						rx_clk_div_resync_counter <= rx_clk_div_resync_counter - 1;
					end if;
				--else
				--	fifo_wr_en <= '0';
				--	rx_clk_div_counter <= (others => '0');
				--	current_bit_count <= (others => '0');
				end if;
			end if;
		end if;
	end process;
	
	-- state register update
	STATE_REG: process (clk)
	begin
		if rising_edge(clk) then
			if reset = '1' then
				state <= S_IDLE;
				fifo_bc_wr_en <= '0';
				fifo_bc_d_in <= (others => '0');
				ready_int <= '0';
				receiving_int <= '0';
			else
				state <= state_next;
				fifo_bc_wr_en <= fifo_bc_wr_en_next;
				fifo_bc_d_in <= fifo_bc_d_in_next;
				ready_int <= ready_next;
				receiving_int <= receiving_next;
			end if;
		end if;
	end process;

	-- input buffering 
	BUFFERING: process (clk)
	begin
		if rising_edge(clk) then
			if reset = '1' then
				start_prev <= '0';
				stop_prev <= '0';
				clear_prev <= '0';
				fifo_din_prev <= '0';
				data_in_fine_prev <= '0';
				
				data_in_sr <= (others => '0');
			else
				start_prev <= start;
				stop_prev <= stop;
				clear_prev <= clear;
				fifo_din_prev <= fifo_din(0);
				data_in_fine_prev <= data_in_fine;
				
				data_in_sr <= data_in_sr(data_in_sr'length-2 downto 0) & data_in; 
			end if;
		end if;
	end process;
end behavioral;
