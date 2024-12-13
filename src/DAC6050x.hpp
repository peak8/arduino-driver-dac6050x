/**
 * @file DAC6050x.hpp
 * @author David Andresky (Peak 8 Connected)
 * @brief Arduino driver for the DACx050x series of digital to analog
 * converters from TI.
 * @version 0.1
 * @date 2024-09-28
 */

#ifndef DAC6050X_H
#define DAC6050X_H

#include <stdint.h>

#include "Arduino.h"
#include "Wire.h"



class DAC6050x {
private:
    uint8_t _address;
    uint32_t _I2Cspeed;  
    TwoWire *_wire;
    uint8_t _gain;
    uint8_t _resolution;
    uint8_t _num_channels;
    uint16_t _device_id;
    
    uint16_t read_register(uint8_t command);
    uint16_t write_register(uint8_t command, uint16_t value);

public:
    /**
     * @brief Construct a new DAC6050x object
     * 
     * @param address - I2C of the device
     * @param wire - reference to I2C0, I2C1, or I2C2
     * @param speed - desired bit rate
     * @param gain - 1 or 2
     */
    DAC6050x(const uint8_t address = 0x48, 
             TwoWire *wire = &Wire, 
             uint32_t speed = 400000,
             uint8_t gain = 2);

    ~DAC6050x();

    /**
     * @brief Starts the arduino wire libray and attempts to reset the DAC to
     * a known state and configure it. Reads the device ID to get resolution
     * and number of channels.
     * 
     * @return 0 = success, see Wire endTransmission() doc for error codes
     */
    uint16_t self_test(void);

    /**
     * @brief Set the DAC output voltage.
     * 
     * @param value DAC counts between the min/max of the resolution
     * @param channel integer values 1 through 8
     * 
     * @return 0 = success, FF - parameter error, see Wire endTransmission()  
     * doc for other error codes.
     */
    uint8_t set_dac_output(uint16_t value, uint8_t channel = 1);

    /**
     * @brief Get the device id that was read during self-test
     * 
     * @return uint16_t device ID
     */
    uint16_t get_device_id(void);

};

#endif /* DAC6050X_H */