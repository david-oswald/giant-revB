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
-- Component name: dac_controller
-- Author: David Oswald <david.oswald@rub.de>
-- Date: 110730
--
-- Description: Controller for AD9283
--
-- Notes:
-- none
--  
-- Dependencies:
-- adc_fifo
-----------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- common stuff
library work;
use work.defaults.all;

-- for Xilinx primitives
library UNISIM;
use UNISIM.vcomponents.all;

entity adc_controller is
	port( 
		-- inputs
		clk : in std_logic;
		ce : in std_logic;
		reset : in std_logic;
		
		-- ADC input pins (from hardware)
		adc_in : in std_logic_vector(7 downto 0);
		
		-- ADC control register
		adc_control : in byte;

		-- ADC encode clock pin (to hardware)
		adc_encode : out std_logic;
		adc_encode_fb : in std_logic;
		
		-- Last value read from ADC
		adc_value : out byte
	);
end adc_controller;

architecture behavioral of adc_controller is
   -- constants
	
   -- components
   
	-- signals
	signal clk_inv : std_logic;
	signal ce_of : std_logic;
	signal adc_clk_int : std_logic;
	signal adc_in_buf : byte;
	signal adc_in_buf_int : byte;
	signal adc_in_buf_int2 : byte;
	signal adc_clk : std_logic;
	
	signal delay_busy, delay_clk, delay_data_cal, delay_data_ce, 
		delay_data_inc : std_logic;
		
	signal delay_inc_prev : std_logic;
	
	attribute IOB : string;
	attribute IOB of adc_in_buf_int : signal is "TRUE";
	--attribute IOB of adc_clk_int : signal is "TRUE";
	
	component input_sync
		generic
		(
			sys_w       : integer := 8;
			dev_w       : integer := 8
		);
		port
		(
			DATA_IN_FROM_PINS       : in    std_logic_vector(sys_w-1 downto 0);
			DATA_IN_TO_DEVICE       : out   std_logic_vector(dev_w-1 downto 0);
			CLK_IN                  : in    std_logic;  
			CLK_OUT                 : out   std_logic;
			CLK_RESET               : in    std_logic;  
			IO_RESET                : in    std_logic;
			-- Input, Output delay control signals
			DELAY_BUSY              : out   std_logic;
			DELAY_CLK               : in    std_logic;
			DELAY_DATA_CAL          : in    std_logic;
			DELAY_DATA_CE           : in    std_logic;                    -- Enable signal for delay 
			DELAY_DATA_INC          : in    std_logic                    -- Delay increment (high), decrement (low) signal
		); 
	end component;
	
	component clock_output_sync
		generic
		(
			sys_w       : integer := 1;
			dev_w       : integer := 1
		);
		port
		(
			DATA_OUT_FROM_DEVICE    : in  std_logic_vector(dev_w-1 downto 0);
			DATA_OUT_TO_PINS        : out std_logic_vector(sys_w-1 downto 0);
			CLK_TO_PINS             : out std_logic;
			CLK_IN                  : in    std_logic; 
			LOCKED_IN               : in    std_logic;
			LOCKED_OUT              : out   std_logic;
			CLK_RESET               : in    std_logic;
			IO_RESET                : in    std_logic
		);    
	end component;
	
begin
	-- assignments	
	clk_inv <= not clk;
	
	--adc_encode <= adc_clk;
	
	DATA_IN_SYNC_inst : input_sync
	port map
	(
		DATA_IN_FROM_PINS => adc_in,
		DATA_IN_TO_DEVICE => adc_in_buf_int,
		CLK_IN => adc_encode_fb,
		CLK_OUT => adc_clk,
		CLK_RESET => reset,
		IO_RESET => reset,
		DELAY_BUSY => delay_busy,    
		DELAY_CLK  => delay_clk,    
		DELAY_DATA_CAL => delay_data_cal,
		DELAY_DATA_CE => delay_data_ce,
		DELAY_DATA_INC  => delay_data_inc
	);
	
	--adc_clk <= clk;
	--adc_in_buf_int <= adc_in;
	
	delay_clk <= adc_clk;
	delay_data_cal <= adc_control(1);
	delay_data_inc <= '1';
	
	DELAY_ADJUST: process(clk)
	begin
		if rising_edge(clk) then
			if(reset = '1') then
				delay_inc_prev <= '0';
				delay_data_ce <= '0';
			elsif ce = '1' then
				if delay_inc_prev = '0' and adc_control(0) = '1' then
					delay_data_ce <= '1';
				else
					delay_data_ce <= '0';
				end if;
				
				delay_inc_prev <= adc_control(0);
				
			end if;
		end if;
	end process DELAY_ADJUST;
	
	CLOCK_OUT_SYNC_inst : clock_output_sync
	port map
	(
		DATA_OUT_FROM_DEVICE(0) => '1',
		DATA_OUT_TO_PINS(0) => open,
		CLK_TO_PINS => adc_encode,
		CLK_IN => clk,
		LOCKED_IN => '1',
		LOCKED_OUT => open,
		CLK_RESET => reset,
		IO_RESET => '0'
	);

    --ODDR2_inst : ODDR2
    --generic map(
    --   DDR_ALIGNMENT => "C0", 
    --   INIT => '0', 
    --   SRTYPE => "ASYNC"
	-- ) 
    --port map (
    --   Q => adc_encode, -- 1-bit output data
    --   C0 => clk, -- 1-bit clock input
    --   C1 => clk_inv, -- 1-bit clock input
    --   CE => ce_of,  -- 1-bit clock enable input
    --   D0 => '1',   -- 1-bit data input (associated with C0)
    --   D1 => '0',   -- 1-bit data input (associated with C1)
    --   R => reset,    -- 1-bit reset input
    --   S => '0'     -- 1-bit set input
    --);
    --
	--adc_clk_int <= adc_encode_fb;
	--
	--INPUT_SYNC_PROC: process(adc_clk_int)
	--begin
	--	if rising_edge(adc_clk_int) then
	--		if(reset = '1') then
	--			adc_in_buf_int <= (others => '0');
	--			adc_in_buf_int2 <= (others => '0');
	--			adc_in_buf <= (others => '0');
	--		elsif ce = '1' then
	--			adc_in_buf_int <= adc_in;
	--			adc_in_buf_int2 <= adc_in_buf_int ;
	--			adc_in_buf <= adc_in_buf_int2;
	--		end if;
	--	end if;
	--end process;
	
	INPUT_SYNC_PROC: process(adc_clk)
	begin
		if rising_edge(adc_clk) then
			if(reset = '1') then
				adc_in_buf <= (others => '0');
			else
				adc_in_buf <= adc_in_buf_int;
			end if;
		end if;
	end process;
	
	MAIN: process(clk)
	begin
		if rising_edge(clk) then
			if(reset = '1') then
				ce_of <= '0';
				--adc_in_buf <= (others => '0');
				adc_value <= (others => '0');
			elsif ce = '1' then
				ce_of <= '1';
				--adc_in_buf <= adc_in;
				adc_value <= adc_in_buf;
			end if;
		end if;
	end process MAIN;

	
	
end behavioral;

