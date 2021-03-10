/* Copyright (C) 2020 AdlinkTech, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/sysfs.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>


typedef enum _comMode {
	com_rs232 = 0,
	com_rs485,
	com_rs422,
	com_max	
}comMode;


struct roscube_mux_ctrl_data{
	struct platform_device *pdev;
	int gpio_com_mode[2];
	int current_com_mode;
	
};
//BSS138 used. input H , output L.
#define MODE_OUTPUT_HIGH 0
#define MODE_OUTPUT_LOW  1

struct roscube_mux_ctrl_data roscubedata;

/*	Truth Table
*  ----------------------
* | Mode0 | Mode1 | DB9    |
* -----------------------
* |    H     |    L     | RS232 |
* -----------------------
* |    L      |   H     | RS485 |
* -----------------------
* |    H     |    H     | RS422 |
* -----------------------
* 
*/

static const char *com_mode_select[] = {"rs232","rs485","rs422"};

enum _modeList{
	mode_232=0,
	mode_485,
	mode_422,
	mode_max
};

static void setMode(int modeIndex)
{
	if(modeIndex==com_rs485){
		// Set Mode_0 = L
		gpio_set_value(roscubedata.gpio_com_mode[0],MODE_OUTPUT_LOW);
	}else{
		// RS232/RS422
		gpio_set_value(roscubedata.gpio_com_mode[0],MODE_OUTPUT_HIGH);
	}

	if(modeIndex==com_rs232){
		// Set Mode_1 = L
		gpio_set_value(roscubedata.gpio_com_mode[1],MODE_OUTPUT_LOW);
	}else{
		gpio_set_value(roscubedata.gpio_com_mode[1],MODE_OUTPUT_HIGH);
	}

	
	printk("SP339 SET MODE : %s\n",com_mode_select[modeIndex]);
	roscubedata.current_com_mode = modeIndex;
	
}

static ssize_t roscube_com_mode_show(struct class *c,
                                struct class_attribute *attr,
                                char *data)
{	
	
	return sprintf(data,"%s",com_mode_select[roscubedata.current_com_mode]);
}

static ssize_t roscube_com_mode_set(struct class *cls,
                                struct class_attribute *attr,
                                const char *buf, size_t count)
{
	int ret=0;	
	int i,sLen;

	if(count < strlen(com_mode_select[1])){
		printk("error setting\n");
		return -EINVAL;
	}
	
	sLen = count;
	if(buf[count-1] == '\n'){
		sLen--;
	}
	
	for(i=0,ret=-1;i<com_max;i++){
		if((strlen(com_mode_select[i]) == sLen) && (memcmp(buf,com_mode_select[i],sLen) == 0)){			
			ret = 0;
			break;			
		}		
	}

	if(ret == 0){
		setMode(i);		
		printk("ROSCube : SP339 Set Mode to [%d]%s\n",i,com_mode_select[i]);		
	}else{
		printk("ROSCube : Invalid COM Mode --> %s\n",buf);
	}
				
	return (ret==0)?count:-EINVAL;
}

static ssize_t comMode_usage_show(struct class *c,
								struct class_attribute *attr,
								char *data)
{
	return sprintf(data,"usage :\n%s\n%s\n%s\n",com_mode_select[0],com_mode_select[1],com_mode_select[2]);
}


static struct class_attribute roscube_com_mode_ctrl_attr[] = {	
	
	/* 1. roscube com mode */
	__ATTR(uartMode, 0660 ,roscube_com_mode_show,roscube_com_mode_set),	
	__ATTR(usage,0440, comMode_usage_show ,NULL),
	__ATTR_NULL,
};

void roscube_com_mode_ctrl_release(struct class *cls)
{
	kfree(cls);
}

struct class roscube_ctrl_class = {
        .name =         "sp339_mode_ctl",
        .owner =        THIS_MODULE,
        .class_release = roscube_com_mode_ctrl_release,
        .class_attrs =  roscube_com_mode_ctrl_attr,
};

static const struct of_device_id roscube_uart_mode_ctrl_of_match[] = {
	{ .compatible = "adlink,sp339_mode_ctl", },
	{ },
};
MODULE_DEVICE_TABLE(of, roscube_uart_mode_ctrl_of_match);


static int roscube_mux_ctrl_probe(struct platform_device *pdev)
{	
	int gpio_cmode[2];	
	int ret;

	gpio_cmode[0] = of_get_named_gpio(pdev->dev.of_node,"sp339_mode_ctl",0);
	if(!gpio_is_valid(gpio_cmode[0])){
		printk("%s:GET ROSCube-X COM MODE1 ERROR\n",__FUNCTION__);	
		goto io_err;
	}else{
		gpio_direction_output(gpio_cmode[0],MODE_OUTPUT_LOW);
	}

	gpio_cmode[1] = of_get_named_gpio(pdev->dev.of_node,"sp339_mode_ctl",1);
	if(!gpio_is_valid(gpio_cmode[1])){
		printk("%s:GET ROSCube-X COM MODE2 ERROR\n",__FUNCTION__);		
		goto io_err;
	}else{
		gpio_direction_output(gpio_cmode[1],MODE_OUTPUT_LOW);
	}	
	
	class_register(&roscube_ctrl_class);
		
	roscubedata.pdev = pdev;
	roscubedata.gpio_com_mode[0] = gpio_cmode[0];
	roscubedata.gpio_com_mode[1] = gpio_cmode[1];

	// set default mode to RS232
	setMode(mode_232);
	
	printk("%s:ROSCube UART MUX probe done\n",__FUNCTION__);	
	
	return 0;

io_err:	
	
	return -EIO;
}

static int roscube_mux_ctrl_remove(struct platform_device *pdev)
{		
	return 0;
}


static struct platform_driver roscube_ctrl_driver = {

	.probe		= roscube_mux_ctrl_probe,
	.remove		= roscube_mux_ctrl_remove,
	.driver		= {
		.name	= "sp339_mode_ctl",
		.of_match_table = of_match_ptr(roscube_uart_mode_ctrl_of_match),
	}
};

static int __init roscube_uart_ctrl_init(void)
{	
	return platform_driver_register(&roscube_ctrl_driver);
}

static void __exit roscube_uart_ctrl_exit(void)
{		
	platform_driver_unregister(&roscube_ctrl_driver);
}

module_init(roscube_uart_ctrl_init);
module_exit(roscube_uart_ctrl_exit);

MODULE_AUTHOR("AdlinkTech, Inc.");
MODULE_DESCRIPTION("Adlinktech ROSCube-x RS232/RS485 control driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
MODULE_ALIAS("platform:sp339_mode_ctl");
