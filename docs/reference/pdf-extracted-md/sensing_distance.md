# Inductive sensing: How far can I sense[unreadable glyph]

- Source PDF: `../application_notes/sensing_distance.pdf`
- Extraction tool: pdfplumber
- Page count: 4
- SHA256: `a5daf27cab5b2467467bd414f65a0c7f420a7d9f921bab11d81aa3226a3caffe`

## Page 1

www.ti.com
Technical Article
Inductive Sensing: How Far Can I Sense[unreadable glyph]
Ben Kasemsadeh
In my last few blog posts on inductive sensing, I introduced the latest addition to TI’s inductance-to-digital
converter (LDC) portfolio: the multichannel LDC1612, LDC1614, LDC1312 and LDC1314. The LDC1312 and
LDC1314 are 13.3KSPS 12-bit resolution converters, useful for a wide range of applications such as rotational
knobs, keypads or flow meters. If system-boundary conditions allow, they can be configured in 16-bit mode
without sacrificing the sampling rate. The dual-channel LDC1612 and quad-channel LDC1614 have integrated
28-bit data converters for use in very high-precision applications such as linear encoders or strain gauges.
How Improved Resolution Increases Sensing Distance
The size of the magnetic field lines around a coil are proportional to the diameter of the sensor inductor.
Therefore, the maximum sensing distance of the LDCs is a function of the coil diameter. The resolution and
signal-to-noise ratio of the LDC do play a role, however, in determining how far a conductive object can be from
the coil in order to detect its presence or measure its distance.
Using a 28-bit LDC over a 12-/16-bit LDC has two advantages:
• You can determine target position to higher accuracy.
• You can detect targets at longer distances.
Methodology and Results
To determine the maximum target distance that the new multichannel LDCs can sense, I stepped an aluminum
target in 0.1mm increments axially from the 14-mm diameter sensor coil of the LDC1612 evaluation module
and captured the LDC response. I recorded both the code change between steps (Figure 1) and the standard
deviation of 100 samples per step (Figure 2).
SSZTCL3 – JUNE 2015 Inductive Sensing: How Far Can I Sense[unreadable glyph] 1
Submit Document Feedback
Copyright © 2023 Texas Instruments Incorporated

## Page 2

www.ti.com
Figure 1. Code Change for 0.1mm Steps versus Target Distance
Figure 2. Standard Deviation versus Target Distance, 100 Samples per Step
Next, I used the measurement data on resolution and standard deviation to determine the maximum target
sensing distance for the following conditions:
• The output code step size sufficient to resolve 0.1mm steps.
• The noise floor at the given target position is low enough to determine its position with 6σ probability
(99.99966%).
By examining the data, I identified the distance above which these two conditions no longer hold true.This point
is the maximum target distance that still meets my accuracy requirements above. The maximum target distance
in each case is shown in Figure 3.
2 Inductive Sensing: How Far Can I Sense[unreadable glyph] SSZTCL3 – JUNE 2015
Submit Document Feedback
Copyright © 2023 Texas Instruments Incorporated

## Page 3

www.ti.com
Figure 3. Maximum Target Distance for Resolving 0.1mm Steps with 6σ Probability
Guidelines for Setting Your Target Distance
The maximum target distance that you can sense varies with system parameters such as accuracy requirements
and target material composition. Therefore, your application may have a longer or shorter maximum sensing
range. However, since sensing range scales with coil diameter, it is possible to establish some rules of thumb
from the data:
• LDCs such as the LDC1312 and LDC1314 operate best if the maximum target distance is kept within half of a
coil diameter.
• In contrast, the high resolution of the channel LDC1612 and LDC1614 can be used to effectively sense
targets as far as two coil diameters away from the sensor.
In my next post, I’ll explain how to configure the LDC1312 in 16-bit mode, which is beneficial in medium-
resolution applications in which the faster sampling rates of the LDC1312 are preferable to the LDC1612.
Additional Resources
• Learn more about inductive sensing.
• Read more blog posts about designing with LDCs.
• Check out this TI Design reference design and video for a 1-degree dial using the LDC1314.
• See how LDCs work in this Touch on Metal Buttons with Integrated Haptic Feedback reference design.
• Start a design today with WEBENCH® Inductive Sensing Designer.
SSZTCL3 – JUNE 2015 Inductive Sensing: How Far Can I Sense[unreadable glyph] 3
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
