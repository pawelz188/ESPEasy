#include "../Helpers/Hardware_device_info.h"

#ifdef ESP32_CLASSIC

bool isFlashInterfacePin_ESPEasy(int gpio) {
  if (getChipFeatures().embeddedFlash ||
      getChipFeatures().embeddedPSRAM) {
    // See page 20: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
    if (getChipFeatures().embeddedFlash) {
      //  ESP32-U4WDH In-Package Flash (4 MB)
      //  SD_DATA_1   IO0/DI      (GPIO-8)
      //  GPIO17      IO1/DO      (GPIO-17)
      //  SD_DATA_0   IO2/WP#     (GPIO-7)
      //  SD_CMD      IO3/HOLD#   (GPIO-11)
      //  SD_CLK      CLK         (GPIO-6)
      //  GPIO16      CS#         (GPIO-16)
      //  GND         VSS
      //  VDD_SDIO1   VDD

      if ((gpio >= 6) && (gpio <= 8)) {                                   return true; }

      if ((gpio == 17) || (gpio == 11) || (gpio == 16)) {                                                   return true; }
    }

    if (getChipFeatures().embeddedPSRAM) {
      //  ESP32-D0WDR2-V3 In-Package PSRAM (2 MB)
      //  SD_DATA_1       SIO0/SI (GPIO-8)
      //  SD_DATA_0       SIO1/SO (GPIO-7)
      //  SD_DATA_3       SIO2    (GPIO-10)
      //  SD_DATA_2       SIO3    (GPIO-9)
      //  SD_CLK          SCLK    (GPIO-6)
      //  GPIO16          CE#     (GPIO-16)
      //  GND             VSS
      //  VDD_SDIO1       VDD
      if ((gpio >= 6) && (gpio <= 10)) {                                    return true; }

      if (gpio == 16) { return true; }
    }
    return false;
  }


  // GPIO-6 ... 11: SPI flash and PSRAM
  // GPIO-16 & 17: CS for PSRAM, thus only unuable when PSRAM is present
  return (gpio) >= 6 && (gpio) <= 11;
}

bool flashVddPinCanBeUsedAsGPIO()
{
  return false;
}

int32_t getEmbeddedFlashSize()
{
  return 0;
}

int32_t getEmbeddedPSRAMSize()
{
  // FIXME TD-er: Need to implement
  return 0;
}

# ifndef isPSRAMInterfacePin
bool isPSRAMInterfacePin(int gpio) {
  // GPIO-6 ... 11: SPI flash and PSRAM
  // GPIO-16 & 17: CS for PSRAM, thus only unuable when PSRAM is present
  return FoundPSRAM() ? ((gpio) == 16 || (gpio) == 17) : false;
}

# endif // ifndef isPSRAMInterfacePin


#endif // ifdef ESP32_CLASSIC
