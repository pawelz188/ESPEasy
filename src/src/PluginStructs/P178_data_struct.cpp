
#include "../PluginStructs/P178_data_struct.h"

#ifdef USES_P178

P178_data_struct::P178_data_struct(uint8_t address, uint16_t freq) : _address(address)
{
  // Reset
  I2C_write8_reg(_address, 0xFB, 0xFB);
  setFrequency(freq);
  update();
}

void P178_data_struct::enablePin(uint8_t pin, bool enable)
{
  if (pin < LU9685_MAX_PINS) {
    if (bitRead(_enabledPin, pin) != enable) {
      bitWrite(_enabledPin, pin, enable);
      setAngle(pin, _data[pin]);
    }
  }
}

void P178_data_struct::setAngle(uint8_t pin, int angle)
{
  if (pin < LU9685_MAX_PINS) {
    if ((angle < 0) || (angle > LU9685_MAX_ANGLE)) {
      bitClear(_enabledPin, pin);
    } else {
      _data[pin] = angle;
    }
    I2C_write8_reg(_address, pin, getAngle(pin));
  }
}

void P178_data_struct::setAngle(
  uint8_t pin,
  int     angle,
  bool    enable)
{
  if (pin < LU9685_MAX_PINS) {
    bitWrite(_enabledPin, pin, enable);
    setAngle(pin, constrain(angle, 0, LU9685_MAX_ANGLE));
  }
}

bool P178_data_struct::update()
{
  // Write all 20 values in a single I2C call
  Wire.beginTransmission(_address);
  Wire.write(0xFD);

  for (uint8_t pin = 0; pin < LU9685_MAX_PINS; ++pin) {
    Wire.write(getAngle(pin));
  }
  return Wire.endTransmission() == 0;
}

void P178_data_struct::setFrequency(uint16_t freq)
{
  I2C_write16_reg(
    _address,
    0xFC,
    constrain(freq, LU9685_MIN_FREQUENCY, LU9685_MAX_FREQUENCY));
}

String P178_data_struct::logPrefix() {
  return concat(formatToHex(_address, F("LU9685 0x"), 2),  F(": "));
}

String P178_data_struct::logPrefix(const __FlashStringHelper *poststr)
{
  return concat(logPrefix(), poststr);
}

uint8_t P178_data_struct::getAngle(uint8_t pin) const
{
  if ((pin < LU9685_MAX_PINS) && bitRead(_enabledPin, pin)) {
    return constrain(_data[pin], 0, LU9685_MAX_ANGLE);
  }
  return LU9685_OUTPUT_DISABLED;
}

#endif // ifdef USES_P178
