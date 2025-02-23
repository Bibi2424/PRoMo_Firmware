# PRoMo_Firmware

This is the repo for the firmware from the PRoMo project.

## What you need to compile

ST provides a bundle of command line utilities necessary to compile and debug a cmake project: [STM32CubeCLT](https://www.st.com/en/development-tools/stm32cubeclt.html).

At the moment, the HAL library needs to be manually added to the project.

## Usefull documentation

 - Reference Manuel for the micro-controler: [RM0383 -Reference manual](https://www.st.com/content/ccc/resource/technical/document/reference_manual/9b/53/39/1c/f7/01/4a/79/DM00119316.pdf/files/DM00119316.pdf/jcr:content/translations/en.DM00119316.pdf). Usefull for a detailed description of the HW modules.
 - Datasheet of the uc : [STM32F411xE Datasheet](https://www.st.com/content/ccc/resource/technical/document/datasheet/b3/a5/46/3b/b4/e5/4c/85/DM00115249.pdf/files/DM00115249.pdf/jcr:content/translations/en.DM00115249.pdf). You can found the list of the pins alternate functions here.
 - You can also found the HAL User Manual with the STM32CubeF4 in _/<User>/STM32Cube/Repository/STM32Cube_FW_F4_V1.21.0/Drivers/STM32F4xx_HAL_Driver/STM32F411xE_User_Manual.chm_. You can see the details of every HAL functions and a list of the structures fields and possible values.

## Debugging

### OpenOCD

Install from package manager (choco for windows works)

```bash
openocd.exe -f ./tools/stlink.cfg -f ./tools/stm32f4x.cfg
```

### GDB

Needs to be `arm-none-eabi-gdb`. Should come with the `arm-none-eabi-` toolchain.

```bash
arm-none-eabi-gdb.exe {BUILD_FOLDER}/{TARGET}.elf -x ./tools/.gdbinit
```

### Setuping with Visual Studio Code

Use STM32 VSCode extension

https://www.st.com/content/st_com/en/campaigns/stm32-vs-code-extension-z11.html

## PyQt Plotter

```python
python .\pyqt_plotter.py -p COM8 --auto-connect -n Left Right
```