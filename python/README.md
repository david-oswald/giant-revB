This directory contains a Python 3 port of the original [GIAnT](sourceforge.net/projects/giant) project. It uses an inexpensive Ztex FPGA an open-source hardware to facilitate voltage glitching. 

# Usage

## Linux

In order to make the Ztex FPGA available for any user and not just root, copy the [rules](./99-ztex-rules.d) file over to the udev rules directory (/etc/udev/rules.d/). Any user in the group `usb` can access the USB device. 

## Windows

Use Zadig to change the driver to WinUSB. Python needs `pyusb`, I verified this to work when using Python 3.8 under Anaconda.

## Programming FPGA / micro

Run the [dl\_firm\_series2\_current.sh](fpga/dl_firm_series2_current.sh) script to download the current v2 GIAnT firmware to the FPGA. Once finished, the GIAnT should be ready to communicate with our python code.

This requires a working Java installation.

## Example code
The [glitcher](glitcher.py) class provides most necessary functionality to start voltage glitching. The below example creates an instance and arms the GIAnT.

```
# Create a glitcher object which will interface with the HW
gl = glitcher(trigger_falling_edge = 1, v_step = -8) # Create glitcher object which triggers on falling edge, ranging from +4V to -4V

gl.set_voltages(0, 3.3, 0) # Set Vf, Vdd, Voff

gl.add_pulse(50e3, 100) # Add 100ns pulse 50mus after trigger (unit here is nanoseconds)

gl.arm() # Arm the HW

# ... Execute reset / serial command / ... 

gl.add_pulse(100, 0.2, overwrite = 1) # Overwrite original pulse 

```

### LPC 1343

The LPC1343 bootloader exploit (https://toothless.co/blog/bootloader-bypass-part1/) is implemented in [this file](example_lpc1343.py). 

The example uses the LPC-P1343 from Olimex (https://www.olimex.com/Products/ARM/NXP/LPC-P1343/). 
For this to work, the following connections and changes are needed:

 * cut the 3.3V_CORE and 3.3V_IO jumpers, connect the chip side with a wire,
  and then connect them to the glitch output
 
 * remove C1 and C4 (decoupling caps)
 
 * close the BLD_E jumper and connect P0_2 to GND to enter the bootloader
 
 * connect the bootloader RX/TX pins (on UEXT) to a USB-to-serial adapter 
  (e.g. a CP2101-based one)
 
 * connect 3V3 and GND from the USB-to-serial to the respective pins on the
  UEXT connector
 
 * connect the RST pin to the GIAnT (I use IO1 pin 6 here)

Example success:

```
v = 0.500000, w = 260, o = 52900, repeat = 1
!!! Yeah, unprotected !!!
'R 0 4\r0\r\n$_!\\`$!`0\r\n299\r\n'
WOOT!
v = 0.500000, w = 260, o = 52900
```

### nRF52832

The APPROTECT bypass exploit (https://limitedresults.com/2021/03/the-pocketglitcher/ and https://github.com/pd0wm/airtag-dump) is implemented in [example_nrf52832.py](https://github.com/david-oswald/giant-revB/blob/nrf52832/python/example_nrf52832.py).
With an adjusted offset, it is also likely to work on other nRF52 microcontrollers.

The following connections are needed:

- Solder a wire to DEC1 on the nRF and connect it to T1 on the GIAnT
  (T1 shorts to ground when the fault is injected).
- It may be necessary to remove the capacitor on DEC1.
- Connect VCC on the nRF to DAC output on the GIAnT, and GND to GND.
- Connect SWDIO and SWDCLK on the nRF to those pins on an ST-LINK or
  similar.

Example success:

```
Attempting to glitch...
w = 180, o = 1071000, repeat = 1
Protected! Next parameter.
Attempting to glitch...
w = 180, o = 1071000, repeat = 2
Success!
w = 180, o = 1071000
```

### Further planned examples

 * General built-in support for at least UART and SWD (for SWD see here: https://research.kudelskisecurity.com/2019/07/31/swd-part-2-the-mem-ap/)
 * ESP32: https://limitedresults.com/2019/09/pwn-the-esp32-secure-boot/
 * STM32F0: https://www.usenix.org/system/files/conference/woot17/woot17-paper-obermaier.pdf
 * MSP430 (MSP430F5172): https://tches.iacr.org/index.php/TCHES/article/download/7390/6562/
 * STM32F1/F3: https://tches.iacr.org/index.php/TCHES/article/download/7390/6562/
 * STM32F2: https://chip.fail/chipfail.pdf
 * STM32F4: https://www.synacktiv.com/sites/default/files/2020-11/presentation.pdf 
 * 9S12C: https://wikilab.myhumankit.org/images/7/77/ChipHackingV07B.pdf
 * STM8: https://github.com/janvdherrewegen/bootl-attacks/tree/main/stm8 and https://itooktheredpill.irgendwo.org/2020/stm8-readout-protection/
 * 78K0: https://github.com/janvdherrewegen/bootl-attacks/tree/main/78k0
 * PIC18F partial overwrite attack: https://get.meriac.com/docs/HID-iCLASS-security.pdf (can use FPGA built-in PIC programmer)
 * LPC1343 stack overwrite (smart detection logic in which CRP)
