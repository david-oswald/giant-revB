# fxclk_in
create_clock -period 38.75 -name fxclk_in [get_ports fxclk_in]
set_property PACKAGE_PIN V13 [get_ports fxclk_in]
set_property IOSTANDARD LVCMOS33 [get_ports fxclk_in]

# reset_in
set_property PACKAGE_PIN AB16 [get_ports reset_in]
set_property IOSTANDARD LVCMOS33 [get_ports reset_in]
set_property PULLUP true [get_ports reset_in]

# lsi_clk
set_property PACKAGE_PIN AB15 [get_ports lsi_clk]
set_property IOSTANDARD LVCMOS33 [get_ports lsi_clk]

# lsi_data
set_property PACKAGE_PIN AA16 [get_ports lsi_data]
set_property IOSTANDARD LVCMOS33 [get_ports lsi_data]
set_property DRIVE 4 [get_ports lsi_data]
set_property PULLUP true [get_ports lsi_data]

# lsi_stop
set_property PACKAGE_PIN AA15 [get_ports lsi_stop]
set_property IOSTANDARD LVCMOS33 [get_ports lsi_stop]

# bitstream settings
set_property BITSTREAM.CONFIG.CONFIGRATE 66 [current_design]  
set_property BITSTREAM.CONFIG.SPI_32BIT_ADDR No [current_design]
set_property BITSTREAM.CONFIG.SPI_BUSWIDTH 2 [current_design]
set_property BITSTREAM.GENERAL.COMPRESS true [current_design] 
