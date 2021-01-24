#ifndef __FPGA_DRIVER_H__
#define __FPGA_DRIVER_H__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/version.h>

#define IOM_DOT_MAJOR	262
#define IOM_DOT_ADDRESS	0x08000210
#define IOM_DOT_NAME	"dot"
#define IOM_DOT_MAX_ROW	10

#define IOM_TEXT_LCD_MAJOR	263
#define IOM_TEXT_LCD_ADDRESS	0x08000090
#define IOM_TEXT_LCD_NAME	"text_lcd"
#define IOM_TEXT_LCD_MAX_BUF	32

#define IOM_PUSH_SWITCH_MAJOR		265
#define IOM_PUSH_SWITCH_ADDRESS		0x08000050
#define IOM_PUSH_SWITCH_NAME		"push_switch"
#define IOM_PUSH_SWITCH_MAX_BUTTON	9

#define IOM_BUZZER_MAJOR	264
#define IOM_BUZZER_ADDRESS	0x08000070
#define IOM_BUZZER_NAME		"buzzer"

#define IOM_FND_MAJOR	261
#define IOM_FND_NAME	"fpga_fnd"
#define IOM_FND_ADDRESS	0x08000004

#endif