# LDC1612 LDC1614 Linear Position Sensing (Rev. A)

- Source PDF: `../application_notes/linear_positioning_sensing.pdf`
- Extraction tool: pdfplumber
- Page count: 15
- SHA256: `4eb058ccc3cc7ba9c57733b3643c3c9361c8c11c6384bee2932a81cc3325cdb7`

## Page 1

(1)
Application Report
SNOA931A –April 2015–Revised November 2019
LDC1612/LDC1614 Linear Position Sensing
Ben Kasemsadeh
ABSTRACT
This application note discusses two techniques for using LDC technology to measure the lateral position
of a conductive target. A comparison of each approach, along with design guidelines, is provided. This
application note focuses on the LDC1612 and LDC1614, but the same principles apply to other LDCs
such as:
• LDC100x
• LDC1041
• LDC1312
• LDC1314
Contents
1 Introduction ................................................................................................................... 2
2 Approach 1: Measuring Lateral Movement with a Circular Coil and a Triangular Target .......................... 2
3 Approach 2: Measuring Lateral Movement with a Stretched Coil and a Rectangular Target ..................... 8
4 Summary .................................................................................................................... 13
List of Figures
1 Lateral Movement of a Triangular Target at Position d (Top View) .................................................. 2
x
2 Circular PCB Sensor Coil................................................................................................... 3
3 Target Dimensions and Position........................................................................................... 4
4 Linear Slider Position Versus Measured Inductance.................................................................... 5
5 Lateral Movement of a Rectangular Target at Position d (Top View)................................................ 8
x
6 Starting Position: d = 0 mm (Top View).................................................................................. 8
x
7 Final Position: d = 100 mm (Top View) .................................................................................. 9
x
8 Final Position: d = 100 mm (Side View) ................................................................................. 9
x
9 Linear Slider Position Versus Measured Inductance .................................................................. 10
List of Tables
1 Resolution at Different Target Distances ................................................................................ 6
2 Standard Deviation at d = 50 mm, d = 2 mm........................................................................... 6
x z
3 Z-axis Dependence.......................................................................................................... 7
4 Resolution at Different Target Distances ............................................................................... 11
5 Standard Deviation at d = 50 mm, d = 2 mm ......................................................................... 12
x z
6 Z-axis Dependence ........................................................................................................ 13
(1) WEBENCH is a registered trademark of Texas Instruments.
SNOA931A–April 2015–Revised November 2019 LDC1612/LDC1614 Linear Position Sensing 1
Submit Documentation Feedback
Copyright © 2015–2019, Texas Instruments Incorporated

## Page 2

Introduction www.ti.com
1 Introduction
Linear position sensing determines the position of a target that moves laterally across an inductive sensor
that is generating a magnetic field. An inductance-to-digital converter (LDC), like the LDC1612 or
LDC1614, senses inductance changes of an inductor that comes into proximity with a conductive target,
such as a piece of metal. The LDC measures this inductance shift to provide information about the
position of a conductive target over a sensor coil. The inductance shift is caused by eddy currents
generated in the target due to the magnetic field of the sensor. These eddy currents generate a secondary
magnetic field that opposes the sensor field, causing a shift in the observed inductance (see
http://www.ti.com/lsds/ti/data-converters/inductance-to-digital-converter.page).
Inductive sensing is ideally suited for this type of application because it is a contactless and magnet-free
technology that facilitates systems with very high measurement accuracy and high reliability at low system
cost.
There are two approaches to implement a linear position sensing system with an inductance-to-digital
converter. Both approaches utilize a PCB coil as a sensor.
1. A circular coil can be used to detect the position of a triangular conductive target.
2. A stretched coil design that produces a non-homogeneous AC magnetic field can be used to determine
the position of a rectangular conductive target.
2 Approach 1: Measuring Lateral Movement with a Circular Coil and a Triangular Target
2.1 Concept
Moving a triangular target from the tip to the widest point over a circular PCB sensor coil at a fixed target
distance d decreases coil inductance as the metal exposure over the coil increases. The increase or
z
decrease in the amount of target metal exposed to the field in turn changes the eddy currents and the
strength of the secondary field. Figure 1 shows a diagram of the target movement over the sensor coil.
PCB Coil Metal Target
tTravel Distancet
d
X
Figure 1. Lateral Movement of a Triangular Target at Position d (Top View)
x
2 LDC1612/LDC1614 Linear Position Sensing SNOA931A–April 2015–Revised November 2019
Submit Documentation Feedback
Copyright © 2015–2019, Texas Instruments Incorporated

## Page 3

www.ti.com Approach 1: Measuring Lateral Movement with a Circular Coil and a Triangular Target
2.2 Coil and Target Design
A circular PCB coil as shown in Figure 2 can be used as a sensor. To maximize travel range, the sensor
coil diameter must match or exceed the widest end of the target that is to be measured. The example in
this application note uses a 29-mm PCB coil with 70 turns per layer on a 2-layer PCB. The coil diameter of
29 mm exceeds the 25-mm width of the target.
Texas Instruments provides two tools to assist with design of suitable PCB coils:
• WEBENCH® Inductive Sensing Designer can be used to design a suitable coil for this application:
http://www.ti.com/lsds/ti/analog/webench/inductive-sensing.page. An example that shows how to
design a suitable coil in WEBENCH® is provided at this site:
http://e2e.ti.com/blogs_/b/analogwire/archive/2014/09/17/inductive-sensing-five-minute-sensor-coil-
design.
• Texas Instruments provides PCB layout scripts that generate coils for a variety of applications. Refer to
the application note LDC Sensor Design, and also the coil generation scripts available in the tools
section of the LDC product page.
Figure 2. Circular PCB Sensor Coil
A suitable target is an isosceles triangle that is made from metal such as aluminum or copper. Other metal
types, or plastic targets that have been painted with conductive paint are suitable alternatives but may
result in different performance.
SNOA931A–April 2015–Revised November 2019 LDC1612/LDC1614 Linear Position Sensing 3
Submit Documentation Feedback
Copyright © 2015–2019, Texas Instruments Incorporated

## Page 4

t70 mmt t30 mmt
tmm
52t
PCB Coil Metal Target
mm
92
Starting Position (Top View)
Final Position (Top View)
Final Position (Side View)
Metal Target
PCB Coil =
d Z
mm
2
Approach 1: Measuring Lateral Movement with a Circular Coil and a Triangular Target www.ti.com
The measurements in this application note use a copper target with dimensions and mechanical positions
as shown in Figure 3. The usable travel range of the target over the coil is limited to the range in which the
data output is monotonic and provides sufficient output code change for system requirements. To provide
output data monotonicity beyond the hypotenuse of the triangle, it is recommended to extend the target
shape beyond the hypotenuse of the triangle by the coil diameter to increase the maximum usable travel
distance. The diagram shows the target position in the starting position (d = 0 mm) and the final position
x
(d = 100 mm).
x
d = 0 mm
X
td t
X
d = 100 mm
X
td t
X
d = 100 mm
X
Figure 3. Target Dimensions and Position
4 LDC1612/LDC1614 Linear Position Sensing SNOA931A–April 2015–Revised November 2019
Submit Documentation Feedback
Copyright © 2015–2019, Texas Instruments Incorporated

## Page 5

Linear Slider Position (mm)
)0hc(
edoC
tuptuO
2161CDL
)Hμ(
ecnatcudnI
www.ti.com Approach 1: Measuring Lateral Movement with a Circular Coil and a Triangular Target
2.3 Performance
Moving the target from d = 0 mm to d = 100 mm at a target distance of d = 2 mm in 0.5 mm steps
x x z
results in the data output in Figure 4. Sliding the target from d = 0 mm to d = 100 mm decreases the
x x
sensor inductance from 216.3 μH to 122.2 μH. This results in a code change from 3,998,031 to 5,316,099
codes over this range and therefore can be used to determine the slider position.
5600000 234
5500000 228
5400000 222
5300000 216
5200000 210
5100000 204
5000000 198
4900000 192
4800000 186
4700000 180
4600000 174
4500000 168
4400000 162
4300000 156
4200000 150
4100000 144
4000000 138
3900000 132
3800000 126
3700000 Output Code, dZ = 2 mm 120
3600000 Inductance (μH), dZ = 2 mm 114
3500000 108
0 10 20 30 40 50 60 70 80 90 100
D001
Figure 4. Linear Slider Position Versus Measured Inductance
The effective resolution changes with target position, and decreases at the extremes of the target.
1. For example, the data shows that moving from d = 50.0 mm to d = 50.5 mm results in an output code
x x
change of 12,630 codes (4,564,564 to 4,577,194). Therefore, the average code change over this range
is 25.3 codes per μm.
2. By contrast, moving from d = 7.0 mm to d = 7.5 mm results in an output code change by 54 codes
x x
(3,998,249 to 3,998,303). Therefore, the average code increase over this range is 0.1 codes per μm.
To determine the lower end of system accuracy, it is necessary to include measurement noise addition to
resolution. Measurement noise is affected by the reference count of the device (RCOUNT). An RCOUNT
value of 0xFFFF was used to calculate this data, at which the standard deviation in output codes is 5.7
codes (refer to Table 2).
2.4 System Design Recommendations
2.4.1 Maximum Travel Range (d )
x
For high-accuracy systems, in which the code change at the extremes is insufficient, it is recommended to
use a target whose length is longer than the required travel range such that accuracy requirements can be
met. The target in the example above may be suitable for a precision application in which 82-mm travel
range is required (13 mm ≤ d ≤ 95 mm), because the effective resolution is ≥ 1 code per μm of travel in
x
this range.
By using an even narrower operating range, effective resolution can be improved further; for example, in
the travel range of 23.5 mm ≤ d ≤ 82.5 mm, the effective resolution is ≥ 10 codes per μm of travel. Note
x
that the distance resolution is constrained by the standard deviation of the output code (see Table 2).
SNOA931A–April 2015–Revised November 2019 LDC1612/LDC1614 Linear Position Sensing 5
Submit Documentation Feedback
Copyright © 2015–2019, Texas Instruments Incorporated

## Page 6

Approach 1: Measuring Lateral Movement with a Circular Coil and a Triangular Target www.ti.com
2.4.2 Coil Design: Quality Factor
The coil design must aim to maximize the quality factor (Q) of the sensor. A high Q sensor results in
higher noise immunity that leads to measurement accuracy and a lower dependence on temperature than
a low Q sensor.
Q is determined by Equation 1, Sensor Coil Quality Factor:
1 L
Q u
R C
S
where
• R is the AC series resistance of the inductor, which increases with increasing frequency.
S
• L is the sensor inductance.
• C is the sensor capacitance. (1)
2.4.3 Target Distance (d )
Z
Since the magnetic field strength rapidly decays beyond one coil diameter distance, it is recommended to
keep the target distance less than one coil diameter to ensure precise measurements.
However, best measurement accuracy can be achieved if d is kept even lower. A comparison of the
z
measurement output in the example above at three different target distances shows that the maximum
resolution can be achieved at the lowest d height. When determining the minimum target distance that
z
can be used in a system, care must be taken that within the operating range, a drive current setting is
available that satisfies R and maximum oscillation amplitude requirements.
P
Table 1. Resolution at Different Target Distances
Codes Increase Codes Increase Codes Increase
Output Code Output Code Output Code
d per μm Travel per μm Travel per μm Travel
z at d = 13 mm at d = 50 mm at d = 95 mm
x at d = 13 mm x at d = 50 mm x at d = 95 mm
x x x
1 mm 4,002,506 1.9 codes/μm 4,885,570 43.1 codes/μm 6,341,914 3.7 codes/μm
2 mm 4,001,197 1.4 codes/μm 4,564,564 25.3 codes/μm 5,315,157 1.0 codes/μm
3 mm 4,000,213 1.0 codes/μm 4,378,760 16.2 codes/μm 4,839,031 0.6 codes/μm
2.4.4 Reference Count
The conversion time of the LDC161x represents the number of reference clock cycles used to measure
the sensor frequency. It is set by the CHx_RCOUNT register for the channel. The reference count value
must be chosen to support the required resolution. A higher reference count value results in lower rms
noise at the expense of a longer conversion time. Table 2 shows the standard deviation over 1000
samples. The table shows that increasing the RCOUNT value from 0x00FF to 0xFFFF improves SNR by
33.8 dB. Standard deviation and measurement resolution must both be included to calculate the lower
limit on the system accuracy.
Note that as reference count is increased, the effective sample rate decreases.
Table 2. Standard Deviation at d = 50 mm, d = 2 mm
x z
Standard Deviation Standard Deviation
RCOUNT Conversion time
in Codes (1000 Samples) in μm (1000 Samples)
0xFFFF 26.2 ms at f =40 MHz 5.7 0.36
REF
0x0FFF 1.6 ms at f =40 MHz 20.6 1.29
REF
0x00FF 0.1 ms at f =40 MHz 277.6 17.35
REF
6 LDC1612/LDC1614 Linear Position Sensing SNOA931A–April 2015–Revised November 2019
Submit Documentation Feedback
Copyright © 2015–2019, Texas Instruments Incorporated

## Page 7

www.ti.com Approach 1: Measuring Lateral Movement with a Circular Coil and a Triangular Target
2.4.5 Output Linearization
The output is mostly linear over 60% of the travel range, and provided that the degree of linearization
during this range is sufficient to meet system accuracy requirements, no additional linearization is
necessary. However, a higher degree of linearization is often desired in order to minimize the required
target length, and to improve system accuracy. There are several approaches to improving the linearity of
the measurement::
1. The output code can be translated to travel distance by calculating the best-fit curve through the output
response. For this approach, system accuracy requirements dictate the minimum polynomial degree,
and therefore the required processing power of the microcontroller.
2. The output code can be translated to travel distance by employing a look-up table. This approach
requires little processing power, but requires memory for the look-up table.
3. Instead of using an isosceles triangle, the target shape can be altered such that the output response is
linear. To calculate the precise target shape to result in a linear output, either complex electromagnetic
modeling or complex iterative experimental modeling is required.
2.4.6 Z-axis Compensation
Figure 4 shows that the LDC depends on both d and d . If no measures are taken to compensate for
x z
mechanical tolerances of the target distance, then any error introduced by the mechanical tolerance
introduces an error in d . Table 3 shows the effect that a ± 10% tolerance of the target distance has on the
z
measurement. For example, the error that is introduced by a +0.2-mm target distance change causes a
-1.8-mm error in d . While a system with a lower d offers superior resolution than a system with higher d ,
x z z
it is also more dependent on z-axis tolerance. For example, an uncompensated system that has a 0.2-mm
tolerance in d creates a larger d error if d = 1 mm than if d = 3 mm.
z x z z
Table 3. Z-axis Dependence
d = 1.8 mm d = 2.0 mm d = 2.2 mm
z z z
Output code (d = 50 mm) 4,613,716 4,564,564 4,519,173
x
Error [codes] 49,152 0 -45,391
Error [% of frequency] 1.08% 0% 0.99%
Equivalent d position change 1,940 μm 0 -1,792 μm
x
Therefore, it is necessary to ensure that an error that is introduced by the mechanical tolerance of the
target height does not exceed resolution requirements.
In systems in which linear position must be determined more accurately than tolerances in target height
normally allow, a dual-coil coils can be used to compensate for the z-axis tolerance. Such a system
utilizes two coils; one coil whose objective it is to determine d , and a second coil which is used to
x
determine d . By using a two-dimensional look-up table or curve fitting, a higher degree of accuracy can
z
be achieved to determine d over a range of d as it would be possible with a single-coil solution.
x Z
SNOA931A–April 2015–Revised November 2019 LDC1612/LDC1614 Linear Position Sensing 7
Submit Documentation Feedback
Copyright © 2015–2019, Texas Instruments Incorporated

## Page 8

tmm
51t
t100 mmt
PCB Coil
tXTARGET = 14 mmt
tmm
52
=
Yt
TEGRAT
Approach 2: Measuring Lateral Movement with a Stretched Coil and a Rectangular Target www.ti.com
3 Approach 2: Measuring Lateral Movement with a Stretched Coil and a Rectangular Target
3.1 Concept
As an alternative to shaping the target to produce a varying output when moving a target over a coil, it is
possible to instead shape the AC magnetic field that the coil produces. Figure 5 shows an example of
such a system, in which a rectangular target slides over a coil at a fixed target distance to produce an
LDC output that can be used to determine the target position d .
x
PCB coil
Metal target
tTravel distancet
dX
Figure 5. Lateral Movement of a Rectangular Target at Position d (Top View)
x
The advantage of choosing a stretched coil with rectangular target over a circular coil with triangular target
is that the target can be much smaller and also of a simpler shape. In many systems, where space for the
moving target is restricted, a stretched coil design may be a more feasible approach.
3.2 Coil and Target Design
To use a rectangular target for linear position sensing, the coil has to be shaped to produce a non-
homogeneous AC magnetic field. This can be achieved by ‘stretching’ a coil, such that it produces a
stronger AC magnetic field on one side than on the other. Figure 6 shows an example of such a coil. The
coil measures 100 * 15 mm, has 28 turns per layer on two layers, and a 3.3 mm loop stepping. The AC
magnetic field that the coil produces is strongest at the innermost turn, and decays towards the right side.
Therefore, the peak strength of the AC magnetic field lies left of the geometric center of the coil.
Texas Instruments provides PCB layout scripts that generate stretched coils for linear position sensing
applications. These sensor design generation scripts can be downloaded from www.ti.com/ldc. Application
report SNOA930 contains further information on LDC sensor design.
The target is a rectangular copper or aluminum target. Other metal types, or plastic targets that have been
painted with conductive paint, are suitable alternatives, but may result in different performance. Figure 6
through Figure 8 show the target position in the starting position (d = 0 mm) and the final position (d =
x x
100 mm).
Innermost Turn:
Highest Field Line Density
tdXt
Aluminum Target
Figure 6. Starting Position: d = 0 mm (Top View)
x
8 LDC1612/LDC1614 Linear Position Sensing SNOA931A–April 2015–Revised November 2019
Submit Documentation Feedback
Copyright © 2015–2019, Texas Instruments Incorporated

## Page 9

PCB Coil
tdX t
Aluminum Target
Aluminum Target
2 mm
PCB Coil
mm52
=
Y
TEGRAT
www.ti.com Approach 2: Measuring Lateral Movement with a Stretched Coil and a Rectangular Target
Figure 7. Final Position: d = 100 mm (Top View)
x
Figure 8. Final Position: d = 100 mm (Side View)
x
The target length X impacts resolution and travel range. A longer target improves resolution, but
TARGET
limits the usable travel range. The target width Y must extend past the coil to ensure maximum metal
TARGET
exposure.
SNOA931A–April 2015–Revised November 2019 LDC1612/LDC1614 Linear Position Sensing 9
Submit Documentation Feedback
Copyright © 2015–2019, Texas Instruments Incorporated

## Page 10

Linear Slider Position (mm)
)0hc(
edoC
tuptuO
2161CDL
)Hμ(
ecnatcudnI
Approach 2: Measuring Lateral Movement with a Stretched Coil and a Rectangular Target www.ti.com
3.3 Performance
Moving a target from d = 0 mm to d = 100 mm at a target distance of d = 2 mm in 0.5 mm steps results
x x z
in the data output seen in Figure 9 below. The target is a 14×25 mm aluminum target.
6250000 98
6225000 97
6200000 96
6175000 95
6150000 94
6125000 93
6100000 92
6075000 91
6050000 90
6025000 89
6000000 88
5975000 87
5950000 86
5925000 Output Code, dZ = 2 mm 85
Inductance (μH), dZ = 2 mm
5900000 84
0 10 20 30 40 50 60 70 80 90 100
D002
Figure 9. Linear Slider Position Versus Measured Inductance
The graph that results from sliding the target from d = 0 mm to d = 100 mm can be broken up into three
x x
distinct regions.
1. Between 0.0 mm and 15.0 mm, the target enters the magnetic field of the coil. This region can be used
to determine target position, but the resolution is less than in the center region. For example, moving
from d = 5.0 mm to d = 5.5 mm results in an output code change from 6,014,920 to 6,014,969, a 49
x x
code change. Therefore, the average code increase over this range is 0.1 codes per μm.
2. The center region spans from 15.0 mm to 92.0 mm over which the sensor inductance decreases from
95.4 μH to 89.1 μH. This region can be used to most accurately determine target position. For
example, the data shows that moving from d = 50.0 mm to d = 50.5 mm results in an output code
x x
change by 1,565 codes. Therefore, the average code increase over this range is 3.1 codes per μm.
3. Between d = 92.0 mm and d = 100.0 mm, the trend reverses, which is due to the drop in magnetic
x x
field strength past the center coil loop. Since the LDC output codes cannot be uniquely mapped into
this region, use of this region poses significant system challenges for the small increase in target travel
range it provides.
To determine the lower end of system accuracy, it is necessary to include measurement noise addition to
resolution. Measurement noise is affected by the reference count of the device (RCOUNT). An RCOUNT
value of 0xFFFF was used to calculate this data, at which the standard deviation is 2.38 codes (refer to
Table 5).
10 LDC1612/LDC1614 Linear Position Sensing SNOA931A–April 2015–Revised November 2019
Submit Documentation Feedback
Copyright © 2015–2019, Texas Instruments Incorporated

## Page 11

www.ti.com Approach 2: Measuring Lateral Movement with a Stretched Coil and a Rectangular Target
3.4 System Design Recommendations
3.4.1 Maximum Travel Range (d )
x
The performance calculations in Section 3.3 show that not every region is suitable for measurement. The
region past the center coil loop (between d = 92.0 mm and d = 100.0 mm) is not monotonic and is
x x
therefore unusable for this application. This limits the usable travel range to 92 mm.
Depending on system accuracy requirements, precision applications may also need to discard the region
between 0.0 mm and 15.0 mm. This leads to a usable travel range of 77 mm (77% of coil length). As a
result, the coil design length needs to extend beyond the required travel range.
3.4.2 Coil Design
The coil design must aim to maximize the quality factor (Q) of the sensor (refer to Equation 1). A high-Q
sensor results in higher noise immunity, which leads to measurement accuracy and a lower dependence
on temperature than a low-Q sensor.
3.4.3 Target Distance (d )
z
Since the magnetic field strength rapidly decays beyond one coil diameter distance, it is recommended to
keep the target distance less than the coil diameter to ensure precise measurements. For non-circular
coils such as the one used in this example, the smaller coil dimension must be considered to be the coil
diameter.
However, best measurement accuracy can be achieved if d is kept even lower. A comparison of the
z
measurement output in the example above at three different target distances shows that the maximum
resolution can be achieved at the lowest d height. When determining the minimum target distance that
z
can be used in a system, care must be taken that within the operating range, a drive current setting is
available that satisfies RP and maximum oscillation amplitude requirements.
Table 4. Resolution at Different Target Distances
Codes Increase Codes Increase Codes Increase
Output Code Output Code Output Code
d per μm Travel per μm Travel per μm Travel
z at d = 15 mm at d = 50 mm at d = 92 mm
x at d = 15 mm x at d = 50 mm x at d = 92 mm
x x x
1 mm 6,022,444 1.5 codes/μm 6,142,007 4.7 codes/μm 6,354,896 7.2 codes/μm
2 mm 6,019,754 1.0 codes/μm 6,097,690 3.1 codes/μm 6,229,048 2.9 codes/μm
3 mm 6,018,212 0.7 codes/μm 6,071,355 2.1 codes/μm 6,155,486 0.9 codes/μm
SNOA931A–April 2015–Revised November 2019 LDC1612/LDC1614 Linear Position Sensing 11
Submit Documentation Feedback
Copyright © 2015–2019, Texas Instruments Incorporated

## Page 12

Approach 2: Measuring Lateral Movement with a Stretched Coil and a Rectangular Target www.ti.com
3.4.4 Reference Count
The conversion time of the LDC161x represents the number of reference clock cycles used to measure
the sensor frequency. It is set by the CHx_RCOUNT register for the channel. The reference count value
must be chosen to support the required resolution. A higher reference count value results in lower rms
noise at the expense of a longer conversion time. Table 5 shows the standard deviation over 1000
samples. The table shows that increasing the RCOUNT value from 0x00FF to 0xFFFF improves SNR by
43.8 dB. Standard deviation and measurement resolution must both be included to calculate the lower
limit on the system accuracy.
Note that as reference count is increased, the effective sample rate decreases.
Table 5. Standard Deviation at d = 50 mm, d = 2 mm
x z
Standard Deviation Standard Deviation
RCOUNT Conversion time
in Codes (1000 Samples) in μm (1000 Samples)
0xFFFF 26.2 ms at f = 40 MHz 2.38 0.85
REF
0x0FFF 1.6 ms at f = 40 MHz 15.85 5.66
REF
0x00FF 0.1 ms at f = 40 MHz 370.40 132.28
REF
3.4.5 Output Linearization
The output is mostly linear over 77% of the travel range, and provided that the degree of linearization
during this range is sufficient to meet system accuracy requirements, no additional linearization is
necessary. However, a higher degree of linearization is often desired in order to minimize the required coil
length, and to improve system accuracy. There are several approaches to improving the linearity of the
measurement:
1. The output code can be translated to travel distance by calculating the best-fit curve through the output
response. For this approach, system accuracy requirements dictate the minimum polynomial degree,
and therefore the required processing power of the microcontroller.
2. The output code can be translated to travel distance by employing a look-up table. This approach
requires little processing power, but requires memory for the look-up table.
12 LDC1612/LDC1614 Linear Position Sensing SNOA931A–April 2015–Revised November 2019
Submit Documentation Feedback
Copyright © 2015–2019, Texas Instruments Incorporated

## Page 13

www.ti.com Approach 2: Measuring Lateral Movement with a Stretched Coil and a Rectangular Target
3.5 Z-axis Compensation
Figure 4 shows that the LDC depends on both d and d . If no measures are taken to compensate for
x z
mechanical tolerances of the target distance, then any error introduced by the mechanical tolerance
introduces an error in d . Table 5 shows the effect that a ± 10% tolerance of the target distance has on the
z
measurement. For example, the error that is introduced by a +0.2-mm target distance change causes a
-2.0-mm error in d . While a system with a lower d offers superior resolution than a system with higher d ,
x z z
it is also more dependent on z-axis tolerance. For example, an uncompensated system that has a 0.2-mm
tolerance in d creates a larger d error if d = 1 mm than if d = 3 mm.
z x z z
Table 6. Z-axis Dependence
d = 1.8 mm d = 2.0 mm d = 2.2 mm
z z z
Output code (d = 50 mm) 4,613,716 4,564,564 4,519,173
x
Error [codes] 7,020 0 -6,336
Error [% of frequency] 0.12% 0% 0.10%
Equivalent d position change 2.243 μm 0 -2,024 μm
x
Therefore, it is necessary to ensure that an error that is introduced by the mechanical tolerance of the
target height does not exceed resolution requirements.
In systems in which linear position must be determined more accurately than tolerances in target height
would normally allow, a dual-coil coils can be used to compensate for the z-axis tolerance. Such a system
utilizes two coils; one coil whose objective it is to determine d , and a second coil which is used to
x
determine d . By using a two-dimensional look-up table or curve fitting, a higher degree of accuracy can
z
be achieved to determine d over a range of d as it would be possible with a single-coil solution.
x Z
4 Summary
Inductive sensing is an ideal sensing method for linear position sensing due to the contactless nature and
high reliability of the method.
Linear position can be measured either by using a circular coil and a triangular target, or by using a
stretched coil and a rectangular target. Space requirements for coil and target are the primary deciding
factors on which approach to use for a system:
• Using a circular coil and a triangular target offers excellent resolution in systems in which a target
length that is longer than the required travel range is acceptable. Texas Instruments provides the
WEBENCH® Inductive Sensing Designer and coil scripts that greatly simplify coil design for this
approach.
• Using a stretched coil and rectangular target is suitable for systems in which system space constraints
dictate use of a small target. Texas Instruments provides coil scripts that greatly simplify coil design for
this approach.
SNOA931A–April 2015–Revised November 2019 LDC1612/LDC1614 Linear Position Sensing 13
Submit Documentation Feedback
Copyright © 2015–2019, Texas Instruments Incorporated

## Page 14

Revision History www.ti.com
Revision History
NOTE: Page numbers for previous revisions may differ from page numbers in the current version.
Changes from Original (April 2015) to A Revision .......................................................................................................... Page
• Updated sentence structure throughout................................................................................................ 1
• Fixed broken cross-reference to Table 5. ............................................................................................ 12
• Changed LDC1000 to LDC100x throughout ......................................................................................... 13
14 Revision History SNOA931A–April 2015–Revised November 2019
Submit Documentation Feedback
Copyright © 2015–2019, Texas Instruments Incorporated

## Page 15

IMPORTANT NOTICE AND DISCLAIMER
TI PROVIDES TECHNICAL AND RELIABILITY DATA (INCLUDING DATASHEETS), DESIGN RESOURCES (INCLUDING REFERENCE
DESIGNS), APPLICATION OR OTHER DESIGN ADVICE, WEB TOOLS, SAFETY INFORMATION, AND OTHER RESOURCES “AS IS”
AND WITH ALL FAULTS, AND DISCLAIMS ALL WARRANTIES, EXPRESS AND IMPLIED, INCLUDING WITHOUT LIMITATION ANY
IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT OF THIRD
PARTY INTELLECTUAL PROPERTY RIGHTS.
These resources are intended for skilled developers designing with TI products. You are solely responsible for (1) selecting the appropriate
TI products for your application, (2) designing, validating and testing your application, and (3) ensuring your application meets applicable
standards, and any other safety, security, or other requirements. These resources are subject to change without notice. TI grants you
permission to use these resources only for development of an application that uses the TI products described in the resource. Other
reproduction and display of these resources is prohibited. No license is granted to any other TI intellectual property right or to any third
party intellectual property right. TI disclaims responsibility for, and you will fully indemnify TI and its representatives against, any claims,
damages, costs, losses, and liabilities arising out of your use of these resources.
TI’s products are provided subject to TI’s Terms of Sale (www.ti.com/legal/termsofsale.html) or other applicable terms available either on
ti.com or provided in conjunction with such TI products. TI’s provision of these resources does not expand or otherwise alter TI’s applicable
warranties or warranty disclaimers for TI products.
Mailing Address: Texas Instruments, Post Office Box 655303, Dallas, Texas 75265
Copyright © 2019, Texas Instruments Incorporated
