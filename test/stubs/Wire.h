/// @file Wire.h
/// @brief Minimal Wire stub for native testing
#pragma once

#include <cstdint>
#include <cstddef>

class TwoWire {
public:
  void begin(int sda = -1, int scl = -1) { (void)sda; (void)scl; }
  void setClock(uint32_t freq) { (void)freq; }
  void setTimeOut(uint32_t timeoutMs) { (void)timeoutMs; }

  void beginTransmission(uint8_t addr) { _addr = addr; _txLen = 0; }
  size_t write(uint8_t data) {
    if (_txLen < sizeof(_txBuf)) { _txBuf[_txLen++] = data; return 1; }
    return 0;
  }
  size_t write(const uint8_t* data, size_t len) {
    size_t written = 0;
    for (size_t i = 0; i < len && _txLen < sizeof(_txBuf); i++) {
      _txBuf[_txLen++] = data[i];
      written++;
    }
    return written;
  }
  uint8_t endTransmission(bool stop = true) { (void)stop; return _endTransmissionResult; }

  size_t requestFrom(uint8_t addr, size_t len) {
    (void)addr;
    _rxLen = _forceRequestResult ? _requestFromResult : len;
    _rxIdx = 0;
    return _rxLen;
  }

  int available() {
    return (_rxIdx < _rxLen) ? static_cast<int>(_rxLen - _rxIdx) : 0;
  }
  int read() {
    if (_rxIdx < _rxLen) { return _rxBuf[_rxIdx++]; }
    return -1;
  }

  void setEndTransmissionResult(uint8_t result) { _endTransmissionResult = result; }
  void setRequestFromResult(size_t result) {
    _forceRequestResult = true;
    _requestFromResult = result;
  }
  void clearRequestFromResult() {
    _forceRequestResult = false;
    _requestFromResult = 0;
  }
  void setRxByte(size_t index, uint8_t value) {
    if (index < sizeof(_rxBuf)) {
      _rxBuf[index] = value;
    }
  }
  void resetTestState() {
    _addr = 0;
    _txLen = 0;
    _rxLen = 0;
    _rxIdx = 0;
    _endTransmissionResult = 0;
    _forceRequestResult = false;
    _requestFromResult = 0;
    for (uint8_t& byte : _txBuf) {
      byte = 0;
    }
    for (uint8_t& byte : _rxBuf) {
      byte = 0;
    }
  }

private:
  uint8_t _addr = 0;
  uint8_t _txBuf[32] = {};
  size_t _txLen = 0;
  uint8_t _rxBuf[32] = {};
  size_t _rxLen = 0;
  size_t _rxIdx = 0;
  uint8_t _endTransmissionResult = 0;
  bool _forceRequestResult = false;
  size_t _requestFromResult = 0;
};

extern TwoWire Wire;
