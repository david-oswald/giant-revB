# dac.py is a python port of dac.[h,cpp] to control the glitching part of the GIAnT
from spartan6_fpga import spartan6_fpga
from fpga import Registers, Register_Bits
import logging
from timing_controller import *


class dac(timing_controller):
    '''Controls glitching logic'''

    def __init__(self):
        super().__init__()
        self.ADDR_H = Registers.FI_ADDR_H.value
        self.ADDR_L = Registers.FI_ADDR_L.value
        self.DATA_IN = Registers.FI_DATA_IN.value
        self.DATA_OUT = Registers.FI_DATA_OUT.value
        self.CONTROL_REG = Registers.FI_CONTROL.value
        self.STATUS_REG = Registers.FI_STATUS.value
        self.W_EN_BIT = Register_Bits.FI_CONTROL_W_EN.value
        self.ARM_BIT = Register_Bits.FI_CONTROL_ARM.value
        self.TRIGGER_BIT = Register_Bits.FI_CONTROL_TRIGGER.value

    def setFaultVoltage(self, v):
        ''' Set fault voltage
        :param v Value to set
        '''
        fpga = spartan6_fpga.getInstance()
        fpga.writeRegister(Registers.DAC_V_HIGH.value, v)

    def setNormalVoltage(self, v):
        ''' Set normal voltage
        :param v Value to set
        '''
        fpga = spartan6_fpga.getInstance()
        fpga.writeRegister(Registers.DAC_V_LOW.value, v)

    def setOffVoltage(self, v):
        ''' Set off (inactive) voltage
        :param v Value to set
        '''
        fpga = spartan6_fpga.getInstance()
        fpga.writeRegister(Registers.DAC_V_OFF.value, v)

    def setTriggerEnableState(self, src, state):
        ''' Enable/disable specific trigger source
        :param src Number in trigger control register, e.g., FI_TRIGGER_CONTROL_EXT1
        :param state true to enable, false to disable source
        '''
        fpga = spartan6_fpga.getInstance()
        fpga.setBitRegister(Registers.FI_TRIGGER_CONTROL.value, src, state)

    def setUniversalTriggerEnableState(self, src, state):
        ''' Enable/disable specific universal trigger source
        :param src Number in trigger control register, e.g., FI_UNIVERSAL_TRIGGER_CONTROL_UTX_START
        :param state true to enable, false to disable source
        '''
        fpga = spartan6_fpga.getInstance()
        fpga.setBitRegister(Registers.FI_UNIVERSAL_TRIGGER_CONTROL.value, src, state)

    def setTriggerOnFallingEdge(self, state):
        ''' Enable/disable trigger on falling edge
        :param state true to enable, false to disable
        '''
        fpga = spartan6_fpga.getInstance()
        fpga.setBitRegister(Registers.FI_TRIGGER_CONTROL.value, Register_Bits.FI_TRIGGER_CONTROL_INVERT_EDGE.value, state)

    def setEnabled(self, on):
        ''' Set state of DAC
        :param on True to enable DAC, otherwise false
        '''
        fpga = spartan6_fpga.getInstance()
        fpga.setBitRegister(Registers.DAC_CONTROL.value, Register_Bits.DAC_ENABLE.value, on)

    def setTestModeEnabled(self, on):
        ''' Set test mode state of DAC
        :param on True to enable test mode, otherwise false
        '''
        fpga = spartan6_fpga.getInstance()
        fpga.setBitRegister(Registers.DAC_CONTROL.value, Register_Bits.DAC_TEST_MODE.value, on)

    def setRfidModeEnabled(self, on):
        ''' Control RFID mode of DAC, i.e. DAC will be driven by modulated-sine generator output
        :param on True to enable RFID mode, otherwise false
        :note Will disable UTX mode on enable
        '''
        fpga = spartan6_fpga.getInstance()
        if on:
            self.setUtxModeEnabled(0)

        fpga.setBitRegister(Registers.DAC_CONTROL.value, Register_Bits.DAC_RFID_MODE.value, on)

    def setUtxModeEnabled(self, on):
        ''' Control UTX mode of DAC 
        In this mode, DAC will be switched to off by zero from UTX and left at normal voltage otherwise. Useful e.g. for driving tristate busses + pulse
        :param on True to enable UTX mode, otherwise false
        :note Will disable RFID mode on enable
        '''
        fpga = spartan6_fpga.getInstance()
        if on:
            self.setRfidModeEnabled(0)

        fpga.setBitRegister(Registers.DAC_CONTROL.value, Register_Bits.DAC_UTX_MODE.value, on)
