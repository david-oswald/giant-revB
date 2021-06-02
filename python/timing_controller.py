from spartan6_fpga import spartan6_fpga
import logging

class timing_controller:
    '''Controls timing controller blocks (glitch and utiming)'''

    def __init__(self):
        self.pulses = []
        self.ADDR_H = 0
        self.ADDR_L = 0
        self.DATA_IN = 0
        self.DATA_OUT = 0
        self.CONTROL_REG = 0
        self.STATUS_REG = 0
        self.W_EN_BIT = 0
        self.ARM_BIT = 0
        self.TRIGGER_BIT = 0


    def addPulse(self, offset, width, overwrite = 0):
        ''' Add a pulse to the list of pulses to generate
        :param offset Offset in ns with respect to previous pulse/trigger
        :param width Width in ns
        '''
        logging.debug("Adding pulse at {} μs: width {} μs".format(offset, width))
        fpga = spartan6_fpga.getInstance()
        
        # Cast the results of these since they are floats
        offset_p = int(fpga.getNsToPoint() * offset)
        width_p = int(fpga.getNsToPoint() * width)

        min_offset = 3
        min_width = 1

        if offset_p <= min_offset:
            logging.warning("Requested delay shorter than minimum, truncating to minimum")
            offset_p = min_offset + 1

        offset_p -= min_offset

        if width_p < min_width:
            logging.warning("Requested width shorter than minimum, truncating to minimum")
            width_p = min_width

        width_p -= min_width

        # add to the list if not overwriting
        if not overwrite:
            self.pulses.append((offset_p, width_p))
        else:
            self.pulses[-1] = (offset_p, width_p)

        # overwrite existing config memory
        mem_end = 2 * len(self.pulses) + 2
        fi_config = mem_end << 16 | 0x0
        self.writeMemory32(0, fi_config)

        # overwrite pulse memory
        for p in range(len(self.pulses)):
            # offset
            self.writeMemory32(p*2+2, self.pulses[p][0])
            # width
            self.writeMemory32(p*2+2+1, self.pulses[p][1])


    def clearPulses(self):
        '''
        Clear pulse memory
        '''
        # clear list
        self.pulses.clear()

        # overwrite existing config memory
        mem_end = 2*len(self.pulses) + 2
        fi_config = mem_end << 16 | 0x0
        self.writeMemory32(0, fi_config)
    

    def writeMemory8(self, addr, v):
        ''' Write 8-bit directly to FI memory
        :param addr 14-bit memory address
        :param v 8-bit value to write
        '''
        fpga = spartan6_fpga.getInstance()

        # set address
        fpga.writeRegister(self.ADDR_L, addr & 0xff)
        fpga.writeRegister(self.ADDR_H, (addr >> 8) & 0x7)

        # set data to write
        fpga.writeRegister(self.DATA_IN, v)

        #write data
        fpga.risingEdgeRegister(self.CONTROL_REG, self.W_EN_BIT)
        

    def readMemory8(self, addr):
        ''' Read 8-bit directly from FI memory
        :param addr 14-bit memory address to read
        :return Value at addr
        '''
        fpga = spartan6_fpga.getInstance()

        # set address
        fpga.writeRegister(self.ADDR_L, addr & 0xff)
        fpga.writeRegister(self.ADDR_H, (addr >> 8) & 0x7)

        # get data
        return fpga.readRegister(self.DATA_OUT)
         

    def writeMemory32(self, addr, v):
        ''' Write 32-bit directly to FI memory
        :param addr 12-bit memory address (adressing in 32-bit steps)
        :param v 32-bit value to write
        '''
        for b in range(4):
            self.writeMemory8(4*addr + b, (v >> (8*b)) & 0xff)

    def arm(self):
        ''' Arm trigger '''
        fpga = spartan6_fpga.getInstance()
        fpga.risingEdgeRegister(self.CONTROL_REG, self.ARM_BIT)

    def softwareTrigger(self):
        ''' Force (software) trigger '''
        fpga = spartan6_fpga.getInstance()
        fpga.risingEdgeRegister(self.CONTROL_REG, self.TRIGGER_BIT)

    def getStatus(self):
        ''' Get status
        :return 8-bit status word
        '''
        fpga = spartan6_fpga.getInstance()
        return fpga.readRegister(self.STATUS_REG)