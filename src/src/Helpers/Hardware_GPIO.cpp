#include "../Helpers/Hardware_GPIO.h"


#include "../Globals/Settings.h"
#include "../Helpers/Hardware_defines.h"
#include "../Helpers/Hardware_device_info.h"

// ********************************************************************************
// Get info of a specific GPIO pin
// ********************************************************************************


bool isSerialConsolePin(int gpio) {
  if (!Settings.UseSerial) { return false; }

#if defined(SOC_RX0) && defined(SOC_TX0)
  return (gpio == SOC_TX0 || gpio == SOC_RX0)
         #if USES_ESPEASY_CONSOLE_FALLBACK_PORT
         && Settings.console_serial0_fallback
         #endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  ;
#else
#ifdef ESP32S2

  return (gpio == 1 || gpio == 3)
         #if USES_ESPEASY_CONSOLE_FALLBACK_PORT
         && Settings.console_serial0_fallback
         #endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  ;

#elif defined(ESP32S3)

  return (gpio == 43 || gpio == 44)
         #if USES_ESPEASY_CONSOLE_FALLBACK_PORT
         && Settings.console_serial0_fallback
         #endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  ;

#elif defined(ESP32C3)

  return (gpio == 21 || gpio == 20) 
         #if USES_ESPEASY_CONSOLE_FALLBACK_PORT
         && Settings.console_serial0_fallback
         #endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  ;

#elif defined(ESP32_CLASSIC)
  return gpio == 1 || gpio == 3;

#elif defined(ESP8266)
  return gpio == 1 || gpio == 3;

#else // ifdef ESP32S2
  static_assert(false, "Implement processor architecture");
  return false;
#endif // ifdef ESP32S2
#endif
}
