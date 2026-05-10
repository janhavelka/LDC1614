# LDC1312, LDC1314, LDC1612, LDC1614 Sensor Status Monitoring

- Source PDF: `../application_notes/sensor_status_monitoring.pdf`
- Extraction tool: pdfplumber
- Page count: 14
- SHA256: `e890f35f985029f582b9fd0492bea2bfa5309569b66859ab5396cb6be59d98ef`

## Page 1

LDC1312, LDC1314, LDC1612, LDC1614 Sensor
Status Monitoring
Application Report
Literature Number: SNOA959
October 2016

## Page 2

Contents
1 Reporting Mechanisms ......................................................................................................... 3
1.1 Conversion Output Register Behavior and Available Reports ..................................................... 4
1.2 Status Register Behavior and Available Reports .................................................................... 4
1.3 Reporting Errors and Status on the INTB Pin ....................................................................... 6
2 Reporting of Completed Conversions..................................................................................... 7
2.1 Unread Conversion ...................................................................................................... 7
2.2 Data Ready Reporting .................................................................................................. 7
2.3 Reading Data Without Using DRDY or CHx_UNREADCONV ..................................................... 7
3 Reporting of Errors and Warnings.......................................................................................... 8
3.1 Frequency Under-Range Errors........................................................................................ 8
3.2 Frequency Over-Range Errors ......................................................................................... 8
3.3 Watchdog Timeout Errors ............................................................................................... 8
3.4 Amplitude Warnings ..................................................................................................... 9
3.5 Zero Count Errors ........................................................................................................ 9
4 Summary of Relevant Register Tables .................................................................................. 10
4.1 Address 0x00, DATA_CH0 (LDC1312) .............................................................................. 10
4.2 Address 0x18, STATUS ............................................................................................... 10
4.3 Address 0x19, ERROR_CONFIG .................................................................................... 12
5 Conclusion ........................................................................................................................ 13
2 Table of Contents SNOA959–October 2016
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 3

Application Report
SNOA959 –October 2016
LDC1312, LDC1314, LDC1612, LDC1614 Sensor Status
Monitoring
Ben Kasemsadeh
ABSTRACT
TI’s multichannel inductance-to-digital converters (LDCs) LDC1612, LDC1614, LDC1312 and LDC1314
feature three different methods for reporting conversion status information including errors, warnings, and
completed conversion results. Information is available through the data registers, the status registers, and
the INTB pin of the device. This application note explains usage and interpretation of the information that
the LDC reports in detail.
1 Reporting Mechanisms
The LDC can detect and report on several device conditions. It provides flexibility in the error reporting
mechanism.
Errors can be reported in the following ways:
• by the four MSBs in the conversion output registers DATA_MSB_CHx.
• by the STATUS register.
• by asserting the INTB pin.
Summary tables with the DATA_CHx, STATUS and ERROR_CONFIG registers are shown in section
Section 4 for reference.
Table 1 summarizes which reporting options are available for each error and status condition.
Table 1. Error and Status Condition Reporting Options
DATA_CHx Reporting Status Register Reporting INTB Reporting
Condition Reported
Section 1.1 Section 1.2 Section 1.3
Data ready (DRDY) N/A Reported Set DRDY_2INT=1
Section 2.2
Unread conversion N/A Reported N/A
Section 2.1
Under-range error Set UR_ERR2OUT=1 Reported Set UR_ERR2INT=1
Section 3.1
Over-range error Set OR_ERR2OUT=1 Reported Set OR_ERR2INT=1
Section 3.2
Watchdog timeout error Set WD_ERR2OUT=1 Reported Set WD_ERR2INT=1
Section 3.3
Amplitude high error Set AH_ERR2OUT=1 (1) Reported Set AH_ERR2INT=1
Section 3.4
Amplitude low warning Set AL_ERR2OUT=1(1) Reported Set AL_ERR2INT=1
Section 3.4
Zero count error N/A Reported Set ZC_ERR2INT=1
Section 3.5
(1) If both ERROR_CONFIG.AH_ERR2OUT=1 and AL_ERR2OUT=1, the amplitude warning bit in CHx_ERR_AE will report a logic
OR of the amplitude warnings.
SNOA959–October 2016 LDC1312, LDC1314, LDC1612, LDC1614 Sensor Status Monitoring 3
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 4

Reporting Mechanisms www.ti.com
1.1 Conversion Output Register Behavior and Available Reports
This reporting method supports the following functions:
• Under-range errors
• Over-range errors
• Watchdog timeout errors
• Amplitude warnings
Any error bit set in the DATA_CHx register will be cleared by reading DATA_CHx. If that channel bit
caused the error or warning bit in STATUS to be set and INTB to be asserted, then the STATUS error bit
is cleared.
The error bits set in the DATA_CHx register are not sticky. An error or warning bit will be cleared if the
subsequent conversion of the corresponding channel completes without the particular condition.
If both ERROR_CONFIG.AH_ERR2OUT=1 and AL_ERR2OUT=1, the amplitude warning bit in
CHx_ERR_AE will report a logic OR of the amplitude warnings.
Table 2 shows the output of the DATA_CHx register for each error and status condition.
Table 2. Error and Status Condition Reporting Through the Output Data Register
Condition Reported DATA_CHx Reporting DATA_CHx Output (1) (2)
Data ready (DRDY) N/A 0x0XXX(3)
Unread conversion N/A 0x0XXX(3)
Under-range error Set UR_ERR2OUT=1 0x8000
Over-range error Set OR_ERR2OUT=1 0x4FFF
Watchdog timeout error Set WD_ERR2OUT=1 0x2000
Amplitude high error Set AH_ERR2OUT=1 (4) 0x1XXX (4)
Amplitude low warning Set AL_ERR2OUT=1(4) 0x1XXX (4)
Zero count error N/A 0x0000 or 0x8000
(1) This table uses the LDC1312 DATA_CHx register as an example. Refer to the LDC161x datasheet for the listing of the
equivalent DATA_CHx_MSB and DATA_CHx_LSB registers.
(2) Assuming only the indicated error occurs. If multiple error flags occur, then the leading four bits may be different
(3) If both ERROR_CONFIG.AH_ERR2OUT=1 and AL_ERR2OUT=1, the amplitude warning bit in CHx_ERR_AE will report a logic
OR of the amplitude warnings.
(4) Valid conversion data exists in DATAx
1.2 Status Register Behavior and Available Reports
This reporting method supports the following functions:
• Under-range errors
• Over-range errors
• Watchdog timeout errors
• Amplitude high errors
• Amplitude low warnings
• Zero count errors
• Data ready (DRDY)
• Unread conversion notification
The STATUS.ERR_CHAN records the channel that reported the error. If more than one channel reported
an error, the ERR_CHAN bit reports the first channel in which the error occurred. If errors from multiple
channels occur while using this technique, subsequent errors will not be reported, as shown in Figure 1.
4 LDC1312, LDC1314, LDC1612, LDC1614 Sensor Status Monitoring SNOA959–October 2016
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 5

www.ti.com Reporting Mechanisms
Error
CH0 Conversion
occurs!
Error
CH1 Conversion
occurs!
CH0 CH1
STATUS
Error Error
INTB
I2C Read Status
TIME
CH0 error is CH1 error is
not cleared not reported
Figure 1. CH0 Error is Not Cleared; Subsequent Error is Not Reported
To avoid missing errors from multiple channels, it is recommended to use INTB reporting in addition to
status reporting. This way, INTB will assert when an error occurs, the user can read it, and then INTB will
report again if another error occurs, as shown in Figure 2.
Error
CH0 Conversion
occurs!
Error
CH1 Conversion
occurs!
CH0 CH1
STATUS
Error Error
INTB
I2C Read Status Status
TIME
CH0 error is
CH1 error is
cleared by
reported
reading
correctly
STATUS
Figure 2. CH0 Error is Cleared; Subsequent Error is Reported Correctly
All bits in the STATUS register except for the Unread Conversion notification are sticky. The bits need to
be cleared by reading the STATUS register. Reading from the STATUS register also de-asserts INTB.
SNOA959–October 2016 LDC1312, LDC1314, LDC1612, LDC1614 Sensor Status Monitoring 5
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 6

Reporting Mechanisms www.ti.com
1.3 Reporting Errors and Status on the INTB Pin
Error and status registers can trigger an interrupt on the INTB pin. The following conditions must be met:
1. The error or status register must be unmasked by enabling the appropriate register bit in the
ERROR_CONFIG register
2. The INTB function must be enabled by setting CONFIG.INTB_DIS to 0
Reporting through the INTB pin supports the following functions:
• Under-range errors
• Over-range errors
• Watchdog timeout errors
• Amplitude high errors
• Amplitude low warnings
• Zero count errors
• Data ready (DRDY)
Interrupts are cleared by the following events:
1. Entering Sleep Mode
2. Power-on reset (POR)
3. Device enters Shutdown Mode (SD is asserted)
4. Software reset
5. I2C read of the STATUS register: Reading the STATUS register will clear any error status bit set in
STATUS along with the ERR_CHAN field and de-assert INTB
Setting register CONFIG.INTB_DIS to b1 disables the INTB function and holds the INTB pin high.
6 LDC1312, LDC1314, LDC1612, LDC1614 Sensor Status Monitoring SNOA959–October 2016
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 7

www.ti.com Reporting of Completed Conversions
2 Reporting of Completed Conversions
2.1 Unread Conversion
The LDC reports when a conversion has completed and the conversion result is available and can be read
from the DATA_CHx registers. The status register bit STATUS.CHx_UNREADCONV shows when a
conversion has occurred on a particular channel that has not been read yet. The register is cleared when
either the corresponding DATAx register or the Status register is read.
In multi-channel mode, this register can be used to identify a completed conversion result on a particular
channel without having to wait until the last conversion in the sequence is complete, as shown in Figure 3.
CH0 Conversion Conversion
CH1 Conversion Conversion
CH2 Conversion
CH3 Conversion
UNREAD_CONV
DRDY
TIME
Figure 3. UNREAD_CONV Flags Completed Conversions Without Waiting Until the End of the Cycle
2.2 Data Ready Reporting
When the device is in single-channel continuous conversion mode, which is set by
MUX_CONFIG.AUTOSCAN_EN = 0, data ready will occur upon the completion of each conversion.
When the device is in multi-channel (sequential) mode, which is set by MUX_CONFIG.AUTOSCAN_EN =
1, the data ready will occur on completion of the last conversion in a sequence. For example, if
MUX_CONFIG.RR_SEQUENCE=0, then data ready will occur when both Channel 0 and Channel 1
conversions are complete.
Data ready is reported:
• in the STATUS.DRDY field if ERROR_CONFIG.DRDY_2INT is set to b1.
• by asserting the INTB pin if ERROR_CONFIG.DRDY_2INT is set to b1.
Summary tables with the STATUS and ERROR_CONFIG registers are shown in section Section 4 for
reference.
2.3 Reading Data Without Using DRDY or CHx_UNREADCONV
The deterministic conversion time also allows data polling at a fixed interval instead of using DRDY. As
long as the microcontroller and the LDC are clocked from the same clock source, the conversion time can
be calculated as shown in section 'Multi-Channel and Single Channel Operation'' in the LDC161x
datasheet and the LDC131x datasheet
If it is not feasible to share clocks between the LDC and the microcontroller, repeated data reads may
occur unless the LDC conversion time is faster than the I2C read operation.
SNOA959–October 2016 LDC1312, LDC1314, LDC1612, LDC1614 Sensor Status Monitoring 7
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 8

Reporting of Errors and Warnings www.ti.com
3 Reporting of Errors and Warnings
3.1 Frequency Under-Range Errors
Frequency Under-range errors occur when the output code (DATAx) would be a negative number after
subtracting the offset value in the CHx_OFFSET register. If Frequency Under-range errors are occurring
on a specific channel, reducing the offset value for the channel applied by the OFFSET_CHx register can
resolve them. Under-range errors may also be addressed in some applications by increasing the value in
the CHx_RCOUNT.
When a Frequency Under-range error occurs:
• the LDC1312 and LDC1314 report a DATA[11:0] output of 0x000 for the channel which caused the
violation.
• the LDC1612 and LDC1614 report a DATA[27:0] output of 0x0000000 for the channel which caused
the violation.
Frequency Under-range errors are reported:
• the output register DATA_CHx of the appropriate channel if ERROR_CONFIG.UR_ERR2OUT is set to
b1.
• in the STATUS.ERR_UR field (bit 13) if ERROR_CONFIG.UR_ERR2INT is set to b1.
• by asserting the INTB pin if ERROR_CONFIG.UR_ERR2INT is set to b1.
3.2 Frequency Over-Range Errors
Frequency Over-range errors occur when the sensor frequency exceeds the reference frequency. When a
Frequency Over-range error occurs on a channel, the output code for the channel will be limited full-scale.
If Frequency Over-range errors are occurring on a specific channel, increasing the reference frequency or
decreasing the sensor frequency will resolve this issue. Frequency Over-range errors can also be
addressed by increasing specific channel’s sensor divider in CLOCK_DIVIDERS_CHx.CHx_FIN_DIVIDER
or by decreasing the specific channel’s reference divider setting in
CLOCK_DIVIDERS_CHx.CHx_FREF_DIVIDER.
When a Frequency Over-range error occurs:
• the LDC1312 and LDC1314 report a DATA[11:0] output of 0xFFF for the channel which caused the
violation.
• the LDC1612 and LDC1614 report a DATA[27:0] output of 0xFFFFFFF for the channel which caused
the violation.
Frequency Over-range errors are reported:
• in the output register DATA_CHx of the appropriate channel if ERROR_CONFIG.OR_ERR2OUT is set
to b1.
• in the STATUS.ERR_OR field (bit 12) if ERROR_CONFIG.OR_ERR2INT is set to b1.
• by asserting the INTB pin if ERROR_CONFIG.OR_ERR2INT is set to b1.
3.3 Watchdog Timeout Errors
Watchdog Timeout errors occur in continuous conversion mode when the sensor is no longer oscillating,
or if it is oscillating at a frequency lower than 250Hz. If a Watchdog timeout occurs, the sensor is reset.
The LDC will abort the current conversion, and attempt to restart the sensor on the active channel. If the
sensor resumes oscillation, then the data conversion will resume, and INTB will be de-asserted (if
reporting Watchdog errors is enabled in ERROR_CONFIG.WD_ERR2INT). If the sensor does not restart,
the LDC will issue repeated Watchdog timeout errors.
If a watchdog event occurs, the data read from DATA registers is invalid and must be ignored.
The sensor recovery time which is controlled by the watchdog is approximately 5.2ms – the LDC requires
that the sensor generate at least one oscillation in that time. If the conversion time is less than 5.2ms, then
one or many Zero Count errors will be generated before the Watchdog timeout error.
8 LDC1312, LDC1314, LDC1612, LDC1614 Sensor Status Monitoring SNOA959–October 2016
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 9

www.ti.com Reporting of Errors and Warnings
Watchdog Timeout errors are reported:
• in the output register DATA_CHx of the appropriate channel if ERROR_CONFIG.WD_ERR2OUT is set
to b1.
• in the STATUS.ERR_WD field (bit 11) if ERROR_CONFIG.WD_ERR2INT is set to b1.
• by asserting the INTB pin if ERROR_CONFIG.WD_ERR2INT is set to b1.
Watchdog timeout errors only occur in continuous mode. In sequential mode, zero-count errors and
amplitude warnings should be used to detect a stopped oscillation instead.
3.4 Amplitude Warnings
Amplitude warnings occur when the sensor amplitude is not within the required range when the
conversion begins. There are two types of amplitude warnings - a low amplitude warning and a high
amplitude error. This can be caused by an incorrect setting of the channel IDRIVE, or if the sensor
impedance is outside of the specified driving range of the LDC.
Refer to the application note Setting LDC1312/4, LDC1612/4, and LDC1101 Sensor Drive Configuration
for information on configuring the sensor oscillation amplitude correctly and avoiding Amplitude warnings.
The reporting of amplitude warnings is more useful during the development of a system rather than during
the normal operation. However, in certain circumstances, an amplitude warning may indicate that a
hardware fault with the sensor occurred, such as a physically disconnected sensor capacitor.
Amplitude High Errors are reported:
• in the output register DATA_CHx of the appropriate channel if ERROR_CONFIG.AH_ERR2OUT is set
to b1. Amplitude Low Warning and Amplitude High Errors are OR-ed together in this register bit.
• in the STATUS.ERR_AHE field (bit 10) if ERROR_CONFIG.AH_ERR_2INT is set to b1.
• by asserting the INTB pin if ERROR_CONFIG.AH_ERR2INT is set to b1 .
Amplitude Low Warnings are reported:
• in the output register DATA_CHx of the appropriate channel if ERROR_CONFIG.AL_ERR2OUT is set
to b1. Amplitude Low Warnings and Amplitude High Errors are OR-ed together in this register bit.
• in the STATUS.ERR_ALE field (bit 9) if ERROR_CONFIG.AL_ERR_2INT is set to b1.
• by asserting the INTB pin if ERROR_CONFIG.AL_ERR2INT is set to b1.
3.5 Zero Count Errors
A Zero Count error occurs when no oscillations are recorded for either the sensor channel or on the
reference input. A Zero Count error can indicate that the sensor has stopped oscillating or the external
clock input has stopped. A Zero Count error can also occur if:
1. The conversion time is less than one oscillation period of the sensor. Increase the value of
CHx_RCOUNT or increase the reference clock divider
(CLOCK_DIVIDERS_CHx.CHx.FREF.DIVIDER); note that this will reduce the sample rate.
2. The channel input divider is too large. Reduce the value of
CLOCK_DIVIDERS_CHx.CHx.FIN_DIVIDER.
3. The sensor resonant frequency is too low for the desired measurement. Increase the sensor frequency
to address this issue.
Zero Count Errors are reported:
• in the STATUS.ERR_ZC field (bit 8) if the ERROR_CONFIG.ZC_ERR2INT field is set to b1.
• by asserting the INTB pin if the ERROR_CONFIG.ZC_ERR2INT field is set to b1.
SNOA959–October 2016 LDC1312, LDC1314, LDC1612, LDC1614 Sensor Status Monitoring 9
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 10

Summary of Relevant Register Tables www.ti.com
4 Summary of Relevant Register Tables
4.1 Address 0x00, DATA_CH0 (LDC1312)
The following table uses the LDC1312 DATA_CH0 register as an example. Refer to the LDC161x
datasheet and the LDC131x datasheet for a complete listing of the DATA registers of the appropriate LDC
device.
Figure 4. Address 0x00, DATA_CH0
15 14 13 12 11 10 9 8
CH0_ERR_UR CH0_ERR_OR CH0_ERR_WD CH0_ERR_AE DATA0[11:0]
7 6 5 4 3 2 1 0
DATA0[11:0]
LEGEND: R/W = Read/Write; R = Read only; -n = value after reset
Table 3. Address 0x00, DATA_CH0 Field Descriptions
Bit Field Type Reset Description
15 CH0_ERR_UR R 0 Channel 0 Conversion Under-range Error Flag. Cleared by
reading the bit.
14 CH0_ERR_OR R 0 Channel 0 Conversion Over-range Error Flag. Cleared by
reading the bit.
13 CH0_ERR_WD R 0 Channel 0 Conversion Watchdog Timeout Error Flag. Cleared by
reading the bit.
12 CH0_ERR_AE R 0 Channel 0 Amplitude Warning. Cleared by reading the bit
11:0 DATA0[11:0] R 0000 0000 Channel 0 Conversion Result
0000
4.2 Address 0x18, STATUS
Figure 5. Address 0x18, STATUS
15 14 13 12 11 10 9 8
ERR_CHAN ERR_UR ERR_OR ERR_WD ERR_AHE ERR_ALE ERR_ZC
7 6 5 4 3 2 1 0
RESERVED DRDY RESERVED CH0_UNREA CH1_ CH2_ CH3_
DCONV UNREADCONV UNREADCONV UNREADCONV
LEGEND: R/W = Read/Write; R = Read only; -n = value after reset
Table 4. Address 0x18, STATUS Field Descriptions
Bit Field Type Reset Description
15:14 ERR_CHAN R 00 Warning or Error Channel
Indicates which channel has generated a Warning or Error.
Once flagged, any reported warning or error is latched and
maintained until either the STATUS register or the
DATA_MSB_CHx (LDC161x) or DATA_CHx (LDC131x) register
corresponding to the Warning or Error Channel is read.
b00: Channel 0 is source of warning or error.
b01: Channel 1 is source of warning or error.
b10: Channel 2 is source of warning or error (LDC1614 and
LDC1314 only).
b11: Channel 3 is source of warning or error (LDC1614 and
LDC1314 only).
10 LDC1312, LDC1314, LDC1612, LDC1614 Sensor Status Monitoring SNOA959–October 2016
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 11

www.ti.com Summary of Relevant Register Tables
Table 4. Address 0x18, STATUS Field Descriptions (continued)
Bit Field Type Reset Description
13 ERR_UR R 0 Conversion Under-range Error
b0: No Conversion Under-range error was recorded since the
last read of the STATUS register.
b1: An active channel has generated a Conversion Under-range
error. Refer to STATUS.ERR_CHAN field to determine which
channel is the source of this error.
12 ERR_OR R 0 Conversion Over-range Error.
b0: No Conversion Over-range error was recorded since the last
read of the STATUS register.
b1: An active channel has generated a Conversion Over-range
error. Refer to STATUS.ERR_CHAN field to determine which
channel is the source of this error.
11 ERR_WD R 0 Watchdog Timeout Error
b0: No Watchdog Timeout error was recorded since the last
read of the STATUS register.
b1: An active channel has generated a Watchdog Timeout error.
Refer to STATUS.ERR_CHAN field to determine which channel
is the source of this error.
10 ERR_AHE R 0 Amplitude High Error
b0: No Amplitude High error was recorded since the last read of
the STATUS register.
b1: An active channel has generated an Amplitude High error.
Refer to STATUS.ERR_CHAN field to determine which channel
is the source of this error.
9 ERR_ALE R 0 Amplitude Low Warning
b0: No Amplitude Low warning was recorded since the last read
of the STATUS register.
b1: An active channel has generated an Amplitude Low warning.
Refer to STATUS.ERR_CHAN field to determine which channel
is the source of this warning.
8 ERR_ZC R 0 Zero Count Error
b0: No Zero Count error was recorded since the last read of the
STATUS register.
b1: An active channel has generated a Zero Count error. Refer
to STATUS.ERR_CHAN field to determine which channel is the
source of this error.
6 DRDY R 0 Data Ready Flag.
b0: No new conversion result was recorded in the STATUS
register.
b1: A new conversion result is ready. When in Single Channel
Conversion, this indicates a single conversion is available. When
in sequential mode, this indicates that a new conversion result
for all active channels is now available.
3 CH0_UNREADCONV R 0 Channel 0 Unread Conversion
b0: No unread conversion is present for Channel 0.
b1: An unread conversion is present for Channel 0.
Read Register DATA_CH0 to retrieve conversion results.
2 CH1_ UNREADCONV R 0 Channel 1 Unread Conversion
b0: No unread conversion is present for Channel 1.
b1: An unread conversion is present for Channel 1.
Read Register DATA_CH1 to retrieve conversion results.
1 CH2_ UNREADCONV R 0 Channel 2 Unread Conversion
b0: No unread conversion is present for Channel 2.
b1: An unread conversion is present for Channel 2.
Read Register DATA_CH2 to retrieve conversion results
(LDC1614 and LDC1314 only)
0 CH3_ UNREADCONV R 0 Channel 3 Unread Conversion
b0: No unread conversion is present for Channel 3.
b1: An unread conversion is present for Channel 3.
Read Register DATA_CH3 to retrieve conversion results
(LDC1614 and LDC1314 only)
SNOA959–October 2016 LDC1312, LDC1314, LDC1612, LDC1614 Sensor Status Monitoring 11
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 12

Summary of Relevant Register Tables www.ti.com
4.3 Address 0x19, ERROR_CONFIG
Figure 6. Address 0x19, ERROR_CONFIG
15 14 13 12 11 10 9 8
UR_ERR2OUT OR_ERR2OUT WD_ AH_ERR2OUT AL_ERR2OUT RESERVED
ERR2OUT
7 6 5 4 3 2 1 0
UR_ERR2INT OR_ERR2INT WD_ERR2INT AH_ERR2INT AL_ERR2INT ZC_ERR2INT Reserved DRDY_2INT
LEGEND: R/W = Read/Write; R = Read only; -n = value after reset
Table 5. Address 0x19, ERROR_CONFIG
Bit Field Type Reset Description
15 UR_ERR2OUT R/W 0 Under-range Error to Output Register
b0: Do not report Under-range errors in the DATA_CHx
registers.
b1: Report Under-range errors in the DATA_CHx.CHx_ERR_UR
register field corresponding to the channel that generated the
error.
14 OR_ERR2OUT R/W 0 Over-range Error to Output Register
b0: Do not report Over-range errors in the DATA_CHx registers.
b1: Report Over-range errors in the DATA_CHx.CHx_ERR_OR
register field corresponding to the channel that generated the
error.
13 WD_ ERR2OUT R/W 0 Watchdog Timeout Error to Output Register
b0: Do not report Watchdog Timeout errors in the DATA_CHx
registers.
b1: Report Watchdog Timeout errors in the
DATA_CHx.CHx_ERR_WD register field corresponding to the
channel that generated the error.
12 AH_ERR2OUT R/W 0 Amplitude High Error to Output Register
b0:Do not report Amplitude High errors in the DATA_CHx
registers.
b1: Report Amplitude High errors in the
DATA_CHx.CHx_ERR_AE register field corresponding to the
channel that generated the error.
11 AL_ERR2OUT R/W 0 Amplitude Low Warning to Output Register
b0: Do not report Amplitude High warnings in the DATA_CHx
registers.
b1: Report Amplitude Low warnings in the
DATA_CHx.CHx_ERR_AE register field corresponding to the
channel that generated the warning.
7 UR_ERR2INT R/W 0 Under-range Error to INTB
b0: Do not report Under-range errors by asserting INTB pin and
STATUS register.
b1: Report Under-range errors by asserting INTB pin and
updating STATUS.ERR_UR register field.
6 OR_ERR2INT R/W 0 Over-range Error to INTB
b0: Do not report Over-range errors by asserting INTB pin and
STATUS register.
b1: Report Over-range errors by asserting INTB pin and
updating STATUS.ERR_OR register field.
5 WD_ERR2INT R/W 0 Watchdog Timeout Error to INTB b0:
Do not report Watchdog Timeout errors by asserting INTB pin
and STATUS register.
b1: Report Watchdog Timeout errors by asserting INTB pin and
updating STATUS.ERR_WD register field.
4 AH_ERR2INT R/W 0 Amplitude High Error to INTB b0:
Do not report Amplitude High errors by asserting INTB pin and
STATUS register.
b1: Report Amplitude High errors by asserting INTB pin and
updating STATUS.ERR_AHE register field.
12 LDC1312, LDC1314, LDC1612, LDC1614 Sensor Status Monitoring SNOA959–October 2016
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 13

www.ti.com Conclusion
Table 5. Address 0x19, ERROR_CONFIG (continued)
Bit Field Type Reset Description
3 AL_ERR2INT R/W 0 Amplitude Low Warning to INTB b0:
Do not report Amplitude Low warnings by asserting INTB pin
and STATUS register.
b1: Report Amplitude Low warnings by asserting INTB pin and
updating STATUS.ERR_ALE register field.
2 ZC_ERR2INT R/W 0 Zero Count Error to INTB b0:
Do not report Zero Count errors by asserting INTB pin and
STATUS register.
b1: Report Zero Count errors by asserting INTB pin and
updating STATUS. ERR_ZC register field.
1 Reserved R/W 0 Reserved (set to b0)
0 DRDY_2INT R/W 0 Data Ready Flag to INTB
b0: Do not report Data Ready Flag by asserting INTB pin and
STATUS register.
b1: Report Data Ready Flag by asserting INTB pin and updating
STATUS. DRDY register field.
5 Conclusion
The extensive error reporting that the LDC1612, LDC1614, LDC1312 and LDC1314 provide an effective
means to diagnose sensor issues or device configuration errors. This summary of the various reporting
methods and error conditions can greatly simplify an LDC system design.
SNOA959–October 2016 LDC1312, LDC1314, LDC1612, LDC1614 Sensor Status Monitoring 13
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 14

IMPORTANT NOTICE
Texas Instruments Incorporated and its subsidiaries (TI) reserve the right to make corrections, enhancements, improvements and other
changes to its semiconductor products and services per JESD46, latest issue, and to discontinue any product or service per JESD48, latest
issue. Buyers should obtain the latest relevant information before placing orders and should verify that such information is current and
complete. All semiconductor products (also referred to herein as “components”) are sold subject to TI’s terms and conditions of sale
supplied at the time of order acknowledgment.
TI warrants performance of its components to the specifications applicable at the time of sale, in accordance with the warranty in TI’s terms
and conditions of sale of semiconductor products. Testing and other quality control techniques are used to the extent TI deems necessary
to support this warranty. Except where mandated by applicable law, testing of all parameters of each component is not necessarily
performed.
TI assumes no liability for applications assistance or the design of Buyers’ products. Buyers are responsible for their products and
applications using TI components. To minimize the risks associated with Buyers’ products and applications, Buyers should provide
adequate design and operating safeguards.
TI does not warrant or represent that any license, either express or implied, is granted under any patent right, copyright, mask work right, or
other intellectual property right relating to any combination, machine, or process in which TI components or services are used. Information
published by TI regarding third-party products or services does not constitute a license to use such products or services or a warranty or
endorsement thereof. Use of such information may require a license from a third party under the patents or other intellectual property of the
third party, or a license from TI under the patents or other intellectual property of TI.
Reproduction of significant portions of TI information in TI data books or data sheets is permissible only if reproduction is without alteration
and is accompanied by all associated warranties, conditions, limitations, and notices. TI is not responsible or liable for such altered
documentation. Information of third parties may be subject to additional restrictions.
Resale of TI components or services with statements different from or beyond the parameters stated by TI for that component or service
voids all express and any implied warranties for the associated TI component or service and is an unfair and deceptive business practice.
TI is not responsible or liable for any such statements.
Buyer acknowledges and agrees that it is solely responsible for compliance with all legal, regulatory and safety-related requirements
concerning its products, and any use of TI components in its applications, notwithstanding any applications-related information or support
that may be provided by TI. Buyer represents and agrees that it has all the necessary expertise to create and implement safeguards which
anticipate dangerous consequences of failures, monitor failures and their consequences, lessen the likelihood of failures that might cause
harm and take appropriate remedial actions. Buyer will fully indemnify TI and its representatives against any damages arising out of the use
of any TI components in safety-critical applications.
In some cases, TI components may be promoted specifically to facilitate safety-related applications. With such components, TI’s goal is to
help enable customers to design and create their own end-product solutions that meet applicable functional safety standards and
requirements. Nonetheless, such components are subject to these terms.
No TI components are authorized for use in FDA Class III (or similar life-critical medical equipment) unless authorized officers of the parties
have executed a special agreement specifically governing such use.
Only those TI components which TI has specifically designated as military grade or “enhanced plastic” are designed and intended for use in
military/aerospace applications or environments. Buyer acknowledges and agrees that any military or aerospace use of TI components
which have not been so designated is solely at the Buyer's risk, and that Buyer is solely responsible for compliance with all legal and
regulatory requirements in connection with such use.
TI has specifically designated certain components as meeting ISO/TS16949 requirements, mainly for automotive use. In any case of use of
non-designated products, TI will not be responsible for any failure to meet ISO/TS16949.
Products Applications
Audio www.ti.com/audio Automotive and Transportation www.ti.com/automotive
Amplifiers amplifier.ti.com Communications and Telecom www.ti.com/communications
Data Converters dataconverter.ti.com Computers and Peripherals www.ti.com/computers
DLP® Products www.dlp.com Consumer Electronics www.ti.com/consumer-apps
DSP dsp.ti.com Energy and Lighting www.ti.com/energy
Clocks and Timers www.ti.com/clocks Industrial www.ti.com/industrial
Interface interface.ti.com Medical www.ti.com/medical
Logic logic.ti.com Security www.ti.com/security
Power Mgmt power.ti.com Space, Avionics and Defense www.ti.com/space-avionics-defense
Microcontrollers microcontroller.ti.com Video and Imaging www.ti.com/video
RFID www.ti-rfid.com
OMAP Applications Processors www.ti.com/omap TI E2E Community e2e.ti.com
Wireless Connectivity www.ti.com/wirelessconnectivity
Mailing Address: Texas Instruments, Post Office Box 655303, Dallas, Texas 75265
Copyright © 2016, Texas Instruments Incorporated
