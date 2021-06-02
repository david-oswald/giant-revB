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
-- Component name: gpio_swicth
-- Author: David Oswald <david.oswald@rub.de>
-- Date: 09:32 26.11.2010
--
-- Description: Input muxer
--
-- Notes:
-- none
--  
-- Dependencies:
-- none
--
-----------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.defaults.all;

library UNISIM;
use UNISIM.vcomponents.all;

entity input_mux is
	generic
	(
		PIN_INDEX : integer
	);
	port
	( 
		-- GPIO pins
		gpio : in std_logic_vector(7 downto 0);
		
		-- FPGA-side pin (input to FPGA)
		fpga_i : out std_logic;
		
		-- Select register
		select_d_out : in std_logic_vector(8*8-1 downto 0)
	);
end input_mux;

architecture behavioral of input_mux is
	-- constants
	
	-- signals
	alias io_0: byte is select_d_out(1*8-1 downto 0*8);
	alias io_1: byte is select_d_out(2*8-1 downto 1*8);
	alias io_2: byte is select_d_out(3*8-1 downto 2*8);
	alias io_3: byte is select_d_out(4*8-1 downto 3*8);
	alias io_4: byte is select_d_out(5*8-1 downto 4*8);
	alias io_5: byte is select_d_out(6*8-1 downto 5*8);
	alias io_6: byte is select_d_out(7*8-1 downto 6*8);
	alias io_7: byte is select_d_out(8*8-1 downto 7*8);
begin
	
	MUX_INPUT : process (gpio, io_0, io_2, io_3, io_4, io_5, io_6, io_7)
		begin
        if to_integer(unsigned(io_0)) = PIN_INDEX then
            fpga_i <= gpio(0);
        elsif to_integer(unsigned(io_1)) = PIN_INDEX then
            fpga_i <= gpio(1);
		elsif to_integer(unsigned(io_2)) = PIN_INDEX then
            fpga_i <= gpio(2);	
		elsif to_integer(unsigned(io_3)) = PIN_INDEX then
            fpga_i <= gpio(3);
		elsif to_integer(unsigned(io_4)) = PIN_INDEX then
            fpga_i <= gpio(4);
		elsif to_integer(unsigned(io_5)) = PIN_INDEX then
            fpga_i <= gpio(5);
		elsif to_integer(unsigned(io_6)) = PIN_INDEX then
            fpga_i <= gpio(6);
		elsif to_integer(unsigned(io_7)) = PIN_INDEX then
            fpga_i <= gpio(7);
		else
			fpga_i <= '0';
        end if;
		
    end process;
	
end behavioral;
