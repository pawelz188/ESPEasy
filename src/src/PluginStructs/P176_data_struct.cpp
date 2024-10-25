#include "../PluginStructs/P176_data_struct.h"

#ifdef USES_P176

# include <GPIO_Direct_Access.h>

/**************************************************************************
* Constructor
**************************************************************************/
P176_data_struct::P176_data_struct(struct EventStruct *event) {
  _port        = static_cast<ESPEasySerialPort>(CONFIG_PORT);
  _baud        = P176_SERIAL_BAUDRATE;
  _config      = P176_SERIAL_CONFIG;
  _rxPin       = CONFIG_PIN1;
  _txPin       = CONFIG_PIN2;
  _ledPin      = P176_GET_LED_PIN;
  _ledInverted = P176_GET_LED_INVERTED;
  # if P176_FAIL_CHECKSUM
  _failChecksum = P176_GET_FAIL_CHECKSUM;
  # endif // if P176_FAIL_CHECKSUM
}

P176_data_struct::~P176_data_struct() {
  delete _serial;
  _data.clear();
}

bool P176_data_struct::init() {
  delete _serial;

  if (ESPEasySerialPort::not_set != _port) {
    _serial = new (std::nothrow) ESPeasySerial(_port, _rxPin, _txPin);

    if (nullptr != _serial) {
      # if defined(ESP8266)
      _serial->begin(_baud, (SerialConfig)_config);
      # elif defined(ESP32)
      _serial->begin(_baud, _config);
      # endif // if defined(ESP8266)
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, F("Victron: Serial port started"));
      # endif // ifndef BUILD_NO_DEBUG
    }
  }

  return isInitialized();
}

/*****************************************************
* plugin_read
*****************************************************/
bool P176_data_struct::plugin_read(struct EventStruct *event)           {
  bool success = false;

  if (isInitialized()) {
    for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
      String key = getTaskValueName(event->TaskIndex, i);
      key.toLowerCase();
      String value;

      if (getReceivedValue(key, value)) {
        int32_t iValue      = 0;
        const float fFactor = getKeyFactor(key);

        if (validIntFromString(value, iValue)) {
          UserVar.setFloat(event->TaskIndex, i, iValue * fFactor);
          success = true;
        }
      }
    }
  }

  return success;
}

// In order of most values included
// mV, mAh
const char p176_factor_1000[] PROGMEM =
  "v|v2|v3|vs|vm|vpv|i|i2|i3|il|ce|h1|h2|h3|h6|h7|h8|h15|h16"
;

// 0.01 kWh, 0.01 V
const char p176_factor_0_01[] PROGMEM =
  "h17|h18|h19|h20|h22|ac_out_v|dc_in_v|"
;

// promille
const char p176_factor_10[] PROGMEM =
  "dm|soc|"
;

// 0.1 A
const char p176_factor_0_1[] PROGMEM =
  "ac_out_i|dc_in_i|"
;

float P176_data_struct::getKeyFactor(const String& key) const {
  if (key.isEmpty()) { return 1.0f; }

  if (GetCommandCode(key.c_str(), p176_factor_1000) > -1) {
    return 0.001f;
  }

  if (GetCommandCode(key.c_str(), p176_factor_0_01) > -1) {
    return 100.0f;
  }

  if (GetCommandCode(key.c_str(), p176_factor_10) > -1) {
    return 0.1f;
  }

  if (GetCommandCode(key.c_str(), p176_factor_0_1) > -1) {
    return 10.0f;
  }

  return 1.0f;
}

/*****************************************************
* plugin_ten_per_second
*****************************************************/
bool P176_data_struct::plugin_ten_per_second(struct EventStruct *event)           {
  bool success = false;

  if (isInitialized()) {
    handleSerial(); // Read and process data
    success = true;
  }

  return success;
}

/*****************************************************
* plugin_get_config_value
*****************************************************/
bool P176_data_struct::plugin_get_config_value(struct EventStruct *event,
                                               String            & string)           {
  bool success = false;

  if (isInitialized()) {
    const String key      = parseString(string, 1);
    const String decimals = parseString(string, 2); // Optional decimals, default = 2, neg. = trim trailing zeroes
    String value;

    if (getReceivedValue(key, value)) {
      int32_t iValue      = 0;
      const float fFactor = getKeyFactor(key);

      if (!essentiallyEqual(fFactor, 1.0f) && validIntFromString(value, iValue)) {
        int32_t nrDecimals = 2;
        validIntFromString(decimals, nrDecimals);
        string = toString(iValue * fFactor, abs(nrDecimals), nrDecimals < 0);
      } else {
        string = value;
      }
      success = true;
    }
  }

  return success;
}

size_t P176_data_struct::plugin_size_current_data() {
  return _data.size();
}

bool P176_data_struct::plugin_show_current_data() {
  bool success = false;

  if (_data.size() > 0) {
    html_table(EMPTY_STRING); // Sub-table
    html_table_header(F("Name"),  125);
    html_table_header(F("Data"),  250);
    html_table_header(F("Value"), 125);

    for (auto it = _data.begin(); it != _data.end(); ++it) {
      html_TR_TD();
      addHtml(it->first);
      html_TD();
      addHtml(it->second);
      html_TD();
      int32_t iValue      = 0;
      const float fFactor = getKeyFactor(it->first);

      if (!essentiallyEqual(fFactor, 1.0f) && validIntFromString(it->second, iValue)) {
        addHtml(toString(iValue * fFactor, 3, true));
      }
    }
    html_end_table();
  }
  addHtml(F("Recent checksum errors: "));
  addHtmlInt(_checksumErrors);
  return success;
}

// Support functions

/*****************************************************
* getReceivedValue
*****************************************************/
bool P176_data_struct::getReceivedValue(const String& key,
                                        String      & value) {
  bool success = false;

  if (!key.isEmpty()) { // Find is case-sensitive
    auto it = _data.find(key);

    if (it != _data.end()) {
      value   = it->second;
      success = true;
    }

    # if P176_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO, strformat(F("P176 : getReceivedValue Key:%s, value:%s"),
                                       key.c_str(), value.c_str()));
    }
    # endif // if P176_DEBUG
  }
  return success;
}

/*****************************************************
* handleSerial
*****************************************************/
void P176_data_struct::handleSerial() {
  int    timeOut   = _rxWait;
  int    maxExtend = 5;
  char   ch;
  String dataLine;

  dataLine.reserve(44); // Should be max. line-length sent, including CR/LF

  do {
    if (_serial->available()) {
      if (validGpio(_ledPin)) {
        DIRECT_pinWrite(_ledPin, _ledInverted ? 0 : 1);
      }

      ch = static_cast<uint8_t>(_serial->read());

      # if P176_HANDLE_CHECKSUM
      _checksum += static_cast<uint8_t>(ch);
      _checksum &= 255;
      # endif // if P176_HANDLE_CHECKSUM

      if (ch == '\n') {
        dataLine += static_cast<char>(ch); // append before processing
        processBuffer(dataLine);           // Store data
        dataLine.clear();

        # if P176_DEBUG
        addLog(LOG_LEVEL_ERROR, strformat(F("P176 : Checksum state: %d"),
                                          static_cast<uint8_t>(_checksumState)));
        # endif // if P176_DEBUG
        # if P176_HANDLE_CHECKSUM

        if (Checksum_state_e::ValidateNext == _checksumState) {
          _checksumState = Checksum_state_e::Validating;
        } else
        if (Checksum_state_e::Starting == _checksumState) { // Start counting after a Checksum was received
          _checksumState = Checksum_state_e::Counting;
          _checksum      = 0;
          #  ifndef BUILD_NO_DEBUG
          addLog(LOG_LEVEL_DEBUG, F("Victron: Start counting for checksum"));
          #  endif // ifndef BUILD_NO_DEBUG
        }
        # endif // if P176_HANDLE_CHECKSUM
      } else
      if (ch == '\t') {
        # if P176_HANDLE_CHECKSUM

        if (equals(dataLine, F("Checksum"))) {
          if (Checksum_state_e::Counting == _checksumState) {
            _checksumState = Checksum_state_e::ValidateNext;
            #  ifndef BUILD_NO_DEBUG
            addLog(LOG_LEVEL_INFO, F("Victron: Validate next checksum"));
            #  endif // ifndef BUILD_NO_DEBUG
          } else {
            _checksumState = Checksum_state_e::Starting;
          }
        }
        # endif // if P176_HANDLE_CHECKSUM
        dataLine += static_cast<char>(ch); // append after
      } else {
        dataLine += static_cast<char>(ch);
      }

      # if P176_HANDLE_CHECKSUM

      if (Checksum_state_e::Validating == _checksumState) {
        if (_checksum != 0) {
          _checksumState = Checksum_state_e::Error; // Error if resulting checksum isn't 0
          _checksumErrors++;
          addLog(LOG_LEVEL_ERROR, strformat(F("Victron: Checksum error, expected 0 but counted %d"), _checksum));
        } else {
          _checksumState  = Checksum_state_e::Starting;
          _checksumErrors = 0;
          addLog(LOG_LEVEL_INFO, strformat(F("Victron: Checksum validated Ok at %d"), _checksum));
        }

        #  if P176_FAIL_CHECKSUM

        if ((Checksum_state_e::Error != _checksumState) || !_failChecksum) {
          moveTempToData();
        }
        #  endif // if P176_FAIL_CHECKSUM
      }
      # endif // if P176_HANDLE_CHECKSUM

      if (validGpio(_ledPin)) {
        DIRECT_pinWrite(_ledPin, _ledInverted ? 1 : 0);
      }

      timeOut = _rxWait; // if serial received, reset timeout counter
    } else {
      if (timeOut <= 0) {
        if ((_rxWait > 0) && (maxExtend > 0)) {
          timeOut = _rxWait;
          maxExtend--;
        } else {
          break;
        }
      }
      delay(1);
      --timeOut;
    }
  } while (true);
}

/*****************************************************
* processBuffer
*****************************************************/
void P176_data_struct::processBuffer(const String& message) {
  if (!Settings.UseRules || message.isEmpty()) { return; }
  const String key   = parseString(message, 1, '\t');
  const String value = parseStringToEndKeepCase(message, 2, '\t');

  # if P176_DEBUG
  addLog(LOG_LEVEL_INFO, strformat(F("P176 : Processing data '%s\t%s', checksum: %d"), key.c_str(), value.c_str(), _checksum));
  # endif // if P176_DEBUG

  if (!key.isEmpty() && !value.isEmpty() && !equals(key, F("checksum"))) {
    # if P176_FAIL_CHECKSUM
    _temp[key] = value;
    # else // if P176_FAIL_CHECKSUM
    _data[key] = value;
    # endif // if P176_FAIL_CHECKSUM
  }
}

# if P176_FAIL_CHECKSUM

/*****************************************************
* moveTempToData
*****************************************************/
void P176_data_struct::moveTempToData() {
  for (auto it = _temp.begin(); it != _temp.end(); ++it) {
    _data[it->first] = it->second;
  }
  _temp.clear();
  #  if P176_DEBUG
  addLog(LOG_LEVEL_INFO, F("P176 : Moving _temp to _data"));
  #  endif // if P176_DEBUG
}

# endif // if P176_FAIL_CHECKSUM

#endif // ifdef USES_P176
