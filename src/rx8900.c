
/*
 * Copyright (c) 2020 panrui <https://github.com/Prry/rtt-rx8900>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-04-09     panrui      the first version
 */
 
#include <rtthread.h>
#include <rtdevice.h>
#include "rx8900.h"

#define PKG_USING_RX8900

#ifdef PKG_USING_RX8900

#define DBG_TAG "rx8900"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

/* rx8900 slave address */
#define RX8900_ARRD			0x32	

/* rx8900 time register */
#define	REG_SEC				0x00
#define	REG_MIN				0x01
#define	REG_HOUR			0x02
#define	REG_WEEK			0x03
#define	REG_DAY				0x04
#define	REG_MON				0x05
#define	REG_YEAR			0x06

/* rx8900 alarm register */
#define REG_ALM_MIN 	  	0x08
#define REG_ALM_HOUR     	0x09
#define REG_ALM_DAY_DATE 	0x0A

/* rx8900 control register */
#define REG_TIME_CNT0  		0x0B
#define REG_TIME_CNT1     	0x0C
#define RGE_EXT				0x0D
#define REG_FLAG			0x0E
#define REG_CTRL			0x0F
#define REG_TEMP 			0x17

/* rx8900 flag register bit positions */
#define VALUE_FLAG_VDET 	(1 << 0)
#define VALUE_FLAG_VLF 		(1 << 1)
#define VALUE_FLAG_AF 		(1 << 3)
#define VALUE_FLAG_TF 		(1 << 4)
#define VALUE_FLAG_UF 		(1 << 5)

/* rx8900 ctrl register bit positions */
#define VALUE_CTRL_RESET 	(1 << 0)
#define VALUE_CTRL_AIE 		(1 << 3)
#define VALUE_CTRL_TIE 		(1 << 4)
#define VALUE_CTRL_UIE 		(1 << 5)
#define VALUE_CTRL_CSEL0 	(1 << 6)
#define VALUE_CTRL_CSEL1	(1 << 7)

/* rx8900 ext register bit positions */
#define VALUE_EXT_TSEL0		(1 << 0)
#define VALUE_EXT_TSEL1		(1 << 1)
#define VALUE_EXT_FSEL0		(1 << 2)
#define VALUE_EXT_FSEL1		(1 << 3) 
#define VALUE_EXT_TE 		(1 << 4)
#define VALUE_EXT_USEL		(1 << 5) 
#define VALUE_EXT_WADA		(1 << 6)
#define VALUE_EXT_TEST		(1 << 7)

/* rx8900 alarm select */
#define ALM_ENABLE(value)	((~0x80)&value)	/* active when bit7 is low */
#define ALM_DISABLE(value)	(0x80 | value)

#define RX8900_I2C_BUS		"i2c1"		/* i2c linked */
#define	RX8900_DEVICE_NAME	"rtc"		/* register device name */

static struct rt_device rx8900_dev;		/* rx8900 device */

static unsigned char bcd_to_hex(unsigned char data)
{
    unsigned char temp;

    temp = ((data>>4)*10 + (data&0x0f));
    return temp;
}

static unsigned char hex_to_bcd(unsigned char data)
{
    unsigned char temp;

    temp = (((data/10)<<4) + (data%10));
    return temp;
}

static int get_week_day(unsigned char reg_week_day)
{
	int i, tm_wday = -1;
	
	for ( i=0; i < 7; i++)
	{
		if (reg_week_day & 0x01)
		{
			tm_wday = i;
			break;
		}
		reg_week_day >>= 1;
	}
	
	return 	tm_wday;
}

static rt_err_t  rx8900_read_reg(rt_device_t dev, rt_uint8_t reg,rt_uint8_t *data,rt_uint8_t data_size)
{
    struct rt_i2c_msg msg[2];
	struct rt_i2c_bus_device *i2c_bus = RT_NULL;
	
	RT_ASSERT(dev != RT_NULL);
	 
	i2c_bus = (struct rt_i2c_bus_device*)dev->user_data;
    msg[0].addr  = RX8900_ARRD;
    msg[0].flags = RT_I2C_WR;
    msg[0].len   = 1;
    msg[0].buf   = &reg;
    msg[1].addr  = RX8900_ARRD;
    msg[1].flags = RT_I2C_RD;
    msg[1].len   = data_size;
    msg[1].buf   = data;

    if(rt_i2c_transfer(i2c_bus, msg, 2) == 2)
	{
        return RT_EOK;
    }
    else
    {
	  	LOG_E("i2c bus read failed!\r\n");
        return -RT_ERROR;
    }
}

static rt_err_t  rx8900_write_reg(rt_device_t dev, rt_uint8_t reg, rt_uint8_t *data, rt_uint8_t data_size)
{
    struct rt_i2c_msg msg[2];
	struct rt_i2c_bus_device *i2c_bus = RT_NULL;
	
	RT_ASSERT(dev != RT_NULL);
	
	i2c_bus = (struct rt_i2c_bus_device*)dev->user_data;
    msg[0].addr		= RX8900_ARRD;
    msg[0].flags	= RT_I2C_WR;
    msg[0].len   	= 1;
    msg[0].buf   	= &reg;
    msg[1].addr  	= RX8900_ARRD;
    msg[1].flags	= RT_I2C_WR | RT_I2C_NO_START;
    msg[1].len   	= data_size;
    msg[1].buf   	= data;
    if(rt_i2c_transfer(i2c_bus, msg, 2) == 2)
	{
        return RT_EOK;
    }
    else
    {
	  	LOG_E("i2c bus write failed!\r\n");
        return -RT_ERROR;
    }
}

static rt_err_t rt_rx8900_open(rt_device_t dev, rt_uint16_t flag)
{
    if (dev->rx_indicate != RT_NULL)
    {
        /* open interrupt */
    }

    return RT_EOK;
}

static rt_size_t rt_rx8900_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    return RT_EOK;
}

static rt_err_t rt_rx8900_control(rt_device_t dev, int cmd, void *args)
{
	rt_err_t	ret = RT_EOK;
    time_t 		*time;
    struct tm 	time_temp;	
    rt_uint8_t 	buff[7];
	
    RT_ASSERT(dev != RT_NULL);
    rt_memset(&time_temp, 0, sizeof(struct tm));

    switch (cmd)
    {
    	/* get time */
        case RT_DEVICE_CTRL_RTC_GET_TIME:
	        time = (time_t *)args;
	        ret = rx8900_read_reg(dev, REG_SEC, buff, 7);
			if(ret == RT_EOK)
			{
				time_temp.tm_year  = bcd_to_hex(buff[6]) + 2000 - 1900;
				time_temp.tm_mon   = bcd_to_hex(buff[5]&0x1f) - 1;
				time_temp.tm_mday  = bcd_to_hex(buff[4]&0x3f);
				time_temp.tm_hour  = bcd_to_hex(buff[2]&0x3f);
				time_temp.tm_min   = bcd_to_hex(buff[1]&0x7f);
				time_temp.tm_sec   = bcd_to_hex(buff[0]&0x7f);
	        	*time = mktime(&time_temp);
			}
        break;

		/* set time */
        case RT_DEVICE_CTRL_RTC_SET_TIME:
        {
        	struct tm *time_new;
					
            time = (time_t *)args;
            time_new = localtime(time);
            buff[6] = hex_to_bcd(time_new->tm_year + 1900 - 2000);
            buff[5] = hex_to_bcd(time_new->tm_mon + 1);
            buff[4] = hex_to_bcd(time_new->tm_mday);
            buff[3] = hex_to_bcd(time_new->tm_wday+1);
            buff[2] = hex_to_bcd(time_new->tm_hour);
            buff[1] = hex_to_bcd(time_new->tm_min);
            buff[0] = hex_to_bcd(time_new->tm_sec);
            ret = rx8900_write_reg(dev, REG_SEC, buff, 7);
        }
        break;
	#ifdef RT_USING_ALARM
		/* get alarm time */
		case RT_DEVICE_CTRL_RTC_GET_ALARM:
		{ 	
		  	struct rt_rtc_wkalarm *alm_time;
			rt_uint8_t state[3] = {0x00};		
			
		  	ret = rx8900_read_reg(dev, REG_ALM_MIN, buff, 2);
			if(ret != RT_EOK)
			{
				return -RT_ERROR;
			}
			alm_time = (struct rt_rtc_wkalarm *)args;
			alm_time->tm_hour  = bcd_to_hex(buff[1]&0x7f);	
			alm_time->tm_min   = bcd_to_hex(buff[0]&0x7f);
			alm_time->tm_sec   = 0;	/* hardware alarm precision is 1 minute */
			
			ret = rx8900_read_reg(dev, RGE_EXT, state, 3);
			if(ret != RT_EOK)
			{
				return -RT_ERROR;
			}
			if (state[0] & VALUE_EXT_WADA)
			{/* day alarm */
				/* todo, RTT not supported temporarily */	
			}
			else
			{/* week day alarm */
				/* todo, RTT not supported temporarily */
			}
		}
		break;
		
		/* set alarm time */
		case RT_DEVICE_CTRL_RTC_SET_ALARM:
		{
			struct rt_rtc_wkalarm *alm_time;
			rt_uint8_t ctrl[2] = {0x00};
			
            alm_time = (struct rt_rtc_wkalarm *)args;
            buff[2] = ALM_DISABLE(0x00);	/* week and day alarm disable */	
            buff[1] = hex_to_bcd(ALM_ENABLE(alm_time->tm_hour));/* enable, alarm when hours and minutes match */
            buff[0] = hex_to_bcd(ALM_ENABLE(alm_time->tm_min));
			
			ret = rx8900_read_reg(dev, REG_FLAG, ctrl, 2);
			if (ret != RT_EOK)
			{
				return -RT_ERROR;
			}
			if (ctrl[0]&VALUE_EXT_WADA)
			{/* day alarm */
			  	/* todo, RTT not supported temporarily */
			}
			else
			{/* week day alarm */
				/* todo, RTT not supported temporarily */
			}
	
			/* write to hardware */
			ret = rx8900_write_reg(dev, REG_ALM_MIN, buff, 3);
			if (ret != RT_EOK)
			{
				return -RT_ERROR;
			}
			
			/* clear alarm flag */
			ctrl[1] &= ~VALUE_FLAG_AF;
			ret = rx8900_write_reg(dev, REG_FLAG, &ctrl[1], 1);
		}
		break;
	#endif
        default:
        break;
	}
    return ret;
}

/*
 *  @brief  Read temperature from rx8900
 *  @return Celsius temperature,-55~+125C,0.1C     
 */
float rx8900_get_temperature(void)
{
 	rt_uint8_t value = 0;
	float temp = 0.0f;
	
	rx8900_read_reg(&rx8900_dev, REG_TEMP, (rt_uint8_t*)&value, 1);
	temp = (value*2-187.19)/3.218;
	
	return temp;
}

/*
 *  @brief  Init rx8900 and register into RT-Thread device
 *  @return 'RT_EOK' when  rx8900 init completed successfully, 
 *          '-RT_ERROR' when something went wrong.
 */
int rt_hw_rx8900_init(void)
{	
    struct rt_i2c_bus_device *i2c_bus =RT_NULL;
    rt_uint8_t flags = 0;
	rt_bool_t  need_clear = RT_FALSE;
	rt_bool_t  need_reset = RT_FALSE;
	
    i2c_bus = rt_i2c_bus_device_find(RX8900_I2C_BUS);
    if (i2c_bus == RT_NULL)
    {
        LOG_E("i2c bus device %s not found!\r\n", RX8900_I2C_BUS);
        return -RT_ERROR;
    }				 	
	
    /* register rtc device */
    rx8900_dev.type   	= RT_Device_Class_RTC;
    rx8900_dev.init    	= RT_NULL;
    rx8900_dev.open    	= rt_rx8900_open;
    rx8900_dev.close   	= RT_NULL;
    rx8900_dev.read   	= rt_rx8900_read;
    rx8900_dev.write  	= RT_NULL;
    rx8900_dev.control 	= rt_rx8900_control;
    rx8900_dev.user_data= (void*)i2c_bus;	/* save i2cbus */
    rt_device_register(&rx8900_dev, RX8900_DEVICE_NAME, RT_DEVICE_FLAG_RDWR);
		
    /* init rx8900 */
	if (rx8900_read_reg(&rx8900_dev, REG_FLAG, &flags, 1))
	{
		return -RT_ERROR;
	}
	if (flags & VALUE_FLAG_VLF)
	{
	  	LOG_D("Data loss is detected. All registers must be initialized.\n");
		need_reset = RT_TRUE;
		need_clear = RT_TRUE;
	}
		
	if (flags & VALUE_FLAG_VDET)
	{
	  	LOG_D("Temperature compensation stop detected.\n");
		need_clear = RT_TRUE;
	}
	
	/* alarm */
	if (flags & VALUE_FLAG_AF) 
	{ 
		LOG_D("Alarm was detected.\n");
		need_clear = RT_TRUE;
	}
	
	/* tmer */
	if (flags & VALUE_FLAG_TF) 
	{  
		LOG_D("Timer was detected.\n");
		need_clear = RT_TRUE;
	}
	
	/* time update */
	if (flags & VALUE_FLAG_UF) 
	{ 
		flags &= ~VALUE_FLAG_UF; 
		LOG_D("Update was detected.\n");
		need_clear = RT_TRUE;
	}
	
	/* init register */
	if(need_reset == RT_TRUE)
	{
	  	flags = VALUE_CTRL_CSEL0;
		if (rx8900_write_reg(&rx8900_dev, REG_CTRL, &flags, 1) != RT_EOK)	/* clear ctrl register */
		{
			return -RT_ERROR;
		}
	
		flags = 0x00;
		if (rx8900_write_reg(&rx8900_dev, RGE_EXT, &flags, 1) != RT_EOK)	/* set second update */
		{
			return -RT_ERROR;
		}
	}
	
	if (need_clear == RT_TRUE)
	{
		flags = 0x00;
		if (rx8900_write_reg(&rx8900_dev, REG_FLAG, &flags, 1) != RT_EOK)	/* clear flag register */
		{
			return -RT_ERROR;
		}	
	}
	
	LOG_D("the rtc of rx8900 init succeed!\r\n");
	
    return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_hw_rx8900_init);

#ifdef RT_USING_FINSH
#include <finsh.h>

void list_rx89_temp(void)
{
	float temp = 0.0f;
	
	temp = rx8900_get_temperature();
	
	rt_kprintf("rx8900 temperature: [%d.%dC] \n", (int)temp, (int)(temp * 10) % 10);
}
FINSH_FUNCTION_EXPORT(list_rx89_temp, list rx8900 temperature.)
#endif /* RT_USING_FINSH */

#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
MSH_CMD_EXPORT(list_rx89_temp, list rx8900 temperature.);
#endif /* RT_USING_FINSH & FINSH_USING_MSH */

#endif /* PKG_USING_RX8900 */