# fxclk_in, 26 MHz (period of 38.75ns makes Vivado happy)
create_clock -period 38.75 -name fxclk_in [get_ports fxclk_in]
set_property PACKAGE_PIN V13 [get_ports fxclk_in]
set_property IOSTANDARD LVCMOS33 [get_ports fxclk_in]

# IFCLK, 104 MHz
create_clock -name ifclk_in -period 9.615 [get_ports ifclk_in]
set_property PACKAGE_PIN W11 [get_ports ifclk_in]
set_property IOSTANDARD LVCMOS33 [get_ports ifclk_in]

# GPIO
set_property PACKAGE_PIN AA10 [get_ports gpio_n[0]]  		;# CTL7/PKTEND#/GPIO24
set_property PACKAGE_PIN AA14 [get_ports gpio_n[1]]  		;# CTL6/GPIO23
set_property PACKAGE_PIN V17 [get_ports gpio_n[2]]  		;# GPIO39/CSI_B
set_property PACKAGE_PIN AA19 [get_ports gpio_n[3]]  		;# GPIO38/RDWR_B
set_property IOSTANDARD LVCMOS33 [get_ports {gpio_n[*]}]
set_property DRIVE 4 [get_ports {gpio_n[*]}]
set_property PULLUP true [get_ports {gpio_n[*]}]

# reset
set_property PACKAGE_PIN AB16 [get_ports reset]  		;# CTL8/GPIO25
set_property IOSTANDARD LVCMOS33 [get_ports reset]

# fd
set_property PACKAGE_PIN P22 [get_ports {fd[0]}]  		;# DQ0
set_property PACKAGE_PIN R22 [get_ports {fd[1]}]  		;# DQ1
set_property PACKAGE_PIN P21 [get_ports {fd[2]}]  		;# DQ2
set_property PACKAGE_PIN R21 [get_ports {fd[3]}]  		;# DQ3
set_property PACKAGE_PIN T21 [get_ports {fd[4]}]  		;# DQ4
set_property PACKAGE_PIN U21 [get_ports {fd[5]}]  		;# DQ5
set_property PACKAGE_PIN P19 [get_ports {fd[6]}]  		;# DQ6
set_property PACKAGE_PIN R19 [get_ports {fd[7]}]  		;# DQ7
set_property PACKAGE_PIN T20 [get_ports {fd[8]}]  		;# DQ8
set_property PACKAGE_PIN W21 [get_ports {fd[9]}]  		;# DQ9
set_property PACKAGE_PIN W22 [get_ports {fd[10]}]  		;# DQ10
set_property PACKAGE_PIN AA20 [get_ports {fd[11]}]  		;# DQ11
set_property PACKAGE_PIN AA21 [get_ports {fd[12]}]  		;# DQ12
set_property PACKAGE_PIN Y22 [get_ports {fd[13]}]  		;# DQ13
set_property PACKAGE_PIN AB21 [get_ports {fd[14]}]  		;# DQ14
set_property PACKAGE_PIN AB22 [get_ports {fd[15]}]  		;# DQ15
set_property IOSTANDARD LVCMOS33 [get_ports {fd[*]}]
set_property DRIVE 4 [get_ports {fd[*]}]

# EMPTY
set_property PACKAGE_PIN AB11 [get_ports EMPTY_FLAG]  		;# CTL4/FLAGA/GPIO21
set_property IOSTANDARD LVCMOS33 [get_ports EMPTY_FLAG]

# FULL
set_property PACKAGE_PIN AB13 [get_ports FULL_FLAG]  		;# CTL5/FLAGB/GPIO22
set_property IOSTANDARD LVCMOS33 [get_ports FULL_FLAG]

# SLOE
set_property PACKAGE_PIN AA13 [get_ports SLOE]  		;# CTL2/SLOE#/GPIO19
set_property IOSTANDARD LVCMOS33 [get_ports SLOE]

# SLWR/WE
set_property PACKAGE_PIN Y12 [get_ports SLWR]  			;# CTL1/SLWR#/GPIO18
set_property IOSTANDARD LVCMOS33 [get_ports SLWR]
#set_property DRIVE 4 [get_ports SLWR]
#set_property SLEW FAST [get_ports SLWR]

# PKTEND
set_property PACKAGE_PIN W10 [get_ports PKTEND]  		;# CTL0/SLCS#/GPIO17
set_property IOSTANDARD LVCMOS33 [get_ports PKTEND]

# SLRD
set_property PACKAGE_PIN AB12 [get_ports SLRD]  		;# CTL3/SLRD#/GPIO20
set_property IOSTANDARD LVCMOS33 [get_ports SLRD]


# I/O delays
set_input_delay -clock ifclk_in -min 0 [get_ports {*_FLAG fd[*]}]
set_input_delay -clock ifclk_in -max 3.5 [get_ports {*_FLAG fd[*]}]
set_output_delay -clock ifclk_in -min 0 [get_ports {SLRD SLWR PKTEND}]
set_output_delay -clock ifclk_in -max 7 [get_ports {SLRD SLWR PKTEND}]

# LED's
set_property PACKAGE_PIN V10 [get_ports led]  			;# LED1:red
set_property IOSTANDARD LVCMOS33 [get_ports led]
set_property DRIVE 12 [get_ports led]

set_property PACKAGE_PIN N22 [get_ports {led1[0]}]		;# A6 / N22~IO_L15P_T2_DQS_15
set_property PACKAGE_PIN M20 [get_ports {led1[1]}]		;# B6 / M20~IO_L18N_T2_A23_15
set_property PACKAGE_PIN M22 [get_ports {led1[2]}]		;# A7 / M22~IO_L15N_T2_DQS_ADV_B_15
set_property PACKAGE_PIN M13 [get_ports {led1[3]}]		;# B7 / M13~IO_L20P_T3_A20_15
set_property PACKAGE_PIN L14 [get_ports {led1[4]}]		;# A8 / L14~IO_L22P_T3_A17_15
set_property PACKAGE_PIN L13 [get_ports {led1[5]}]		;# B8 / L13~IO_L20N_T3_A19_15
set_property PACKAGE_PIN L15 [get_ports {led1[6]}]		;# A9 / L15~IO_L22N_T3_A16_15
set_property PACKAGE_PIN L16 [get_ports {led1[7]}]		;# B9 / L16~IO_L23P_T3_FOE_B_15
set_property PACKAGE_PIN M18 [get_ports {led1[8]}]		;# A10 / M18~IO_L16P_T2_A28_15
set_property PACKAGE_PIN K16 [get_ports {led1[9]}]		;# B10 / K16~IO_L23N_T3_FWE_B_15
set_property IOSTANDARD LVCMOS33 [get_ports {led1[*]}]
set_property DRIVE 12 [get_ports {led1[*]}]

set_property PACKAGE_PIN AB3 [get_ports {led2[0]}]		;# C3 / AB3~IO_L8P_T1_34
set_property PACKAGE_PIN AB2 [get_ports {led2[1]}]		;# D3 / AB2~IO_L8N_T1_34
set_property PACKAGE_PIN  W7 [get_ports {led2[2]}]		;# C4 / W7~IO_L19N_T3_VREF_34
set_property PACKAGE_PIN AB1 [get_ports {led2[3]}]		;# D4 / AB1~IO_L7N_T1_34
set_property PACKAGE_PIN AA3 [get_ports {led2[4]}]		;# C5 / AA3~IO_L9N_T1_DQS_34
set_property PACKAGE_PIN AA1 [get_ports {led2[5]}]		;# D5 / AA1~IO_L7P_T1_34
set_property PACKAGE_PIN  Y3 [get_ports {led2[6]}]		;# C6 / Y3~IO_L9P_T1_DQS_34
set_property PACKAGE_PIN  Y1 [get_ports {led2[7]}]		;# D6 / Y1~IO_L5N_T0_34
set_property PACKAGE_PIN  Y2 [get_ports {led2[8]}]		;# C7 / Y2~IO_L4N_T0_34
set_property PACKAGE_PIN  W1 [get_ports {led2[9]}]		;# D7 / W1~IO_L5P_T0_34
set_property PACKAGE_PIN  W2 [get_ports {led2[10]}]		;# C8 / W2~IO_L4P_T0_34
set_property PACKAGE_PIN  V4 [get_ports {led2[11]}]		;# D8 / V4~IO_L12P_T1_MRCC_34
set_property PACKAGE_PIN  U3 [get_ports {led2[12]}]		;# C9 / U3~IO_L6P_T0_34
set_property PACKAGE_PIN  W4 [get_ports {led2[13]}]		;# D9 / W4~IO_L12N_T1_MRCC_34
set_property PACKAGE_PIN  V3 [get_ports {led2[14]}]		;# C10 / V3~IO_L6N_T0_VREF_34
set_property PACKAGE_PIN  U1 [get_ports {led2[15]}]		;# D10 / U1~IO_L1N_T0_34
set_property PACKAGE_PIN  R2 [get_ports {led2[16]}]		;# C11 / R2~IO_L3N_T0_DQS_34
set_property PACKAGE_PIN  T1 [get_ports {led2[17]}]		;# D11 / T1~IO_L1P_T0_34
set_property PACKAGE_PIN  R3 [get_ports {led2[18]}]		;# C12 / R3~IO_L3P_T0_DQS_34
set_property PACKAGE_PIN  R1 [get_ports {led2[19]}]		;# D12 / R1~IO_L20P_T3_35
set_property IOSTANDARD LVCMOS33 [get_ports {led2[*]}]
set_property DRIVE 12 [get_ports {led2[*]}]

# switches
#set_property PACKAGE_PIN L18 [get_ports SW7]			;# A11 / L18~IO_L16N_T2_A27_15
#set_property PACKAGE_PIN L19 [get_ports SW8]			;# B11 / L19~IO_L14P_T2_SRCC_15
#set_property PACKAGE_PIN M21 [get_ports SW9]			;# A12 / M21~IO_L10P_T1_AD11P_15
set_property PACKAGE_PIN L20 [get_ports SW10]			;# B12 / L20~IO_L14N_T2_SRCC_15
set_property IOSTANDARD LVCMOS33 [get_ports {SW*}]
set_property PULLUP true [get_ports {SW*}]

# location constraints
#set_property LOC PLLE2_ADV_X1Y1 [get_cells dram_fifo_inst/dram_fifo_pll_inst]

# TIG's
set_false_path -from [get_clocks *ifclk_out] -to [get_clocks *clk200_in]
set_false_path -from [get_clocks *ifclk_out] -to [get_clocks ]
set_false_path -from [get_clocks *clk_pll_i] -to [get_clocks *ifclk_out]

# bitstream settings
set_property BITSTREAM.CONFIG.CONFIGRATE 66 [current_design]  
set_property BITSTREAM.CONFIG.SPI_32BIT_ADDR No [current_design]
set_property BITSTREAM.CONFIG.SPI_BUSWIDTH 2 [current_design]
set_property BITSTREAM.GENERAL.COMPRESS true [current_design] 



