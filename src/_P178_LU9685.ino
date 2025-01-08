#include "_Plugin_Helper.h"
#ifdef USES_P178

# include "src/PluginStructs/P178_data_struct.h"

# include "ESPEasy-Globals.h"


// #######################################################################################################
// #################################### Plugin 178: LU9685 ###############################################
// #######################################################################################################


# define PLUGIN_178
# define PLUGIN_ID_178         178
# define PLUGIN_NAME_178       "Extra IO - LU9685 Servo controller"
# define PLUGIN_VALUENAME1_178 "PWM"

# define P178_I2C_ADDR  CONFIG_PORT
# define P178_FREQ      PCONFIG(0)

boolean Plugin_178(uint8_t function, struct EventStruct *event, String& string)
{
  boolean  success = false;
  int      address = P178_I2C_ADDR;
  uint16_t freq    = P178_FREQ;


  if ((address < LU9685_ADDRESS) || (address > LU9685_MAX_ADDRESS)) {
    address = LU9685_ADDRESS;
  }

  if ((freq < LU9685_MIN_FREQUENCY) || (freq > LU9685_MAX_FREQUENCY)) {
    freq = LU9685_DEFAULT_FREQUENCY;
  }


  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number = PLUGIN_ID_178;
      dev.Type   = DEVICE_TYPE_I2C;
      dev.VType  = Sensor_VType::SENSOR_TYPE_NONE;
      dev.Ports  = 1;
      dev.Custom = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_178);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_178));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      constexpr size_t address_count = (LU9685_MAX_ADDRESS - LU9685_ADDRESS) + 1;
      uint8_t optionValues[address_count];

      for (uint8_t i = 0; i < address_count; ++i)
      {
        optionValues[i] = LU9685_ADDRESS + i;
      }
      addFormSelectorI2C(F("i2c_addr"), address_count, optionValues, address);
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = address;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      P178_I2C_ADDR = LU9685_ADDRESS;
      P178_FREQ     = LU9685_DEFAULT_FREQUENCY;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(
        F("Frequency"),
        F("pfreq"),
        freq,
        LU9685_MIN_FREQUENCY,
        LU9685_MAX_FREQUENCY);
      addFormNote(concat(F("default "), LU9685_DEFAULT_FREQUENCY));
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P178_I2C_ADDR = getFormItemInt(F("i2c_addr"));
      P178_FREQ     = getFormItemInt(F("pfreq"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      success = initPluginTaskData(event->TaskIndex, new (std::nothrow) P178_data_struct(P178_I2C_ADDR, P178_FREQ));
      break;
    }

    case PLUGIN_WRITE:
    {
      # if FEATURE_I2C_DEVICE_CHECK

      if (!I2C_deviceCheck(address, event->TaskIndex, 10, PLUGIN_I2C_GET_ADDRESS)) {
        break; // Will return the default false for success
      }
      # endif // if FEATURE_I2C_DEVICE_CHECK
      P178_data_struct *P178_data =
        static_cast<P178_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P178_data) {
        break;
      }
      const String command = parseString(string, 1);

      if (!(equals(command, F("lu9685")))) {
        break;
      }

      const String subcommand = parseString(string, 2);

      // Command: lu9685,servo,<pin>,<angle>
      // Negative angle will disable pulse on pin
      if ((equals(subcommand, F("servo"))))
      {
        success = true;
        const uint32_t servoPin = event->Par2;
        const int32_t  angle    = event->Par3;

        if (servoPin > LU9685_MAX_PINS) {
          addLog(LOG_LEVEL_ERROR, concat(P178_data->logPrefix(F("Incorrect pin: ")), servoPin));
          success = false;
        }

        if (angle > LU9685_MAX_ANGLE) {
          addLog(LOG_LEVEL_ERROR, concat(P178_data->logPrefix(F("Incorrect angle: ")), angle));
          success = false;
        }

        if (success) {
          P178_data->setAngle(servoPin, angle, angle >= 0);
        }
        break;
      }

      break;
    }
  }
  return success;
}

#endif // ifdef USES_P178
