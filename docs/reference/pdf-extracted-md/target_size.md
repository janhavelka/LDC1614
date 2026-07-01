# Inductive sensing: target size matters

- Source PDF: `../application_notes/target_size.pdf`
- Extraction tool: pdfplumber
- Page count: 4
- SHA256: `ffc330a3e3296f5db0a0a9bf41f23ba95a5e87304ca7f001cf610a842e36202a`

## Page 1

www.ti.com
Technical Article
Inductive Sensing: Target Size Matters
Ben Kasemsadeh
In previous posts, we have been talking a lot about how sensor size affects sensing range. Using coils with
large diameters increases the inductive-sensing range; however, target size also has an impact. Sensing range
decreases when the target is smaller than the coil. In this post, I’ll investigate the extent of the relationship
between target size and sensing range.
Using the LDC1612, I collected data with a 29mm coil diameter and various target sizes of copper tape, as
shown in Figure 1.
Figure 1. Conductive Targets Made from Copper Tape (All Sizes in Millimeters)
SSZTBX4 – NOVEMBER 2015 Inductive Sensing: Target Size Matters 1
Submit Document Feedback
Copyright © 2023 Texas Instruments Incorporated

## Page 2

www.ti.com
The coil is “coil J” from the reference coil board evaluation module (EVM). The 29mm coil diameter represents a
sensing area of 660.5mm2. Table 1 shows the target dimensions.
Table 1. Coil parameters/target sizes
Target dimensions (mm) Target area (mm2) Target area/coil area (%)
3 x 3 9 1.4
5 x 5 25 3.8
8 x 8 64 9.7
10 x 10 100 15.1
15 x 15 225 34.1
20 x 20 400 60.6
25 x 25 625 94.6
30 x 30 900 136.3
35 x 35 1,225 185.5
40 x 40 1,600 242.2
50 x 50 2,500 378.5
I moved the target from 0.1mm to 14.5mm to represent a travel distance of half the coil diameter and recorded
the LDC1612 output codes. For comparison, each response is normalized relative to the maximum code change
of a 50mm-by-50mm target. Figure 2 shows that targets larger than the coil diameter have diminishing benefits
in sensitivity.
Figure 2. Increasing Target Size Has Diminishing Benefits beyond Coil Size
Figure 3 shows the code change that a given target has, normalized to the maximum code change of the
50-by-50mm target. For example, at a 5mm target distance, a 10-by-10mm target causes a frequency shift of
just 1.1%. A 30-by-30mm target achieves the same frequency shift at a 12.5mm target distance.
2 Inductive Sensing: Target Size Matters SSZTBX4 – NOVEMBER 2015
Submit Document Feedback
Copyright © 2023 Texas Instruments Incorporated

## Page 3

www.ti.com
Figure 3. Percentage of Maximum Frequency Shift vs. Target Distance (in Millimeters)
The data collected shows that not all target geometries produce the same response. Using larger targets
produces significantly higher sensitivity, which manifests itself either as an increased sensing range or improved
system accuracy. It is not beneficial, however, to increase the target diameter significantly past the coil
geometries.
While the data was collected using the LDC1612, this same concept applies to our other LDC devices like the
LDC1312, LDC1314, LDC1614, LDC0851, LDC1101, LDC2114, LDC2112, and LDC3114-Q1.
Have any questions[unreadable glyph] Please check out our additional resources below or start a new question on our forum.
Additional Resources
• Check out other blogs about inductive sensors.
• Check out our sensors forum.
• Learn more about TI’s inductive sensors home page.
• See more information on LDC applications in Common Inductive and Capacitive Sensing Applications (Rev.
B).
• Design your sensor coil and start your system design in seconds with the Inductive Sensing Design
Calculator Tool (Rev. G)
SSZTBX4 – NOVEMBER 2015 Inductive Sensing: Target Size Matters 3
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
