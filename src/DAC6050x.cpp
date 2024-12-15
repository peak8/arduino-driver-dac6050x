/**
 * @file DAC6050x.cpp
 * @author David Andresky
 * @version 0.1
 * @date 2024-09-28
 */

#include "DAC6050x.hpp"



#define CMD_NOOP                        0
#define DEVICE_ID_CMD                   1
#define CMD_SYNC                        2
#define CMD_CONFIG                      3
#define GAIN_CMD                        4
#define TRIGGER_CMD                     5
#define CMD_BROADCAST                   6
#define CMD_STATUS                      7
#define DAC0_DATA_CMD                   8

// Register defines
#define DEVICE_ID_RESOLUTION_MSK        0x7000
#define DEVICE_ID_RESOLUTION_SHIFT      12
#define DEVICE_ID_NUM_CHANNELS_MSK      0x0F00
#define DEVICE_ID_NUM_CHANNELS_SHIFT    8
#define DEVICE_ID_RSTSEL_MSK            0x0080
#define DEVICE_ID_RSTSEL_SHIFT          7

#define GAIN_BUFF_GAIN_1                0x0000
#define GAIN_BUFF_GAIN_2                0x0003
#define GAIN_REF_DIV_BY_2               0x0100
#define TRIGGER_SOFT_RESET              0x000A

#define CONFIG_REF_PWDWN                0x0100

#define RESOLUTION_12_BIT               0x02
#define MSK_12_BIT_RESOLUTION           0x0FFF
#define RESOLUTION_14_BIT               0x01
#define MSK_14_BIT_RESOLUTION           0x3FFF
#define RESOLUTION_16_BIT               0x00
#define MSK_16_BIT_RESOLUTION           0xFFFF



uint16_t DAC6050x::read_register(uint8_t command) {
    _wire->setClock(_I2Cspeed);

    // first set the register pointer
    _wire->beginTransmission(_address);
    _wire->write(command);
    _wire->endTransmission();

    uint16_t val = 0x0000;

    int available = _wire->requestFrom((int)_address, 2);
    if(available == 2) {
        val = _wire->read() << 8;
        val |= _wire->read();
        // val = 0xFFF0;
    } else if(available == 0) {
        val = 0xFFFF;
    } else {
        val = available;
    }

    return val;
}

uint16_t DAC6050x::write_register(uint8_t command, uint16_t value) {
    _wire->setClock(_I2Cspeed);
    _wire->beginTransmission(_address);
    _wire->write(command);
    _wire->write((uint8_t)(value >> 8));
    _wire->write((uint8_t)(value & 0x00FF));
    return _wire->endTransmission();
}



DAC6050x::DAC6050x(const uint8_t address, 
             TwoWire *wire, 
             uint32_t speed,
             uint8_t gain) {
    _address = address;
    _wire = wire;
    _I2Cspeed = speed;
    _gain = 2;
    _num_channels = 1;
    _device_id = 0xFFFF;
}

DAC6050x::~DAC6050x() {
    _wire = nullptr;
}

uint16_t DAC6050x::setup(void) {
    uint16_t result = 0;

    _wire->begin();
    _wire->setClock(_I2Cspeed);

    // disable the internal reference
    result = write_register((uint8_t)CMD_CONFIG, (uint16_t)CONFIG_REF_PWDWN);

    // configure the gain
    if(result == 0) {
        if(_gain == 1) {
            result = write_register((uint8_t)GAIN_CMD,
                        (uint16_t)(GAIN_REF_DIV_BY_2 | GAIN_BUFF_GAIN_1));
        } else {
            result = write_register((uint8_t)GAIN_CMD,
                        (uint16_t)(GAIN_REF_DIV_BY_2 | GAIN_BUFF_GAIN_2));
        }
    } else {
        result = __LINE__;
    }

    if(result == 0) {
        // read the device ID register and get the resolution and number of channels.
        _device_id = read_register(DEVICE_ID_CMD);
        if(_device_id > 0) {
            _num_channels = (uint8_t)((_device_id & DEVICE_ID_NUM_CHANNELS_MSK) >> DEVICE_ID_NUM_CHANNELS_SHIFT);
            uint8_t res = (uint8_t)((_device_id & DEVICE_ID_RESOLUTION_MSK) >> DEVICE_ID_RESOLUTION_SHIFT);
            if(_num_channels > 8) { result = __LINE__; }
            else if(res != 2) { result = __LINE__; }
        } else {
            result = __LINE__;
        }
    }

    if(result == 0) {
        result = set_dac_output(0, 0x7FF0);
    }
    if(result == 0) {
        result = set_dac_output(1, 0x7FF0);
    }

    _wire->end();
    
    return result;
}

uint8_t DAC6050x::set_dac_output(uint8_t channel, uint16_t value) {
    uint8_t result;

    // Test if channel is within the range obtained from the DEVICE_ID command.
    // channel indexing starts at 0, so add 1 for this test.
    if(channel+1 > _num_channels) {
        return 0xFF;
    } 
    
    // Test if the value exceeds the range allowed by the resolution obtained
    // from the DEVICE ID command read when we called the begin() function.
    if(value > 4095) {
        return 0xFF;
    }

    // value is limited to the range of a 12 bit number but 16 bits is transmitted
    // and the msb of the value needs to be shifted to the msb of the 16-bit command
    // bytes
    uint16_t temp_val = value << 4;

    // We have a good value and channel number, fire up the I2C and set it.
    _wire->begin();
    _wire->setClock(_I2Cspeed);

    // start with channel 0 command byte (8) and add the selected channel to
    // get the correct command byte value.
    result = write_register((uint8_t)DAC0_DATA_CMD + channel, temp_val);

    _wire->end();
    
    return result;
}

 uint16_t DAC6050x::get_device_id(void) {
    return _device_id;
 }
