/*%
   memfifo -- Connects the bi-directional high speed interface of default firmware to a FIFO built of on-board SDRAM or on-chip BRAM
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
   Top level module: glues everything together.    
*/  

`define IDX(x) (((x)+1)*(8)-1):((x)*(8))

module memfifo (
	input fxclk_in,
	input ifclk_in,
	input reset,
	// debug
	output [9:0] led1,
	output [13:0] led2,
	input SW8,
	input SW10,
	// ddr
	inout[15:0] ddr_dram_dq,
        inout ddr_rzq,
        inout ddr_zio,
        inout ddr_dram_udqs,
        inout ddr_dram_dqs,
	output[12:0] ddr_dram_a,
	output[1:0] ddr_dram_ba,
        output ddr_dram_cke,
        output ddr_dram_ras_n,
        output ddr_dram_cas_n,
        output ddr_dram_we_n,
        output ddr_dram_dm,
        output ddr_dram_udm,
        output ddr_dram_ck,
        output ddr_dram_ck_n,
	// ez-usb
        inout [15:0] fd,
	output SLWR, SLRD,
	output SLOE, FIFOADDR0, FIFOADDR1, PKTEND,
	input FLAGA, FLAGB,
	// GPIO
	input gpio_clk, gpio_dir,
	inout gpio_dat
    );

    localparam APP_ADDR_WIDTH = 18;

    wire reset_mem, reset_usb;
    wire ifclk;
    reg reset_ifclk;
    wire [APP_ADDR_WIDTH-1:0] mem_free;
    wire [9:0] status;
    wire [3:0] if_status;
    wire [3:0] mode;
    
    // input fifo
    reg [31:0] DI;
    wire FULL, WRERR, USB_DO_valid;
    reg WREN, wrerr_buf;
    wire [15:0] USB_DO;
    reg [31:0] in_data;
    reg [3:0] wr_cnt;
    reg [6:0] test_cnt;
    reg [13:0] test_cs;
    reg in_valid;
    wire test_sync;
    reg [1:0] clk_div;
    reg DI_run;

    // output fifo
    wire [31:0] DO;
    wire EMPTY, RDERR, USB_DI_ready;
    reg RDEN, rderr_buf, USB_DI_valid;
    reg [31:0] rd_buf;
    reg rd_cnt;

    dram_fifo dram_fifo_inst (
	.fxclk_in(fxclk_in),					// 48 MHz input clock pin
        .reset(reset || reset_usb),
        .reset_out(reset_mem),					// reset output
	// Memory interface ports
	.ddr_dram_dq(ddr_dram_dq),
        .ddr_rzq(ddr_rzq),
        .ddr_zio(ddr_zio),
        .ddr_dram_udqs(ddr_dram_udqs),
        .ddr_dram_dqs(ddr_dram_dqs),
	.ddr_dram_a(ddr_dram_a),
	.ddr_dram_ba(ddr_dram_ba),
        .ddr_dram_cke(ddr_dram_cke),
        .ddr_dram_ras_n(ddr_dram_ras_n),
        .ddr_dram_cas_n(ddr_dram_cas_n),
        .ddr_dram_we_n(ddr_dram_we_n),
        .ddr_dram_dm(ddr_dram_dm),
        .ddr_dram_udm(ddr_dram_udm),
        .ddr_dram_ck(ddr_dram_ck),
        .ddr_dram_ck_n(ddr_dram_ck_n),
	// input fifo interface
	.DI(DI),			// must be hold while FULL is asserted
        .FULL(FULL),                    // 1-bit output: Full flag
        .WRERR(WRERR),                  // 1-bit output: Write error
        .WREN(WREN),                    // 1-bit input: Write enable
        .WRCLK(ifclk),                  // 1-bit input: Rising edge write clock.
	// output fifo interface
	.DO(DO),
	.EMPTY(EMPTY),                  // 1-bit output: Empty flag
        .RDERR(RDERR),                  // 1-bit output: Read error
        .RDCLK(ifclk),                  // 1-bit input: Read clock
        .RDEN(RDEN),                    // 1-bit input: Read enable
	// free memory
	.mem_free_out(mem_free),
	// for debugging
	.status(status)
    );

    ezusb_gpio gpio_inst (
	.clk(ifclk),			// system clock, minimum frequency is 24 MHz
	// hardware pins
	.gpio_clk(gpio_clk),		// data clock; data sent on both edges
	.gpio_dir(gpio_dir),		// 1: output, 0->1 transition latches input data and starts writing
	.gpio_dat(gpio_dat),
	// interface
	.in(mode),
	.out(4'd0)			// wired or: GPIO's not used for output should be 0
    );

    ezusb_io #(
	.OUTEP(2),		        // EP for FPGA -> EZ-USB transfers
	.INEP(6) 		        // EP for EZ-USB -> FPGA transfers 
    ) ezusb_io_inst (
        .ifclk(ifclk),
        .reset(reset),   		// asynchronous reset input
        .reset_out(reset_usb),		// synchronous reset output
        // pins
        .ifclk_in(ifclk_in),
        .fd(fd),
	.SLWR(SLWR),
	.SLRD(SLRD),
	.SLOE(SLOE), 
	.PKTEND(PKTEND),
	.FIFOADDR({FIFOADDR1, FIFOADDR0}), 
	.EMPTY_FLAG(FLAGA),
	.FULL_FLAG(FLAGB),
	// signals for FPGA -> EZ-USB transfer
	.DI(rd_buf[15:0]),		// data written to EZ-USB
	.DI_valid(USB_DI_valid),	// 1 indicates data valid; DI and DI_valid must be hold if DI_ready is 0
	.DI_ready(USB_DI_ready),	// 1 if new data are accepted
	.DI_enable(1'b1),		// setting to 0 disables FPGA -> EZ-USB transfers
    	.pktend_arm(mode[2]),		// 0->1 transition enables the manual PKTEND mechanism:
    	                                // PKTEND is asserted as soon output becomes idle
    	                                // recommended procedure for accurate packet transfers:
    	                                //   * DI_validgoes low after last data of package
    	                                //   * monitor PKTEND and hold DI_valid until PKTEND is asserted (PKTEND = 0)
        .pktend_timeout(16'd366),	// automatic PKTEN assertation after pktend_timeout*65536 (approx. 0.5s) clocks of no
                                        // output data. Setting to 0 disables this feature.
	// signals for EZ-USB -> FPGA transfer
	.DO(USB_DO),			// data read from EZ-USB
	.DO_valid(USB_DO_valid),	// 1 indicated valid data
	.DO_ready((mode[1:0]==2'd0) && !reset_ifclk && !FULL),	// setting to 1 enables writing new data to DO in next clock; DO and DO_valid are hold if DO_ready is 0
        // debug output
	.status(if_status)	
    );

/*    BUFR ifclkin_buf (
	.I(ifclk_in),
	.O(ifclk) 
    ); */
//    assign ifclk = ifclk_in;

    // debug board LEDs    
    assign led1 = SW10 ? status : { EMPTY, FULL, wrerr_buf, rderr_buf, if_status, FLAGB, FLAGA };
    
    assign led2[0] = mem_free[APP_ADDR_WIDTH-1:APP_ADDR_WIDTH-5] == 5'd31;
    assign led2[1] = mem_free[APP_ADDR_WIDTH-1:APP_ADDR_WIDTH-5] < 5'd30;
    assign led2[2] = mem_free[APP_ADDR_WIDTH-1:APP_ADDR_WIDTH-5] < 5'd27;
    assign led2[3] = mem_free[APP_ADDR_WIDTH-1:APP_ADDR_WIDTH-5] < 5'd25;
    assign led2[4] = mem_free[APP_ADDR_WIDTH-1:APP_ADDR_WIDTH-5] < 5'd22;
    assign led2[5] = mem_free[APP_ADDR_WIDTH-1:APP_ADDR_WIDTH-5] < 5'd20;
    assign led2[6] = mem_free[APP_ADDR_WIDTH-1:APP_ADDR_WIDTH-5] < 5'd17;
    assign led2[7] = mem_free[APP_ADDR_WIDTH-1:APP_ADDR_WIDTH-5] < 5'd15;
    assign led2[8] = mem_free[APP_ADDR_WIDTH-1:APP_ADDR_WIDTH-5] < 5'd12;
    assign led2[9] = mem_free[APP_ADDR_WIDTH-1:APP_ADDR_WIDTH-5] < 5'd10;
    assign led2[10] = mem_free[APP_ADDR_WIDTH-1:APP_ADDR_WIDTH-5] < 5'd7;
    assign led2[11] = mem_free[APP_ADDR_WIDTH-1:APP_ADDR_WIDTH-5] < 5'd5;
    assign led2[12] = mem_free[APP_ADDR_WIDTH-1:APP_ADDR_WIDTH-5] < 5'd2;
    assign led2[13] = mem_free == { (APP_ADDR_WIDTH){1'b0} };

    assign test_sync = wr_cnt[0] || (wr_cnt == 4'd14);

    always @ (posedge ifclk)
    begin
	reset_ifclk <= reset || reset_usb || reset_mem;
	
	if ( reset_ifclk ) 
	begin
	    rderr_buf <= 1'b0;
	    wrerr_buf <= 1'b0;
	end else
	begin
	    rderr_buf <= rderr_buf || RDERR;
	    wrerr_buf <= wrerr_buf || WRERR;
	end

	// FPGA -> EZ-USB FIFO
	DI_run <= !mode[2];
        if ( reset_ifclk )
        begin
	    rd_cnt <= 1'd0;
	    USB_DI_valid <= 1'd0;
	end else if ( USB_DI_ready )
	begin
	    USB_DI_valid <= !EMPTY && DI_run;
	    if ( !EMPTY && DI_run )
	    begin
	        if ( rd_cnt == 1'd0 )
	        begin
	    	    rd_buf <= DO;
		end else
	    	begin
	    	    rd_buf[15:0] <= rd_buf[31:16];
		end
		rd_cnt <= rd_cnt + 1'd1;
	    end
	end
	RDEN <= !reset_ifclk && USB_DI_ready && !EMPTY && (rd_cnt==1'd0) && DI_run;
	
	// data source
	if ( reset_ifclk ) 
	begin
	    in_data <= 31'd0;
	    in_valid <= 1'b0;
	    wr_cnt <= 4'd0;
	    test_cnt <= 7'd0;
	    test_cs <= 12'd47;
	    WREN <= 1'b0;
	    clk_div <= 2'd3;
	    DI <= 32'h05040302;
	end else if ( !FULL )
	begin
	    if ( in_valid ) DI <= in_data;
//	    DI <= DI + 32'h03030303;

    	    if ( mode[1:0] == 2'd0 )		// input from USB
    	    begin
    		if ( USB_DO_valid )
    		begin
		    in_data <= { USB_DO, in_data[31:16] };
		    in_valid <= wr_cnt[0];
	    	    wr_cnt <= wr_cnt + 4'd1;
	    	end else
	    	begin
		    in_valid <= 1'b0;
		end
    	    end else if ( clk_div == 2'd0 )	// test data generator
	    begin
	        if ( wr_cnt == 4'd15 )
	        begin
	    	    test_cs <= 12'd47;
		    in_data[30:24] <= test_cs[6:0] ^ test_cs[13:7];
		end else
		begin
		    test_cnt <= test_cnt + 7'd111;
		    test_cs <= test_cs + { test_sync, test_cnt };
		    in_data[30:24] <= test_cnt;
		end
		in_data[31] <= test_sync;
		in_data[23:0] <= in_data[31:8];
		in_valid <= wr_cnt[1:0] == 2'd3;
	    	wr_cnt <= wr_cnt + 4'd1;
	    end else 
	    begin
	        in_valid <= 1'b0;
	    end

	    if ( (mode[1:0]==2'd1) || ((mode[1:0]==2'd3) && SW8 ) ) 
	    begin
	        clk_div <= 2'd0;	// data rate: 48 MByte/s
	    end else 
	    begin
	        clk_div <= clk_div + 2'd1;	// data rate: 12 MByte/s
	    end
	end
	WREN <= !reset_ifclk && in_valid && !FULL;
//	WREN <= !reset_ifclk && !FULL;
    end
	    
endmodule

