from spartan6_fpga import spartan6_fpga
from fpga import Registers, Register_Bits, GPIO_Pins, GPIO_Select_Bits
import logging

class gpio:
    '''Controls timing controller blocks (glitch and utiming)'''

    def __init__(self):
        self.num_pins = 8
        self.settings = [GPIO_Select_Bits.VALUE_Z.value for i in range(self.num_pins)]
        self.CONTROL_REG = Registers.GPIO1_CONTROL.value
        self.SELECT_REG = Registers.GPIO1_SELECT.value
        self.clear()

    def clear(self):
        ''' Disable GPIO and clear'''
        self.disable()
        fpga = spartan6_fpga.getInstance()
        fpga.risingEdgeRegister(self.CONTROL_REG, Register_Bits.GPIO_CLEAR.value)

    def disable(self):
        ''' Disable GPIO'''
        logging.info("Disabling GPIO...")
        fpga = spartan6_fpga.getInstance()
        fpga.setBitRegister(self.CONTROL_REG, Register_Bits.GPIO_ENABLE.value, False)
        
    def setPinMux(self, pin, select):
        ''' Setup a specific pin in internal mux state
        :param pin Pin ID
        :param select Selected internal value
        '''
        if pin >= self.num_pins:
            raise ValueError("Selected pin does not exist")
        else:
            self.settings[pin] = select
        
        
    def updateMuxState(self):
        ''' Updates mux state on FPGA '''
        self.clear()
        
        fpga = spartan6_fpga.getInstance()
        
        # Write pin mux state
        pin = 7
        
        # Need to write in reverse order
        for select in reversed(self.settings):
            logging.info("Setting GPIO {} -> mode {}".format(pin, select)) 
            fpga.writeRegister(self.SELECT_REG, select)
            pin = pin - 1
            
        # Enable
        logging.info("Enabling GPIO...")
        fpga.setBitRegister(self.CONTROL_REG, Register_Bits.GPIO_ENABLE.value, True)
