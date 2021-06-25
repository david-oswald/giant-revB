from spartan6_fpga import spartan6_fpga
from fpga import Registers, Register_Bits, GPIO_Pins, GPIO_Select_Bits
from gpio import gpio
from utx import utx, ClockMode, OutputMode
from urx import urx
from utrigger import utrigger, TriggerId, TriggerMode
import logging
import time
from bitarray import bitarray
from bitarray.util import ba2int
from enum import Enum

class UartParity(Enum):
    NONE = 0
    EVEN = 1
    ODD = 2
    
class uart:
    ''' UART based on UTX and URX'''

    def __init__(self, txPin, rxPin, baudrate, nbits, parity, nstop):
        self.txPin = txPin
        self.rxPin = rxPin
        self.baudrate = baudrate
        self.nbits = nbits
        self.parity = parity
        self.nstop = nstop
        self.configure()
    
    def configure(self):
        
        # check validity
        if self.nstop < 0 or self.nstop > 2:
            raise ValueError("Stop bits must be between 0 and 2")
        
        # Setup TX/RX pin
        io = gpio()
        io.setPinMux(self.txPin, GPIO_Select_Bits.UTX_DATA_OUT.value)
        io.setPinMux(self.rxPin, GPIO_Select_Bits.URX_DATA_IN_WITH_TX.value)
        io.updateMuxState()
        logging.warning("RX not implemented yet")
        
        # Compute clock divider
        fpga = spartan6_fpga.getInstance()
        f = fpga.getFClkNormal()
        div = round(f / self.baudrate) - 1
        logging.info("Using divider " + str(div) + " for baudrate " + str(self.baudrate))
        
        # Setup Utx
        tx = utx()
        
        # Divider computed above
        tx.setClockDivider(div)
        
        tx.setTxDisconnectFromRx(True)
        
        # UART is by default high
        tx.setOutputMode(OutputMode.One.value)
        
        # Setup URX
        rx = urx()
        # TODO: rx.setStartViaUtiming(False)
        rx.setClockDivider(div)
        rx.setResyncFalling(False)
        rx.setResyncRising(False)
        rx.clear()
        
        # Trigger to start RX
        trig1 = utrigger(TriggerId.Trigger1)
        trig1.setDelay(0)
        trig1.setHoldTime(0)
        trig1.setEventMode(TriggerMode.Falling)
        trig1.setOutputMode(TriggerMode.Rising)
        
        # Trigger to stop RX (for one character)
        delay = self.getPacketLength() / float(self.baudrate)
        trig2 = utrigger(TriggerId.Trigger2)
        trig2.setDelay(delay)
        trig2.setHoldTime(0)
        trig2.setEventMode(TriggerMode.Falling)
        trig2.setOutputMode(TriggerMode.Rising)
    
    def sendValue(self, value):
        d = self.makePacket(value)
        
        tx = utx()
        tx.writeBitarray(d)
        tx.send()
    
    def waitForByte(self):
        rx = urx()
        trig1 = utrigger(TriggerId.Trigger1)
        trig2 = utrigger(TriggerId.Trigger2)
        
        trig1.arm()
        trig2.arm()
        
        while trig2.hasTriggered() == False:
            time.sleep(1e-3)
        
        pkts = rx.getPacketCount()
        p = rx.readPacket()
        logging.info("Received {} packets, got one with {} bytes".format(pkts, len(p)))
        logging.info("Data = " + ' '.join(map(bin, p)))
    
    def sendBuffer(self, buffer):
        tx = utx()
        bits = bitarray(0)
        
        for val in buffer:
             d = self.makePacket(val)
             bits.extend(d)
             
        tx.writeBitarray(bits)    
        tx.send()
    
    def getPacketLength(self):
        ''' Get number of bits in a packet '''
        nparity = 0
        
        if self.parity != UartParity.NONE:
            nparity = 1
        
        # Length in bit
        length = 1 + self.nbits + nparity + self.nstop
        
        return length
        
    def makePacket(self, value):
        ''' Prepare a single UART character packet '''
        
        # Number of parity bits
        nparity = 0
        
        if self.parity != UartParity.NONE:
            nparity = 1
        
        # Length in bit
        length = self.getPacketLength()
        
        # Assemble it
        d = bitarray('0' * length)
        
        # Copy data bits - LSBit first
        p = 0
        
        for i in range(0, self.nbits):
            b = (value >> i) & 0x1
            d[i + 1] = b
            p = p ^ b 
            
        if self.parity == UartParity.ODD:
            p = p ^ 1
        
        # Add parity bit
        if self.parity != UartParity.NONE:
            d[1 + self.nbits] = p
          
        # Stop bits must be HIGH
        for i in range(0, self.nstop):
            d[1 + self.nbits + nparity + i] = 1
        
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
         