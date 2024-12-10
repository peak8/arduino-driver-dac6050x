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
#define GAIN_BUFF_GAIN_2                0x00FF
#define GAIN_REF_DIV_BY_2               0x0100
#define TRIGGER_SOFT_RESET              0x000A

#define RESOLUTION_12_BIT               0x02
#define MSK_12_BIT_RESOLUTION           0x0FFF
#define RESOLUTION_14_BIT               0x01
#define MSK_14_BIT_RESOLUTION           0x3FFF
#define RESOLUTION_16_BIT               0x00
#define MSK_16_BIT_RESOLUTION           0xFFFF

#define NUM_CHANNELS_1                  0x01
#define NUM_CHANNELS_2                  0x02
#define NUM_CHANNELS_4                  0x04
#define NUM_CHANNELS_8                  0x08



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
    }

    return val;
}

uint16_t DAC6050x::write_register(uint8_t command, uint16_t value) {
    _wire->beginTransmission(_address);
    _wire->write(command);
    _wire->write((uint8_t)(value >> 8));
    _wire->write((uint8_t)(value & 0xFF));
    return _wire->endTransmission();
}



DAC6050x::DAC6050x(const uint8_t address, 
             TwoWire *wire, 
             uint32_t speed,
             uint8_t gain) {
    _address = address;
    _wire = wire;
    _I2Cspeed = speed;
    _gain = gain;
    // init with 12-bit and 1 channel. Will be update by begin().
    _resolution = (uint8_t)RESOLUTION_12_BIT;
    _num_channels = (uint8_t)NUM_CHANNELS_1;
    _device_id = 0xFFFF;
}

DAC6050x::~DAC6050x() {
    _wire = nullptr;
}

uint16_t DAC6050x::self_test(void) {
    uint16_t result;

    _wire->begin();
    _wire->setClock(_I2Cspeed);

    // Reset the DAC to defaults
    result = write_register((uint8_t)TRIGGER_CMD, (uint16_t)TRIGGER_SOFT_RESET);

    if(result == 0) {
        // allow the DAC to complete reset
        delayMicroseconds(100);
        // configure the gain, default after reset is 2
        if(_gain == 1) {
            result = write_register((uint8_t)GAIN_CMD,
                        (uint16_t)(GAIN_REF_DIV_BY_2 | GAIN_BUFF_GAIN_1));
        }
    }

    if(result == 0) {
        // read the device ID register and get the resolution and number of channels.
        _device_id = read_register(DEVICE_ID_CMD);
        if(_device_id > 0) {
            _resolution = (uint8_t)((_device_id & DEVICE_ID_RESOLUTION_MSK) >> DEVICE_ID_RESOLUTION_SHIFT);
            _num_channels = (uint8_t)((_device_id & DEVICE_ID_NUM_CHANNELS_MSK) >> DEVICE_ID_NUM_CHANNELS_SHIFT);
        } else {
            result = __LINE__;
        }
    }

    _wire->end();
    
    return result;
}

uint8_t DAC6050x::set_dac_output(uint16_t value, uint8_t channel) {
    uint8_t result;

    // Test if channel is within the range obtained from the DEVICE_ID command,
    // read when we called the begin() function.
    if(channel > _num_channels) {
        return 0xFF;
    } 
    
    // Test if the value exceeds the range allowed by the resolution obtained
    // from the DEVICE ID command read when we called the begin() function.
    if((value > (value & (uint16_t)MSK_12_BIT_RESOLUTION)) ||
       (value > (value & (uint16_t)MSK_14_BIT_RESOLUTION))) {
        return 0xFF;
    }

    // We have a good value and channel number, fire up the I2C and set it.
    _wire->begin();
    _wire->setClock(_I2Cspeed);

    // Use the DAC0 command byte as the base and add the user selected 
    // channel number to get the proper command byte .
    result = write_register((uint8_t)DAC0_DATA_CMD + channel, value);

    _wire->end();
    
    return result;
}

 uint16_t DAC6050x::get_device_id(void) {
    return _device_id;
 }
