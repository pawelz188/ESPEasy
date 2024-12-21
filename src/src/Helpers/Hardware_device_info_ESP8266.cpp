#include "../Helpers/Hardware_device_info.h"


#ifdef ESP8266

bool isFlashInterfacePin_ESPEasy(int gpio) {
  if (isESP8285()) {
    return (gpio) == 6 || (gpio) == 7 || (gpio) == 8 || (gpio) == 11;
  }
  return (gpio) >= 6 && (gpio) <= 11;
}



#endif