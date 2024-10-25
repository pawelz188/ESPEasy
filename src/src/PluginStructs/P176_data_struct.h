#ifndef PLUGINSTRUCTS_P176_DATA_STRUCT_H
#define PLUGINSTRUCTS_P176_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P176

# include <ESPeasySerial.h>

# define P176_DEBUG 0           // Enable some extra (development) logging

# define P176_HANDLE_CHECKSUM 1 // Implement checksum?
# define P176_FAIL_CHECKSUM   1 // Fail/ignore data on checksum errors (optional)?

# define P176_SERIAL_CONFIG             PCONFIG(0)
# define P176_SERIAL_BAUDRATE           PCONFIG(1)
# define P176_RX_WAIT                   PCONFIG(2)

# define P176_FLAGS                     PCONFIG_ULONG(0)
# define P176_FLAG_LED_PIN              0 // 8 bit
# define P176_FLAG_LED_INVERTED         8 // 1 bit
# define P176_FLAG_FAIL_CHECKSUM        9 // 1 bit
# define P176_GET_LED_PIN    get8BitFromUL(P176_FLAGS, P176_FLAG_LED_PIN)
# define P176_SET_LED_PIN(N) set8BitToUL(P176_FLAGS, P176_FLAG_LED_PIN, N)
# define P176_GET_LED_INVERTED    bitRead(P176_FLAGS, P176_FLAG_LED_INVERTED)
# define P176_SET_LED_INVERTED(N) bitWrite(P176_FLAGS, P176_FLAG_LED_INVERTED, N)
# if P176_FAIL_CHECKSUM
#  define P176_GET_FAIL_CHECKSUM    bitRead(P176_FLAGS, P176_FLAG_FAIL_CHECKSUM)
#  define P176_SET_FAIL_CHECKSUM(N) bitWrite(P176_FLAGS, P176_FLAG_FAIL_CHECKSUM, N)
# endif // if P176_FAIL_CHECKSUM

# define P176_DEFAULT_BAUDRATE          19200
# define P176_DEFAULT_RX_WAIT           10

struct P176_data_struct : public PluginTaskData_base {
public:

  P176_data_struct(struct EventStruct *event);

  P176_data_struct() = delete;
  virtual ~P176_data_struct();

  bool   init();

  bool   plugin_read(struct EventStruct *event);
  bool   plugin_ten_per_second(struct EventStruct *event);
  bool   plugin_get_config_value(struct EventStruct *event,
                                 String            & string);
  size_t plugin_size_current_data();
  bool   plugin_show_current_data();
  bool   isInitialized() const {
    return nullptr != _serial;
  }

private:

  float getKeyFactor(const String& key) const;
  bool  getReceivedValue(const String& key,
                         String      & value);
  void  handleSerial();
  void  processBuffer(const String& message);
  # if P176_FAIL_CHECKSUM
  void  moveTempToData();
  # endif // if P176_FAIL_CHECKSUM

  ESPeasySerial *_serial = nullptr;

  int8_t            _ledPin         = -1;
  bool              _ledInverted    = false;
  ESPEasySerialPort _port           = ESPEasySerialPort::not_set;
  uint8_t           _config         = 0;
  int8_t            _rxPin          = -1;
  int8_t            _txPin          = -1;
  int               _baud           = P176_DEFAULT_BAUDRATE;
  int               _rxWait         = 0;
  uint32_t          _checksumErrors = 0;

  // String            _buffer;
  # if P176_HANDLE_CHECKSUM
  enum class Checksum_state_e: uint8_t {
    Undefined = 0,
    Starting,
    Counting,
    ValidateNext,
    Validating,
    Error,
  };
  Checksum_state_e _checksumState = Checksum_state_e::Undefined;
  uint16_t         _checksum      = 0;
  bool             _failChecksum  = false;
  # endif // if P176_HANDLE_CHECKSUM

  // Key is stored in lower-case as PLUGIN_GET_CONFIG_VALUE passes in the variable name in lower-case
  // FIXME
  std::map<String, String>_data{}; // = { { "i", "Data I" }, { "error", "No error detected" }, { "v", "25.75" } };
  # if P176_FAIL_CHECKSUM
  std::map<String, String>_temp{};
  # endif // if P176_FAIL_CHECKSUM
};

#endif // ifdef USES_P176
#endif // ifndef PLUGINSTRUCTS_P176_DATA_STRUCT_H
