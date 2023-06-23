/***********************************************************************************************************************
 * Source of the RZ CRU Sample Application.
 *
 * Copyright (C) 2023 Renesas Electronics Corp. All rights reserved.
 ***********************************************************************************************************************/
/***********************************************************************************************************************
 * File Name    : cru_img_proc.h
 * Version      : 1.0.0
 * Description  : RZ MPU CRU Sample Application Image Processing Functions Headfile
 ***********************************************************************************************************************/

#ifndef CRU_IMG_PROC_H
#define CRU_IMG_PROC_H

#include <linux/videodev2.h>
#include <math.h>

/*Macro Definition*/
#define RZ_MPU_USER_CRU_BASE  (0x009819e0)
#define RZ_MPU_CRU_LINEAR_MATRIX_ROF  (RZ_MPU_USER_CRU_BASE + 6)
#define RZ_MPU_CRU_LINEAR_MATRIX_GOF  (RZ_MPU_USER_CRU_BASE + 7)
#define RZ_MPU_CRU_LINEAR_MATRIX_BOF  (RZ_MPU_USER_CRU_BASE + 8)
#define RZ_MPU_CRU_LINEAR_MATRIX_RR   (RZ_MPU_USER_CRU_BASE + 9)
#define RZ_MPU_CRU_LINEAR_MATRIX_RG   (RZ_MPU_USER_CRU_BASE + 10)
#define RZ_MPU_CRU_LINEAR_MATRIX_RB   (RZ_MPU_USER_CRU_BASE + 11)
#define RZ_MPU_CRU_LINEAR_MATRIX_GR   (RZ_MPU_USER_CRU_BASE + 12)
#define RZ_MPU_CRU_LINEAR_MATRIX_GG   (RZ_MPU_USER_CRU_BASE + 13)
#define RZ_MPU_CRU_LINEAR_MATRIX_GB   (RZ_MPU_USER_CRU_BASE + 14)
#define RZ_MPU_CRU_LINEAR_MATRIX_BR   (RZ_MPU_USER_CRU_BASE + 15)
#define RZ_MPU_CRU_LINEAR_MATRIX_BG   (RZ_MPU_USER_CRU_BASE + 16)
#define RZ_MPU_CRU_LINEAR_MATRIX_BB   (RZ_MPU_USER_CRU_BASE + 17)

#define CRU_HW_PARAMETER (1024) /*Please fix it unchanged*/

#define CLEAR(x)    (memset(&(x), 0, sizeof(x)))

#define FB_DEV  ("/dev/fb0")
#define VIDEO_DEV   ("/dev/video0")

#define IMAGE_WIDTH (1280) /*Please fix it unchanged*/
#define IMAGE_HEIGHT (960) /*Please fix it unchanged*/
#define IMAGE_CHANNEL (4) /*BGRA*/
#define IAMGE_SIZE (IMAGE_WIDTH*IMAGE_HEIGHT*IMAGE_CHANNEL)

#define BLOCK_PIXEL (16)
#define BLOCK_SIZE  (BLOCK_PIXEL*BLOCK_PIXEL) /*Statistics block size, please fix it unchanged*/
#define STATS_PIXEL ((int)(IMAGE_WIDTH/BLOCK_PIXEL)*(int)(IMAGE_HEIGHT/BLOCK_PIXEL)) 
#define STATS_WIDTH (IMAGE_WIDTH/BLOCK_PIXEL)
#define STATS_HEIGHT (IMAGE_HEIGHT/BLOCK_PIXEL)
#define STATS_ADDRESS (0x4b0000) /*0x"IMAGE_SIZE", please fix it unchanged*/

#define AE_GRAY_THRESHOLD (140) /*For reference only*/
#define AE_GRAY_RANGE (20) /*For reference only*/
#define AE_GAIN (120) /*For reference only*/

#define AF_GRADIENT_THRESHOLD (4) /*For reference only*/
#define AF_GRAY_THRESHOLD (50) /*For reference only*/
#define DIFF_GAIN (2) /*For reference only*/

#define POSITION_STATS_RGB  (160)
#define POSITION_STATS_DIFF (20)
#define DIFFGAIN        (10)

#define CRUAPP_FUN_ON  (1)
#define CRUAPP_FUN_OFF (0)
#define WB_AWB_ON (1)
#define WB_FLUORESCENT_ON (2)
#define WB_TUNGSTEN_ON (3)


/*Global value*/
extern int AWB_ON;
extern int AE_ON;
extern int AF_ON;
extern int STATS_RGB_ON;
extern int STATS_DIFF_ON;
extern int LMP_ON;
extern int CRU_OFF;
extern int WB_ON;
extern int WB_TYPE;
extern float camera_cs_r;
extern float camera_cs_g;
extern float camera_cs_b;

extern int line_length;
extern unsigned int* screen_base;

struct buffer 
{
    void* start;
    size_t length;
}buffer;
struct buffer* buffers;

struct stats
{
    float rsum;
    float gsum;
    float bsum;
    float gray;
    float diffavg;
    float max_rgb_avg;
    int diff[STATS_PIXEL];
    int rgb[STATS_PIXEL];
};
struct stats statistics;     
struct v4l2_buffer v4l2buffers;

/*Function Declaration*/
int xioctl(int fh, int request, void* arg);

int rz_cru_lmp_all(int rof, int gof, int bof,
        int rr, int rg, int rb,
        int gr, int gg, int gb,
        int br, int bg, int bb, int fd);

int rz_cru_lmp_rgb(int rr, int gg, int bb, int fd);

void rz_cru_AWB(int fd);

void rz_cru_WB(float cs_r, float cs_g, float cs_b,int fd);

void rz_cru_AE(int fd);

void rz_cru_AF(void);

void rz_cru_statistics_rgb (void);

void rz_cru_statistics_diff (void);

void rz_cru_get_statistics (void);

#endif /* CRU_IMG_PROC_H */
