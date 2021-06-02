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
-- Description: Controller to generate downsampled clock by CE 
--              control
--
-- Notes:
-- none
--  
-- Dependencies:
-- none
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

entity downsampling_controller is
	port( 
		-- inputs
		clk : in std_logic;
		ce : in std_logic;
		reset : in std_logic;
		
		-- Downsampling factor
		-- 0 means no downsampling, 1 every second cycle, 2 every third cycle 
		-- etc.
		downsampling : in byte;

		-- Clock enable pin to hardware
		ce_out: out std_logic
	);
end downsampling_controller;

architecture behavioral of downsampling_controller is
	-- constants

	-- components

	-- signals
	signal downsampling_counter : unsigned(7 downto 0);
	
begin
	
	MAIN: process(clk)
	begin
		if rising_edge(clk) then
			if(reset = '1') then
				ce_out <= '0';
				downsampling_counter <= (others => '0');
			elsif ce = '1' then
				if downsampling_counter = 0 then
					downsampling_counter <= unsigned(downsampling);
					ce_out <= '1';
				else
					downsampling_counter <= downsampling_counter - 1;
					ce_out <= '0';
				end if;
			end if;
		end if;
	end process MAIN;

	
	
end behavioral;

