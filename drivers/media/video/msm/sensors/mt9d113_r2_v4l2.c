/* Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "msm_sensor.h"
#include "sensor_ctrl.h" 
#include "sensor_i2c.h" 


#if 1//test
#include <linux/regulator/machine.h> 
#include "sensor_ctrl.h"

#include <media/v4l2-subdev.h>
#include "msm.h"
#include "msm_ispif.h"
#include "msm_camera_i2c.h"
#if 1 // jjhwang File 
#include <linux/syscalls.h>
#endif

#include <linux/gpio.h>
#endif
//#include "mt9d113_cfg_SKT_v4l2.h"

#define SENSOR_NAME "mt9d113"
#define PLATFORM_DRIVER_NAME "msm_camera_mt9d113"
#define mt9d113_obj mt9d113_##obj

#define F_MT9D113_POWER
#if 1
#undef CDBG
#define CDBG(fmt, args...) pr_err(fmt, ##args)
#endif 
#if defined(CONFIG_MACH_MSM8X60_EF65L)
/* CE1612 + MT9D113 */

#define CAMIO_R_RST_N	0
#define CAMIO_R_STB_N	1
#define CAMIO_F_RST_N	2
#define CAMIO_F_STB	3
#define CAMIO_MAX	4

static sgpio_ctrl_t sgpios[CAMIO_MAX] = {
	{CAMIO_R_RST_N, "CAMIO_R_RST_N", 106},
	{CAMIO_R_STB_N, "CAMIO_R_STB_N",  62},
	{CAMIO_F_RST_N, "CAMIO_F_RST_N", 137},
	{CAMIO_F_STB,   "CAMIO_F_STB",   139},
};

#define CAMV_IO_1P8V	0
#define CAMV_CORE_1P8V	1
#define CAMV_A_2P8V	2
#define CAMV_MAX	3

static svreg_ctrl_t svregs[CAMV_MAX] = {
	{CAMV_IO_1P8V,   "8901_lvs1", NULL, 0},//1800}, /* I2C pull-up */
	{CAMV_CORE_1P8V, "8901_mvs0", NULL, 0},//1800},
	{CAMV_A_2P8V,    "8058_l9",   NULL, 2800},
};
#elif defined(CONFIG_MACH_MSM8X60_EF40S) || \
    defined(CONFIG_MACH_MSM8X60_EF40K) 
/* CE1612 + MT9D113 */

#define CAMIO_R_RST_N	0
#define CAMIO_R_STB_N	1
#define CAMIO_F_RST_N	2
#define CAMIO_F_STB	3
#define CAMIO_MAX	4

static sgpio_ctrl_t sgpios[CAMIO_MAX] = {
	{CAMIO_R_RST_N, "CAMIO_R_RST_N", 106},
	{CAMIO_R_STB_N, "CAMIO_R_STB_N",  46},
	{CAMIO_F_RST_N, "CAMIO_F_RST_N", 137},
	{CAMIO_F_STB,   "CAMIO_F_STB",   139},
};

#define CAMV_IO_1P8V	0
#define CAMV_CORE_1P8V	1
#define CAMV_A_2P8V	2
#define CAMV_MAX	3

static svreg_ctrl_t svregs[CAMV_MAX] = {
	{CAMV_IO_1P8V,   "8901_mvs0", NULL, 0},//1800}, /* I2C pull-up */
	{CAMV_CORE_1P8V, "8901_lvs3", NULL, 0},//1800},
	{CAMV_A_2P8V,    "8058_l19",  NULL, 2800},
};

#else
#error "unknown machine!"
#endif

#if 1
static struct msm_cam_clk_info cam_mclk_info[] = {
	{"cam_clk", MSM_SENSOR_MCLK_24HZ},
};
#endif

#if 1//needtocheck
extern si2c_const_param_t mt9d113_const_params[SI2C_PID_MAX];
static si2c_param_t mt9d113_params[SI2C_PID_MAX];

/* I2C slave address */
#define SI2C_SA	((mt9d113_client->addr) >> 1)

#if 0
typedef struct {
	struct work_struct work;
} mt9d113_work_t;

typedef struct {
	const struct msm_camera_sensor_info *sinfo;
} mt9d113_ctrl_t;
#endif

extern si2c_const_param_t mt9d113_const_params[SI2C_PID_MAX];
static si2c_param_t mt9d113_params[SI2C_PID_MAX];

//static mt9d113_ctrl_t *mt9d113_ctrl = NULL;
static struct i2c_client *mt9d113_client = NULL;
//static mt9d113_work_t *mt9d113_work = NULL;
#endif

DEFINE_MUTEX(mt9d113_mut);
static struct msm_sensor_ctrl_t mt9d113_s_ctrl;

static int8_t sensor_mode = -1;   			// 0: full size,  1: qtr size, 2: fullhd size, 3: ZSL

static int current_fps = 31;
static int mt9d113_sensor_set_preview_fps(struct msm_sensor_ctrl_t *s_ctrl ,int8_t preview_fps);

#if 0//def F_STREAM_ON_OFF	
static struct msm_camera_i2c_reg_conf mt9d113_start_settings[] = {
	{0x0100, 0x01},
};

static struct msm_camera_i2c_reg_conf mt9d113_stop_settings[] = {
	{0x0100, 0x00},
};
#endif

#if 0
static struct msm_camera_i2c_reg_conf mt9d113_groupon_settings[] = {
	{0x104, 0x01},
};

static struct msm_camera_i2c_reg_conf mt9d113_groupoff_settings[] = {
	{0x104, 0x00},
};
#endif

#if 0
static struct msm_camera_i2c_reg_conf mt9d113_prev_settings[] = {
};

static struct msm_camera_i2c_reg_conf mt9d113_snap_settings[] = {
    {SI2C_WR, SI2C_A2D2, 0x098c, 0xa115, 0},
    #if 0 //20111006 fix VT brightness issue/ def C_PANTECH_CAMERA_EF40_MT9D113_TP10	
    {SI2C_WR, SI2C_A2D2, 0x0990, 0x0012, 0},
    #else
    {SI2C_WR, SI2C_A2D2, 0x0990, 0x0002, 0},
    #endif
    {SI2C_WR, SI2C_A2D2, 0x098c, 0xa103, 0},
    {SI2C_WR, SI2C_A2D2, 0x0990, 0x0002, 0},
    {SI2C_DELAY, SI2C_A1D1, 0, 0, 350},
    {SI2C_EOC, SI2C_A1D1, 0, 0, 0},
};

static struct msm_camera_i2c_reg_conf mt9d113_recommend_settings[] = {
};
#endif

static struct v4l2_subdev_info mt9d113_subdev_info[] = {
	{
	.code   =V4L2_MBUS_FMT_YUYV8_2X8,// V4L2_MBUS_FMT_SBGGR10_1X10,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};

#if 0
static struct msm_camera_i2c_conf_array mt9d113_init_conf[] = {
	{&mt9d113_recommend_settings[0],
	ARRAY_SIZE(mt9d113_recommend_settings), 0, MSM_CAMERA_I2C_WORD_DATA}
};

static struct msm_camera_i2c_conf_array mt9d113_confs[] = {
	{&mt9d113_snap_settings[0],
	ARRAY_SIZE(mt9d113_snap_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9d113_prev_settings[0],
	ARRAY_SIZE(mt9d113_prev_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9d113_prev_settings[0],
	ARRAY_SIZE(mt9d113_prev_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9d113_snap_settings[0],
	ARRAY_SIZE(mt9d113_snap_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
};
#endif

#if 0
static struct msm_sensor_output_info_t mt9d113_dimensions[] = {
	{
		.x_output = 0x1070,
		.y_output = 0xC30,
		.line_length_pclk = 0x1178,
		.frame_length_lines = 0xC90,
		.vt_pixel_clk = 182400000,
		.op_pixel_clk = 182400000,
		.binning_factor = 1,
	},
	{
		.x_output = 0x838,
		.y_output = 0x618,
		.line_length_pclk = 0x1178,
		.frame_length_lines = 0x634,
		.vt_pixel_clk = 216000000,
		.op_pixel_clk = 108000000,
		.binning_factor = 2,
	},
};

/* CAMIF output resolutions */
#define MT9D113_FULL_WIDTH                 1600
#define MT9D113_FULL_HEIGHT                1200
#define MT9D113_QTR_WIDTH                  800
#define MT9D113_QTR_HEIGHT                 600

/* Preview size 800x600, 24MHz MCLK 24Mhz PCLK 48Mhz (MIPI clk : 384 Mhz) */
uint32_t MT9D113_FULL_SIZE_DUMMY_PIXELS   = 0;
uint32_t MT9D113_FULL_SIZE_DUMMY_LINES    = 0;
uint32_t MT9D113_FULL_SIZE_WIDTH          = MT9D113_FULL_WIDTH;
uint32_t MT9D113_FULL_SIZE_HEIGHT         = MT9D113_FULL_HEIGHT;

uint32_t MT9D113_QTR_SIZE_DUMMY_PIXELS    = 0;
uint32_t MT9D113_QTR_SIZE_DUMMY_LINES     = 0;
uint32_t MT9D113_QTR_SIZE_WIDTH           = MT9D113_QTR_WIDTH;
uint32_t MT9D113_QTR_SIZE_HEIGHT          = MT9D113_QTR_HEIGHT;

uint32_t MT9D113_HRZ_FULL_BLK_PIXELS      = 16;
uint32_t MT9D113_VER_FULL_BLK_LINES       = 12;
uint32_t MT9D113_HRZ_QTR_BLK_PIXELS       = 16;
uint32_t MT9D113_VER_QTR_BLK_LINES        = 12;
#else

#define C_PANTECH_CAMERA_MIN_PREVIEW_FPS	5
#define C_PANTECH_CAMERA_MAX_PREVIEW_FPS	31

#define MT9D113_FULL_SIZE_DUMMY_PIXELS   0
#define MT9D113_FULL_SIZE_DUMMY_LINES    0
#define MT9D113_FULL_SIZE_WIDTH          1600//1280//3264
#define MT9D113_FULL_SIZE_HEIGHT         1200//960//2448

#define MT9D113_QTR_SIZE_DUMMY_PIXELS    0
#define MT9D113_QTR_SIZE_DUMMY_LINES     0
#define MT9D113_QTR_SIZE_WIDTH           800//640//1280 
#define MT9D113_QTR_SIZE_HEIGHT          600//480//960 

#define MT9D113_HRZ_FULL_BLK_PIXELS      16//0
#define MT9D113_VER_FULL_BLK_LINES       12//0

#define MT9D113_HRZ_QTR_BLK_PIXELS       16//0
#define MT9D113_VER_QTR_BLK_LINES        12//0

//#define S5K6AAFX13_1080P_SIZE_WIDTH        1920
//#define S5K6AAFX13_1080P_SIZE_HEIGHT        1088

//#define S5K6AAFX13_HRZ_1080P_BLK_PIXELS      0
//#define S5K6AAFX13_VER_1080P_BLK_LINES        0

//#define S5K6AAFX13_ZSL_SIZE_WIDTH    2560 
//#define S5K6AAFX13_ZSL_SIZE_HEIGHT   1920 


static struct msm_sensor_output_info_t mt9d113_dimensions[] = {
	{
		.x_output = MT9D113_FULL_SIZE_WIDTH,
		.y_output = MT9D113_FULL_SIZE_HEIGHT,
		.line_length_pclk = MT9D113_FULL_SIZE_WIDTH + MT9D113_HRZ_FULL_BLK_PIXELS ,
		.frame_length_lines = MT9D113_FULL_SIZE_HEIGHT+ MT9D113_VER_FULL_BLK_LINES ,
		.vt_pixel_clk = 21811200,//35000000,//198000000, //207000000, //276824064, 
		.op_pixel_clk = 40000000,//35000000,//198000000, //207000000, //276824064, 
		.binning_factor = 1,
	},
	{
		.x_output = MT9D113_QTR_SIZE_WIDTH,
		.y_output = MT9D113_QTR_SIZE_HEIGHT,
		.line_length_pclk = MT9D113_QTR_SIZE_WIDTH + MT9D113_HRZ_QTR_BLK_PIXELS,
		.frame_length_lines = MT9D113_QTR_SIZE_HEIGHT+ MT9D113_VER_QTR_BLK_LINES,
		.vt_pixel_clk = 21811200,//35000000, //207000000, //276824064, 
		.op_pixel_clk = 40000000,//35000000, //207000000, //276824064, 
		.binning_factor = 1,//2,
	},
	{
		.x_output = MT9D113_QTR_SIZE_WIDTH,
		.y_output = MT9D113_QTR_SIZE_HEIGHT,
		.line_length_pclk = MT9D113_QTR_SIZE_WIDTH + MT9D113_HRZ_QTR_BLK_PIXELS,
		.frame_length_lines = MT9D113_QTR_SIZE_HEIGHT+ MT9D113_VER_QTR_BLK_LINES,
		.vt_pixel_clk = 21811200,//35000000, //207000000, //276824064, 
		.op_pixel_clk = 40000000,//35000000, //207000000, //276824064, 
		.binning_factor = 1,//2,
	},
	{
		.x_output = MT9D113_FULL_SIZE_WIDTH,
		.y_output = MT9D113_FULL_SIZE_HEIGHT,
		.line_length_pclk = MT9D113_FULL_SIZE_WIDTH + MT9D113_HRZ_FULL_BLK_PIXELS ,
		.frame_length_lines = MT9D113_FULL_SIZE_HEIGHT+ MT9D113_VER_FULL_BLK_LINES ,
		.vt_pixel_clk = 21811200,//35000000, //207000000, //276824064, 
		.op_pixel_clk = 40000000,//35000000, //207000000, //276824064, 
		.binning_factor = 1,
	},	
};

#endif
static struct msm_camera_csi_params mt9d113_csic_params = {
	.data_format = CSI_8BIT,//CSI_10BIT,
	.lane_cnt    = 1,//4,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 0x14,
};

static struct msm_camera_csi_params *mt9d113_csic_params_array[] = {
	&mt9d113_csic_params,
	&mt9d113_csic_params,
	&mt9d113_csic_params,
	&mt9d113_csic_params,
};

static struct msm_camera_csid_vc_cfg mt9d113_cid_cfg[] = {
	{0, CSI_YUV422_8, CSI_DECODE_8BIT},//CSI_RAW10, CSI_DECODE_10BIT},
	{1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
	{2, CSI_RESERVED_DATA_0, CSI_DECODE_8BIT},
	{3, CSI_RESERVED_DATA_0, CSI_DECODE_8BIT},	
};

static struct msm_camera_csi2_params mt9d113_csi_params = {
	.csid_params = {
		.lane_cnt = 1,//4,
		.lut_params = {
			.num_cid = ARRAY_SIZE(mt9d113_cid_cfg),
			.vc_cfg = mt9d113_cid_cfg,
		},
	},
	.csiphy_params = {
		.lane_cnt = 1,//4,
		.settle_cnt = 0x14,//0x1B,
	},
};

static struct msm_camera_csi2_params *mt9d113_csi_params_array[] = {
	&mt9d113_csi_params,
	&mt9d113_csi_params,
	&mt9d113_csi_params,
	&mt9d113_csi_params,
};

#if 0 //for build
static struct msm_sensor_output_reg_addr_t s5k6aafx13_reg_addr = {
	.x_output = 0x34C,
	.y_output = 0x34E,
	.line_length_pclk = 0x342,
	.frame_length_lines = 0x340,
};
#endif

static struct msm_sensor_id_info_t mt9d113_id_info = {
	.sensor_id_reg_addr = 0x0000,
	.sensor_id = 0x0,//0x0074,
};

#if 0 //for build
static struct msm_sensor_exp_gain_info_t s5k6aafx13_exp_gain_info = {
	.coarse_int_time_addr = 0x202,
	.global_gain_addr = 0x204,
	.vert_offset = 3,
};
#endif

int32_t mt9d113_sensor_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
    int rc = 0;
#if 0    
    uint16_t chipid = 999;
#endif
	struct msm_sensor_ctrl_t *s_ctrl;
	pr_err("%s i2c_probe called\n", __func__); //build
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("%s %s i2c_check_functionality failed\n",
			__func__, client->name);
		rc = -EFAULT;
		return rc;
	}

#if 1
    mt9d113_client = client;
#else
    mt9d113_work = kzalloc(sizeof(s5k6aafx13_work_t), GFP_KERNEL);
    if (!mt9d113_work) {
        pr_err("%s (-ENOMEM)\n", __func__);
        return -ENOMEM;
    }

    i2c_set_clientdata(client, s5k6aafx13_work);
    mt9d113_client = client;
#endif

	s_ctrl = (struct msm_sensor_ctrl_t *)(id->driver_data);
	if (s_ctrl->sensor_i2c_client != NULL) {
		s_ctrl->sensor_i2c_client->client = client;
		if (s_ctrl->sensor_i2c_addr != 0)
			s_ctrl->sensor_i2c_client->client->addr =
				s_ctrl->sensor_i2c_addr;
	} else {
		pr_err("%s %s sensor_i2c_client NULL\n",
			__func__, client->name);
		rc = -EFAULT;
		return rc;
	}

	s_ctrl->sensordata = client->dev.platform_data;
	if (s_ctrl->sensordata == NULL) {
		pr_err("%s %s NULL sensor data\n", __func__, client->name);
		return -EFAULT;
	}
	rc = s_ctrl->func_tbl->sensor_power_up(s_ctrl);
	if (rc < 0) {
		pr_err("%s %s power up failed\n", __func__, client->name);
		return rc;
	}
    
#if 0//check
    pr_err("msm_sensor id: %d\n", chipid);
//    msleep(3000);
//    pr_err("msleep(3000);\n");


    rc = msm_camera_i2c_read(
            s_ctrl->sensor_i2c_client,
            s_ctrl->sensor_id_info->sensor_id_reg_addr, &chipid,
            MSM_CAMERA_I2C_WORD_DATA);
    pr_err("msm_sensor id: %d\n", chipid);

    if (rc < 0) {
        pr_err("%s: %s: read id failed\n", __func__,
        	s_ctrl->sensordata->sensor_name);
        //return rc;
        goto probe_fail;
    }

#else
	if (s_ctrl->func_tbl->sensor_match_id)
		rc = s_ctrl->func_tbl->sensor_match_id(s_ctrl);
	else
		rc = msm_sensor_match_id(s_ctrl);
	if (rc < 0)
		goto probe_fail;
    
#endif

	snprintf(s_ctrl->sensor_v4l2_subdev.name,
		sizeof(s_ctrl->sensor_v4l2_subdev.name), "%s", id->name);
	v4l2_i2c_subdev_init(&s_ctrl->sensor_v4l2_subdev, client,
		s_ctrl->sensor_v4l2_subdev_ops);

	msm_sensor_register(&s_ctrl->sensor_v4l2_subdev);
	goto i2c_probe_end;
    
probe_fail:
    pr_err("%s %s_i2c_probe failed\n", __func__, client->name);
    
i2c_probe_end:
#if 1
 	if (rc > 0)
		rc = 0;
	s_ctrl->func_tbl->sensor_power_down(s_ctrl);
#endif
    CDBG("%s_i2c_probe end\n", client->name);
	return rc; 
}

static const struct i2c_device_id mt9d113_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&mt9d113_s_ctrl},
	{ }
};

static struct i2c_driver mt9d113_i2c_driver = {
	.id_table = mt9d113_i2c_id,
	.probe  = mt9d113_sensor_i2c_probe,//msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client mt9d113_sensor_i2c_client = {
	.addr_type =MSM_CAMERA_I2C_WORD_ADDR,// MSM_CAMERA_I2C_WORD_ADDR, //ksy check
};

#if 1//def F_MT9D113_POWER
/* mt9d113_vreg_init */
static int mt9d113_vreg_init(void)
{
    int rc = 0;

    pr_err("%s E\n", __func__);
    
    rc = sgpio_init(sgpios, CAMIO_MAX);
    if (rc < 0)
        goto sensor_init_fail;

    rc = svreg_init(svregs, CAMV_MAX);
    if (rc < 0)
        goto sensor_init_fail;

    pr_err("%s X\n", __func__);
    return 0;
    
sensor_init_fail:
    svreg_release(svregs, CAMV_MAX);
    sgpio_release(sgpios, CAMIO_MAX);
    return -ENODEV;
}
/* msm_sensor_power_up */
//mt9d113_sensor_power_up
int32_t mt9d113_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
    int32_t rc = 0;
    CDBG("%s E\n", __func__);

#if 1//
        memset(mt9d113_params, 0, sizeof(mt9d113_params));
    
        rc = si2c_init(mt9d113_client->adapter, 
                mt9d113_const_params, mt9d113_params);
    
#else
        mt9d113_ctrl = kzalloc(sizeof(mt9d113_ctrl_t), GFP_KERNEL);
        if (!mt9d113_ctrl) {
            pr_err("%s err(-ENOMEM)\n", __func__);
        }
        mt9d113_ctrl->sinfo = s_ctrl->sensordata;
#endif

    rc = msm_sensor_power_up(s_ctrl);
    CDBG(" %s : msm_sensor_power_up : rc = %d E\n",__func__, rc);  

    mt9d113_vreg_init();

    if (sgpio_ctrl(sgpios, CAMIO_R_RST_N, 0) < 0)	rc = -EIO;
    if (sgpio_ctrl(sgpios, CAMIO_R_STB_N, 0) < 0)	rc = -EIO;
    mdelay(1);

    if (sgpio_ctrl(sgpios, CAMIO_F_STB, 0) < 0)	rc = -EIO;
    if (sgpio_ctrl(sgpios, CAMIO_F_RST_N, 1) < 0)	rc = -EIO;
    mdelay(1);

    if (svreg_ctrl(svregs, CAMV_IO_1P8V, 1) < 0)	rc = -EIO;
    mdelay(2); /* > 1ms */
    if (svreg_ctrl(svregs, CAMV_CORE_1P8V, 1) < 0)	rc = -EIO;
    mdelay(1);
    //msm_camio_clk_rate_set(24000000);     //build error : undefined reference
#if 1
pr_err("%s: [wsyang_debug] msm_cam_clk_enable() / 1 \n", __func__);

msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
    cam_mclk_info, &s_ctrl->cam_clk, ARRAY_SIZE(cam_mclk_info), 1);
#endif
    msleep(10); /* > 1 clk (approx. 0.042us) */
    if (sgpio_ctrl(sgpios, CAMIO_F_RST_N, 0) < 0)	rc = -EIO;
    /*PANTECH_CAMERA_TODO, reset was grounded with 10uF cap in WS20*/
    mdelay(10); /* > 10 clks (approx. 0.42us) */
    if (sgpio_ctrl(sgpios, CAMIO_F_RST_N, 1) < 0)	rc = -EIO;
    mdelay(1); /* > 1 clk (apporx. 0.042us) */
    if (svreg_ctrl(svregs, CAMV_A_2P8V, 1) < 0)	rc = -EIO;
    mdelay(2); /* > 6000 clks (approx. 252us) */

    current_fps = 31;

    CDBG("%s X (%d)\n", __func__, rc);
    return rc;
}

/* msm_sensor_power_down */
//mt9d113_sensor_power_down
int32_t mt9d113_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
    int32_t rc = 0;

    CDBG("%s E\n", __func__);

    msm_sensor_power_down(s_ctrl);
    CDBG(" %s : msm_sensor_power_down : rc = %d E\n",__func__, rc);

    if (sgpio_ctrl(sgpios, CAMIO_F_STB, 0) < 0) rc = -EIO;
    if (sgpio_ctrl(sgpios, CAMIO_F_RST_N, 0) < 0)   rc = -EIO;
    mdelay(1);
    
    /* MCLK will be disabled once again after this. */
    //  (void)msm_camio_clk_disable(CAMIO_CAM_MCLK_CLK);
	
    if (svreg_ctrl(svregs, CAMV_IO_1P8V, 0) < 0)    rc = -EIO;
    if (svreg_ctrl(svregs, CAMV_CORE_1P8V, 0) < 0)  rc = -EIO;
    if (svreg_ctrl(svregs, CAMV_A_2P8V, 0) < 0) rc = -EIO;
    mdelay(1);
    
    if (sgpio_ctrl(sgpios, CAMIO_R_RST_N, 0) < 0)   rc = -EIO;
    if (sgpio_ctrl(sgpios, CAMIO_R_STB_N, 0) < 0)   rc = -EIO;
    mdelay(1);
    
    svreg_release(svregs, CAMV_MAX);
    sgpio_release(sgpios, CAMIO_MAX);
    
    si2c_release();    
    CDBG("%s X (%d)\n", __func__, rc);
    return rc;
}
#endif

void mt9d113_sensor_start_stream(struct msm_sensor_ctrl_t *s_ctrl)
{
#if 1 
    SKYCDBG("%s: %d\n", __func__, __LINE__);
#endif
}

void mt9d113_sensor_stop_stream(struct msm_sensor_ctrl_t *s_ctrl)
{
#if 1 
    SKYCDBG("%s: %d\n", __func__, __LINE__);
#endif
}

int mt9d113_sensor_init(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rc = 0;
	//uint8_t data_buf[4];

	SKYCDBG("%s: E\n",__func__);



#if 1//needtocheck
	//sensor_mode = ??;//SENSOR_PREVIEW_MODE;

//	memset(mt9d113_params, 0, sizeof(mt9d113_params));

    //s_ctrl->sensor_i2c_client->client->adapter,
//	rc = si2c_init(s5k6aafx13_client->adapter, s5k6aafx13_const_params, s5k6aafx13_params);

#if 0
    rc = si2c_init(mt9d113_client->adapter, 
            mt9d113_const_params, mt9d113_params);
    
	if (!mt9d113_ctrl) {
		SKYCDBG("%s err(-ENOMEM)\n", __func__);
		goto sensor_init_fail;
	}
#endif


    rc = si2c_write_param(SI2C_SA, SI2C_INIT, mt9d113_params);
SKYCDBG("%s: si2c_write_param / rc:%d \n",__func__, rc);
    if (rc < 0)
        goto sensor_init_fail;

#endif
    sensor_mode = 1;

    SKYCDBG("%s: X\n",__func__);    
    return rc;
    
#if 1//needtocheck    
sensor_init_fail:

#if 0
	if (mt9d113_ctrl) {
		kfree(mt9d113_ctrl);
		mt9d113_ctrl = NULL;
	}
	(void)mt9d113_power_off();
	svreg_release(svregs, CAMV_MAX);
	sgpio_release(sgpios, CAMIO_MAX);
#endif    
SKYCDBG("%s: sensor_init_fail / si2c_release(); \n",__func__);
    si2c_release();
    
	SKYCDBG("%s err(%d)\n", __func__, rc);
	return rc;
#endif    
}


#if 0
/* msm_sensor_write_init_settings */
//mt9d113_sensor_write_init_settings
int32_t s5k6aafx13_sensor_write_init_settings(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc;
	rc = msm_sensor_write_all_conf_array(
		s_ctrl->sensor_i2c_client,
		s_ctrl->msm_sensor_reg->init_settings,
		s_ctrl->msm_sensor_reg->init_size);
	return rc;
}

/* msm_sensor_write_res_settings */
//mt9d113_sensor_write_res_settings
int32_t s5k6aafx13_sensor_write_res_settings(struct msm_sensor_ctrl_t *s_ctrl,
	uint16_t res)
{
	int32_t rc;
	rc = msm_sensor_write_conf_array(
		s_ctrl->sensor_i2c_client,
		s_ctrl->msm_sensor_reg->mode_settings, res);
	if (rc < 0)
		return rc;

	rc = msm_sensor_write_output_settings(s_ctrl, res);
	if (rc < 0)
		return rc;

	if (s_ctrl->func_tbl->sensor_adjust_frame_lines)
		rc = s_ctrl->func_tbl->sensor_adjust_frame_lines(s_ctrl, res);

	return rc;
}
#endif

/* msm_sensor_setting */
//mt9d113_sensor_setting
int32_t mt9d113_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;

SKYCDBG("%s: update_type = %d, res=%d E\n", __func__, update_type, res);
	s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
	msleep(30);
	if (update_type == MSM_SENSOR_REG_INIT) {
		s_ctrl->curr_csi_params = NULL;
		msm_sensor_enable_debugfs(s_ctrl);
//needtocheck//        
		mt9d113_sensor_init(s_ctrl);//msm_sensor_write_init_settings(s_ctrl);
	} else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
//needtocheck// 	
		//msm_sensor_write_res_settings(s_ctrl, res);
		if (s_ctrl->curr_csi_params != s_ctrl->csi_params[res]) {
            SKYCDBG("%s: ==> MIPI setting  E %d\n", __func__, update_type);
			s_ctrl->curr_csi_params = s_ctrl->csi_params[res];
			s_ctrl->curr_csi_params->csid_params.lane_assign =
				s_ctrl->sensordata->sensor_platform_info->
				csi_lane_params->csi_lane_assign;
			s_ctrl->curr_csi_params->csiphy_params.lane_mask =
				s_ctrl->sensordata->sensor_platform_info->
				csi_lane_params->csi_lane_mask;
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_CSID_CFG,
				&s_ctrl->curr_csi_params->csid_params);
			mb();
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_CSIPHY_CFG,
				&s_ctrl->curr_csi_params->csiphy_params);
			mb();
			msleep(20);
            SKYCDBG("%s: ==> MIPI setting  X %d\n", __func__, update_type);		
		}

		v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
			NOTIFY_PCLK_CHANGE, &s_ctrl->msm_sensor_reg->
			output_settings[res].op_pixel_clk);
		//s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
#if 0//needtocheck		
		switch (res) {
			case 0:
//				rc = ce1612_snapshot_config(s_ctrl);	
				break;

			case 1:
//				rc = ce1612_video_config(s_ctrl);
                si2c_write_param(SI2C_SA, SI2C_INIT, s5k6aafx13_params);

				SKYCDBG("Sensor setting : Case 1 Video config"); 
				break;

	      //case 2: 
			//	rc = ce1612_1080p_config(s_ctrl);	
			//	break;

			case 3: 
//				rc = ce1612_ZSL_config(s_ctrl);	
				SKYCDBG("Sensor setting : Case 3 ZSL config");				
				break;	
				 
			default:
//				rc = ce1612_video_config(s_ctrl);
				SKYCDBG("Sensor setting : Default Video config"); 
				break;
		}
#endif
		sensor_mode = res;
		SKYCDBG("Sensor setting : Res = %d\n", res);
        
		msleep(30);
	}
	SKYCDBG("%s: %d x\n", __func__, __LINE__);
	return rc;
}

int32_t mt9d113_sensor_setting1(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;
	static int csi_config;
    SKYCDBG("%s: update_type = %d, res=%d E\n", __func__, update_type, res);

	s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
	msleep(30);
	if (update_type == MSM_SENSOR_REG_INIT) {
		CDBG("Register INIT\n");
		s_ctrl->curr_csi_params = NULL;
		msm_sensor_enable_debugfs(s_ctrl);
//needtocheck//        
		mt9d113_sensor_init(s_ctrl);//msm_sensor_write_init_settings(s_ctrl);
		
		csi_config = 0;
	} else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
		CDBG("PERIODIC : %d\n", res);
		//msm_sensor_write_conf_array(
		//	s_ctrl->sensor_i2c_client,
		//	s_ctrl->msm_sensor_reg->mode_settings, res);
		msleep(30);
		if (!csi_config) {
            SKYCDBG("%s: ==> MIPI setting  E %d\n", __func__, update_type);
			s_ctrl->curr_csic_params = s_ctrl->csic_params[res];
			CDBG("CSI config in progress\n");
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_CSIC_CFG,
				s_ctrl->curr_csic_params);
			CDBG("CSI config is done\n");
			mb();
			msleep(30);
			csi_config = 1;
            SKYCDBG("%s: ==> MIPI setting  X %d\n", __func__, update_type);	
		}
		v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
			NOTIFY_PCLK_CHANGE,
			&s_ctrl->sensordata->pdata->ioclk.vfe_clk_rate);

		//s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
#if 1//needtocheck		
        switch (res) {
            case 0:
//              rc = ce1612_snapshot_config(s_ctrl);  
                rc = si2c_write_param(SI2C_SA, SI2C_SNAPSHOT, mt9d113_params);
                if (rc < 0) {
                    SKYCDBG("%s snapshot config err(%d)\n", __func__, rc);
                    return rc;
                }

                SKYCDBG("Sensor setting : Case 0 snapshot config"); 

                break;

            case 1:
//              rc = ce1612_video_config(s_ctrl);
                rc = si2c_write_param(SI2C_SA, SI2C_PREVIEW, mt9d113_params);
                if (rc < 0) {
                    SKYCDBG("%s video config err(%d)\n", __func__, rc);
                    return rc;
                }
#if 1//for_test
                if(current_fps != 31)
                    mt9d113_sensor_set_preview_fps(s_ctrl ,current_fps);
#else                
                if ((previewFPS > C_PANTECH_CAMERA_MIN_PREVIEW_FPS) && (previewFPS < C_PANTECH_CAMERA_MAX_PREVIEW_FPS)) 
	            {  
			       SKYCDBG("%s: preview_fps=%d\n", __func__, previewFPS);
                   mt9d113_sensor_set_preview_fps(NULL ,previewFPS);
                }
#endif
                SKYCDBG("Sensor setting : Case 1 Video config"); 
                break;

          //case 2: 
            //  rc = ce1612_1080p_config(s_ctrl);   
            //  break;

            case 3: 
//              rc = ce1612_ZSL_config(s_ctrl); 
                SKYCDBG("Sensor setting : Case 3 ZSL config");              
                break;  
                 
            default:
//              rc = ce1612_video_config(s_ctrl);
                SKYCDBG("Sensor setting : Default Video config"); 
                break;
        }
#endif
		sensor_mode = res;
		SKYCDBG("Sensor setting : Res = %d\n", res);

		
		msleep(50);
	}
	return rc;
}

/* sensor_set_function */
//mt9d113_set_function 
#ifdef CONFIG_PANTECH_CAMERA
static int mt9d113_sensor_set_brightness(struct msm_sensor_ctrl_t *s_ctrl ,int8_t brightness)
{
    si2c_pid_t pid = SI2C_PID_MAX;
    int rc = 0;

    CDBG("%s brightness=%d\n", __func__, brightness);

    switch (brightness) {
    case 0: pid = SI2C_BRIGHTNESS_M4; break;
    case 1: pid = SI2C_BRIGHTNESS_M3; break;
    case 2: pid = SI2C_BRIGHTNESS_M2; break;
    case 3: pid = SI2C_BRIGHTNESS_M1; break;
    case 4: pid = SI2C_BRIGHTNESS_0; break;
    case 5: pid = SI2C_BRIGHTNESS_P1; break;
    case 6: pid = SI2C_BRIGHTNESS_P2; break;
    case 7: pid = SI2C_BRIGHTNESS_P3; break;
    case 8: pid = SI2C_BRIGHTNESS_P4; break;
    default:
        SKYCDBG("%s err(-EINVAL)\n", __func__);
        return -EINVAL;
    }

    rc = si2c_write_param(SI2C_SA, pid, mt9d113_params);
    if (rc < 0) {
        SKYCDBG("%s err(%d)\n", __func__, rc);
        return rc;
    }

    CDBG("%s X\n", __func__);
    return 0;
}

static int mt9d113_sensor_set_effect(struct msm_sensor_ctrl_t *s_ctrl ,int8_t effect)
{
    si2c_pid_t pid = SI2C_PID_MAX;
    int rc = 0;

    CDBG("%s effect=%d\n", __func__, effect);

    switch (effect) {
    case 0: pid = SI2C_EFFECT_OFF; break;
    case 1: pid = SI2C_EFFECT_MONO; break;
    case 2: pid = SI2C_EFFECT_NEGATIVE; break;
    case 3: pid = SI2C_EFFECT_SOLARIZE; break;
    case 4: pid = SI2C_EFFECT_SEPIA; break;
    default:
        SKYCDBG("%s err(-EINVAL)\n", __func__);
        return -EINVAL;
    }

    rc = si2c_write_param(SI2C_SA, pid, mt9d113_params);
    if (rc < 0) {
        SKYCDBG("%s err(%d)\n", __func__, rc);
        return rc;
    }

    CDBG("%s X\n", __func__);
    return 0;
}

static int mt9d113_sensor_set_exposure_mode(struct msm_sensor_ctrl_t *s_ctrl ,int8_t exposure)
{
    si2c_pid_t pid = SI2C_PID_MAX;
    int rc = 0;

    CDBG("%s exposure=%d\n", __func__, exposure);

    switch (exposure) {
    case 1: pid = SI2C_EXPOSURE_AVERAGE; break;
    case 2: 
    case 3: pid = SI2C_EXPOSURE_CENTER; break;
    default:
        SKYCDBG("%s err(-EINVAL)\n", __func__);
        return -EINVAL;
    }

    rc = si2c_write_param(SI2C_SA, pid, mt9d113_params);
    if (rc < 0) {
        SKYCDBG("%s err(%d)\n", __func__, rc);
        return rc;
    }

    CDBG("%s X\n", __func__);
    return 0;
}

static int mt9d113_sensor_set_wb(struct msm_sensor_ctrl_t *s_ctrl ,int8_t wb)
{
    si2c_pid_t pid = SI2C_PID_MAX;
    int rc = 0;

    CDBG("%s wb=%d\n", __func__, wb);

    switch (wb) {
    case 1: pid = SI2C_WB_AUTO; break;
    case 3: pid = SI2C_WB_INCANDESCENT; break;
    case 4: pid = SI2C_WB_FLUORESCENT; break;
    case 5: pid = SI2C_WB_DAYLIGHT; break;
    case 6: pid = SI2C_WB_CLOUDY; break;
    default:
        SKYCDBG("%s err(-EINVAL)\n", __func__);
        return -EINVAL;
    }

    rc = si2c_write_param(SI2C_SA, pid, mt9d113_params);
    if (rc < 0) {
        SKYCDBG("%s err(%d)\n", __func__, rc);
        return rc;
    }

    CDBG("%s X\n", __func__);
    return 0;
}

static int mt9d113_sensor_set_preview_fps(struct msm_sensor_ctrl_t *s_ctrl ,int8_t preview_fps)
{
    si2c_pid_t pid = SI2C_PID_MAX;
    int rc = 0;

    SKYCDBG("%s preview_fps=%d\n", __func__, preview_fps);
	
    switch (preview_fps) {
	case 31:
    case  0: pid = SI2C_FPS_VARIABLE; break;
    case  7: pid = SI2C_FPS_FIXED7; break;
    case  8: pid = SI2C_FPS_FIXED8; break;
    case 10: pid = SI2C_FPS_FIXED10; break;
    case 15: pid = SI2C_FPS_FIXED15; break;
	case 30:
    case 20: pid = SI2C_FPS_FIXED20; break;
    case 24: pid = SI2C_FPS_FIXED24; break;
    case 25: pid = SI2C_FPS_FIXED25; break;
    default:
        //PANTECH_CAMERA_TODO
		SKYCDBG("%s preview_fps: %d default return 0\n", __func__, preview_fps);  
        return 0;
        //SKYCDBG("%s err(-EINVAL)\n", __func__);
        //return -EINVAL;
    }

    SKYCDBG("%s preview_fps:%d/ pid :%d\n", __func__, preview_fps, pid); 

    rc = si2c_write_param(SI2C_SA, pid, mt9d113_params);
    if (rc < 0) {
        SKYCDBG("%s err(%d)\n", __func__, rc);
        return rc;
    }

	current_fps = preview_fps;

    CDBG("%s X\n", __func__);
    return 0;
}

static int mt9d113_sensor_set_reflect(struct msm_sensor_ctrl_t *s_ctrl ,int8_t reflect)
{
    si2c_pid_t pid = SI2C_PID_MAX;
    int rc = 0;

    CDBG("%s reflect=%d\n", __func__, reflect);

    switch (reflect) {
#ifdef CONFIG_MACH_MSM8X60_EF65L 
    case 0: pid = SI2C_REFLECT_MIRROR_WATER; break;
    case 1: pid = SI2C_REFLECT_WATER; break;
    case 2: pid = SI2C_REFLECT_MIRROR; break;
    case 3: pid = SI2C_REFLECT_OFF; break;
#else
    case 0: pid = SI2C_REFLECT_OFF; break;
    case 1: pid = SI2C_REFLECT_MIRROR; break;
    case 2: pid = SI2C_REFLECT_WATER; break;
    case 3: pid = SI2C_REFLECT_MIRROR_WATER; break;
#endif
    default:
        SKYCDBG("%s err(-EINVAL)\n", __func__);
        return -EINVAL;
    }

    rc = si2c_write_param(SI2C_SA, pid, mt9d113_params);
    if (rc < 0) {
        SKYCDBG("%s err(%d)\n", __func__, rc);
        return rc;
    }

    CDBG("%s X\n", __func__);
    return 0;
}

#if 1 //def F_PANTECH_CAMERA_FIX_CFG_AE_AWB_LOCK
static int32_t mt9d113_sensor_set_aec_lock(struct msm_sensor_ctrl_t *s_ctrl ,int8_t is_lock)
{
    si2c_pid_t pid = SI2C_PID_MAX;
    int rc = 0;

    CDBG("%s is_lock = %d\n",__func__, is_lock);
    
    switch (is_lock) {
    case 0: pid = SI2C_AEC_ON; break;
    case 1: pid = SI2C_AEC_OFF; break;
    default:
        SKYCDBG("%s err(-EINVAL)\n", __func__);
        return -EINVAL;
    }

    rc = si2c_write_param(SI2C_SA, pid, mt9d113_params);
    if (rc < 0) {
        SKYCDBG("%s err(%d)\n", __func__, rc);
        return rc;
    }
    
    CDBG("%s X\n", __func__);
    return 0;
}

static int32_t mt9d113_sensor_set_awb_lock(struct msm_sensor_ctrl_t *s_ctrl ,int8_t is_lock)
{
    si2c_pid_t pid = SI2C_PID_MAX;
    int rc = 0;

    CDBG("%s is_lock = %d\n",__func__, is_lock);
    
    switch (is_lock) {
    case 0: pid = SI2C_AWB_ON;break;
    case 1: pid = SI2C_AWB_OFF;break;
    default:
        SKYCDBG("%s err(-EINVAL)\n", __func__);
        return -EINVAL;
    }

    rc = si2c_write_param(SI2C_SA, pid, mt9d113_params);
    if (rc < 0) {
        SKYCDBG("%s err(%d)\n", __func__, rc);
        return rc;
    }
    
    CDBG("%s X\n", __func__);
    return 0;
}
#endif

#ifdef CONFIG_PANTECH_CAMERA_TUNER
static int mt9d113_set_tuner(struct tuner_cfg tuner)
{
	si2c_cmd_t *cmds = NULL;
	char *fbuf = NULL;

	CDBG("%s fbuf=%p, fsize=%d\n", __func__, tuner.fbuf, tuner.fsize);

	if (!tuner.fbuf || (tuner.fsize == 0)) {
		SKYCDBG("%s err(-EINVAL)\n", __func__);
		return -EINVAL;
	}

	fbuf = (char *)kmalloc(tuner.fsize, GFP_KERNEL);
	if (!fbuf) {
		SKYCDBG("%s err(-ENOMEM)\n", __func__);
		return -ENOMEM;
	}

	if (copy_from_user(fbuf, tuner.fbuf, tuner.fsize)) {
		SKYCDBG("%s err(-EFAULT)\n", __func__);
		kfree(fbuf);
		return -EFAULT;
	}

	cmds = ptune_parse("@init", fbuf);
	if (!cmds) {
		SKYCDBG("%s no @init\n", __func__);
		kfree(fbuf);
		return -EFAULT;
	}

	mt9d113_tuner_params[SI2C_INIT].cmds = cmds;
	mt9d113_params[SI2C_INIT].cmds = cmds;

	kfree(fbuf);

	CDBG("%s X\n", __func__);
	return 0;
}
#endif

#endif


static int __init msm_sensor_init_module(void)
{
          CDBG("%s E\n", __func__);
	return i2c_add_driver(&mt9d113_i2c_driver);
}

static struct v4l2_subdev_core_ops mt9d113_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops mt9d113_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops mt9d113_subdev_ops = {
	.core = &mt9d113_subdev_core_ops,
	.video  = &mt9d113_subdev_video_ops,
};

static struct msm_sensor_fn_t mt9d113_func_tbl = {
#if 1//def F_STREAM_ON_OFF
	.sensor_start_stream = mt9d113_sensor_start_stream,
	.sensor_stop_stream = mt9d113_sensor_stop_stream,
#endif
#if 0
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = msm_sensor_set_fps,
	.sensor_write_exp_gain = msm_sensor_write_exp_gain1,
	.sensor_write_snapshot_exp_gain = msm_sensor_write_exp_gain1,
#endif
	.sensor_setting = mt9d113_sensor_setting,//msm_sensor_setting,
	.sensor_csi_setting = mt9d113_sensor_setting1,//msm_sensor_setting1,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
#ifdef F_MT9D113_POWER
    .sensor_power_up = mt9d113_sensor_power_up,//msm_sensor_power_up,
    .sensor_power_down = mt9d113_sensor_power_down,//msm_sensor_power_down,
#else
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
#endif
//	.sensor_adjust_frame_lines = msm_sensor_adjust_frame_lines,
	.sensor_get_csi_params = msm_sensor_get_csi_params,
//mt9d113_set_function
#ifdef CONFIG_PANTECH_CAMERA
    .sensor_set_brightness = mt9d113_sensor_set_brightness,
    .sensor_set_effect = mt9d113_sensor_set_effect,
    .sensor_set_exposure_mode = mt9d113_sensor_set_exposure_mode,
    .sensor_set_wb = mt9d113_sensor_set_wb,
    .sensor_set_preview_fps = mt9d113_sensor_set_preview_fps,
    .sensor_set_reflect = mt9d113_sensor_set_reflect,
#if 1 //def F_PANTECH_CAMERA_FIX_CFG_AE_AWB_LOCK
    .sensor_set_aec_lock = mt9d113_sensor_set_aec_lock,
    .sensor_set_awb_lock = mt9d113_sensor_set_awb_lock,
#endif

#if 0//def CONFIG_PANTECH_CAMERA_TUNER
    .sensor_set_tuner = s5k6aafx13_set_tuner,
#endif        
#endif

};

static struct msm_sensor_reg_t mt9d113_regs = {
	.default_data_type = MSM_CAMERA_I2C_WORD_DATA, //MSM_CAMERA_I2C_BYTE_DATA,
#if 0//def F_STREAM_ON_OFF	        
	.start_stream_conf = mt9d113_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(mt9d113_start_settings),
	.stop_stream_conf = mt9d113_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(mt9d113_stop_settings),
#endif
#if 0
	.group_hold_on_conf = mt9d113_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(mt9d113_groupon_settings),
	.group_hold_off_conf = mt9d113_groupoff_settings,
	.group_hold_off_conf_size =
		ARRAY_SIZE(mt9d113_groupoff_settings),
#endif
	.init_settings = NULL,//&mt9d113_init_conf[0],
	.init_size = 0,//ARRAY_SIZE(mt9d113_init_conf),
	.mode_settings = NULL,//&mt9d113_confs[0],
	.output_settings = &mt9d113_dimensions[0],
	.num_conf = ARRAY_SIZE(mt9d113_cid_cfg),//ARRAY_SIZE(mt9d113_confs),
};

static struct msm_sensor_ctrl_t mt9d113_s_ctrl = {
	.msm_sensor_reg = &mt9d113_regs,
	.sensor_i2c_client = &mt9d113_sensor_i2c_client,
	.sensor_i2c_addr = 0x78,//0x34,
//	.sensor_output_reg_addr = &mt9d113_reg_addr,
	.sensor_id_info = &mt9d113_id_info,
//	.sensor_exp_gain_info = &mt9d113_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &mt9d113_csic_params_array[0],
	.csi_params = &mt9d113_csi_params_array[0],
	.msm_sensor_mutex = &mt9d113_mut,
	.sensor_i2c_driver = &mt9d113_i2c_driver,
	.sensor_v4l2_subdev_info = mt9d113_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(mt9d113_subdev_info),
	.sensor_v4l2_subdev_ops = &mt9d113_subdev_ops,
	.func_tbl = &mt9d113_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
};

late_initcall(msm_sensor_init_module);//module_init//late_initcall
MODULE_DESCRIPTION("Aptina 2MP SoC Sensor Driver");
MODULE_LICENSE("GPL v2");
