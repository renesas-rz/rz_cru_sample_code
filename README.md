# RZ MPU CRU Sample Application

## Overview

This is a sample application for understanding how to control Camera Receiving Unit (CRU) on RZ MPUs using Linux driver and other modules.

## Target Devices and Supported Environments

### Target Devices

* [RZ/V2L](https://www.renesas.com/us/en/products/microcontrollers-microprocessors/rz-mpus/rzv2l-general-purpose-microprocessor-equipped-renesas-original-ai-accelerator-drp-ai-12ghz-dual).
* [RZ/G2L](https://www.renesas.com/eu/en/products/microcontrollers-microprocessors/rz-mpus/rzg2l-general-purpose-microprocessors-dual-core-arm-cortex-a55-12-ghz-cpus-and-single-core-arm-cortex-m33),
[RZ/G2LC](https://www.renesas.com/eu/en/products/microcontrollers-microprocessors/rz-mpus/rzg2lc-general-purpose-mcus-dual-core-arm-cortex-a55-12-ghz-cpus-and-single-core-arm-cortex-m33-200-mhz-cpu),
[RZ/G2UL](https://www.renesas.com/eu/en/products/microcontrollers-microprocessors/rz-mpus/rzg2ul-general-purpose-microprocessors-single-core-arm-cortex-a55-10-ghz-cpu-and-single-core-arm-cortex-m33).

### Supported Environments

* [Board Support Package (BSP) v3.0.6-update3](https://github.com/renesas-rz/meta-renesas/tree/BSP-3.0.6-update3).
* [Verified Linux Package (VLP)/G v3.0.6-update3](https://www.renesas.com/us/en/products/microcontrollers-microprocessors/rz-mpus/rzg-linux-platform/rzg-marketplace/verified-linux-package/rzg-verified-linux-package#overview).


## Features

This sample application highlights the following features on CRU.

* Demosaicing
* Linear Matrix Processing
* Statistics

## License and notice

The CRU Sample Application source file **"_src_"** is licensed under the MIT License. It is provided **"AS IS"** with no warranty. See the LICENSE file for more details.

The Coral Camera driver patch file in the **"_patch_"** folder is provided **"AS IS"** with no warranty and follows the same license as the driver.

Renesas does not provide any support for this sample application. Questions about this sample will not be answered and no corrections will be made.

The code is not intended for production purposes. It may still contain bugs, lack critical error handling, etc. Please check the code yourself and use it at your own risk.

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

To build the Board Support Package with the Coral Camera driver patch, follow the instructions below. First, refer to the [Yocto Build layer (v3.0.6-update3)](https://github.com/renesas-rz/meta-renesas/tree/BSP-3.0.6-update3) or the RZ/G Verified Linux Package (VLP) Linux Start-up Guide, Section 2.2 (1)-(6) and Chapter 6 to build the image and SDK. Secondly, build the linux package and SDK, and install SDK to your environment.<br>
The current implementation utilizes the CMOS sensor from the Coral camera bundled with the RZ/V2L evaluation board kit. Should you choose a different CMOS sensor, please adjust the source code accordingly to suit your production needs.

1. Patch the Coral Camera driver in the kernel source inside Yocto:

   ```bash
   $ cd ~/rzg_vlp_v3.0.6/build/tmp/work-shared/<board>/kernel-source
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

The application can be executed with the following commands using a terminal that is connected to the target board.

  ```bash
  $ cd /home/root
  $ ./cru_sample
  ```

## Notes

### Device and Sensor Compatibility for Sample Application

Please note that this sample application has been developed and tested on the RZ/G2{L, LC, UL} and RZ/V2L devices. If you plan to use it on other MPU, you may need to make modifications based on the specifications of your target MPU. It is highly recommended to thoroughly evaluate the application after making any modifications.

The current implementation of the sample application utilizes the CMOS sensor from the Coral camera that comes bundled with the RZ/V2L evaluation board kit. However, please be aware that the CMOS camera sensor model (OV5645) mentioned is no longer available. Therefore, it should not be used for mass production purposes.

Please ensure that you select an alternative CMOS sensor that suits your production needs. In such cases, you will need to adjust the source code accordingly to accommodate the specifications of your chosen sensor.
