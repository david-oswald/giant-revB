/*%
   memfifo -- Connects the bi-directional high speed interface of default firmware to a FIFO built of on-board SDRAM or on-chip BRAM
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
import java.text.*;
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
		"    -d <number>    Device Number (default: 0)\n" +
		"    -r             Reset EZ-USB\n" +
		"    -v             Print debug info\n" +
		"    -p             Print bus info\n" +
		"    -h             This help" );
    
    public ParameterException (String msg) {
	super( msg + "\n" + helpMsg );
    }
}

// *****************************************************************************
// ******* UsbTestWriter *******************************************************
// *****************************************************************************
class UsbTestWriter extends Thread {
    private volatile boolean terminate = true;
    private volatile boolean isAlive = false;
    private ZtexUsbWriter writer;
    private byte[] b;
    private int bufCount = 0;

    public UsbTestWriter ( Ztex1v1 p_ztex ) throws InvalidFirmwareException, UsbException, CapabilityException {
        super ();
	writer = new ZtexUsbWriter( p_ztex, 8, 512*1024 );
	b = new byte[writer.bufSize()];
    }
    
    public ZtexUsbWriter writer() {
	return writer;
    }

    public boolean terminate(int timeout) throws UsbException {
	terminate = true;
	writer.cancel();
	int i;
	for (i=0; isAlive && i<=timeout; i+=20 ) {
	    try { sleep(20); } catch ( InterruptedException e) { } 
	}
	return (!isAlive) && writer.cancelWait(timeout-i+100);
    }

    public void run() {
	isAlive = true;
	terminate = false;
	int k = 0;
	int cs = 47;
	int sync;
	Random random = new Random();
	while ( !terminate ) {
	    for ( int i=0; !terminate && i<writer.bufSize(); i++ ) {
		int j = k & 15;
		sync = ( ((j & 1)==1) || (j==14) ) ? 128 : 0;
		if ( j == 15 ) {
		    b[i] = (byte) (sync | ((cs & 127) ^ (cs>>7)));
		    cs = 47;
		}
		else {
//		    b[i] = (byte) ( (j==0 ? (k>>4) & 127 : random.nextInt(128)) | sync );
		    b[i] = (byte) ( (((k>>4)+j) & 127) | sync );
		    cs += (b[i] & 255);
		}
		k=(k+1) & 65535;
	    }
	    try {
		if ( writer.transmitBuffer(b,2000)<0 ) {
		    terminate = true;
		    System.err.println("Timeout error transmitting buffer "+bufCount);
		}
	    } 
	    catch ( Exception e) {
		terminate = true;
		System.err.println("Error transmitting buffer "+bufCount+": "+e);
	    }
	    bufCount += 1;
	}
	isAlive = false;
    }
}


// *****************************************************************************
// ******* MemFifo *************************************************************
// *****************************************************************************
class MemFifo extends Ztex1v1 {

    // constructor
    public MemFifo ( ZtexDevice1 pDev ) throws UsbException {
	super ( pDev );
    }

    // set mode
    public void setMode( int i ) throws InvalidFirmwareException, UsbException, CapabilityException {
	defaultGpioCtl(7,i & 3);
    }

    // manual PKTEND
    public void manPktend() throws InvalidFirmwareException, UsbException, CapabilityException {
	defaultGpioCtl(4,4);
	defaultGpioCtl(4,0);
    }

    // reset
    public void reset( ) throws  InvalidFirmwareException, UsbException, CapabilityException {
	defaultGpioCtl(7,0);
	defaultReset();
	debug2PrintNextLogMessages(System.out); 
    }

    // reads data and verifies them. It exits either if iz buffers are read of if a short package occurs
    // returns number of read bytes
    // rate is data rate in kBytes; <=0 means unlimited
    public long verify ( ZtexUsbReader reader, int iz, int rate, boolean verbose, boolean manPktend) throws UsbException {
    	boolean valid = false;
	int byte_cnt = 0, sync_cnt = 0, cs = 47;
	boolean memError = false;
	byte[] b = new byte[reader.bufSize()];
	long size = 0;

	for (int i=0; i<iz; i++) {
	    if ( i>0 && rate>0 ) {
		try {
		    Thread.sleep(reader.bufSize()/rate);
        	}
        	catch ( InterruptedException e) {
        	} 
	    }
	    int bb = reader.getBuffer(b, 5000);
	    size += bb;
	    
	    // manually assert PKTEND in after 21 buffers
	    try {
		if (manPktend && (i==20)) manPktend();
	    }
	    catch ( Exception e) {
		System.err.println(e);
	    }
	    
	    if ( bb<0 ) throw new UsbException("Timeout during reading buffer "+i);
	    else if (bb==0) {
		System.out.println("Received no data for buffer "+i+"                                  ");
		return 0;
	    }

	    int serrors = 0;
	    int merrors = 0;

	    for (int k=0; k<bb; k++ ) {
		byte_cnt++;
		sync_cnt++;
		if ( (b[k] & 128) == 0 ) sync_cnt=0;
		if ( sync_cnt == 3 ) {
		    boolean serror = byte_cnt != 16;
		    boolean merror = (byte_cnt == 16) && ( ( b[k] & 127 ) != ((cs & 127) ^ (cs>>7)) );
		    valid = valid || ( !serror && !merror );
		    if ( valid ) {
			if ( serror ) serrors += 1;
			if ( merror ) merrors += 1;
			if ( verbose && ((serror && serrors <= 2) || (merror && merrors <= 2) )) {
			    if ( serror ) System.out.print( "Sync Error: " + byte_cnt + " at " + k + ": " );
			    else System.out.print( "Data Error: at " + k + ": " );
			    for ( int l=k-byte_cnt-2; l<k+3; l++ )
				if ( (l>=0) && (l<bb) ) 
				    System.out.print((b[l] < 0 ? "*" : "" ) + (b[l] & 127) + " ");
			}
		    }
		    cs=47;
		    byte_cnt = 0;
		} else {
		    cs += b[k] & 255;
		}
	    } 
		    
	    if ( ! valid ) {
		System.out.println("Invalid data");
		return size;
	    }
	
	    System.out.print("Buffer " + i + (bb==reader.bufSize() ? ": " : "(short packet): ") + serrors + " sync errors,    " + merrors + " memory or transfer errors" + "                 \r" );
	    if ( bb!=reader.bufSize() ) {
	        System.out.println();
	        return size;
	    }
	    if ( merrors + serrors > (i==0 ? 2 : 0 ) ) { // two errors in first buffer may occurs after changing mode
	        System.out.println();
	        memError |= (bb!=reader.bufSize());
	    } else if (bb!=reader.bufSize()) {
	        System.out.println();
	    }  
	}
        System.out.println();
	return size;
    }

// ******* main ****************************************************************
    public static void main (String args[]) {
    
	int devNum = 0;
	boolean verbose = false;
	boolean reset = false;
	ZtexUsbReader reader = null;
	UsbTestWriter writer = null;
	ZtexEventHandler eventHandler = null;

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
		else if ( args[i].equals("-v") ) {
		    verbose = true;
		}
		else if ( args[i].equals("-r") ) {
		    reset = true;
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

	    String errStr = "";

// create the main class	    
	    if ( bus.numberOfDevices() <= 0) {
		System.err.println("No devices found");
	        System.exit(0);
	    }
	    MemFifo ztex = new MemFifo ( bus.device(devNum) );
	    bus.unref();
	    
	    try {
	    
// reset EZ-USB
		if ( reset ) {
		    System.out.println("Reset EZ-USB");
		    ztex.resetEzUsb();
		}
		else {
		    ztex.resetDevice(false);	// sync data toggles
		}


// check for firmware
		ztex.defaultCheckVersion(1);
		ztex.debug2PrintNextLogMessages(System.out); 
		if (ztex.config == null ) throw new Exception("Invalid configuration data");
	    
// upload the bitstream 
		String configFN = ztex.config.defaultBitstreamPath("memfifo");
		System.out.println("Found " + ztex.config.getName()+",  using bitstream "+configFN);
	        System.out.println("FPGA configuration time: " + ztex.configureFpga( configFN , true, -1 ) + " ms");
	        
// upload the bitstream 
		boolean isFx3 = ztex.dev().fx3();
	        int sizeFact = isFx3 ? 2 : 1;

// claim interface
    	    	ztex.trySetConfiguration ( 1 );
    	    	ztex.claimInterface ( 0 );
		ztex.debug2PrintNextLogMessages(System.out); 
		eventHandler = new ZtexEventHandler(ztex);
		eventHandler.start();

// start bulk reader
		reader = new ZtexUsbReader( ztex, 8, 512*1024 );
		reader.start(-1);

// Mode 1: 208 MByte/s Test data generator: used for speed test
		ztex.setMode(1);
		System.out.println((isFx3 ? "208" : "48") + " MByte/s test data generator: ");
		
		long oldByteCount = 0;
		for (int i=0; i<4; i++) {
		    System.out.print("2s read only test: ");
		    try { Thread.sleep(2000); } catch ( InterruptedException e) { } 
		    System.out.println(Math.round((reader.byteCount()-oldByteCount)/2000000.0) + " MByte/s");
		    oldByteCount = reader.byteCount();
		}

		reader.cancelWait(5000);
		reader.start(0);
		long t0 = new Date().getTime();
		long l = ztex.verify(reader, 1000*sizeFact, 0, verbose, false);
		System.out.println("Read + verify data rate: " + Math.round(l/((new Date().getTime()-t0)*1000.0)) + " MByte/s");
		ztex.debug2PrintNextLogMessages(System.out); 

// Mode 2: 13 MByte/s Test data generator: used for speed test
		ztex.setMode(2);
		System.out.println((isFx3 ? "\n13" : "\n12") + " MByte/s test data generator: ");
		t0 = new Date().getTime();
		ztex.verify(reader, 500*sizeFact, 0, verbose, false);
		ztex.debug2PrintNextLogMessages(System.out);

// PKTEND tests
		if ( (ztex.defaultVersion() > 1) || (ztex.defaultSubVersion() > 2) ) {
		    System.out.println("\nPKTEND tests (may not work on all platforms)");
		    try {
			System.out.println("Manual PKTEND test (1/2):");
			l = ztex.verify(reader, 500, 0, verbose, true);
			System.out.println("Seems" + (l<500*reader.bufSize() ? "" : " not") + " to work: received " + (l >> 9) +"*512 + " + (l&511) + " bytes");
			System.out.println("Manual PKTEND test (2/2):");
			l = ztex.verify(reader, 500, 0, verbose, true);
			System.out.println("Seems" + (l<500*reader.bufSize() ? "" : " not") + " to work: received " + (l >> 9) +"*512 + " + (l&511) + " bytes"); 
			System.out.println("Automatic PKTEND assertion after 0.5s timeout test:");
   			try { Thread.sleep(1000); } catch ( InterruptedException e) { } 
			ztex.setMode(0);
			l = ztex.verify(reader, 1000, 0, verbose, false);
			System.out.println("Seems" + (l<1000*reader.bufSize() ? "" : " not") + " to work: received " + (l >> 9) +"*512 + " + (l&511) + " bytes");
			System.out.println("Zero length packet test:");
			ztex.manPktend();
			l = ztex.verify(reader, 1000, 0, verbose, false);
			System.out.println("Seems" + (l==0 ? "" : " not") + " to work: received " + l + " bytes");
		    }
		    catch ( Exception e ) {
			System.err.println("Error during PKTEND tests: "+e);
		    } 
		}
		    

// Mode 0: write+read mode
		reader.cancel();
		System.out.println("\nUSB write + read mode: on some USB implementations concurrent reading / writing may cause deadlocks or timeout errors");
		writer = new UsbTestWriter( ztex );
		ztex.reset();
//		ztex.setMode(3);
		writer.start();
		if ( !reader.cancelWait(5000) ) System.err.println("Unable to cancel transfers. Following data may be invalid");
		reader.start(0);
		
		ztex.debug2PrintNextLogMessages(System.out); 

		System.out.println("USB write + read mode: speed test");
		t0 = new Date().getTime();
		l = ztex.verify(reader, 750*sizeFact, 0, verbose, false);
		System.out.println("Read + Write data rate: " + Math.round(l/((new Date().getTime()-t0)*100.0)*0.1) + " MByte/s");
		ztex.debug2PrintNextLogMessages(System.out); 
        	
		System.out.println("USB write + read mode: 10 MByte/s read test");
		ztex.verify(reader, 300, 10000, verbose, false);
		ztex.debug2PrintNextLogMessages(System.out); 
    	    }
	    catch (Exception e) {
		System.out.println("Error: "+e.getLocalizedMessage() );
		ztex.debug2PrintNextLogMessages(System.out); 
	    } 
	    
// terminating threads and releasing resources
	    if ( reader!=null ) reader.cancel();
	    if ( (writer!=null) && (!writer.terminate(10000)) ) System.err.println("Unable to cancel writing");
	    if ( (eventHandler!=null) && !eventHandler.terminate() ) System.err.println("Unable to terminate event handler");
	    if ( (reader!=null) && (!reader.cancelWait(10000)) ) System.err.println("Unable to cancel reading");
	    ztex.dispose();  // this also releases claimed interfaces
	}
	catch (Exception e) {
	    System.out.println("Error: "+e.getLocalizedMessage() );
	} 
	
   } 
   
}
