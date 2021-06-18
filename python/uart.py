from spartan6_fpga import spartan6_fpga
from fpga import Registers, Register_Bits, GPIO_Pins, GPIO_Select_Bits
from gpio import gpio
from utx import utx, ClockMode, OutputMode
import logging
import time
from bitarray import bitarray
from bitarray.util import ba2int
    
class uart:
    ''' UART based on UTX and URX'''

    def __init__(self, txPin, rxPin, baudrate, nbits, nparity, nstop):
        self.txPin = txPin
        self.rxPin = rxPin
        self.baudrate = baudrate
        self.nbits = nbits
        self.nparity = nparity
        self.nstop = nstop
        self.configure()
    
    def configure(self):
        
        # Setup TX/RX pin
        io = gpio()
        io.setPinMux(self.txPin, GPIO_Select_Bits.UTX_DATA_OUT.value)
        io.updateMuxState()
        logging.warning("RX not implemented yet")
        
        # Compute clock divider
        div = round(50e6 / self.baudrate) - 1
        logging.info("Using divider " + str(div) + " for baudrate " + str(self.baudrate))
        
        # Setup Utx
        tx = utx()
        
        # Divider computed above
        tx.setClockDivider(div)
        
        # UART is by default high
        tx.setOutputMode(OutputMode.One.value)
    
    def sendChar(self, value):
        d = self.makePacket(value)
        
        tx = utx()
        tx.writeBitarray(d)
        tx.send()
    
    def makePacket(self, value):
        ''' Prepare a single UART character packet '''
        
        # Length in bit
        length = 1 + self.nbits + self.nparity + self.nstop
        
        # Assemble it
        d = bitarray('0' * length)
        
        # Copy data bits - LSBit first
        parity = 0
        
        for i in range(0, self.nbits):
            b = (value >> i) & 0x1
            d[i + 1] = b
            parity = parity ^ b 
            
        if self.nparity != 0:
            d[1 + self.nbits] = parity
          
        return d 
        
    def setClockMode(self, mode):
        raise RuntimeError("setClockMode() is not implemented yet")
        
    def setOutputMode(self, mode):
        ''' Define at which level the output should stay '''
        bit1 = (mode >> 1) & 0x1
        bit0 = mode & 0x1
        
        fpga = spartan6_fpga.getInstance()
        fpga.setBitRegister(Registers.UTX_CONTROL.value, Register_Bits.UTX_CONTROL_OUT_MODE_0.value, bit0)
        fpga.setBitRegister(Registers.UTX_CONTROL.value, Register_Bits.UTX_CONTROL_OUT_MODE_1.value, bit1)
         