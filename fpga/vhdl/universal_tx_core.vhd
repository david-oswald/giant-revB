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
-- Component name: universal_tx_core
-- Author: David Oswald <david.oswald@rub.de>
-- Date: 10:54 28.03.2012
--
-- Description: Universal receiver core
--
-- Notes:
-- The send process starts/stops with one clock delay from rising edge
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

entity universal_tx_core is
	port (
		-- standard inputs
		clk : in std_logic;
		reset : in std_logic; 
		
		-- generated TX clock
		clk_tx : out std_logic;
		
		-- data output pin, connect to top-level IO
		data_out : out std_logic;
		
		-- output valid pin
		data_out_valid : out std_logic;
		
		-- Ready to accept commands and data
		ready : out std_logic;
		
		-- currently transmitting data
		transmitting : out std_logic;
		
		-- high for one clock cycle before transmission
		transmit_start : out std_logic;
		
		-- high for one clock cycle after transmission
		transmit_done : out std_logic;
		
		-- output mode
		-- 00: Set to 0 after data has been set
		-- 01: Set to 1 after data has been set
		-- 10: Set to tristate after data has been set
		-- 11: Keep last output bit
		data_out_mode : in std_logic_vector(1 downto 0);
		
		-- convert logic one to tristate (for pullup busses)
		convert_one_to_Z : in std_logic;
		
		-- start sending on rising edge
		start : in std_logic;
		
		-- stop sending on rising edge
		stop : in std_logic;
		
		-- clear all contents on rising edge
		clear : in std_logic;
		
		-- divider for tx clock
		tx_clk_divide : in byte;
		
		-- write enable for tx_clk_divide (edge triggered)
		tx_clk_divide_we : in std_logic;
		
		-- data input (data to send)
		data_in : in byte;

		-- write enable for data_in (edge triggered)
		data_in_we : in std_logic;
		
		-- FIFO for packet size input
		packet_size_in_lo : in byte;
		packet_size_in_hi : in byte;
		
		-- write enable for packet_size_in (edge triggered)
		packet_size_in_lo_we : in std_logic;
		packet_size_in_hi_we : in std_logic;
		
		-- number of data packets in FIFO
		packet_count : out byte
	);
end universal_tx_core;

architecture behavioral of universal_tx_core is
	
	-- constants
	-- trigger state machine
	type state_type is (
		S_IDLE,
		S_SENDING,
		S_SKIPPING_BITS
	);
	
	-- components
	component universal_tx_fifo
		port 
		(
			rst : in std_logic;
			wr_clk : in std_logic;
			rd_clk : in std_logic;
			din : in std_logic_vector(7 downto 0);
			wr_en : in std_logic;
			rd_en : in std_logic;
			dout : out std_logic_vector(0 downto 0);
			full : out std_logic;
			empty : out std_logic
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
	signal fifo_din: std_logic_vector(7 downto 0);
	signal fifo_wr_en: std_logic;
	signal fifo_rd_en: std_logic;
	signal fifo_rd_en_next: std_logic;
	signal fifo_dout: std_logic_vector(0 downto 0);
	signal fifo_full: std_logic;
	signal fifo_empty: std_logic;
	
	-- clock divider
	constant CLK_DIV_WIDTH : positive := 16;
	signal tx_clk_divide_int : std_logic_vector(CLK_DIV_WIDTH-1 downto 0);
	signal tx_clk_div_counter : unsigned(CLK_DIV_WIDTH-1 downto 0);
	signal tx_clk_div_counter_next : unsigned(CLK_DIV_WIDTH-1 downto 0);
	
	-- Current bit counter
	signal current_bit_count : unsigned(15 downto 0);
	signal current_bit_count_next : unsigned(15 downto 0);
	
	-- internal tx clock
	signal clk_tx_int : std_logic;
	signal clk_tx_int_prep : std_logic;
	signal clk_tx_int_next : std_logic;
	
	-- data valid strobe
	signal data_valid : std_logic;
	signal data_valid_next : std_logic;
	
	-- status bits
	signal ready_int : std_logic;
	signal ready_next : std_logic;
	signal transmitting_int : std_logic;
	signal transmitting_next : std_logic;
	signal transmit_done_next : std_logic;
	
	-- FIFO for packet bit counts
	constant FIFO_BC_WIDTH : positive := 32;
	signal fifo_bc_count : unsigned(log2_ceil(FIFO_BC_WIDTH)-1 downto 0);
	signal fifo_bc_rd_en : std_logic;
	signal fifo_bc_rd_en_next : std_logic;
	signal fifo_bc_d_out : std_logic_vector(15 downto 0);
	signal fifo_bc_reset : std_logic;
	signal fifo_bc_out_u8 : unsigned(15 downto 0);
	signal fifo_bc_d_out_mod8 : unsigned(15 downto 0);
	signal fifo_bc_d_out_mod8_next : unsigned(15 downto 0);
	
	-- Edge detection
	signal start_prev : std_logic;
	signal stop_prev : std_logic;
	signal clear_prev : std_logic;

begin
	-- Outputs
	packet_count <= "000" & std_logic_vector(fifo_bc_count);
	clk_tx_int_prep  <= not clk when tx_clk_divide = "00000000" else
	                    not clk_tx_int;
	
	clk_tx <= clk_tx_int_prep when data_valid = '1' else
			  '0';
	
	data_out_valid <= data_valid;
	
	data_out <= 'Z' when data_valid = '1' and fifo_dout(0) = '1' and convert_one_to_Z = '1' else
				fifo_dout(0) when data_valid = '1' else
				'0'          when data_out_mode = "00" else
				'1'          when data_out_mode = "01" else
				'Z'          when data_out_mode = "10" else
				fifo_dout(0) when data_out_mode = "11";
	
	ready <= ready_int;
	transmitting <= transmitting_int;
		
	-- Components
	
	-- Clock divide FIFO
	SINGLE_WRITE_FIFO : u8_to_parallel
	generic map(
		WIDTH => 2
	)
	port map(
		clk => clk,
		reset => reset,
		d_in => tx_clk_divide,
		w_en => tx_clk_divide_we,
		clear => '0', 
		count => open, 
		d_out => tx_clk_divide_int
	);
	
	-- Packet bit count FIFO
	SHIFT_REGISTER_inst_lo : shift_register
	generic map
	(
		WIDTH => FIFO_BC_WIDTH
	)
	port map
	(
		clk   => clk,
		reset => fifo_bc_reset or reset,
		d_in  => packet_size_in_lo,
		w_en  => packet_size_in_lo_we,
		count => fifo_bc_count,
		d_out => fifo_bc_d_out(7 downto 0),
		r_en  => fifo_bc_rd_en 
	);
	
	SHIFT_REGISTER_inst_hi : shift_register
	generic map
	(
		WIDTH => FIFO_BC_WIDTH
	)
	port map
	(
		clk   => clk,
		reset => fifo_bc_reset or reset,
		d_in  => packet_size_in_hi,
		w_en  => packet_size_in_hi_we,
		count => open,
		d_out => fifo_bc_d_out(15 downto 8),
		r_en  => fifo_bc_rd_en 
	);
	
	fifo_bc_out_u8 <= unsigned(fifo_bc_d_out) - to_unsigned(1, fifo_bc_d_out'length);
	
	--packet_count <= byte(current_bit_count(7 downto 0));
	
	-- Data FIFO
	UNIVERSAL_TX_FIFO_inst: universal_tx_fifo
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
		empty         => fifo_empty
	);
	
	fifo_rd_clk <= clk;
	fifo_wr_clk <= clk;
	fifo_wr_en <= data_in_we;
	fifo_din <= data_in;

	--clk_tx_int <= clk_tx when use_clk_tx = '1' else
	--		        clk;

	-- FSM next state decoding
	NEXT_STATE_DECODE : process(state, start, start_prev,
		stop, stop_prev, clear, current_bit_count, fifo_bc_d_out,
		tx_clk_div_counter, data_valid, ready_int, transmitting_int,
		tx_clk_divide_int, fifo_bc_out_u8, fifo_bc_d_out_mod8)
	begin
		-- default is to stay in current state
		state_next <= state;
		
		-- default values
		fifo_rst <= '0';
		fifo_bc_reset <= '0';
		fifo_bc_rd_en_next <= '0';
		fifo_rd_en_next <= '0';
		clk_tx_int_next <= '0';
		transmit_done_next <= '0';
		tx_clk_div_counter_next <= (others => '0');
		data_valid_next <= data_valid;
		current_bit_count_next <= current_bit_count;
		ready_next <= ready_int;
		transmitting_next <= transmitting_int;
		transmit_start <= '0';
		fifo_bc_d_out_mod8_next <= fifo_bc_d_out_mod8;
		
		case state is
			when S_IDLE =>
				data_valid_next <= '0';
				current_bit_count_next <= fifo_bc_out_u8;
				fifo_bc_d_out_mod8_next <= "0000000000000" & unsigned(fifo_bc_d_out(2 downto 0));
				ready_next <= '1';
				transmitting_next <= '0';
				
				if clear = '1' and clear_prev = '0' then
					-- reset RX fifo
					fifo_rst <= '1';
					
					-- reset packet boundary fifo
					fifo_bc_reset <= '1';
					
					ready_next <= '0';
					
				elsif start = '1' and start_prev = '0' then
					state_next <= S_SENDING;
					fifo_rd_en_next <= '1';
					tx_clk_div_counter_next  <= unsigned(tx_clk_divide_int);
					
					ready_next <= '0';
					transmitting_next <= '1';
					transmit_start <= '1';
				end if;
			
			when S_SENDING =>
			
				if (stop = '1' and stop_prev = '0') then
					fifo_rd_en_next <= '1';
					data_valid_next <= '1';
					transmitting_next <= '0';
					state_next <= S_IDLE;
					transmit_done_next <= '1';
				
				elsif tx_clk_div_counter = 0 then
					fifo_rd_en_next <= '1';
					data_valid_next <= '1';
					tx_clk_div_counter_next  <= unsigned(tx_clk_divide_int);
					current_bit_count_next <= current_bit_count - 1;
					
					if current_bit_count = 0 then
						fifo_bc_rd_en_next <= '1';
						
						-- Check if bits are to be removed due to multiple of 8
						if fifo_bc_d_out_mod8 = 0 then
							state_next <= S_IDLE;
							fifo_rd_en_next <= '0';
							transmit_done_next <= '1';
						else
							state_next <= S_SKIPPING_BITS;
							current_bit_count_next <= 7 - fifo_bc_d_out_mod8;
						end if;
					end if;
					
				elsif tx_clk_div_counter < unsigned(tx_clk_divide_int)/2 then
					tx_clk_div_counter_next <= tx_clk_div_counter - 1;
					--data_valid_next <= '1';
					
				elsif tx_clk_div_counter >= unsigned(tx_clk_divide_int)/2 then
					tx_clk_div_counter_next <= tx_clk_div_counter - 1;
					data_valid_next <= '1';
					clk_tx_int_next <= '1';	
					
				end if;
			
		when S_SKIPPING_BITS =>
				data_valid_next <= '0';
				transmitting_next <= '0';
				
				if current_bit_count = 0 then
					state_next <= S_IDLE;
					transmit_done_next <= '1';
				else 
					fifo_rd_en_next <= '1';
					current_bit_count_next <= current_bit_count - 1;
				end if;
				
		end case;
	end process;
	
	-- state register update
	STATE_REG: process (clk)
	begin
		if rising_edge(clk) then
			if reset = '1' then
				state <= S_IDLE;
				fifo_bc_rd_en <= '0';
				clk_tx_int <= '0';
				tx_clk_div_counter <= (others => '0');
				current_bit_count <= (others => '0');
				data_valid <= '0';
				fifo_rd_en <= '0';
				ready_int <= '0';
				transmitting_int <= '0';
				transmit_done <= '0';
				fifo_bc_d_out_mod8 <= (others => '0');
			else
				state <= state_next;
				fifo_bc_rd_en <= fifo_bc_rd_en_next;
				clk_tx_int <= clk_tx_int_next;
				tx_clk_div_counter <= tx_clk_div_counter_next;
				current_bit_count <= current_bit_count_next;
				data_valid <= data_valid_next;
				fifo_rd_en <= fifo_rd_en_next;
				ready_int <= ready_next;
				transmitting_int <= transmitting_next;
				transmit_done <= transmit_done_next;
				fifo_bc_d_out_mod8 <= fifo_bc_d_out_mod8_next;
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
			else
				start_prev <= start;
				stop_prev <= stop;
				clear_prev <= clear;
			end if;
		end if;
	end process;
end behavioral;
