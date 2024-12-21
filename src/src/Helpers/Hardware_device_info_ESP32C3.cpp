#include "../Helpers/Hardware_device_info.h"

#ifdef ESP32C3

// See: https://github.com/espressif/esptool/blob/master/esptool/targets/esp32c3.py


  # include <soc/soc.h>
  # include <soc/efuse_reg.h>
  # include <soc/spi_reg.h>
  # include <soc/rtc.h>


/** EFUSE_FLASH_CAP : R; bitpos: [29:27]; default: 0;
 * register: EFUSE_RD_MAC_SPI_SYS_3_REG
 */
  # define EFUSE_FLASH_CAP    0x00000007U
  # define EFUSE_FLASH_CAP_M  (EFUSE_FLASH_CAP_V << EFUSE_FLASH_CAP_S)
  # define EFUSE_FLASH_CAP_V  0x00000007U
  # define EFUSE_FLASH_CAP_S  27


/** EFUSE_VDD_SPI_AS_GPIO : RO; bitpos: [26]; default: 0;
 *  Set this bit to vdd spi pin function as gpio.
 *  register: EFUSE_RD_REPEAT_ERR0_REG
 */
# define EFUSE_VDD_SPI_AS_GPIO    (BIT(26))
# define EFUSE_VDD_SPI_AS_GPIO_M  (EFUSE_VDD_SPI_AS_GPIO_V << EFUSE_VDD_SPI_AS_GPIO_S)
# define EFUSE_VDD_SPI_AS_GPIO_V  0x00000001U
# define EFUSE_VDD_SPI_AS_GPIO_S  26

bool isFlashInterfacePin_ESPEasy(int gpio) {
  // GPIO-11: Flash voltage selector
  // GPIO-12 ... 17: Connected to flash
  return (gpio) >= 12 && (gpio) <= 17;
}

bool flashVddPinCanBeUsedAsGPIO()
{
  const bool efuse_vdd_spi_as_gpio = REG_GET_FIELD(EFUSE_RD_REPEAT_ERR0_REG, EFUSE_VDD_SPI_AS_GPIO) != 0;

  return efuse_vdd_spi_as_gpio;
}

int32_t getEmbeddedFlashSize()
{
  const uint32_t flash_cap = REG_GET_FIELD(EFUSE_RD_MAC_SPI_SYS_3_REG, EFUSE_FLASH_CAP);

  switch (flash_cap) {
    case 0: return 0;
    case 1: return 4;
    case 2: return 2;
    case 3: return 1;
    case 4: return 8;
  }

  // Unknown value, thus mark as negative value
  return -1 *  static_cast<int32_t>(flash_cap);
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
#endif // ifdef ESP32C3
