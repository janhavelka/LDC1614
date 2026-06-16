# Sensor Design for Inductive Sensing Applications Using LDC (Rev. C)

- Source PDF: `../application_notes/sensor_design.pdf`
- Extraction tool: pdfplumber
- Page count: 23
- SHA256: `7a743b2a05a74a574f9f912599cffd32b49d25dd1feb327a36800223052bea27`

## Page 1

www.ti.com Table of Contents
Application Report
Sensor Design for Inductive Sensing Applications
Using LDC
ABSTRACT
Getting the best performance out of an LDC requires a sensor suitable for the measurement. This app note
covers the parameters to consider when designing a sensor for a specific application. Specific areas of focus
include the physical routing characteristics of PCB based sensors, considerations for the sensor capacitor, and
techniques to minimize or compensate for parasitic effects.
Table of Contents
1 The Sensor.............................................................................................................................................................................. 2
1.1 Sensor Frequency..............................................................................................................................................................2
1.2 R and R ......................................................................................................................................................................... 3
S P
2 Inductor Characteristics........................................................................................................................................................ 4
2.1 Inductor Shape...................................................................................................................................................................4
2.2 Number of Turns................................................................................................................................................................ 5
2.3 Multiple Layers................................................................................................................................................................... 7
2.4 Inductor Size.....................................................................................................................................................................11
2.5 Self-Resonance Frequency..............................................................................................................................................13
3 Capacitor Characteristics.................................................................................................................................................... 17
3.1 Capacitor R , Q, and SRF............................................................................................................................................... 17
S
3.2 Effect of Parasitic Capacitance........................................................................................................................................ 17
3.3 Capacitor Placement........................................................................................................................................................17
4 Physical Coil Design............................................................................................................................................................ 18
4.1 Example Design Procedure Using WEBENCH................................................................................................................18
4.2 PCB Layout Recommendations....................................................................................................................................... 21
5 Summary............................................................................................................................................................................... 22
6 References............................................................................................................................................................................ 22
7 Revision History................................................................................................................................................................... 22
List of Figures
Figure 1-1. Sensor Frequency Versus Inductance and Capacitance.......................................................................................... 2
Figure 1-2. R and R .................................................................................................................................................................3
S P
Figure 2-1. RLC Model................................................................................................................................................................ 4
Figure 2-2. Axial Sensing with Circular Spiral Inductor................................................................................................................4
Figure 2-3. Trapezoidal Inductor for Rotational Encoding........................................................................................................... 5
Figure 2-4. Flat Circular Spiral Inductor.......................................................................................................................................5
Figure 2-5. Inductance Versus Number of Turns for 18-mm Circular Inductor............................................................................ 6
Figure 2-6. Multiple Layer Inductor Construction (Series)........................................................................................................... 7
Figure 2-7. Simplified Electrical Model of Four-Layer Series Sensor (Ignoring R ).................................................................... 8
P
Figure 2-8. Mutual Inductance for Two-Layer Sensor (Ignoring R )............................................................................................8
P
Figure 2-9. Multi-layer Parallel Coil Schematic............................................................................................................................9
Figure 2-10. Multi-layer Parallel Inductor...................................................................................................................................10
Figure 2-11. R Versus Normalized Target Distance................................................................................................................. 11
P
Figure 2-12. Inductance Versus Normalized Target Distance....................................................................................................11
Figure 2-13. Sensor “Diameter” for a Non-circular Coil............................................................................................................. 12
Figure 2-14. Parasitic Capacitive Components in an Inductor...................................................................................................13
Figure 2-15. Inductance T Versus Frequency............................................................................................................................14
Figure 2-16. Measurement of SRF with a VNA......................................................................................................................... 14
Figure 2-17. Wire-Wound Inductor Parasitic Capacitance.........................................................................................................15
Figure 2-18. Winding-Out Method............................................................................................................................................. 15
SNOA930C – MARCH 2015 – REVISED MAY 2021 Sensor Design for Inductive Sensing Applications 1
Submit Document Feedback Using LDC
Copyright © 2021 Texas Instruments Incorporated

## Page 2

Figure 2-19. Winding Crossing for Honeycomb Coil..................................................................................................................15
Figure 2-20. Combination of Both Winding-out and Crossing................................................................................................... 16
Figure 4-1. WEBENCH Coil Designer Tool................................................................................................................................18
Figure 4-2. Minimize Copper Around the Sensor...................................................................................................................... 21
Trademarks
WEBENCH® is a registered trademark of Texas Instruments.
All trademarks are the property of their respective owners.
1 The Sensor
LDC sensing applications use a sensor composed of an inductor in parallel with a capacitor to form an L-C tank
oscillator.
1.1 Sensor Frequency
The inductance and capacitance determine the sensor frequency, from the equation:
(1)
Figure 1-1 graphs several sensor frequency settings across capacitor and inductor values. For example, a
5-MHz sensor could use a 1-nF capacitor with an approximately 25-μH inductor. Refer to the Analog Wire blog
post Inductive Sensing: Sensor frequency constraints for more information on how this graph is constructed.
10,000.0
f= 5kHz
1,000.0
f= 10kHz
100.0
10.0
f= 10MHz f= 1MHz f= 100kHz
1.0
0.1
0.01 0.1 1 10 100 1000
)Hμ(
ecnatcudnI
Trademarks www.ti.com
Capacitance (nF)
Figure 1-1. Sensor Frequency Versus Inductance and Capacitance
TI’s LDC devices work over a wide frequency range, from 1 kHz to 10 MHz for the LDC1312 family and
LDC1612 family of devices. The LDC0851 can operate up to 19 MHz, and the LDC211x and LDC3114 can
support a sensor frequency up to 30 MHz.
It is important to remember that the frequency of operation changes based on the position of the target. Typically,
when the target is closest to the sensor, the sensor frequency is highest. The highest frequency cannot exceed
the specified operation range for the LDC.
2 Sensor Design for Inductive Sensing Applications SNOA930C – MARCH 2015 – REVISED MAY 2021
Using LDC Submit Document Feedback
Copyright © 2021 Texas Instruments Incorporated

## Page 3

www.ti.com The Sensor
1.2 R and R
S P
An inductive sensor is intrinsically lossy due to series losses in the conductor used to construct the inductor.
These resistive losses mainly come from two sources – the energy dissipated in the target or other nearby
conductors, and the distributed losses from the conductive windings of the inductor. When measuring R , the
P
intention is to measure only the eddy current losses on the target. The distributed losses in the sensor reduce
the measurement dynamic range of the LDC. Even when measuring inductance (L only) with an LDC131x or
LDC161x device, higher losses reduce the measurement accuracy.
Users can represent this loss electrically in one of two ways – as a series model or a parallel model, as shown in
Figure 1-2. In the series representation, the higher R , the more parasitic losses and the more energy the LDC
S
needs to drive into the sensor to maintain oscillation. This model matches the physics of the system more closely
than does the parallel model.
The parallel model is easier to determine what sensor current is needed to for a given sensor peak voltage.
With the parallel model, it is clear that if R is too low it attenuates the sensor oscillation. If the R becomes too
P S
high (which is the same as the R becoming too low), the LDC may not be able to effectively drive the sensor,
P
resulting in increased noise or even a collapse of the sensor oscillation.
The R can be calculated from the R with:
P S
R = L / (R C) = (2πƒ L)2/R
P S SENSOR S
From previous equation, it is clear that the R is a function of the sensor frequency and the sensor inductance.
P
The lowest sensor R that occurs in the system must be within the LDC drive capabilities.
P
L C
C
L R
P
or
R
S
Series Electrical Parallel Electrical
Model Model
Figure 1-2. R and R
S P
1.2.1 AC Resistance
As the sensor is oscillating a specific frequency range, it is critical to always use the AC resistance in the
frequency of interest. Unless specifically mentioned otherwise, the resistances in this app note are always the
AC resistance.
In general, trying to minimize R is recommended to improve the sensor performance.
S
1.2.2 Skin Effect
A DC current can take advantage of the entire cross section of a conductor. However, at higher frequencies
electrical current prefers to simply travel along the surface of the conductor. This tendency is called Skin Effect,
and is primarily a function of the conductivity and the frequency. With a copper conductor, more than 95% of a
1 MHz current flows in at the surface in a shell only 0.2 mm thick. At 10 MHz, the 95% of the current flows in a
shell only 0.06 mm from the surface of the conductor.
The skin effect is the primary source of the increase in the AC R at higher frequencies. This effect also affects
S
the eddy currents on the target surface - the generated eddy currents flow on the surface of the conductive
target closest to the inductor.
SNOA930C – MARCH 2015 – REVISED MAY 2021 Sensor Design for Inductive Sensing Applications 3
Submit Document Feedback Using LDC
Copyright © 2021 Texas Instruments Incorporated

## Page 4

2 Inductor Characteristics
To better understand how to obtain the optimum capabilities of a sensor, the sensor must be deconstructed into
the inductor and the capacitor components. There are several inductor characteristics to consider, and while it
may seem that there is an infinite range of possibilities for a given sensor, with use of the guidelines provided
here, users can quickly converge on a suitable sensor design.
A parallel R-L-C electrical model of the sensor is shown in Figure 2-1.
L R C
LDC
P
Figure 2-1. RLC Model
First, this app note needs to review the basic characteristics which determine the sensor capabilities.
2.1 Inductor Shape
The inductor shape is an important characteristic of the inductor because it determines the shape of the
generated magnetic field. A circular spiral generates a more symmetrical magnetic field than other shapes, and
is the optimum shape from an inductance verus R consideration. In general, it is recommended to use a circular
S
inductor for the highest possible sensing capabilities, unless specific system requirements warrant the tradeoffs
of an alternative shape.
For nearly all proximity applications, in which a target is moved orthogonally to the sensor plane, the appropriate
shape is a circular sensor that has the best Q (and lowest R ) for a given area, as shown in Figure 2-2.
S
x
Inductor Characteristics www.ti.com
Conductive Target
Figure 2-2. Axial Sensing with Circular Spiral Inductor
For other applications, different shapes may be more suitable. For PCB based sensors, designing alternative
shapes is much easier than wire-wound inductors. For example, rectangular coils can be used to sense accurate
movement along the X-axis while having reduced sensitivity to a shift in the Y-axis.
4 Sensor Design for Inductive Sensing Applications SNOA930C – MARCH 2015 – REVISED MAY 2021
Using LDC Submit Document Feedback
Copyright © 2021 Texas Instruments Incorporated

## Page 5

www.ti.com Inductor Characteristics
2.1.1 Example Uses of Different Inductor Shapes
Figure 2-3 is an example of a trapezoidal inductor. For a physically small rotational measurement application,
using a set of trapezoidal inductors increases the inductor area compared to circular coil; this increase in area
increases the sensor inductance. This is especially useful for very small sensor sizes, which may be lower than
the minimum inductance that the LDC can effectively drive. In such a case, it may be necessary to include an
additional series SMT inductor if the inductance is not high enough. Refer to the Analog Wire blog post Inductive
sensing: How to use a tiny 2mm PCB inductor as a sensor for more details.
Figure 2-3. Trapezoidal Inductor for Rotational Encoding
2.2 Number of Turns
d d
out in
Figure 2-4. Flat Circular Spiral Inductor
For a single-layer PCB spiral inductor, Mohan’s Equation, as discussed in Reference [1], is useful for
understanding how inductance is related to coil geometry. This equation can be used to calculate the overall
inductance of a coil for various geometries:
(2)
where
• K and K are geometry dependent, based on the shape of the inductor
1 2
• μ is the permeability of free space, 4π×10−7
o
• n is the number of turns of the inductor
• d is the average diameter of the turns = (d + d )/2
avg OUT IN
• ρ = (d – d )/(d + d ), and represents the fill ratio of the inductor – small values of ρ are a hollow
OUT IN OUT IN
inductor (d ≈ d ), while large values correspond to (d ≫ d )
OUT IN OUT IN
SNOA930C – MARCH 2015 – REVISED MAY 2021 Sensor Design for Inductive Sensing Applications 5
Submit Document Feedback Using LDC
Copyright © 2021 Texas Instruments Incorporated

## Page 6

Inductor Characteristics www.ti.com
• c are layout dependent factors based on the geometry (for a circle, use c = 1.0, c = 2.46, c = 0, c = 0.20),
i 1 2 3 4
refer to [1] for additional shapes
As the total inductance is proportional to the number of turns, adjusting the number of turns is an effective
control on the total inductance. However, when adding inner turns (which reduces the inner diameter), the d
avg
value begins to decrease, which reduces the additional inductance from the extra turns. For most applications,
the ratio of d /d must be greater than 0.3 for a higher inductor Q. The reason for this guideline is that
IN OUT
the inner turns, which do not have a significant area, do not contribute significantly to the overall inductance
while they still increase R . However, for applications where the target is very close to the sensor, such as
S
touch-on-metal, a ratio as low as 0.05 is often acceptable, as the inner turns provide increased sensitivity. Refer
to Reference [2] for more information.
Figure 2-5. Inductance Versus Number of Turns for 18-mm Circular Inductor
A key limitation on the number of turns that can be added is the practical minimum PCB trace width – a common
value is 0.1 mm (or 0.004 in.). Under this constraint, with each increase of 2 mm in sensor diameter up to 5
additional turns can be added to a PCB inductor.
In Figure 2-5, it can be seen that the first few turns contribute the most inductance, while the last few turns
contribute less inductance. This example with an 18-mm outer diameter coil with 0.15-mm trace width and trace
spacing shows that the total inductance levels off at approximately 20 turns.
6 Sensor Design for Inductive Sensing Applications SNOA930C – MARCH 2015 – REVISED MAY 2021
Using LDC Submit Document Feedback
Copyright © 2021 Texas Instruments Incorporated

## Page 7

www.ti.com Inductor Characteristics
2.3 Multiple Layers
For PCB inductors, there is a maximum number of turns that can be placed in a given diameter. If the overall
inductance of the sensor is still too low, adding an additional inductor on another layer increases the total
inductance. Note that these additional inductors need to be physically aligned, as shown in Figure 2-6, so that
the magnetic fields positively add. Electrically, the inductors are connected in series. In general, with a PCB
spiral inductor, a second layer is generally available because most PCBs are at least two layers.
When routing multiple layers, it is important to alternate the rotation of the coils – if the top layer is routed
with a clockwise rotation, the next layer down must have a counter-clockwise rotation. While this may seem
counter-intuitive, this physical arrangement is needed in order to keep the current rotating in a constant direction.
Current Direction
Counter-Clockwise Out
Spiral on Layer 1
Via from Layer 1 to Layer 2 Sensor
Capacitor
Current Direction
Clockwise Out Spiral
on Layer 2
Via from
Via from
Layer 2 to
Current
Layer 4 to
Layer 3
Direction
Layer 1
Counter-Clockwise
Out Spiral on Layer 3
Via from Layer 3 to Layer 4
Current Direction
Clockwise Out Spiral
on Layer 4
Figure 2-6. Multiple Layer Inductor Construction (Series)
SNOA930C – MARCH 2015 – REVISED MAY 2021 Sensor Design for Inductive Sensing Applications 7
Submit Document Feedback Using LDC
Copyright © 2021 Texas Instruments Incorporated

## Page 8

Inductor Characteristics www.ti.com
L
S-LAYER1
C
L SENSOR
S-LAYER2
L
S-LAYER3
L
S-LAYER4
Figure 2-7. Simplified Electrical Model of Four-Layer Series Sensor (Ignoring R )
P
2.3.1 Mutual Inductance of Coils in Series
With multi-layer coils aligned as in Figure 2-6, the magnetic fields between each of the layers couple, resulting in
an increase in the overall inductance. The equivalent circuit model for a two coil arrangement is shown in Figure
2-8.
L
1
M
C
SENSOR
L
2
Figure 2-8. Mutual Inductance for Two-Layer Sensor (Ignoring R )
P
Because the linkage between the coil fields is positive, the total inductance of the coils in series is:
L = L + L + 2M (3)
TOTAL 1 2
where
• L = inductance of coil 1,
1
• L = inductance of coil 2, which is typical the same as L as the geometries are the same, and
2 1
• M = mutual inductance between the coils = k × √(L ×L )
1 2
The parameter k is a measure of the flux linkage between the coils and varies between 0 and 1, and depends
only on the distance between the coils. The mutual inductance is stable and, as shown by Equation 3, increases
the total inductance beyond the simple sum of the individual inductances.
8 Sensor Design for Inductive Sensing Applications SNOA930C – MARCH 2015 – REVISED MAY 2021
Using LDC Submit Document Feedback
Copyright © 2021 Texas Instruments Incorporated

## Page 9

www.ti.com Inductor Characteristics
In general, for N coils connected in series with positively linked fluxes, the total inductance is given by:
N § N(cid:16)1 N ·
L ¦ L (cid:14) 2 ̃ ̈ ¦ ¦ M ̧
TOTAL i ̈ j,m ̧
i 1 © j 1 m j(cid:14)1 1 (4)
Equation 4 takes into account that there is flux linkage between coils in adjacent layers, plus between coils
in alternating layers. Because the degree of mutual inductance between coils is dependent on the separation
between layers, the individual mutual inductances vary.
For example, an 18-mm outer diameter, 0.15-mm trace width and trace spacing coil with 12 turns has an
inductance of 3.5 μH. If a multi-layer inductor is constructed using of 4 of these single-layer inductors in a 1.0
mm thick PCB, the total inductance is not 4×3.5 μH (14 μH), but is actually 39 μH. The additional 25 μH mutual
inductance is actually greater than the 14 μH from the four coils. This mutual inductance has the additional
beneficial property of not increasing the sensor R .
S
Texas Instruments online WEBENCH® tool supports multi-layer coil design and eliminates the need to employ
complex calculations. This can be accessed at http://webench.ti.com/wb5/LDC/#/spirals.
2.3.2 Multi-Layer Parallel Inductor
Minimizing the R is necessary to obtain the highest resolution R measurements. This also improves the L
S P
measurements for the LDC161x and LDC131x devices. With four or more layers users can start using the
parallel coil structure shown in Figure 2-9 and Figure 2-10 to lower R . This design may be useful for some
S
applications which need to optimize R measurements.
P
L L
S-LAYER1 S-LAYER3
R C
R S-LAYER3 SENSOR
S-LAYER1
L L
S-LAYER2 S-LAYER4
R R
S-LAYER2 S-LAYER4
Figure 2-9. Multi-layer Parallel Coil Schematic
SNOA930C – MARCH 2015 – REVISED MAY 2021 Sensor Design for Inductive Sensing Applications 9
Submit Document Feedback Using LDC
Copyright © 2021 Texas Instruments Incorporated

## Page 10

Inductor Characteristics www.ti.com
Counter-Clockwise Out
Spiral on Layer 1
Clockwise Out Spiral on
Layer 2 Sensor
Capacitor
Counter-Clockwise Out
Spiral on Layer 3
Clockwise Out Spiral on
Layer 4
Figure 2-10. Multi-layer Parallel Inductor
2.3.3 Temperature Compensation
When the inductor is composed of a pair of coils on multiple layers, the multiple layers result in an inductor with
improved temperature stability. The application note LDC100x Temperature Compensation discusses this effect
in more detail.
10 Sensor Design for Inductive Sensing Applications SNOA930C – MARCH 2015 – REVISED MAY 2021
Using LDC Submit Document Feedback
Copyright © 2021 Texas Instruments Incorporated

## Page 11

www.ti.com Inductor Characteristics
2.4 Inductor Size
The primary inductor characteristic that determines sensing range is the physical size of the inductor. More
specifically, the outer diameter (d ) determines the sensing range. In order to sense the movement of a target,
OUT
the magnetic field of the sensor needs to extend to the target. The physical size of the inductor controls the
physical size of the generated magnetic field. The change in sensor inductance as a function of the target
distance has a consistent shape, as seen in Figure 2-11 and Figure 2-12. The change in inductance versus
the change in target distance has such a consistent shape that users can effectively scale the response to the
sensor diameter (d ) to simplify system design.
OUT
It may be surprising to learn that the overall inductance of the sensor, which corresponds to the intensity of the
magnetic field, does not significantly affect the sensing range.
Figure 2-11 and Figure 2-12 show the transfer functions of an example 14-mm diameter sensor. While a different
inductor geometry has a different scale on the Y-axis on the graphs, the overall shape is similar as the target is
proportionally moved. As the distance between the sensor and target decreases, both L and R decrease. The
P
amount of the decrease is based on the target size and composition, although for some target materials the L
and R can actually increase as the target gets closer. It must be noted that in the sensor/target scenario, the
P
flux linkage between the sensor and target does not add, but instead, the fields oppose one another, so that the
mutual inductance between the two has on a negative sign.
Figure 2-11. R Versus Normalized Target Distance
P
Figure 2-12. Inductance Versus Normalized Target Distance
SNOA930C – MARCH 2015 – REVISED MAY 2021 Sensor Design for Inductive Sensing Applications 11
Submit Document Feedback Using LDC
Copyright © 2021 Texas Instruments Incorporated

## Page 12

Inductor Characteristics www.ti.com
For LDC131x and LDC100x devices, the effective sensing range is approximately one-half the sensor diameter.
While the L versus distance response is the same for LDC161x devices, the higher resolution of the LDC161x
enables it to effectively sense target shifts at much larger distances – up to twice the sensor diameter, but with
reduced effective measurement resolution. The LDC211x and LDC3114 devices have an equivalent sensing
range to the LDC161x. For the LDC0851, the maximum switching distance is 40% of the coil diameter.
If a system requires a sensing range of 4 mm, then the minimum sensor size must be 8 mm; and an even larger
sensor provides improved measurement resolution. This leads to the first rule of thumb for inductive sensor
design: use the biggest coil that can physically fit in the application. The second rule of thumb for inductive
sensor design is that the target must be similar in size to the sensor.
When using rectangular inductors or other non-circular shapes, the sensing range is based on the smaller axis
rather than the longer axis, as shown in Figure 2-13.
Ø
EFFECTIVE
Figure 2-13. Sensor “Diameter” for a Non-circular Coil
12 Sensor Design for Inductive Sensing Applications SNOA930C – MARCH 2015 – REVISED MAY 2021
Using LDC Submit Document Feedback
Copyright © 2021 Texas Instruments Incorporated

## Page 13

www.ti.com Inductor Characteristics
2.5 Self-Resonance Frequency
Figure 2-14 shows how the individual turns of an inductor have a certain physical area and are separated by
a dielectric; this produces a small parasitic capacitor across each turn. At a sufficiently high frequency, signals
find it easier to simply jump across the distributed parasitic capacitance rather than traverse along the spiral
winding of the trace. The frequency where these two paths are matched is self-resonance frequency, which is
represented as ƒ or SRF. Because the parasitic capacitance is not very stable, it is recommended to keep the
SR
sensor frequency below 75% of the SRF.
Figure 2-14. Parasitic Capacitive Components in an Inductor
SNOA930C – MARCH 2015 – REVISED MAY 2021 Sensor Design for Inductive Sensing Applications 13
Submit Document Feedback Using LDC
Copyright © 2021 Texas Instruments Incorporated

## Page 14

Inductor Characteristics www.ti.com
2.5.1 Measurement of SRF
The SRF of an inductor can be easily measured with an impedance analyzer. With an impedance analyzer, the
SRF is measured by simply connecting the inductor and plotting the magnitude of the impedance and phase
versus frequency. The frequency at which θ=0° corresponds to the SRF. In the example shown in Figure 2-15,
the SRF occurs at 4.6 MHz. Note that the SRF is a function of the target interaction — when the target is
interaction is stronger, the SRF almost always increases.
Figure 2-15. Inductance T Versus Frequency
A Vector Network Analyzer (VNA) is also able to perform this measurement. As always with VNAs, it is important
to properly calibrate the VNA. The calibration plane must be at the connections to the inductor, otherwise the
offset introduces a transmission line into the measurement that results in a measurement error. Once calibrated,
simply measure S11 over an appropriate frequency span. The SRF is the frequency where the S graph
11
crosses the real axis close to the open, as indicated in Figure 2-16.
Figure 2-16. Measurement of SRF with a VNA
14 Sensor Design for Inductive Sensing Applications SNOA930C – MARCH 2015 – REVISED MAY 2021
Using LDC Submit Document Feedback
Copyright © 2021 Texas Instruments Incorporated

## Page 15

www.ti.com Inductor Characteristics
2.5.2 Techniques to Improve SRF for Wire-wound Inductors
Wire-wound inductors can typically have a very large numbers of turns (or windings) and, as a result, while they
can have very high inductances and relatively high R values, they often have a low SRF.
P
Parasitic Capacitance across windings
Figure 2-17. Wire-Wound Inductor Parasitic Capacitance
The SRF is a highly dependent on the geometry of the windings. With wire-wound inductors composed of large
numbers of turns, there are specific winding patterns that can reduce the parasitic capacitance and therefore
increase the SRF. One of the first methods is to wind the coil out, then wind across, as shown in Figure 2-18.
This method reduces area between turns with a large voltage differential.
Figure 2-18. Winding-Out Method
A second winding method, shown in Figure 2-19, is to configure windings so that they cross at an angle, which
reduces the cross-sectional area between windings. Crossing angles must be as close to 90° as possible. A coil
designed in this manner is often called a Honeycomb coil.
spacer
Coils cross at an angle ±
the closer to 90° the better
Figure 2-19. Winding Crossing for Honeycomb Coil
SNOA930C – MARCH 2015 – REVISED MAY 2021 Sensor Design for Inductive Sensing Applications 15
Submit Document Feedback Using LDC
Copyright © 2021 Texas Instruments Incorporated

## Page 16

Inductor Characteristics www.ti.com
It is an acceptable to combine the techniques to produce groups of windings, as shown in Figure 2-20.
Figure 2-20. Combination of Both Winding-out and Crossing
16 Sensor Design for Inductive Sensing Applications SNOA930C – MARCH 2015 – REVISED MAY 2021
Using LDC Submit Document Feedback
Copyright © 2021 Texas Instruments Incorporated

## Page 17

www.ti.com Capacitor Characteristics
3 Capacitor Characteristics
The capacitor used in the sensor is a critical, yet often overlooked component. In LDC applications, we
recommend using C0G-grade ceramic capacitors (they are also referred to as NP0 capacitors). These
capacitors use the highest quality ceramic dielectric, and as a result they are very stable and avoid many of
the non-idealities capacitors using other dielectrics:
• They are not polarized.
• They exhibit minimal aging shifts.
• Excellent temperature stability at only ±30 ppm/°C.
• Very low ESR (equivalent series resistance) – typically less than a few mili-Ω.
• No piezoelectric effects – some dielectrics convert physical stresses into electrical shifts; which is
undesirable.
• Can operate at very high frequencies.
• Exhibit almost no dC/dV effects, in which the voltage across the capacitance affects the capacitance.
• Generate minimal distortion.
• Available in voltage ratings suitable for LDC operation.
However, compared to other dielectrics, they do not have a large amount of capacitance per unit volume, so
it becomes difficult to find C0G capacitors of more than 0.47 μF. For capacitance values of 47 pF to 3300 pF
typically used for LDC sensors, C0G capacitors are available in small footprints and are reasonably priced.
3.1 Capacitor R , Q, and SRF
S
The R of the sensor capacitor is added to the sensor, but this contribution is typically very low compared to the
S
inductor – for a 1000-pF 0603 C0G capacitor, the R is typically around 20 mΩ (compared to 2 Ω or more for
S
the inductor). Due to its very low R , the capacitor Q is typically very high – well over 100, and reaching 1000 in
S
some cases. However, the lower Q of the inductor is what dominates the response.
Capacitors also have a SRF, but it is typically much higher than the SRF of the inductor due to the smaller
physical size and the construction. For example, 1000-pF capacitors with SRF above 200 MHz are readily
available. Since the SRF is so much higher than that of the sensor inductor, it typically does not affect the
performance of the sensor.
3.2 Effect of Parasitic Capacitance
While the sensor capacitance seems simple, it is quite easy to get a few pF of parasitic capacitance in
many physical systems. Generally, parasitic capacitances are not very stable. Because users expect only the
inductance of the sensor to vary, variable capacitance causes measurement inaccuracy. For example, with a
sensor of 100 μH in parallel with 10.1 pF, an additional 0.5 pF of parasitic capacitance causes the free-air sensor
frequency to shift from 5 MHz to 4.88 MHz. This is equivalent to a shift in this inductance of this sensor of 5
μH. If the absolute value of the inductance is the critical measurement, then a 5% error has been introduced
by this shift in the parasitic capacitance. If the sensor capacitor is increased to 704 pF, the sensor frequency is
nominally 600 kHz, but the same parasitic shift of 0.5 pF causes only produce a shift from 600 Hz to 599.8 kHz;
which only appears as an inductance shift of 0.07 μH, which is an error of less than 0.07%.
Some of the sources of parasitic capacitances include the wiring connecting the LDC pins to the inductor, the
ESD structures of the LDC input pins, and the actual turns of the inductor. Standard PCB design techniques
such as minimizing ground floods near the inductive sensors can reduce the parasitic capacitance.
3.2.1 Recommended Capacitor Values
For most application, a sensor capacitance of 300 pF to 2 nF is generally recommended unless a specific sensor
frequency is needed. For some touch-on-metal applications in which the sensor is placed very close to the
conductive target, smaller sensor capacitances down to 47 pF may be optimum.
3.3 Capacitor Placement
For the LDC1101, LDC131x, and LDC161x devices, it is recommended to place the capacitor physically as close
as possible to the inductor to minimize the parasitic R due to the traces between the inductor and capacitor. For
S
the LDC0851, LDC211x and LDC3114 devices, the sensor capacitor must be placed as close as possible to the
appropriate device pin.
SNOA930C – MARCH 2015 – REVISED MAY 2021 Sensor Design for Inductive Sensing Applications 17
Submit Document Feedback Using LDC
Copyright © 2021 Texas Instruments Incorporated

## Page 18

Physical Coil Design www.ti.com
4 Physical Coil Design
4.1 Example Design Procedure Using WEBENCH
WEBENCH provides two methods for PCB coil design. The first method finds a coil design that fits a specific
application, in which the desired measurement capabilities are used to recommend a sensor.
The second method is an interactive design tool in which the coil size, shape, number of turns, and other
parameters can be adjusted to optimize the sensor design to fit a specific need. This tool can be accessed at
WEBENCH Inductor Design Tool.
Figure 4-1. WEBENCH Coil Designer Tool
The top-middle pull-down menu selects the desired coil shape – choices include:
• Circular
• Square
• Hexagonal
• Octagonal
18 Sensor Design for Inductive Sensing Applications SNOA930C – MARCH 2015 – REVISED MAY 2021
Using LDC Submit Document Feedback
Copyright © 2021 Texas Instruments Incorporated

## Page 19

www.ti.com Physical Coil Design
As discussed earlier, for most applications a circular sensor is the optimum shape, although square coils may be
useful for smaller geometry sensors.
The lower-left section of the screen is where various sensor parameters can be entered and adjusted.
Parameters to enter include the size, number of turns, and PCB fabrication characteristics.
The middle section reports the sensor characteristics based on the current settings.
Finally, the sensor design can be exported to the following output formats:
• Altium Designer
• Cadence Allegro 16.0-16.6
• CadSoft EAGLE PCB (v6.4 or newer)
• DesignSpark PCB
• Mentor Graphics PADS PCB
The output can then be imported into CAD tools to complete a system design.
SNOA930C – MARCH 2015 – REVISED MAY 2021 Sensor Design for Inductive Sensing Applications 19
Submit Document Feedback Using LDC
Copyright © 2021 Texas Instruments Incorporated

## Page 20

Physical Coil Design www.ti.com
4.1.1 General Design Sequence
The recommended sensor design process is:
1. Determine the maximum size inductor that can physically be used in the application, and set the d value
OUT
to that size. If the sensor is near the edge of a PCB, be sure to include any edge clearance rules from the
PCB manufacturing rules.
2. Based on the PCB manufacturing rules, set:
a. Trace Width and Trace Space to the minimum permitted. This value is commonly between 4 mils (0.1
mm) and 6 mils (0.15 mm).
b. Copper thickness; this is commonly 1 oz-cu (approximately 35 μm). Thicker is better.
3. Set the Number of Layers to match the number of board layers in the design.
4. Set the Capacitance — the range of 300 pF to 2 nF is typically near the optimum unless a specific sensor
frequency is needed. Adjust the capacitance as needed if the sensor frequency is not with the device limits.
5. Set the number of turns so that the ratio of D / D is > 0.3.
in out
6. Export the design into the desired format.
20 Sensor Design for Inductive Sensing Applications SNOA930C – MARCH 2015 – REVISED MAY 2021
Using LDC Submit Document Feedback
Copyright © 2021 Texas Instruments Incorporated

## Page 21

www.ti.com Physical Coil Design
4.2 PCB Layout Recommendations
4.2.1 Minimize Conductors Near Sensor
To keep the R as high as possible, keep ground planes and any thick traces away from the sensor — minimize
P
any conductors within at least 30% of the diameter of sensor. This includes ground planes and power planes. Do
not place a ground pour around the sensor or use thieving on the board. In Figure 4-2, the ground pour on the
inner tan layer has been recessed away from the coil to reduce coupling to the inner grounds.
Figure 4-2. Minimize Copper Around the Sensor
4.2.2 Sensor Vias and Other Techniques for PCBs
• Inner vias must be placed close to the traces and not in the absolute center of the inductor — this reduces
the parasitic resistance.
• Keep the inner 30% of the sensor area unwound for most applications, but for inductive metal button
replacement, place as many turns as possible.
• The sensor capacitor must be placed as close as possible to the sensor to minimize the R of the traces.
S
Note that this rule does not apply to LDC0851, LDC2112, LDC2114, and LDC3114 devices. For these
devices, place the sensor capacitor close to the corresponding device pin.
• Use thicker traces between the inductor and capacitor when possible. Typically 10 mil (0.25 mm) is sufficient.
• Do not use a ferrite bead as a sensor inductor — it does not generate a magnetic field effective for LDC
applications.
SNOA930C – MARCH 2015 – REVISED MAY 2021 Sensor Design for Inductive Sensing Applications 21
Submit Document Feedback Using LDC
Copyright © 2021 Texas Instruments Incorporated

## Page 22

Summary www.ti.com
5 Summary
This application report has reviewed the main characteristics of sensors, various geometries, the uses of them,
and how to construct wire-wound and PCB inductors. The core principles for sensor design — use the biggest
sensor that can fit in the application, use a circular coil if possible for optimum R , and leave the center 30% of a
S
PCB sensor open are effective guidelines.
Remember that coil diameter must be larger than twice the maximum expected sensing distance to maintain
adequate resolution.
Use NP0/C0G grade capacitors for the sensor capacitor, which are placed as close to the inductor as possible.
Also use larger sensor capacitor values to minimize the effects of parasitic capacitances.
6 References
[1] S. S. Mohan, M.del Mar Hershenson, S. P. Boyd, and T. H. Lee, “Simple Accurate Expressions for Planar
Spiral Inductances,” IEEE Journal OF Solid-state Circuits, vol. 34, no. 10, pp 1419-1424, Oct. 1999.
[2] Y.Su, X. Liu, C.K. Lee, and S.Y. Hui, “On the Relationship of Quality Factor and Hollow Winding Structure of
Coreless Printed Spiral Winding (CPSW) Inductor,” IEEE Transactions on Power Electronics, vol. 27, no. 6, pp.
3050–3056, Jun. 2012.
• Texas Instruments, Inductive Sensing FAQ
• Texas Instruments, Inductive Sensing Design Calculator Tool
7 Revision History
NOTE: Page numbers for previous revisions may differ from page numbers in the current version.
Changes from Revision B (September 2019) to Revision C (May 2021) Page
• Updated title........................................................................................................................................................1
• Updated the numbering format for tables, figures, and cross-references throughout the document..................1
• Added references to the LDC3114 device.......................................................................................................... 1
• Updated application references for clarity.......................................................................................................... 1
Changes from Revision A (April 2018) to Revision B (September 2019) Page
• Updated language for clarity throughout.............................................................................................................1
• Added references to the LDC1001 device..........................................................................................................1
Changes from Revision * (March 2015) to Revision A (April 2018) Page
• Added information on LDC0851 and LDC211x device performance.................................................................. 2
• Changed phrasing of various sections for better clarity......................................................................................3
• Added details on calculation of R .....................................................................................................................3
P
• Added sensing range information on LDC0851 and LDC211x devices............................................................ 11
• Added sensor capacitor placement guidelines for the LDC0851 and LDC211x devices..................................17
22 Sensor Design for Inductive Sensing Applications SNOA930C – MARCH 2015 – REVISED MAY 2021
Using LDC Submit Document Feedback
Copyright © 2021 Texas Instruments Incorporated

## Page 23

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
