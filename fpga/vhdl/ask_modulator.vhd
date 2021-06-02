-----------------------------------------------------------------
-- This file is part of GIAnt, the Generic Implementation ANalysis Toolkit
--
-- Visit www.sourceforge.net/projects/giant/
--
-- Copyright (C) 2010 - 2011 David Oswald <david.oswald@rub.de>
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
-- Component name: ask_modulator
-- Author: David Oswald <david.oswald@rub.de>
-- Date: 091004
--
-- Description:
-- Amplitude Shift Keying modulator, with 13.56 MHz output waveform
--
-- Notes:
-- none
--  
-- Dependencies:
--
--
-----------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.defaults.all;

entity ask_modulator is
	port( 
		-- standard inputs
		clk : in std_logic;
		ce : in std_logic;
		reset : in std_logic;
		
		-- max value for output
		out_amplitude : in byte;
		
		-- modulating signal
		data : in std_logic;
		
		-- modulated output
		modulated : out byte;
		
		-- Reset input to start RFID field reset (rising edge)
		field_reset : in std_logic;
		
		-- Ready output after field restart (generates rising edge)
		field_reset_done : out std_logic;
		
		-- FIFO for 32 bit time for field shutdown
		field_reset_time_in : in byte;
		
		-- write enable for packet_size_in(edge triggered)
		field_reset_time_in_we : in std_logic
	);
end ask_modulator;

architecture behavioral of ask_modulator is
   -- constants
   
	-- State machine
	type state_type is (
		S_MODULATING,
		S_RESETTING
	);
   
   -- components
	component rfid_freqgen
		port (
			ce : in std_logic;
			clk: in std_logic;
			sclr : in std_logic;
			rdy : out std_logic;
			cosine: out byte
		);
	end component;
	
	component multiplier
		port (
			clk: in std_logic;
			a: in std_logic_vector(7 downto 0);
			b: in std_logic_vector(7 downto 0);
			ce: in std_logic;
			p: out std_logic_vector(7 downto 0)
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
	signal carrier_ac, modulated_ac, mod_ac : std_logic_vector(7 downto 0);
	signal modulated_next : std_logic_vector(7 downto 0);
	signal modulated_int : std_logic_vector(7 downto 0);
	
	signal state, state_next : state_type;
	
	-- field reset control
	signal field_reset_time : std_logic_vector(31 downto 0);
	signal field_reset_counter: unsigned(31 downto 0);
	signal field_reset_counter_next: unsigned(31 downto 0);
	signal field_reset_prev : std_logic;
	signal rfid_reset, rfid_reset_next : std_logic;
	signal rfid_ce, rfid_ce_next : std_logic;
	signal rfid_rdy : std_logic;
begin

	-- Field reset counter
	SINGLE_WRITE_FIFO : u8_to_parallel
	generic map(
		WIDTH => 4
	)
	port map(
		clk => clk,
		reset => reset,
		d_in => field_reset_time_in,
		w_en => field_reset_time_in_we,
		clear => '0', 
		count => open, 
		d_out => field_reset_time
	);
	
	RFID_FREQGEN_INST : rfid_freqgen
	port map(
		ce => rfid_ce,
		clk => clk,
		sclr => rfid_reset,
		cosine => carrier_ac,
		rdy => rfid_rdy
	);
	
	mod_ac <= std_logic_vector(out_amplitude) when data = '1' else
				 (others => '0');
	
	MODULATOR : multiplier
	port map(
		clk => clk,
		a => carrier_ac,
		b => mod_ac,
		ce => ce,
		p => modulated_ac
	);
			
	MAIN : process(clk)
	begin
		if rising_edge(clk) then
			if reset = '1' then
				state <= S_MODULATING;
				modulated_int <= std_logic_vector(to_signed(128, modulated_ac'length));
				field_reset_counter <= (others => '0');
			else
				state <= state_next;
				modulated_int <= modulated_next;
				field_reset_counter <= field_reset_counter_next;
			end if;
		end if;
	end process;
	
	modulated <= modulated_int;
	
	-- FSM next state decoding
	NEXT_STATE_DECODE : process(state, field_reset_counter, field_reset,
		field_reset_prev, modulated_int, rfid_reset, rfid_ce, field_reset_time,
		rfid_rdy)
	begin
		-- default is to stay in current state
		state_next <= state;
		
		-- default values
		rfid_reset_next <= rfid_reset;
		rfid_ce_next <= rfid_ce;
		field_reset_done <= '0';
		field_reset_counter_next <= field_reset_counter;
		modulated_next <= modulated_int;
		
		case state is
			when S_MODULATING =>
				
				rfid_reset_next <= '0';
				rfid_ce_next <= '1';
			
				if field_reset = '1' and field_reset_prev = '0' then
					field_reset_counter_next <= unsigned(field_reset_time);
					state_next <= S_RESETTING;
				else
					if rfid_rdy = '1' then
						modulated_next <= std_logic_vector(signed(modulated_ac) + to_signed(128, modulated_ac'length));
					else
						modulated_next <= std_logic_vector(to_signed(128, modulated_ac'length));
					end if;
				end if;
			
			when S_RESETTING =>
				
				modulated_next <= std_logic_vector(to_signed(128, modulated_ac'length));
				
				if field_reset_counter = 0 then
					rfid_reset_next <= '0';
					rfid_ce_next <= '1';
					state_next <= S_MODULATING;
					field_reset_done <= '1';
					
				elsif field_reset_counter < to_unsigned(8, field_reset_counter'length) then
					rfid_reset_next <= '0';
					rfid_ce_next <= '1';
					
					field_reset_counter_next <= field_reset_counter - 1;
				
					if field_reset_counter = to_unsigned(1, field_reset_counter'length) then
						field_reset_done <= '1';
					end if;
					
				else
					rfid_reset_next <= '1';
					rfid_ce_next <= '0';
					
					field_reset_counter_next <= field_reset_counter - 1;
				end if;
				
		end case;
	end process;
	
	EDGES : process(clk)
	begin
		if rising_edge(clk) then
			if reset = '1' then
				field_reset_prev <= '0';
				rfid_reset <= '1';
				rfid_ce <= '1';
			else
				field_reset_prev <= field_reset;
				rfid_reset <= rfid_reset_next;
				rfid_ce <= rfid_ce_next;
			end if;
		end if;
	end process;
	
end behavioral;
