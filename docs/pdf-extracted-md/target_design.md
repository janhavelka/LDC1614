# LDC Target Design (Rev. B)

- Source PDF: `../application_notes/target_design.pdf`
- Extraction tool: pdfplumber
- Page count: 11
- SHA256: `f1e201e34fcf9eb21ecbef1cbe53e55323c7877bc881d2353de12ab8e4a07496`

## Page 1

www.ti.com Table of Contents
Application Report
LDC Target Design
ABSTRACT
Texas Instruments’ Inductive-to-Digital Converter (LDC) technology can accurately measure with a wide variety
of target sizes, shapes, and material composition. There are several target design guidelines to maximize the
effectiveness of an LDC measurement system. This application note covers the relevant factors of target design
that affect inductive sensing, and provides guidelines for optimizing sensing range and accuracy.
Table of Contents
1 The Sensor.............................................................................................................................................................................. 2
2 Eddy Currents......................................................................................................................................................................... 2
2.1 Image Currents and Target Size........................................................................................................................................ 2
2.2 Skin Depth..........................................................................................................................................................................4
2.3 Sensors Have Two Sides................................................................................................................................................... 6
2.4 LDC Interaction Through Conductor.................................................................................................................................. 7
3 Target Shape........................................................................................................................................................................... 7
4 Target Composition................................................................................................................................................................ 8
4.1 Perfect Target Material Characteristics.............................................................................................................................. 8
4.2 Aluminum Targets.............................................................................................................................................................. 8
4.3 Copper Targets...................................................................................................................................................................8
4.4 Steel and Magnetic Material Targets..................................................................................................................................9
4.5 Conductive Ink................................................................................................................................................................... 9
4.6 Ineffective Target Materials.............................................................................................................................................. 10
5 Summary............................................................................................................................................................................... 10
6 References............................................................................................................................................................................ 10
7 Revision History................................................................................................................................................................... 10
Trademarks
All trademarks are the property of their respective owners.
SNOA957B – SEPTEMBER 2016 – REVISED JUNE 2021 LDC Target Design 1
Submit Document Feedback
Copyright © 2021 Texas Instruments Incorporated

## Page 2

The Sensor www.ti.com
1 The Sensor
An inductive-to-digital converter (LDC) senses the change in inductance caused by the movement of a
conductive target through the senor’s AC magnetic field. The target movement with respect to the sensor may
be either axial or lateral with respect to the plane of the sensor. The characteristics of the sensor, as discussed in
the TI application note LDC Sensor Design, determine the characteristics of the sensor’s magnetic field and the
overall system measurement resolution and accuracy.
2 Eddy Currents
Whenever a conductor interacts with an AC magnetic field, eddy currents are induced on the conductor’s
surface. Lenz’s Law states that induced currents will flow in a manner to oppose the magnetic field, which
weakens the original magnetic field in a measurable way. LDC devices use an inductor in a resonant circuit to
generate an AC magnetic field, as shown in Figure 2-1. Any generated eddy currents will weaken the inductor’s
magnetic field, which effectively reduces the inductance of the resonant circuit, which is typically detectable by
the LDC. Depending on the device, an LDC measures either the shift in resonance and/or the energy losses in
the resonant circuit and in the target (due to the eddy currents).
Figure 2-1. AC Magnetic Field Interaction With Conductor
2.1 Image Currents and Target Size
For many LDC applications, a flat spiral inductor printed on a PCB is an effective sensor design. An example
is shown in Figure 2-2. Some of the primary considerations for the sensor include the total inductance, the
resonant frequency, and the physical size of the inductor.
2 LDC Target Design SNOA957B – SEPTEMBER 2016 – REVISED JUNE 2021
Submit Document Feedback
Copyright © 2021 Texas Instruments Incorporated

## Page 3

www.ti.com Eddy Currents
Figure 2-2. Spiral PCB Inductor
When a conductive target is brought into the magnetic field of the inductor coil, the eddy currents on the surface
of the conductor will flow in the lowest possible impedance path. This path take is composed of concentric loops
which match the shape of the sensor.
Target Eddy
Currents
Sensor
Current
PCB Sensor
Figure 2-3. Image Current Flow on Target
Any differences in the image current path compared to the sensor current path will result in a weaker change
in the sensor’s magnetic field. For this reason, the strongest target response occurs when the target size larger
than the sensor size, as the image current path can better mirror the flow of current in the sensor. If the target
size is smaller than the sensor, the change in sensor frequency will be reduced, as shown in Figure 2-4.
SNOA957B – SEPTEMBER 2016 – REVISED JUNE 2021 LDC Target Design 3
Submit Document Feedback
Copyright © 2021 Texas Instruments Incorporated

## Page 4

Target Radius (cm)
)%f(
egnahC
ecnatcudnI
rosneS
Eddy Currents www.ti.com
25%
20%
15%
10%
5%
0
0 0.1 0.2 0.3 0.4 0.5
D001
Figure 2-4. Sensor Inductance Shift vs Target Radius With Target 1 mm From Sensor
In Figure 2-4, the sensor is a spiral of 10 mm diameter, 25 turns per layer, with 4 layers having an inductance
of 36 μH. The targets are stainless steel 304 disks, and held coaxially at a 1 mm distance from the sensor. As
always, a larger change in the sensor inductance is desirable, since a larger change allows the target position
to be measured with higher resolution. When the target radius is less than 0.1 cm, the sensor measurement
changes by less than 1% of its nominal value. Position measurement of such a small target will have lower
effective resolution due to the smaller inductance change. When the target size is increased to match the size
of the sensor, the sensor measurement changes by >21%. This larger shift can be measured with much greater
resolution.
2.2 Skin Depth
Like all AC currents, the eddy currents induced by the LDC’s AC magnetic field flow near the surface of the
conductor, and reduce in amplitude deeper into the conductor. The attenuation of current follows an exponential
trend with distance from the surface. The skin depth, δ, is the distance at which the current is reduced to 1/e
(~37%) of the density at the surface. Every additional increase of δ from surface will see an additional 1/e
reduction in current.
For highly conductive materials, such as metals, the skin depth δ can be calculated by:
s
U
G
S
SP¦
(1)
where
• μ is the magnetic permeability of the material, which is μo (4π×10-7) multiplied by the conductor’s relative
permeability,
• ρ is the resistivity of the conductor, and
• f is the frequency of the AC magnetic field.
Skin depth varies with the material and sensor frequency. At higher frequencies, the skin depth becomes smaller
and smaller, which results in the eddy currents concentrating at the surface of the conductor. Poorer conductors,
such as carbon, have a larger value for their skin depth, as shown in Figure 2-5. Poorly conductive materials
have a limit on skin depth; as an example, silicon never gets below 11 m.
4 LDC Target Design SNOA957B – SEPTEMBER 2016 – REVISED JUNE 2021
Submit Document Feedback
Copyright © 2021 Texas Instruments Incorporated

## Page 5

Frequency (MHz)
)mm(
htpeD
nikS
10
Aluminum Steel 1006
Copper Carbon
1
0.1
0.02
0.1 1 10
D002
Figure 2-5. Skin Depth vs. Frequency for a Variety of Conductors
A conductor with a thickness equal to 1 skin depth for a given frequency will carry 63.2% of the current of an
infinitely thick conductor. With a conductor of 3 skin depths thick, 95% of the total current will be induced, as
shown in Figure 2-6.
To achieve best performance, a good rule of thumb is to that the target thickness should be at least 2 to 3 skin
depths. To achieve this, either increase the target thickness or sensor frequency.
Conductor Thickness in Skin Depths
deirraC
tnerruC
www.ti.com Eddy Currents
100%
90%
80%
70%
60%
50%
40%
30%
20%
10%
0
0 1 2 3 4 5 6 7 8 9 10
D003
Figure 2-6. Current vs. Conductor Skin Depth
SNOA957B – SEPTEMBER 2016 – REVISED JUNE 2021 LDC Target Design 5
Submit Document Feedback
Copyright © 2021 Texas Instruments Incorporated

## Page 6

Eddy Currents www.ti.com
2.3 Sensors Have Two Sides
The magnetic field generated by the sensor, is symmetrical above and below the sensor, as shown in Figure 2-7.
Magnetic
Field Lines
Sensor PCB
Figure 2-7. Symmetrical Field Lines
Movement of a conductive object on the opposite side of the sensor from the target can interfere with
measurement of the target position. There are several methods that can be used to mitigate this effect:
• Move the interferer far enough away from the target that the interferer movement does not affect the
target measurement (refer to the Texas Instruments Analog Wire blog post How far can I sense for more
information.
• Bring the target closer to the sensor.
• If the interfering conductor is farther away than the target, consider reducing the sensor size (although this
can reduce the measurement resolution when the target-to-sensor distance exceeds sensor radius).
• Fix the position of the interferer so that it does not move with respect to the sensor. This approach is effective
as long as the loading from the interferer does not exceed the sensor drive.
• Change the material composition of the interferer to larger resistivity such as plastic.
• Use a magnetic shielding material such as a ferrite. This does not reduce the sensitivity to the target, and in
some cases can even extend the range slightly – refer to the TI Analog Wire blog post How to shield from
metal interference for additional details.
• Use a sheet of metal to shield. This will reduce the measurement accuracy of the target position. A minimum
metal sheet thickness of 3 skin depths is recommended to shield from any other conductive objects on the
reverse side of the sensor.
6 LDC Target Design SNOA957B – SEPTEMBER 2016 – REVISED JUNE 2021
Submit Document Feedback
Copyright © 2021 Texas Instruments Incorporated

## Page 7

www.ti.com Eddy Currents
2.4 LDC Interaction Through Conductor
With lower sensor frequencies, skin depths increase, such that the sensor field can pass through the target
and interact with conductive objects behind the target. Interference can occur with conductors located behind
thin targets. Movement of a conductive object behind the target can affect the LDC measurement, as shown in
Figure 2-8.
Interfering conductor
Target
Sensor PCB
Figure 2-8. Interference from a Conductor Behind The Target
Depending on the system accuracy and resolution requirements, this can be a concern even with targets that
are 3 skin depths thick. Simply increasing the sensor frequency will reduce the effect if it is not feasible to use a
thicker target.
Some LDC devices, such as the LDC1612 and LDC1614, have specific sensor frequency ranges which provide
higher measurement resolution (as discussed in section 2.4 of the TI application note Optimizing L Measurement
Resolution for the LDC161x and LDC1101). If the increased sensor frequency is outside of the optimum range
for the LDC, the LDC’s internal input dividers can be used to mitigate this degradation. For applications that
require a wider frequency range, the LDC3114 and LDC2112/4 devices can operate with a sensor frequency up
to 30 MHz.
3 Target Shape
Because the eddy currents flow on the target in closed loops, any discontinuities in the current paths can result
in higher measurement noise. If the target has any gaps or voids, the eddy currents on the target surface will
need to flow around the void, as seen in Figure 3-1.
Figure 3-1. Current Flow for a Uniform Surface Target vs a Target With a Gap
This mismatch in image current path compared to sensor current path reduces the coupling between the target
and the sensor, and results in a smaller shift in sensor inductance and reduced measurement resolution.
SNOA957B – SEPTEMBER 2016 – REVISED JUNE 2021 LDC Target Design 7
Submit Document Feedback
Copyright © 2021 Texas Instruments Incorporated

## Page 8

4 Target Composition
4.1 Perfect Target Material Characteristics
From an electrical perspective, the perfect target material needs to have the highest possible conductivity
for optimum inductive measurements. The highest possible conductivity results in the most generated eddy
currents; the eddy currents create the strongest opposition magnetic field. The stronger magnetic field results in
a larger shift in the sensor inductance, which can be measured with more resolution.
Silver, the highest conductivity metal, is the optimum target material based on this parameter. However, other
characteristics such as cost limit the suitability of silver.
From a mechanical perspective, the target material should be physically stable and exhibit uniform movement
without warping or tilting, which could be incorrectly measured as a position shift. The material's thermal
expansion coefficient should be as low as possible so that any temperature change does not change the
effective position of the target.
Figure 4-1 shows the change in sensor frequency vs target position for targets of different materials. A larger
frequency shift is desirable; since the LDC can measure the larger shift can with more resolution. Unfortunately,
a silver target was not available for comparison.
Normalized Distance
)zHM(
f
ROSNES
Target Composition www.ti.com
5
AL1100
4.8 Bronze
4.6 Nickel N200
Steel_C10101
4.4 SS304
SS430L
4.2 Copper (CDA110)
4
3.8
3.6
3.4
3.2
3
0.01 0.1 1
D004
Figure 4-1. Comparison of Sensor Frequency Shift for Different Target Metals
The data used in Figure 4-1 was collected for a 14 mm diameter circular sensor with a free space resonant
frequency of 3.11 MHz. The target distance is scaled by the sensor diameter, so the 0.1 value on the X-axis
corresponds to a sensor to target distance of 1.4 mm.
4.2 Aluminum Targets
Aluminum is an excellent target material for many reasons – it is reasonably inexpensive, light weight, strong,
easy to machine, and also resistant to corrosion. Its high conductivity results in a skin depth only 25% larger than
copper. It has a slightly lower temperature-coefficient than copper – 4200 ppm/°C vs. copper’s 4300 ppm/°C.
Anodized finishes, which are typically only a few microns thick, do not affect the performance of aluminum
targets as long as the overall thickness of the aluminum is sufficient for the sensor frequency.
Aluminum alloys, such as AL6061, exhibit all of these benefits, but it is recommended to check the conductivity
of any selected alloy.
4.3 Copper Targets
Copper is an excellent target material, with conductivity that is 95% of silver. But it is heavier and weaker than
aluminum, so from a mechanical perspective it is often not an optimum target material. Note that some copper
alloys may have lower conductivity than aluminum, leading to the unexpected result of a weaker response than
an Al target.
Constructing a target as a copper region on a PCB is a technique that we have utilized for many applications.
Commonly available PCB fabrication can reliably produce features finer than 5 mils (0.125 mm) with 1-oz.
copper on FR4. The common plating thickness of 1-oz. copper is 37 μm thick, which is one skin depth for a
8 LDC Target Design SNOA957B – SEPTEMBER 2016 – REVISED JUNE 2021
Submit Document Feedback
Copyright © 2021 Texas Instruments Incorporated

## Page 9

sensor frequency of 3.1 MHz. While operation below one skin depth is still effective, to obtain a larger response,
the sensor frequency should be above 6 MHz with 1-oz. copper. Thinner copper platings perform better with a
correspondingly higher sensor frequency.
Duplicating the target design onto several layers will provide some improvement in response with lower
frequency sensors.
FR4 is an excellent substrate for the PCB target – it is strong, dimensionally stable, light, and has a low loss
tangent at the frequencies used by LDC sensors. In addition, the FR4 has a temperature coefficient of expansion
which closely matches the copper traces on the board.
If the sensor is a spiral trace on a PCB, then the thermal expansion of the PCB target has the added benefit of
matching the thermal expansion of the sensor.
4.4 Steel and Magnetic Material Targets
Steel and targets composed of magnetic materials can be used in LDC applications, but these materials can
present special challenges for inductance measurements. In general, systems using these targets should have
either ƒ < 20 kHz or ƒ > 1 MHz.
SENSOR SENSOR
Steel has a lower conductivity than copper (typically <10% of copper). This reduces the amount of eddy currents
generated on the surface of the target. As a result, the inductance shift due to target movement is reduced at
lower sensor frequencies. The smaller inductance shift results in lower measurement resolution of the target
movement.
Magnetic steels, such as Stainless Steel 416, produce a significantly different inductance response across
sensor frequency, as shown in Figure 4-2. This behavior is caused by the magnetic field lines permeating the
steel, which results in an increase in the sensor inductance, instead of a decrease in the inductance due to eddy
currents. At higher frequencies, the skin depth becomes small enough that the eddy current generation on the
surface of the steel blocks the magnetic field lines from entering the metal. At these higher sensor frequencies
the inductance drops in a manner consistent with other conductive materials.
There is actually a frequency at which the magnetic permeability and eddy currents balance and there is
no noticeable inductance shift. This frequency is not stable and can shift due to outside influences, such as
temperature. This effect can actually result in a non-monotonic output for target movement. In addition, the
sensor frequency changes due to target movement, and it is possible that the sensor frequency can shift to a
poorer measurement accuracy range.
R measurements do not have this issue, as the eddy current losses are still present.
P
Sensor Frequency (MHz)
)%(
tfihS
ecnatcudnI
rosneS
www.ti.com Target Composition
60%
Free-Space
Stainless Steel 416
40% Aluminum 1100
Stainless Steel 304
20%
0
-20%
-40%
-60%
0.001 0.01 0.1 1 10
D005
Figure 4-2. Shift in Sensor Inductance for Different Target Materials vs Frequency
4.5 Conductive Ink
Targets printed in conductive ink can be used to create a wide variety of target shapes. Many conductive inks
are based on silver, which is an excellent conductor but due to the additional materials in the ink conductivity
is typically just ~30% of pure silver. This reduction in conductivity unfortunately increases the skin depth of
SNOA957B – SEPTEMBER 2016 – REVISED JUNE 2021 LDC Target Design 9
Submit Document Feedback
Copyright © 2021 Texas Instruments Incorporated

## Page 10

Target Composition www.ti.com
conductive inks, which are also quite thin - typically 15 μm or thinner. The measured LDC response is typically
<10% compared to a silver conductor with a thickness of 4+ skin depths.
This reduced response limits the precision of applications using conductive ink targets, but for many cases it is
acceptable.
4.6 Ineffective Target Materials
Use of low conductivity materials will result in a correspondingly small inductive sensor shift, which restricts the
measurement resolution of the LDC. For example, water is not an effective target material. Even a saturated
salt water solution has a conductivity that is 0.000001% of copper, which results in an extremely small inductive
response.
The human body is another poor target; while in some conditions it may appear to provide a noticeable
response, the actual source of the shift is a capacitive interaction with the sensor.
5 Summary
LDC systems provide the highest performance when the target is able to provide the largest shift in sensor
inductance. To obtain the largest inductance shift requires maximizing the eddy currents on the target subject to
the constraints of the system design. Using the techniques discussed here, matching the shape and size of the
target to the sensor, using targets with the highest possible conductivity, and ensuring that the target thickness
is sufficient to support at least 3 skin depths, will maximize the sensor response to provide the highest possible
measurement resolution.
6 References
• TI Analog Wire blog post How far can I sense
• TI Analog Wire blog post How to shield from metal interference
• LDC1612/LDC1614 Linear Position Sensing (SNOA931)
• Optimizing L Measurement Resolution for the LDC161x and LDC1101 (SNOA944)
• 1° Dial Reference Design Using the LDC1314 (TIDU953)
• 32 Position Encoder Using the LDC1312 (TIDUBG4)
7 Revision History
NOTE: Page numbers for previous revisions may differ from page numbers in the current version.
Changes from Revision A (May 2017) to Revision B (June 2021) Page
• Updated the numbering format for tables, figures, and cross-references throughout the document..................1
• Deleted Image Currents and Target Size section............................................................................................... 7
Changes from Revision * (September 2016) to Revision A (May 2017) Page
• Changed to correct link.....................................................................................................................................10
10 LDC Target Design SNOA957B – SEPTEMBER 2016 – REVISED JUNE 2021
Submit Document Feedback
Copyright © 2021 Texas Instruments Incorporated

## Page 11

IMPORTANT NOTICE AND DISCLAIMER
TI PROVIDES TECHNICAL AND RELIABILITY DATA (INCLUDING DATA SHEETS), DESIGN RESOURCES (INCLUDING REFERENCE
DESIGNS), APPLICATION OR OTHER DESIGN ADVICE, WEB TOOLS, SAFETY INFORMATION, AND OTHER RESOURCES “AS IS”
AND WITH ALL FAULTS, AND DISCLAIMS ALL WARRANTIES, EXPRESS AND IMPLIED, INCLUDING WITHOUT LIMITATION ANY
IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT OF THIRD
PARTY INTELLECTUAL PROPERTY RIGHTS.
These resources are intended for skilled developers designing with TI products. You are solely responsible for (1) selecting the appropriate
TI products for your application, (2) designing, validating and testing your application, and (3) ensuring your application meets applicable
standards, and any other safety, security, regulatory or other requirements.
These resources are subject to change without notice. TI grants you permission to use these resources only for development of an
application that uses the TI products described in the resource. Other reproduction and display of these resources is prohibited. No license
is granted to any other TI intellectual property right or to any third party intellectual property right. TI disclaims responsibility for, and you
will fully indemnify TI and its representatives against, any claims, damages, costs, losses, and liabilities arising out of your use of these
resources.
TI’s products are provided subject to TI’s Terms of Sale or other applicable terms available either on ti.com or provided in conjunction with
such TI products. TI’s provision of these resources does not expand or otherwise alter TI’s applicable warranties or warranty disclaimers for
TI products.
TI objects to and rejects any additional or different terms you may have proposed. IMPORTANT NOTICE
Mailing Address: Texas Instruments, Post Office Box 655303, Dallas, Texas 75265
Copyright © 2022, Texas Instruments Incorporated
