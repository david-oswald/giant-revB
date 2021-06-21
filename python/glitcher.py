from spartan6_fpga import spartan6_fpga
from fpga import *
from dac import dac
from gpio import gpio
from uart import uart, UartParity
from utx import utx, ClockMode, OutputMode
import logging
import time
import traceback


class glitcher:
    '''Generic class that interfaces with the GIAnT HW'''

    
    def __init__(self, trigger_falling_edge = 1, v_step = -8):
        '''Initialise the GIAnT HW
            :trigger_falling_edge: Sets the trigger on falling edge (1) or rising edge (0)
            :v_step: Voltage step - specify the range on the GIAnT hardware. Here +4V to -4V
        '''
        # create dac object
        self.dac = dac()
        # init singleton instance
        self.fpga = spartan6_fpga.getInstance()

        # Initialise FPGA
        self.reset_fpga()

        self.dac.setTestModeEnabled(0)
        self.dac.setRfidModeEnabled(0)

        self.dac.clearPulses()
        self.dac.setTriggerEnableState(Register_Bits.FI_TRIGGER_CONTROL_EXT1.value, 0)

        # Default trigger on falling edge - set this to zero
        self.dac.setTriggerOnFallingEdge(1)
        # Set the trigger to external
        self.dac.setTriggerEnableState(Register_Bits.FI_TRIGGER_CONTROL_EXT1.value, 1)

        self.V_STEP = v_step/256


    def calc_voltage(self, v):
        '''Calculates the value to write to the GIAnT for the voltage'''
        return int(v/self.V_STEP + 128)

    def set_voltages(self, f_voltage, norm_voltage, off_voltage):
        '''Write the voltages to the GIAnT 

        :param f_voltage: the fault voltage (in V - e.g. 1.8V)
        :param norm_voltage: the normal voltage (e.g. 3.3V)
        :param off_voltage: the off voltage (e.g. 0V)
        '''
        v_normal = self.calc_voltage(norm_voltage)
        v_off = self.calc_voltage(off_voltage)
        v_fault = self.calc_voltage(f_voltage)

        # Set voltages
        self.dac.setFaultVoltage(v_fault)
        self.dac.setNormalVoltage(v_normal)
        self.dac.setOffVoltage(v_off)

    def set_f_voltage(self, f_voltage):
        '''Calculates and sets only the fault voltage'''
        v_fault = self.calc_voltage(f_voltage)
        self.dac.setFaultVoltage(v_fault)

    def add_pulse(self, offset, width, overwrite = 0):
        '''Sets the width and offset of a pulse - given in nanosecond
        
        :param width: width of the pulse (in nanosecond)
        :param offset: offset from the trigger (in nanosecond)
        '''

        # Set a pulse
        self.dac.addPulse(offset, width, overwrite)

    def close(self):
        return self.fpga.close()
           
    def test_mode(self):
        '''
            Enables the test mode on the GIAnT - Voltage goes from highest possible to lowest possible
        '''
        self.reset_fpga()
        self.dac.setTestModeEnabled(1)

    def reset_fpga(self):
        '''
            Resets the FPGA connection
        '''
        # Open USB connection
        self.fpga.open()
        self.fpga.resetFpga()

        # Enable the DAC 
        self.dac.setEnabled(1)


    def test_gpio(self):
        io = gpio()
        
        # Mux a constant 1 on GPIO1_6
        io.setPinMux(GPIO_Pins.GPIO6.value, GPIO_Select_Bits.VALUE_1.value)
        io.updateMuxState()
        time.sleep(1)
        
        # Mux the internal output 0 to the same pin and set it to zero
        io.setInternalOutput(GPIO_Pins.GPIO0.value, False);
        io.setPinMux(GPIO_Pins.GPIO6.value, GPIO_Select_Bits.GPIO_OUTPUT_0.value)
        io.updateMuxState()
        time.sleep(1)
        
        # Now set it to one
        #io.setInternalOutput(GPIO_Pins.GPIO0.value, True);
        #time.sleep(2)
        #io.setInternalOutput(GPIO_Pins.GPIO0.value, False);
        
    def test_fi(self):
        '''
            tests the Fault Injection on the FPGA. Adds two pulses, sets a software trigger and arms the FPGA
        '''
        #self.reset_fpga()
        self.dac.setEnabled(True)
        self.dac.setTestModeEnabled(0)
        self.dac.setRfidModeEnabled(0)
        
        # Mux the internal output 0 to the same pin and set it to zero
        io = gpio()
        io.setInternalOutput(GPIO_Pins.GPIO0.value, False);
        io.setPinMux(GPIO_Pins.GPIO6.value, GPIO_Select_Bits.GPIO_OUTPUT_0.value)
        io.updateMuxState()

        # Setup trigger on this internal output (which is also on GPIO1_6)
        self.dac.setTriggerEnableState(Register_Bits.FI_TRIGGER_GPIO_OUTPUT_0.value, True)

        # Set the fault voltage, normal voltage, off voltage
        self.set_voltages(0, 3.3, 0)

        # Clean up any previous pulses
        self.dac.clearPulses()

        # Set a pulse
        self.add_pulse(200000, 50)
        self.add_pulse(1000, 50)
        
        # Arm the fault
        self.dac.arm()

        # This should trigger
        io.setInternalOutput(GPIO_Pins.GPIO0.value, True);

    def test_utx(self):
        
        io = gpio()
        tx = utx()
        
        # First shortly a high level
        io.setPinMux(GPIO_Pins.GPIO6.value, GPIO_Select_Bits.VALUE_1.value)
        io.updateMuxState()
        time.sleep(1)
        
        # Then mux UTX out to pin 5
        io.setPinMux(GPIO_Pins.GPIO6.value, GPIO_Select_Bits.UTX_DATA_OUT.value)
        io.updateMuxState()
        time.sleep(1)
        
        # Then push some data into UTX
        data = [0x12, 0xff, 0x00, 0xAA, 0xaa, 0xaa, 0xaf]
        tx.setClockDivider(9)
        tx.setOutputMode(OutputMode.Zero.value)
        tx.writeBuffer(data, 0)
        tx.send()

    def test_uart(self):
        
        baudrate = 115200
        s = uart(GPIO_Pins.GPIO6.value, GPIO_Pins.GPIO5.value, baudrate, 
                 8, UartParity.NONE, 1)
        
        time.sleep(1)
        
        s.sendBuffer([0xBB, 0xCC, 0xAA])
        
    def clear_pulses(self):
        '''Clears all pulses in the dac'''
        self.dac.clearPulses()

    def arm(self):
        '''
            Arm the dac - will insert a glitch when trigger pin goes high
        '''
        self.dac.arm()




if __name__ == "__main__":
    logging.basicConfig(level = logging.INFO)
    glitcher = glitcher()

    try:
        glitcher.reset_fpga()
        # glitcher.test_gpio()
        # glitcher.test_utx()
        glitcher.test_uart()
        # glitcher.test_fi()
    except Exception as e:
        logging.error(traceback.format_exc())
        
    glitcher.close()
