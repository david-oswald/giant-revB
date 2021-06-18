from spartan6_fpga import spartan6_fpga
from fpga import Registers, Register_Bits, GPIO_Pins, GPIO_Select_Bits
import logging
import time
from enum import Enum

# WARNING: This is not yet supported FPGA-side
class ClockMode(Enum):
    Default =    0b00
    Shifted90 =  0b01
    Shifted180 = 0b10
    Shifted270 = 0b11
    
class OutputMode(Enum):
    Zero      = 0b00
    One       = 0b01
    Tristate  = 0b10
    LastValue = 0b11
    
class utx:
    '''Controls universal TX'''

    def __init__(self):
        self.clear()
        self.packetSizes = []
        
    def clear(self):
        ''' Clear UTX '''
        fpga = spartan6_fpga.getInstance()
        fpga.risingEdgeRegister(Registers.UTX_CONTROL.value, Register_Bits.UTX_CONTROL_CLEAR.value)
        
    def stop(self):
        ''' Clear UTX '''
        fpga = spartan6_fpga.getInstance()
        fpga.risingEdgeRegister(Registers.UTX_CONTROL.value, Register_Bits.UTX_CONTROL_STOP.value)
    
    def getStatus(self):
        ''' Read status reg '''
        fpga = spartan6_fpga.getInstance()
        return fpga.readRegister(Registers.UTX_STATUS.value)
    
    def getPacketCount(self):
        ''' Read status reg '''
        fpga = spartan6_fpga.getInstance()
        return fpga.readRegister(Registers.UTX_PACKET_COUNT.value)
    
    def setOneAsTristate(self, state):
        ''' Whether a one is a hard one or Z '''
        ''' WARNING: This might not work correctly yet '''
        logging.warn("setOneAsTristate() might not work yet")
        fpga = spartan6_fpga.getInstance()
        fpga.setBitRegister(Registers.UTX_CONTROL.value, Register_Bits.UTX_CONTROL_ONE_TO_Z.value, state)
       
    def setTxDisconnectFromRx(self, state):
        ''' Whether TX is internally disconnected from URX '''
        logging.warn("setTxDisconnectFromRx() is untested, be careful")
        fpga = spartan6_fpga.getInstance()
        fpga.setBitRegister(Registers.UTX_CONTROL.value, Register_Bits.UTX_CONTROL_DISCONNECT_TX_URX.value, state)
        
    def setStartViaUtiming(self, state):
        ''' Whether TX can be started from Utiming (for fixed timing stuff) '''
        logging.warn("setStartViaUtiming() is untested, be careful")
        fpga = spartan6_fpga.getInstance()
        fpga.setBitRegister(Registers.UTX_CONTROL.value, Register_Bits.UTX_CONTROL_ENABLE_UTIMING.value, state)
        
    def setClockDivider(self, div):
        ''' Set clock divider '''
        if div < 1 or div > 65536:
            raise ValueError("Value out of range [1, 65536]")
        
        fpga = spartan6_fpga.getInstance()
        fpga.writeRegister16(Registers.UTX_CLKDIV.value, div)
    
    def write(self, byte):
        ''' Write a single byte to internal FIFO '''
        fpga = spartan6_fpga.getInstance()
        fpga.writeRegister(Registers.UTX_DATA_IN.value, byte & 0xff)
        
    def isIdle(self):
        ''' Check if UTX is idle '''
        status = self.getStatus()
        return status & (1 << Register_Bits.UTX_STATUS_READY.value)
    
    def waitReady(self):
        ''' Wait until UTX no longer busy '''
        while self.isIdle() == False:
            time.sleep(1e-3)
            
        return True
    
    
    def writeBitarray(self, bits):
        valid = len(bits)
        logging.info("Will write " + str(valid) + " bits")
        b = bytearray(bits.tobytes())
        self.writeBuffer(b, valid)
        
    def writeBuffer(self, data, valid):
        ''' Push a whole buffer into UTX '''
        
        # Wait for UTX to be ready
        if not self.waitReady():
            raise IOError("UTX timed out")
            
        # Push data
        for val in data:
            self.write(val)
            
        # If a valid number of bits is specified, use that, else full buffer size
        if valid > 0:
            self.packetSizes.append(valid)
        else:
            self.packetSizes.append(len(data) * 8)
        
    def writeSizes(self):
        ''' Write internal size buffers to FPGA '''
        fpga = spartan6_fpga.getInstance()
        
        
        # Write in reverse order
        for val in reversed(self.packetSizes):
            high = (val >> 8) & 0xff
            low = val & 0xff
            fpga.writeRegister(Registers.UTX_PACKETSIZE_LO.value, low)
            fpga.writeRegister(Registers.UTX_PACKETSIZE_HI.value, high)
       
    def send(self):
        ''' Push data packets out '''
        fpga = spartan6_fpga.getInstance()
        
        # Push over the size info
        self.writeSizes()
        
        # Trigger data sending
        fpga.risingEdgeRegister(Registers.UTX_CONTROL.value, Register_Bits.UTX_CONTROL_START.value)
        
    def setClockMode(self, mode):
        raise RuntimeError("setClockMode() is not implemented yet")
        
    def setOutputMode(self, mode):
        ''' Define at which level the output should stay '''
        bit1 = (mode >> 1) & 0x1
        bit0 = mode & 0x1
        
        fpga = spartan6_fpga.getInstance()
        fpga.setBitRegister(Registers.UTX_CONTROL.value, Register_Bits.UTX_CONTROL_OUT_MODE_0.value, bit0)
        fpga.setBitRegister(Registers.UTX_CONTROL.value, Register_Bits.UTX_CONTROL_OUT_MODE_1.value, bit1)
         