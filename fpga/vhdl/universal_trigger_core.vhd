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
-- Component name: universal_trigger_core
-- Author: David Oswald <david.oswald@rub.de>
-- Date: 10:54 28.03.2012
--
-- Description: Universal trigger core
--
-- Notes:
-- Minimum trigger delay (rising edge, output in rising edge mode, delay = 0)
-- is 3 cycles (rising edge input -> rising edge trigger)
--  
-- Dependencies:
-- none
-----------------------------------------------------------------


library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.defaults.all;

library UNISIM;
use UNISIM.vcomponents.all;

entity universal_trigger_core is
	port (
		-- standard inputs
		clk : in std_logic;
		reset : in std_logic; 
			
		-- arm trigger on rising edge
		arm : in std_logic;
		
		-- armed status
		armed : out std_logic;
		
		-- has triggered
		triggered : out std_logic;
		
		-- trigger delay setting (in cycles of clk), 4 byte loaded
		-- sequentially
		delay_in : in byte;
		
		-- Edge-triggered write enable for delay_in
		delay_in_we : in std_logic;
		
		-- hold time setting (in cycles of clk), 4 byte loaded
		-- sequentially
		hold_time_in : in byte;
		
		-- Edge-triggered write enable for hold_time_in
		hold_time_in_we : in std_logic;
		
		-- force trigger (skips delay) on rising edge
		force_trigger : in std_logic;
		
		-- Mode for trigger input event
		-- 0: Disabled
		-- 1: Level high
		-- 2: Level low
		-- 3: Rising edge
		-- 4: Falling edge
		-- 5: Any edge
		input_mode : unsigned(2 downto 0);
		
		-- input signal
		input : in std_logic;
		
		-- Mode for trigger output
		-- 0: Disabled
		-- 1: Level high
		-- 2: Level low
		-- 3: Rising edge
		-- 4: Falling edge
		-- 5: Switch current state
		output_mode : unsigned(2 downto 0);
		
		-- trigger output
		trigger : out std_logic
	);
end universal_trigger_core;

architecture behavioral of universal_trigger_core is
	
	-- constants
	-- trigger state machine
	type state_type is (
		S_IDLE,
		S_ARMED,
		S_HOLD,
		S_DELAYING,
		S_TRIGGERED
	);
	
	component u8_to_parallel is
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
			clear : in std_logic;
			count : out unsigned(log2_ceil(WIDTH)-1 downto 0);
			d_out : out std_logic_vector(WIDTH*8-1 downto 0)
		);
	end component;

	signal state, state_next : state_type;
	
	signal trigger_int_output : std_logic;
	signal trigger_int : std_logic;
	signal triggered_int : std_logic;
	signal trigger_condition_met_int : std_logic;
	signal trigger_held_int : std_logic;
	signal armed_int : std_logic;
	signal delay_int : unsigned(31 downto 0);
	signal delay : std_logic_vector(31 downto 0);
	signal hold_time : std_logic_vector(31 downto 0);
	signal hold_time_int : unsigned(31 downto 0);
	signal input_mode_int : unsigned(2 downto 0);
	signal output_mode_int : unsigned(2 downto 0);
	signal arm_int : std_logic;
	signal input_int : std_logic;
	signal force_trigger_int : std_logic;
	
	signal arm_prev : std_logic;
	signal force_trigger_prev : std_logic;
	signal input_prev : std_logic;
	
	signal trigger_next : std_logic;	
	signal triggered_next : std_logic;	
	signal trigger_int_output_next : std_logic;
	signal armed_next : std_logic;
	signal delay_int_next : unsigned(31 downto 0);
	signal hold_time_int_next : unsigned(31 downto 0);
	signal input_mode_int_next : unsigned(2 downto 0);
	signal output_mode_int_next : unsigned(2 downto 0);

begin
	-- Outputs
	armed <= armed_int;
	trigger <= trigger_int_output;
	triggered <= triggered_int;
	
	-- delay register
	DELAY_inst : u8_to_parallel 
	generic map
	(
		WIDTH => 4
	)
	port map
	( 
		clk => clk,
		reset => reset,
		d_in => delay_in,
		w_en => delay_in_we,
		clear => '0',
		count => open,
		d_out => delay
	);
	
	-- hold time register
	HOLD_TIME_inst : u8_to_parallel 
	generic map
	(
		WIDTH => 4
	)
	port map
	( 
		clk => clk,
		reset => reset,
		d_in => hold_time_in,
		w_en => hold_time_in_we,
		clear => '0',
		count => open,
		d_out => hold_time
	);
	
	-- trigger input decoding
	TRIGGER_INPUT_DECODE : process(input_int, input_prev, input_mode_int)
	begin
	
		-- check for trigger condition 
		case input_mode_int is
			-- 0: Disabled
			when "000" =>
				trigger_condition_met_int <= '0';
			
			-- 1: Level high
			when "001" =>
				trigger_condition_met_int <= input_int;
			
			-- 2: Level low
			when "010" =>
				trigger_condition_met_int <= not input_int;
				
			-- 3: Rising edge
			when "011" =>
				if (input_int = '1' and input_prev = '0') then
					trigger_condition_met_int  <= '1';
				else
               trigger_condition_met_int  <= '0';
				end if;
				
			-- 4: Falling edge
			when "100" =>
				if (input_int = '0' and input_prev = '1') then
					trigger_condition_met_int  <= '1';
				else
                    trigger_condition_met_int  <= '0';
				end if;
				
			-- 5: Any edge
			when "101" =>
				trigger_condition_met_int  <= input_int xor input_prev;
				
			when others =>
				trigger_condition_met_int <= '0';
				
		end case;
	end process;
	
	-- trigger hold condition decoding
	TRIGGER_HOLD_DECODE : process(input_int, input_prev, input_mode_int)
	begin
	
		-- check for trigger condition 
		case input_mode_int is
			-- 0: Disabled
			when "000" =>
				trigger_held_int <= '0';
			
			-- 1: Level high
			when "001" =>
				trigger_held_int <= input_int;
			
			-- 2: Level low
			when "010" =>
				trigger_held_int <= not input_int;
				
			-- 3: Rising edge
			when "011" =>
				trigger_held_int <= input_int;
				
			-- 4: Falling edge
			when "100" =>
				trigger_held_int <= not input_int;
				
			-- 5: Any edge
			when "101" =>
				trigger_held_int <= not (input_int xor input_prev);
				
			when others =>
				trigger_held_int <= '0';
				
		end case;
	end process;

	-- trigger output encoding
	TRIGGER_OUTPUT_ENCODE : process(output_mode_int, trigger_int)
	begin
		-- default: stay in previous state
		trigger_int_output_next <= trigger_int_output;
		
		-- encode
		case output_mode_int is
			-- 0: Disabled
			when "000" =>
				trigger_int_output_next <= '0';
					
			-- 1: Level high
			when "001" =>
				if trigger_int = '1' then
					trigger_int_output_next <= '1';
				end if;
			
			-- 2: Level low
			when "010" =>
				if trigger_int = '1' then
					trigger_int_output_next <= '0';
				end if;
				
			-- 3: Rising edge
			when "011" =>
				trigger_int_output_next <= trigger_int;
				
			-- 4: Falling edge
			when "100" =>
				trigger_int_output_next <= not trigger_int;
				
			-- 5: Switch state
			when "101" =>
				if trigger_int = '1' then
					trigger_int_output_next <= not trigger_int_output;
				end if;
				
			when others =>
				trigger_int_output_next <= '0';
				
		end case;
	end process;

	
	
	-- FSM next state decoding
	NEXT_STATE_DECODE : process(state, arm_int, arm_prev,
		force_trigger_int, force_trigger_prev, delay, delay_int,
		input_mode, output_mode, input_mode_int, output_mode_int,
		trigger_condition_met_int, triggered_int, hold_time,
		input_int, input_prev, hold_time_int, trigger_held_int)
	begin
		-- default is to stay in current state
		state_next <= state;
		
		-- default values
		input_mode_int_next <= input_mode_int;
		output_mode_int_next <= output_mode_int;
		delay_int_next <= delay_int;
		hold_time_int_next <= hold_time_int;
		triggered_next <= triggered_int;
		
		trigger_next <= '0';
		armed_next <= '0';
		
		case state is
			when S_IDLE =>
				-- Arm on rising edge on arm_int
				if arm_int = '1' and arm_prev = '0' then
					state_next <= S_ARMED;
					armed_next <= '1';
					triggered_next <= '0';
					
					-- clock in delay and modes
					delay_int_next <= unsigned(delay)-1;
					hold_time_int_next <= unsigned(hold_time)-2;
					input_mode_int_next <= input_mode;
					output_mode_int_next <= output_mode;
					
				end if;
			
			when S_ARMED =>
			
				armed_next <= '1';
				
				-- Force trigger
				if force_trigger_int = '1' and force_trigger_prev = '0' then
					armed_next <= '0';
					state_next <= S_TRIGGERED;
					
				elsif trigger_condition_met_int = '1' then
				
					if unsigned(delay) = 0 and unsigned(hold_time) <= 1 then
						armed_next <= '0';
						state_next <= S_TRIGGERED;
						
					elsif not (unsigned(delay) = 0) and unsigned(hold_time) <= 1then
						armed_next <= '1';
						state_next <= S_DELAYING;
					
					elsif not (unsigned(hold_time) = 0) then
						armed_next <= '1';
						hold_time_int_next <= unsigned(hold_time)-2;
						state_next <= S_HOLD;
					end if;
					
					
				end if;
			
			when S_HOLD =>
				armed_next <= '1';
				
				if force_trigger_int = '1' and force_trigger_prev = '0' then
					armed_next <= '0';
					state_next <= S_TRIGGERED;
					
				elsif trigger_held_int = '1' then
				
					-- Trigger held
					if hold_time_int = 0 and unsigned(delay) = 0 then
						armed_next <= '0';
						state_next <= S_TRIGGERED;
					
					elsif hold_time_int = 0 and not (unsigned(delay) = 0) then
						armed_next <= '1';
						state_next <= S_DELAYING;
					
					-- Increase hold time
					else
						hold_time_int_next <= hold_time_int - 1;
					end if;	
				
				-- hold time violated, back to armed
				else
					state_next <= S_ARMED;
					hold_time_int_next <= unsigned(hold_time)-2;
				end if;
			
			when S_DELAYING =>
				armed_next <= '0';
				
				if force_trigger_int = '1' and force_trigger_prev = '0' then
					armed_next <= '0';
					state_next <= S_TRIGGERED;
				elsif delay_int = 0 then
					armed_next <= '0';
					state_next <= S_TRIGGERED;
				else
					delay_int_next <= delay_int - 1;
				end if;
				
			when S_TRIGGERED =>
				-- Generate high level on internal trigger for one cycle
				triggered_next <= '1';
				trigger_next <= '1';
				armed_next <= '0';
				state_next <= S_IDLE;
			
		end case;
	end process;
	
	-- state register update
	STATE_REG: process (clk, reset)
	begin
		if rising_edge(clk) then
			if reset = '1' then
				state <= S_IDLE;
				trigger_int <= '0';
				triggered_int <= '0';
				armed_int <= '0';
				delay_int <= (others => '0');
				input_mode_int <= (others => '0');
				output_mode_int <= (others => '0');
				delay_int <= (others => '0');
				hold_time_int <= (others => '0');
				trigger_int_output <= '0';
			else
				state <= state_next;
				trigger_int <= trigger_next;
				triggered_int <= triggered_next;
				armed_int <= armed_next;
				delay_int <= delay_int_next;
				input_mode_int <= input_mode_int_next;
				output_mode_int <= output_mode_int_next;
				delay_int <= delay_int_next;
				hold_time_int <= hold_time_int_next;
				trigger_int_output <= trigger_int_output_next;
			end if;
		end if;
	end process;

	-- input buffering 
	BUFFERING: process (clk)
	begin
		if rising_edge(clk) then
			if reset = '1' then
				arm_prev <= '0';
				force_trigger_prev <= '0';
				input_prev <= '0';
				
				arm_int <= '0';
				force_trigger_int <= '0';
				input_int <= '0';
			else
				arm_prev <= arm_int;
				force_trigger_prev <= force_trigger_int;
				input_prev <= input_int;
				
				arm_int <= arm;
				force_trigger_int <= force_trigger;
				input_int <= input;
			end if;
		end if;
	end process;
end behavioral;
