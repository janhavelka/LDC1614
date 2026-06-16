# Setting LDC1312/4, LDC1612/4, and LDC1101 Sensor Drive Configuration

- Source PDF: `../application_notes/sensor_configuration_settings.pdf`
- Extraction tool: pdfplumber
- Page count: 8
- SHA256: `7aefcf4e074efab78fef7dce1da638025683b9fcd0f44a210988b154dc275fa4`

## Page 1

Application Report
SNOA950 –April 2016
Setting LDC1312/4, LDC1612/4, and LDC1101 Sensor Drive
Configuration
Ben Kasemsadeh
ABSTRACT
TI’s multichannel inductance-to-digital converters (LDCs) LDC1612, LDC1614, LDC1312 and LDC1314
feature an adjustable sensor current-drive to set the sensor amplitude. The LDC1101, a high-speed
device, also has an equivalent set of controls. The optimal current-drive of these devices is sensor-
dependent and is based on the sensor parallel resistance at the sensor resonant frequency, R . A sensor
P
with a lower R needs a higher current-drive than one with a higher R . This application note explains how
P P
an appropriate IDRIVE setting can be selected by analyzing the sensor signal with an oscilloscope, which
is usually the most straight-forward method.
Contents
1 Why Do Different Sensors Need Different IDRIVE Settings[unreadable glyph].......................................................... 2
2 Why is the Correct Sensor Amplitude Important[unreadable glyph] ....................................................................... 3
3 Which Methods Are There for Determining an Appropriate IDRIVE Setting[unreadable glyph] ....................................... 3
4 Why Not Use the Automatic Amplitude Setting[unreadable glyph] ........................................................................ 4
5 How to Set IDRIVE .......................................................................................................... 4
6 Procedure for Determining an Appropriate Current-Drive .............................................................. 5
7 High Current Sensor Drive Mode ......................................................................................... 6
8 How to Use Multiple Sensors .............................................................................................. 6
9 LDC1101 Sensor Amplitude Control ...................................................................................... 7
10 Summary...................................................................................................................... 7
11 Additional Resources........................................................................................................ 7
All trademarks are the property of their respective owners.
SNOA950–April 2016 Setting LDC1312/4, LDC1612/4, and LDC1101 Sensor Drive Configuration 1
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 2

RS
C
L
RP C LDC
Series electrical model Parallel electrical model
L
R =
P C ́ R
S
4R ́ I
P DRIVE
V =
OSC p
tegraT
x
tegraT
x
Why Do Different Sensors Need Different IDRIVE Settings[unreadable glyph] www.ti.com
1 Why Do Different Sensors Need Different IDRIVE Settings[unreadable glyph]
The sensor can be modeled by a series electrical model, or as its equivalent parallel model, as shown in
Figure 1.
LDC
L
Figure 1. Electrical Models of the LC Sensor
R can be calculated by Equation 1.
P
where
• R is the equivalent parallel resistance at the sensor resonant frequency
P
• L is the sensor coil inductance
• C is the sensor capacitor (including parasitic capacitance)
• R is the series resistance of the inductor at the sensor resonant frequency (1)
S
Small coils and coils with wide traces tend to have a lower R than larger coils and coils with narrow trace
S
width. Equation 1 shows that a low R results in a high R , and vice versa.
S P
To maintain a given sensor oscillation amplitude, a sensor with a lower R needs a higher current-drive
P
than one with a higher R . The peak sensor oscillation voltage is approximated by Equation 2.
P
(2)
The multi-channel LDCs have a dedicated current-drive control called the IDRIVE. This value is fixed, so
Equation 2 says that as R varies, V will also vary. Each channel has an independent current drive
P SENSOR
setting which can range from 16 μA (IDRIVE = 0) to 1.56 mA (IDRIVE = 31), as shown in Table 1. A
higher current increases a sensor’s oscillation amplitude.
Table 1. CHx_IDRIVE Values for Maximum Measured R
P
Sensor R (kΩ) CHx_IDRIVE Register Field Value (bits [15:11]) Nominal Current (μA)
P
90.0 0 (b00000) 16
77.6 1 (b00001) 18
66.9 2 (b00010) 20
57.6 3 (b00011) 23
49.7 4 (b00100) 28
42.8 5 (b00101) 32
36.9 6 (b00110) 40
31.8 7 (b00111) 46
27.4 8 (b01000) 52
23.6 9 (b01001) 59
20.4 10 (b01010) 72
17.6 11 (b01011) 82
15.1 12 (b01100) 95
13.0 13 (b01101) 110
11.2 14 (b01110) ) 127
9.69 15 (b01111) 146
2 Setting LDC1312/4, LDC1612/4, and LDC1101 Sensor Drive Configuration SNOA950–April 2016
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 3

www.ti.com Why is the Correct Sensor Amplitude Important[unreadable glyph]
Table 1. CHx_IDRIVE Values for Maximum Measured R (continued)
P
Sensor R (kΩ) CHx_IDRIVE Register Field Value (bits [15:11]) Nominal Current (μA)
P
8.36 16 (b10000) 169
7.20 17 (b10001) 195
6.21 18 (b10010) 212
5.35 19 (b10011) 244
4.61 20 (b10100) 297
3.98 21 (b10101) 342
3.43 22 (b10110) 424
2.95 23 (b10111) 489
2.55 24 (b11000) 551
2.20 25 (b11001) 635
1.89 26 (b11010) 763
1.63 27 (b11011) 880
1.40 28 (b11100) 1017
1.21 29 (b11101) 1173
1.05 30 (b11110) 1355
0.90 31 (b11111) 1563
NOTE: For LDC1312, LDC1314, LDC1612, and LDC1614, the preferred IDRIVE setting is the
highest value for which V < 1.8 V .
OSC P
2 Why is the Correct Sensor Amplitude Important[unreadable glyph]
A sensor oscillation amplitude (V ) between 1.2 V to 1.8 V results in the best measurement accuracy
OSC P P
for the multichannel LDC devices. If the amplitude is not within that range:
• If V > 1.8 V , due to the internal architecture of the LDC, the measurement accuracy is reduced
OSC P
over temperature.
• If V < 1.2 V , then the Signal-to-noise ratio (SNR) degrades.
OSC P
• If V < about 0.5 V , then the sensor may not have a stable oscillation and the LDC cannot measure
OSC P
the inductance.
It is more important that the sensor amplitude is below 1.8 V than it is to stay above 1.2 V . In general,
P P
the sensor amplitude decreases as the target moves close to the sensor because R decreases. To avoid
P
exceeding the upper limit, the measurement should be taken at the maximum target distance of the
system. This can be free air if a target is not necessarily present in the system.
3 Which Methods Are There for Determining an Appropriate IDRIVE Setting[unreadable glyph]
There are several ways to determine a suitable current drive level:
1. The R value of the sensor only can be measured with a network analyzer or impedance analyzer (see
P
SNOA936).
2. IDRIVE can be adjusted until no warning flags are indicated (ERR_AHE and ERR_ALE).
3. The automatic amplitude control feature can be used during system prototyping to let the LDC select
CHx_IDRIVE (see SNAA221).
4. Use an oscilloscope to measure oscillation amplitude.
SNOA950–April 2016 Setting LDC1312/4, LDC1612/4, and LDC1101 Sensor Drive Configuration 3
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 4

Why Not Use the Automatic Amplitude Setting[unreadable glyph] www.ti.com
4 Why Not Use the Automatic Amplitude Setting[unreadable glyph]
Automatic amplitude features are only recommended for one-time system configuration purposes. If Auto
amplitude control is enabled for normal operation, the LDC may adjust the current during a measurement
and introduce an offset that appears as a step in the target position. For normal operation, enable R
P
override (RP_OVERRIDE_EN=1) and disable auto amplitude correction (AUTO_AMP_DIS=1), which
forces the LDC uses the current-drive setting in the IDRIVE register. Manual control ensures that the
same IDRIVE setting is used on each channel every time the system powers up, and that the same
current-drive is used regardless of target distance.
5 How to Set IDRIVE
The IDRIVE setting can either be directly written to the DRIVE_CURRENT_CHx registers, or programmed
using the GUI if using the LDC EVM, as shown in Figure 2.
Figure 2. Programming Sensor IDRIVE in the Sensing Solution’s Evaluation Module GUI
4 Setting LDC1312/4, LDC1612/4, and LDC1101 Sensor Drive Configuration SNOA950–April 2016
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 5

www.ti.com Procedure for Determining an Appropriate Current-Drive
6 Procedure for Determining an Appropriate Current-Drive
The following steps can be taken to determine an appropriate IDRIVE setting with an oscilloscope:
1. Move the target to the expected maximum distance from the sensor as it will be during normal system
operation.
2. Configure IDRIVE to 31 (maximum). If the sensor R is known, the closest R value in the table can be
P P
used to estimate the starting value for IDRIVE.
3. Measure the oscillation amplitude with respect to ground at the INAx pin
4. Reduce IDRIVE until V < 1.8 V .
OSC P
Figure 3 shows an example oscilloscope plot in which the peak amplitude is 3.3 V at an IDRIVE setting of
P
25. The peak amplitude is higher than the 1.8 V recommended limit.
P
V > 1.8 V
OSC P
Measure after sensor
(not recommended)
amplitude settled
Figure 3. IDRIVE = 25 Causes V > 1.8 V (Not Recommended)
OSC
IDRIVE is then decreased until V < 1.8 V. With the chosen sensor and target distance, this happens at
OSC
an IDRIVE setting = 19 (see Figure 4). Note that reducing the target distance caused the amplitude to
decrease, but that is not a concern as long as system-accuracy specifications are still met.
SNOA950–April 2016 Setting LDC1312/4, LDC1612/4, and LDC1101 Sensor Drive Configuration 5
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 6

High Current Sensor Drive Mode www.ti.com
V < 1.8 V
OSC P
(recommended)
Figure 4. IDRIVE = 19, Which Is The Highest Setting That Meets the V
OSC
7 High Current Sensor Drive Mode
The LDC has a high current sensor drive mode (HIGH_CURRENT_DRV) for channel 0 only, which
ignores the IDRIVE setting. If R is so low that an IDRIVE setting of 31 shows an amplitude of less than
P
1.2 V , then the current can be boosted by setting HIGH_CURRENT_DRV=1. In this mode, the current
P
drive increases to twice its normal maximum of 1.5 mA in normal mode to 3 mA.
This mode is useful for applications in which very small sensors with a low R is connected. Note that
P
AUTOSCAN_EN must be set to 0 (single channel mode).
8 How to Use Multiple Sensors
It is not necessary to use the same IDRIVE setting across all channels. If different channels use different
sensor components, then each channel must be evaluated individually.
However, many systems use multiple sensors with the same sensor characteristics in order to perform
measurements of multiple targets, to perform differential measurements, or to use one sensor as a
reference sensor. In this case, it is recommended to use the same IDRIVE setting for all channels to
ensure consistent measurement results.
6 Setting LDC1312/4, LDC1612/4, and LDC1101 Sensor Drive Configuration SNOA950–April 2016
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 7

www.ti.com LDC1101 Sensor Amplitude Control
In case the procedure above results in different IDRIVE settings for different sensors, then the lower
IDRIVE setting should be used. For example, if the procedure above shows that the optimal IDRIVE
settings for the four LDC1614 channels are 14 (channel 0), 13 (channel 1), 13 (channel 2), and 14
(channel 3) and the same sensor components are used, then all four IDRIVE settings should be set to the
lowest setting of 13.
9 LDC1101 Sensor Amplitude Control
The LDC1101 includes a LHR function (L Measurement, High-Resolution) that can be considered a single-
channel, 24 bit equivalent of the LDC1612. The default configuration of the LDC1101 automatically
controls the sensor amplitude; however, this automatic amplitude control increases the noise of the
inductance measurement. To disable the automatic amplitude control, set the LOPTIMAL and
DOK_REPORT device fields to 1. More information on this setting is available in section 9.1.10 of the
LDC1101 device data sheet (SNOSD01).
The multichannel LDC devices use the IDRIVE setting; the equivalent control on the LDC1101 is RPMIN.
Table 2. LDC1101 RPMIN Sensor Drive Settings
RPMIN Field Value Sensor Drive (μA) Minimum Sensor R (kΩ) Maximum Sensor R (kΩ)
P P
b111 600 0.53 1.65
b110 300 1.1 3.3
b101 150 2.1 6.5
b100 75 4.2 13.1
b011 37.5 8.4 26.2
b010 18.7 16.9 52.4
b001 9.4 33.9 105
b000 4.7 67.9 209
While the LDC1101 has a different field name and different available settings for the IDRIVE control, the
appropriate value can be determined using the Oscilloscope technique described in this application note.
NOTE: For the LDC1101, the preferred RPMIN setting is the highest value for which V < 1.25 V .
OSC P
10 Summary
Knowing the R value is not necessary for determining the preferred drive current. The optimal setting can
P
also be determined by measuring the sensor amplitude with an oscilloscope with the target at the
maximum operating distance from the sensor.
To achieve the highest performance, set the current drive setting as follows:
• For LDC1312, LDC1314, LDC1612, and LDC1614, set the IDRIVE current to the highest setting that
gives V ≤ 1.8 V .
OSC P
• For LDC1101 in LHR mode, set the RPMIN setting to the highest setting that gives V ≤ 1.25 V .
OSC P
11 Additional Resources
• Download the data sheet for LDC1612 and LDC1614 (SNOSCY9).
• Download the data sheet for LDC1312 and LDC1314 (SNOSCZ0).
• Download the data sheet for LDC1101 (SNOSD01).
• Learn more about TI’s inductive-sensing portfolio.
• Design your sensor coil and start your system design in seconds with WEBENCH® Inductive Sensing
Designer.
SNOA950–April 2016 Setting LDC1312/4, LDC1612/4, and LDC1101 Sensor Drive Configuration 7
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 8

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
