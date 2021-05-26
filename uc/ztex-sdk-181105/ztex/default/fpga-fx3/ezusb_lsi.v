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
   Implements the low speed interface of default firmwares.
   
   It allows easy-to-use low speed communication with a SRAM-like 
   interface (32 bit width, 8 bit address). For example it can be used
   to transfer things like settings and debug data.
*/  

module ezusb_lsi (
	// control signals
	input clk,			// at least 20MHz
	input reset_in,			// high-active asynchronous reset
	output reg reset = 1'b1,	// synchronous reset output
	// hardware pins
	input data_clk,			// all action happens if this signal changes (both edges)
	inout data,			// data in and out, LSB transmitted first
	input stop,                     // defines how data is processed at data_clk change:
					// * if low: data is read and shifted into a read buffer
	                                // * if high while data is low: data is written to the user application (on FPGA): last 8 bits are the address, preceding 32bit are the data
	                                // * if high while data is high: write-to-host sequence is started and remains active until stop goes low, last 8 bits are the address:
	                                //      1. continuously read out_data (user data, see below) and write first bit to host until data_clk changes
	                                //      2..n. shift next bit to host and wait until data_clk changes, repeated this until stop goes low
	// user interface
	output reg [7:0] in_addr,	// input address
	output reg [31:0] in_data,	// input data
	output reg in_strobe = 1'b0,	// 1 indicates new data received (1 for one cycle)
	output reg in_valid = 1'b0,	// 1 if date is valid
	output reg [7:0] out_addr,      // output address
	input [31:0] out_data,          // output data
	output reg out_strobe = 1'b0	// 1 indicates new data request (1 for one cycle)
    );
    
    reg [39:0] read_reg;
    reg [31:0] write_reg;
    reg [2:0] data_clk_buf;
    reg dir = 0;	// 0 : read
    reg do_write = 0;
    
    assign data = dir && (!write_reg[0]) ? 1'b0 : 1'bz;
    
//    wire data_clk_edge = ( (data_clk==data_clk_buf[0]) && (data_clk==data_clk_buf[1]) && (data_clk!=data_clk_buf[2]) );
    wire data_clk_edge = ( (data_clk_buf[0]!=data_clk_buf[1]) && (data_clk_buf[1]==data_clk_buf[2]) );

    
    always @ (posedge clk)
    begin
	reset <= reset_in;
	data_clk_buf <= { data_clk_buf[1:0], data_clk };
	in_strobe <= (!reset) && data_clk_edge && (!dir) && stop && (!data);
	out_strobe <= (!reset) && data_clk_edge && (!dir) && stop && data;
	dir <= (!reset) && stop && ((data_clk_edge && data) || dir);  
	if ( reset ) 
	begin 
	    in_valid <= 1'b0;
	    do_write <= 1'b0;
	end else if ( data_clk_edge )
	begin
	    if ( !dir )  // read from fx3
	    begin
	        if ( stop )
	        begin
	    	    if ( data ) // last 8 bit contain write address
		    begin
//		        dir <= 1'b1;
		        out_addr <= read_reg[39:32];
		        do_write <= 1'b1;
		    end else 
		    begin
		        in_valid <= 1'b1;
		        in_addr <= read_reg[39:32];
		        in_data <= read_reg[31:0];
		    end
		end else
		begin
		    read_reg <= { data, read_reg[39:1] };
		end
	    end else  // write to fx3
	    begin
	        write_reg[30:0] <= write_reg[31:1]; // shift next bit to host
	        do_write <= 1'b0;
//	        dir <= stop;
	    end
	end else if ( dir && do_write )  // write the first bit to host
	begin
	    write_reg <= out_data;
	end
    end
endmodule
