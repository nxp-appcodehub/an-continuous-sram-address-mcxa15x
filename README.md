# NXP Application Code Hub
[<img src="https://mcuxpresso.nxp.com/static/icon/nxp-logo-color.svg" width="100"/>](https://www.nxp.com)

## AN14377: Continuous SRAM address usage on MCXA15x

This software accompanies application note [AN14377], which configures and uses the SRAM X0 Alias to form continuous SRAM address, and validates the feasibility of continuous SRAM address.

#### Boards: FRDM-MCXA156
#### Categories: Memory
#### Peripherals: DMA
#### Toolchains: MCUXpresso IDE, IAR, MDK

## Table of Contents
1. [Software](#step1)
2. [Hardware](#step2)
3. [Setup](#step3)
4. [Results](#step4)
5. [FAQs](#step5) 
6. [Support](#step6)
7. [Release Notes](#step7)

## 1. Software<a name="step1"></a>
- [MCUXpresso IDE V11.9.0 or later](https://www.nxp.com/design/software/development-software/mcuxpresso-software-and-tools-/mcuxpresso-integrated-development-environment-ide:MCUXpresso-IDE)
- [IAR 9.50.1](https://www.iar.com/)
- [Keil 5.38](https://www.keil.com/)
- [SDK_2.16.0_FRDM-MCXA156](https://mcuxpresso.nxp.com/en/welcome)

## 2. Hardware<a name="step2"></a>
- FRDM-MCXA156 Rev A board:

![FRDM-MCXA156](image/FRDM_MCXA156.png)

- One Type-C USB cable.

## 3. Setup<a name="step3"></a>
This software support MCUXpresso, IAR, and Keil IDEs at the same time, you can choose the corresponding project according to your needs.

### 3.1 Hardware connection
Use a Type-C USB cable to connect J21 of FRDM-MCXA156 and the USB port of the PC.

### 3.2 MCUXpresso import, build and download the project
1. Open MCUXpresso IDE 11.9.0, in the Quick Start Panel, choose **Import from Application Code Hub**

![import_from_ACH](image/import_from_ACH.png)


2. Enter the **demo name** in the search bar.

![ACH](image/ACH.JPG)

3. Click **Copy GitHub link**, MCUXpresso IDE will automatically retrieve project attributes, then click **Next>**.

![copy_github_link](image/copy_github_link.png)

4. Select **main** branch and then click **Next>**, select the MCUXpresso project, click **Finish** button to complete import.

> You need to install the [SDK_2.16.0_FRDM-MCXA156](https://mcuxpresso.nxp.com/en/welcome) on your MCUXpresso IDE.

5. Click **Build** button from the toolbar, then wait for the build to complete.

![MUCXpresso_Build](image/MCUXpresso_Build.png)

6. Select the **GUI Flash Tool** from the toolbar to program the executable to the board.

![MUCXpresso_Flash](image/MCUXpresso_Flash.png)

### 3.3 IAR import, build and download the project
1. Clone the project from the following link: https://github.com/nxp-appcodehub/an-continuous-sram-address-mcxa15x.git

2. Open, make and download the project.

![IAR_Make_Download](image/IAR_Make_Download.png)

### 3.4 Keil import, build and download the project
1. Clone the project from the following link: https://github.com/nxp-appcodehub/an-continuous-sram-address-mcxa15x.git

2. Open, build and download the project.

![Keil_Build_Download](image/Keil_Build_Download.png)

### 3.5 Validate the feasibility of continuous SRAM address
Open a serial terminal with 115200 baud rate, reset the MCU, then follow the prompts to validate the feasibility of continuous SRAM address.

## 4. Results<a name="step4"></a>
As shown below, the corresponding remap bits are enable and the value of the SP register is the end address of SRAM X0 Alias plus one. The above operation is completed before using the stack.

![Remap_bit_and_SP_register](image/Remap_bit_and_SP_register.png)

The below figure shows the read and write test for boundary unaligned address.

![Test_for_boundary_unaligned_address](image/Test_for_boundary_unaligned_address.png)

The below figure shows DMA access to continuous SRAM address test.

![DMA_access_to_continuous_SRAM_address](image/DMA_access_to_continuous_SRAM_address.png)


## 5. FAQs<a name="step5"></a>
No FAQs have been identified for this project.

## 6. Support<a name="step6"></a>
*Please contact NXP for additional support.*

#### Project Metadata
<!----- Boards ----->
[![Board badge](https://img.shields.io/badge/Board-FRDM&ndash;MCXA156-blue)](https://github.com/search?q=org%3Anxp-appcodehub+FRDM-MCXA156+in%3Areadme&type=Repositories)

<!----- Categories ----->
[![Category badge](https://img.shields.io/badge/Category-MEMORY-yellowgreen)](https://github.com/search?q=org%3Anxp-appcodehub+memory+in%3Areadme&type=Repositories)

<!----- Peripherals ----->
[![Peripheral badge](https://img.shields.io/badge/Peripheral-DMA-yellow)](https://github.com/search?q=org%3Anxp-appcodehub+dma+in%3Areadme&type=Repositories)

<!----- Toolchains ----->
[![Toolchain badge](https://img.shields.io/badge/Toolchain-MCUXPRESSO%20IDE-orange)](https://github.com/search?q=org%3Anxp-appcodehub+mcux+in%3Areadme&type=Repositories) [![Toolchain badge](https://img.shields.io/badge/Toolchain-IAR-orange)](https://github.com/search?q=org%3Anxp-appcodehub+iar+in%3Areadme&type=Repositories) [![Toolchain badge](https://img.shields.io/badge/Toolchain-MDK-orange)](https://github.com/search?q=org%3Anxp-appcodehub+mdk+in%3Areadme&type=Repositories)

Questions regarding the content/correctness of this example can be entered as Issues within this GitHub repository.

>**Warning**: For more general technical questions regarding NXP Microcontrollers and the difference in expected funcionality, enter your questions on the [NXP Community Forum](https://community.nxp.com/)

[![Follow us on Youtube](https://img.shields.io/badge/Youtube-Follow%20us%20on%20Youtube-red.svg)](https://www.youtube.com/@NXP_Semiconductors)
[![Follow us on LinkedIn](https://img.shields.io/badge/LinkedIn-Follow%20us%20on%20LinkedIn-blue.svg)](https://www.linkedin.com/company/nxp-semiconductors)
[![Follow us on Facebook](https://img.shields.io/badge/Facebook-Follow%20us%20on%20Facebook-blue.svg)](https://www.facebook.com/nxpsemi/)
[![Follow us on Twitter](https://img.shields.io/badge/Twitter-Follow%20us%20on%20Twitter-white.svg)](https://twitter.com/NXP)

## 7. Release Notes<a name="step7"></a>
| Version | Description / Update                           | Date                        |
|:-------:|------------------------------------------------|----------------------------:|
| 1.0     | Initial release on Application Code Hub        | July 5<sup>th</sup> 2024 |

