"""
This example shows how to glitch a Pi Pico using a crowbar (transistor) glitch

For this to work, the following connections are needed:

- TODO

Example success:
    TODO
"""

import time
import serial as ser
import uu
from glitcher import glitcher
import logging
from fpga import *
from gpio import gpio
import subprocess


def checkSuccess():
    """
    Returns True if the glitch did something
    """
    try:
        print("TODO")
    except Exception as e:
        print(e)
        pass
    return False


def main():
    """
    Iterate over the parameter search space and inject a voltage glitch.

    The uc is reset before every voltage glitch by disabling and then enabling
    the DAC.
    The trigger (start of the glitch offset period) is the DAC being enabled,
    so the glitch is triggered every time the uc starts receiving power.

    `enable DAC -> wait for the offset period -> short T1 to GND`
    """
    logging.basicConfig(level = logging.INFO)

    gl = glitcher()
    gl.resetFpga() 
    gl.dac.setTestModeEnabled(0)
    gl.dac.setRfidModeEnabled(0)
    
    io = gpio()

    # Trigger when DAC power high (uc VCC high)
    gl.dac.setTriggerEnableState(
        Register_Bits.FI_TRIGGER_CONTROL_DAC_POWER.value,
        True
    )
    
    # Offsets
    offset_start = 1050000
    offset_end = 1309000
    offset_step = 1000
    
    # Width range
    w_start = 100
    w_end = 250
    w_step = 10
    
    # Repeat each attempt how many times?
    repeat = 2
    
    # Loop state
    run = True
    w = w_start
    offset = offset_start
    r = 0

    # Both fault voltage and normal voltage are 3.3V.
    # We want the fault on T1, not on DAC power.
    gl.setVoltages(3.3, 3.3, 0)
    gl.dac.setEnabled(False)

    while run:
        print("Attempting to glitch...")
        print("w = {:d}, o = {:d}, repeat = {:d}".format(w, offset, r))
        # Turn off uc
        gl.dac.setEnabled(False)
        time.sleep(0.05)
        
        # Clean up any previous pulses
        gl.dac.clearPulses()
    
        # Set a pulse
        gl.addPulse(offset, w)
        
        # Arm the fault
        gl.dac.arm()
        
        # Bring uc out of reset. This will also trigger the glitch
        gl.dac.setEnabled(True)
        time.sleep(0.05)
        
        # Now check if we succeeded
        success = checkSuccess()
        
        if not success:
            print("Not glitched! Next parameter.")
            
            # Next param
            if r < repeat:
                r = r + 1
            elif w < w_end:
                r = 0
                w = w + w_step
            elif offset < offset_end:
                w = w_start
                offset = offset + offset_step
            else:
                run = False
        else:
            print("Success!")
            print("w = {:d}, o = {:d}".format(w, offset))
            run = False
    
    gl.close()


if __name__=="__main__":
    main()
