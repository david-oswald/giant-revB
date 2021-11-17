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
-- Component name: main
-- Author: David Oswald <david.oswald@rub.de>
-- Date: 10:51 11.06.2012
--
-- Description: Top level of GIAnT (Generic Implementation Analysis Toolkit)
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

library UNISIM;
use UNISIM.vcomponents.all;

entity main is
	port( 
		-- standard inputs
		clk_in : in std_logic;
		reset_in : in std_logic;
		
		-- uC <-> FPGA interface
		w_en : in std_logic;
		r_en : in std_logic;
		in_pin : in std_logic;
		out_pin : out std_logic;
		
		-- LEDs
		led : out std_logic_vector(3 downto 0);
		
		-- smartcard
		sc_io : inout std_logic;
		sc_clk : out std_logic;
		sc_rst : out std_logic;
		sc_pin4 : in std_logic;
		sc_pin6 : in std_logic;
		sc_pin8 : in std_logic;
		sc_sw1 : out std_logic;
		sc_sw2 : in std_logic;
		
		-- DAC interface
		dac_v_out : out std_logic_vector(7 downto 0);
		dac_clk : out std_logic;
		
		-- ADC interface
		-- adc_v_in : in std_logic_vector(7 downto 0);
		-- adc_encode : out std_logic;
		-- adc_encode_fb : in std_logic;
		
		-- DDR interface
		-- mcb3_dram_dq    : inout std_logic_vector(15 downto 0);
		-- mcb3_dram_a     : out std_logic_vector(12 downto 0);
		-- mcb3_dram_ba    : out std_logic_vector(1 downto 0);
		-- mcb3_dram_cke   : out std_logic;
		-- mcb3_dram_ras_n : out std_logic;
		-- mcb3_dram_cas_n : out std_logic;
		-- mcb3_dram_we_n  : out std_logic;
		-- mcb3_dram_dm    : out std_logic;
		-- mcb3_dram_udqs  : inout  std_logic;
		-- mcb3_rzq        : inout  std_logic;
		-- mcb3_dram_udm   : out std_logic;
		-- mcb3_dram_dqs   : inout  std_logic;
		-- mcb3_dram_ck    : out std_logic;
		-- mcb3_dram_ck_n  : out std_logic;
		-- 
		-- -- USB FIFO
		-- IFCLK         : in std_logic;
		-- FD            : out std_logic_vector(15 downto 0); 
		-- SLOE          : out std_logic;
		-- SLRD          : out std_logic;
		-- SLWR          : out std_logic;
		-- FIFOADR0      : out std_logic;
		-- FIFOADR1      : out std_logic;
		-- PKTEND        : out std_logic;
		-- EMPTYFLAGC	  : in std_logic;
		-- FULLFLAGB	  : in std_logic;
		-- FLAGA	      : in std_logic;
			
		-- user I/O pins
		-- This is the GPIO1 port on the new FPGA board
		gpio1 : inout std_logic_vector(7 downto 0);
		
		-- GPIO2: to be done
		-- gpio2 : inout std_logic_vector(7 downto 0)
		
		-- transistor outputs
		transistors : out std_logic_vector(3 downto 0);
		
		-- Clk pins
		clk_r9 : out std_logic;
		clk_2  : out std_logic;
		clk_28 : out std_logic
	);
end main;

architecture behavioral of main is
	-- constants

	-- main clk frequency
	constant F_CLK : positive := 50000000;
	-- main clk period (in ns)
	constant T_CLK : positive := 20;

	-- first 32 (0...31) are read-only, others may be written
	constant RD_REG_COUNT : integer := 32;
	constant WR_REG_COUNT : integer := 64;
	constant REG_FILE_LENGTH : integer := RD_REG_COUNT+WR_REG_COUNT;
   
	-- components
	component io_controller is
		generic(
			WR_REG_COUNT : natural := 32;
			RD_REG_COUNT : natural := 32;
			-- Warning: this is overdimensioned, so if WR_REG_COUNT is larger than 256
			-- this will not correctly init
			WR_REG_INIT_VALUES : byte_vector(255 downto 0) := (others => (others => '0'))
		);
		port( 
			clk_in : in std_logic;
			reset_in : in std_logic;
			clk : in std_logic;
			reset : in std_logic;
			uc_in_w_en : in std_logic;
			uc_out_r_en : in std_logic;
			uc_in_pin : in std_logic;
			uc_out_pin : out std_logic;
			register_file_readonly : in byte_vector(RD_REG_COUNT-1 downto 0);
			register_file_writable : out byte_vector(WR_REG_COUNT-1 downto 0);
			register_file_r : out std_logic_vector(RD_REG_COUNT+WR_REG_COUNT-1 downto 0);
			register_file_w : out std_logic_vector(RD_REG_COUNT+WR_REG_COUNT-1 downto 0)
		);
	end component;
	
	component clock_domain_sync_1to8
		port (
			rst : in std_logic;
			wr_clk : in std_logic;
			rd_clk : in std_logic;
			din : in std_logic_vector(0 downto 0);
			wr_en : in std_logic;
			rd_en : in std_logic;
			dout : out std_logic_vector(7 downto 0);
			full : out std_logic;
			wr_ack : out std_logic;
			empty : out std_logic;
			valid : out std_logic
		);
	end component;
   
	-- component memory_interface is
	-- 	generic
	-- 	(
	-- 		C3_P0_MASK_SIZE           : integer := 4;
	-- 		C3_P0_DATA_PORT_SIZE      : integer := 32;
	-- 		C3_P1_MASK_SIZE           : integer := 4;
	-- 		C3_P1_DATA_PORT_SIZE      : integer := 32;
	-- 		C3_NUM_DQ_PINS          : integer := 16; 
	-- 		C3_MEM_ADDR_WIDTH       : integer := 13; 
	-- 		C3_MEM_BANKADDR_WIDTH   : integer := 2 
	-- 	);
	-- 	port (
	-- 		clk : in std_logic;
	-- 		reset : in std_logic; 
	-- 		single_write	 : in byte;
	-- 		single_write_w	 : in std_logic;
	-- 		single_write_commit : in std_logic;
	-- 		single_read	 	 : out byte;
	-- 		single_read_r	 : in std_logic;
	-- 		single_read_commit : in std_logic;
	-- 		address : in byte;
	-- 		address_w : in std_logic;
	-- 		block_count      : in byte;
	-- 		block_count_w    : in std_logic;
	-- 		slave_fifo_start : in std_logic;
	-- 		status : out byte;
    -- 
	-- 		mcb3_dram_dq    : inout std_logic_vector(C3_NUM_DQ_PINS-1 downto 0);
	-- 		mcb3_dram_a     : out std_logic_vector(C3_MEM_ADDR_WIDTH-1 downto 0);
	-- 		mcb3_dram_ba    : out std_logic_vector(C3_MEM_BANKADDR_WIDTH-1 downto 0);
	-- 		mcb3_dram_cke   : out std_logic;
	-- 		mcb3_dram_ras_n : out std_logic;
	-- 		mcb3_dram_cas_n : out std_logic;
	-- 		mcb3_dram_we_n  : out std_logic;
	-- 		mcb3_dram_dm    : out std_logic;
	-- 		mcb3_dram_udqs  : inout  std_logic;
	-- 		mcb3_rzq        : inout  std_logic;
	-- 		mcb3_dram_udm   : out std_logic;
	-- 		mcb3_dram_dqs   : inout  std_logic;
	-- 		mcb3_dram_ck    : out std_logic;
	-- 		mcb3_dram_ck_n  : out std_logic;
	-- 		dma_start       : in std_logic;
	-- 		dma_input       : in std_logic_vector(15 downto 0);
	-- 		dma_ce 			 : in std_logic;
	-- 		IFCLK         : in std_logic;
	-- 		FD            : out std_logic_vector(15 downto 0); 
	-- 		SLOE          : out std_logic;
	-- 		SLRD          : out std_logic;
	-- 		SLWR          : out std_logic;
	-- 		FIFOADR0      : out std_logic;
	-- 		FIFOADR1      : out std_logic;
	-- 		PKTEND        : out std_logic;
	-- 		EMPTYFLAGC	  : in std_logic;
	-- 		FULLFLAGB	  : in std_logic;
	-- 		FLAGA	      : in std_logic
	-- 	);
	-- end component;
	
	component sc_controller is
		generic(
			CLK_PERIOD : positive
		);
		port( 
			clk : in std_logic;
			reset : in std_logic;
			switch_power : in std_logic;
			transmit : in std_logic;
			data_in : in byte;
			data_in_we : in std_logic;
			data_in_count : out byte;
			data_out : out byte;
			data_out_count : out byte;
			data_out_re : in std_logic;
			status : out byte;
			data_sent_trigger : out std_logic;
			data_sending_trigger : out std_logic;
			sc_v_cc_en : out std_logic;
			sc_io : inout std_logic;
			sc_rst : out std_logic;
			sc_clk : out std_logic
		);
	end component;
	
	component pic_programmer is
		generic(
			CLK_PERIOD : positive
		);
		port( 
			clk : in std_logic;
			reset : in std_logic;
			data_in : in std_logic_vector(21 downto 0);
			has_data : in std_logic;
			get_response : in std_logic;
			send : in std_logic;
			prog_startstop : in std_logic;
			start_and_send : in std_logic;
			programming: out std_logic;
			data_out : out std_logic_vector(13 downto 0);
			v_dd_en : out std_logic;
			v_pp_en : out std_logic;
			pgm : out std_logic;
			ispclk : out std_logic;
			ispdat : inout std_logic;
			ispdat_output : out std_logic
		);
	end component;
	
	component dac_controller is
		port ( 
			clk : in std_logic;
			ce : in std_logic;
			reset : in std_logic;
			test_mode : in std_logic;
			voltage_low : in byte;
			voltage_high : in byte;
			voltage_off : in byte;
			voltage_select : in std_logic;
			voltage_update : in std_logic;
			off : in std_logic;
			voltage_out : out byte;
			sleep : out std_logic;
			clk_dac : out std_logic
		);
	end component;
	
	-- component ask_modulator is
	-- 	port( 
	-- 		clk : in std_logic;
	-- 		ce : in std_logic;
	-- 		reset : in std_logic;
	-- 		out_amplitude : in byte;
	-- 		data : in std_logic;
	-- 		modulated : out byte;
	-- 		field_reset : in std_logic;
	-- 		field_reset_done : out std_logic;
	-- 		field_reset_time_in : in byte;
	-- 		field_reset_time_in_we : in std_logic
	-- 	);
	-- end component;
	
	component timing_controller_waveform is
		generic( 
			TIME_REGISTER_WIDTH : positive
		);
		port( 
			clk : in std_logic;
			ce : in std_logic;
			reset : in std_logic;
			arm : in std_logic;
			disarm : in std_logic;
			trigger : in std_logic;
			armed : out std_logic;
			ready : out std_logic;
			inject_fault : out std_logic;
			addr : in std_logic_vector(9 downto 0);
			w_en : in std_logic;
			d_in : in std_logic_vector(7 downto 0);
			d_out : out std_logic_vector(7 downto 0)
		);
	end component;
	
	component clock_divider is
		generic
		(
			FACTOR : positive := 1
		);
		port( 
			clk : in std_logic;
			reset : in std_logic;
			output_enable : in std_logic;
			clk_out: out std_logic
		);
	end component;
	
	-- component adc_controller is
	-- 	port( 
	-- 		clk : in std_logic;
	-- 		ce : in std_logic;
	-- 		reset : in std_logic;
	-- 		adc_in : in std_logic_vector(7 downto 0);
	-- 		adc_control : in byte;
	-- 		adc_encode : out std_logic;
	-- 		adc_encode_fb : in std_logic;
	-- 		adc_value : out byte
	-- 	);
	-- end component;
	
	-- component pattern_detector is
	-- 	port( 
	-- 		clk : in std_logic;
	-- 		reset : in std_logic;
	-- 		ce : in std_logic;
	-- 		pattern_in : in u8;
	-- 		pattern_we: in std_logic;
	-- 		pattern_sample_count : in unsigned(7 downto 0);
	-- 		adc_in: in u8;
	-- 		adc_we : in std_logic;
	-- 		d_out : out unsigned(15 downto 0)
	-- 	);
	-- end component;
	-- 
	-- component trigger_generator is
	-- 	port (
	-- 		clk : in std_logic;
	-- 		reset : in std_logic; 
	-- 		arm : in std_logic;
	-- 		armed : out std_logic;
	-- 		coarse_trigger_en : in std_logic;
	-- 		coarse_trigger : in std_logic;
	-- 		force_trigger : in std_logic;
	-- 		detector_in : in unsigned(15 downto 0);
	-- 		threshold : in byte;
	-- 		threshold_w : in std_logic;
	-- 		trigger : out std_logic
	-- 	);
	-- end component;
   
   component clock_generator
		port (
			CLK_IN1           : in     std_logic;
			CLK_OUT1          : out    std_logic;
			CLK_OUT2          : out    std_logic;
			CLK_OUT3          : out    std_logic;
			RESET             : in     std_logic;
			LOCKED            : out    std_logic
		);
	end component;
	
	component downsampling_controller is
		port ( 
			clk : in std_logic;
			ce : in std_logic;
			reset : in std_logic;
			downsampling : in byte;
			ce_out: out std_logic
		);
	end component;
	
	component universal_tx_core
		port
		(
			clk : in std_logic;
			reset : in std_logic; 
			clk_tx : out std_logic;
			data_out : out std_logic;
			data_out_valid : out std_logic;
			ready : out std_logic;
			transmitting : out std_logic;
			transmit_start : out std_logic;
			transmit_done : out std_logic;
			data_out_mode : in std_logic_vector(1 downto 0);
			convert_one_to_Z : in std_logic;
			start : in std_logic;
			stop : in std_logic;
			clear : in std_logic;
			tx_clk_divide : in byte;
			tx_clk_divide_we : in std_logic;
			data_in : in byte;
			data_in_we : in std_logic;
			packet_size_in_lo : in byte;
			packet_size_in_hi : in byte;
			packet_size_in_lo_we : in std_logic;
			packet_size_in_hi_we : in std_logic;
			packet_count : out byte
		);
	end component;
	
	component universal_rx_core is
		generic (
			INPUT_DELAY_WIDTH : positive := 4
		);
		port (
			clk : in std_logic;
			reset : in std_logic; 
			clk_rx : in std_logic;
			use_clk_rx : in std_logic;
			data_in_delay : in unsigned(INPUT_DELAY_WIDTH-1 downto 0);
			data_in_divide_delay : in unsigned(INPUT_DELAY_WIDTH-1 downto 0);
			resync_to_falling_edges : in std_logic;
			resync_to_rising_edges : in std_logic;
			data_in : in std_logic;
			start : in std_logic;
			stop : in std_logic;
			clear : in std_logic;
			rx_clk_divide : in byte;
			rx_clk_divide_we : in std_logic;
			ready : out std_logic;
			receiving : out std_logic;
			data_in_sample : out std_logic;
			packet_count : out byte;
			data_out : out byte;
			data_out_re : in std_logic;
			packet_size_out_low : out byte;
			packet_size_out_high : out byte;
			packet_size_out_low_re : in std_logic;		
			packet_size_out_high_re : in std_logic		
		);
	end component;
	
	component universal_trigger_core is
		port 
		(
			clk : in std_logic;
			reset : in std_logic; 
			arm : in std_logic;
			armed : out std_logic;
			triggered : out std_logic;
			delay_in : in byte;
			delay_in_we : in std_logic;
			hold_time_in : in byte;
			hold_time_in_we : in std_logic;
			force_trigger : in std_logic;
			input_mode : unsigned(2 downto 0);
			input : in std_logic;
			output_mode : unsigned(2 downto 0);
			trigger : out std_logic
		);
	end component;
	
	component gpio_switch is
		port( 
			clk : in std_logic;
			reset : in std_logic;
			gpio : inout std_logic_vector(7 downto 0);
			gpio_enable : in std_logic;
			fpga_i : out std_logic_vector(31 downto 0);
			fpga_o : in std_logic_vector(31 downto 0);
			fpga_io_output : in std_logic_vector(31 downto 0);
			select_clear : std_logic;
			select_in : byte;
			select_in_w_en : std_logic
		);
	end component;

	component BUFG
		port(
			I: in STD_LOGIC; 
			O: out STD_LOGIC
		);
	end component;
	
	-- signals
   
    -- gpio mux
	signal gpio_fpga_i : std_logic_vector(31 downto 0);
	signal gpio_fpga_o : std_logic_vector(31 downto 0);
	signal gpio_fpga_io_output : std_logic_vector(31 downto 0);
	signal gpio_select_in : byte;
	signal gpio_control : byte;
	signal gpio_outputs : byte;
	signal gpio_inputs  : byte;
	signal gpio_select_in_w_en : std_logic;
	
	-- universal tx core
	signal utx_clk_tx : std_logic;
	signal utx_data_out : std_logic;
	signal utx_data_out_valid : std_logic;
	signal utx_ready : std_logic;
	signal utx_start : std_logic;
	signal utx_transmitting : std_logic;
	signal utx_transmit_done: std_logic;
	signal utx_transmit_start: std_logic;
	signal utx_convert_one_to_Z : std_logic;
	signal utx_data_out_mode : std_logic_vector(1 downto 0);
	signal utx_tx_clk_divide : byte;
	signal utx_data_in : byte;
	signal utx_control : byte;
	signal utx_packet_size_in_lo, utx_packet_size_in_hi : byte;
	signal utx_packet_count : byte;
	
	-- universal rx core
	constant URX_INPUT_DELAY_WIDTH : positive := 4;
	signal urx_data_in : std_logic;
	signal urx_data_out : byte;
	signal urx_data_in_delay : unsigned(URX_INPUT_DELAY_WIDTH-1 downto 0);
	signal urx_data_in_divide_delay : unsigned(URX_INPUT_DELAY_WIDTH-1 downto 0);
	signal urx_rx_clk_divide : byte;
	signal urx_ready : std_logic;
	signal urx_receiving : std_logic;
	signal urx_start : std_logic;
	signal urx_stop : std_logic;
	signal urx_status : byte;
	signal urx_control : byte;
	signal urx_packet_size_out_low, urx_packet_size_out_high : byte;
	signal urx_packet_count : byte;
	
	-- trigger cores for start/stop
	signal utrig1_control, utrig2_control : byte;
	signal utrig1_status, utrig2_status : byte;
	signal utrig1_delay, utrig2_delay : byte;
	signal utrig1_hold, utrig2_hold: byte;
	signal utrig1_in, utrig2_in : std_logic;
	signal utrig1_trigger, utrig2_trigger : std_logic;
   
    -- pattern detector
	-- signal detector_ce : std_logic;
	-- signal detector_adc_we : std_logic;
	-- signal detector_pattern_in : byte;
	-- signal detector_adc_in : byte;
	-- signal detector_out : unsigned(15 downto 0);
	-- signal detector_pattern_sample_count : unsigned(7 downto 0);
	
	-- ADC controller
	-- signal adc_control : byte;
   
	-- DDR controller
	-- signal ddr_single_write, ddr_single_read : byte;
	-- signal ddr_control, ddr_address, ddr_status, ddr_fifo_count : byte;
	-- signal ddr_dma_in : std_logic_vector(15 downto 0);
	signal ddr_dma_start, ddr_dma_ce : std_logic;
	
	-- smartcard
	signal sc_data_in, sc_data_out, sc_control, sc_status : byte;
	signal sc_data_in_count, sc_data_out_count : byte;
	signal sc_vcc_en, sc_clk_gen : std_logic;
	signal sc_data_sent_trigger : std_logic;
	signal sc_data_sending_trigger : std_logic;

	-- DCM signals
	--signal clk, clk_50, clk_50, clk_90, clk_180, clk_270 : std_logic;
	--signal clk_100, clk_100_180 : std_logic;
	--signal clk_div : std_logic;
	--signal clk_fx, clk_fx_180 : std_logic;
	--signal clk_fb, clk_locked, clk_psdone : std_logic;
	--signal clk_psclk, clk_psen, clk_psincdec : std_logic;
	--signal clk_status : byte;
	signal clk : std_logic;
	signal clk_locked: std_logic;	
	signal clk_50, clk_100, clk_48 : std_logic;
   
	-- PIC programmer
	signal pic_data_in: std_logic_vector(21 downto 0);
	signal pic_control: byte;
	signal pic_data_out : std_logic_vector(13 downto 0);
	signal pic_ispdat_output, pic_v_dd_en, pic_v_pp_en, pic_pgm, pic_ispclk, pic_ispdat, pic_programming : std_logic;
	
	-- DAC controller
	signal dac_ce, dac_test_mode, dac_v_select, dac_v_update, dac_off : std_logic;
	signal dac_v_high, dac_v_low, dac_v_off, dac_control : byte;
	
	-- Timing controller
	signal fi_control, fi_status : byte;
	signal fi_ce, fi_arm, fi_trigger, fi_armed, fi_ready : std_logic;
	signal fi_disarm : std_logic;
	signal fi_inject_fault : std_logic;
	signal fi_addr : std_logic_vector(9 downto 0);
	signal fi_w_en : std_logic;
	signal fi_d_in, fi_d_out : byte;
	signal fi_trigger_control : byte;
	signal fi_universal_trigger_control : byte;
	signal fi_trigger_ext : std_logic;
	signal fi_universal_trigger : std_logic;
	
	-- Timing controller for UTX triggering
	signal utiming_control, utiming_status : byte;
	signal utiming_arm, utiming_trigger, utiming_armed, utiming_ready : std_logic;
	signal utiming_disarm: std_logic;
	signal utiming_out: std_logic;
	signal utiming_addr : std_logic_vector(9 downto 0);
	signal utiming_w_en : std_logic;
	signal utiming_d_in, utiming_d_out : byte;
	
	-- ADC controller
	-- signal thresh_control : byte;
	-- signal thresh_status : byte;
	-- signal adc_value : byte;
	-- signal adc_ce : std_logic;
	-- signal thresh_value: byte;
	signal thresh_trigger : std_logic;
	signal thresh_armed : std_logic;
	-- signal thresh_coarse_trigger : std_logic;
	-- signal thresh_force_trigger : std_logic;
	
	-- RFID interface
	signal rfid_trigger : std_logic;
	signal rfid_to_dac_enabled : std_logic;
	signal rfid_ask_modulated : byte;
	signal rfid_ask_data : std_logic;
	signal rfid_ask_amplitude : byte;
	signal rfid_control : byte;
	signal rfid_field_reset : std_logic;
	signal rfid_field_reset_done : std_logic;
	signal rfid_field_reset_time_in : byte;
	
	-- Downsampling
	-- signal scope_downsampling_ce_out : std_logic;
	-- signal scope_downsampling_factor : byte;
	-- signal detector_downsampling_ce_out : std_logic;
	-- signal detector_downsampling_factor : byte;
	
	-- register file of 64 registers
	signal register_file_readonly : byte_vector(RD_REG_COUNT-1 downto 0);
	signal register_file_writable : byte_vector(WR_REG_COUNT-1 downto 0);
	alias  register_file_writable_5 : byte is register_file_writable(5);
	alias  register_file_writable_13 : byte is register_file_writable(13);
	alias  register_file_writable_21 : byte is register_file_writable(21);
	alias  register_file_writable_31 : byte is register_file_writable(31);
	alias  register_file_readonly_22 : byte is register_file_readonly(22);


	-- read/write strobes for register_file
	signal register_file_r : std_logic_vector(REG_FILE_LENGTH-1 downto 0);
	signal register_file_w : std_logic_vector(REG_FILE_LENGTH-1 downto 0);

	-- internal reset
	signal reset : std_logic;
begin
	-- components
	
	-- Universal triggers
	-- Both inputs are connected to the internal urx_data_in pin
	
	-- Register mapping:
	-- utrig1_status       (r)  : 27
	-- utrig1_control      (r/w): 74 (32 + 42)
	-- utrig1_delay        (r/w): 75 (32 + 43)
	-- utrig1_hold         (r/w): 76 (32 + 44)
	UTRIG1_inst : universal_trigger_core 
	port map
	(
		clk => clk,
		reset => reset,
		arm => utrig1_control(0),
		armed => utrig1_status(0),
		triggered => utrig1_status(1),
		delay_in => utrig1_delay,
		delay_in_we => register_file_w(75),
		hold_time_in => utrig1_hold,
		hold_time_in_we => register_file_w(76),
		force_trigger => utrig1_control(1),
		input_mode => unsigned(utrig1_control(4 downto 2)),
		input => utrig1_in,
		output_mode => unsigned(utrig1_control(7 downto 5)),
		trigger => utrig1_trigger
	);
	
	utrig1_in <= urx_data_in;
	
	-- Control register
	-- 0: Arm
	-- 1: Force trigger
	-- 4, 3, 2: Input mode
	-- 7, 6, 5: Output mode
	utrig1_control <= register_file_writable(42);
	utrig1_delay <= register_file_writable(43);
	utrig1_hold <= register_file_writable(44);
	
	-- Status register
	-- 0: Armed
	-- 1: Triggered
	utrig1_status(7 downto 2) <= (others => '0');
	register_file_readonly(27) <= utrig1_status;
	
	-- Register mapping:
	-- utrig2_status       (r)  : 28
	-- utrig2_control      (r/w): 77 (32 + 45)
	-- utrig2_delay        (r/w): 78 (32 + 46)
	-- utrig2_hold         (r/w): 79 (32 + 47)
	UTRIG2_inst : universal_trigger_core 
	port map
	(
		clk => clk,
		reset => reset,
		arm => utrig2_control(0),
		armed => utrig2_status(0),
		triggered => utrig2_status(1),
		delay_in => utrig2_delay,
		delay_in_we => register_file_w(78),
		hold_time_in => utrig2_hold,
		hold_time_in_we => register_file_w(79),
		force_trigger => utrig2_control(1),
		input_mode => unsigned(utrig2_control(4 downto 2)),
		input => utrig2_in,
		output_mode => unsigned(utrig2_control(7 downto 5)),
		trigger => utrig2_trigger
	);
	
	utrig2_in <= urx_data_in;
	
	-- Control register
	-- 0: Arm
	-- 1: Force trigger
	-- 4, 3, 2: Input mode
	-- 7, 6, 5: Output mode
	utrig2_control <= register_file_writable(45);
	utrig2_delay <= register_file_writable(46);
	utrig2_hold <= register_file_writable(47);
	
	-- Status register
	-- 0: Armed
	-- 1: Triggered
	utrig2_status(7 downto 2) <= (others => '0');
	register_file_readonly(28) <= utrig2_status;
	
	-- URX interface
	-- Register mapping:
	-- urx_status               (r)  : 23
	-- urx_data_out             (r)  : 24
	-- urx_packet_size_out_low  (r)  : 25
	-- urx_packet_size_out_high (r)  : 29
	-- urx_packet_count         (r)  : 26
	-- urx_control              (r/w): 71 (32 + 39)
	-- urx_rx_clk_divide        (r/w): 72 (32 + 40)
	-- urx_delay                (r/w): 73 (32 + 41)
	UNIVERSAL_RX_CORE_inst : universal_rx_core 
	generic map
	(
		INPUT_DELAY_WIDTH => 4
	)
	port map
	(
		clk => clk,
		reset => reset,
		clk_rx => '0',
		use_clk_rx => '0',
		data_in_delay => urx_data_in_delay,
		data_in_divide_delay => urx_data_in_divide_delay,
		resync_to_rising_edges => urx_control(3),
		resync_to_falling_edges => urx_control(4),
		data_in => urx_data_in,
		start => urx_control(0) or urx_start,
		stop => urx_control(1) or urx_stop,
		clear => urx_control(2),
		rx_clk_divide => urx_rx_clk_divide,
		rx_clk_divide_we  => register_file_w(72),
		ready => urx_status(0),
		receiving => urx_status(1),
		data_in_sample => urx_status(2),
		packet_count => urx_packet_count,
		data_out => urx_data_out,
		data_out_re => register_file_r(24),
		packet_size_out_low => urx_packet_size_out_low,
		packet_size_out_high => urx_packet_size_out_high,
		packet_size_out_low_re => register_file_r(25),
		packet_size_out_high_re => register_file_r(29)
	);
	
	urx_start <= utrig1_trigger;
	urx_stop <= utrig2_trigger;
	
	urx_control <= register_file_writable(39);
	urx_rx_clk_divide <= register_file_writable(40);
	urx_data_in_delay <= unsigned(register_file_writable(41)(3 downto 0));
	urx_data_in_divide_delay <= unsigned(register_file_writable(41)(7 downto 4));
		
	urx_status(7 downto 3) <= (others => '0');
	register_file_readonly(23) <= urx_status;
	register_file_readonly(24) <= urx_data_out;
	register_file_readonly(25) <= urx_packet_size_out_low;
	register_file_readonly(29) <= urx_packet_size_out_high;
	register_file_readonly(26) <= urx_packet_count;
	
	
	-- UTX interface
	-- Register mapping:
	-- utx_status            (r)  : 22 
	-- utx_control           (r/w): 67 (32 + 35)
	-- utx_data_in           (r/w): 68 (32 + 36)
	-- utx_packet_size_in_lo (r/w): 69 (32 + 37)
	-- utx_packet_size_in_hi (r/w): 80 (32 + 48)
	-- utx_tx_clk_divide     (r/w): 70 (32 + 38)
	UNIVERSAL_TX_CORE_inst : universal_tx_core
	port map
	(
		clk               => clk_50,
		reset             => reset,
		clk_tx            => utx_clk_tx,
		data_out          => utx_data_out,
		data_out_valid    => utx_data_out_valid,
		ready             => utx_ready,
		transmitting      => utx_transmitting,
		transmit_done     => utx_transmit_done,
		transmit_start    => utx_transmit_start,
		data_out_mode     => utx_data_out_mode,
		convert_one_to_Z  => utx_convert_one_to_Z,
		start             => utx_start,
		stop              => utx_control(1),
		clear             => utx_control(2),
		tx_clk_divide     => utx_tx_clk_divide,
		tx_clk_divide_we  => register_file_w(70),
		data_in           => utx_data_in,
		data_in_we        => register_file_w(68),
		packet_size_in_lo    => utx_packet_size_in_lo ,
		packet_size_in_hi    => utx_packet_size_in_hi ,
		packet_size_in_lo_we => register_file_w(69),
		packet_size_in_hi_we => register_file_w(80),
		packet_count      => utx_packet_count
	);
	
	-- Register mapping
	-- 0: Start
	-- 1: Stop
	-- 2: Clear
	-- 4 ... 3 : Data out mode
	-- 5: Convert 1 to tristate
	-- 6: Disable output to GPIO pin
	-- 7: Enable sending via utiming
	utx_control <= register_file_writable(35);
	utx_data_out_mode <= utx_control(4 downto 3);
	utx_convert_one_to_Z <= utx_control(5);
	
	utx_start <= utx_control(0) when utx_control(7) = '0' else
					 utiming_out;
					 
	utx_data_in <= register_file_writable(36);
	utx_packet_size_in_lo <= register_file_writable(37);
	utx_packet_size_in_hi <= register_file_writable(48);
	utx_tx_clk_divide <= register_file_writable(38);
	
	register_file_readonly(14) <= utx_packet_count;
	
	-- Status register
	-- 0: Ready
	-- 1: Transmitting
	register_file_readonly_22(0) <= utx_ready;
	register_file_readonly_22(1) <= utx_transmitting;
	register_file_readonly_22(7 downto 2) <= (others => '0');
	
	-- IO/register controller
	IO_CONTROLLER_inst : io_controller
	generic map(
		WR_REG_COUNT => WR_REG_COUNT,
		RD_REG_COUNT => RD_REG_COUNT,
		-- Default some DAC values to mid-point instead of zero:
		-- dac_v_low      (r/w): 40 (32 + 8)
		-- dac_v_high     (r/w): 41 (32 + 9)
		-- dac_v_off      (r/w): 46 (32 + 14)
		WR_REG_INIT_VALUES => (
			8 => "10000000", 
			9 => "10000000", 
			14 => "10000000", 
			others => (others => '0')
		)
	)
	port map( 
		clk_in => clk_48,
		reset_in => reset_in,
		clk => clk,
		reset => reset,
		uc_in_w_en => w_en,
		uc_out_r_en => r_en,
		uc_in_pin => in_pin,
		uc_out_pin => out_pin,
		register_file_readonly => register_file_readonly,
		register_file_writable => register_file_writable,
		register_file_r => register_file_r,
		register_file_w => register_file_w
	);
	
	-- DDR controller
	-- Register mapping:
	-- ddr_in_low         (r): 11
	-- ddr_in_high        (r): 19
	-- ddr_single_read    (r): 17 
	-- ddr_status         (r): 18
	-- ddr_control        (r/w): 56 (32 + 24)
	-- ddr_single_write   (r/w): 57 (32 + 25)
	-- ddr_address        (r/w): 58 (32 + 26)
	-- ddr_fifo_count     (r/w): 59 (32 + 27)
	
	-- Control register bits
	-- 0: Single write commit
	-- 1: Single read commit
	-- 2: Start slave FIFO read
	-- 3: Reset memory interface
	-- 4: Start DMA write
	-- 5: Select 0 for DMA input source selection
	-- 6: Select 1 for DMA input source selection
	-- ddr_control <= register_file_writable(24);
	-- ddr_single_write <= register_file_writable(25);
	-- ddr_address <= register_file_writable(26);
	-- ddr_fifo_count <= register_file_writable(27);
	-- register_file_readonly(17) <= ddr_single_read;
	-- register_file_readonly(18) <= ddr_status;
	-- register_file_readonly(11) <= ddr_dma_in(7 downto 0);
	-- register_file_readonly(19) <= ddr_dma_in(15 downto 8);
	-- 
	-- DDR_inst : memory_interface
	-- port map(
	-- 	clk => clk_50,
	-- 	--clk => clk_100,
	-- 	reset => reset, -- or ddr_control(3)
	-- 	single_write => ddr_single_write,
	-- 	single_write_w => register_file_w(57),
	-- 	single_write_commit => ddr_control(0),
	-- 	single_read	=> ddr_single_read,
	-- 	single_read_r => register_file_r(17),
	-- 	single_read_commit => ddr_control(1),
	-- 	address => ddr_address,
	-- 	address_w => register_file_w(58),
	-- 	block_count => ddr_fifo_count,
	-- 	block_count_w => register_file_w(59),
	-- 	slave_fifo_start => ddr_control(2),
	-- 	status => ddr_status,
	-- 	mcb3_dram_dq     => mcb3_dram_dq,    
	-- 	mcb3_dram_a      => mcb3_dram_a,     
	-- 	mcb3_dram_ba     => mcb3_dram_ba,    
	-- 	mcb3_dram_cke    => mcb3_dram_cke,   
	-- 	mcb3_dram_ras_n  => mcb3_dram_ras_n, 
	-- 	mcb3_dram_cas_n  => mcb3_dram_cas_n, 
	-- 	mcb3_dram_we_n   => mcb3_dram_we_n, 
	-- 	mcb3_dram_dm     => mcb3_dram_dm,    
	-- 	mcb3_dram_udqs   => mcb3_dram_udqs,  
	-- 	mcb3_rzq         => mcb3_rzq,        
	-- 	mcb3_dram_udm    => mcb3_dram_udm,   
	-- 	mcb3_dram_dqs    => mcb3_dram_dqs,   
	-- 	mcb3_dram_ck     => mcb3_dram_ck,    
	-- 	mcb3_dram_ck_n   => mcb3_dram_ck_n,  
	-- 	dma_start        => ddr_dma_start,
	-- 	dma_input        => ddr_dma_in,
	-- 	dma_ce           => ddr_dma_ce,
	-- 	IFCLK       => IFCLK,
	-- 	FD          => FD,         
	-- 	SLOE        => SLOE,      
	-- 	SLRD        => SLRD,      
	-- 	SLWR        => SLWR,      
	-- 	FIFOADR0    => FIFOADR0,  
	-- 	FIFOADR1    => FIFOADR1,  
	-- 	PKTEND      => PKTEND,    
	-- 	EMPTYFLAGC	=> EMPTYFLAGC,
	-- 	FULLFLAGB 	=> FULLFLAGB,
	-- 	FLAGA	    => FLAGA	  
	-- );
	-- 
	-- ddr_dma_ce <= scope_downsampling_ce_out;
	-- 
	-- ddr_dma_in <= "00000000" & std_logic_vector(adc_value) when ddr_control(6 downto 5) = "00" else
	-- 	          std_logic_vector(detector_out) when ddr_control(6 downto 5) = "01" else
	-- 			  "00000000" & std_logic_vector(adc_value);
	-- 
	-- -- DMA is either started from software (via DDR control) or from the
	-- -- threshold trigger generator
	-- ddr_dma_start <= ddr_control(4) or thresh_trigger;
	
	-- Smartcard controller
	-- Register mapping:
	-- sc_control        (r/w): 34 (32 + 2)
	-- sc_data_in        (r/w): 35 (32 + 3)
	-- sc_status           (r): 3  
	-- sc_data_out         (r): 4
	-- sc_data_out_count   (r): 9
	-- sc_data_in_count    (r): 10
	sc_control <= register_file_writable(2);
	sc_data_in <= register_file_writable(3);
	register_file_readonly(3) <= sc_status;
	register_file_readonly(4) <= sc_data_out;
	register_file_readonly(9) <= sc_data_out_count;
	register_file_readonly(10) <= sc_data_in_count;
	
	SC_CTRL_inst : sc_controller
	generic map(
		CLK_PERIOD => T_CLK
	)
	port map(
		clk => clk_50,
		reset => reset,
		switch_power => sc_control(0),
		transmit => sc_control(1),
		data_in => sc_data_in,
		data_in_we => register_file_w(35),
		data_in_count => sc_data_in_count,
		data_out => sc_data_out,
		data_out_count => sc_data_out_count,
		data_out_re => register_file_r(4),
		status => sc_status,
		data_sent_trigger => sc_data_sent_trigger,
		data_sending_trigger => sc_data_sending_trigger,
		sc_v_cc_en => sc_vcc_en,
		sc_io => sc_io,
		sc_rst => sc_rst,
		sc_clk => sc_clk_gen
	);
	
	sc_clk <= sc_clk_gen;
	sc_sw1 <= '1';
	
	-- PIC programmer
	-- Register mapping:
	-- pic_control    (r/w): 36 (32 + 4)
	-- pic_command    (r/w): 37 (32 + 5)
	-- pic_data_in_l  (r/w): 38 (32 + 6)
	-- pic_data_in_h  (r/w): 39 (32 + 7)
	-- pic_data_out_l (r): 5
	-- pic_data_out_h (r): 6
	pic_control <= register_file_writable(4);
	pic_data_in(5 downto 0) <=  register_file_writable_5(5 downto 0);--std_logic_vector(resize(unsigned(register_file_writable(5)), 6));
	pic_data_in(13 downto 6) <= register_file_writable(6);
	pic_data_in(21 downto 14) <= register_file_writable(7);
	
	register_file_readonly(5) <= pic_data_out(7 downto 0);
	register_file_readonly(6) <= "00" & pic_data_out(13 downto 8);
	
	PIC_inst : pic_programmer 
	generic map(
		CLK_PERIOD => T_CLK
	)
	port map( 
		clk => clk,
		reset => reset,
		data_in => pic_data_in,
		has_data => pic_control(0),
		get_response => pic_control(1),
		send => pic_control(2),
		prog_startstop => pic_control(3),
		start_and_send => pic_control(4),
		programming => pic_programming,
		data_out => pic_data_out,
		v_dd_en => pic_v_dd_en,
		v_pp_en => pic_v_pp_en,
		pgm => pic_pgm,
		ispclk => pic_ispclk,
		ispdat => pic_ispdat,
		ispdat_output => pic_ispdat_output
	);
	
	-- Register mapping:
	-- rfid_reset_time (r/w): 54 (32 + 22)
	-- rfid_control    (r/w): 55 (32 + 23)
	-- ask_modulator_inst : ask_modulator
	-- port map( 
	-- 	clk => clk_100,
	-- 	ce => '1',
	-- 	reset => reset, -- or utiming_trigger,
	-- 	out_amplitude => rfid_ask_amplitude,
	-- 	data => rfid_ask_data,
	-- 	modulated => rfid_ask_modulated,
	-- 	field_reset =>  rfid_field_reset,
	-- 	field_reset_done => rfid_field_reset_done, 
	-- 	field_reset_time_in => rfid_field_reset_time_in,
	-- 	field_reset_time_in_we => register_file_w(54)
	-- );
	-- 
	-- -- control register
	-- -- 0: Enable/Disable
	-- -- 1: Trigger field reset
	-- rfid_control <= register_file_writable(23);
	-- 
	-- rfid_field_reset <= rfid_control(1);
	-- rfid_field_reset_time_in <= register_file_writable(22);
	-- 
	-- rfid_ask_amplitude <= register_file_writable(9) when fi_inject_fault = '0' else
	-- 					  register_file_writable(8);
	-- 	
	-- 	
	-- rfid_ask_data <= utx_data_out when rfid_control(0) = '1' else
	-- 	             '0';
    -- 
	-- rfid_trigger <= utx_transmitting when rfid_control(0) = '1' else
	-- 	            '0';
	
	
	-- DAC controller
	-- Register mapping:
	-- dac_v_low      (r/w): 40 (32 + 8)
	-- dac_v_high     (r/w): 41 (32 + 9)
	-- dac_v_off      (r/w): 46 (32 + 14)
	-- dac_control    (r/w): 48 (32 + 16)
	dac_v_low <= register_file_writable(8) when rfid_to_dac_enabled = '0' else
					 rfid_ask_modulated;
					 
	dac_v_high <= register_file_writable(9);
	dac_v_off <= register_file_writable(14);
	

	-- several control functions
	-- 0: enable DAC if = 1, else disable
	-- 1: in test mode if = 1
	-- 2: using output of RFID fault injector
	dac_control <= register_file_writable(16);
	rfid_to_dac_enabled <= dac_control(2);
		
	-- update output on voltage low/high change
	dac_v_update <= register_file_w(40) or register_file_w(41) 
		or register_file_w(46) or register_file_w(48) or rfid_to_dac_enabled;
	
	DAC_CONTROLLER_inst : dac_controller
	port map(
		clk => clk_100,
		ce => dac_ce,
		reset => reset,
		test_mode => dac_test_mode,
		voltage_low => dac_v_low,
		voltage_high => dac_v_high,
		voltage_off => dac_v_off,
		voltage_select => dac_v_select,
		voltage_update => dac_v_update,
		off => dac_off,
		voltage_out => dac_v_out,
		sleep => open,
		clk_dac => dac_clk
	);
	
	dac_ce <= '1';
	dac_off <= not (pic_v_dd_en or dac_control(0));
	dac_test_mode <= dac_control(1);
	dac_v_select <= fi_inject_fault when rfid_to_dac_enabled = '0' else
						 '0';
	
	
	-- Timing controller
	-- fi_control                   (r/w): 42 (32 + 10)
	-- fi_d_in                      (r/w): 43 (32 + 11)
	-- fi_addr_l                    (r/w): 44 (32 + 12)
	-- fi_addr_h                    (r/w): 45 (32 + 13)
	-- fi_trigger_control           (r/w): 47 (32 + 15)
	-- fi_universal_trigger_control (r/w): 81 (32 + 49)
	-- fi_status                    (r): 7
	-- fi_d_out                     (r): 8
	fi_control <= register_file_writable(10);
	fi_trigger_control <= register_file_writable(15);
	fi_universal_trigger_control <= register_file_writable(49);
	fi_d_in <= register_file_writable(11);
	fi_addr(7 downto 0) <= register_file_writable(12);
	fi_addr(9 downto 8) <= register_file_writable_13(1 downto 0);
	
	register_file_readonly(7) <= fi_status;
	register_file_readonly(8) <= fi_d_out;
	
	TIMING_CONTROLLER_WAVEFORM_inst : timing_controller_waveform generic map(
		TIME_REGISTER_WIDTH => 32
	)
	port map(
		clk => clk_100,
		ce => fi_ce,
		reset => reset,
		arm => fi_arm,
		disarm => fi_disarm,
		trigger => fi_trigger,
		armed => fi_armed,
		ready => fi_ready,
		inject_fault => fi_inject_fault,
		addr => fi_addr,
		w_en => fi_w_en,
		d_in => fi_d_in,
		d_out => fi_d_out
	);
	
	fi_ce <= '1';
	
	-- control register
	-- 0: w_en
	-- 1: arm
	-- 2: trigger force
	-- 3: disarm
	fi_w_en    <= fi_control(0);
	fi_arm     <= fi_control(1);
	fi_disarm     <= fi_control(3);
	-- trigger if enabled in resp. control register
	-- 0: DAC enable (for powerup trigger)
	-- 1: Universal internal trigger
	-- 2: External trigger
	-- 3: RESERVED: ADC trigger
	-- 4: Internal GPIO output 0
	-- 5: Internal GPIO output 1
	-- 6: RESERVED: GPIO input 0
	-- 7: Invert trigger edge direction
	fi_trigger <= (
			(
				(dac_control(0) and fi_trigger_control(0)) or
				(fi_universal_trigger and fi_trigger_control(1)) or
				(fi_trigger_ext and fi_trigger_control(2)) or
				-- 3: ADC trigger currently missing
				(gpio_outputs(0) and fi_trigger_control(4)) or
				(gpio_outputs(1) and fi_trigger_control(5)) 
			) xor fi_trigger_control(7)
		) or fi_control(2); -- this is the software trigger, always enabled
	
	-- Internal universal module trigger
	-- 0: RFID
	-- 1: UTX start
	-- 2: Utiming out
	-- 3: Utrig 1 trigger
	-- 4: Smartcard data sent trigger 
	-- 5: Smartcard data begin sending trigger
	fi_universal_trigger <= (rfid_trigger and fi_universal_trigger_control(0)) or 
	    (utx_start and fi_universal_trigger_control(1)) or
	    (utiming_out and fi_universal_trigger_control(2)) or
	    (utrig1_trigger and fi_universal_trigger_control(3)) or
		(sc_data_sent_trigger and fi_universal_trigger_control(4)) or 
		(sc_data_sending_trigger and fi_universal_trigger_control(5));
	
	-- status register
	-- 0: ready
	-- 1: armed
	fi_status(0) <= fi_ready;
	fi_status(1) <= fi_armed;
	fi_status(7 downto 2) <= (others => '0');
	
	-- Timing controller
	-- utiming_control         (r/w): 50 (32 + 18)
	-- utiming_d_in            (r/w): 51 (32 + 19)
	-- utiming_addr_l          (r/w): 52 (32 + 20)
	-- utiming_addr_h          (r/w): 53 (32 + 21)
	-- utiming_status          (r): 13
	utiming_control <= register_file_writable(18);
	utiming_d_in <= register_file_writable(19);
	utiming_addr(7 downto 0) <= register_file_writable(20);
	utiming_addr(9 downto 8) <= register_file_writable_21(1 downto 0);
	
	register_file_readonly(13) <= utiming_status;
	
	UTIMING_inst : timing_controller_waveform generic map(
		TIME_REGISTER_WIDTH => 32
	)
	port map(
		clk => clk_100,
		ce => '1',
		reset => reset,
		arm => utiming_arm,
		disarm => utiming_disarm,
		trigger => utiming_trigger,
		armed => utiming_armed,
		ready => utiming_ready,
		inject_fault => utiming_out,
		addr => utiming_addr,
		w_en => utiming_w_en,
		d_in => utiming_d_in,
		d_out => open
	);
	
	
	-- control register
	-- 0: w_en
	-- 1: arm
	-- 2: trigger force
	-- 3: disarm
	utiming_w_en    <= utiming_control(0);
	utiming_arm     <= utiming_control(1);
	utiming_trigger  <= rfid_field_reset_done or utiming_control(2); 
	utiming_disarm <= utiming_control(3);
	
	-- status register
	-- 0: ready
	-- 1: armed
	utiming_status(0) <= utiming_ready;
	utiming_status(1) <= utiming_armed;
	utiming_status(7 downto 2) <= (others => '0');
	
	-- ADC controller
	-- adc_control (r/w): 64 (32 + 32)
	-- ADC_CONTROLLER_inst : adc_controller 
	-- port map(
	-- 	-- clk => clk_100, 
	-- 	clk => clk_50, 
	-- 	reset => reset,
	-- 	ce => adc_ce,
	-- 	adc_in => adc_v_in,
	-- 	adc_control => adc_control,
	-- 	adc_encode => adc_encode,
	-- 	adc_encode_fb => adc_encode_fb,
	-- 	adc_value => adc_value
	-- );
	-- adc_control <= register_file_writable(32);
	-- adc_ce <= '1';
	-- 
	-- -- Threshold trigger generator
	-- -- thresh_status  (r): 12
	-- -- thresh_control (r/w): 49 (32 + 17)
	-- -- thresh_value (r/w): 61 (32 + 29)
	-- register_file_readonly(12) <= thresh_status;
	-- 
	-- -- Status register bits
	-- -- Bit 0: Armed
	-- thresh_status(0) <= thresh_armed;
	-- thresh_status(7 downto 1) <= (others => '0');
	-- 
	-- -- Control register
	-- -- Bit 0: Arm
	-- -- Bit 1: Enable coarse trigger
	-- -- Bit 2: Software trigger
	-- -- Bit 3: Invert coarse trigger, i.e., trigger on falling edge if set
	-- thresh_control <= register_file_writable(17);
	-- thresh_value <= register_file_writable(29);
	-- thresh_force_trigger <= thresh_control(2);
	-- 
	-- TRIGGER_GENERATOR_inst: trigger_generator 
	-- port map(
	-- 	clk => clk_50,
	-- 	reset => reset,
	-- 	arm => thresh_control(0),
	-- 	armed => thresh_armed,
	-- 	coarse_trigger_en => thresh_control(1),
	-- 	coarse_trigger => thresh_coarse_trigger,
	-- 	force_trigger => thresh_force_trigger,
	-- 	detector_in => detector_out,
	-- 	threshold => thresh_value,
	-- 	threshold_w => register_file_w(61),
	-- 	trigger => thresh_trigger
	-- );
    -- 
	-- thresh_coarse_trigger <= fi_trigger xor thresh_control(3); 
	-- 
	-- -- Pattern detector
	-- -- pattern_in      (r/w): 60 (32 + 28)
	-- -- pattern_debug   (r/w): 62 (32 + 30)
	-- -- pattern_sample_count   (r/w): 63 (32 + 31)
	-- detector_pattern_in <= register_file_writable(28);
	-- 
	-- -- uncomment the following for debug mode (i.e. input from PC controlled 
	-- -- register)
	-- --detector_adc_in <= register_file_writable(30);
	-- --detector_adc_we <= register_file_w(62);
	-- 
	-- -- uncomment the following for normal mode (i.e. input from ADC)
	-- detector_adc_in <= adc_value;
	-- detector_adc_we <= detector_downsampling_ce_out;
	-- detector_pattern_sample_count <= unsigned(register_file_writable_31(7 downto 0));
	-- 
	-- detector_ce <= '1';
	-- 
	-- PATTERN_DETECTOR_inst : pattern_detector
	-- port map( 
	-- 	clk => clk_50,
	-- 	reset => reset,
	-- 	ce => detector_ce,
	-- 	pattern_in => u8(detector_pattern_in), 
	-- 	pattern_we => register_file_w(60),
	-- 	pattern_sample_count => detector_pattern_sample_count,
	-- 	adc_in => u8(detector_adc_in),
	-- 	adc_we => detector_adc_we,
	-- 	d_out => detector_out
	-- );
    -- 
	-- -- Downsampling
	-- -- scope_downsampling_factor      (r/w): 65 (32 + 33)
	-- -- detector_downsampling_factor   (r/w): 66 (32 + 34)
	-- scope_downsampling_factor <= register_file_writable(33);
	-- detector_downsampling_factor <= register_file_writable(34);
	-- 
	-- SCOPE_DOWNSAMPLING_CONTROLLER_inst : downsampling_controller
	-- port map 
	-- ( 
	-- 	clk => clk_50,
	-- 	ce => '1',
	-- 	reset => reset,
	-- 	downsampling => scope_downsampling_factor,
	-- 	ce_out => scope_downsampling_ce_out
	-- );
	-- 
	-- DETECTOR_DOWNSAMPLING_CONTROLLER_inst : downsampling_controller
	-- port map 
	-- ( 
	-- 	clk => clk_50,
	-- 	ce => '1',
	-- 	reset => reset,
	-- 	downsampling => detector_downsampling_factor,
	-- 	ce_out => detector_downsampling_ce_out
	-- );
	
	-- Clock generation
	CLKGEN_inst : clock_generator
	port map
	(
		-- Clock in ports
		CLK_IN1 => clk_in,
		-- Clock out ports
		CLK_OUT1 => clk_100,
		CLK_OUT2 => clk_50,
		CLK_OUT3 => clk_48,
		-- Status and control signals
		RESET  => reset_in,
		LOCKED => clk_locked
	);
	
	-- clocking & reset
	clk <= clk_50;
	reset <= not clk_locked;
	
	-- LEDs
	led(0) <= clk_locked;
	led(1) <= thresh_armed;
	led(2) <= fi_trigger;
	led(3) <= utiming_out;
	
	-- Transistors
	transistors(0) <= fi_inject_fault; -- T1
	transistors(1) <= '0'; -- T2
	transistors(2) <= not fi_inject_fault; -- T3
	transistors(3) <= '0'; -- T4
	
	
	-- GPIO
	-- gpio_select_in     (r/w): 82 (32 + 50)
	-- gpio_control       (r/w): 83 (32 + 51)
	-- gpio_outputs       (r/w): 84 (32 + 52)
	GPIO_SW : gpio_switch
	port map
	( 
		clk => clk,
		reset => reset,
		gpio => gpio1,
		gpio_enable => gpio_control(0),
		fpga_i => gpio_fpga_i,
		fpga_o => gpio_fpga_o,
		fpga_io_output => gpio_fpga_io_output,
		select_clear => gpio_control(1),
		select_in => gpio_select_in,
		select_in_w_en => gpio_select_in_w_en
	);
	
	-- Register mapping
	-- 0: Enable (if = 1), else tristate GPIO
	-- 1: Clear assignment
	gpio_control <= register_file_writable(51);
	
	-- Select register
	gpio_select_in <= register_file_writable(50);
	gpio_select_in_w_en <= register_file_w(82);

	-- Output register
	gpio_outputs <= register_file_writable(52);
	
	-- UTX pin - 'Z' when no valid data (useful for tristate busses)
	gpio_fpga_o(0) <= utx_data_out when utx_data_out_valid = '1' else 'Z';
	gpio_fpga_io_output(0) <= utx_data_out_valid;
	
	-- Internal, combined trigger (out)
	gpio_fpga_o(1) <= fi_trigger;
	gpio_fpga_io_output(1) <= '1';
	
	-- Fault injection point (out)
	gpio_fpga_o(2) <= fi_inject_fault;
	gpio_fpga_io_output(2) <= '1';
	
	-- URX sampling points
	gpio_fpga_o(3) <= utx_start;
	gpio_fpga_io_output(3) <= '1';
	
	-- Combined URX/TX pin - TX can be disconnected by bit 6 in UTX control for separate RX/TX pins
	urx_data_in <= gpio_fpga_i(4);
	gpio_fpga_o(4) <= utx_data_out when utx_control(6) = '0' else 'Z';
	gpio_fpga_io_output(4) <= not utx_control(6);
	
	-- SC trigger
	gpio_fpga_o(5) <= ddr_dma_start; 
	gpio_fpga_io_output(5) <= '1';
	
	gpio_fpga_o(6) <= sc_data_sending_trigger;
	gpio_fpga_io_output(6) <= '1';
	
	gpio_fpga_o(7) <= sc_data_sent_trigger;
	gpio_fpga_io_output(7) <= '1';
	
	-- FI trigger pins
	gpio_fpga_o(8) <= thresh_trigger;
	gpio_fpga_io_output(8) <= '1';
	
	-- PIC programmer
	gpio_fpga_o(9) <= pic_v_dd_en;
	gpio_fpga_io_output(9) <= '1';
	
	gpio_fpga_o(10) <= not pic_v_pp_en;
	gpio_fpga_io_output(10) <= '1';
	
	--
	-- TODO: THIS NEEDS TO BE CHECKED AND MIGHT BE WRONG
	--
	gpio_fpga_o(11) <= pic_ispdat;
	gpio_fpga_io_output(11) <= pic_ispdat_output;
	
	-- Utiming
	gpio_fpga_o(12) <= utiming_out;
	gpio_fpga_io_output(12) <= '1';
	
	-- Utrig 1 & 2
	gpio_fpga_o(13) <= utrig1_trigger;
	gpio_fpga_io_output(13) <= '1';
	
	gpio_fpga_o(14) <= utrig2_trigger;
	gpio_fpga_io_output(14) <= '1';
	
	-- GPIO outputs
	gpio_fpga_o(15) <= gpio_outputs(0);
	gpio_fpga_io_output(15) <= '1';
	
	gpio_fpga_o(16) <= gpio_outputs(1);
	gpio_fpga_io_output(16) <= '1';
	
	gpio_fpga_o(17) <= gpio_outputs(2);
	gpio_fpga_io_output(17) <= '1';
	
	gpio_fpga_o(18) <= gpio_outputs(3);
	gpio_fpga_io_output(18) <= '1';
	
	-- Additional TX/RX modes
	gpio_fpga_o(19) <= utx_data_out;
	gpio_fpga_io_output(19) <= '1';
	
	-- FI external trigger in
	fi_trigger_ext <= gpio_fpga_i(20);
	gpio_fpga_io_output(20) <= '0';
	
	-- TODO: GPIO inputs
	
	-- Standard values
	gpio_fpga_o(29) <= '0';
	gpio_fpga_io_output(29) <= '1';
	
	gpio_fpga_o(30) <= '1';
	gpio_fpga_io_output(30) <= '1';
	
	gpio_fpga_o(31) <= 'Z';
	gpio_fpga_io_output(31) <= '1';
	
	-- Reset of pins
	gpio_fpga_o(28 downto 21) <= (others => '0');
	gpio_fpga_io_output(28 downto 21) <= (others => '0');
	
	
	-- For now: tristate clk pins
	clk_r9 <= 'Z';
	clk_2 <= 'Z';
	clk_28 <= utx_clk_tx;
	
	-- The UTX clock is always routed to CLK28
	-- ODDR2_inst : ODDR2
    -- generic map(
    --   DDR_ALIGNMENT => "NONE", 
    --   INIT => '0', 
    --   SRTYPE => "SYNC"
	-- ) 
    -- port map (
    --   Q => clk_28, -- 1-bit output data
    --   C0 => utx_clk_tx, -- 1-bit clock input
    --   C1 => not utx_clk_tx, -- 1-bit clock input
    --   CE => '1',  -- 1-bit clock enable input
    --   D0 => '1',   -- 1-bit data input (associated with C0)
    --   D1 => '0',   -- 1-bit data input (associated with C1)
    --   R => reset,    -- 1-bit reset input
    --   S => '0'     -- 1-bit set input
    -- );
	
	-- Processes
	
	
end behavioral;


