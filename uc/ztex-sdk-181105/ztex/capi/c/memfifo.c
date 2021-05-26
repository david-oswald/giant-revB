/*%
   ZTEX Core API for C with examples
   Copyright (C) 2009-2017 ZTEX GmbH.
   http://www.ztex.de
   
   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this file,
   You can obtain one at http://mozilla.org/MPL/2.0/.

   Alternatively, the contents of this file may be used under the terms
   of the GNU General Public License Version 3, as described below:

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see http://www.gnu.org/licenses/.
%*/

/** @file
memfifo example for C.
This example demonstrates the usage of the C API and the high speed interface of the default firmware.

It performs certain tests (device and data integrity, read-write tests, performance) using the memory FIFO as described on
<a href="http://wiki.ztex.de/doku.php?id=en:ztex_boards:ztex_fpga_boards:memfifo:memfifo">memfifo example</a>.

The correct bitstream is detected and found automatically if the binary is executed from 
the directory tree of the SDK. Otherwise it can be specified with parameter '-s'.

Full list of options can be obtained with '-h'
@include memfifo.c
@cond memfifo
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <libusb-1.0/libusb.h>

#include "ztex.h"

#define BULK_BUF_SIZE 4*1024*1024
#define RW_SIZE 65000			// must be smaller then FIFO size, 64K on USB-FPGA Module 2.01

static char* prog_name = NULL;		// name of the program

static int paramerr(const char* format,...)
{
    fprintf(stderr, "Usage: %s options\n",prog_name);
    fprintf(stderr, "  -h                           Display this usage information\n"
		    "  -fu <vendor ID>:<product ID> Select device by USB IDs, default: 0x221A:0x100, <0:ignore ID\n"
		    "  -fd <bus>:<device>           Select device by bus number and device address\n"
		    "  -fs <string>                 Select device by serial number string\n"
		    "  -fp <string>                 Select device by product string\n"
		    "  -s <path>                    Additional search path for bitstream, default '.."DIRSEP".."DIRSEP"examples"DIRSEP"memfifo'\n"
		    "  -r                           Reset device (default: reset configuration only)\n"
		    "  -i                           Print device info\n"
		    "  -p                           Print matching USB devices\n"
		    "  -pa                          Print all USB devices\n"
	    );
    if ( format ) {
	va_list args;
	va_start(args,format);
    	vfprintf(stderr, format, args);
    	va_end(args);
	return 1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    int id_vendor = 0x221A;  	// ZTEX vendor ID
    int id_product = 0x100; 	// default product ID for ZTEX firmware
    int status = 0;
    libusb_device **devs = NULL;
    int print_all=0, print=0, print_info=0, reset_dev=0;
    int busnum = -1, devnum = -1;
    char *sn_string = NULL, *product_string = NULL;
    libusb_device_handle *handle = NULL;
    ztex_device_info info;
    char *bitstream_fn = NULL, *bitstream_path = NULL;
    char sbuf[8192];
    unsigned char *mbuf = NULL;
    int transferred, size;
    struct timeval tv1,tv2;

    // process parameters
    prog_name = argv[0];
    for (int i=1; i<argc; i++) {
	if ( !strcmp(argv[i],"-h") ) return paramerr(NULL);
	else if ( !strcmp(argv[i],"-p") ) print=1;
	else if ( !strcmp(argv[i],"-pa") ) print_all=1;
	else if ( !strcmp(argv[i],"-i") ) print_info=1;
	else if ( !strcmp(argv[i],"-r") ) reset_dev=1;
	else if ( !strcmp(argv[i],"-fu") ) {
	    i++;
	    if (i>=argc || sscanf(argv[i],"%i:%i", &id_vendor, &id_product)!=2 ) return paramerr("Error: <vendor ID>:<product ID> expected after -fu\n");
	}
	else if ( !strcmp(argv[i],"-fd") ) {
	    i++;
	    if (i>=argc || sscanf(argv[i],"%i:%i", &busnum, &devnum)!=2 ) return paramerr("Error: <bus>:<device> expected after -fd\n");
	}
	else if ( !strcmp(argv[i],"-fs") ) {
	    i++;
	    if (i>=argc ) return paramerr("Error: <string> expected after -fs\n");
	    sn_string = argv[i];
	}
	else if ( !strcmp(argv[i],"-fp") ) {
	    i++;
	    if (i>=argc ) return paramerr("Error: <string> expected after -fp\n");
	    product_string = argv[i];
	}
	else if ( !strcmp(argv[i],"-s") ) {
	    i++;
	    if (i>=argc ) return paramerr("Error: <path> expected after -s\n");
	    bitstream_path = argv[i];
	}
	else return paramerr("Error: Invalid parameter %s\n", argv[i]);
    }
    
    
    // INIT libusb
    status = libusb_init(NULL);
    if (status < 0) {
    	fprintf(stderr,"Error: Unable to init libusb: %s\n", libusb_error_name(status));
	return 1;
    }

    // find all USB devices
    status = libusb_get_device_list(NULL, &devs);
    if (status < 0) {
	fprintf(stderr,"Error: Unable to get device list: %s\n", libusb_error_name(status));
	goto err;
    }
    
    // print bus info or find device
    int dev_idx = ztex_scan_bus(sbuf, sizeof(sbuf), devs, print_all ? -1 : print ? 1 : 0, id_vendor, id_product, busnum, devnum, sn_string, product_string);
    printf(sbuf);
    fflush(stdout);
    if ( print || print_all ) {
	status = 0;
	goto noerr;
    } else if ( dev_idx<0 ) {
	if (dev_idx==-1) fprintf(stderr,"Error: No device found\n");
	goto err;
    }

    // open device
    status = libusb_open(devs[dev_idx], &handle);
    if (status < 0) {
	fprintf(stderr,"Error: Unable to open device: %s\n", libusb_error_name(status));
	goto err;
    }
    libusb_free_device_list(devs, 1);
    devs=NULL;
    
    // reset configuration or device
   if ( ! reset_dev ) {
	status = libusb_set_configuration(handle,-1);
	if (status < 0) {
	    fprintf(stderr,"Warning: Unable to unconfigure device: %s, trying to reset it\n", libusb_error_name(status));
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
	    fprintf(stderr,"Due to limitations of Windows neither this nor device reset works. This may cause further errors ...\n");
#endif    
	    reset_dev = 1;
	}
    }
    if ( reset_dev ) {
	status = libusb_reset_device(handle);
	if (status < 0) {
	    fprintf(stderr,"Error: Unable to reset device: %s\n", libusb_error_name(status));
	    goto err;
	}
    }
    status = libusb_set_configuration(handle,1);
    if (status < 0) fprintf(stderr,"Warning: Unable to set configuration 1: %s\n", libusb_error_name(status));
    fflush(stderr);

    // get and print device info
    status = ztex_get_device_info(handle, &info);
    if ( status < 0 ) {
	fprintf(stderr,"Error: Unable to get device info: %s\n", libusb_error_name(status));
	goto err;
    }
    if ( print_info ) {
	ztex_print_device_info( sbuf, sizeof(sbuf), &info );
	printf(sbuf);
	status = ztex_get_fpga_config(handle);
	if ( status < 0 ) {
	    fprintf(stderr,"Error: Unable to get FPGA configuration state: %s\n", libusb_error_name(status));
	    goto err;
	}
	printf("FPGA: %s\n", status==0 ? "unconfigured" : "configured");
	status = 0;
	goto noerr;
    }
    
    // find bitstream
    bitstream_fn = ztex_find_bitstream( &info, bitstream_path ? bitstream_path : ".."DIRSEP".."DIRSEP"examples"DIRSEP"memfifo" , "memfifo");
    if ( bitstream_fn )  {
	printf("Using bitstream '%s'\n", bitstream_fn);
	fflush(stdout);
    }
    else {
        fprintf(stderr,"Warning: Bitstream not found\n");
        goto nobitstream;
    }
    
    // read and upload bitstream
    FILE *fd = fopen(bitstream_fn, "rb");
    if ( fd == NULL ) {
        fprintf(stderr,"Warning: Error opening file '%s'\n", bitstream_fn);
        goto nobitstream;
    }
    status = ztex_upload_bitstream(sbuf,sizeof(sbuf),handle,&info,fd,-1);
    fclose(fd);
    fprintf(stderr,sbuf);

nobitstream:
    fflush(stderr);
    // check config
    status = ztex_get_fpga_config(handle);
    if ( status < 0 ) {
        fprintf(stderr,"Error: Unable to get FPGA configuration state: %s\n", libusb_error_name(status));
        goto err;
    } else if ( status == 0 ) {
        fprintf(stderr,"Error: FPGA not configured\n");
        goto err;
    }

    // claim interface
    status = libusb_claim_interface(handle, 0); 
    if ( status < 0 ) {
        fprintf(stderr,"Error claiming interface 0: %s\n", libusb_error_name(status));
        goto err;
    }

    mbuf = malloc(BULK_BUF_SIZE);
    if ( !mbuf ) {
        fprintf(stderr,"Error allocating %d bytes\n", BULK_BUF_SIZE);
        goto err;
    }

// verify mode and prepare device if necessary
    if ( ztex_default_gpio_ctl(handle,0,0) ) {
	fprintf(stderr,"Warning: wrong initial mode, switching to mode 0\n");
	ztex_default_gpio_ctl(handle,7,0);
        ztex_default_reset(handle,0);
        libusb_bulk_transfer(handle, info.default_in_ep, mbuf, BULK_BUF_SIZE, &transferred, 250);
    } else {
	status = libusb_bulk_transfer(handle, info.default_in_ep, mbuf, BULK_BUF_SIZE, &transferred, 250);
	if ( (status>=0) && (transferred>0) ) fprintf(stderr,"Warning: found %d bytes in EZ-USB FIFO\n", transferred);
    }
    fflush(stderr);

// test 1: read-write test (mode 0)
    for (int i=0; i<RW_SIZE; i+=2) {
	mbuf[i] = (i>>1) & 127;
	mbuf[i+1] = 128 | ((i >> 8 ) & 127);
    }
    TWO_TRIES(status, libusb_bulk_transfer(handle, info.default_out_ep, mbuf, RW_SIZE, &transferred, 2000));
    if ( status < 0 ) {
        fprintf(stderr,"Bulk write error: %s\n", libusb_error_name(status));
        goto err;
    }
    printf("Read-write test, short packet test: wrote %d Bytes\n", transferred);
    fflush(stdout);
    
    TWO_TRIES(status, libusb_bulk_transfer(handle, info.default_in_ep, mbuf, BULK_BUF_SIZE, &transferred, 4000));
    if ( status < 0 ) {
        fprintf(stderr,"Bulk read error: %s\n", libusb_error_name(status));
        goto err;
    }
    {
	int i=mbuf[0]>>7;
	int j=mbuf[i] | ((mbuf[i+1] & 127)<<7);
	printf("Read-write test: read (%d=%d*512+%d) Bytes.  %d leading Bytes lost", transferred, transferred/512, transferred&511, j*2  );
	if ( j ) printf("(This may be platform specific)" );
	size = 0;
	for (i=i+2; i+1<transferred; i+=2) {
	    int k = mbuf[i] | ((mbuf[i+1] & 127)<<7);
	    if ( k != ((j+1) & 0x3fff) ) size +=1;
	    j=k;
        }
	printf(". %d data errors.  %d Bytes remaining in FIFO due to memory transfer granularity\n", size, RW_SIZE-transferred);
	fflush(stdout);
    }

// test 2: read rate test using test data generator (mode 1)
    // reset application and set mode 1
    ztex_default_reset(handle,0);
    status = ztex_default_gpio_ctl(handle,7,1);
    if ( status < 0 ) {
        fprintf(stderr,"Error setting GPIO's: %s\n", libusb_error_name(status));
        goto err;
    }

    // read data and measure time, first packets are ignored because EZ-USB buffer may be filled
    printf("Measuring read rate using libusb_bulk_transfer ... \n");
    fflush(stdout);
    size=0;
    for (int i=0; i<55; i++) {
	if ( i==5 ) gettimeofday(&tv1, NULL);
	TWO_TRIES( status, libusb_bulk_transfer(handle, info.default_in_ep, mbuf, BULK_BUF_SIZE, &transferred, 2000));
	if ( status < 0 ) {
    	    fprintf(stderr,"Bulk read error: %s\n", libusb_error_name(status));
    	    goto err;
	}
	if ( (i==0) && ( (mbuf[0]!=0) && (mbuf[1]!=239) ) ) {
	    fprintf(stderr,"Warning: Invalid start of data: %d %d, leading data may went lost\n",mbuf[0], mbuf[1]);
	}
	if ( i>=5 ) size+=transferred;
    }
    gettimeofday(&tv2, NULL);
    printf("Read %.1f MB at %.1f MB/s\n", size/(1024.0*1024.0), size/( (tv2.tv_sec-tv1.tv_sec)*1e6 + (tv2.tv_usec-tv1.tv_usec) )); 
    fflush(stdout);

    // release resources
    status = 0;
    goto noerr;
err:
    status = 1;
noerr:
    if ( mbuf ) free(mbuf);
    if ( bitstream_fn ) free(bitstream_fn);
    if ( handle ) {
	libusb_release_interface(handle,0);
	libusb_close(handle);
    }
    if ( devs ) libusb_free_device_list(devs, 1);
    libusb_exit(NULL);
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    printf("Press <return> to quit\n");
    fflush(NULL);
    fgetc(stdin);
#endif    
    return status;
}
///@endcond
