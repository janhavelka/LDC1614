# Inductive Sensing Touch-On-Metal Buttons Design Guide

- Source PDF: `../application_notes/sensing_touch_on_metal.pdf`
- Extraction tool: pdfplumber
- Page count: 21
- SHA256: `e2caf821b3cef9ea6028a8522945e65faf9430c0a0e3a2e801273b4f7efe1b17`

## Page 1

Application Report
SNOA951 –June 2016
Inductive Sensing Touch-On-Metal Buttons Design Guide
Ben Kasemsadeh, Luke LaPointe
ABSTRACT
This application note covers the fundamentals of Touch-on-Metal (ToM) technology using an LDC1612
Inductance-to-Digital Converter (LDC) and provides guidance for constructing ToM buttons. Simple on and
off buttons can be easily implemented using inductive sensing. Additionally, by using a high resolution
LDC, microscopic movements in a flat metal button can be sensed and processed to determine how hard
a given button was pressed. This approach allows reuse of existing metal surfaces commonly found in
many applications such as consumer electronics and appliances. This report contains a design example
for a multi-button brushed aluminum panel and provides guidance on the mechanical system and sensor
design, as well as measured performance results of the complete system.
Contents
1 ToM Basics ................................................................................................................... 1
2 How Are Inductive Touch-On-Metal Buttons Implemented[unreadable glyph] ........................................................... 2
3 System Design Procedure .................................................................................................. 3
4 Results....................................................................................................................... 18
5 Summary .................................................................................................................... 20
6 Additional resources ....................................................................................................... 20
Trademarks
All trademarks are the property of their respective owners.
1 ToM Basics
ToM buttons refers to using a flat metal surface as a button and a high resolution inductance converter
such as the LDC1612 to detect the microscopic metal deflection that occurs when the button is pressed.
Figure 1 shows a block diagram of a touch-on-metal solution with two buttons. When even a light force is
applied to a button, the inner surface of the metal sheet will be pushed towards the PCB sensors. The
metal sheet does not contact the sensors but the small amount of deflection from the press causes a shift
in the sensor inductance that can be detected by the LDC and then interpreted as a button press by a
microcontroller (MCU). Haptics such as a vibration, audible beep, or visual indication may also be
triggered to give the user an acknowledgement of an accepted button press.
SNOA951–June 2016 Inductive Sensing Touch-On-Metal Buttons Design Guide 1
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 2

LC Sensor
L
C
LDC
x
tegraT
lateM
How Are Inductive Touch-On-Metal Buttons Implemented[unreadable glyph] www.ti.com
Figure 1. System Block Diagram of a Touch-On-Metal Implementation
Inductive-sensing based designs for touch-on-metal offers a completely sealed and contactless solution
with a greatly simplified assembly process. In addition to being insensitive to dirt, moisture, and other
contaminants, inductive touch-on-metal buttons offer a robust solution that does not use moving
mechanical parts, and offers a flat surface that is easy to clean for home appliances. Unlike mechanical
buttons, inductive sensing-based buttons can detect the amount of pressure on the button, allowing for
adjustable sensitivity or the ability to program the button for different functions depending on the amount of
pressure applied. In addition to working with grounded and ungrounded button panels, inductive sensing
also provides excellent immunity towards EMI sources due to a narrow-band resonant sensing approach.
2 How Are Inductive Touch-On-Metal Buttons Implemented[unreadable glyph]
Inductive-to-Digital Converters (LDC) are able to measure proximity to metal by detecting the subtle
changes in an AC magnetic field resulting from the interaction with the metal target. The LDC generates
an AC magnetic field by supplying an AC current into the parallel LC resonant circuit shown in Figure 2.
Sensing
distance AC
Current
Figure 2. LC Sensor Components
If a conductive target is brought into the vicinity of the inductor’s AC magnetic field, small circulating
currents known as eddy currents will be induced by the magnetic field onto the surface of the conductor
shown below in Figure 3.
2 Inductive Sensing Touch-On-Metal Buttons Design Guide SNOA951–June 2016
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 3

www.ti.com System Design Procedure
Eddy Currents Metal Target
PCB with Spiral Inductor
AC Inductor Current
Figure 3. Eddy Currents Induced on Metal Surface
These eddy currents produce their own magnetic field that opposes the one created by the inductor which
reduces the effective inductance of the coil. The resulting inductance shift is measured by the LDC and
can be used to provide information about the position of the target over a sensor coil such as distance or
equivalently the force of a button press.
3 System Design Procedure
In order to construct a ToM system with the optimal performance, the following should be considered:
1. Mechanical system design: The quantity, size, and arrangement of buttons as well as the optimal
target-to-sensor spacing can influence the response of the system.
2. Sensor design: Best practices to LDC sensor design and shape to ensure that the LDC can detect
microscopic deflection in metal.
3. Other considerations: Multiplexing multiple buttons, power consumption, detection algorithms to
automatically adjust for long-term drift or permanent mechanical changes, and EMI.
3.1 Mechanical System Design
This aspect of the system design is used to address the physical interface presented to the user.
Considerations such as the number of buttons, the size and shape of the buttons, and material
composition all need to be determined.
A typical home appliance example with a ToM control panel might have two or more adjacent buttons. For
ease of use, the buttons should not be too small; typical applications may use 20-mm diameter buttons,
which is sufficiently large for easy actuation. Typically the button panel is a flat metal surface constructed
from a single sheet of metal. ToM buttons may use a wide variety of metals, but many consumer and
industrial systems prefer stainless steel or aluminum surfaces which are commonly available materials.
Indicating the location of the button can be handled with a wide range of approaches – from adhesive
overlays, to painted markings, or even putting grooves or patterning onto the surface of the metal.
Figure 4 shows an example button panel which has been produced from a 0.8-mm thick sheet of
Aluminum Al6061-T6 – the buttons are clearly identified by the circular grooves.
SNOA951–June 2016 Inductive Sensing Touch-On-Metal Buttons Design Guide 3
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 4

System Design Procedure www.ti.com
Figure 4. Manufactured Button Panel
Figure 5 shows a side view of the two adjacent buttons in this example application.
Figure 5. Using Adjacent Buttons
When a light force is applied onto button A, the inner surface of the metal sheet will be pushed towards
the PCB sensors. This deflection causes a frequency shift in the LC sensor and must be enough to be
easily detected by the LDC and then interpreted as a button press by the MCU in the system.
Sources of error, such as adjacent button deflection or other environmental noises, could mask the
desired response and make it difficult for the MCU to distinguish the real button press. It is recommended
that the desired amount of deflection for a button detection event should produce a response that is
greater than the noise of the system by a factor of 10. For example, if the system noise appears like ±0.5-
μm movement, then a button needs to move at least 5μm to be easily detected.
There are number of factors that influence how much metal deflection is produced by a button press, such
as metal material and thickness. With good system design, the deflection of the metal for a typical button
press is around 20 to 50 μm.
4 Inductive Sensing Touch-On-Metal Buttons Design Guide SNOA951–June 2016
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 5

Force (N)
)mμ(
noitcelfeD
kaeP
www.ti.com System Design Procedure
3.1.1 Designing for Natural Button Force
A key component to designing a ToM button is the user experience and how much force is required to
detect a button press. Mechanical buttons typically require between 2N and 5N of force. For buttons of
non-moving parts, a consumer’s natural instinct is to press with less force, typically 0.5 to 2 N. To relate
the amount of applied force to the amount of metal deflection produced, it is necessary to consider metal
composition, thickness, and structure of the button. In general, a larger deflection is desirable, as it
provides a larger shift in inductor response and provides more flexibility for button detection threshold.
3.1.1.1 Metal Composition
The material choice can have a large impact on how much force is required to achieve the required
deflection at a given metal thickness. The key parameter is Young’s modulus, which is a measure of the
elasticity of the metal and is measured in units of pascal (Pa). Materials with a lower Young’s Modulus are
typically more flexible. Aluminum (AL6061-T6) has a Young’s modulus of 68.9 GPa, while Stainless Steel
(SS304) has a higher Young’s modulus of 203 GPa, which makes it about 3 times less flexible than
aluminum. Aluminum is an excellent material choice for inductive sensing because it is both flexible and
asserts a high inductance change on an inductive sensing coil. Materials such as SS304, can also be
used and provide robust results. The difference in deflection for a given amount of pressure between the
two materials is shown in Figure 6.
100
10
1
Aluminum (AL6061-T6)
Stainless Steel (SS304)
0.1
0 0.5 1 1.5 2
D001
Figure 6. Force vs Peak Deflection for Different Materials, Diameter = 20 mm, Thickness = 0.25 mm
3.1.1.2 Metal Thickness
Using thinner metal sheets allows higher deformation at a given force than using thicker sheets, as
illustrated in Figure 3. For example, with a 20-mm diameter button of aluminum and a force of 1 N, the
0.25-mm thick sheet has a peak deflection of 27 μm, whereas a 1-mm thick sheet only has a deflection of
0.42 μm.
SNOA951–June 2016 Inductive Sensing Touch-On-Metal Buttons Design Guide 5
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 6

Force (N)
)mμ(
noitcelfeD
kaeP
System Design Procedure www.ti.com
100
10
1
0.1
Thickness = 0.25 mm
Thickness = 0.5 mm
Thickness = 1 mm
0.01
0 0.5 1 1.5 2
D002
Figure 7. Force vs Peak Deflection for an Example Sheet of Al-6061-T6
3.1.1.3 Mechanical Structure of the Button
The structure and shape of the button will also determine how much deflection is achieved. A ToM design
should consist of a flat sheet of metal with spacers between the metal and the PCB to allow for deflection.
The width and position of the spacers act like fulcrums to the metal and can improve button deflection if
narrow and placed far apart. The designer may also etch a cutout beneath the button to allow for
controlled deflection. This would allow the PCB to be placed flush against the metal without the use of
spacers. Additionally, the cutout will decrease the thickness of the metal localized to the button area only
which will improve the deflection and thus the response of the button. Depending on the available
assembly processes and cost constraints, one approach may be more desirable than the other.
3.1.2 Target Distance
The nominal spacing between the inner metal surface and the PCB sensor is important to consider for
both mechanical/assembly and electrical considerations. As the metal target approaches the sensor, the
amount of inductance shift increases rapidly. The optimal target distance is where the sensor sensitivity is
at its peak, but still has room for mechanical deflection. Metals that approach the sensor capture more of
the electromagnetic field such that the highest sensitivity of the system occurs when the metal target is as
close to the sensor as possible. However, to account for manufacturing tolerances and to ensure that
there is still room for metal deflection, it is recommended for a nominal metal to sensor spacing be kept
above 0.2mm. This spacing can be achieved by creating recessed area in the metal above the sensor for
systems where the PCB is placed flush to the metal or by using a small spacer between the metal and the
PCB sensor with a cutout to allow the metal to deflect, as shown in Figure 18. Additionally, the sensitivity
rapidly decreases beyond 20% of the coil diameter, therefore it is recommended to place the sensor within
this distance while leaving >0.2 mm of room for deflection, as shown in Figure 8.
6 Inductive Sensing Touch-On-Metal Buttons Design Guide SNOA951–June 2016
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 7

www.ti.com System Design Procedure
Figure 8. Optimal Target Distance
3.1.3 Mechanical Isolation
When multiple buttons are present in a system, it is possible for undesirable mechanical interaction
between different buttons to occur. For example, when pressing button A, the contiguous metal surface
may deform in such a manner that a significant amount of movement occurs over the neighboring button B
sensor, and could appear as an unintended button press of button B. The following principles can be
applied to reduce the mechanical crosstalk between adjacent buttons during an active press:
1. Physical supports between buttons or standoffs can facilitate stronger metal deformation on the button
that is pressed, as shown in Figure 5.
2. Ensuring a larger physical deflection for the intended button. From an electrical perspective, a large
signal-to-noise ratio between a true button press versus an undesired detection is the easiest way to
detect the correct button press event. Using thinner metal or selecting materials with a low Young’s
modulus ensures that metals are easier to deform and have less impact on the neighboring buttons.
For example, aluminum is more flexible than many stainless steel alloys.
3. Increasing the distance or adding grooves between adjacent buttons improves mechanical isolation.
For cross-talk minimization, button-to-button separation should also be greater than one coil diameter.
3.1.4 Mounting Techniques
For effective operation of ToM button, the sensor PCB should be fixed at a constant offset from the metal
surface. If the sensor PCB is not held firmly, then it could move or vibrate away from the metal surface,
which could be misinterpreted as a button press or change the desired button sensitivity.
SNOA951–June 2016 Inductive Sensing Touch-On-Metal Buttons Design Guide 7
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 8

System Design Procedure www.ti.com
In many applications the sensor coil can be part of the main PCB and mounting holes can be used to align
the sensor to the outline of the metal button. The height at which the metal button sits above the PCB will
have the largest impact on the operating point of the LDC. In other applications where there is not much
space for a monolithic PCB, the sensor may be put onto a flex PCB board and attached to the main PCB
with a connector. The same mechanical design considerations must be taken into account as well as
adding stiffeners to the sensor to prevent false detection. Some applications may also be restricted in
mechanical arrangement, so that the distance between the sensor PCB and button panel cannot be
reduced sufficiently. In such cases, a wire-wound surface-mount inductor may be used instead of a PCB
coil.
For a standard FR4 PCB implementation, either of the two following assembly stack-ups are
recommended:
1. A metal panel with a recessed area directly beneath the button on the bottom of the panel which can
be created by either milling or etching a cavity into the metal, as shown in Figure 9. In this case, the
left-over areas act as standoffs which support the PCB and ensure that there is room for metal
deflection above the sensor coil. Double-sided adhesive (such as 3M 300LSE adhesive tape), or epoxy
can be used to attach the PCB to the metal panel. If using adhesive tape, bubbles in the adhesive can
form causing a non-uniform button response and potentially cause false detections. These air bubbles
can be eliminated by either using adhesive with micro-channels or by adding non-plated vias to the
PCB and applying force during the assembly process to compress the structure and force the air out
and create a secure bond between the metal and PCB.
Metal sheet
Button A location Button B location
(aluminum or
stainless steel)
Double-sided Recessed area for Recessed area for
adhesive or epoxy deflection deflection
PCB with sensor coils
Figure 9. Assembly with Standoffs in the Metal Sheet
2. A plastic spacer with cutouts can be placed between the metal panel and the PCB. This approach is
useful in systems where a flat sheet of metal is used and it is not possible to do additional cutouts or
milling to the metal. The spacer provides the necessary air gap between the metal sheet and the
sensor to allow for deflection. Therefore, the cut-outs are of the same dimension and location as the
button itself. Double-sided adhesive or epoxy can also be used to secure the assembly. Adding non-
plated vias to both the PCB and the spacer that are aligned and adding sufficient pressure during the
assembly process is important to remove the air bubbles that could accumulate and create undesired
offsets or crosstalk between the channels.
8 Inductive Sensing Touch-On-Metal Buttons Design Guide SNOA951–June 2016
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 9

www.ti.com System Design Procedure
Figure 10. Assembly with Plastic Spacer
3.2 Sensor Design
3.2.1 PCB Design
LDCs typically utilize a multi-layer PCB spiral inductor as the inductive element for the LC resonator
circuit. An external capacitor is added in parallel to the sense coil as shown in Figure 2, whose value
remains static regardless of metal proximity. The key coil parameters are shown in Figure 11.
W d
IN
S d
OUT
Figure 11. PCB Coil Parameters
LDC sensor design guidelines are described in detail in the LDC Sensor Design application note
(SNOA930).
When determining the geometries of the sensor coil for ToM applications, it is generally recommended to
match the shape of coil to the metal button so that the deflection is evenly distributed over the sensor coil.
SNOA951–June 2016 Inductive Sensing Touch-On-Metal Buttons Design Guide 9
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 10

System Design Procedure www.ti.com
The outer diameter (d ) of the coil should be large enough to sense the metal proximity at the nominal
OUT
distance, but due to the concave nature of a button press, very little metal is deflected at the edges of the
metal button. Therefore it is recommended to keep the outer diameter between 50% to 60% of the metal
button diameter to maximize the sensitive area of the coil to the metal deflection. Once the diameters of
the metal button and the coil have been determined, the sensor PCB coil can be placed ideally within 20%
of the coil diameter from the metal according to Figure 8. This nominal spacing can be achieved by using
a spacer that is 20% of the coil diameter thick or by milling out the metal above the sensor.
The coil fill ratio is also important to consider for ToM applications, because the metal target deforms the
most at the center when pressed and causes an uneven amount of coverage over the sensor coil with the
closest proximity to the coil at the center. This deflection shape makes the inner turns more useful and
therefore ToM sensors should use many inner turns as possible to achieve a d / d - ratio of less than
IN OUT
0.4. This effect can be seen by examining the eddy currents produced on the surface of the metal button.
There is a higher density of eddy currents where the most magnetic field lines are concentrated as shown
in Figure 12. Therefore by increasing the number of inner turns more eddy currents are generated on the
surface of the metal which increases the sensitivity of the ToM system.
Figure 12. Eddy Current Density Induced by an AC Magnetic Field
For this example application, a PCB with the coil characteristics in Table 1 was designed on a two-layer
PCB, as shown in Figure 13 to Figure 15.
Table 1. Sensor Coil Parameters
PARAMETER VALUE
d 12 mm
OUT
d 3.3 mm
IN
Number of turns / layer 14
Number of layers 2
Trace width (W) 6 mil (0.15 mm)
Trace spacing (S) 6 mil (0.15 mm)
10 Inductive Sensing Touch-On-Metal Buttons Design Guide SNOA951–June 2016
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 11

www.ti.com System Design Procedure
With an inner metal surface-to-sensor distance of d1=0.55 mm, the coil design produces a nominal
inductance of 3 μH. Note that although the metal button is 12 mm, the outer silkscreen with 20-mm
diameter indicates the portion on the bottom of the metal panel which has been recessed by 0.55 mm to
provide room for deflection of the button, refer to Figure 18.
Figure 13. Sensor PCB (All Layers)
SNOA951–June 2016 Inductive Sensing Touch-On-Metal Buttons Design Guide 11
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 12

System Design Procedure www.ti.com
Figure 14. Sensor PCB (Top Layer)
12 Inductive Sensing Touch-On-Metal Buttons Design Guide SNOA951–June 2016
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 13

www.ti.com System Design Procedure
Figure 15. Sensor PCB (Bottom Layer)
3.2.2 Sensor Frequency Selection
The choice of sensor capacitor value and type is important to maintain a stable oscillation of the resonant
circuit and is critical for optimum signal-to-noise ratio. The capacitor characteristics can affect the resonant
behavior, so a high quality dielectric is recommended. A NP0/C0G capacitor is chosen because it does
not exhibit common non-idealities such as piezo-electric effects, dC/dV, or a significant temperature
coefficient. The combination of inductance and sensor capacitance determines the sensor frequency of
the LC tank determined by Equation 1.
1
f =
SENSOR
2p ́ LC
(1)
The optimal choice of sensor frequency depends on the selection of metal material and thickness. Metals
with higher conductivity, such as aluminum have a shallower skin depth which moves the induced eddy
currents to surface of the material. This dense concentration of eddy currents produces a greater shift in
the AC magnetic field of the LC sensor, making the metal deflection easier to detect. This also enables
use of a wider sensor frequency range and thinner metal surfaces. Alloys with a lower conductivity such
as stainless steel do not produce as much inductance shift at low sensor frequencies and therefore
require an increased sensor frequency to produce an equivalent response. As a rule of thumb, it is better
to operate at a high sensor frequency to provide the most flexibility in material selection, especially when a
small metal thickness is used. The LDC1612 has a maximum sensor frequency of 10 MHz, but to allow
manufacturing tolerance, a value of 100 pF is chosen to achieve a sensor frequency of 9.1 MHz and
provide some system margin.
SNOA951–June 2016 Inductive Sensing Touch-On-Metal Buttons Design Guide 13
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 14

System Design Procedure www.ti.com
3.2.3 Sensor Amplitude Selection
The oscillation amplitude of the LDC1612 sensor is determined by the drive current strength and is
adjustable by the device register settings. Larger sensor oscillation amplitudes improve system SNR, but
could result in higher emissions. The oscillation amplitude has been chosen to be 1.3 V , which is in the
P
recommended range according to datasheet specifications. It is important to note that as the metal
approaches the sensor, the oscillation amplitude will decrease. Therefore it is important to set the
oscillation amplitude with the metal present in the system to maintain good system SNR.
3.3 Other Considerations
3.3.1 Button Quantity and Multiplexing
The dual-channel LDC1612 supports two buttons. With the quad-channel LDC1614, four-button systems
can be implemented. For systems with more than four buttons, multiplexing may be utilized with a single
LDC1614. Table 2 shows the recommended device for the required amount of buttons
Table 2. Button Quantity vs Device Selection
Number of Buttons Recommended Device
1-2 LDC1612
3-4 LDC1614
5+ LDC1614 + external multiplexing
3.3.2 Power Consumption
The full sample speed of the LDC is not required for HMI application which allows for duty cycling and
power consumption savings. The LDC can be sampled at periodic intervals to conserve power
consumption, while still providing real-time button detection.
The Inductive Sensing Design Calculator Tool spreadsheet (SLYC137) can calculate the average current
consumption in duty-cycled applications. A typical application with two buttons and a sample rate of 10
samples per second may have an average current consumption of 225 μA (113 μA per button), as shown
Table 3. In battery powered applications, the tradeoff between sample rate and resolution can be adjusted
for further power savings.
Table 3. Average Current Consumption
Parameter Value Unit
f input to CLKIN 20 MHz
REFERENCE
RCOUNT Setting (range:0x0003 to 0xFFFF) 1000 hex
Desired Sample Rate 10 sps
Number of channels of measurement 2
Sensor Frequency (open air) 8 MHz
Sensor Frequency (in presence of Aluminum at 0.55mm distance) 9.1 MHz
Sensor Q (max) 20
I2C Data rate 400 kbit/s
Sensor RP 10 kΩ
LDC Device LDC1612
Programmed settle count 64
(registers
Minimum Settle Count Register Setting 0x4
0x10:0x13)
single conversion time 3.28 ms
LDC Current consumption using Shutdown Mode 225.29 μA
LDC Current consumption using Sleep Mode 256.24 μA
14 Inductive Sensing Touch-On-Metal Buttons Design Guide SNOA951–June 2016
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 15

www.ti.com System Design Procedure
3.3.3 Software Algorithm
The LDC measures the oscillation frequency of the sensor and converts it to a raw output code from which
the sensor inductance can be calculated. An I2C interface is used to stream the 28-bit digitized data to the
processor or microcontroller for post-processing. An interrupt driven method is efficient for button
applications because the microcontroller does not have to poll the LDC for information.
An algorithm is required to interpret the raw output code received by the microcontroller. A straight-forward
implementation uses a simple moving average function (SMA) with a dynamically adjusted threshold to
determine when a button has been pressed. This algorithm is sufficient to neglect changes due to
environmental factors such as temperature, while checking for fast changes that signal button press
events.
For applications that do not need to detect simultaneous button presses, a simple comparison function
can be implemented where the button with the strongest force applied that exceeds the detection
threshold is selected. More advanced algorithms can allow for simultaneous button presses or multiple
thresholds to distinguish between different levels of force. Even compensation for mechanical twisting or
permanent damage from a dented panel can be handled algorithmically which allows the inductive sensing
ToM solution to excel in reliability and user experience.
After the MCU has identified the button press, an acknowledgement needs to be provided to the user to
indicate that the button press was recognized. In the absence of moving parts, the MCU can send a
trigger to an audible, haptic, and/or visual feedback device. TI offers several haptic drivers such as the
DRV2605 which can be used to drive a small motor or buzzer for button recognition.
3.3.4 EMI Emissions Testing
The system was tested in a certified EMI test facility according the CISPR-22 emissions standard. During
testing, there were two active buttons with a sensor oscillation frequency of 9MHz and an oscillation
amplitude of 1.3 V .
P
Figure 16. EMI data of LDC1612 with CISPR 22 mask
The blue line shows the maximum allowed emissions as per CISPR22 standard. The measured results in
Figure 16 show that the LDC solution passes the CISPR22 test across the whole frequency range of
interest.
SNOA951–June 2016 Inductive Sensing Touch-On-Metal Buttons Design Guide 15
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 16

Common-mode choke
(eg. SRF3216-222Y)
LDC CSENSOR
LSENSOR
33 pF 33 pF
Button A
Metal sheet Button B
(Ø=12 mm) (not shown)
Metal thickness = 0.8 mm 0.25 mm 0.55 mm
Coil Sensor A
(Ø=12mm)
PCB with
sensor coils Standoff for
mechanical
Bottom cutout
isolation
(Ø=20 mm)
Target
x
System Design Procedure www.ti.com
When using short trace lengths between the sensor and the IC, no external components are required to
reduce emissions in order to pass this test. When using larger sensors or remote sensing in a system with
long wires between the sensor and the IC, external passive components such a common-mode choke and
capacitor solution may be needed to pass emissions testing.
Figure 17. Using a Common-Mode Choke to Reduce EMI Emissions
3.4 Design Implementation
The panel for this example was manufactured as shown in Figure 18 to Figure 20 from a 0.8-mm thick
sheet of aluminum Al6061-T6 (all dimensions are in mm).
Figure 18. Side View (Button A)
16 Inductive Sensing Touch-On-Metal Buttons Design Guide SNOA951–June 2016
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 17

www.ti.com System Design Procedure
Figure 19. Top View
Figure 20. Bottom View
SNOA951–June 2016 Inductive Sensing Touch-On-Metal Buttons Design Guide 17
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 18

Force (N)
)mμ(
noitcelfeD
lateM
egarevA
1
0
10 20 30 40 50 60 70 80 90
Samples (sampled at 10 samples / second)
35
30
25
20
15
10
5
0
0 0.5 1 1.5 2
D003
)N(
ecroF
deilppA
Results www.ti.com
The recessed area on the outside metal surface, as shown in Figure 19, provides a clear indication to
users on where to physically press.
4 Results
Figure 21 shows the real-time output response when pressing the buttons with a finger. The LDC output is
repeatable and button presses do not cause significant interference. A configurable threshold is added in
software to detect button press events. More sophisticated software algorithms can also be applied to add
functionality such as multi-button support and multi-level force sensitivity, and to remove unwanted effects
which are caused by drastic changes in temperature or humidity, or permanent deformation of the metal.
These algorithms can be run on ultra-low power microcontrollers such as an MSP430.
Button Detection
LDC output (Button A)
Threshold set to 1 N
3 LDC output (Button B)
2
Figure 21. Real-Time Button Response Output
The buttons can be characterized by measuring the output response while different amounts of force are
applied to the center of the button.
Figure 22 shows the measurement data of the average metal deflection for the panel. From the data, it
can be seen that deflections as high as 15 μm can be achieved with as little as 1 N of force, which
represents a light button press.
Figure 22. Force vs Metal Deflection (Button A)
Figure 23 shows that applying 1 N of force to Button A results in an clearly detectable inductance change
of -1.07%.
18 Inductive Sensing Touch-On-Metal Buttons Design Guide SNOA951–June 2016
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 19

Force (N)
1.4%
Button 1 Response
Button 2 Response
1.2%
1%
e
g
a n 0.8%
h
C
y 0.6%
c
n
e
q u 0.4%
e
Fr
0.2%
0
Button 1 Pressed Button 2 Pressed
-0.2%
D005
æ Code change from baseline ö
SNR = 20 ́ log ç ÷
10
è
6 ́s
noise ø
egnahC
ecnatcudnI
www.ti.com Results
0
Button 1
Button 2
-0.5%
-1%
-1.5%
-2%
-2.5%
0 0.5 1 1.5 2
D004
Figure 23. Applied Force vs Sensor Frequency Change
Figure 24 shows how small the mechanical crosstalk is between the two buttons. Due to the mechanical
structure of the panel, the output reading in button B moves by 0.03% into the opposite direction. In
systems with poor mechanical isolation this opposite response could be larger, which could represent a
problem when the metal relaxes making it look like a button press. As long as the amount of opposite
deflection is less than button detection threshold it will not trigger a button press. This can be achieved
when following the guidelines detailed in this report.
Figure 24. Isolation Between Button A and Button B (1-N Force)
To calculate the SNR, Equation 2 is used.
(2)
Even at a very light button press of 0.5 N of force, the signal-to-noise ratio exceeds 47 dB, as shown in
Figure 25. This SNR is much higher than the recommended minimum SNR of 20 dB, which is required by
simple software algorithms to reliably detect button presses.
SNOA951–June 2016 Inductive Sensing Touch-On-Metal Buttons Design Guide 19
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 20

Force (N)
)Bd(
RNS
Summary www.ti.com
61
59
57
55
53
51
49
47
45
0.5 1 1.5 2
D006
Figure 25. Signal-to-Noise Ratio
5 Summary
LDC technology can be used to create robust, non-wearing, and versatile buttons for human-machine
interface needs. With proper mechanical and sensor design, an inductance-to-digital converter can detect
even very light button presses of 0.5 N.
The following guidelines are presented in this report:
1. Mechanical system design considerations including the metal material, thickness, and shape of the
buttons are discussed. The quantity and arrangement of buttons for mechanical isolation as well as the
optimal target-to-sensor spacing which influences the response of the system. Metal etching and
mounting techniques are also presented.
2. Best sensor design practices ensure that the LDC can detect microscopic deflection in metal.
3. Other considerations such as multiplexing multiple buttons, advanced button detection algorithms,
power consumption, and EMI performance are explained.
This design is scalable in both resolution and sample rate making the LDC useful in a wide variety of
applications from systems that need to detect small μm changes to systems that require update rates of
several hundred times per second.
6 Additional resources
• Find out more about the LDC1612 and download the LDC1612 datasheet.
• Design your sensor coil and start your system design in seconds with WEBENCH® Inductive Sensing
Designer.
• LDC sensor design guidelines are described in the LDC Sensor Design application note.
• More information on TI’s inductive sensing technology, including additional applications and devices,
can be found on the LDC homepage.
20 Inductive Sensing Touch-On-Metal Buttons Design Guide SNOA951–June 2016
Submit Documentation Feedback
Copyright © 2016, Texas Instruments Incorporated

## Page 21

IMPORTANT NOTICE FOR TI DESIGN INFORMATION AND RESOURCES
Texas Instruments Incorporated (‘TI”) technical, application or other design advice, services or information, including, but not limited to,
reference designs and materials relating to evaluation modules, (collectively, “TI Resources”) are intended to assist designers who are
developing applications that incorporate TI products; by downloading, accessing or using any particular TI Resource in any way, you
(individually or, if you are acting on behalf of a company, your company) agree to use it solely for this purpose and subject to the terms of
this Notice.
TI’s provision of TI Resources does not expand or otherwise alter TI’s applicable published warranties or warranty disclaimers for TI
products, and no additional obligations or liabilities arise from TI providing such TI Resources. TI reserves the right to make corrections,
enhancements, improvements and other changes to its TI Resources.
You understand and agree that you remain responsible for using your independent analysis, evaluation and judgment in designing your
applications and that you have full and exclusive responsibility to assure the safety of your applications and compliance of your applications
(and of all TI products used in or for your applications) with all applicable regulations, laws and other applicable requirements. You
represent that, with respect to your applications, you have all the necessary expertise to create and implement safeguards that (1)
anticipate dangerous consequences of failures, (2) monitor failures and their consequences, and (3) lessen the likelihood of failures that
might cause harm and take appropriate actions. You agree that prior to using or distributing any applications that include TI products, you
will thoroughly test such applications and the functionality of such TI products as used in such applications. TI has not conducted any
testing other than that specifically described in the published documentation for a particular TI Resource.
You are authorized to use, copy and modify any individual TI Resource only in connection with the development of applications that include
the TI product(s) identified in such TI Resource. NO OTHER LICENSE, EXPRESS OR IMPLIED, BY ESTOPPEL OR OTHERWISE TO
ANY OTHER TI INTELLECTUAL PROPERTY RIGHT, AND NO LICENSE TO ANY TECHNOLOGY OR INTELLECTUAL PROPERTY
RIGHT OF TI OR ANY THIRD PARTY IS GRANTED HEREIN, including but not limited to any patent right, copyright, mask work right, or
other intellectual property right relating to any combination, machine, or process in which TI products or services are used. Information
regarding or referencing third-party products or services does not constitute a license to use such products or services, or a warranty or
endorsement thereof. Use of TI Resources may require a license from a third party under the patents or other intellectual property of the
third party, or a license from TI under the patents or other intellectual property of TI.
TI RESOURCES ARE PROVIDED “AS IS” AND WITH ALL FAULTS. TI DISCLAIMS ALL OTHER WARRANTIES OR
REPRESENTATIONS, EXPRESS OR IMPLIED, REGARDING TI RESOURCES OR USE THEREOF, INCLUDING BUT NOT LIMITED TO
ACCURACY OR COMPLETENESS, TITLE, ANY EPIDEMIC FAILURE WARRANTY AND ANY IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT OF ANY THIRD PARTY INTELLECTUAL
PROPERTY RIGHTS.
TI SHALL NOT BE LIABLE FOR AND SHALL NOT DEFEND OR INDEMNIFY YOU AGAINST ANY CLAIM, INCLUDING BUT NOT
LIMITED TO ANY INFRINGEMENT CLAIM THAT RELATES TO OR IS BASED ON ANY COMBINATION OF PRODUCTS EVEN IF
DESCRIBED IN TI RESOURCES OR OTHERWISE. IN NO EVENT SHALL TI BE LIABLE FOR ANY ACTUAL, DIRECT, SPECIAL,
COLLATERAL, INDIRECT, PUNITIVE, INCIDENTAL, CONSEQUENTIAL OR EXEMPLARY DAMAGES IN CONNECTION WITH OR
ARISING OUT OF TI RESOURCES OR USE THEREOF, AND REGARDLESS OF WHETHER TI HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGES.
You agree to fully indemnify TI and its representatives against any damages, costs, losses, and/or liabilities arising out of your non-
compliance with the terms and provisions of this Notice.
This Notice applies to TI Resources. Additional terms apply to the use and purchase of certain types of materials, TI products and services.
These include; without limitation, TI’s standard terms for semiconductor products http://www.ti.com/sc/docs/stdterms.htm), evaluation
modules, and samples (http://www.ti.com/sc/docs/sampterms.htm).
Mailing Address: Texas Instruments, Post Office Box 655303, Dallas, Texas 75265
Copyright © 2018, Texas Instruments Incorporated
