# PRoMo_Firmware

This is the repo for the firmware from the PRoMo project.

At the moment, based on STM32F411RE Nucleo board.

## What you need to compile

This project is compiled using a Makefile, but you can easily integrate that in your favorite IDE if you prefer to.
To be able to make, you need:

 - Install the GNU Arm Embedded Toolchain: https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads
 - You need the STM32cubeF4 HAL (You can have it throw [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html) or [directly](https://www.st.com/content/st_com/en/products/embedded-software/mcus-embedded-software/stm32-embedded-software/stm32cube-mcu-packages/stm32cubef4.html))
 - Go in the Makefile and update the PATH for both of them

To be able to flash the chip, I use a ST-LINK/V2 and the ST-LINK Utility CLI (that can be found [here](https://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-programmers/stsw-link004.html)).
Update the PATH for that in the Makefile also

## Usefull documentation

 - Reference Manuel for the micro-controler: [RM0383 -Reference manual](https://www.st.com/content/ccc/resource/technical/document/reference_manual/9b/53/39/1c/f7/01/4a/79/DM00119316.pdf/files/DM00119316.pdf/jcr:content/translations/en.DM00119316.pdf). Usefull for a detailed description of the HW modules.
 - Datasheet of the uc : [STM32F411xE Datasheet](https://www.st.com/content/ccc/resource/technical/document/datasheet/b3/a5/46/3b/b4/e5/4c/85/DM00115249.pdf/files/DM00115249.pdf/jcr:content/translations/en.DM00115249.pdf). You can found the list of the pins alternate functions here.
 - You can also found the HAL User Manual with the STM32CubeF4 in _\<User>/STM32Cube/Repository/STM32Cube_FW_F4_V1.21.0/Drivers/STM32F4xx_HAL_Driver/STM32F411xE_User_Manual.chm_. You can see the details of every HAL functions and a list of the structures fields and possible values.

