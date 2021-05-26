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

/// @file ztex.h 

/** @mainpage
@brief 
The C Core API of the <a href="http://www.ztex.de/firmware-kit/index.e.html">ZTEX SDK</a>.
<p>
This API is used for developing host software in C. It is a re-implementation a subset of the <a href="../../java/index.html">JAVA API</a>.
<p>
<h2>Features</h2>
The main features are:
<ul>
    <li> Full support of <a href="http://www.ztex.de/firmware-kit/default.e.html">Default Interface</a> 
	<ul>
	    <li> Multiple communication interfaces: high speed, low speed, GPIO's, reset signal
	    <li> Compatibility allows board independent host software
	</ul>
    </li>
    <li> Bitstream upload directly to the FPGA
    <li> Utility functions (gathering information about FPGA Boards, finding devices using filters, ...)
</ul>    

<p>
<h2>Usage, Examples</h2>
The library consists in the single file pair 'ztex.c' and 'ztex.h'. It has been tested with gcc and should work with no/minor
modifications with other compilers.
<p>
Its recommended to link the application directly against the object file.
<p>
The package contains two examples, 'ucecho.c' and 'memfifo.c' which demonstrate the usage of the API including the default interface.
<p>
Object files and examples can be built using make. Under Windows it has been tested with MSYS/MinGW, see 
<a href="http://wiki.ztex.de/doku.php?id=en:software:windows_hints#hints">Hints for Windows users</a>


<h2>Related Resources</h2>
Additional information can be found at 
<ul> 
  <li> <a href="http://www.ztex.de/firmware-kit/index.e.html">ZTEX SDK</a>
  <li> <a href="http://wiki.ztex.de/">ZTEX Wiki</a>
</ul>
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <libusb-1.0/libusb.h>
#if defined(unix) || defined(_unix)
#include <unistd.h>
#endif


#include "ztex.h"

#define SPRINTF_INIT( buf, maxlen )  	\
char* buf__ = buf;			\
int bufptr__ = 0;			\
int bufremain__ = maxlen-1;		\
buf__[0]=0;

#define SPRINTF_RETURN return bufptr__;

#define SPRINTF( args... ) { 						\
    if ( bufremain__>0 ) {						\
	int bufincr = snprintf(buf__+bufptr__,bufremain__, args);	\
	if ( bufincr>0 ) {						\
	    if (bufincr > bufremain__) bufincr = bufremain__;		\
	    bufptr__+=bufincr;						\
	    bufremain__-=bufincr;					\
	    buf__[bufptr__] = 0;					\
	}								\
    }									\
}

// ******* ztex_scan_bus *******************************************************
/** Scans the bus and performs certain operations on it. 
  * @param devs A null terminated list of devices
  * @param op Operation to perform. <0: print bus and ignore filters, 0: find a device using the filters, >0 print all devices matching the filters
  * @param id_vendor,id_product Filter by USB ID's that specify the device, ignored if negative
  * @param busnum,devnum Filter by bus number and device address, ignored if negative
  * @param sn Filter by serial number, ignored if NULL
  * @param ps Filter by product string, ignored if NULL
  * @param sbuf string buffer for output 
  * @param sbuflen length of the string buffer
  * @return The device index if filter result unique, otherwise -1
  */
int ztex_scan_bus(char *sbuf, int sbuflen, libusb_device **devs, int op, int id_vendor, int id_product, int busnum, int devnum, char* sn, char* ps)
{
    libusb_device *dev;
    int status = 0;
    int bn = -1, dn = -1;
    struct libusb_device_descriptor dev_desc;
    libusb_device_handle *handle = NULL;
    char product_string[128];
    char sn_string[64];

    SPRINTF_INIT(sbuf,sbuflen);

    if (op<0) {
	id_vendor = id_product = busnum = devnum = -1;
	sn = ps = NULL;
    }
    
    int dev_idx=-1, result=-1;
    for (int i=0; (dev=devs[i]) != NULL; i++) {
        bn = libusb_get_bus_number(dev);
        dn = libusb_get_device_address(dev);
        status = libusb_get_device_descriptor(dev, &dev_desc);
	if (status < 0) {
	    SPRINTF("Warning: Bus %d Device %d: can't read device descriptor %s\n",  bn, dn, libusb_error_name(status));
	} else if ( ((id_vendor<0) || (dev_desc.idVendor==id_vendor)) && 
	            ((id_product<0) || (dev_desc.idProduct==id_product)) &&
	            ((busnum<0) || (busnum==bn)) && 
	            ((devnum<0) || (devnum==dn)) ) {
	    if ( (sn!=NULL) || (ps!=NULL) || (op!=0) ) {
	        status = libusb_open(dev, &handle);
		if (status < 0) {
		    //SPRINTF(stderr, "Warning: libusb_open() failed: %s\n", libusb_error_name(status));
	    	    product_string[0] = 0;
	    	    sn_string[0] = 0;
		}
		else {
	    	    if ( libusb_get_string_descriptor_ascii(handle, dev_desc.iProduct, (unsigned char*)product_string, 128) < 0 ) product_string[0] = 0;
	    	    if ( libusb_get_string_descriptor_ascii(handle, dev_desc.iSerialNumber, (unsigned char*)sn_string, 64) < 0 ) sn_string[0] = 0;
	    	    libusb_close(handle);
		}
	    }
	    
	    if ( ( (sn==NULL) || (strcmp(sn,sn_string)==0) ) && ( (ps==NULL) || (strcmp(ps,product_string)==0) ) ) {
		if ( op ) SPRINTF("Bus %d Device %d:    ID 0x%x:0x%x    Product '%s'    SN '%s'\n",bn, dn, dev_desc.idVendor, dev_desc.idProduct, product_string, sn_string);
		if ( dev_idx<=0 ) {
		    result = i;
		    dev_idx = i;
		}
		else {
		    result = -2;
		    if ( !op ) {
			SPRINTF("Error: More than 1 matching devices found:\n");
			i=dev_idx-1;
			op=1;
		    }
		}
	    }
	}
    }
    return result;
}


// ******* ztex_get_device_info ************************************************
/** Get device information.
  * @param handle device handle
  * @param info structure where device information are stored.
  * @return the error code or 0, if no error occurs.
 */
int ztex_get_device_info(libusb_device_handle *handle, ztex_device_info *info) {
    struct libusb_device_descriptor dev_desc;
    unsigned char *buf = info->product_string; // product string is used as buffer
    int status;

    // VR 0x33: fast configuration info
    TWO_TRIES(status, libusb_control_transfer(handle, 0xc0, 0x33, 0, 0, buf, 128, 1500));
    info->fast_config_ep = status > 0 ? buf[0] : 0;
    info->fast_config_if = status == 2 ? buf[1] : 0;

    // VR 0x3b: configuration data
    TWO_TRIES(status, libusb_control_transfer(handle, 0xc0, 0x3b, 0, 0, buf, 128, 1500));
    if ( status < 0 ) status = libusb_control_transfer(handle, 0xc0, 0x3b, 0, 0, buf, 128, 1500);
    if ( (status==128) && (buf[0]==67) && (buf[1]==68) && (buf[2]==48) ) {
	info->fx_version = buf[3];
	info->board_series = buf[4];
	info->board_number = buf[5];
	info->board_variant[0] = buf[6];
	info->board_variant[1] = buf[7];
	info->board_variant[2] = 0;
    }
    else {
	info->fx_version = 0;
	info->board_series = 255;
	info->board_number = 255;
	info->board_variant[0] = 0;
    }

    // VR 0x64: default interface info
    TWO_TRIES(status, libusb_control_transfer(handle, 0xc0, 0x64, 0, 0, buf, 128, 1500));
    if ( status>2 && buf[0]>0 ) {
	info->default_version1 = buf[0];
	info->default_version2 = status>3 ? buf[3] : 0;
	info->default_out_ep = buf[1] & 127;
	info->default_in_ep = buf[2] | 128;
    }
    else {
	info->default_version1 = 0;
	info->default_version2 = 0;
	info->default_out_ep = 255;
	info->default_in_ep = 255;
    }
    
    // string descriptors
    status = libusb_get_device_descriptor(libusb_get_device(handle), &dev_desc);
    if ( status < 0 ) return status;
    if ( libusb_get_string_descriptor_ascii(handle, dev_desc.iProduct, info->product_string, 128) < 0 ) info->product_string[0] = 0;
    if ( libusb_get_string_descriptor_ascii(handle, dev_desc.iSerialNumber, info->sn_string, 64) < 0 ) info->sn_string[0] = 0;
    
    return 0;
}


// ******* ztex_print_device_info **********************************************
/** Print device info to a string. 
  * The output may be truncated and is always null-terminated.
  * @param sbuf string buffer for output 
  * @param sbuflen length of the string buffer
  * @param info structure where device information are stored.
  * @return length of the output string (may be truncated)
  */
int ztex_print_device_info(char* sbuf, int sbuflen, const ztex_device_info *info) {
    SPRINTF_INIT(sbuf,sbuflen);
    SPRINTF("Product: '%s'\n", info->product_string);
    SPRINTF("Serial number: '%s'\n", info->sn_string);
    SPRINTF("USB-Controller: %s\n", info->fx_version==2 ? "EZ-USB FX2" : info->fx_version==3 ? "EZ-USB FX3" : "unknown" );
    if ( (info->board_series<255) && (info->board_number<255) )
	SPRINTF("Board: ZTEX USB-FPGA %d.%.2d%s\n",info->board_series, info->board_number, info->board_variant)
    else 
	SPRINTF("Board: unknown\n")
    if ( info->fast_config_ep ) 
	SPRINTF("High speed FPGA configuration: EP %d,  IF %d\n",info->fast_config_ep, info->fast_config_if)
    else 
	SPRINTF("High speed FPGA configuration: not supported\n")
    if ( (info->default_version1>0) ) 
	SPRINTF("Default interface: v%d.%d,  OUT EP %d,  IN EP %d\n", info->default_version1, info->default_version2, info->default_out_ep & 127, info->default_in_ep & 127)
    else 
	SPRINTF("Default interface: not available\n")
    SPRINTF_RETURN;
}


// ******* ztex_get_fpga_config ************************************************
/** Get device information.
  * @param handle device handle
  * @return <0 if an USB error occurred, 0 if unconfigured, 1 if configured
 */
int ztex_get_fpga_config(libusb_device_handle *handle) {
    unsigned char buf[16]; 
    int status;
    TWO_TRIES(status, libusb_control_transfer(handle, 0xc0, 0x30, 0, 0, buf, 16, 1500));
    return status < 0 ? status : status == 0 ? -255 : buf[0] == 0 ? 1 : 0;
}

#if defined(unix) || defined(_unix)
#define FILE_EXISTS(fn) access(fn,R_OK)
#else
// the ugly method for ugly windows
int FILE_EXISTS(const char* fn) {
    FILE *f = fopen(fn,"rb");
    if ( !f ) return -1;
    fclose(f);
    return 0;
}
#endif

// ******* ztex_find_bitstream *************************************************
/** Search for bitstream at standard locations.
    @param info device information, used for determining locations of the SDK examples, ignored if NULL
    @param path additional path to search, ignored if NULL
    @param name without suffix '.bit'
    @return the file name or NULL no file found
  */
char* ztex_find_bitstream(const ztex_device_info *info, const char *path, const char* name) {
    char *fn = malloc(256);
    if ( !fn )  return NULL;  // out of memory, should not occur
    int status = -1;
    
    if ( snprintf(fn, 256, "%s.bit", name) > 0 ) status=FILE_EXISTS(fn);
    if ( (status<0) && path && (snprintf(fn, 256, "%s"DIRSEP"%s.bit", path, name) > 0 ) ) status=FILE_EXISTS(fn);
    
    if ( (info!=NULL) && (info->board_series<255) && (info->board_number<255) ) {
	if ( (status<0) && (snprintf(fn, 256, "fpga-%d.%.2d%s"DIRSEP"%s.bit", info->board_series, info->board_number, info->board_variant, name)) ) status=FILE_EXISTS(fn);
	if ( (status<0) && (snprintf(fn, 256, "fpga-%d.%.2d"DIRSEP"%s.runs"DIRSEP"impl_%d_%.2d%s"DIRSEP"%s.bit" , info->board_series, info->board_number, name, info->board_series, info->board_number, info->board_variant, name)) ) status=FILE_EXISTS(fn);
	if ( (status<0) && path && (snprintf(fn, 256, "%s"DIRSEP"fpga-%d.%.2d%s"DIRSEP"%s.bit", path, info->board_series, info->board_number, info->board_variant, name)) ) status=FILE_EXISTS(fn);
	if ( (status<0) && path && (snprintf(fn, 256, "%s"DIRSEP"fpga-%d.%.2d"DIRSEP"%s.runs"DIRSEP"impl_%d_%.2d%s"DIRSEP"%s.bit" , path, info->board_series, info->board_number, name, info->board_series, info->board_number, info->board_variant, name)) ) status=FILE_EXISTS(fn);
    }
    
    if ( (status<0) && (snprintf(fn, 256, "fpga"DIRSEP"%s.bit", name) > 0) ) status=FILE_EXISTS(fn);
    if ( (status<0) && path && (snprintf(fn, 256, "%s"DIRSEP"fpga"DIRSEP"%s.bit", path, name) > 0) ) status=FILE_EXISTS(fn);
    if ( status<0 ) return NULL;
    return fn;
}


// ******* ztex_upload_bitstream ***********************************************
/** Upload bitstream to volatile memory.
  * @param sbuf string buffer for error messages
  * @param sbuflen length of the string buffer
  * @param handle device handle
  * @param info device information, used for determining high speed configuration settings, ignored if NULL
  * @param fd File to read from. I/O errors are ignored.
  * @param bs 0: disable bit swapping, 1: enable bit swapping, all other values: automatic detection of bit order.
  * @return -1 if an error occurred, otherwise 0. Error messages are printed to sbuf.
  */
int ztex_upload_bitstream(char* sbuf, int sbuflen, libusb_device_handle *handle, const ztex_device_info *info, FILE *fd, int bs) {
#define EP0_TRANSACTION_SIZE 2048
#define BUF_SIZE 32768
#define BUFS_SIZE 2048
    unsigned char *bufs[BUFS_SIZE]; 	// up to 64MB
    uint8_t *buf;
    int status, size = 0, bufs_idx, buf_idx=BUF_SIZE;
    SPRINTF_INIT(sbuf,sbuflen);

    if ( bs > 1 ) bs=-1;    
    // read bitstream
    for ( bufs_idx=0; (bufs_idx<BUFS_SIZE) && (buf_idx==BUF_SIZE); bufs_idx++ ) {
	buf = malloc(BUF_SIZE);
	bufs[bufs_idx] = buf;
	if ( buf == NULL ) {
	    SPRINTF("Error allocating memory\n");
	    return -1;
	}
	// prepend 512 bytes dummy data
	if ( bufs_idx==0 ) {
	    buf_idx  = 512;
	    for (int i=0; i<512; i++) buf[i]=0;
	}
	else {
	    buf_idx=0;
	}
	
	do {
	    status = fread(buf+buf_idx, 1, BUF_SIZE-buf_idx, fd);
	    if ( status > 0 ) {
		size += status;
		buf_idx+=status;
	    }
	} while ( (buf_idx<BUF_SIZE) && (status>0) );
	if ( (buf_idx<BUF_SIZE) && (buf_idx % 64 == 0 ) )  {
	    buf_idx ++;		// append a dummy byte for proper end of transfer detection
	}
	
	// detect bit order
	if ( bs<0 ) {
	    for ( int i=0; (bs<0) && (i<buf_idx-3); i++ ) {
		if ( (buf[i]==0xaa) && (buf[i+1]==0x99) && (buf[i+2]==0x55) && (buf[i+3]==0x66) ) 
		    bs = 1;
		if ( (buf[i]==0x55) && (buf[i+1]==0x99) && (buf[i+2]==0xaa) && (buf[i+3]==0x66) )
		    bs = 0;
	    }
	    if (bs<0) {
		SPRINTF("Warning: Unable to detect bitstream order\n");
		bs = 0;
	    } 
	}	    
	if ( bs > 0 ) {
	    for (int i=0; i<buf_idx; i++ ) {
		char b = buf[i];
		buf[i] = ((b & 128) >> 7) |
 		     	 ((b &  64) >> 5) |
		     	 ((b &  32) >> 3) |
		     	 ((b &  16) >> 1) |
		     	 ((b &   8) << 1) |
		     	 ((b &   4) << 3) |
		     	 ((b &   2) << 5) |
		     	 ((b &   1) << 7);
	    }
	}
    }
    
    if ( size < 1024 || size % 64 == 0 ) {
	SPRINTF("Error: Invalid bitstream size: %d\n",size);
	free(bufs[0]);
	return -1;
    }
    
    status = -1;
    int claimed_if = -1;
    for (int try=0; (status<0) && (try<5); try++) {
	// choose EP and claim interface if necessary
	int ep = ((try<2) && (info!=NULL)) ? (info->fast_config_ep & 127) : 0;
	//printf("Try %d: EP=%d  \n",try,ep);
	if ( (ep>0) && (info->fast_config_if!=claimed_if) ) {
	    claimed_if = info->fast_config_if;
	    status = libusb_claim_interface(handle, claimed_if); 
	    if ( status < 0 ) SPRINTF("Warning: Unable to claim interface %d: %s\n", claimed_if, libusb_error_name(status));

	}
	
	// reset FPGA
	TWO_TRIES(status, libusb_control_transfer(handle, 0x40, 0x31, 0, 0, NULL, 0,  1500));
	if ( status < 0 ) SPRINTF("Warning: Unable to reset FPGA: %s\n", libusb_error_name(status));

	// init FPGA configuration
	if ( ep>0 ) {
	    TWO_TRIES( status, libusb_control_transfer(handle, 0x40, 0x34, 0, 0, NULL, 0,  1500));
	    if ( status < 0 ) {
		SPRINTF("Warning: Unable to initialize high speed configuration: %s\n", libusb_error_name(status));
		break;
	    }
	}
	
	// transfer data
	status = 0;
	for ( int i=0; (status>=0) && (i<bufs_idx); i++ ) {
	    int j = i == (bufs_idx-1) ? buf_idx : BUF_SIZE;
	    int k;
	    buf=bufs[i];
	    if ( ep>0 ) {
		while ( (status>=0) && (j > 0) ) {
		    TWO_TRIES(status, libusb_bulk_transfer(handle, ep, buf, j, &k, 2000));
		    if ( status<0 ) {
			//printf("try=%d, i=%d, j=%d, k=%d\n",try,i,j,k);
			SPRINTF("Warning: Error transferring bitstream: %s\n", libusb_error_name(status));
		    } else if (k<1 ) {
			SPRINTF("Warning: Unknown transfer error\n");
			status=-1;
		    } else {
			buf+=k;
			j-=k;
		    }
		}
	    }
	    else {
		for (int k=0; (status>=0) && (k<j); k+=EP0_TRANSACTION_SIZE) {
		    int l = j-k < EP0_TRANSACTION_SIZE ? j-k : EP0_TRANSACTION_SIZE;
		    TWO_TRIES( status, libusb_control_transfer(handle, 0x40, 0x32, 0, 0, buf+k, l, 1500));
		    if ( status<0 ) {
			SPRINTF("Warning: Error transferring bitstream: %s\n", libusb_error_name(status));
		    } 
		    else if ( status!=l ) {
			SPRINTF("Warning: Error transferring bitstream: %d bytes lost\n", l-status);
			status=-1;
		    }
		}
	    }
	}

	// finish FPGA configuration
	if ( ep>0 ) {
	    TWO_TRIES(status, libusb_control_transfer(handle, 0x40, 0x35, 0, 0, NULL, 0,  1500));
	    if ( status < 0 ) SPRINTF("Warning: Unable to finish high speed configuration: %s\n", libusb_error_name(status));
	}

	// check configuration state
	status = ztex_get_fpga_config(handle);
	if ( status < 0 ) {
	    SPRINTF("Error: Unable to get FPGA configuration state: %s\n", libusb_error_name(status));
	    status = -1;
	    break;
	} 
	if ( status==0 ) {
	    SPRINTF("Warning: %s FPGA configuration failed\n", ep>0 ? "high-speed" : "low-speed");
	    status = -1;
	} else {
	    status = 0;
	}
    }
    
    if ( claimed_if>=0 ) {
	libusb_release_interface(handle, claimed_if);
    }

    for ( int i=0; i<bufs_idx; i++ ) {
	free(bufs[i]);
    }

    return status;
}


// ******* ztex_default_gpio_ctl ***********************************************
/**
  * Reads and modifies the 4 GPIO pins.
  * @param handle device handle
  * @param mask Bitmask for the pins which are modified. 1 means a bit is set. Only the lowest 4 bits are significant.
  * @param value The bit values which are to be set. Only the lowest 4 bits are significant.
  * @return current values of the GPIO's or <0 if an USB error occurred
  */
int ztex_default_gpio_ctl (libusb_device_handle *handle, int mask, int value) {
    unsigned char buf[8];
    int status;
    TWO_TRIES(status, libusb_control_transfer(handle, 0xc0, 0x61, value, mask, buf, 8, 1500));
    return status < 0 ? status : buf[0];
}

// ******* ztex_default_reset **************************************************
/**
  * Assert the reset signal.
  * @param handle device handle
  * @param leave if >0, the signal is left active. Otherwise only a short impulse is sent.
  * @return 0 on success or <0 if an USB error occurred
  */
int ztex_default_reset(libusb_device_handle *handle,  int leave ) {
    int status;
    TWO_TRIES(status, libusb_control_transfer(handle, 0x40, 0x60, leave ? 1 : 0, 0, NULL, 0 , 1500));
    return status;
}


// ******* ztex_default_lsi_set1 ***********************************************
/**
  * Send data to the low speed interface of default firmwares.
  * It's implemented as a SRAM-like interface and is typically used used to read/write configuration data, debug information or other things.
  * This function sets one register.
  * @param handle device handle
  * @param addr The address. Valid values are 0 to 255.
  * @param val The register data with a width of 32 bit.
  * @return 0 on success or <0 if an USB error occurred
  */
int ztex_default_lsi_set1 (libusb_device_handle *handle, uint8_t addr, uint32_t val) {
    unsigned char buf[] = { val, val>>8, val>>16, val>>24, addr & 255 };
    int status;
    TWO_TRIES(status, libusb_control_transfer(handle, 0x40, 0x62, 0, 0, buf, 5, 1500));
    return status;
}


// ******* ztex_default_lsi_set2 ***********************************************
/**
  * Send data to the low speed interface of default firmwares.
  * It's implemented as a SRAM-like interface and is typically used used to read/write configuration data, debug information or other things.
  * This function sets a sequential set of registers.
  * @param handle device handle
  * @param addr The starting address address. Valid values are 0 to 255. Address is wrapped from 255 to 0.
  * @param val The register data array with a word width of 32 bit.
  * @param length The length of the data array.
  * @return 0 on success or <0 if an USB error occurred
  */
int ztex_default_lsi_set2 (libusb_device_handle *handle, uint8_t addr, const uint32_t *val, int length) {
    if (length<0) return 0;
    int ia = length-256;
    if (ia<0) ia = 0;
    uint8_t *buf = malloc((length-ia)*5);
    for (int i=ia; i<length; i++) {
        buf[(i-ia)*5+0]=val[i];
        buf[(i-ia)*5+1]=val[i]>>8;
        buf[(i-ia)*5+2]=val[i]>>16;
        buf[(i-ia)*5+3]=val[i]>>24;
        buf[(i-ia)*5+4]=addr+i;
    }
    int status;
    TWO_TRIES(status, libusb_control_transfer(handle, 0x40, 0x62, 0, 0, buf, (length-ia)*5, 1500));
    free(buf);
    return status;
}


// ******* ztex_default_lsi_set3 **********************************************
/**
  * Send data to the low speed interface of default firmwares.
  * It's implemented as a SRAM-like interface and is typically used used to read/write configuration data, debug information or other things.
  * This function sets a sequential set of registers.
  * @param handle device handle
  * @param addr The register addresses. Valid values are 0 to 255.
  * @param val The register data array with a word width of 32 bit.
  * @param length The length of the data array.
  * @return 0 on success or <0 if an USB error occurred
  */
int ztex_default_lsi_set3 (libusb_device_handle *handle, const uint8_t *addr, const uint32_t *val, int length) {
    if (length<0) return 0;
    int ia = length-256;
    if (ia<0) ia = 0;
    uint8_t *buf = malloc((length-ia)*5);
    for (int i=ia; i<length; i++) {
        buf[(i-ia)*5+0]=val[i];
        buf[(i-ia)*5+1]=val[i]>>8;
        buf[(i-ia)*5+2]=val[i]>>16;
        buf[(i-ia)*5+3]=val[i]>>24;
        buf[(i-ia)*5+4]=addr[i];
    }
    int status;
    TWO_TRIES(status, libusb_control_transfer(handle, 0x40, 0x62, 0, 0, buf, (length-ia)*5, 1500));
    free(buf);
    return status;
}


// ******* ztex_default_lsi_get1 ***********************************************
/**
  * Read data from the low speed interface of default firmwares.
  * It's implemented as a SRAM-like interface and is typically used used to read/write configuration data, debug information or other things.
  * This function reads one register.
  * @param handle device handle
  * @param addr The address. Valid values are 0 to 255.
  * @return The unsigned register value (32 Bits) or <0 if an error occurred.
  */
int64_t ztex_default_lsi_get1 (libusb_device_handle *handle, uint8_t addr) {
    uint8_t buf[4];
    int status;
    TWO_TRIES(status, libusb_control_transfer(handle, 0xc0, 0x63, 0, addr, buf, 4, 1500));
    return status<0 ? status : buf[0] | (buf[1]<<8) | (buf[2]<<16) | (buf[3]<<24);
}

// ******* ztex_default_lsi_get2 ***********************************************
/**
  * Read data from the low speed interface of default firmwares.
  * It's implemented as a SRAM-like interface and is typically used used to read/write configuration data, debug information or other things.
  * This function reads a sequential set of registers.
  * @param handle device handle
  * @param addr The start address. Valid values are 0 to 255. Address is wrapped from 255 to 0.
  * @param val The array where to store the register data with a word width of 32 Bit.
  * @param length The amount of register to be read.
  * @return 0 on success or <0 if an USB error occurred
  */
int ztex_default_lsi_get2 (libusb_device_handle *handle, uint8_t addr, uint32_t *val, int length) {
    int l=length;
    if (l>256) l=256;
    uint8_t *buf = malloc(l*4);
    int status;
    TWO_TRIES(status, libusb_control_transfer(handle, 0xc0, 0x63, 0, addr, buf, l*4, 1500));
    if (status < 0 ) return status;
    for (int i=0; i<length; i++) {
	int j = i & 255;
	val[i] = buf[j*4+0] | (buf[j*4+1]<<8) | (buf[j*4+2]<<16) | (buf[j*4+3]<<24);
    }
    free(buf);
    return 0;
}
