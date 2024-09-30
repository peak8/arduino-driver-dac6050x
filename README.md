# DAC6050x

Arduino class for the TI digital to analog converter family DAC6050x, DAC7050x, and DAC8050x. The following parts are supported:

| Part | Description |
| ---- | ----------- |
| DAC60501 | 12-bit, 1-channel DAC |
| DAC60502 | 12-bit, 2-channel DAC |
| DAC60504 | 12-bit, 4-channel DAC |
| DAC60508 | 12-bit, 8-channel DAC |
| DAC70501 | 14-bit, 1-channel DAC |
| DAC70502 | 14-bit, 2-channel DAC |
| DAC70504 | 14-bit, 4-channel DAC |
| DAC70508 | 14-bit, 8-channel DAC |
| DAC80501 | 16-bit, 1-channel DAC |
| DAC80502 | 16-bit, 2-channel DAC |
| DAC80504 | 16-bit, 4-channel DAC |
| DAC80508 | 16-bit, 8-channel DAC |

The class constructor allows customization of the following parameters:

- I2C address (defaults to 0x48)
- I2C Port Reference - Wire, Wire1 or Wire2 (defaults to Wire)
- I2C speed (defaults to 4MBit)
- gain (defaults to gain of 1)

Resolution and number of channels are read from the device connected to the configured address and cannot be customized.