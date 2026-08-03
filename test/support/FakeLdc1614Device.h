#pragma once

#include <cstddef>
#include <cstdint>

#include "LDC1614/CommandTable.h"
#include "LDC1614/Status.h"

namespace ldc1614_test {

namespace cmd = LDC1614::cmd;

/// Fixed-memory behavioral LDC1612/LDC1614 transport fixture.
///
/// The fixture deliberately models the destructive DATA/STATUS behavior that a
/// flat register array misses: DATAx_MSB latches the matching LSB, consumes the
/// channel's unread indication, and can clear channel error/INTB evidence;
/// STATUS returns a snapshot and clears sticky error/INTB evidence.
struct FakeLdc1614Device {
  using Status = LDC1614::Status;
  using Err = LDC1614::Err;

  static constexpr uint8_t NO_REG = 0xFF;
  static constexpr uint16_t STATUS_ERROR_MASK =
      cmd::MASK_STATUS_ERR_UR | cmd::MASK_STATUS_ERR_OR |
      cmd::MASK_STATUS_ERR_WD | cmd::MASK_STATUS_ERR_AHE |
      cmd::MASK_STATUS_ERR_ALE | cmd::MASK_STATUS_ERR_ZC;
  static constexpr uint16_t DATA_ERROR_MASK =
      cmd::MASK_DATA_ERR_UR | cmd::MASK_DATA_ERR_OR |
      cmd::MASK_DATA_ERR_WD | cmd::MASK_DATA_ERR_AE;
  static constexpr uint8_t MAX_TRANSFERS = 192;

  enum class TransferKind : uint8_t { WRITE, READ };

  struct Transfer {
    TransferKind kind = TransferKind::WRITE;
    uint8_t address = 0;
    uint8_t reg = 0;
    uint16_t value = 0;
    uint32_t timeoutMs = 0;
  };

  uint8_t acceptedAddress = 0x2A;
  uint16_t reg[128] = {};
  uint16_t liveMsb[4] = {};
  uint16_t liveLsb[4] = {};
  uint16_t shadowMsb[4] = {};
  uint16_t shadowLsb[4] = {};
  bool shadowValid[4] = {};
  uint8_t unreadMask = 0;
  uint16_t stickyStatusErrors = 0;
  uint8_t errorChannel = 0;
  bool intbAsserted = false;

  Transfer transferLog[MAX_TRANSFERS] = {};
  uint16_t transferCalls = 0;
  uint16_t writeCalls = 0;
  uint16_t readCalls = 0;
  uint16_t intbCalls = 0;
  uint8_t transferLogCount = 0;

  uint16_t failTransfer = 0;
  Status failStatus = Status::Error(Err::I2C_TIMEOUT, "forced transfer failure", -7001);
  bool commitWriteBeforeFailure = false;

  uint16_t injectAfterTransfer = 0;
  uint8_t injectChannel = 0;
  uint32_t injectRaw28 = 0;
  uint16_t injectDataErrors = 0;
  uint16_t injectStatusErrors = 0;
  bool injectionPending = false;

  FakeLdc1614Device() { resetDevice(); }

  void resetDevice() {
    for (uint16_t& value : reg) {
      value = 0;
    }
    reg[cmd::REG_MANUFACTURER_ID] = cmd::MANUFACTURER_ID_VALUE;
    reg[cmd::REG_DEVICE_ID] = cmd::DEVICE_ID_VALUE;
    reg[cmd::REG_CONFIG] = cmd::CONFIG_DEFAULT;
    reg[cmd::REG_MUX_CONFIG] = cmd::MUX_CONFIG_DEFAULT;
    reg[cmd::REG_ERROR_CONFIG] = cmd::ERROR_CONFIG_DEFAULT;
    for (uint8_t ch = 0; ch < 4; ++ch) {
      liveMsb[ch] = 0;
      liveLsb[ch] = 0;
      shadowMsb[ch] = 0;
      shadowLsb[ch] = 0;
      shadowValid[ch] = false;
    }
    unreadMask = 0;
    stickyStatusErrors = 0;
    errorChannel = 0;
    intbAsserted = false;
  }

  void clearIo() {
    transferCalls = 0;
    writeCalls = 0;
    readCalls = 0;
    intbCalls = 0;
    transferLogCount = 0;
    failTransfer = 0;
    commitWriteBeforeFailure = false;
    injectionPending = false;
  }

  void failOnTransfer(uint16_t transfer, const Status& status,
                      bool commitBeforeFailure = false) {
    failTransfer = transfer;
    failStatus = status;
    commitWriteBeforeFailure = commitBeforeFailure;
  }

  void injectConversion(uint8_t ch, uint32_t raw28,
                        uint16_t dataErrors = 0,
                        uint16_t statusErrors = 0) {
    if (ch >= 4) {
      return;
    }
    raw28 &= 0x0FFFFFFFU;
    liveMsb[ch] = static_cast<uint16_t>(((raw28 >> 16U) & 0x0FFFU) |
                                        (dataErrors & DATA_ERROR_MASK));
    liveLsb[ch] = static_cast<uint16_t>(raw28 & 0xFFFFU);
    unreadMask |= static_cast<uint8_t>(1U << ch);
    if ((statusErrors & STATUS_ERROR_MASK) != 0U) {
      stickyStatusErrors = static_cast<uint16_t>(statusErrors & STATUS_ERROR_MASK);
      errorChannel = ch;
    }
    const uint16_t errorConfig = reg[cmd::REG_ERROR_CONFIG];
    const bool drdyRouted = (errorConfig & cmd::MASK_ERRCFG_DRDY_2INT) != 0U;
    const bool errorRouted = (errorConfig & 0x00FCU) != 0U;
    const bool intbDisabled = (reg[cmd::REG_CONFIG] & cmd::MASK_CFG_INTB_DIS) != 0U;
    if (!intbDisabled && ((drdyRouted && unreadMask != 0U) ||
                         (errorRouted && stickyStatusErrors != 0U))) {
      intbAsserted = true;
    }
  }

  void scheduleConversionAfter(uint16_t transfer, uint8_t ch, uint32_t raw28,
                               uint16_t dataErrors = 0,
                               uint16_t statusErrors = 0) {
    injectAfterTransfer = transfer;
    injectChannel = ch;
    injectRaw28 = raw28;
    injectDataErrors = dataErrors;
    injectStatusErrors = statusErrors;
    injectionPending = true;
  }

  uint16_t statusValue() const {
    uint16_t value = stickyStatusErrors;
    if (stickyStatusErrors != 0U) {
      value |= static_cast<uint16_t>(errorChannel) << cmd::BIT_STATUS_ERR_CHAN;
    }
    if (unreadMask != 0U &&
        (reg[cmd::REG_ERROR_CONFIG] & cmd::MASK_ERRCFG_DRDY_2INT) != 0U) {
      value |= cmd::MASK_STATUS_DRDY;
    }
    for (uint8_t ch = 0; ch < 4; ++ch) {
      if ((unreadMask & static_cast<uint8_t>(1U << ch)) != 0U) {
        value |= unreadStatusBit(ch);
      }
    }
    return value;
  }

  static Status write(uint8_t address, const uint8_t* data, size_t len,
                      uint32_t timeoutMs, void* user) {
    auto* self = static_cast<FakeLdc1614Device*>(user);
    if (self == nullptr || data == nullptr || len != 3U) {
      return Status::Error(Err::INVALID_PARAM, "invalid fake write");
    }
    const uint8_t targetReg = data[0];
    const uint16_t value = static_cast<uint16_t>(
        (static_cast<uint16_t>(data[1]) << 8U) | data[2]);
    self->record(TransferKind::WRITE, address, targetReg, value, timeoutMs);
    self->writeCalls++;

    if (address != self->acceptedAddress) {
      self->runScheduledInjection();
      return Status::Error(Err::I2C_NACK_ADDR, "unexpected fake address", address);
    }

    const bool fail = self->failTransfer != 0U &&
                      self->transferCalls == self->failTransfer;
    if (!fail || self->commitWriteBeforeFailure) {
      self->applyWrite(targetReg, value);
    }
    self->runScheduledInjection();
    return fail ? self->failStatus : Status::Ok();
  }

  static Status writeRead(uint8_t address, const uint8_t* txData, size_t txLen,
                          uint8_t* rxData, size_t rxLen, uint32_t timeoutMs,
                          void* user) {
    auto* self = static_cast<FakeLdc1614Device*>(user);
    if (self == nullptr || txData == nullptr || txLen != 1U ||
        rxData == nullptr || rxLen != 2U) {
      return Status::Error(Err::INVALID_PARAM, "invalid fake read");
    }

    const uint8_t targetReg = txData[0];
    self->record(TransferKind::READ, address, targetReg, 0U, timeoutMs);
    self->readCalls++;
    if (address != self->acceptedAddress) {
      self->runScheduledInjection();
      return Status::Error(Err::I2C_NACK_ADDR, "unexpected fake address", address);
    }

    const uint16_t value = self->readValue(targetReg);
    if (self->transferLogCount != 0U) {
      self->transferLog[self->transferLogCount - 1U].value = value;
    }
    const bool fail = self->failTransfer != 0U &&
                      self->transferCalls == self->failTransfer;
    if (!fail) {
      rxData[0] = static_cast<uint8_t>((value >> 8U) & 0xFFU);
      rxData[1] = static_cast<uint8_t>(value & 0xFFU);
      self->applyReadSideEffects(targetReg);
    }
    self->runScheduledInjection();
    return fail ? self->failStatus : Status::Ok();
  }

  static Status readIntb(bool& asserted, void* user) {
    auto* self = static_cast<FakeLdc1614Device*>(user);
    if (self == nullptr) {
      asserted = false;
      return Status::Error(Err::INVALID_PARAM, "invalid fake INTB context");
    }
    ++self->intbCalls;
    asserted = self->intbAsserted;
    return Status::Ok();
  }

private:
  static uint16_t unreadStatusBit(uint8_t ch) {
    return static_cast<uint16_t>(1U << (3U - ch));
  }

  static int8_t dataMsbChannel(uint8_t targetReg) {
    for (uint8_t ch = 0; ch < 4; ++ch) {
      if (targetReg == cmd::regDataMsb(ch)) {
        return static_cast<int8_t>(ch);
      }
    }
    return -1;
  }

  static int8_t dataLsbChannel(uint8_t targetReg) {
    for (uint8_t ch = 0; ch < 4; ++ch) {
      if (targetReg == cmd::regDataLsb(ch)) {
        return static_cast<int8_t>(ch);
      }
    }
    return -1;
  }

  uint16_t readValue(uint8_t targetReg) {
    if (targetReg == cmd::REG_STATUS) {
      return statusValue();
    }
    const int8_t msbCh = dataMsbChannel(targetReg);
    if (msbCh >= 0) {
      const uint8_t ch = static_cast<uint8_t>(msbCh);
      shadowMsb[ch] = liveMsb[ch];
      shadowLsb[ch] = liveLsb[ch];
      shadowValid[ch] = true;
      return shadowMsb[ch];
    }
    const int8_t lsbCh = dataLsbChannel(targetReg);
    if (lsbCh >= 0) {
      const uint8_t ch = static_cast<uint8_t>(lsbCh);
      return shadowValid[ch] ? shadowLsb[ch] : 0U;
    }
    return targetReg < 128U ? reg[targetReg] : 0U;
  }

  void applyReadSideEffects(uint8_t targetReg) {
    if (targetReg == cmd::REG_STATUS) {
      stickyStatusErrors = 0;
      errorChannel = 0;
      intbAsserted = false;
      return;
    }
    const int8_t msbCh = dataMsbChannel(targetReg);
    if (msbCh < 0) {
      return;
    }
    const uint8_t ch = static_cast<uint8_t>(msbCh);
    unreadMask &= static_cast<uint8_t>(~(1U << ch));
    liveMsb[ch] &= static_cast<uint16_t>(~DATA_ERROR_MASK);
    if (stickyStatusErrors != 0U && errorChannel == ch) {
      stickyStatusErrors = 0;
      errorChannel = 0;
    }
    if (unreadMask == 0U && stickyStatusErrors == 0U) {
      intbAsserted = false;
    }
  }

  void applyWrite(uint8_t targetReg, uint16_t value) {
    if (targetReg == cmd::REG_RESET_DEV &&
        (value & cmd::MASK_RESET_DEV) != 0U) {
      resetDevice();
      return;
    }
    if (targetReg < 128U) {
      reg[targetReg] = value;
    }
    if (targetReg == cmd::REG_CONFIG &&
        (value & cmd::MASK_CFG_SLEEP_MODE_EN) != 0U) {
      unreadMask = 0;
      stickyStatusErrors = 0;
      errorChannel = 0;
      intbAsserted = false;
    }
  }

  void record(TransferKind kind, uint8_t address, uint8_t targetReg,
              uint16_t value, uint32_t timeoutMs) {
    transferCalls++;
    if (transferLogCount < MAX_TRANSFERS) {
      Transfer& transfer = transferLog[transferLogCount++];
      transfer.kind = kind;
      transfer.address = address;
      transfer.reg = targetReg;
      transfer.value = value;
      transfer.timeoutMs = timeoutMs;
    }
  }

  void runScheduledInjection() {
    if (injectionPending && transferCalls == injectAfterTransfer) {
      injectionPending = false;
      injectConversion(injectChannel, injectRaw28, injectDataErrors,
                       injectStatusErrors);
    }
  }
};

}  // namespace ldc1614_test
