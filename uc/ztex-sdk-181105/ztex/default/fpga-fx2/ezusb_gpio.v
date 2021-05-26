/*%
   Common communication interface of default firmwares
   Copyright (C) 2009-2017 ZTEX GmbH.
   http://www.ztex.de

   Copyright and related rights are licensed under the Solderpad Hardware
   License, Version 0.51 (the "License"); you may not use this file except
   in compliance with the License. You may obtain a copy of the License at
   
       http://solderpad.org/licenses/SHL-0.51.
       
   Unless required by applicable law or agreed to in writing, software, hardware
   and materials distributed under this License is distributed on an "AS IS"
   BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
   implied. See the License for the specific language governing permissions
   and limitations under the License.
%*/
/* 
   Implements the 4 bi-directional GPIO's of the default interface.

   Outputs on both ends are or-ed.

   Remember (because it's not implemented here) default interface always
   contains a reset signal.
*/  


// all directions are seen from FPGA
module ezusb_gpio (
	// control signals
	input clk,			// system clock, minimum frequency is 24 MHz
	// hardware pins
	input gpio_clk,			// data clock; data sent on both edges
	input gpio_dir,			// 1: output, 0->1 transition latches input data and starts writing
	inout gpio_dat,
	// interface
	output reg [3:0] in,	
	input [3:0] out			// wired or: GPIO's not used for output should be 0
    );

    reg [2:0] gpio_clk_buf, gpio_dir_buf;
    reg [3:0] in_buf, out_reg, in_reg;
    reg [7:0] in_tmp;
    reg do_out;

    wire clk_edge = ( (gpio_clk_buf[0]!=gpio_clk_buf[1]) && (gpio_clk_buf[1]==gpio_clk_buf[2]) );
    wire dir_edge = ( (gpio_dir_buf[0]!=gpio_dir_buf[1]) && (gpio_dir_buf[1]==gpio_dir_buf[2]) );

    assign gpio_dat = gpio_dir ? out_reg[3] : 1'bz;

    always @ (posedge clk)
    begin
	gpio_clk_buf <= { gpio_clk_buf[1:0], gpio_clk };
	gpio_dir_buf <= { gpio_dir_buf[1:0], gpio_dir };
	
	do_out <= (do_out && gpio_dir_buf[0] && !clk_edge ) || (dir_edge && gpio_dir_buf[0]);
	
	if ( dir_edge && gpio_dir_buf[0] ) in_buf <= in_reg;
	if ( do_out ) out_reg <= out | in_reg;
	
	if ( clk_edge ) 
	begin
	    if ( gpio_dir_buf[0] ) out_reg <= {out_reg[2:0], 1'b0};
	    else in_reg <= { gpio_dat, in_reg[3:1] };
	end;
	
	in <= in_buf | out;
    end
endmodule
