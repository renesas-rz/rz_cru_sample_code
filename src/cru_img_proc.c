/***********************************************************************************************************************
 * Source of the RZ CRU Sample Application.
 *
 * Copyright (C) 2023 Renesas Electronics Corp. All rights reserved.
 ***********************************************************************************************************************/
/***********************************************************************************************************************
 * File Name    : cru_img_proc.c
 * Version      : 1.0.0
 * Description  : RZ MPU CRU Sample Application Image Processing Functions
 ***********************************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "cru_img_proc.h"

int AWB_ON = 0;
int AE_ON = 0;
int AF_ON = 0;
int STATS_RGB_ON = 0;
int STATS_DIFF_ON = 0;
int LMP_ON = 0;
int CRU_OFF = 0;
int WB_ON = 0;
int WB_TYPE = -1;

float camera_cs_r = 1;
float camera_cs_g = 1;
float camera_cs_b = 1;

int line_length = -1;
unsigned int* screen_base = NULL;

/*General function*/

/*
 * int xioctl(int fh, int request, void* arg)
 * Description:
 A system call that allows an application to control a device driver or to communicate with a device driver outside of the normal flow of reading and 
 writing data.
 * Parameters:
 fh: opened file descriptor (mainly the framebuffer and v4l2 device in this demo)
 request: requests (mainly the framebuffer requests and v4l2 device requests in this demo)
 arg: address corresponding to requests
 * Return Value:
 0  success
 -1 system call failed
 * Remark:
 If a signal is received during ioctl() processing, it ends with an error, but if the cause of the error is signal reception (EINTR), it becomes a wrapper 
 function that automatically retries ioctl().
 */
int xioctl(int fh, int request, void* arg)
{
    int ioctl_result;

    do {
        ioctl_result = ioctl(fh, request, arg);
    } while (ioctl_result == -1 && ((errno == EINTR) || (errno == EAGAIN)));

    return ioctl_result;
}

/*CRU image processing function*/

/* 
 * int rz_cru_lmp_all(int rof, int gof, int bof,
 int rr,  int rg,  int rb,
 int gr,  int gg,  int gb,
 int br,  int bg,  int bb)
 * Description:
 Linear Matrix Processing can change output color data value based on setting for R, G, and B coefficients and offsets. This
 function can allow customers to make color corrections, and support for AWB, AE, AF, etc.
 This function sets parameters for linear matrix operation with the 1st to 12th arguments and executes color correction of 
 the video stream.
 * Parameters:
 rof: R offset value for Linear Matrix calculation
 Setting range: -128 to 127
 gof: G offset value for Linear Matrix calculation
 Setting range: -128 to 127
 bof: B offset value for Linear Matrix calculation
 Setting range: -128 to 127
 rr: Coefficient R of R data for Linear Matrix calculation
 Specify a signed integer obtained by multiplying the desired coefficient value (-4 to about 4) by 1024.
 Setting range: -4096 to 4095
 rg: Coefficient G of R data for Linear Matrix calculation
 Specify a signed integer obtained by multiplying the desired coefficient value (-4 to about 4) by 1024.
 Setting range: -4096 to 4095
 rb: Coefficient B of R data for Linear Matrix calculation
 Specify a signed integer obtained by multiplying the desired coefficient value (-4 to about 4) by 1024.
 Setting range: -4096 to 4095
 gr: Coefficient R of G data for Linear Matrix calculation
 Specify a signed integer obtained by multiplying the desired coefficient value (-4 to about 4) by 1024.
 Setting range: -4096 to 4095
 gg: Coefficient G of G data for Linear Matrix calculation
 Specify a signed integer obtained by multiplying the desired coefficient value (-4 to about 4) by 1024.
 Setting range: -4096 to 4095
 gb: Coefficient B of G data for Linear Matrix calculation
 Specify a signed integer obtained by multiplying the desired coefficient value (-4 to about 4) by 1024.
 Setting range: -4096 to 4095
 br: Coefficient R of B data for Linear Matrix calculation
 Specify a signed integer obtained by multiplying the desired coefficient value (-4 to about 4) by 1024.
 Setting range: -4096 to 4095
 bg: Coefficient G of B data for Linear Matrix calculation
 Specify a signed integer obtained by multiplying the desired coefficient value (-4 to about 4) by 1024.
 Setting range: -4096 to 4095
 bb: Coefficient B of B data for Linear Matrix calculation
 Specify a signed integer obtained by multiplying the desired coefficient value (-4 to about 4) by 1024.
 Setting range: -4096 to 4095
 * Return Value:
 0  success
 -1 setting failed
 * Remark:
 Linear Matrix sets the coefficients in the register for R, G, and B. There are coefficients and offset related 
formula:
R^' = (Rr*(R+Rof)+Rg*(G+Gof)+Rb*(B+Bof))/1024
G^' = (Gr*(G+Gof)+Gg*(G+Gof)+Gb*(B+Bof))/1024
B^' = (Br*(R+Rof)+Bg*(G+Gof)+Bb*(B+Bof))/1024
*/
int rz_cru_lmp_all(int rof, int gof, int bof,
        int rr, int rg, int rb,
        int gr, int gg, int gb,
        int br, int bg, int bb, int fd)
{
    struct v4l2_control ctrl;

    ctrl.id = RZ_MPU_CRU_LINEAR_MATRIX_ROF;
    ctrl.value = rof;
    if (-1 == xioctl(fd, VIDIOC_S_CTRL, &ctrl))
    {
        exit(EXIT_FAILURE);
	return -1;
    };

    ctrl.id = RZ_MPU_CRU_LINEAR_MATRIX_GOF;
    ctrl.value = gof;
    if (-1 == xioctl(fd, VIDIOC_S_CTRL, &ctrl))
    {
        exit(EXIT_FAILURE);
	return -1;
    };

    ctrl.id = RZ_MPU_CRU_LINEAR_MATRIX_BOF;
    ctrl.value = bof;
    if (-1 == xioctl(fd, VIDIOC_S_CTRL, &ctrl))
    {
        exit(EXIT_FAILURE);
	return -1;
    };


    ctrl.id = RZ_MPU_CRU_LINEAR_MATRIX_RR;
    ctrl.value = rr;
    if (-1 == xioctl(fd, VIDIOC_S_CTRL, &ctrl))
    {
        exit(EXIT_FAILURE);
	return -1;
    };

    ctrl.id = RZ_MPU_CRU_LINEAR_MATRIX_RG;
    ctrl.value = rg;
    if (-1 == xioctl(fd, VIDIOC_S_CTRL, &ctrl))
    {
        exit(EXIT_FAILURE);
	return -1;
    };

    ctrl.id = RZ_MPU_CRU_LINEAR_MATRIX_RB;
    ctrl.value = rb;
    if (-1 == xioctl(fd, VIDIOC_S_CTRL, &ctrl))
    {
        exit(EXIT_FAILURE);
	return -1;
    };

    ctrl.id = RZ_MPU_CRU_LINEAR_MATRIX_GR;
    ctrl.value = gr;
    if (-1 == xioctl(fd, VIDIOC_S_CTRL, &ctrl))
    {
        exit(EXIT_FAILURE);
	return -1;
    };

    ctrl.id = RZ_MPU_CRU_LINEAR_MATRIX_GG;
    ctrl.value = gg;
    if (-1 == xioctl(fd, VIDIOC_S_CTRL, &ctrl))
    {
        exit(EXIT_FAILURE);
	return -1;
    };

    ctrl.id = RZ_MPU_CRU_LINEAR_MATRIX_GB;
    ctrl.value = gb;
    if (-1 == xioctl(fd, VIDIOC_S_CTRL, &ctrl))
    {
        exit(EXIT_FAILURE);
	return -1;
    };

    ctrl.id = RZ_MPU_CRU_LINEAR_MATRIX_BR;
    ctrl.value = br;
    if (-1 == xioctl(fd, VIDIOC_S_CTRL, &ctrl))
    {
        exit(EXIT_FAILURE);
	return -1;
    };

    ctrl.id = RZ_MPU_CRU_LINEAR_MATRIX_BG;
    ctrl.value = bg;
    if (-1 == xioctl(fd, VIDIOC_S_CTRL, &ctrl))
    {
        exit(EXIT_FAILURE);
	return -1;
    };

    ctrl.id = RZ_MPU_CRU_LINEAR_MATRIX_BB;
    ctrl.value = bb;
    if (-1 == xioctl(fd, VIDIOC_S_CTRL, &ctrl))
    {
        exit(EXIT_FAILURE);
	return -1;
    };

    return 0;
}

/*CRU image processing function*/
/*
 * int rz_cru_lmp_all(int rr, int gg, int bb,int fd)
 * Description:
 Linear Matrix Processing can change output color data value based on setting for Rr, Gg, and Bb coefficients. This
 function can allow customers to make color corrections, and support for AWB, AE, AF, etc.
 This function sets parameters for linear matrix operation with the 1st to 12th arguments and executes color correction of
 the video stream.
 * Parameters:
 rr: Coefficient R of R data for Linear Matrix calculation
 Specify a signed integer obtained by multiplying the desired coefficient value (-4 to about 4) by 1024.
 Setting range: -4096 to 4095
 gg: Coefficient G of G data for Linear Matrix calculation
 Specify a signed integer obtained by multiplying the desired coefficient value (-4 to about 4) by 1024.
 Setting range: -4096 to 4095
 bb: Coefficient B of B data for Linear Matrix calculation
 Specify a signed integer obtained by multiplying the desired coefficient value (-4 to about 4) by 1024.
 Setting range: -4096 to 4095
 * Return Value:
 0      success
 -1     setting failed
 * Remark:
 Linear Matrix sets the coefficients in the register for R, G, and B. There are coefficients and offset related
formula:
R^' = (Rr*(R+Rof)+Rg*(G+Gof)+Rb*(B+Bof))/1024
G^' = (Gr*(G+Gof)+Gg*(G+Gof)+Gb*(B+Bof))/1024
B^' = (Br*(R+Rof)+Bg*(G+Gof)+Bb*(B+Bof))/1024
*/
int rz_cru_lmp_rgb(int rr, int gg, int bb, int fd)
{
    struct v4l2_control ctrl;

    ctrl.id = RZ_MPU_CRU_LINEAR_MATRIX_RR;
    ctrl.value = rr;
    if (-1 == xioctl(fd, VIDIOC_S_CTRL, &ctrl))
    {
        exit(EXIT_FAILURE);
	return -1;
    };

    ctrl.id = RZ_MPU_CRU_LINEAR_MATRIX_GG;
    ctrl.value = gg;
    if (-1 == xioctl(fd, VIDIOC_S_CTRL, &ctrl))
    {
        exit(EXIT_FAILURE);
	return -1;
    };

    ctrl.id = RZ_MPU_CRU_LINEAR_MATRIX_BB;
    ctrl.value = bb;
    if (-1 == xioctl(fd, VIDIOC_S_CTRL, &ctrl))
    {
        exit(EXIT_FAILURE);
	return -1;
    };

    return 0;
}

/*
 * void rz_cru_AWB(void)
 *Description:
 Customer can select and set the auto white balance (AWB).
 The coefficients of R, G, and B are calculated by the gray-scale world formula mentioned in Remark. 
 (When options other than AWB are activated, we multiply the RGB coefficients calculated by AWB above by 
 the corresponding data coefficients to achieve color temperature settings.) And call the rz_cru_lmp_rgb () 
 function to set the parameters.
Argument:
-
Return Value:
-
Remark:
Cannot be applied at the same time with other rzg_cru_lmp series functions.
The AWB of this Demo adopts the most common grayscale world algorithm, the formula is as follows:
gary = max(R_avg, G_avg, B_avg) <<default>>
R' = camera_cs_r*(gray/R_avg)*R*CRU_HW_PARAMETER
G' = camera_cs_g*(gray/G_avg)*G*CRU_HW_PARAMETER
B' = camera_cs_b*(gray/B_avg)*B*CRU_HW_PARAMETER
or
gary = (R_avg+G_avg+B_avg)/3
R' = camera_cs_r*(gray/R_avg)*R*CRU_HW_PARAMETER
G' = camera_cs_g*(gray/G_avg)*G*CRU_HW_PARAMETER
B' = camera_cs_b*(gray/B_avg)*B*CRU_HW_PARAMETER
(You can refer to the code below and modify it accordingly)
int r = camera_cs_r*(statistics.gray/statistics.rsum)*CRU_HW_PARAMETER;
int g = camera_cs_g*(statistics.gray/statistics.gsum)*CRU_HW_PARAMETER;
int b = camera_cs_b*(statistics.gray/statistics.bsum)*CRU_HW_PARAMETER;
rz_cru_lmp_rgb(r,g,b,fd);
*/
void rz_cru_AWB(int fd)
{
    /*gray-scale world max_rgb_avg ver.*/
    int gain_r = camera_cs_r*(statistics.max_rgb_avg / statistics.rsum)*CRU_HW_PARAMETER;
    int gain_g = camera_cs_g*(statistics.max_rgb_avg / statistics.gsum)*CRU_HW_PARAMETER;
    int gain_b = camera_cs_b*(statistics.max_rgb_avg / statistics.bsum)*CRU_HW_PARAMETER;
    rz_cru_lmp_rgb(gain_r,gain_g,gain_b,fd);
    return;
}

/*
 * void rz_cru_WB(void)
 *Description:
 Users can select and set the color temperature (3 types of Auto, Fluorescent, and tungsten), 
 and the results are displayed on the monitor. 
 The rz_cru_lmp_rgb() function is then called with the color temperature parameters to set the parameters.
 *Argument:
 -
*Return Value:
 -
*Remark:
Cannot be applied at the same time with other rzg_cru_lmp series functions.
The AWB of this Demo adopts the most common grayscale world algorithm, the formula is as follows:

gary = (R_avg+G_avg+B_avg)/3 <default>
R' = camera_cs_r*(gray/R_avg)*R*CRU_HW_PARAMETER
G' = camera_cs_g*(gray/G_avg)*G*CRU_HW_PARAMETER
B' = camera_cs_b*(gray/B_avg)*B*CRU_HW_PARAMETER
or
gary = max(R_avg, G_avg, B_avg)
R' = camera_cs_r*(gray/R_avg)*R*CRU_HW_PARAMETER
G' = camera_cs_g*(gray/G_avg)*G*CRU_HW_PARAMETER
B' = camera_cs_b*(gray/B_avg)*B*CRU_HW_PARAMETER
(You can refer to the code below and modify it accordingly)
int gain_r = ct_r*(statistics.max_rgb_avg / statistics.rsum)*CRU_HW_PARAMETER;
int gain_g = ct_g*(statistics.max_rgb_avg / statistics.gsum)*CRU_HW_PARAMETER;
int gain_b = ct_b*(statistics.max_rgb_avg / statistics.bsum)*CRU_HW_PARAMETER;

*/
void rz_cru_WB(float cs_r, float cs_g, float cs_b,int fd)
{
    switch(WB_TYPE){
        case WB_AWB_ON:
            rz_cru_AWB(fd);
            break;
        case WB_FLUORESCENT_ON:
	    /*Fluorescent reference data, for reference only, please replace the ideal data by yourself*/
            cs_r = 1/1.1875;
            cs_g = 1;
            cs_b = 1/1.3125;
            break;
        case WB_TUNGSTEN_ON:
            /*Tungsten reference data, for reference only, please replace the ideal data by yourself*/
	    cs_r = 1;
            cs_g = 1/1.008;
            cs_b = 1/1.28;
            break;
        default:
            break;
    }
    if ( (WB_TYPE == WB_AWB_ON) | (WB_TYPE == WB_FLUORESCENT_ON) | (WB_TYPE == WB_TUNGSTEN_ON) )
    {
        float ct_r = camera_cs_r*cs_r;
        float ct_g = camera_cs_g*cs_g;
        float ct_b = camera_cs_b*cs_b;
        int r = ct_r*(statistics.gray/statistics.rsum)*CRU_HW_PARAMETER;
        int g = ct_g*(statistics.gray/statistics.gsum)*CRU_HW_PARAMETER;
        int b = ct_b*(statistics.gray/statistics.bsum)*CRU_HW_PARAMETER;
        rz_cru_lmp_rgb(r,g,b,fd);
    }
    return;
}

/*
 * void rz_cru_AE (int fd)
 * Description:
 When the customer turns on this function, if the brightness is lower or higher than a certain value, the gain adjustment 
 will be performed automatically.

 Specific steps are as follows:
 Step 1: Use the RGB average value after 0x4b0000 obtained from the CRU to perform brightness statistics on the current 
 image.
 Step 2: Determine whether the brightness needs to be changed according to the current image brightness (this demo needs 
 to be other than 140±20).
 Step 3: If need to change it, fix brightness to 120 (because the focus of this demo is not the algorithm, so no new 
 exposure parameters, exposure time, aperture, gain are calculated).
 Step 4: Apply the new parameters to the next frame by applying rz_cru_lmp_all.
Parameters:
@ gray: statistics data for AE (the average gray value of the image)
Return Value:
0   success
Remark:
Cannot be applied at the same time with other rzg_cru_lmp series functions.
This function is a SIMULATION. Instead of aperture and shutter speed control, switch to automatic gain adjustment when 
the lighting environment is dim, and it is implemented by the user by changing the camera control source code as needed.
*/
void rz_cru_AE(int fd)
{
    float gray_old = statistics.gray;
    /*120 average brightness and 140±20 threshold judgment standard are for reference only*/
    if((AWB_ON == CRUAPP_FUN_ON && AE_ON == CRUAPP_FUN_ON)|(WB_ON == CRUAPP_FUN_ON && AE_ON == CRUAPP_FUN_ON))
    {
        if( fabsf( statistics.gray - AE_GRAY_THRESHOLD) > AE_GRAY_RANGE)
        {
            statistics.gray = AE_GAIN;
            statistics.max_rgb_avg = AE_GAIN;
        }
    }

    if( AWB_ON == CRUAPP_FUN_OFF && AE_ON == CRUAPP_FUN_ON)
    {
        if( fabsf( statistics.gray - AE_GRAY_THRESHOLD) > AE_GRAY_RANGE)
        {
            rz_cru_lmp_rgb(
                    (AE_GAIN/gray_old * CRU_HW_PARAMETER),
                    (AE_GAIN/gray_old * CRU_HW_PARAMETER),
                    (AE_GAIN/gray_old * CRU_HW_PARAMETER),fd);
        }
    }
    if( AWB_ON == CRUAPP_FUN_OFF && AE_ON == CRUAPP_FUN_OFF && LMP_ON == CRUAPP_FUN_OFF)
    {
	    /*reset*/
	    rz_cru_lmp_rgb(CRU_HW_PARAMETER, CRU_HW_PARAMETER, CRU_HW_PARAMETER,fd);
    }
    return;
}


/*
 * Function: diff_info_printf
 * -------------------------
 *   printf the image evaluation information for AF.
 *
 *   returns: None.
 */
static void diff_info_printf(void)
{   
    static float c = 0;
    if ( fabsf(statistics.diffavg - c) > 0.1)
    {   
        printf("Image Evaluation Gradient:%f\n",statistics.diffavg);
        c = statistics.diffavg;
    }
    return;
}

/*
 * void rz_cru_AF (void)
 *Description:
 When the customer turns on this function, the image clarity is calculated through statistical data and fed back to the 
 customer in real time.
 Specific steps are as follows:
 Step 1: Use the absolute value of the difference of the adjacent pxiel after 0x4b0000 obtained from the CRU to calculate 
 the sharpness (average gradient) of the current image.
 Step 2: Determine whether a warning is required according to the current image sharpness (average gradient) 
 (in this demo, the default value of sharpness is less than 4 to warning)
 Step 3: If a warning is required, print the warning message and make the background of the warning position to black. 
 if not, the background of the warning position is white (because the focus of this demo is not the algorithm, 
 so there is no response parameter to the camera, etc.).

Parameters:
-
Return Value:
-
Remark:
Cannot be applied at the same time with other rzg_cru_lmp series functions.
This function is a SIMULATION. We only provide an example of the calculation principle after the statistical data is 
obtained through the CRU, and the user can modify the code to change the relevant camera and lens parameters to achieve 
the current AF.

Please avoid subjects that are difficult to focus on while testing
1. far,dark, and fast moveing
2. Weak contrast
3. Glass,mirrors and luminous,etc
4. blinking and backlit
5. A continuous repeating pattern, such as the wall
*/
void rz_cru_AF(void)
{
    int i;
    unsigned int* AF_base = NULL;
    diff_info_printf();

    if  (statistics.gray < AF_GRAY_THRESHOLD) /*Improve the problem of warnings in dark environments, for reference only*/
    {
        statistics.diffavg *= DIFF_GAIN;
    }
    /*The gradient threshold is for reference only, please modify according to the situation*/
    if (statistics.diffavg < AF_GRADIENT_THRESHOLD)
    {
        for (i = 0, AF_base = screen_base+IMAGE_WIDTH*IMAGE_CHANNEL +STATS_WIDTH*IMAGE_CHANNEL;i < STATS_HEIGHT; i++)
        {
            memset(AF_base, 0x00, STATS_WIDTH * IMAGE_CHANNEL);
            AF_base += line_length;
        }
        printf("WARNING!!!!We need to adjust the distance between the camera and the subject!\n");
    }
    else
    {
        for (i = 0, AF_base = screen_base+IMAGE_WIDTH*IMAGE_CHANNEL +STATS_WIDTH*IMAGE_CHANNEL;i < STATS_HEIGHT; i++)
        {
            memset(AF_base, 0xff, STATS_WIDTH * IMAGE_CHANNEL);
            AF_base += line_length;  
        }
    }
   return;
}

/*
 *void rz_cru_statistics_rgb (void)
 *Description:
 Show the RGB image of statistics data on the monitor.
Parameters:
-
Return Value:
-
Remark:
-
*/
void rz_cru_statistics_rgb (void)
{
    int i;
    static unsigned int* base = NULL;
    static int* stats_rgb_base = NULL;
    for (i = 0, base = screen_base+IMAGE_WIDTH * IMAGE_CHANNEL +POSITION_STATS_RGB, stats_rgb_base = statistics.rgb;i < STATS_HEIGHT; i++)
    {
        memcpy(base, stats_rgb_base, STATS_WIDTH * IMAGE_CHANNEL);
        base += line_length;  
        stats_rgb_base += STATS_WIDTH;
    }
    return;
}

/*
 *void rz_cru_statistics_diff (void)
 *Description:
 Show the differential image of statistics data on the monitor.
Parameters:
-
Return Value:
-
Remark:
-
*/
void rz_cru_statistics_diff (void)
{
    int i;
    static unsigned int* base = NULL;
    static int* stats_diff_base = NULL;
    for (i = 0, base = screen_base+IMAGE_WIDTH* IMAGE_CHANNEL +POSITION_STATS_DIFF, stats_diff_base = statistics.diff;i < STATS_HEIGHT; i++)
    {
        memcpy(base, stats_diff_base, STATS_WIDTH * IMAGE_CHANNEL);
        base += line_length;
        stats_diff_base += STATS_WIDTH;
    }
    return;
}

/*
 *static int statistics_max (int a int b)
Description:
max (a,b)
Parameters:
a: one of the statistics.rsum,gsum,bsum
b: one of the statistics.rsum,gsum,bsum
Return Value:
the max value
Remark:
-
*/
static int statistics_max(int a, int b) 
{
    return (a > b) ? a : b;
}

/*
 *void rz_cru_get_statistics (void)
Description:
Get statistics and calculate the required data for AWB, AF, AE based on the statistics. At the same time, the statistics are saved in a matrix for displaying differential images and statistics RGB images.
Parameters:
-
Return Value:
-
Remark:
-
*/
void rz_cru_get_statistics (void)
{
    const int NUM_PIXELS = STATS_PIXEL;
    int i;
    const int gain = DIFFGAIN;
    float diff_avg = 0;
    int rsum = 0;
    int gsum = 0;
    int bsum = 0;

    for (i = 0; i < NUM_PIXELS; i++)
    {
        int data = *(int*)(buffers[v4l2buffers.index].start + STATS_ADDRESS + i * IMAGE_CHANNEL);
        int diff_byte = (data >> 24) & 0x000000ff; /*Get diff statistics data from (diff R G B) 32-bit statistics data*/
        int red = (data >> 16) & 0x000000ff; /*Get red statistics data from (diff R G B) 32-bit statistics data*/
        int green = (data >> 8) & 0x000000ff;  /*Get green statistics data from (diff R G B) 32-bit statistics data*/
        int blue = data & 0x000000ff; /*Get blue statistics data from (diff R G B) 32-bit statistics data*/

        /*for statirgb image*/
        statistics.rgb[i] = data & 0x00ffffff;

        /*for AF*/
        diff_avg += data >> 24;

        /*for AWB and AE*/
        rsum += red;
        gsum += green;
        bsum += blue;

        /*for diff image*/
        statistics.diff[i] = (diff_byte*gain | (diff_byte*gain << 8) | (diff_byte*gain << 16)) & 0x00ffffff;
    }
    statistics.rsum = rsum / NUM_PIXELS;
    statistics.gsum = gsum / NUM_PIXELS;
    statistics.bsum = bsum / NUM_PIXELS;
    statistics.gray = (statistics.rsum + statistics.gsum + statistics.bsum) / 3;
    statistics.diffavg = diff_avg / NUM_PIXELS;
    statistics.max_rgb_avg = statistics_max(statistics.rsum, statistics_max(statistics.gsum, statistics.bsum));
    return;
}
