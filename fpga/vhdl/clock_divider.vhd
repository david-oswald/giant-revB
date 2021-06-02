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
-- Component name: clock_divider
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

entity clock_divider is
	generic
	(
		FACTOR : positive := 1
	);
	port( 
		-- inputs
		clk : in std_logic;
		reset : in std_logic;

		-- enable output
		output_enable : in std_logic;
		
		-- Divided clock output
		clk_out: out std_logic
	);
end clock_divider;

architecture behavioral of clock_divider is
	-- constants

	-- components

	-- signals
	signal downsampling_counter : unsigned(31 downto 0);
	
begin
	
	MAIN: process(clk)
	begin
		if rising_edge(clk) then
			if(reset = '1') then
				clk_out <= '0';
				downsampling_counter <= (others => '0');
			else
				clk_out <= '0';
				
				if downsampling_counter = 0 then
					downsampling_counter <= to_unsigned(FACTOR, downsampling_counter'length);
				else
					downsampling_counter <= downsampling_counter - 1;
					
					if downsampling_counter < to_unsigned(FACTOR, downsampling_counter'length)/2 and output_enable = '1' then
						clk_out <= '1';
					end if;
					
				end if;
			end if;
		end if;
	end process MAIN;

	
	
end behavioral;

