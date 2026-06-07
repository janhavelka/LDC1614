/// @file CommandTable.h
/// @brief Register addresses and bit definitions for LDC1614/LDC1612
#pragma once

#include <cstdint>

namespace LDC1614 {
namespace cmd {

// ============================================================================
// Register Addresses (all 16-bit wide)
// ============================================================================

static constexpr uint8_t REG_DATA0_MSB       = 0x00;  ///< Ch 0 MSB conversion result + error flags (R)
static constexpr uint8_t REG_DATA0_LSB       = 0x01;  ///< Ch 0 LSB conversion result (R)
static constexpr uint8_t REG_DATA1_MSB       = 0x02;  ///< Ch 1 MSB conversion result + error flags (R)
static constexpr uint8_t REG_DATA1_LSB       = 0x03;  ///< Ch 1 LSB conversion result (R)
static constexpr uint8_t REG_DATA2_MSB       = 0x04;  ///< Ch 2 MSB result + error flags (LDC1614 only) (R)
static constexpr uint8_t REG_DATA2_LSB       = 0x05;  ///< Ch 2 LSB result (LDC1614 only) (R)
static constexpr uint8_t REG_DATA3_MSB       = 0x06;  ///< Ch 3 MSB result + error flags (LDC1614 only) (R)
static constexpr uint8_t REG_DATA3_LSB       = 0x07;  ///< Ch 3 LSB result (LDC1614 only) (R)

static constexpr uint8_t REG_RCOUNT0         = 0x08;  ///< Ch 0 Reference Count (R/W)
static constexpr uint8_t REG_RCOUNT1         = 0x09;  ///< Ch 1 Reference Count (R/W)
static constexpr uint8_t REG_RCOUNT2         = 0x0A;  ///< Ch 2 Reference Count (R/W, LDC1614 only)
static constexpr uint8_t REG_RCOUNT3         = 0x0B;  ///< Ch 3 Reference Count (R/W, LDC1614 only)

static constexpr uint8_t REG_OFFSET0         = 0x0C;  ///< Ch 0 Conversion Offset (R/W)
static constexpr uint8_t REG_OFFSET1         = 0x0D;  ///< Ch 1 Conversion Offset (R/W)
static constexpr uint8_t REG_OFFSET2         = 0x0E;  ///< Ch 2 Conversion Offset (R/W, LDC1614 only)
static constexpr uint8_t REG_OFFSET3         = 0x0F;  ///< Ch 3 Conversion Offset (R/W, LDC1614 only)

static constexpr uint8_t REG_SETTLECOUNT0    = 0x10;  ///< Ch 0 Settling Reference Count (R/W)
static constexpr uint8_t REG_SETTLECOUNT1    = 0x11;  ///< Ch 1 Settling Reference Count (R/W)
static constexpr uint8_t REG_SETTLECOUNT2    = 0x12;  ///< Ch 2 Settling Reference Count (R/W, LDC1614 only)
static constexpr uint8_t REG_SETTLECOUNT3    = 0x13;  ///< Ch 3 Settling Reference Count (R/W, LDC1614 only)

static constexpr uint8_t REG_CLOCK_DIVIDERS0 = 0x14;  ///< Ch 0 Reference & Sensor Dividers (R/W)
static constexpr uint8_t REG_CLOCK_DIVIDERS1 = 0x15;  ///< Ch 1 Reference & Sensor Dividers (R/W)
static constexpr uint8_t REG_CLOCK_DIVIDERS2 = 0x16;  ///< Ch 2 Reference & Sensor Dividers (R/W, LDC1614 only)
static constexpr uint8_t REG_CLOCK_DIVIDERS3 = 0x17;  ///< Ch 3 Reference & Sensor Dividers (R/W, LDC1614 only)

static constexpr uint8_t REG_STATUS          = 0x18;  ///< Device Status Report (R)
static constexpr uint8_t REG_ERROR_CONFIG    = 0x19;  ///< Error Reporting Configuration (R/W)
static constexpr uint8_t REG_CONFIG          = 0x1A;  ///< Conversion Configuration (R/W)
static constexpr uint8_t REG_MUX_CONFIG      = 0x1B;  ///< Channel Multiplexing Configuration (R/W)
static constexpr uint8_t REG_RESET_DEV       = 0x1C;  ///< Reset Device (R/W)

static constexpr uint8_t REG_DRIVE_CURRENT0  = 0x1E;  ///< Ch 0 Sensor Current Drive (R/W)
static constexpr uint8_t REG_DRIVE_CURRENT1  = 0x1F;  ///< Ch 1 Sensor Current Drive (R/W)
static constexpr uint8_t REG_DRIVE_CURRENT2  = 0x20;  ///< Ch 2 Sensor Current Drive (R/W, LDC1614 only)
static constexpr uint8_t REG_DRIVE_CURRENT3  = 0x21;  ///< Ch 3 Sensor Current Drive (R/W, LDC1614 only)

static constexpr uint8_t REG_MANUFACTURER_ID = 0x7E;  ///< Manufacturer ID (R) = 0x5449
static constexpr uint8_t REG_DEVICE_ID       = 0x7F;  ///< Device ID (R) = 0x3055

// ============================================================================
// Default Register Values
// ============================================================================

static constexpr uint16_t RCOUNT_DEFAULT         = 0x0080;
static constexpr uint16_t OFFSET_DEFAULT         = 0x0000;
static constexpr uint16_t SETTLECOUNT_DEFAULT    = 0x0000;
static constexpr uint16_t CLOCK_DIVIDERS_DEFAULT = 0x0000;
static constexpr uint16_t STATUS_DEFAULT         = 0x0000;
static constexpr uint16_t ERROR_CONFIG_DEFAULT   = 0x0000;
static constexpr uint16_t CONFIG_DEFAULT         = 0x2801;
static constexpr uint16_t MUX_CONFIG_DEFAULT     = 0x020F;
static constexpr uint16_t DRIVE_CURRENT_DEFAULT  = 0x0000;

static constexpr uint16_t MANUFACTURER_ID_VALUE  = 0x5449;  ///< "TI" in ASCII
static constexpr uint16_t DEVICE_ID_VALUE        = 0x3055;  ///< Shared LDC1612/LDC1614

// ============================================================================
// DATAx_MSB Bit Masks (per-channel error flags + data)
// ============================================================================

static constexpr uint16_t MASK_DATA_ERR_UR   = 0x8000;  ///< Under-range error flag
static constexpr uint16_t MASK_DATA_ERR_OR   = 0x4000;  ///< Over-range error flag
static constexpr uint16_t MASK_DATA_ERR_WD   = 0x2000;  ///< Watchdog timeout error flag
static constexpr uint16_t MASK_DATA_ERR_AE   = 0x1000;  ///< Amplitude error flag (OR of high+low)
static constexpr uint16_t MASK_DATA_MSB_DATA = 0x0FFF;  ///< 12 MSBs of 28-bit result [27:16]

static constexpr uint8_t BIT_DATA_ERR_UR = 15;
static constexpr uint8_t BIT_DATA_ERR_OR = 14;
static constexpr uint8_t BIT_DATA_ERR_WD = 13;
static constexpr uint8_t BIT_DATA_ERR_AE = 12;

// ============================================================================
// CLOCK_DIVIDERSx Bit Masks
// ============================================================================

static constexpr uint16_t MASK_FIN_DIVIDER  = 0xF000;  ///< Sensor input frequency divider [15:12]
static constexpr uint16_t MASK_FREF_DIVIDER = 0x03FF;  ///< Reference clock divider [9:0]

static constexpr uint8_t BIT_FIN_DIVIDER  = 12;
static constexpr uint8_t BIT_FREF_DIVIDER = 0;

// ============================================================================
// STATUS Register (0x18) Bit Masks
// ============================================================================

static constexpr uint16_t MASK_STATUS_ERR_CHAN      = 0xC000;  ///< Error channel [15:14]
static constexpr uint16_t MASK_STATUS_ERR_UR        = 0x2000;  ///< Under-range error (sticky)
static constexpr uint16_t MASK_STATUS_ERR_OR        = 0x1000;  ///< Over-range error (sticky)
static constexpr uint16_t MASK_STATUS_ERR_WD        = 0x0800;  ///< Watchdog timeout error (sticky)
static constexpr uint16_t MASK_STATUS_ERR_AHE       = 0x0400;  ///< Amplitude high error (sticky)
static constexpr uint16_t MASK_STATUS_ERR_ALE       = 0x0200;  ///< Amplitude low error (sticky)
static constexpr uint16_t MASK_STATUS_ERR_ZC        = 0x0100;  ///< Zero count error (sticky)
static constexpr uint16_t MASK_STATUS_DRDY          = 0x0040;  ///< Data Ready flag
static constexpr uint16_t MASK_STATUS_UNREADCONV0   = 0x0008;  ///< Ch 0 unread conversion
static constexpr uint16_t MASK_STATUS_UNREADCONV1   = 0x0004;  ///< Ch 1 unread conversion
static constexpr uint16_t MASK_STATUS_UNREADCONV2   = 0x0002;  ///< Ch 2 unread conversion
static constexpr uint16_t MASK_STATUS_UNREADCONV3   = 0x0001;  ///< Ch 3 unread conversion

static constexpr uint8_t BIT_STATUS_ERR_CHAN = 14;
static constexpr uint8_t BIT_STATUS_ERR_UR  = 13;
static constexpr uint8_t BIT_STATUS_ERR_OR  = 12;
static constexpr uint8_t BIT_STATUS_ERR_WD  = 11;
static constexpr uint8_t BIT_STATUS_ERR_AHE = 10;
static constexpr uint8_t BIT_STATUS_ERR_ALE = 9;
static constexpr uint8_t BIT_STATUS_ERR_ZC  = 8;
static constexpr uint8_t BIT_STATUS_DRDY    = 6;

// ============================================================================
// ERROR_CONFIG Register (0x19) Bit Masks
// ============================================================================

static constexpr uint16_t MASK_ERRCFG_UR_ERR2OUT  = 0x8000;  ///< Under-range → DATAx_MSB
static constexpr uint16_t MASK_ERRCFG_OR_ERR2OUT  = 0x4000;  ///< Over-range → DATAx_MSB
static constexpr uint16_t MASK_ERRCFG_WD_ERR2OUT  = 0x2000;  ///< Watchdog → DATAx_MSB
static constexpr uint16_t MASK_ERRCFG_AH_ERR2OUT  = 0x1000;  ///< Amplitude high → DATAx_MSB
static constexpr uint16_t MASK_ERRCFG_AL_ERR2OUT  = 0x0800;  ///< Amplitude low → DATAx_MSB
static constexpr uint16_t MASK_ERRCFG_UR_ERR2INT  = 0x0080;  ///< Under-range → INTB + STATUS
static constexpr uint16_t MASK_ERRCFG_OR_ERR2INT  = 0x0040;  ///< Over-range → INTB + STATUS
static constexpr uint16_t MASK_ERRCFG_WD_ERR2INT  = 0x0020;  ///< Watchdog → INTB + STATUS
static constexpr uint16_t MASK_ERRCFG_AH_ERR2INT  = 0x0010;  ///< Amplitude high → INTB + STATUS
static constexpr uint16_t MASK_ERRCFG_AL_ERR2INT  = 0x0008;  ///< Amplitude low → INTB + STATUS
static constexpr uint16_t MASK_ERRCFG_ZC_ERR2INT  = 0x0004;  ///< Zero count → INTB + STATUS
static constexpr uint16_t MASK_ERRCFG_DRDY_2INT   = 0x0001;  ///< Data ready → INTB + STATUS

/// ERROR_CONFIG bits supported by LDC1612/LDC1614. Reserved bits must stay 0.
static constexpr uint16_t MASK_ERRCFG_ALLOWED =
    MASK_ERRCFG_UR_ERR2OUT | MASK_ERRCFG_OR_ERR2OUT |
    MASK_ERRCFG_WD_ERR2OUT | MASK_ERRCFG_AH_ERR2OUT |
    MASK_ERRCFG_AL_ERR2OUT | MASK_ERRCFG_UR_ERR2INT |
    MASK_ERRCFG_OR_ERR2INT | MASK_ERRCFG_WD_ERR2INT |
    MASK_ERRCFG_AH_ERR2INT | MASK_ERRCFG_AL_ERR2INT |
    MASK_ERRCFG_ZC_ERR2INT | MASK_ERRCFG_DRDY_2INT;

// ============================================================================
// CONFIG Register (0x1A) Bit Masks
// ============================================================================

static constexpr uint16_t MASK_CFG_ACTIVE_CHAN        = 0xC000;  ///< Active channel [15:14]
static constexpr uint16_t MASK_CFG_SLEEP_MODE_EN      = 0x2000;  ///< Sleep mode enable
static constexpr uint16_t MASK_CFG_RP_OVERRIDE_EN     = 0x1000;  ///< RP override enable
static constexpr uint16_t MASK_CFG_SENSOR_ACTIVATE_SEL = 0x0800; ///< Sensor activation mode
static constexpr uint16_t MASK_CFG_AUTO_AMP_DIS       = 0x0400;  ///< Auto amplitude disable
static constexpr uint16_t MASK_CFG_REF_CLK_SRC        = 0x0200;  ///< Reference clock source
static constexpr uint16_t MASK_CFG_INTB_DIS           = 0x0080;  ///< INTB disable
static constexpr uint16_t MASK_CFG_HIGH_CURRENT_DRV   = 0x0040;  ///< High current drive (Ch0 only)
static constexpr uint16_t MASK_CFG_RESERVED_LOW       = 0x003F;  ///< Reserved [5:0], must be 0x01

static constexpr uint8_t BIT_CFG_ACTIVE_CHAN        = 14;
static constexpr uint8_t BIT_CFG_SLEEP_MODE_EN      = 13;
static constexpr uint8_t BIT_CFG_RP_OVERRIDE_EN     = 12;
static constexpr uint8_t BIT_CFG_SENSOR_ACTIVATE_SEL = 11;
static constexpr uint8_t BIT_CFG_AUTO_AMP_DIS       = 10;
static constexpr uint8_t BIT_CFG_REF_CLK_SRC        = 9;
static constexpr uint8_t BIT_CFG_INTB_DIS           = 7;
static constexpr uint8_t BIT_CFG_HIGH_CURRENT_DRV   = 6;

/// CONFIG reserved bits [5:0] must be set to 0b000001
static constexpr uint16_t CONFIG_RESERVED_VALUE = 0x0001;

// ============================================================================
// MUX_CONFIG Register (0x1B) Bit Masks
// ============================================================================

static constexpr uint16_t MASK_MUX_AUTOSCAN_EN  = 0x8000;  ///< Auto-scan enable
static constexpr uint16_t MASK_MUX_RR_SEQUENCE  = 0x6000;  ///< Round-robin sequence [14:13]
static constexpr uint16_t MASK_MUX_RESERVED     = 0x1FF8;  ///< Reserved [12:3], must be 0x0208
static constexpr uint16_t MASK_MUX_DEGLITCH     = 0x0007;  ///< Deglitch filter bandwidth [2:0]

static constexpr uint8_t BIT_MUX_AUTOSCAN_EN = 15;
static constexpr uint8_t BIT_MUX_RR_SEQUENCE = 13;
static constexpr uint8_t BIT_MUX_DEGLITCH    = 0;

/// MUX_CONFIG reserved bits [12:3] must be set to 0b0001000001 = 0x0208
static constexpr uint16_t MUX_CONFIG_RESERVED_VALUE = 0x0208;

// ============================================================================
// RESET_DEV Register (0x1C)
// ============================================================================

static constexpr uint16_t MASK_RESET_DEV = 0x8000;  ///< Write 1 to reset; always reads 0

// ============================================================================
// DRIVE_CURRENTx Register Bit Masks
// ============================================================================

static constexpr uint16_t MASK_IDRIVE      = 0xF800;  ///< Sensor drive current [15:11], 5-bit
static constexpr uint16_t MASK_INIT_IDRIVE = 0x07C0;  ///< Initial drive current [10:6], read-only

static constexpr uint8_t BIT_IDRIVE      = 11;
static constexpr uint8_t BIT_INIT_IDRIVE = 6;

// ============================================================================
// Deglitch Filter Values
// ============================================================================
// Datasheet note: the MUX_CONFIG field description and reset default encode
// 33 MHz as b111, while the application deglitch table has a conflicting b011
// entry. The driver follows MUX_CONFIG field text/default until TI errata or
// hardware validation proves otherwise.

static constexpr uint16_t DEGLITCH_1MHZ  = 0x0001;  ///< 1.0 MHz bandwidth
static constexpr uint16_t DEGLITCH_3MHZ  = 0x0004;  ///< 3.3 MHz bandwidth
static constexpr uint16_t DEGLITCH_10MHZ = 0x0005;  ///< 10 MHz bandwidth
static constexpr uint16_t DEGLITCH_33MHZ = 0x0007;  ///< 33 MHz bandwidth

// ============================================================================
// Round-Robin Sequence Values
// ============================================================================

static constexpr uint16_t RR_SEQ_CH0_CH1          = 0x0000;  ///< Ch0, Ch1
static constexpr uint16_t RR_SEQ_CH0_CH1_CH2      = 0x2000;  ///< Ch0, Ch1, Ch2 (LDC1614)
static constexpr uint16_t RR_SEQ_CH0_CH1_CH2_CH3  = 0x4000;  ///< Ch0, Ch1, Ch2, Ch3 (LDC1614)

// ============================================================================
// Limits and Constraints
// ============================================================================

static constexpr uint16_t RCOUNT_MIN          = 0x0005;  ///< Minimum usable RCOUNT value
static constexpr uint16_t RCOUNT_MAX          = 0xFFFF;  ///< Maximum RCOUNT for full 28-bit resolution
static constexpr uint8_t  FIN_DIVIDER_MIN     = 1;       ///< Minimum sensor frequency divider
static constexpr uint8_t  FIN_DIVIDER_MAX     = 15;      ///< Maximum sensor frequency divider
static constexpr uint16_t FREF_DIVIDER_MIN    = 1;       ///< Minimum reference clock divider
static constexpr uint16_t FREF_DIVIDER_MAX    = 0x03FF;  ///< Maximum reference clock divider
static constexpr uint8_t  IDRIVE_MIN          = 0;       ///< Minimum drive current index
static constexpr uint8_t  IDRIVE_MAX          = 31;      ///< Maximum drive current index
static constexpr uint8_t  MAX_CHANNELS        = 4;       ///< Maximum number of channels

// ============================================================================
// Per-Channel Register Offsets
// ============================================================================

/// Get DATAx_MSB register address for channel 0..3
static constexpr uint8_t regDataMsb(uint8_t ch) { return static_cast<uint8_t>(REG_DATA0_MSB + ch * 2); }

/// Get DATAx_LSB register address for channel 0..3
static constexpr uint8_t regDataLsb(uint8_t ch) { return static_cast<uint8_t>(REG_DATA0_LSB + ch * 2); }

/// Get RCOUNTx register address for channel 0..3
static constexpr uint8_t regRcount(uint8_t ch) { return static_cast<uint8_t>(REG_RCOUNT0 + ch); }

/// Get OFFSETx register address for channel 0..3
static constexpr uint8_t regOffset(uint8_t ch) { return static_cast<uint8_t>(REG_OFFSET0 + ch); }

/// Get SETTLECOUNTx register address for channel 0..3
static constexpr uint8_t regSettleCount(uint8_t ch) { return static_cast<uint8_t>(REG_SETTLECOUNT0 + ch); }

/// Get CLOCK_DIVIDERSx register address for channel 0..3
static constexpr uint8_t regClockDividers(uint8_t ch) { return static_cast<uint8_t>(REG_CLOCK_DIVIDERS0 + ch); }

/// Get DRIVE_CURRENTx register address for channel 0..3
static constexpr uint8_t regDriveCurrent(uint8_t ch) { return static_cast<uint8_t>(REG_DRIVE_CURRENT0 + ch); }

} // namespace cmd
} // namespace LDC1614
