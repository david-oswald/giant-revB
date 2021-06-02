--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   08:26:20 04/02/2012
-- Design Name:   
-- Module Name:   U:/code/fpga/vhdl/universal_rx_core_tb.vhd
-- Project Name:  giant-ifx-ecc
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: universal_rx_core
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

library work;
use work.defaults.all;
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
USE ieee.numeric_std.ALL;
 
ENTITY universal_rx_core_tb IS
END universal_rx_core_tb;
 
ARCHITECTURE behavior OF universal_rx_core_tb IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
	COMPONENT universal_rx_core
	GENERIC (
		INPUT_DELAY_WIDTH : positive := 4
	);
	PORT(
		clk : IN  std_logic;
		reset : IN  std_logic;
		clk_rx : in std_logic;
		use_clk_rx : IN std_logic;
		data_in_delay : IN unsigned(INPUT_DELAY_WIDTH-1 downto 0);
		data_in_divide_delay : in unsigned(INPUT_DELAY_WIDTH-1 downto 0);
		data_in : IN  std_logic;
		start : IN  std_logic;
		stop : IN  std_logic;
		clear : IN  std_logic;
		rx_clk_divide : IN  std_logic_vector(7 downto 0);
		ready : out std_logic;
		receiving : out std_logic;
		data_in_sample : out std_logic;
		packet_count : OUT  std_logic_vector(7 downto 0);
		data_out : OUT  std_logic_vector(7 downto 0);
		data_out_re : IN  std_logic;
		packet_size_out : out byte;
		packet_size_out_re : in std_logic
	);
	END COMPONENT;
    

   --Inputs
   signal clk : std_logic := '0';
   signal reset : std_logic := '0';
	signal data_in_delay : unsigned(3 downto 0) := (others => '0');
	signal data_in_divide_delay : unsigned(3 downto 0) := (others => '0');
   signal data_in : std_logic := '0';
   signal start : std_logic := '0';
   signal stop : std_logic := '0';
   signal clear : std_logic := '0';
   signal clk_rx : std_logic := '0';
   signal use_clk_rx : std_logic := '0';
   signal rx_clk_divide : std_logic_vector(7 downto 0) := (others => '0');
   signal data_out_re : std_logic := '0';
   signal packet_size_out_re : std_logic := '0';

 	--Outputs
   signal packet_count : std_logic_vector(7 downto 0);
   signal data_out : std_logic_vector(7 downto 0);
   signal packet_size_out : std_logic_vector(7 downto 0);
   signal ready : std_logic;
   signal receiving : std_logic;
   signal data_in_sample : std_logic;
   
   -- Clock period definitions
   constant clk_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: universal_rx_core 
	GENERIC MAP (
		INPUT_DELAY_WIDTH => 4
	)
	PORT MAP (
          clk => clk,
          reset => reset,
          clk_rx => clk_rx,
          use_clk_rx => use_clk_rx,
          data_in_delay => data_in_delay,
          data_in_divide_delay => data_in_divide_delay,
          data_in => data_in,
          start => start,
          stop => stop,
          clear => clear,
          packet_count => packet_count,
          rx_clk_divide => rx_clk_divide,
		  ready => ready,
          receiving => receiving,
          data_in_sample => data_in_sample,
		  data_out => data_out,
		  data_out_re => data_out_re,
			 packet_size_out => packet_size_out,
          packet_size_out_re => packet_size_out_re
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
		variable data: std_logic_vector(15 downto 0) := "1100110011110001";
		variable data2: std_logic_vector(6 downto 0) := "1001011";
   begin		
      -- hold reset state for 100 ns.
      reset <= '1';
		start <= '0';
		stop <= '0';
		clear <= '0';
		rx_clk_divide <= "00000000";
		data_in_delay <= "0000";
		
		wait for 100 ns;	
		reset <= '0';
      wait for clk_period*10;

		----------------
      -- INPUT 1 -----
		----------------
		
		data_in <= '1';
		start <= '1';
		wait for clk_period;
		--data_in <= '0';
		start <= '0';
		--wait for clk_period;
		
		for i in 15 downto 1 loop
			data_in <= data(i);
			wait for clk_period;
		end loop;
		
		data_in <= data(0);
		stop <= '1';
		wait for clk_period;
		stop <= '0';
		wait for clk_period;
		
		----------------
      -- INPUT 2 -----
		----------------
		rx_clk_divide <= "00000011";
		data_in_delay <= "0000";
		data_in_divide_delay <= "0010";
		
		data_in <= '1';
		
		wait for clk_period*4*5;
		
		start <= '1';
		wait for clk_period;
		--data_in <= '0';
		start <= '0';
		--wait for clk_period;
		
		for i in 6 downto 1 loop
			data_in <= data2(i);
			wait for 4*clk_period;
		end loop;
		
		data_in <= data2(0);
		stop <= '1';
		wait for clk_period;
		stop <= '0';
		wait for 3*clk_period;
		
		----------------
      -- ReadOut -----
		----------------
		
		data_out_re <= '1';
		wait for clk_period;
		data_out_re <= '0';
		wait for clk_period;
		wait for clk_period;
		wait for clk_period;
		data_out_re <= '1';
		wait for clk_period;
		data_out_re <= '0';
		wait for clk_period;
		wait for clk_period;
		wait for clk_period;
      data_out_re <= '1';
		wait for clk_period;
		data_out_re <= '0';
		wait for clk_period;
		wait for clk_period;
		wait for clk_period;
		
		----------------
      -- ReadPakSize -
		----------------
		
		packet_size_out_re <= '1';
		wait for clk_period;
		packet_size_out_re <= '0';
		wait for clk_period;
		wait for clk_period;
		wait for clk_period;
		packet_size_out_re <= '1';
		wait for clk_period;
		packet_size_out_re <= '0';
		
      wait;
		wait;
   end process;

END;
