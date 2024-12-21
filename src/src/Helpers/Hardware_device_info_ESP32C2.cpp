#include "../Helpers/Hardware_device_info.h"

#ifdef ESP32C2

bool isFlashInterfacePin_ESPEasy(int gpio) {
  // GPIO-11: Flash voltage selector
  // For chip variants with a SiP flash built in, GPIO11~ GPIO17 are dedicated to connecting SiP flash, not for other uses
  return (gpio) >= 12 && (gpio) <= 17;
}

bool flashVddPinCanBeUsedAsGPIO()
{
  return false;
}

int32_t getEmbeddedFlashSize()
{
  // ESP32-C2 doesn't have eFuse field FLASH_CAP.
  // Can't get info about the flash chip.
  return 0;
}

int32_t getEmbeddedPSRAMSize()
{
  // Doesn't have PSRAM
  return 0;
}

# ifndef isPSRAMInterfacePin
bool isPSRAMInterfacePin(int gpio) {
  return false;
}

# endif // ifndef isPSRAMInterfacePin

#endif // ifdef ESP32C2
