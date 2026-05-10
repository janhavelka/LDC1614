# Inductive sensing: How to design an inductive sensor with the new WEBENCH Coil Des

- Source PDF: `../howto_guides/webench_coil_design.pdf`
- Extraction tool: pdfplumber
- Page count: 6
- SHA256: `6abcba5df7265630b744c8874b05002c5b21b75f312ceb1876862a2e7481f8e2`

## Page 1

www.ti.com
Technical Article
Inductive Sensing: How to Design an Inductive Sensor
with the New WEBENCH Coil Designer
Ben Kasemsadeh
Designing coils for inductive sensing may initially appear like a daunting task, but the WEBENCH® Coil Designer
makes the process very simple.
If you have designed with TI's inductance-to-digital converters (LDCs), then you may have already used
WEBENCH Designer for Inductive Sensing Applications, which suggests suitable coils for given input
parameters and exports them to PCB CAD tools.
Figure 1 is a screenshot of our second inductive sensing WEBENCH tool. WEBENCH Coil Designer designs
custom sensor coils for applications where you already know the system constraints. It supports real-world PCB
manufacturing constraints such as adjustment of trace geometries and PCB thickness.
Both tools export the resulting coil directly to popular PCB layout tools and allow complete sensor coil design in
only five minutes.
Figure 1. WEBENCH Offers Two Tools for Inductive Sensing
You can design a custom sensor coil in five simple steps:
SSZTCB1 – AUGUST 2015 Inductive Sensing: How to Design an Inductive Sensor with the New 1
Submit Document Feedback WEBENCH Coil Designer
Copyright © 2023 Texas Instruments Incorporated

## Page 2

www.ti.com
1. Select LDC Device:
Let’s use the LDC1612 to design a coil. While this selection does not impact the coil directly, we recommend
selecting the most appropriate LDC because WEBENCH Coil Designer considers device-specific boundary
conditions during its calculations (Figure 2).
Figure 2. WEBENCH Coil Designer Displays and Considers Device-specific Boundary Conditions
2. Select Coil Type:
WEBENCH Coil Designer supports four coil types. Circular coils are used for most applications because they
offer a higher Q-factor than the other choices, but sometimes system geometry requirements and sensor
inductance requirements dictate the use of square coils. Figure 3 defines inner and outer coil diameter, trace
width and trace spacing.
Figure 3. WEBENCH Coil Designer Supports Circular, Hexagonal, Octagonal and Square Coil
3. Select Coil Geometry and Other Parameters:
In this window, I specified the physical coil properties such as the number of turns, PCB layers and trace
geometries, as shown in Figure 4.
Trace width and trace spacing affect the manufacturing cost; narrower traces and spacing typically result in
more expensive PCBs. The outer coil diameter has the largest impact on the maximum sensing range. A coil fill
ratio (inner diameter / outer diameter) of ≥ 0.3 is recommended. Typically, smaller coil fill ratios do not improve
performance because the innermost turns add little inductance compared to the increase in AC resistance, and
therefore have a reduced Q-factor. The “view more” option shows advanced information about the sensor.
2 Inductive Sensing: How to Design an Inductive Sensor with the New SSZTCB1 – AUGUST 2015
WEBENCH Coil Designer Submit Document Feedback
Copyright © 2023 Texas Instruments Incorporated

## Page 3

www.ti.com
Figure 4. Output Parameters for Given Coil Geometries. Clicking ‘View More’ Displays Advanced Output
Parameters
The WEBENCH tool displays a warning if violations of device-specific boundary conditions or recommendations
occur. For example, a sensor with an oscillation frequency of 8MHz is suitable for the LDC1612, but not for the
LDC1000, which has a maximum sensor frequency of 5MHz (Figure 5).
Figure 5. Designing an 8MHz Sensor for the LDC1000 Produces a Warning and Recommendation for
Avoiding the System Constraint
SSZTCB1 – AUGUST 2015 Inductive Sensing: How to Design an Inductive Sensor with the New 3
Submit Document Feedback WEBENCH Coil Designer
Copyright © 2023 Texas Instruments Incorporated

## Page 4

www.ti.com
4. Output Graph:
The window in Figure 6 shows the sensor characteristics based on the inputs in step 3. The tool generates
a wide range of plots after you select the desired parameters from the drop-down menus, and compares the
performance of different coil types.
Figure 6. Output Parameters Plotted against Input Parameters and Compared to Different Coil Types
5. Export Design:
Finally, you can export the coil design into one of five different PCB CAD tools, as shown in Figure 7.
Figure 7. The Export Function Exports the Coil into PCB Layout Software
I exported the coil to the Altium Designer format and open the resulting file, as shown in Figure 8.
4 Inductive Sensing: How to Design an Inductive Sensor with the New SSZTCB1 – AUGUST 2015
WEBENCH Coil Designer Submit Document Feedback
Copyright © 2023 Texas Instruments Incorporated

## Page 5

www.ti.com
Figure 8. The Resulting Sensor Coil When Imported into Altium Designer
The new WEBENCH Coil Designer complements the WEBENCH Inductive Sensing Designer by offering more
control over the physical coil properties.
Do you find our WEBENCH tools for inductive sensing useful[unreadable glyph] Are there other WEBENCH tool features that
would make your system design with LDCs easier[unreadable glyph] If so, leave a note in the comments section below.
Additional Resources
• Read more inductive sensing blogs, including one I wrote about “Five-minute sensor coil design.”
• Watch a WEBENCH Coil Designer video.
• Read this application note for more information on how to design an LDC sensor.
• Check out additional tools in the WEBENCH Design Center.
SSZTCB1 – AUGUST 2015 Inductive Sensing: How to Design an Inductive Sensor with the New 5
Submit Document Feedback WEBENCH Coil Designer
Copyright © 2023 Texas Instruments Incorporated

## Page 6

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
