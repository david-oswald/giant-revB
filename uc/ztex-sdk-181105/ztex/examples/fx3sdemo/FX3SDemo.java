/*%
   fx3sdemo -- Demonstrates common features of the FX3S
   Copyright (C) 2009-2017 ZTEX GmbH.
   http://www.ztex.de

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
%*/

import java.io.*;
import java.util.*;
import java.nio.*;

import org.usb4java.*;

import ztex.*;

// *****************************************************************************
// ******* ParameterException **************************************************
// *****************************************************************************
// Exception the prints a help message
class ParameterException extends Exception {
    public final static String helpMsg = new String (
		"Parameters:\n"+
		"    -d <number>       Device number (default: 0)\n" +
		"    -f                Force uploads\n" +
		"    -p                Print bus info\n" +
		"    -h                This help" );
    
    public ParameterException (String msg) {
	super( msg + "\n" + helpMsg );
    }
}

// *****************************************************************************
// ******* FX3SDemo ************************************************************
// *****************************************************************************
class FX3SDemo extends Ztex1v1 {

// ******* Debug ***************************************************************
// constructor
    public FX3SDemo ( ZtexDevice1 pDev ) throws UsbException {
	super ( pDev );
    }

// ******* main ****************************************************************
    public static void main (String args[]) {
    
	int devNum = 0;
	boolean force = false;
	String bitStream = null;

	if ( ! System.getProperty("os.name").equalsIgnoreCase("linux") ) {
	    Runtime.getRuntime().addShutdownHook(new Thread() {
		public void run() { 
    		    Scanner s=new Scanner(System.in);
    		    System.out.println("Press <enter> to continue ...");
    		    s.nextLine();
		}
	    });	
	}
		
	try {
// init USB stuff

// Scan the USB. This also creates and initializes a new USB context.
	    ZtexScanBus1 bus = new ZtexScanBus1( ZtexDevice1.ztexVendorId, ZtexDevice1.ztexProductId, true, false, 1);
	    
// scan the command line arguments
    	    for (int i=0; i<args.length; i++ ) {
	        if ( args[i].equals("-d") ) {
	    	    i++;
		    try {
			if (i>=args.length) throw new Exception();
    			devNum = Integer.parseInt( args[i] );
		    } 
		    catch (Exception e) {
		        throw new ParameterException("Device number expected after -d");
		    }
		}
	        if ( args[i].equals("-b") ) {
	    	    i++;
		    try {
			if (i>=args.length) throw new Exception();
    			bitStream =args[i];
		    } 
		    catch (Exception e) {
		        throw new ParameterException("Bitstream file expected after -b");
		    }
		}
		else if ( args[i].equals("-f") ) {
		    force = true;
		}
		else if ( args[i].equals("-p") ) {
	    	    bus.printBus(System.out);
		    System.exit(0);
		}
		else if ( args[i].equals("-p") ) {
	    	    bus.printBus(System.out);
		    System.exit(0);
		}
		else if ( args[i].equals("-h") ) {
		    System.err.println(ParameterException.helpMsg);
	    	    System.exit(0);
		}
		else throw new ParameterException("Invalid Parameter: "+args[i]);
	    }
	    

// create the main class	    
	    if ( bus.numberOfDevices() <= 0) {
		System.err.println("No devices found");
	        System.exit(0);
	    }
	    FX3SDemo ztex = new FX3SDemo ( bus.device(devNum) );
	    bus.unref();
	    
// upload the firmware if necessary
	    if ( force || ! ztex.valid() || ! ztex.dev().productString().equals("Demo for ZTEX FX3S Boards")  ) {
		if ( ztex.dev().valid() ) 
		System.out.println("Trying to overwrite firmware in RAM. This only works if a board specific firmware with reset support is running.\n"+
		                   "If uploading firmware fails please restart board with firmware disabled." );
		System.out.println("Firmware upload time: " + ztex.uploadFirmware( "fx3sdemo.img", force ) + " ms");
	    }
	    
// print log
	    ztex.debug2PrintNextLogMessages(System.out);

// test flash2
	    System.out.println("SD Flash Test:");
	    if ( ztex.flash2Enabled() ) {
		System.out.println("Sector size: "+ztex.flash2SectorSize());
		System.out.println("Sector size: "+ztex.flash2Sectors());
		System.out.println(ztex.flash2Info());
		final int nsec = 65536 / ztex.flash2SectorSize();
		final int size = nsec * ztex.flash2SectorSize();
//		final int startSector = 131070;
		final int startSector = 711;
	        byte[] buf0 = new byte[size];
		byte[] buf1 = new byte[size];
		byte[] buf2 = new byte[size];
		for (int i=0; i<size; i++ ) {
		    buf1[i] = (byte)((i % 251) & 255);
		    buf2[i] = (byte) 255;
		}
		try {
    		    System.out.println("Reading backup of sectors " + startSector + ".." + (startSector+nsec-1));
		    ztex.flash2ReadSector(startSector, nsec, buf0);
    		    System.out.println("Writing sectors " + startSector + ".." + (startSector+nsec-1));
  		    ztex.flash2WriteSector(startSector, nsec, buf1);
    		    System.out.println("Reading sectors " + startSector + ".." + (startSector+nsec-1));
		    ztex.flash2ReadSector(startSector, nsec, buf2);
		    int errors = 0;
		    for (int i=0; i<size; i++ ) {
			if ( buf1[i] != buf2[i] ) {
			    if ( errors<10 ) System.out.println("Flash error at " + i + ":  " + (buf1[i] & 255) + " " + (buf2[i] & 255)+"        ");
			    else if (errors==10) System.out.println("...");
			    errors++;
			}
		    } 
		    if ( errors == 0 ) System.out.println("No Flash errors");
    		    System.out.println("Restoring sectors " + startSector + ".." + (startSector+nsec-1));
  		    ztex.flash2WriteSector(startSector, nsec, buf0); 

    		    System.out.println("Hint: Try out ../flashbench example for further tests");
		}
		catch (Exception e) {
		    System.out.println("Flash error: " + e.getLocalizedMessage() );
		} 
	    } else {
		System.out.println("No flash present");
	    }

	    ztex.debug2PrintNextLogMessages(System.out); 

// release resources
	    ztex.dispose();
	    
	}
	catch (Exception e) {
	    System.out.println("Error: "+e.getLocalizedMessage() );
	} 
   } 
   
}
