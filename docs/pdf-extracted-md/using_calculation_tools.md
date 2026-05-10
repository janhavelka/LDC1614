# How to use the LDC calculations tool

- Source PDF: `../application_notes/using_calculation_tools.pdf`
- Extraction tool: pdfplumber
- Page count: 5
- SHA256: `5e5403efa75a932c85456ae603e1e8ee1be87474fbef1482df923674d9b66e9a`

## Page 1

www.ti.com
Technical Article
How to Use the LDC Calculations Tool
Chris Oberhauser
While developing inductance-to-digital converters (LDCs) at TI, we often use a variety of spreadsheets to
determine the appropriate settings or capabilities. I decided to combine all of the various tools into a single
spreadsheet for convenience, and we’re releasing this spreadsheet to the web to help in your LDC system
design.
The spreadsheet runs on Microsoft Excel, and you can download it here. The calculator tool doesn’t use any
macros or special add-ons, so it is an easy download. While we do our best to make this tool as accurate as
possible, we don’t give any warranty on the results.
The starting point for the spreadsheet tool is the Contents tab, as shown in Figure 1. This tab has a list of all the
available calculation tools (see Table 1), each housed on a separate tab. Simply click the blue links to go to the
appropriate calculator.
To use this tool, enter your parameters in the yellow fields; the results are in the orange fields. Don’t change the
formulas in the orange tabs, or else you’ll have errors or incorrect calculations.
SSZTAN1 – NOVEMBER 2016 How to Use the LDC Calculations Tool 1
Submit Document Feedback
Copyright © 2023 Texas Instruments Incorporated

## Page 2

www.ti.com
Figure 1. The Contents Tab of the Excel Spreadsheet
2 How to Use the LDC Calculations Tool SSZTAN1 – NOVEMBER 2016
Submit Document Feedback
Copyright © 2023 Texas Instruments Incorporated

## Page 3

www.ti.com
Table 1. Spreadsheet Calculator Tabs
Calculator Tab Description
Racetrack Inductor Designer Designs a circular- or racetrack-shaped sensor.
Sample Rate Calculator Calculates the sample rates for various LDC devices.
Inductance and Frequency from Output Code Calculates sensor inductance and resonant frequency based on the
device output code. Can also calculate R from the output code for
P
the LDC1101, LDC1000, LDC1041 and LDC1051.
Skin Depth Calculation Skin depth is important for understanding performance based on
target thickness and material.
LDC161x/LDC131x Current Consumption Estimator Estimates the current consumption for the LDC131x and LDC161x
by putting the LDC into sleep mode or shutdown mode. The
application note, “Power Reduction Techniques for the Multichannel
LDCs in Inductive Sensing Applications,” covers this topic in detail.
LDC131x/LDC161x Sensor Configuration Calculates appropriate settings for the LDC131x and LDC161x
IDRIVE. The application note, “Configuring Inductive-to-Digital
Converters for Parallel Resistance (R ) Variation in L-C Tank
P
Sensors” has more information on this topic.
Remote Coil Maximum Distance Calculator LDC devices can operate when the sensor is located some distance
from the LDC. This tool calculates the maximum distance for a given
sensor.
LDC0851 Calculator Tool Calculates the LDC0851 functionality for a given sensor, including
sample rate, current consumption and estimated switching distance.
LDC1101 Rp Configuration Calculates the RP_MIN, RP_MAX, T1, C1, T2 and C2 for the
LDC1101.
Spring Sensor Calculator Tool While mechanical engineers think that a wound-wire coil is a spring,
electrical engineers know that it is really an inductor. This tool can
estimate performance when using a spring as the LDC sensor.
All tabs contain a link that says Return to Main Page at the top of the tab, which will take you back to the
Contents tab. See Figure 2.
Figure 2. Return Link
The Quick Sensor L/C/f Calculator is shown in Figure 3. It is a simple but very useful tool located right on the
Contents tab that calculates the inductance, capacitance or oscillation frequency of an LDC sensor. (Note that
this calculation doesn’t work for the LDC0851, which I’ll discuss in a future blog post.)
SSZTAN1 – NOVEMBER 2016 How to Use the LDC Calculations Tool 3
Submit Document Feedback
Copyright © 2023 Texas Instruments Incorporated

## Page 4

www.ti.com
Figure 3. Quick Sensor L/C/f Calculator on the Contents Tab
First, select which parameter you wish to calculate – fsensor, L or C. Then enter the two values into the yellow
fields. One nice thing about this calculator is that you can copy the 3-by-3 group of cells and paste the cells into
another spreadsheet, and the copy will work independently of the original.
Right below is the Rp/Rs/Q Calculator (refer to Figure 4), which is helpful for transforming between R and R . It
S P
works similarly to the L/C/f calculator.
Figure 4. Rp/RS/Q Calculator
In my next post, I will cover how to use the Racetrack Calculator tool to design a sensor.
Do you have any questions about or ideas for the LDC Calculator tool[unreadable glyph] Do you want to know more about a
specific aspect of this spreadsheet[unreadable glyph] If so, log in and leave a comment below.
Additional Resources
• Download these application reports:
– “LDC Device Selection Guide.”
– “LDC Sensor Design.”
4 How to Use the LDC Calculations Tool SSZTAN1 – NOVEMBER 2016
Submit Document Feedback
Copyright © 2023 Texas Instruments Incorporated

## Page 5

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
