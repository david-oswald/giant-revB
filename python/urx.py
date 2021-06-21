from spartan6_fpga import spartan6_fpga
from fpga import Registers, Register_Bits, GPIO_Pins, GPIO_Select_Bits
import logging
import time
import math
from enum import Enum
    
class urx:
    '''Controls universal RX'''

    def __init__(self):
        self.clear()
        self.packetSizes = []
        
    def clear(self):
        ''' Clear URX '''
        fpga = spartan6_fpga.getInstance()
        fpga.risingEdgeRegister(Registers.URX_CONTROL.value, Register_Bits.URX_CONTROL_CLEAR.value)
        
    def forceStop(self):
        ''' Stop URX '''
        fpga = spartan6_fpga.getInstance()
        fpga.risingEdgeRegister(Registers.URX_CONTROL.value, Register_Bits.URX_CONTROL_STOP.value)
        
    def forceStart(self):
        ''' Start URX '''
        fpga = spartan6_fpga.getInstance()
        fpga.risingEdgeRegister(Registers.URX_CONTROL.value, Register_Bits.URX_CONTROL_START.value)
    
    def getStatus(self):
        ''' Read status reg '''
        fpga = spartan6_fpga.getInstance()
        return fpga.readRegister(Registers.URX_STATUS.value)
    
    def getPacketCount(self):
        ''' Read status reg '''
        fpga = spartan6_fpga.getInstance()
        return fpga.readRegister(Registers.URX_PACKET_COUNT.value)
    
    def setStartViaUtiming(self, state):
        ''' Whether RX can be started from Utiming (or UTX) '''
        raise RuntimeError("setStartViaUtiming() is not implemented FPGA-side yet")
        #fpga = spartan6_fpga.getInstance()
        #fpga.setBitRegister(Registers.URX_CONTROL.value, Register_Bits.URX_CONTROL_UTIMING_OR_UTX.value, state)
        
    def setClockDivider(self, div):
        ''' Set clock divider '''
        if div < 1 or div > 65536:
            raise ValueError("Value out of range [1, 65536]")
        
        fpga = spartan6_fpga.getInstance()
        fpga.writeRegister16(Registers.URX_CLKDIV.value, div)
    
    def isIdle(self):
        ''' Check if URX is idle '''
        status = self.getStatus()
        return status & (1 << Register_Bits.URX_STATUS_READY.value)
    
    def waitReady(self):
        ''' Wait until URX no longer busy '''
        while self.isIdle() == False:
            time.sleep(1e-3)
            
        return True
    
    def readByte(self):
        ''' Read a single byte '''
        fpga = spartan6_fpga.getInstance()
        return fpga.readRegister(Registers.URX_DATA_OUT.value)
    
    def setResyncFalling(self, state):
        ''' Whether RX should resync on falling edges '''
        fpga = spartan6_fpga.getInstance()
        fpga.setBitRegister(Registers.URX_CONTROL.value, Register_Bits.URX_CONTROL_RESYNC_TO_FALLING_EDGES.value, state)
        
    def setResyncRising(self, state):
        ''' Whether RX should resync on rising edges '''
        fpga = spartan6_fpga.getInstance()
        fpga.setBitRegister(Registers.URX_CONTROL.value, Register_Bits.URX_CONTROL_RESYNC_TO_RISING_EDGES.value, state)
        
    def setResyncEdges(self, state):
        ''' Whether RX should resync on all edges '''
        fpga = spartan6_fpga.getInstance()
        fpga.setBitRegister(Registers.URX_CONTROL.value, Register_Bits.URX_CONTROL_RESYNC_ON_EDGE.value, state)
        
    def readPacket(self):
        ''' Read a full packet from URX '''
        packets = self.getPacketCount()
        logging.info("{} packets in URX".format(packets))
        
        fpga = spartan6_fpga.getInstance()
        bits = fpga.readRegister(Registers.URX_PACKET_SIZE_OUT_LOW.value)
        bits = bits | (fpga.readRegister(Registers.URX_PACKET_SIZE_OUT_HIGH.value) << 8)
        
        byteCnt = math.ceil(bits / 8)
        
        logging.info("{} bits = {} bytes".format(bits, byteCnt))
        
        result = []
        
        for i in range(0, byteCnt):
            result.append(self.readByte())
        
        return result
        