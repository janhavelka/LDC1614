# Inductive sensing: How to sense spring compression

- Source PDF: `../howto_guides/spring_compression.pdf`
- Extraction tool: pdfplumber
- Page count: 4
- SHA256: `ab250cc6029b6cc5544eec789b1dc090798fd8999c1a5abd5a759d3fadc3905a`

## Page 1

www.ti.com
Technical Article
Inductive Sensing: How to Sense Spring Compression
Ben Kasemsadeh
While most inductive-sensing applications use either printed circuit board (PCB) coils or wire-wound inductors
as the sensor, inductance-to-digital converters (LDCs) can use almost any inductor as a sensor – even a spring.
Springs are useful as sensors because the spring’s inductance varies directly with changes in length or other
physical changes. Figure 1 shows how to connect a spring to an LDC.
Figure 1. Spring Used as a Sensor by an LDC
To evaluate using a spring as a sensor, I used the LDC1612EVM evaluation module to measure the inductance
of a spring as I extended the spring across a range of lengths. To do this, I first removed the on-board sensor
from the EVM and replaced it with a spring. The spring was made of 0.7mm-thick steel, had 46 turns and a
diameter of 7.3mm. Figure 2 shows the spring that I connected to the EVM.
Figure 2. Spring Setup
SSZTCH7 – JULY 2015 Inductive Sensing: How to Sense Spring Compression 1
Submit Document Feedback
Copyright © 2023 Texas Instruments Incorporated

## Page 2

www.ti.com
The inductance of my spring is too low to be used as a sensor for the LDC1612 on its own, so I added a 2.2μH
fixed wire-wound surface-mount device (SMD) inductor in series. (For details on how to use a series inductor to
increase sensor impedance, see my blog post “How to use a tiny 2mm PCB inductor as a sensor.”) With a 1nF
sensor capacitor, the oscillation frequency was 2.5MHz. Figure 3 shows the sensor components that I used.
Figure 3. Sensor Components
I stretched the spring from 50mm to 100mm in 5mm increments and measured LDC1612 output data at each
step. From the data, I calculated the sensor inductance using Equation 1:
(1)
where
and f = reference clock (40MHz on the LDC1612 EVM).
ref
Figure 4 shows the data and spring inductance after subtracting the 2.2μH series inductor.
2 Inductive Sensing: How to Sense Spring Compression SSZTCH7 – JULY 2015
Submit Document Feedback
Copyright © 2023 Texas Instruments Incorporated

## Page 3

www.ti.com
Figure 4. LDC1612 Data and Spring Inductance versus Spring Length
The data samples that I collected when extending the spring from 50mm to 100mm in 5mm steps are monotonic
and can be used to precisely determine the length of the spring. During this spring-compression range, the
inductance decreases from 1.92μH (LDC output 16,644,000) to 1.01μH (LDC output 18,840,000). Thus, over this
range, stretching the spring by 1μm results in a 44-codes increment in the LDC1612 data output on average.
This data shows that you can use inductive sensing to directly measure the inductance shift that results from
compressing a spring, and that springs can serve as an alternative sensor to PCB coils and wire-wound
inductors.
Additional Resources
• Learn more about inductive sensing.
• Download the LDC1612 datasheet.
• Read more inductive sensing blogs, including “How to use a tiny 2mm PCB inductor as a sensor.”
• Check out WEBENCH® Inductive Sensing Designer.
SSZTCH7 – JULY 2015 Inductive Sensing: How to Sense Spring Compression 3
Submit Document Feedback
Copyright © 2023 Texas Instruments Incorporated

## Page 4

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
Copyright © 2023, Texas Instruments Incorporated
