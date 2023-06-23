# RZ MPU CRU Sample Application

## Overview

The sample application provides instructions for developing a simple camera function with image processing using the CRU driver in the RZ/G2L and RZ/V2L groups.
Specifically, it explains how to use the functions of the CRU. The sample application includes standard image processing algorithms that can be replaced with user-defined algorithms.

Specifically, this sample application includes a sample program that performs demosaicing, color correction, Auto White Balance (AWB), Auto Exposure (AE) , and Auto Focus (AF) based on raw data captured from the camera.
The result can be shown on the monitor through the framebuffer.
These image processing use the CRU data flowing in the pipeline, not as a single image, but everything that can be done (demosaicing, calculation of data required for image processing, color correction, etc.) CPU load can be reduced by pipeline processing.
___

## Target Devices and Supported Environments

### Target Devices

* [RZ/V2L](https://www.renesas.com/us/en/products/microcontrollers-microprocessors/rz-mpus/rzv2l-general-purpose-microprocessor-equipped-renesas-original-ai-accelerator-drp-ai-12ghz-dual).
* [RZ/G2L](https://www.renesas.com/eu/en/products/microcontrollers-microprocessors/rz-mpus/rzg2l-general-purpose-microprocessors-dual-core-arm-cortex-a55-12-ghz-cpus-and-single-core-arm-cortex-m33),
[RZ/G2LC](https://www.renesas.com/eu/en/products/microcontrollers-microprocessors/rz-mpus/rzg2lc-general-purpose-mcus-dual-core-arm-cortex-a55-12-ghz-cpus-and-single-core-arm-cortex-m33-200-mhz-cpu),
[RZ/G2UL](https://www.renesas.com/eu/en/products/microcontrollers-microprocessors/rz-mpus/rzg2ul-general-purpose-microprocessors-single-core-arm-cortex-a55-10-ghz-cpu-and-single-core-arm-cortex-m33).

### Supported Environments

* [Board Support Package (BSP) v3.0.3](https://github.com/renesas-rz/meta-renesas/tree/BSP-3.0.3).
* [Verified Linux Package (VLP)/G v3.0.3](https://www.renesas.com/us/en/products/microcontrollers-microprocessors/rz-mpus/rzg-linux-platform/rzg-marketplace/verified-linux-package/rzg-verified-linux-package#overview).

___

## Features

This section describes a summary of this sample application's features.

* <strong>Demosaicing, Linear Matrix Processing, and Statistics:</strong> 

  Explore the functions of CRU to implement camera capabilities. Learn how to leverage demosaicing, linear matrix processing, and statistical analysis to image quality and extract valuable insights from camera data. <br>
  To delve deeper into the features and implementation of CRU, please refer to the [CRU Function and Sample Application Function](#cru-function-and-sample-application-function) for details.

* <strong>Standard Image Processing Algorithms and V4L2 Architecture:</strong> 

  Master the essentials of using the CRU driver and harness the Video for Linux version 2 (V4L2) to realize camera functionality with a standard image processing algorithm. This example serves as a practical guide, equipping you with the necessary skills to navigate the complexities of camera integration using the CRU driver.

* <strong>Application Modes:</strong> 

  Experience two modes to interact with the camera functionality. The keyboard input mode enables intuitive control within the application, while the debug mode integrates seamlessly with Linux commands, providing a versatile environment for testing and development.

By showcasing the capabilities of CRU and offering distinct application modes along with practical guidance on V4L2 API integration.

___

## CRU Function and Sample Application Function

This section introduces the functions of CRU Sample Application software (SW) and the functions of CRU hardware (HW).

<table>
   <tr>
        <th style="white-space:nowrap"><strong>Sample Application Function Name</strong></td>
        <th style="white-space:nowrap"><strong>Releated CRU Function</strong></td>
        <th><strong>Description</strong></td>
  </tr>
  <tr>
        <td>Demosaicing</td>
        <td>Demosaicing</td>
        <td>This function is only valid on input data of the RAW type. Its operation is on the assumption that the RAW data are Bayer data and generates pixel data that are interpolated from the RAW data. The demosaicing function uses a bilinear method. Of the peripheral 3 × 3 pixels at the position to be calculated, 2 or 4 pixels are used for interpolation processing. For details, please refer to the Hardware Manual.</td>
  </tr>
  <tr>
    <td>Color Correction</td>
    <td style="white-space:nowrap" rowspan="2">Linear Matrix Processing</td>
    <td>Linear matrix processing involves a calculation of the matrix of R', G', and B' by using the registers to set the coefficients for R, G, and B after demosaicing. Color correction can be performed with parameters specified by the user. The result set by the user will be reflected on the monitor in real time.</td>
  </tr>
  <tr>
    <td>Debug Mode</td>
    <td>Display the Spectrum (red, orange, yellow, green, blue, indigo, purple) conversion in real time on the monitor.</td>
  </tr>
  <tr>
    <td>Display of In-block Accumulation</td>
    <td rowspan="2">Statistics  <br> (In-block Accumulation and <br>In-block Sum of Absolute Difference Values)</td>
    <td>Display the cumulative addition data of R, G, B in the statistical data brought by the CRU pipeline processing in the form of RGB image on the monitor.</td>
  </tr>
  <tr>
    <td>Display of In-block Sum of Absolute Difference Values</td>
    <td>Display the absolute difference value sum of adjacent pixels in the statistics brought by the CRU pipeline processing as a differential image (grayscale image) on the monitor.</td>
  </tr>
  <tr>
    <td>Auto White Balance (AWB)</td>
    <td rowspan="3"style="white-space:nowrap">Linear Matrix Processing and Statistics</td>
    <td>AWB refers to making the camera have a chromatic adaptation similar to human vision, which can perceive the original color of the object even if the illumination light changes.
The sample application applies the grey world algorithm. Here, the result of the statistical calculation function of the CRU can be used as a parameter to feed back to the linear matrix calculation function of the CRU and perform color correction, thereby realizing AWB. <br>User can select and set the auto white balance (AWB); the set results are displayed on the monitor. <br>User can also select  White Balance (WB) model and set the color temperature (3 types of Auto, Fluorescent, and Tungsten), and the results are displayed on the monitor.</td>
  </tr>
  <tr>
    <td>Auto Exposure (AE) (pseudo)</td>
    <td>AE refers to controlling the lens iris, image sensor gains, and shutter speed based on brightness information to keep the image brightness constant.
The sample application applies a simple luminance detection implementation. The AE function is realized by flexibly applying the average RGB value data in the statistical calculation of the CRU, and the linear matrix processing of the CRU.
In this sample application, this function is a pseudo function. Instead of aperture and shutter speed control, switch to automatic gain adjustment when the lighting environment is dim, and it is implemented by the user by changing the camera control source code as needed. <br> When the user turns on this function, if the brightness is lower than a certain value, the gain adjustment will be made automatically.</td>
  </tr>
  <tr>
    <td>Auto Focus (AF) (pseudo)</td>
    <td>AF refers to a function in which the camera detects changes in the scene and focal length, controls the lens, etc., and focuses automatically.
The sample application applies a detection sharpness implementation. The AF function is to realize AF through the linear matrix processing of the adjacent pixel difference absolute value sum data obtained in the statistical calculation of the CRU.
In this sample application, this function is a pseudo function. We only provide an example of the calculation principle after the statistical data is obtained through the CRU, and the user can modify the code to change the relevant camera and lens parameters to achieve the current AF. <br> When the user turns on this function, the image clarity is calculated through statistical data and fed back to the user in real time.
return a warning when the sharpness result falls below the threshold.</td>
  </tr>
</table>

___

## Build Instructions

### Download the Sample Application Source Code

Please git clone the sample application source code to your local Linux PC or manually download it. <br>
The file list is as follows:

```bash
rz_cru_sample_code
├── patch
│   └── 0001-media-i2c-ov5645-support-SBGGR8-format-instead-of-UY.patch # Coral Camera Driver Patch
└── src
    ├── cru_img_proc.c # Image Processing Library
    ├── cru_img_proc.h #Image Processing Library Headfile
    ├── LICENSE # src File LICENSE Information
    ├── main.c # Sample Application
    └── Makefile # Makefile
```

### Building the Image with Coral Camera Driver Patch

To build the Board Support Package with the Coral Camera driver patch, follow the instructions below. First, refer to the [Yocto Build layer (v3.0.3)](https://github.com/renesas-rz/meta-renesas/tree/BSP-3.0.3) and the RZ/G Verified Linux Package (VLP) Release Note, Section 3.1 (1)-(6) to build the image. <br>
The current implementation utilizes the CMOS sensor from the Coral camera bundled with the RZ/V2L evaluation board kit. Should you choose a different CMOS sensor, please adjust the source code accordingly to suit your production needs.

1. Patch the Coral Camera driver in the kernel source inside Yocto:

   ```bash
   $ cd ~/rzg_vlp_v3.0.3/build/tmp/work-shared/<board>/kernel-source
   $ patch -p1 < /path/to/patch/0001-media-i2c-ov5645-support-SBGGR8-format-instead-of-UY.patch
   ```

2. Rebuild the kernel (Yocto recipe linux-renesas):

    ```bash
    $ MACHINE=<board> bitbake linux-renesas -c compile -f
    ```

3. Rebuild the total image:

    ```bash
    $ MACHINE=<board> bitbake core-image-weston
    ```

Make sure to replace `<board>` with the appropriate board name:

- RZ/G2L Evaluation Board Kit PMIC version: `smarc-rzg2l`
- RZ/G2LC Evaluation Board Kit: `smarc-rzg2lc`
- RZ/G2UL Evaluation Board Kit: `smarc-rzg2ul`
- RZ/V2L Evaluation Board Kit: `smarc-rzv2l`

### Compiling the CRU Sample Application

Follow the steps below to compile the CRU Sample Application after building and installing the Yocto SDK. Note that the SDK build process for the CRU Sample Application is similar to VLP.

1. After building and installing the SDK, source the environment setup script provided by the SDK:

    ```bash
    $ source /path/to/sdk/environment-setup-aarch64-poky-linux
    ```

2. Go to directory _src_ directory and run _make_ command:

    ```bash
    $ cd src
    $ make
    ```

3. Copy the output application file _cru_sample_ inside the src directory to /home/root directory on the target board. If you have an Ethernet connection to your board, you can use scp. Otherwise just manually copy it.

    ```bash
    $ scp cru_sample <username>@<board IP>:/home/root/
    ```

___

## How to use the Application Mode

This section describes how to execute this sample application through the keyboard.

To run the CRU Sample Application, follow the steps below:

1. Open a terminal software on your Development PC that is connected to the serial console of your board.
2. Run the sample application using the commands below:

    ```bash
    $ cd /home/root
    $ ./cru_sample
    ```

3. Please use the keyboard connected to the serial terminal and press the following keys to enable or disable the CRU Sample Application functions: <br>

   **W**: Auto White Balance (AWB) Enable/Disable <br>
   **E**: Auto Exposure (AE) Enable/Disable <br>
   **F**: Auto Focus (AF) Enable/Disable <br>
   **I**: White Balance (WB) Enable/Disable (press number keys after WB ON) <br>
   **R**: Show Statistics RGB data <br>
   **D**: Show Statistics differential image <br>
   **ESC**: Stop the CRU Sample Application <br>

Instructions on how to enable/disable features using keys will be displayed on the terminal. Please refer to your terminal software for the instructions.

___

## How to use the Debug Mode

This section describes how to execute this sample application through Linux commands.

The CRU sample application provides the following functional options:

- **-r/-g/-b**: Linear matrix processing parameters setting of offsets (rof, gof, and bof)
- **--rr/--rg/--rb/--gr/--gg/--gb/--br/--bg/--bb**: Linear matrix processing parameters setting of coefficients
- **--csr/--csg/--csb**: color separation for change AWB results
- **--awb**: AWB on
- **--ae**: AE on
- **--af**: AF on
- **--wb**: `--wb=n`, n: 1.Auto, 2.Fluorescent, and 3.Tungsten
- **--statsrgb**: Display of In-block Accumulation, show stats rgb images (statsrgb)
- **--statsdiff**: Display of In-block Sum of Absolute Difference Values, show stats differential images (statsdiff)
- **--nondisplay**: Do not use CPU for display activities, only use CRU function,
  it is convenient for customers to measure the CPU usage rate of 0 when only using CRU, you can try to use `./cru_sample & top`
- **--crop_x**: The starting x-coordinate of the cropped image (default value 240 for cropping 800x600 from the center)
- **--crop_y**: The starting y-coordinate of the cropped image (default value 180 for cropping 800x600 from the center)
- **--target_width**: The Display image width (default value 800)
- **--target_height**: The Display image height (default value 600)

Please note that the specified color correction function cannot be used with the 3A algorithm at the same time. Other functions such as adjusting display size, color separation, etc. can be used together with Application Mode. For example, you can try `./cru_sample --crop_x=240 --crop_y=180 --target_width=640 --target_height=480` to adjust the display size, and then press the **W** key to turn on the AWB function.

### Color Correction (-r/-g/-b/--rr/--rg/--rb/--gr/--gg/--gb/--br/--bg/--bb)

Linear Matrix Processing can change output color data value based on setting for R, G, and B coefficients and offsets.
Linear Matrix sets the coefficients and offsets in the register for R, G, and B.

* Red:

```bash
  $ ./cru_sample -r=127 -g=0 -b=0 --rr=4095 --rg=0 --rb=0 --gr=0 --gg=0 --gb=0 --br=0 --bg=0 --bb=0
```

* Green:

```bash
  $ ./cru_sample -r=0 -g=127 -b=0 --rr=0 --rg=0 --rb=0 --gr=0 --gg=4095 --gb=0 --br=0 --bg=0 --bb=0
```

* Blue:

```bash
 $ ./cru_sample -r=0 -g=0 -b=127 --rr=0 --rg=0 --rb=0 --gr=0 --gg=0 --gb=0 --br=0 --bg=0 --bb=4095
```

### 3A (AWB, AE, AF) and WB (--awb/--ae/--af/--wb/--csr/--csg/--csb)

The 3A Mode includes demosaicing, linear matrix calculations and statistics. Apply feedback the parameters to linear matrix calculation through algorithm by use the statistical calculation data. The RAW data is converted into an RGB image by demosaicing, and the result of the linear matrix calculation is fed back to the image. Shows the image on the monitor.

* Run the CRU Sample Application without any image processing functions:

```bash
  $ ./cru_sample
```

* Run the CRU Sample Application with AWB is shown below:

```bash
  $ ./cru_sample --awb --frame=50
```

or run the sample application with color separation for change AWB results in your environment:

```bash
  $ ./cru_sample  --awb --csr=1.2 --csg=1.01 --csb=1.17 --frame=50
```

* Run the CRU Sample Application with AE is shown below:

```bash
  $ ./cru_sample --ae --frame=50
```

* Run the CRU Sample Application with AF is shown below:

```bash
  $ ./cru_sample --af --frame=50
```

* Run the CRU Sample Application  with WB (1.Auto, 2.Fluorescent, and 3.Tungsten) is shown below:

```bash
  $ ./cru_sample --wb=1 --frame=50
```

### Display of Statistics Data (--statsrgb/--statsdiff)

The CRU Sample Application provides the visualization of the following two statistical data.

* Display of In-block Accumulation: <br>
Display the cumulative addition data of R, G, B in the statistical data brought by the CRU pipeline processing in the form of RGB image on the monitor. <br>
Run the CRU Sample Application with statistical data RGB image is shown below:

```bash
  $ ./cru_sample --statsrgb
```

* Display of In-block Sum of Absolute Difference Values <br>
Display the absolute value sum of adjacent pixels in the statistics brought by the CRU pipeline processing as a differential image (grayscale image) on the monitor. <br>
Run the CRU Sample Application with differential image is shown below:

```bash
  $ ./cru_sample --statsdiff
```

### CPU usage when only using CRU (--nondisplay)

The CRU (Camera Data Receiving Unit) receives data transmitted from the camera.
The CRU consists of a MIPI CSI-2 block and an Image Processing block.
The Image Processing block has three main functions: Demosaicing, Statistics, and Linear Matrix Processing.
When only using the CRU to process data from the camera, the CPU usage can reach 0.

Run the command to turn off the display (CPU copy) as below:

```bash
   $ ./cru_sample --nondisplay
```

Do not use the CPU for display activities when trying to measure the CPU usage when the CRU function is used. You can use the top command to show a CPU usage rate of 0 when using the CRU.

### How to crop the display size (--crop_x/--crop_y/--target_width/--target_height)

The CRU Sample Application provides commands to adjust the size of the displayed video, including the following four commands:

- **--crop_x**: The starting x-coordinate of the cropped image (default value 240 for cropping 800x600 from the center).
- **--crop_y**: The starting y-coordinate of the cropped image (default value 180 for cropping 800x600 from the center).
- **--target_width**: The Display image width (default value 800).
- **--target_height**: The Display image height (default value 600).

Example of how to change the display size is shown below:

```bash
  $ ./cru_sample --crop_x=240 --crop_y=180 --target_width=800 --target_height=600
```

___

## License

The CRU Sample Application source file **"_src_"** is licensed under the MIT License. It is provided **"AS IS"** with no warranty. See the LICENSE file for more details. <br>
Please check the contents of the license and consider the applicability to the product carefully. <br>
The Coral Camera driver patch file in the **"_patch_"** folder is provided **"AS IS"** with no warranty and follows the same license as the driver. <br>
Please check the contents of the license and consider the applicability to the product carefully. <br>

## Notes

### Device and Sensor Compatibility for Sample Application

Please note that this sample application has been developed and tested on the RZ/G2{L, LC, UL} and RZ/V2L devices. If you plan to use it on other MPU, you may need to make modifications based on the specifications of your target MPU. It is highly recommended to thoroughly evaluate the application after making any modifications.

The current implementation of the sample application utilizes the CMOS sensor from the Coral camera that comes bundled with the RZ/V2L evaluation board kit. However, please be aware that the CMOS camera sensor model (OV5645) mentioned is no longer available. Therefore, it should not be used for mass production purposes.

Please ensure that you select an alternative CMOS sensor that suits your production needs if you intend to use this sample application. In such cases, you will need to adjust the source code accordingly to accommodate the specifications of your chosen sensor.

Lastly, please note that any software support provided with this sample application is solely for evaluation purposes and may not be suitable for production use.
