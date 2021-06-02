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
-- Description: Controller for gpio => giant
--
-- Notes:
-- none
--  
-- Dependencies:
-- u8_to_parallel
-- input_mux
-----------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.defaults.all;

library UNISIM;
use UNISIM.vcomponents.all;

entity gpio_switch is
	port( 
		clk : in std_logic;
		reset : in std_logic;
			
		-- GPIO pins
		gpio : inout std_logic_vector(7 downto 0);
		
		-- FPGA-side pins (input to FPGA)
		fpga_i : out std_logic_vector(31 downto 0);
		-- FPGA-side pins (output from FPGA)
		fpga_o : in std_logic_vector(31 downto 0);
		-- Enable output
		fpga_io_output : in std_logic_vector(31 downto 0);
		
		-- Tristate GPIO (= 0)
		gpio_enable : in std_logic;
		 
		-- Switch input shift register
		select_clear : std_logic;
		select_in : byte;
		select_in_w_en : std_logic
	);
end gpio_switch;

architecture behavioral of gpio_switch is
	-- constants
	
	-- components
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
	
	component input_mux is
		generic
		(
			PIN_INDEX : integer
		);
		port
		( 
			gpio : in std_logic_vector(7 downto 0);
			fpga_i : out std_logic;
			select_d_out : in std_logic_vector(8*8-1 downto 0)
		);
	end component;

	-- signals
	signal select_d_out : std_logic_vector(8*8-1 downto 0);
	alias io_0: byte is select_d_out(1*8-1 downto 0*8);
	alias io_1: byte is select_d_out(2*8-1 downto 1*8);
	alias io_2: byte is select_d_out(3*8-1 downto 2*8);
	alias io_3: byte is select_d_out(4*8-1 downto 3*8);
	alias io_4: byte is select_d_out(5*8-1 downto 4*8);
	alias io_5: byte is select_d_out(6*8-1 downto 5*8);
	alias io_6: byte is select_d_out(7*8-1 downto 6*8);
	alias io_7: byte is select_d_out(8*8-1 downto 7*8);
begin
	
	INPUT_SELECT : u8_to_parallel
	generic map
	(
		WIDTH => 8
	)
	port map
	( 
		clk => clk,
		reset => reset,
		d_in => select_in,
		w_en => select_in_w_en,
		clear => select_clear,
		count => open,
		d_out => select_d_out
	);

	-- switch GPIO according to current selection
	gpio(0) <= fpga_o(to_integer(unsigned(io_0))) when (gpio_enable = '1' and fpga_io_output(to_integer(unsigned(io_0))) = '1') else 'Z';
	gpio(1) <= fpga_o(to_integer(unsigned(io_1))) when (gpio_enable = '1' and fpga_io_output(to_integer(unsigned(io_1))) = '1') else 'Z';
	gpio(2) <= fpga_o(to_integer(unsigned(io_2))) when (gpio_enable = '1' and fpga_io_output(to_integer(unsigned(io_2))) = '1') else 'Z';
	gpio(3) <= fpga_o(to_integer(unsigned(io_3))) when (gpio_enable = '1' and fpga_io_output(to_integer(unsigned(io_3))) = '1') else 'Z';
	gpio(4) <= fpga_o(to_integer(unsigned(io_4))) when (gpio_enable = '1' and fpga_io_output(to_integer(unsigned(io_4))) = '1') else 'Z';
	gpio(5) <= fpga_o(to_integer(unsigned(io_5))) when (gpio_enable = '1' and fpga_io_output(to_integer(unsigned(io_5))) = '1') else 'Z';
	gpio(6) <= fpga_o(to_integer(unsigned(io_6))) when (gpio_enable = '1' and fpga_io_output(to_integer(unsigned(io_6))) = '1') else 'Z';
	gpio(7) <= fpga_o(to_integer(unsigned(io_7))) when (gpio_enable = '1' and fpga_io_output(to_integer(unsigned(io_7))) = '1') else 'Z';
	
	-- Input muxes
	MUX_INPUTS: for i in 31 downto 0 generate
        MUX_INPUT: input_mux 
		generic map
		(
			PIN_INDEX => i
		)
		port map
		(
			gpio => gpio,
			fpga_i => fpga_i(i),
			select_d_out => select_d_out
		);
    end generate;
	
	--fpga_i(to_integer(unsigned(io_0))) <= gpio(0); -- when (fpga_io_output(to_integer(unsigned(io_0))) = '0') else 'Z';
	--fpga_i(to_integer(unsigned(io_1))) <= gpio(1); -- when (fpga_io_output(to_integer(unsigned(io_0))) = '0') else 'Z';
	--fpga_i(to_integer(unsigned(io_2))) <= gpio(2); -- when (fpga_io_output(to_integer(unsigned(io_0))) = '0') else 'Z';
	--fpga_i(to_integer(unsigned(io_3))) <= gpio(3); -- when (fpga_io_output(to_integer(unsigned(io_0))) = '0') else 'Z';
	--fpga_i(to_integer(unsigned(io_4))) <= gpio(4); -- when (fpga_io_output(to_integer(unsigned(io_0))) = '0') else 'Z';
	--fpga_i(to_integer(unsigned(io_5))) <= gpio(5); -- when (fpga_io_output(to_integer(unsigned(io_0))) = '0') else 'Z';
	--fpga_i(to_integer(unsigned(io_6))) <= gpio(6); -- when (fpga_io_output(to_integer(unsigned(io_0))) = '0') else 'Z';
	--fpga_i(to_integer(unsigned(io_7))) <= gpio(7); -- when (fpga_io_output(to_integer(unsigned(io_0))) = '0') else 'Z';

end behavioral;
