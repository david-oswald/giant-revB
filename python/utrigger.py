from spartan6_fpga import spartan6_fpga
from fpga import Registers, Register_Bits, GPIO_Pins, GPIO_Select_Bits
import logging
import time
from enum import Enum

class TriggerMode(Enum):
    Off = 0b00
    High = 0b001
    Low = 0b010
    Rising = 0b011
    Falling = 0b100
    Change = 0b101
    
class TriggerId(Enum):
    Trigger1 = 0
    Trigger2 = 1
    
class utrigger:
    '''Controls universal trigger'''

    def __init__(self, triggerId):
        self.triggerId = triggerId
        
        regs = [
            [Registers.UTRIG1_CONTROL.value, Registers.UTRIG1_STATUS.value, 
             Registers.UTRIG1_HOLD.value, Registers.UTRIG1_DELAY.value],
            [Registers.UTRIG2_CONTROL.value, Registers.UTRIG2_STATUS.value, 
             Registers.UTRIG2_HOLD.value, Registers.UTRIG2_DELAY.value]
        ]
        
        if triggerId.value >= len(regs):
            raise ValueError("Invalid trigger ID")
            
        # Setup registers
        self.creg = regs[triggerId.value][0]
        self.sreg = regs[triggerId.value][1]
        self.hreg = regs[triggerId.value][2]
        self.dreg = regs[triggerId.value][3]
        
    def getStatus(self):
        ''' Read status reg '''
        fpga = spartan6_fpga.getInstance()
        return fpga.readRegister(self.sreg)
    
    def isArmed(self):
        ''' Check if trigger is armed '''
        status = self.getStatus()
        return status & (1 << Register_Bits.UTRIG_STATUS_ARMED.value)
    
    def hasTriggered(self):
        ''' Check if trigger has triggered '''
        status = self.getStatus()
        return status & (1 << Register_Bits.UTRIG_STATUS_TRIGGERED.value)
        
    def setOutputMode(self, mode):
        ''' Define at which level the output should stay '''
        bit2 = (mode.value >> 2) & 0x1
        bit1 = (mode.value >> 1) & 0x1
        bit0 = mode.value & 0x1
        
        fpga = spartan6_fpga.getInstance()
        fpga.setBitRegister(self.creg, Register_Bits.UTRIG_CONTROL_OUTPUT_MODE_0.value, bit0)
        fpga.setBitRegister(self.creg, Register_Bits.UTRIG_CONTROL_OUTPUT_MODE_1.value, bit1)
        fpga.setBitRegister(self.creg, Register_Bits.UTRIG_CONTROL_OUTPUT_MODE_2.value, bit2)
        
    def setEventMode(self, mode):
        ''' Define at which level the trigger should be considered asserted '''
        bit2 = (mode.value >> 2) & 0x1
        bit1 = (mode.value >> 1) & 0x1
        bit0 = mode.value & 0x1
        
        fpga = spartan6_fpga.getInstance()
        fpga.setBitRegister(self.creg, Register_Bits.UTRIG_CONTROL_INPUT_MODE_0.value, bit0)
        fpga.setBitRegister(self.creg, Register_Bits.UTRIG_CONTROL_INPUT_MODE_1.value, bit1)
        fpga.setBitRegister(self.creg, Register_Bits.UTRIG_CONTROL_INPUT_MODE_2.value, bit2)
        
    def setDelay(self, delay):
        ''' Define trigger delay '''
        fpga = spartan6_fpga.getInstance()
        d = round(fpga.getFClkNormal() * delay)
        fpga.writeRegister32(self.dreg, d)
        
    def setHoldTime(self, hold):
        ''' Define for how long the trigger is held asserted '''
        fpga = spartan6_fpga.getInstance()
        d = round(fpga.getFClkNormal() * hold)
        fpga.writeRegister32(self.hreg, d)    
        
    def softwareTrigger(self):
        ''' Trigger from software '''
        fpga = spartan6_fpga.getInstance()
        fpga.risingEdgeRegister(self.creg, Register_Bits.UTRIG_CONTROL_FORCE.value)
        
    def arm(self):
        ''' Arm the triffer '''
        fpga = spartan6_fpga.getInstance()
        fpga.risingEdgeRegister(self.creg, Register_Bits.UTRIG_CONTROL_ARM.value)
         