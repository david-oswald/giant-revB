--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   11:53:06 03/28/2012
-- Design Name:   
-- Module Name:   U:/code/fpga/vhdl/ise/universal_trigger_core_tb.vhd
-- Project Name:  giant-ifx-ecc
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: universal_trigger_core
-- 
-- Dependencies:
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
--
-- Notes: 
-- This testbench has been automatically generated using types std_logic and
-- std_logic_vector for the ports of the unit under test.  Xilinx recommends
-- that these types always be used for the top-level I/O of a design in order
-- to guarantee that the testbench will bind correctly to the post-implementation 
-- simulation model.
--------------------------------------------------------------------------------
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
use ieee.numeric_std.all;

library work;
use work.defaults.all;

ENTITY universal_trigger_core_tb IS
END universal_trigger_core_tb;
 
ARCHITECTURE behavior OF universal_trigger_core_tb IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT universal_trigger_core
    PORT(
         clk : IN  std_logic;
         reset : IN  std_logic;
         arm : IN  std_logic;
         armed : OUT  std_logic;
         triggered : OUT  std_logic;
        delay_in : in byte;
		delay_in_we : in std_logic; 
		hold_time_in : in byte;
		hold_time_in_we : in std_logic;
         force_trigger : IN  std_logic;
         input_mode : IN  unsigned(2 downto 0);
         input : IN  std_logic;
         output_mode : IN  unsigned(2 downto 0);
         trigger : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal clk : std_logic := '0';
   signal reset : std_logic := '0';
   signal arm : std_logic := '0';
   signal delay_in : byte := (others => '0');
   signal hold_time_in : byte := (others => '0');
   signal force_trigger : std_logic := '0';
   signal delay_in_we : std_logic := '0';
   signal hold_time_in_we : std_logic := '0';
   signal input_mode : unsigned(2 downto 0) := (others => '0');
   signal input : std_logic := '0';
   signal output_mode : unsigned(2 downto 0) := (others => '0');

 	--Outputs
   signal armed : std_logic;
   signal trigger : std_logic;
   signal triggered : std_logic;

   -- Clock period definitions
   constant clk_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: universal_trigger_core PORT MAP (
          clk => clk,
          reset => reset,
          arm => arm,
          armed => armed,
          delay_in => delay_in,
          delay_in_we => delay_in_we,
			 hold_time_in => hold_time_in,
          hold_time_in_we => hold_time_in_we,
          force_trigger => force_trigger,
          input_mode => input_mode,
          input => input,
          output_mode => output_mode,
          trigger => trigger,
          triggered => triggered
        );

   -- Clock process definitions
   clk_process :process
   begin
		clk <= '0';
		wait for clk_period/2;
		clk <= '1';
		wait for clk_period/2;
   end process;
 

	-- Stimulus process
	stim_proc: process
	begin		
		-- hold reset state for 100 ns.
		reset <= '1';
		input <= '0';
		arm <= '0';
		
		wait for 100 ns;	

		reset <= '0';

		wait for clk_period*10;

		-- insert stimulus here 
		
		-- set delay
		for i in 1 to 3 loop
			delay_in <= "00000000";
			delay_in_we <= '1';
			wait for clk_period;
			delay_in_we <= '0';
			wait for clk_period;
		end loop;
		
		delay_in <= "00000001";
		delay_in_we <= '1';
		wait for clk_period;
		delay_in_we <= '0';
		wait for clk_period;
		
		-- Test: Rising edge trigger
		input_mode <= "011";
		output_mode <= "011";

		arm <= '1';
		wait for clk_period;
		arm <= '0';
		wait for clk_period*3;

		input <= '1';
		wait for clk_period;
		input <= '0';
		wait for 10*clk_period;
		
		-- Test: Rising edge trigger
		-- with hold time
		input_mode <= "011";
		output_mode <= "011";
		
		-- set hold time
		for i in 1 to 3 loop
			hold_time_in <= "00000000";
			hold_time_in_we <= '1';
			wait for clk_period;
			hold_time_in_we <= '0';
			wait for clk_period;
		end loop;
		
		hold_time_in <= "00000011";
		hold_time_in_we <= '1';
		wait for clk_period;
		hold_time_in_we <= '0';
		wait for clk_period;

		arm <= '1';
		wait for clk_period;
		arm <= '0';
		wait for clk_period*3;

		input <= '1';
		wait for 3*clk_period;
		input <= '0';
		wait for 10*clk_period;
		
		-- Test: Falling edge trigger
		input_mode <= "100";
		output_mode <= "011";
		wait for clk_period;
		input <= '1';
		wait for clk_period;
		
		arm <= '1';
		wait for clk_period;
		arm <= '0';
		wait for clk_period*3;

		input <= '0';
		wait for 10*clk_period;
		
		-- Test: Switch output
		input_mode <= "100";
		output_mode <= "101";
		wait for clk_period;
		input <= '1';
		wait for clk_period;
		
		arm <= '1';
		wait for clk_period;
		arm <= '0';
		wait for clk_period*3;

		input <= '0';
		wait for 30*clk_period;
		input <= '1';
		
		arm <= '1';
		wait for clk_period;
		arm <= '0';
		wait for clk_period*3;

		input <= '0';
		wait for 10*clk_period;
		
		-- Test: No delay
		-- Test: Rising edge trigger
		input_mode <= "011";
		output_mode <= "011";
		
		-- set delay
		for i in 1 to 4 loop
			delay_in <= "00000000";
			delay_in_we <= '1';
			wait for clk_period;
			delay_in_we <= '0';
			wait for clk_period;
		end loop;

		arm <= '1';
		wait for clk_period;
		arm <= '0';
		wait for clk_period*3;

		input <= '1';
		wait for clk_period;
		input <= '0';
		wait for 20*clk_period;
		
		wait;
	end process;

END;
