/*%
   Default firmware and loader for ZTEX USB-FPGA Modules 2.04
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
		"    -d <number>  Device Number (default: 0)\n" +
		"    -p           Print bus info\n" +
		"    -f           Force upload Firmware to RAM\n" + 
//		"    -va          Upload configuration data for USB-FPGA Modules 2.04a\n" +
		"    -vb          Upload configuration data for USB-FPGA Modules 2.04b\n" +
//		"    -vc          Upload configuration data for USB-FPGA Modules 2.04b\n" +
		"    -c           Clear settings from configuration data\n" +
		"    -ue          Upload Firmware to EEPROM\n" +
		"    -re          Reset EEPROM Firmware\n" +
		"    -r           Reset device after uploading\n" +
		"    -h           This help" );
    
    public ParameterException (String msg) {
	super( msg + "\n" + helpMsg );
    }
}

// *****************************************************************************
// ******* Default *************************************************************
// *****************************************************************************
class Default {

// ******* main ****************************************************************
    public static void main (String args[]) {
    
	int devNum = 0;
	boolean force = false;
	boolean clear = false;
	boolean reset = false;
	int variant = 0;

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
// Scan the USB. This also creates and initializes a new USB context.
	    ZtexScanBus1 bus = new ZtexScanBus1( ZtexDevice1.ztexVendorId, ZtexDevice1.ztexProductId, true, false, 1);
	    if ( bus.numberOfDevices() <= 0) {
		System.err.println("No devices found");
	        System.exit(0);
	    }
	    
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
		else if ( args[i].equals("-p") ) {
	    	    bus.printBus(System.out);
		    System.exit(0);
		}
		else if ( args[i].equals("-f") ) {
	    	    force = true;
		}
		else if ( args[i].equals("-va") ) {
		    variant = 1;
		}
		else if ( args[i].equals("-vb") ) {
		    variant = 2;
		}
		else if ( args[i].equals("-vc") ) {
		    variant = 3;
		}
		else if ( args[i].equals("-c") ) {
		    clear = true;
		}
		else if ( args[i].equals("-r") ) {
		    reset = true;
		}
		else if ( args[i].equals("-h") ) {
		    System.err.println(ParameterException.helpMsg);
	    	    System.exit(0);
		}
		else if ( !args[i].equals("-re") && !args[i].equals("-ue") )
		    throw new ParameterException("Invalid Parameter: "+args[i]);
	    }

// create the main class	    
	    Ztex1v1 ztex = new Ztex1v1 ( bus.device(devNum) );
	    bus.unref();
	    
// upload the firmware if necessary
	    if ( force || ! ztex.valid() || ! ztex.InterfaceCapabilities(ztex.CAPABILITY_EEPROM) || ! ztex.InterfaceCapabilities(ztex.CAPABILITY_MAC_EEPROM) ) {
		System.out.println("Firmware upload time: " + ztex.uploadFirmware( "default.ihx", force ) + " ms");
	    }	
	    
    	    for (int i=0; i<args.length; i++ ) {
		if ( args[i].equals("-re") ) {
		    ztex.nvDisableFirmware();
		} 
		else if ( args[i].equals("-ue") ) {
		    System.out.println("Firmware to EEPROM upload time: " + ztex.nvUploadFirmware( "default.ihx", force ) + " ms");
		}
	    }

//	    if ( ztex.config!=null ) System.out.println(ztex.config.getName());
	    
// generate and upload config data
	    if ( variant > 0 )
	    {
    		ConfigData config = new ConfigData();
    		if ( ! clear  ) {
    		    if ( config.connect(ztex) ) 
    			System.out.println("Reading configuration data."); 
    		    config.disconnect();
    		}
    		
    		if ( variant == 1 ) {
		    config.setName("ZTEX USB-FPGA Module", 2, 04, "a");
		    config.setFpga("XC6SLX9", "FTG256", "2C");
		    config.setMaxBitstreamSize(112);
		    config.setRam(64,"DDR-400 SDRAM");
		}
    		else if ( variant == 2 ) {
		    config.setName("ZTEX USB-FPGA Module", 2, 04, "b");
		    config.setFpga("XC6SLX16", "FTG256", "2C");
		    config.setMaxBitstreamSize(160);
		    config.setRam(64,"DDR-400 SDRAM");
		}
    		else {
		    config.setName("ZTEX USB-FPGA Module", 2, 04, "c");
		    config.setFpga("XC6SLX25", "FTG256", "3C");
		    config.setMaxBitstreamSize(256);
		    config.setRam(64,"DDR-400 SDRAM");
		}

		System.out.println("Writing configuration data."); 
    		ztex.config=null;
		ztex.macEepromWrite(0, config.data(), 128);
	    }
	    
	    if ( reset ) ztex.resetEzUsb();

	    ztex.dispose();  // this also releases clamied interfaces
		
	}
	catch (Exception e) {
	    System.out.println("Error: "+e.getLocalizedMessage() );
	} 
	catch (Error e) {
	    System.out.println("Error: "+e.getLocalizedMessage() );
	} 
    } 
   
}
