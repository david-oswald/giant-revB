# This file contains register definitions and constants for the FPGA
from enum import Enum


class Commands(Enum):
    FPGA_READ_REGISTER = 0x01
    FPGA_WRITE_REGISTER = 0x02
    FPGA_RESET = 0x03

class Registers(Enum):

    ## Read-only registers
    
    # Smartcard
    SC_STATUS = 3
    SC_DATA_OUT = 4
    SC_DATA_OUT_COUNT = 9
    SC_DATA_IN_COUNT = 10

    # PIC programmer
    PIC_DATA_OUT_L = 5
    PIC_DATA_OUT_H = 6
    
    # FI pulse generator
    FI_STATUS = 7
    FI_DATA_OUT = 8
    
    # DDR (currently disabled)
    DDR_DMA_IN_L = 11
    DDR_SINGLE_READ = 17
    DDR_STATUS = 18
    DDR_DMA_IN_H = 19
    
    # Threshold trigger (currently disabled)
    THRESHOLD_STATUS = 12
    
    # Utiming
    UTIMING_STATUS = 13
    
    # UTX
    UTX_PACKET_COUNT = 14
    UTX_STATUS = 22
    
    # URX
    URX_STATUS = 23,        
    URX_DATA_OUT = 24,  
    URX_PACKET_SIZE_OUT_LOW = 25 
    URX_PACKET_COUNT = 26
    URX_PACKET_SIZE_OUT_HIGH = 29
    
    # Utrig
    UTRIG1_STATUS = 27
    UTRIG2_STATUS = 28
    
    ## Read-write registers
    
    # Smartcard
    SC_CONTROL = 34
    SC_DATA_IN = 35
    
    # PIC programmer
    PIC_CONTROL = 36
    PIC_COMMAND = 37
    PIC_DATA_IN_L = 38
    PIC_DATA_IN_H = 39
    
    # DAC
    DAC_V_LOW = 40
    DAC_V_HIGH = 41
    DAC_V_OFF = 46
    DAC_CONTROL = 48

    # FI pulse generator
    FI_CONTROL = 42
    FI_DATA_IN = 43
    FI_ADDR_L = 44
    FI_ADDR_H = 45
    FI_TRIGGER_CONTROL = 47
    FI_UNIVERSAL_TRIGGER_CONTROL = 81
   
    # Threshold trigger (currently disabled)
    THRESHOLD_CONTROL = 49
    THRESHOLD_VALUE = 61
    
    # Utiming
    UTIMING_CONTROL = 50
    UTIMING_DATA_IN = 51
    UTIMING_ADDR_L = 52
    UTIMING_ADDR_H = 53
    
    # RFID (currently disabled)
    RFID_RESET_TIME = 54
    RFID_CONTROL = 55
    
    # DDR (currently disabled)
    DDR_CONTROL = 56
    DDR_SINGLE_WRITE = 57
    DDR_ADDRESS = 58
    DDR_DATA_COUNT = 59
    
    # Pattern trigger (currently disabled)
    DETECTOR_PATTERN = 60
    DETECTOR_DEBUG = 62
    DETECTOR_PATTERN_SAMPLE_COUNT = 63
    
    # ADC (currently disabled)
    ADC_CONTROL = 64
    
    # Downsampling (currently disabled)
    SCOPE_DOWNSAMPLING_FACTOR = 65
    DETECTOR_DOWNSAMPLING_FACTOR = 66
    
    # UTX
    UTX_CONTROL = 67
    UTX_DATA_IN = 68
    UTX_PACKETSIZE_LO = 69
    UTX_CLKDIV = 70
    UTX_PACKETSIZE_HI = 80
    
    # URX
    URX_CONTROL = 71
    URX_CLKDIV = 72
    URX_DELAY = 73
    
    # Utrig
    UTRIG1_CONTROL = 74
    UTRIG1_DELAY = 75
    UTRIG1_HOLD = 76
    UTRIG2_CONTROL = 77
    UTRIG2_DELAY = 78
    UTRIG2_HOLD = 79
    
    # GPIO switch
    GPIO1_SELECT = 82
    GPIO1_CONTROL = 83
    GPIO_OUTPUTS = 84

class Register_Bits(Enum):
    """register bits definitions"""
    
    # Universal trigger status register bits
    UTRIG_STATUS_ARMED = 0
    UTRIG_STATUS_TRIGGERED = 1

    # Universal trigger control register bits
    UTRIG_CONTROL_ARM = 0
    UTRIG_CONTROL_FORCE = 1
    UTRIG_CONTROL_INPUT_MODE_0 = 2
    UTRIG_CONTROL_INPUT_MODE_1 = 3
    UTRIG_CONTROL_INPUT_MODE_2 = 4
    UTRIG_CONTROL_OUTPUT_MODE_0 = 5
    UTRIG_CONTROL_OUTPUT_MODE_1 = 6
    UTRIG_CONTROL_OUTPUT_MODE_2 = 7

    # Universal TX control register bits
    UTX_CONTROL_START = 0
    UTX_CONTROL_STOP = 1
    UTX_CONTROL_CLEAR = 2
    UTX_CONTROL_OUT_MODE_0 = 3
    UTX_CONTROL_OUT_MODE_1 = 4
    UTX_CONTROL_ONE_TO_Z = 5
    UTX_CONTROL_DISCONNECT_TX_URX = 6
    UTX_CONTROL_ENABLE_UTIMING = 7

    # Universal TX clock control register bits
    UTX_SHIFT_CLK_MODE_0 = 0
    UTX_SHIFT_CLK_MODE_1 = 1

    # UTX status register bits
    UTX_STATUS_READY = 0
    UTX_STATUS_TRANSMITTING = 1

    # Universal RX control register bits
    URX_CONTROL_START = 0
    URX_CONTROL_STOP = 1
    URX_CONTROL_CLEAR = 2
    URX_CONTROL_RESYNC_TO_RISING_EDGES = 3
    URX_CONTROL_RESYNC_TO_FALLING_EDGES = 4
    URX_CONTROL_UTIMING_OR_UTX = 5
    URX_CONTROL_RESYNC_ON_EDGE = 6

    # URX status register bits
    URX_STATUS_READY = 0
    URX_STATUS_RECEIVING = 1
    URX_STATUS_DATA_IN_SAMPLE = 2

    # Universal PW RX control register bits
    URX_PW_CONTROL_START = 0
    URX_PW_CONTROL_STOP = 1
    URX_PW_CONTROL_CLEAR = 2
    URX_PW_CONTROL_RESYNC_TO_RISING_EDGES = 3
    URX_PW_CONTROL_RESYNC_TO_FALLING_EDGES = 4
    URX_PW_CONTROL_UTIMING_OR_UTX = 5
    URX_PW_CONTROL_RESYNC_ON_EDGE = 6

    # URX PW status register bits
    URX_PW_STATUS_READY = 0
    URX_PW_STATUS_RECEIVING = 1
    URX_PW_STATUS_DATA_IN_SAMPLE = 2

    # DDR control register bits
    DDR_CONTROL_WRITE_COMMIT = 0
    DDR_CONTROL_READ_COMMIT = 1
    DDR_CONTROL_SLAVE_FIFO_START = 2
    DDR_CONTROL_RESET = 3
    DDR_CONTROL_DMA_SOFTWARE_WRITE_START = 4
    DDR_CONTROL_DMA_IN_SEL0 = 5
    DDR_CONTROL_DMA_IN_SEL1 = 6

    # DDR status register bits
    DDR_STATUS_DMA = 6

    # Smartcard control register bits
    SC_CONTROL_POWER = 0
    SC_CONTROL_TRANSMIT = 1

    # Smartcard status register bits
    SC_STATUS_POWERED = 0
    SC_STATUS_POWERING_UP = 1
    SC_STATUS_TRANSMITTING = 2
    SC_STATUS_WAITING = 3
    SC_STATUS_DECODING = 4

    # PIC programmer control register bits
    PIC_CONTROL_HAS_DATA = 0
    PIC_CONTROL_GET_RESPONSE = 1
    PIC_CONTROL_TRANSMIT = 2
    PIC_CONTROL_PROG_STARTSTOP = 3
    PIC_CONTROL_START_AND_TRANSMIT = 4

    # DAC control register bits
    DAC_ENABLE = 0
    DAC_TEST_MODE = 1
    DAC_RFID_MODE = 2
    DAC_UTX_MODE = 3

    # ADC control register bits
    ADC_CONTROL_DELAY_INC = 0
    ADC_CONTROL_DELAY_CAL = 1

    # ADC control register bits
    THRESHOLD_CONTROL_ARM = 0
    THRESHOLD_CONTROL_COARSE_TRIGGER_EN = 1
    THRESHOLD_CONTROL_SOFTWARE_TRIGGER = 2
    THRESHOLD_CONTROL_COARSE_TRIGGER_INVERT = 3

    # ADC status register bits
    THRESHOLD_STATUS_ARMED = 0

    # FI control register bits
    FI_CONTROL_W_EN = 0
    FI_CONTROL_ARM = 1
    FI_CONTROL_TRIGGER = 2

    # utiming control register bits
    UTIMING_CONTROL_W_EN = 0
    UTIMING_CONTROL_ARM = 1
    UTIMING_CONTROL_TRIGGER = 2
    UTIMING_CONTROL_DISARM = 3

    # utiming status register bits
    UTIMING_STATUS_READY = 0
    UTIMING_STATUS_ARMED = 1

    # SV control register bits
    SV_CONTROL_VDD = 0
    SV_CONTROL_BUTTON = 1
    SV_CONTROL_SW_TRIGGER_EN = 2
    SV_CONTROL_SW_TRIGGER = 3
    SV_CONTROL_GPIO5 = 7
    SV_CONTROL_GPIO6 = 6

    # FI trigger control register bits
    FI_TRIGGER_CONTROL_DAC_POWER = 0
    FI_TRIGGER_CONTROL_UNIVERSAL = 1
    FI_TRIGGER_CONTROL_EXT1 = 2
    FI_TRIGGER_CONTROL_ADC = 3
    FI_TRIGGER_GPIO_OUTPUT_0 = 4
    FI_TRIGGER_GPIO_OUTPUT_1 = 5
    FI_TRIGGER_CONTROL_INVERT_EDGE = 7
    
    # Universal FI trigger
    FI_UNIVERSAL_TRIGGER_CONTROL_RFID = 0
    FI_UNIVERSAL_TRIGGER_CONTROL_UTX_START = 1
    FI_UNIVERSAL_TRIGGER_CONTROL_UTIMING = 2
    FI_UNIVERSAL_TRIGGER_CONTROL_UTRIG1 = 3
    FI_UNIVERSAL_TRIGGER_CONTROL_SC_SENT = 4
    FI_UNIVERSAL_TRIGGER_CONTROL_SC_START_SEND = 5

    # FI status register bits
    FI_STATUS_READY = 0
    FI_STATUS_ARMED = 1

    # RFID control register bits
    RFID_CONTROL_ENABLED = 0
    RFID_CONTROL_TRIGGER_RESET = 1
    
    # GPIO control bits
    GPIO_ENABLE = 0
    GPIO_CLEAR = 1
    
class GPIO_Pins(Enum):
    GPIO0 = 0
    GPIO1 = 1
    GPIO2 = 2
    GPIO3 = 3
    GPIO4 = 4
    GPIO5 = 5
    GPIO6 = 6
    GPIO7 = 7
    
class GPIO_Select_Bits(Enum):
    UTX_DATA_OUT_VALID = 0
    FI_TRIGGER = 1
    FI_INJECT_FAULT = 2
    UTX_START = 3,
    URX_DATA_IN_WITH_TX = 4,
    DDR_DMA_START = 5
    SC_DATA_SENDING_TRIGGER = 6
    SC_DATA_SENT_TRIGGER = 7
    THRESH_TRIGGER = 8
    PIC_V_DD_EN = 9
    NOT_PIC_V_PP_EN = 10
    PIC_ISPDAT = 11
    UTIMING_OUT = 12
    UTRIG1_TRIGGER = 13
    UTRIG2_TRIGGER = 14
    GPIO_OUTPUT_0 = 15
    GPIO_OUTPUT_1 = 16
    GPIO_OUTPUT_2 = 17
    GPIO_OUTPUT_3 = 18
    UTX_DATA_OUT = 19
    FI_EXTERNAL_TRIGGER_IN = 20
    VALUE_0 = 29
    VALUE_1 = 30
    VALUE_Z = 31

class FPGA_Vars(Enum):
    # FPGA result codes
    FPGA_SUCCESS = 0x00
    FPGA_FAILURE = 0xff

    # Begin and number of readable/writable registers
    FPGA_REG_READ_BEGIN = 0
    FPGA_REG_WRITE_BEGIN = 32
    FPGA_REG_READ_COUNT = 32+64
    FPGA_REG_WRITE_COUNT = 64
