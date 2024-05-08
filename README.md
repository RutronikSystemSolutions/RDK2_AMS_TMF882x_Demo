# RDK2 ams MS-TMF882x-01 Code Example

Rutronik Development Kit Programmable System-on-Chip CY8C6245AZI-S3D72 "RDK2 ams MS-TMF882x-01" Code Example. 

This code example demonstrates the ams ToF sensor's TMF8820 and TMF8828 capabilities.

 <img src="images/rutdevkit_model.png" style="zoom:20%;" />

## Requirements

- [ModusToolbox® software](https://www.infineon.com/cms/en/design-support/tools/sdk/modustoolbox-software/) **v3.x** [built with **v3.0**]

## Supported toolchains (make variable 'TOOLCHAIN')

- GNU Arm&reg; Embedded Compiler v11.3.1 (`GCC_ARM`) - Default value of `TOOLCHAIN`

## Using the code example

Create the project and open it using one of the following:

<details><summary><b>In Eclipse IDE for ModusToolbox&trade; software</b></summary>



1. Click the **New Application** link in the **Quick Panel** (or, use **File** > **New** > **ModusToolbox&trade; Application**). This launches the [Project Creator](https://www.infineon.com/ModusToolboxProjectCreator) tool.

2. Pick a kit supported by the code example from the list shown in the **Project Creator - Choose Board Support Package (BSP)** dialog.

   When you select a supported kit, the example is reconfigured automatically to work with the kit. To work with a different supported kit later, use the [Library Manager](https://www.infineon.com/ModusToolboxLibraryManager) to choose the BSP for the supported kit. You can use the Library Manager to select or update the BSP and firmware libraries used in this application. To access the Library Manager, click the link from the **Quick Panel**.

   You can also just start the application creation process again and select a different kit.

   If you want to use the application for a kit not listed here, you may need to update the source files. If the kit does not have the required resources, the application may not work.

3. In the **Project Creator - Select Application** dialog, choose the example by enabling the checkbox.

4. (Optional) Change the suggested **New Application Name**.

5. The **Application(s) Root Path** defaults to the Eclipse workspace which is usually the desired location for the application. If you want to store the application in a different location, you can change the *Application(s) Root Path* value. Applications that share libraries should be in the same root path.

6. Click **Create** to complete the application creation process.

For more details, see the [Eclipse IDE for ModusToolbox&trade; software user guide](https://www.infineon.com/MTBEclipseIDEUserGuide) (locally available at *{ModusToolbox&trade; software install directory}/docs_{version}/mt_ide_user_guide.pdf*).

</details>

### Operation

The firmware example uses KitProg3 Debug UART for the debug output. The results are read from the sensor and are printed on the terminal window. The distance of every sensor is represented in millimeters (if choosing the 3x3 mode) or in centimeters (if choosing the 8x8 mode).

Example of the console output for the 3x3 mode:

<img src="images/tmf8820_info.png" style="zoom:100%;" />

To select between 3x3 or 8x8 mode, just change the define in the file <b>AMS_TOF882x\Example\Simple\tmf882x_example_simple.c</b> line 40.

    /**
    * If defined, then the 8x8 mode will be used
    * If not defined, the 3x3 mode will be used
    */
    #undef USE_8X8_MODE


The additional debugging information can be enabled or disabled by modifying the code in a "tmf882x_example_simple.c" file. 

```
/**************************************************************************
*
* DISABLE debug logging in the TMF882X core driver
*
*************************************************************************/
if (g_core_drv_logging > 1)
tmf882x_set_debug(&tof, false);
```



The application always waits for the "**USER-BTN1**" to be pressed after the start-up of the system. Please press the button "**USER-BTN1**" on the RDK2 board to start or to stop the measurements of the TMF8820.

<img src="images/user_button.png" style="zoom:25%;" />

### Debugging

If you successfully have imported the example, the debugging configurations are already prepared to be used with a the KitProg3, MiniProg4, or J-link. Open the ModusToolbox perspective and find the Quick Panel. Click on the desired debug launch configuration and wait for the programming to  complete and debugging process to start.

<img src="images/debugging.png" style="zoom:100%;" />

## Legal Disclaimer

The evaluation board including the software is for testing purposes only and, because it has limited functions and limited resilience, is not suitable for permanent use under real conditions. If the evaluation board is nevertheless used under real conditions, this is done at one’s responsibility; any liability of Rutronik is insofar excluded. 

<img src="images/rutronik_origin_kaunas.png" style="zoom:50%;" />



