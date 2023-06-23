/***********************************************************************************************************************
 * Source of the RZ CRU Sample Application.
 *
 * Copyright (C) 2023 Renesas Electronics Corp. All rights reserved.
 ***********************************************************************************************************************/
/***********************************************************************************************************************
 * File Name    : main.c
 * Version      : 1.0.0
 * Description  : RZ MPU CRU Sample Application Main Sequence
 ***********************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <unistd.h>
#include <math.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include "cru_img_proc.h"

#define KEY_DEV    "/dev/input/event1"
#define V4L2_BUF_NUM              (4)

#define OPTIONS_R  (0)
#define OPTIONS_G  (1)
#define OPTIONS_B  (2)
#define OPTIONS_RR (3)
#define OPTIONS_RG (4)
#define OPTIONS_RB (5)
#define OPTIONS_GR (6)
#define OPTIONS_GG (7)
#define OPTIONS_GB (8)
#define OPTIONS_BR (9)
#define OPTIONS_BG (10)
#define OPTIONS_BB (11)
#define OPTIONS_AWB (12)
#define OPTIONS_AE (13)
#define OPTIONS_AF (14)
#define OPTIONS_STATSRGB (15)
#define OPTIONS_STATSDIFF (16)
#define OPTIONS_FRAME (17)
#define OPTIONS_CSR (18)
#define OPTIONS_CSG (19)
#define OPTIONS_CSB (20)
#define OPTIONS_WB (21)
#define OPTIONS_NONDISPLAY (22)

#define OPTIONS_CROP_X (23)
#define OPTIONS_CROP_Y (24)
#define OPTIONS_TARGET_WIDTH (25)
#define OPTIONS_TARGET_HEIGHT (26)

#define CROP_X (240)
#define CROP_Y (180)
#define TARGET_WIDTH (800)
#define TARGET_HEIGHT (600)

/*Global Variable*/

static int              fd = -1, fb_fd = -1, key_fd = -1;
static unsigned int DISPLAY_ON = CRUAPP_FUN_ON;
volatile sig_atomic_t ctrl_c_flag = CRUAPP_FUN_OFF;

static unsigned int crop_x = CROP_X;
static unsigned int crop_y = CROP_Y;
static unsigned int target_width = TARGET_WIDTH;
static unsigned int target_height = TARGET_HEIGHT;

pthread_t tid;
static unsigned int n_buffers = -1;
static unsigned int frame_count = 0;
int is_set[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int linear_matrix_val[12] = {0, 0, 0, CRU_HW_PARAMETER, 0, 0, 0, CRU_HW_PARAMETER, 0, 0, 0, CRU_HW_PARAMETER};
int is_set_func[5] = {0,0,0,0,0};
int func_val[5] = {0,0,0,0,0};
char *name[12]={"r", "g", "b", "rr", "rg", "rb", "gr", "gg", "gb", "br", "bg", "bb"};
char *func_name[5]={"AWB","AE","AF","STATS RGB","STATS DIFF"};
static struct option cru_long_options[] =
{
    /* Save 3 1st places for r, g, b indexes */
    {"rr", required_argument, NULL, OPTIONS_RR},
    {"rg", required_argument, NULL, OPTIONS_RG},
    {"rb", required_argument, NULL, OPTIONS_RB},
    {"gr", required_argument, NULL, OPTIONS_GR},
    {"gg", required_argument, NULL, OPTIONS_GG},
    {"gb", required_argument, NULL, OPTIONS_GB},
    {"br", required_argument, NULL, OPTIONS_BR},
    {"bg", required_argument, NULL, OPTIONS_BG},
    {"bb", required_argument, NULL, OPTIONS_BB},
    {"awb", no_argument, NULL, OPTIONS_AWB},
    {"ae", no_argument, NULL, OPTIONS_AE},
    {"af", no_argument, NULL, OPTIONS_AF},
    {"statsrgb", no_argument, NULL, OPTIONS_STATSRGB},
    {"statsdiff", no_argument, NULL, OPTIONS_STATSDIFF},
    {"frame", required_argument,NULL, OPTIONS_FRAME},
    {"csr", required_argument,NULL, OPTIONS_CSR},
    {"csg", required_argument,NULL, OPTIONS_CSG},
    {"csb", required_argument,NULL, OPTIONS_CSB},
    {"wb",required_argument,NULL, OPTIONS_WB},
    {"nondisplay",no_argument,NULL, OPTIONS_NONDISPLAY},
    {"crop_x",required_argument,NULL, OPTIONS_CROP_X},
    {"crop_y",required_argument,NULL, OPTIONS_CROP_Y},
    {"target_width",required_argument,NULL, OPTIONS_TARGET_WIDTH},
    {"target_height",required_argument,NULL, OPTIONS_TARGET_HEIGHT},
};


/*
 * Function: errno_exit
 * -------------------------
 *   Prints an error message to stderr, indicating the error number and error string 
 *   associated with the errno global variable, and then terminates the program.
 *   Deletes all buffers and closes all open files before exiting the program.
 *   s: The error message to be printed to stderr.
 *
 *   returns: None.
 */
static void errno_exit(const char* s)
{
    fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
    exit(EXIT_FAILURE);
    return;
}

/*
 * Function: rz_cru_capability
 * -------------------------
 *   Queries the capabilities of the video device and outputs the results to the console.
 *
 *   returns: None.
 */

static void rz_cru_capability (void)
{
    struct   v4l2_capability   cap;
    if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) 
    {
        errno_exit("Error opening device");
    }
    else
    {
        printf("===========================CRU DEMO APPLICATION===========================\n");
        printf("||CRU Device Driver Infromations||\n");
        printf("driver:\t\t%s\n",cap.driver);
        printf("card:\t\t%s\n",cap.card);
        printf("bus_info:\t%s\n",cap.bus_info);
        printf("version:\t%d\n",cap.version);
        printf("capabilities:\t%x\n",cap.capabilities);

        if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE) 
        {
            printf("Device %s: supports capture.\n",VIDEO_DEV);
        }

        if ((cap.capabilities & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING) 
        {
            printf("Device %s: supports streaming.\n",VIDEO_DEV);
        }
    } 
    return;
}


/*device init*/

/*
 * Function: fb_dev_init
 * -------------------------
 *   Initializes the framebuffer device specified by the constant `FB_DEV`, by
 *   opening the device, querying information about it, and mapping it to memory.
 *   The size of the screen and line length information is stored as global
 *   variables.
 *
 *   returns: None.
 */
static void fb_dev_init(void)
{

    struct fb_var_screeninfo fb_var = { 0 };
    struct fb_fix_screeninfo fb_fix = { 0 };
    unsigned long screen_size;

    /* open framebuffer dev */
    fb_fd = open(FB_DEV, O_RDWR);
    if (fb_fd < 0) 
    {
        errno_exit("open framebuffer");
    }

    /* get framebuffer info */
    xioctl(fb_fd, FBIOGET_VSCREENINFO, &fb_var);
    xioctl(fb_fd, FBIOGET_FSCREENINFO, &fb_fix);

    screen_size = fb_fix.line_length * fb_var.yres;
    line_length = fb_fix.line_length / (fb_var.bits_per_pixel / 8);

    /* map */
    screen_base = mmap(NULL, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if (MAP_FAILED == (void*)screen_base) 
    {
        close(fb_fd);
        errno_exit("mmap framebuffer");
    }

    /* LCD white background */
    memset(screen_base, 0xFF, screen_size);
    
    return;
}

/*
 * Function: v_dev_init
 * -------------------------
 *   Initializes the video device specified by the constant `VIDEO_DEV`, by
 *   opening the device and setting its format.
 *
 *   returns: None.
 */

static void v_dev_init(void)
{
    struct v4l2_format fmt;

    fd = open(VIDEO_DEV, O_RDWR /* required */ | O_NONBLOCK, 0);
    if (-1 == fd)
    {
        errno_exit("Cannot open video device");
    }

    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    xioctl(fd, VIDIOC_G_FMT, &fmt);

    fmt.fmt.pix.width = IMAGE_WIDTH;
    fmt.fmt.pix.height = IMAGE_HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_ABGR32;

    xioctl(fd, VIDIOC_S_FMT, &fmt);

    return;
}

/*request buffers*/

/*
 *Function: rz_cru_requestbuffers
 * -------------------------
 *  Requests a certain number of memory-mapped buffers from the
 *  device specified by the global file descriptor fd. The number of
 *  requested buffers and their properties are specified by the req structure. 
 *  The memory-mapped buffers are then stored in the global buffers array and 
 *  their lengths and starting addresses are stored in the corresponding array elements.
 *  
 *  returns: None.
 */
static void rz_cru_requestbuffers(void)
{
    struct v4l2_requestbuffers req;

    CLEAR(req);
    req.count = V4L2_BUF_NUM;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    xioctl(fd, VIDIOC_REQBUFS, &req);

    if (req.count == 0)
    {
        errno_exit("Failed to allocate buffers due to out of memory");
    }

    buffers = calloc(req.count, sizeof(*buffers));
    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
        CLEAR(v4l2buffers);

        v4l2buffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        v4l2buffers.memory = V4L2_MEMORY_MMAP;
        v4l2buffers.index = n_buffers;

        xioctl(fd, VIDIOC_QUERYBUF, &v4l2buffers);

        buffers[n_buffers].length = v4l2buffers.length;
        buffers[n_buffers].start = mmap(NULL, v4l2buffers.length,
                PROT_READ | PROT_WRITE, MAP_SHARED,
                fd, v4l2buffers.m.offset);

        if (MAP_FAILED == buffers[n_buffers].start) 
        {
            errno_exit("request buffers");
        }
    }

    for (unsigned int i = 0; i < n_buffers; ++i) {
        CLEAR(v4l2buffers);
        v4l2buffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        v4l2buffers.memory = V4L2_MEMORY_MMAP;
        v4l2buffers.index = i;
        xioctl(fd, VIDIOC_QBUF, &v4l2buffers);
    }
    
    return;
}

/*
 *Function: rz_cru_start_streaming
 * -------------------------
 *  start streaming.
 *  
 *  returns: None.
 */
static void rz_cru_start_streaming(void)
{
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
    {
        errno_exit("VIDIOC_STREAMON");
    }

    return;
}

/*
 *Function: cru_to_display
 * -------------------------
 *  copy the cru results to the frambuffer, and display the image.
 *  
 *  returns: None.
 */
static void cru_to_display(unsigned int crop_x, unsigned int crop_y, unsigned int target_width, unsigned int target_height)
{

    unsigned int i = 0;
    unsigned int* base = NULL, *start = NULL;

    for (i = 0, base = screen_base, start = buffers[v4l2buffers.index].start + crop_y * IMAGE_WIDTH + crop_x;
            i < target_height; i++)
    {
        memcpy(base, start, target_width *4);
        base += line_length;
        start += IMAGE_WIDTH;
    }

    return;
}

/*
 *Function: rz_cru_img_processing
 * -------------------------
 *  This function processes the captured image.
 *  It performs various functions like auto exposure (AE), auto focus (AF),
 *  auto white balance (AWB), linear matrix processing (LMP), and also
 *  calculates statistical data (RGB and differential image).
 *
 *  returns: None.
 */

static void rz_cru_img_processing(void)
{
    fd_set                          fds;
    struct timeval                  tv;    
    unsigned int    r;    

    do 
    {
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        r = select(fd + 1, &fds, NULL, NULL, &tv);
    } while (((unsigned int)-1 == r && (errno == EINTR)));
    if ((unsigned int) -1 == r) 
    {
        errno_exit("select");
    }

    CLEAR(v4l2buffers);    
    v4l2buffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2buffers.memory = V4L2_MEMORY_MMAP;
    xioctl(fd, VIDIOC_DQBUF, &v4l2buffers);    
    /*display the cru results*/
    if (DISPLAY_ON == CRUAPP_FUN_ON)
    {
        cru_to_display(crop_x,crop_y,target_width,target_height);
    }
    /*get statistics data when functions on*/
    if ( (CRUAPP_FUN_ON == AWB_ON) | (CRUAPP_FUN_ON == WB_ON) |(CRUAPP_FUN_ON == AE_ON) | (CRUAPP_FUN_ON == AF_ON) | (CRUAPP_FUN_ON == STATS_RGB_ON) | (CRUAPP_FUN_ON == STATS_DIFF_ON))
    {
        rz_cru_get_statistics();
    }
    /*linear matrix processing (can not work with 3A fuction)*/
    if (CRUAPP_FUN_ON == LMP_ON)
    {
        AE_ON = CRUAPP_FUN_OFF;
        AF_ON = CRUAPP_FUN_OFF;
        AWB_ON = CRUAPP_FUN_OFF;
        rz_cru_lmp_all (linear_matrix_val[0],linear_matrix_val[1],linear_matrix_val[2],
                linear_matrix_val[3],linear_matrix_val[4],linear_matrix_val[5],
                linear_matrix_val[6],linear_matrix_val[7],linear_matrix_val[8],
                linear_matrix_val[9],linear_matrix_val[10],linear_matrix_val[11],fd);
    }
    /*show statistics data rgb image*/
    if (CRUAPP_FUN_ON == STATS_RGB_ON)
    {
        rz_cru_statistics_rgb();
    }
    /*show statistics differential image*/
    if (CRUAPP_FUN_ON == STATS_DIFF_ON)
    {
        rz_cru_statistics_diff();    
    }

    /*AE start*/
    if ((CRUAPP_FUN_ON == AE_ON) | (CRUAPP_FUN_OFF == AE_ON))
    {
        rz_cru_AE(fd);
    }
    /*AE end*/    

    /*AF start*/
    if (CRUAPP_FUN_ON == AF_ON)
    {
        rz_cru_AF();
    }
    /*AF end*/
    /*AWB start*/
    if (CRUAPP_FUN_ON == AWB_ON && CRUAPP_FUN_OFF == WB_ON)
    {
        rz_cru_AWB(fd);
    }
    if (CRUAPP_FUN_ON == WB_ON)
    {
        rz_cru_WB(1,1,1,fd);
    }
    /*AWB end*/
    CLEAR(statistics);

    xioctl(fd, VIDIOC_QBUF, &v4l2buffers);

    return;
}

/*
 *Function: rz_cru_stop_streaming
 * -------------------------
 *  stop streaming.
 *
 *  returns: None.
 */

static void rz_cru_stop_streaming(void)
{
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
    {
        errno_exit("VIDIOC_STREAMOFF");
    }

    return;
}

/*
 *Function: v_dev_freebuffers
 * -------------------------
 *  free buffers.
 *
 *  returns: None.
 */
static void v_dev_freebuffers(void)
{
    for (unsigned int i = 0; i < n_buffers; ++i)
    {
        if (-1 == munmap(buffers[i].start, buffers[i].length))
        {
            errno_exit("munmap video buffers");
        }
    }

    return;
}

/*
 *Function: fb_dev_buffers
 * -------------------------
 *  free buffers.
 *
 *  returns: None.
 */
static void fb_dev_freebuffers(void)
{
    if (-1 == munmap (screen_base, sizeof (screen_base)))
    {
        errno_exit("munmap frame buffers");
    }

    return;
}

/*
 *Function: v_dev_close
 * -------------------------
 *  close devices.
 *
 *  returns: None.
 */

static void v_dev_close(void)
{
    if (-1 == close(fd))
    {
        errno_exit("Close video device");
        fd = -1;
    }

    return;
}

/*
 *Function: fb_dev_close
 * -------------------------
 *  close devices.
 *
 *  returns: None.
 */
 
static void fb_dev_close(void)
{
    if (-1 == close(fb_fd))
    {
        errno_exit("Close framebuffer device");
        fb_fd = -1;
    }

    return;
}

/*
 *Function: camera_media_ctl
 * -------------------------
 *  media-ctl commands for initializing MIPI camera capture.
 *
 *  returns: None.
 */
static void camera_media_ctl(void)
{
    const char* com[4] = {
        "media-ctl -d /dev/media0 -r",
        "media-ctl -d /dev/media0 -l \"\'rzg2l_csi2 10830400.csi2\':1 -> \'CRU output\':0 [1]\"",
        "media-ctl -d /dev/media0 -V \"\'rzg2l_csi2 10830400.csi2\':1 [fmt:SBGGR8_1X8/1280x960 field:none]\"",
        "media-ctl -d /dev/media0 -V \"\'ov5645 0-003c\':0 [fmt:SBGGR8_1X8/1280x960 field:none]\""
    };
    for(int i=0; i<4; i++){
        int ret = system(com[i]);
        if(ret<0){
            errno_exit("media-ctl");
        }
    }

    return;
}
/*
 *Function: camera_function_off
 * -------------------------
 *  v4l2-ctl commands for stopping OV5645 camera sensor functions.
 *  v4l2-ctl commands for enable linear matrix processing and statistics.
 *  v4l2-ctl commands for skipping the first 3 frames feature wait stability state from the camera sensor.
 *  killall weston for using the framebuffer.
 *  returns: None.
 */
static void camera_function_off(void)
{
    char com[256];

    sprintf(com, 
            "killall weston-launch;"
            "v4l2-ctl -cgain_automatic=0;"
            "v4l2-ctl -cwhite_balance_automatic=0;"
            "v4l2-ctl -clinear_matrix_processing_enable=1;"
            "v4l2-ctl -cstatistics_data_enable_disable=1;"
            "v4l2-ctl -cskipping_frames_enable_disable=1");
    system(com);

    return;
}

/*
 *Function: option_rgb_check
 * -------------------------
 * The CUI interface for r,g,b indexes.
 * returns: None.
 */
static int option_rgb_check(int c)
{
    switch (c) {
        case 'r':
            LMP_ON = CRUAPP_FUN_ON;
            return OPTIONS_R;
        case 'g':
            LMP_ON = CRUAPP_FUN_ON;
            return OPTIONS_G;
        case 'b':
            LMP_ON = CRUAPP_FUN_ON;
            return OPTIONS_B;
        default:
            printf("RGB check failed. Invalid %c", c);
            return -1;
    }
}

/*
 *Function: print_cru_cui_info
 * -------------------------
 * Print information about CUI settings.
 * returns: None.
 */

static void print_cru_cui_info(void)
{

    printf("Linear Matrix Porcessing Setting\n");
    for (int i = 0; i < 12; i++)
    {
        printf("Setting:%s\t\tValue:%d\tIS_SET %d\n", name[i],  linear_matrix_val[i], is_set[i]);
    }
    printf("CRU Demo function Setting\n");
    for (int i = 0; i < 5; i++)
    {
        if (i < 3)
        {
            printf("Setting:%s\t\tValue:%s\tIS_SET %d\n", func_name[i],  "-", is_set_func[i]);
        }
        else
        {
            printf("Setting:%s\tValue:%s\tIS_SET %d\n", func_name[i],  "-", is_set_func[i]);
        }
    }
    printf("%s\t\tValue:%d\n", "Frame",  frame_count);
    printf("The video size is %dx%d, "
            "and the croping coordinates are x:%d, y:%d\n",
            target_width,target_height,crop_x,crop_y);
    printf("Please use CTRL+C on the keyboard of the development PC to stop the CRU Sample Application at any time.\n");
    
    return;
}

/*
 *Function: check_adjust_coefficient
 * -------------------------
 * This function check the coefficient value and adjust it.
 * returns: None.
 */

static int check_adjust_coefficient(int x)
{
    int result;
    if ( (x > 4095) | (x < -4096) ) 
    {
        printf("--------------------------------------------------------------------------\n");
        printf("Warning: value is out of range.\n");
        result = (x > 0) ? 4095 : -4096;
        printf("The coefficient value is reset to %d.\n",(int)result);
        printf("Please use v4l2-ctl -L command to refer to the coefficient parameter range.\n");
        printf("--------------------------------------------------------------------------\n");
    } 
    else 
    {
        result = x;
    }
    return result;
}
/*
 *Function: check_adjust_offset
 * -------------------------
 * This function check the offset value and adjust it.
 * returns: None.
 */
static int check_adjust_offset(int x)
{
    int result;
    if ( (x > 127) | (x < -128) )
    {
        printf("--------------------------------------------------------------------------\n");
        printf("Warning: value is out of range.\n");
        result = (x > 0) ? 127 : -128;
        printf("The offset value is reset to %d.\n", (int)result);
        printf("Please use v4l2-ctl -L command to refer to the offset parameter range.\n");
        printf("--------------------------------------------------------------------------\n");
    }
    else
    {
        result = x;
    }
    return result;
}

/*
 *Function: check_adjust_csp
 * -------------------------
 * This function check the Color Separation Parameters(CSP) values and adjust it.
 * The upper limit here is a temporary setting, 
 * please set the value reasonably according to your environment.
 * returns: None.
 */
static float check_adjust_csp(float x,char* s)
{
    float result = x;
    if (x < 0)
    {

        printf("--------------------------------------------------------------------------\n");
        printf("Warning: The Color Separation Parameters(CSP) %s Value is outside the lower bounds.\n",s);
        result = 1;
        printf("The CSP %s value is reset to %d.\n", s,(int)result);
        printf("--------------------------------------------------------------------------\n");
    }
    
    if(x > 2)
    {
        printf("--------------------------------------------------------------------------\n");
        printf("Warning: The Color Separation Parameters(CSP) %s Value is outside the upper bounds.\n",s);
        result = 1;
        printf("The CSP %s value is reset to %d.\n",s, (int)result);
        printf("--------------------------------------------------------------------------\n");
    }
    return result;
}

/*
 *Function: check_crop_image
 * -------------------------
 * This function check the crop image values and adjust it.
 * returns: None.
 */

static void check_crop_image(int crop_x, int crop_y, int target_width, int target_height) {
    /*Check if crop parameters are valid*/
    if (crop_x < 0 || crop_y < 0 || target_width < 1 || target_height < 1 ||
            crop_x + target_width > IMAGE_WIDTH || crop_y + target_height > IMAGE_HEIGHT) {
        printf("--------------------------------------------------------------------------\n");
        printf("Invalid crop parameters! Original image size is %dx%d.\n"
                "Requested crop parameters are (crop_x:%d,crop_y:%d,target_width:%d,target_height:%d):\n", 
                IMAGE_WIDTH, IMAGE_HEIGHT, crop_x, crop_y, target_width, target_height);
        if (crop_x < 0 || crop_y < 0) {
            printf("Start point (%d,%d) cannot be less than the top-left corner (0,0) of the original image.\n",
                    crop_x, crop_y);
            printf("--------------------------------------------------------------------------\n");
            printf("CRU OFF\n");
            system("weston-start");
            exit (EXIT_FAILURE);

        }
        if (crop_x + target_width > IMAGE_WIDTH || crop_y + target_height > IMAGE_HEIGHT) {
            printf("Crop parameters are out of bounds.\n");
            printf("--------------------------------------------------------------------------\n");
            printf("CRU OFF\n");
            system("weston-start");
            exit (EXIT_FAILURE);

        }
        if (target_width < 1 || target_height < 1) {
            printf("Crop size cannot be less than 1.\n");
            printf("--------------------------------------------------------------------------\n");
            printf("CRU OFF\n");
            system("weston-start");
            exit (EXIT_FAILURE);

        }
    }
    return;
}
/*
 *Function: rz_cru_cui
 * -------------------------
 * This function provides the CUI interface of the linux specification.
 * returns: None.
 */

static void rz_cru_cui (int argc, char **argv)
{
    int c;

    while (1)
    {
        int option_index = 0;
        c = getopt_long(argc, argv, "r:g:b:",
                cru_long_options, &option_index);
        if (c == -1)
        {
            break;
        }
        switch (c)
        {
            case OPTIONS_RR:
            case OPTIONS_RG:
            case OPTIONS_RB:
            case OPTIONS_GR:
            case OPTIONS_GG:
            case OPTIONS_GB:
            case OPTIONS_BR:
            case OPTIONS_BG:
            case OPTIONS_BB:
                linear_matrix_val[c] = check_adjust_coefficient(atoi(optarg));
                is_set[c] = 1;
                LMP_ON = CRUAPP_FUN_ON;
                break;
            case OPTIONS_AWB:
                AWB_ON = CRUAPP_FUN_ON;
                is_set_func[c-12] = 1;
                break;
            case OPTIONS_AE:
                AE_ON = CRUAPP_FUN_ON;
                is_set_func[c-12] = 1;
                break;
            case OPTIONS_AF:
                AF_ON = CRUAPP_FUN_ON;
                is_set_func[c-12] = 1;
                break;
            case OPTIONS_STATSRGB:
                STATS_RGB_ON = CRUAPP_FUN_ON;
                is_set_func[c-12] = 1;
                break;
            case OPTIONS_STATSDIFF:
                STATS_DIFF_ON = CRUAPP_FUN_ON;
                is_set_func[c-12] = 1;
                break;
            case OPTIONS_FRAME:
                frame_count = atoi(optarg);
                break;
            case OPTIONS_CSR:
                camera_cs_r = check_adjust_csp(atof(optarg),"csr");
                break;
            case OPTIONS_CSG:
                camera_cs_g = check_adjust_csp(atof(optarg),"csg");
                break;
            case OPTIONS_CSB:
                camera_cs_b = check_adjust_csp(atof(optarg),"csb");
                break;
            case OPTIONS_WB:
                WB_ON = CRUAPP_FUN_ON;
                WB_TYPE = atoi(optarg);
                break;
            case OPTIONS_NONDISPLAY:
                DISPLAY_ON = CRUAPP_FUN_OFF;
                break;
            case OPTIONS_CROP_X:
                crop_x = atoi(optarg);
                break;
            case OPTIONS_CROP_Y:
                crop_y = atoi(optarg);
                break;
            case OPTIONS_TARGET_WIDTH:
                target_width = atoi(optarg);
                break;
            case OPTIONS_TARGET_HEIGHT:
                target_height = atoi(optarg);
                break;
            case 'r':
            case 'g':
            case 'b':
                c = option_rgb_check(c);
                linear_matrix_val[c]= check_adjust_offset(atoi(optarg+1));/*remove "="*/
                is_set[c] = 1;
                break;

            case '?':
                printf("CRU OFF\n");
                system("weston-start");
                exit (EXIT_FAILURE);

                break;

            default:
                printf("?? getopt returned character code 0%o ??\n", c);
        }
    }
    if (optind < argc)
    {
        printf("non-option ARGV-elements: ");
        while (optind < argc)
        {
            printf("%s ", argv[optind++]);
        }
        printf("\n");
    }
    if ( (CRUAPP_FUN_ON == LMP_ON) && ((CRUAPP_FUN_ON == AWB_ON) | (CRUAPP_FUN_ON == AE_ON) | (CRUAPP_FUN_ON == AF_ON)) )
    {
        printf("The specified color correction function cannot be used simultaneously with the 3A algorithm!\n");
        exit(EXIT_FAILURE);
    }
    if( (CRUAPP_FUN_ON == LMP_ON) |(frame_count > 0))
    {
        print_cru_cui_info();
    }
    
    check_crop_image(crop_x,crop_y,target_width,target_height);

    return;
}

/*
 *Function: rz_cru_key_detection
 * -------------------------
 * This function can detect keyboard input events and realize the on and off of various image processing functions.
 * returns: None.
 */
static void* rz_cru_key_detection(void* arg __attribute__((unused)))
{
    int k_ret = -1;
    struct input_event key_event  = {0};
    key_fd = open(KEY_DEV, O_RDONLY);
    if(key_fd <= 0)
    {
        errno_exit("open key device");    
    }
    while(1)
    {
        k_ret = lseek(key_fd, 0, SEEK_SET);
        k_ret = read(key_fd, &key_event, sizeof(key_event));
        if(k_ret)
        {
            if(key_event.type == EV_KEY
                    && ( key_event.value == 1))
            {
                if(key_event.code == KEY_W)
                {
                    AWB_ON = (~AWB_ON)&0x1;
                    if (AWB_ON == CRUAPP_FUN_ON)
                    {
                        printf ("AWB ON\n");
                    }
                    if (AWB_ON == CRUAPP_FUN_OFF)
                    {
                        printf("AWB OFF\n");
                    }
                }
                if(key_event.code == KEY_E)
                {
                    AE_ON = (~AE_ON)&0x1;
                    if (AE_ON == CRUAPP_FUN_ON)
                    {
                        printf ("AE ON\n");
                    }
                    if (AE_ON == CRUAPP_FUN_OFF)
                    {
                        printf("AE OFF\n");
                    }
                }
                if(key_event.code == KEY_F)
                {
                    AF_ON = (~AF_ON)&0x1;
                    if (AF_ON == CRUAPP_FUN_ON)
                    {
                        printf ("AF ON\n");
                    }
                    if (AF_ON == CRUAPP_FUN_OFF)
                    {
                        printf("AF OFF\n");
                    }
                }
                if(key_event.code == KEY_R)
                {
                    STATS_RGB_ON = (~STATS_RGB_ON)&0x1;
                }
                if(key_event.code == KEY_D)
                {
                    STATS_DIFF_ON = (~STATS_DIFF_ON)&0x1;
                }
                if(key_event.code == KEY_I)
                {
                    WB_ON = (~WB_ON)&0x1;
                    if (WB_ON == CRUAPP_FUN_ON)
                    {
                        printf ("WB ON\n");
                        printf("WB model:\n1.AWB, 2.FLUORESCENT, 3.TUNGSTEN\n");
                    }
                    if (WB_ON == CRUAPP_FUN_OFF)
                    {
                        printf("WB OFF\n");
                    }
                }

                if (key_event.code == KEY_1 && WB_ON == CRUAPP_FUN_ON)
                {
                    WB_TYPE = WB_AWB_ON;
                    printf("AWB ON\n");
                }
                if (key_event.code == KEY_2 && WB_ON == CRUAPP_FUN_ON)
                {
                    WB_TYPE = WB_FLUORESCENT_ON;
                    printf("FLUORESCENT ON\n");
                }
                if (key_event.code == KEY_3 && WB_ON == CRUAPP_FUN_ON)
                {
                    WB_TYPE = WB_TUNGSTEN_ON;
                    printf("TUNGSTEN ON\n");
                }
                if(key_event.code == KEY_ESC)
                {              
                    CRU_OFF = CRUAPP_FUN_ON;
                    printf("CRU closing\n");
                }

            }
        }

    }
    close(key_fd);
    return NULL;
}

/*
 *Function: print_information
 * -------------------------
 * This function can print the instruction information of the CRU Demo function and
 * the warning information under the CUI command.
 * returns: None.
 */
static void print_information(void)
{
    if (LMP_ON == CRUAPP_FUN_OFF)
    {
        printf("--------------------------------------------------------------------------\n"
                "Please use the keyboard connected to the development board to\n"
                "press the following keys to enable or disable the CRU demo function\n"
                "--------------------------------------------------------------------------\n"
                "W:\t Auto White Balance (AWB) Enable/Disable\n"
                "E:\t Auto Exposure (AE) Enable/Disable\n"
                "F:\t Auto Focus (AF) Enable/Disable\n"
                "I:\t White Balance (WB) Enable/Disable (press number keys after WB ON)\n"
		"R:\t Show Statitics rgb data\n"
                "D:\t Show Statitics differential image\n"
                "ESC:\t Stop the CRU Smaple Application\n"
                "--------------------------------------------------------------------------\n"
              );
    }
    if (LMP_ON == CRUAPP_FUN_ON)
    {
        printf("--------------------------------------------------------------------------\n"
                "Linear Matrix Processing debug mode is on, AWB, AE, AF is not working.\n"
                "--------------------------------------------------------------------------\n");
    }

    return;
}

/*
 * Function: sigint_handler
 * ------------------------
 * Signal handler for SIGINT (Ctrl+C) signal.
 * Sets the global flag ctrl_c_flag to ON to indicate that the signal has been received.
 *
 * sig: the signal number, which is not used in this function.
 *
 * returns: None.
 */

static void sigint_handler(int sig)
{
    (void)sig;
    ctrl_c_flag = CRUAPP_FUN_ON;

    return;
}

/*
 *Function: main
 * -------------------------
 * The CRU Sample Application main processing.
 * returns: None.
 */
int main(int argc, char** argv)
{
    signal(SIGINT, sigint_handler);
    
    /*camera function off*/
    camera_media_ctl();
    camera_function_off();
    
    /*get CUI*/
    rz_cru_cui(argc,argv);

    /*device init*/
    v_dev_init();
    fb_dev_init();
    
    /*get device info*/
    rz_cru_capability();
    print_information();
    
    /*linear matrix processing init*/
    rz_cru_lmp_all(0,0,0,1*CRU_HW_PARAMETER,0,0,0,1*CRU_HW_PARAMETER,0,0,0,1*CRU_HW_PARAMETER,fd);
    
    /*get key events on the second thread*/
    pthread_create(&tid,NULL,rz_cru_key_detection,NULL);
    
    /*cru main start*/
    rz_cru_requestbuffers();    
    rz_cru_start_streaming();    
    printf("CRU ON\n");
    if (frame_count != 0)
    {
        for (unsigned int i = 0; i < frame_count; i++)
        {
            rz_cru_img_processing();
            if ( (CRU_OFF == CRUAPP_FUN_ON) | ctrl_c_flag)
            {
                i = frame_count;
            }
        }
    }
    else
    {
        for (;;)
        {
            /*img proc*/
            rz_cru_img_processing();
            if ( (CRU_OFF == CRUAPP_FUN_ON) | ctrl_c_flag)
            {
                break;
            }
        }

    }
    rz_cru_stop_streaming();
    /*cru main end*/
    
    /*free buffers*/
    v_dev_freebuffers();
    fb_dev_freebuffers();
    
    /*close device*/
    v_dev_close();
    fb_dev_close();
    printf("CRU OFF\n");
    
    /*back to weston*/
    system("weston-start");
    exit (EXIT_SUCCESS);
}
