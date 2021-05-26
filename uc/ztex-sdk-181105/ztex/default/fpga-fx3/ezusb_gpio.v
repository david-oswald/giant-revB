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

   On FX3 outputs on both ends are or-wired.

   Remember (because it's not implemented here) default interface always
   contains a reset signal.
*/  


module ezusb_gpio (
	// control signals
	input clk,			// system clock
	// hardware pins
	inout [3:0] gpio_n,		// wired or: low-active open-drain output
	// interface
	output reg [3:0] in,	
	input [3:0] out			// wired or: GPIO's not used for output should be 0
    );

    assign gpio_n[0] = !out[0] ? 1'bz : 1'b0;
    assign gpio_n[1] = !out[1] ? 1'bz : 1'b0;
    assign gpio_n[2] = !out[2] ? 1'bz : 1'b0;
    assign gpio_n[3] = !out[3] ? 1'bz : 1'b0;

    always @ (posedge clk)
    begin
	in <= ~gpio_n;
    end
endmodule
