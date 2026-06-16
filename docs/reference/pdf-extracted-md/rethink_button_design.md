# Inductive sensing: rethink the button

- Source PDF: `../application_notes/rethink_button_design.pdf`
- Extraction tool: pdfplumber
- Page count: 3
- SHA256: `4deb2f51ddd05c9b9461392028e1726187a18d831cfdbf24afc4767fceb0e554`

## Page 1

www.ti.com
Technical Article
Inductive Sensing: Rethink the Button
Luke LaPointe
Inductive-sensing technology is causing designers to rethink many problems that have existed for decades.
Today, I will show you a modern approach to a button design that does not use any moving parts.
Traditionally, buttons on appliances and consumer electronics used a resistive contact solution that relies on the
proper functioning of a mechanical button. The use of moving parts is susceptible to long-term reliability issues
and may not be aesthetically pleasing. This approach also does not allow a complete seal of the case, making it
sensitive to moisture and other contaminants.
The use of capacitive buttons has solved some of the problems that mechanical buttons have, such as reliability
issues, but capacitive buttons are still affected by foreign substances such as water drops on the button’s
surface. Additionally, capacitive buttons are not operable with most types of gloves.
Inductance-to-digital conversion (LDC) with touch-on-metal (ToM) technology solves these issues while allowing
a modern-looking and extremely reliable button. The case can be manufactured from a single piece of metal (as
shown in Figure 1), making it highly cost-effective and immune to moisture damage.
Figure 1. ToM Design with Four Buttons
The sensor is a printed circuit board (PCB) coil on the inside of the casing, not exposed to the external
environment. The LDC takes advantage of the relationship between inductance and metal proximity, which
enables the LDC to translate metal deflection caused by an intentional button push to a detectable inductance
change. The use of TI 28-bit LDCs such as the LDC1612 and LDC1614 allow detection of sub-micrometer
changes in metal proximity, which is completely invisible to the naked eye and does not require a lot of force.
Figure 2 shows a button application with a 20mm-diameter circular button and a 0.25mm-thick aluminum sheet.
The button sits at a nominal height of 0.55mm, away from the PCB coil. A button push with typical force causes
a deflection of 5μm.
SSZTBS6 – DECEMBER 2015 Inductive Sensing: Rethink the Button 1
Submit Document Feedback
Copyright © 2023 Texas Instruments Incorporated

## Page 2

www.ti.com
Figure 2. Metal Deflection Caused by Button Push
The LDC1612 features a configurable resolution and sample rate to allow for the most flexibility when designing
the human-machine interface and metal-deflection distance. In this example, the LDC1612 is configured for
155SPS and 13 bits of resolution, which is sufficient to detect a metal deflection of <5μm at a nominal proximity
of 0.55mm. A microcontroller can process the data to detect when the inductance crosses the pre-determined
threshold. A haptic feedback event is also possible, thus providing tactile feedback and signifying a button push
to the user.
Figure 3. LDC1612 Output Response with Four Button Pushes
Figure 3 shows the LDC1612 raw output and threshold setting when the button is pressed four times. When the
LDC output passes the threshold, the microcontroller can signify that a button press has occurred and trigger
a haptic response. The threshold can be set higher or lower depending on whether you want weak or strong
button-push detection.
To see a complete design using the principles I’ve covered in this post, check out the TI Designs Touch on Metal
Buttons with Integrated Haptic Feedback Reference Design.
It is easy to see that with inductive technology and post-processing, the way we think about buttons has a new
path forward. The possibilities are endless. Log in to comment below and let me know the button applications
you’d like to improve.
Additional Resources
• Learn more about inductive sensing
• Check out other blog posts about inductive sensing
• Design your sensor coil and start your system design in seconds with the WEBENCH® Inductive Sensing
Designer
2 Inductive Sensing: Rethink the Button SSZTBS6 – DECEMBER 2015
Submit Document Feedback
Copyright © 2023 Texas Instruments Incorporated

## Page 3

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
