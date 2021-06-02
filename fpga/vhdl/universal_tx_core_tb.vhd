--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   10:53:21 04/03/2012
-- Design Name:   
-- Module Name:   U:/code/fpga/vhdl/universal_tx_core_tb.vhd
-- Project Name:  giant-ifx-ecc
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: universal_tx_core
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
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
 
ENTITY universal_tx_core_tb IS
END universal_tx_core_tb;
 
ARCHITECTURE behavior OF universal_tx_core_tb IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT universal_tx_core
    PORT(
         clk : IN  std_logic;
         reset : IN  std_logic;
         clk_tx : OUT  std_logic;
         data_out : out  std_logic;
		 data_out_valid : out std_logic;
		 ready : out std_logic;
		transmitting : out std_logic;
		 data_out_mode : in std_logic_vector(1 downto 0);	 
         start : IN  std_logic;
         stop : IN  std_logic;
         clear : IN  std_logic;
         tx_clk_divide : IN  std_logic_vector(7 downto 0);
         data_in : IN  std_logic_vector(7 downto 0);
         data_in_we : IN  std_logic;
         packet_size_in : IN  std_logic_vector(7 downto 0);
         packet_size_in_we : IN  std_logic;
         packet_count : OUT  std_logic_vector(7 downto 0)
        );
    END COMPONENT;
    

   --Inputs
   signal clk : std_logic := '0';
   signal reset : std_logic := '0';
   signal start : std_logic := '0';
   signal stop : std_logic := '0';
   signal clear : std_logic := '0';
   signal tx_clk_divide : std_logic_vector(7 downto 0) := (others => '0');
   signal data_in : std_logic_vector(7 downto 0) := (others => '0');
   signal data_in_we : std_logic := '0';
   signal packet_size_in : std_logic_vector(7 downto 0) := (others => '0');
   signal packet_size_in_we : std_logic := '0';
   signal data_out_mode : std_logic_vector(1 downto 0) := "00";
   

 	--Outputs
   signal clk_tx : std_logic;
   signal packet_count : std_logic_vector(7 downto 0);
   signal data_out : std_logic;
   signal data_out_valid : std_logic;
   signal ready : std_logic;
   signal transmitting : std_logic;
   
   -- Clock period definitions
   constant clk_period : time := 10 ns;
   constant clk_tx_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: universal_tx_core PORT MAP (
          clk => clk,
          reset => reset,
          clk_tx => clk_tx,
          data_out => data_out,
          data_out_valid => data_out_valid,
			 ready => ready,
			 transmitting => transmitting,
          data_out_mode => data_out_mode,
          start => start,
          stop => stop,
          clear => clear,
          tx_clk_divide => tx_clk_divide,
          data_in => data_in,
          data_in_we => data_in_we,
          packet_size_in => packet_size_in,
          packet_size_in_we => packet_size_in_we,
          packet_count => packet_count
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
		wait for 100 ns;	
		reset <= '0';
      wait for clk_period*10;

	
      ------------
		-- Data1 ---
		------------
		tx_clk_divide <= "00000100";
		data_in <= "11001100";
		data_in_we <= '1';
		--packet_size_in <= "00010000";
		-- 10 bit
		packet_size_in <= "00001010";
		packet_size_in_we <= '1';
		wait for clk_period;
		data_in_we <= '0';
		packet_size_in_we <= '0';
		wait for clk_period;
		
		data_in <= "10001011";
		data_in_we <= '1';
		wait for clk_period;
		data_in_we <= '0';
		wait for 3*clk_period;
		
		data_out_mode <= "00";
		
		start <= '1';
		wait for clk_period;
		start <= '0';

		wait until data_out_valid = '1';
		wait until data_out_valid = '0';
		wait for 10.5*clk_period;
		
		------------
		-- Data2 ---
		------------
		tx_clk_divide <= "00000000";
		data_in <= "11110000";
		data_in_we <= '1';
		packet_size_in <= "00010000";
		packet_size_in_we <= '1';
		wait for clk_period;
		data_in_we <= '0';
		packet_size_in_we <= '0';
		wait for clk_period;
		
		data_in <= "10101010";
		data_in_we <= '1';
		wait for clk_period;
		data_in_we <= '0';
		wait for 3*clk_period;
		
		start <= '1';
		wait for clk_period;
		start <= '0';
		wait for clk_period;
		
		
      wait;
		
   end process;

END;
