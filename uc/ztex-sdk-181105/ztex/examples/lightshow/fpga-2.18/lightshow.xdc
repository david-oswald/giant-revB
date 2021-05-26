# CLKOUT/FXCLK 
create_clock -name fxclk -period 10 [get_ports fxclk]
set_property PACKAGE_PIN V13 [get_ports fxclk]
set_property IOSTANDARD LVCMOS33 [get_ports fxclk]

# led0
set_property PACKAGE_PIN V10 [get_ports led0]
set_property IOSTANDARD LVCMOS33 [get_ports led0]
set_property DRIVE 12 [get_ports led0]

# led1
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

# sw
set_property PACKAGE_PIN L18 [get_ports sw[0]]			;# A11 / L18~IO_L16N_T2_A27_15
set_property PACKAGE_PIN L19 [get_ports sw[1]]			;# B11 / L19~IO_L14P_T2_SRCC_15
set_property PACKAGE_PIN M21 [get_ports sw[2]]			;# A12 / M21~IO_L10P_T1_AD11P_15
set_property PACKAGE_PIN L20 [get_ports sw[3]]			;# B12 / L20~IO_L14N_T2_SRCC_15
set_property IOSTANDARD LVCMOS33 [get_ports {sw[*]}]
set_property PULLUP true [get_ports {sw[*]}]

# led2
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

# bitstream settings
set_property BITSTREAM.CONFIG.CONFIGRATE 66 [current_design]  
set_property BITSTREAM.CONFIG.SPI_32BIT_ADDR No [current_design]
set_property BITSTREAM.CONFIG.SPI_BUSWIDTH 2 [current_design]
set_property BITSTREAM.GENERAL.COMPRESS true [current_design] 
