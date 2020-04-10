# 高精度RTC rx8900 驱动软件包


<br>


## 1 介绍

rx8900 软件包是针对RT-Thread驱动框架实现的实时时钟驱动，遵循RT-Thread RTC框架，可以从芯片内置RTC无缝切换使用外置高精度rx8900 RTC。rx8900是爱普生公司一款i2c接口的高精度RTC芯片，集成32768Hz晶振和温度传感器，并能根据环境温度调整频率，具有供电范围灵活、待机电流低、走时精度高等优点。



### 1.1 支持功能

* 实时时钟
* 闹钟
* 片内温度读取



### 1.2 目录结构

| 名称     | 说明       |
| -------- | ---------- |
| docs | 文档目录 |
| inc |头文件目录 |
| src | 源代码目录 |
|LICENSE| 许可证文件 |
|SConscript|RT-Thread默认构建脚本|



### 1.3 许可证

rx8900 软件包遵循 Apache license v2.0 许可，详见 `LICENSE` 文件。


### 1.4 依赖
- RT-Thread 3.0+
- RT-Thread I2C设备驱动框架
- RT-Thread RTC设备驱动框架

<br>

## 2 获取 rx8900 软件包

使用 rx8900 package 需要在 RT-Thread 的包管理器中选择它，具体路径如下：

```
RT-Thread online packages --->
    peripheral libraries and drivers --->
        [*] extren rtc drivers  --->
            [*] ds3231：Extern RTC drivers fo rx8900 
                 Version (latest)  --->
```

然后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到 BSP 中。

<br>

## 3 使用 rx8900 软件包

### 3.1 初始化
首先需初始化rx8900 驱动，可以手动在初始化线程合适的地方调用<code>rt_hw_rx8900 _init()</code>初始化，也可以直接使用<code>INIT_DEVICE_EXPORT</code>通过RT-Thread的自动初始化机制来初始化注册。初始化成功后，会注册名称为“rtc”的驱动设备。

```c
INIT_DEVICE_EXPORT(rt_hw_rx8900_init);	/* 通过RTT的自动初始化机制初始化rx8900 */
```

> 注：
>
> 如果开启了芯片内部RTC，需先禁止内部RTC



### 3.2 使用方式

对于时间和闹钟的访问，使用RT-Thread标准RTC和闹钟接口访问。片内温度读取，则由软件包提供一个函数接口。



* **获取当前时间**

```
time_t time(time_t *t)
time_t now;      
now = time(RT_NULL);
```

* **设置时间**

```
 #define RT_DEVICE_CTRL_RTC_SET_TIME     0x11        /**< set time */
 rt_err_t rt_device_control(rt_device_t dev, int cmd, void *arg)；
```

* **获取闹钟时间**

```
 #define RT_DEVICE_CTRL_RTC_GET_ALARM    0x12        /**< get alarm */
 rt_err_t rt_device_control(rt_device_t dev, int cmd, void *arg)；
```

* **设置闹钟时间**

```
 #define RT_DEVICE_CTRL_RTC_SET_ALARM    0x13      /**< set alarm */
 rt_err_t rt_device_control(rt_device_t dev, int cmd, void *arg)；
```

> 注：
>
> 详细用法可以参考“/components/drivers/rtc.c”和“/components/drivers/alarm.c”源码。



* **提供一个获取片内温度接口**

```
float rx8900 _get_temperature(void)；
```



### 3.3 msh/finsh测试

* msh获取时间

```
msh >date
Sat Jan  1 00:00:04 2000
```

* msh设置时间

```
msh date 2020 04 10 17 41 10  
msh >date
Fri Apr 10 17:41:15 2020
```

* finsh获取时间

```
finsh >list_date()
Fri Apr 10 17:41:49 2020
```

* finsh设置时间

```
finsh >set_date(2020,5,1)
        0, 0x00000000
finsh >set_time(12,0,0)  
        0, 0x00000000
finsh >list_date()
Fri May  1 12:00:04 2020
```

* msh打印温度

```
msh >list_rx89_temp
rx8900 temperature: [27.5C] 
```

* finsh打印温度

```
finsh >list_rx89_temp()
rx8900 temperature: [27.5C]
```




<br>

## 4 注意事项

使用RT-Thread的RTC框架，RTC设备注册名称为“rtc”，注意需先屏蔽内置芯片RTC驱动。

<br>

## 5 联系方式

- 维护：[Acuity](https://github.com/Prry)
- 主页：<https://github.com/Prry/rtt-rx8900>      




