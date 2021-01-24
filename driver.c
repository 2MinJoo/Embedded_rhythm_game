#include "../include/project_driver.h"
#include "../include/dot_define.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MIN JOO");

static int iom_init(void);

//-------------------- BUZZER SETTING --------------------

static int buzzer_port_usage=0;
static unsigned char *iom_fpga_buzzer_addr;

static ssize_t iom_buzzer_read(struct file *file, char *buf, size_t count, loff_t *f_pos);
static ssize_t iom_buzzer_write(struct file *file, const char *buf, size_t count, loff_t *f_pos);
static int iom_buzzer_open(struct inode *inode, struct file *file);
static int iom_buzzer_release(struct inode *inode, struct file *file);

struct file_operations iom_buzzer_fops = 
{
	.owner	=	THIS_MODULE,
	.read	=	iom_buzzer_read,
	.write	=	iom_buzzer_write,
	.open	=	iom_buzzer_open,
	.release=	iom_buzzer_release
};

static int iom_buzzer_open(struct inode *inode, struct file *file) 
{
	if(buzzer_port_usage)
		return -EBUSY;
	buzzer_port_usage=1;
	return 0;
}

static int iom_buzzer_release(struct inode *inode, struct file *file) 
{
	buzzer_port_usage = 0;
	return 0;
}

static ssize_t iom_buzzer_read(struct file *file, char *buf, size_t count, loff_t *f_pos) 
{
	unsigned char value;
	unsigned short _s_value;
	_s_value = inw((unsigned int)iom_fpga_buzzer_addr);
	value = _s_value & 0xF;
	if (copy_to_user(buf, &value, 1)) return -EFAULT;
	return 1;
}

static ssize_t iom_buzzer_write(struct file *file, const char *buf, size_t count, loff_t *f_pos) 
{
	unsigned char value;
	unsigned short _s_value;

	if (copy_from_user(&value, buf, 1)) return -EFAULT;
	_s_value = value & 0xF;
	outw(_s_value, (unsigned int)iom_fpga_buzzer_addr);
	return 1;
}

//-------------------- DOT MATRIX SETTING --------------------

static int dot_port_usage=0;
static unsigned char *iom_fpga_dot_addr;

static ssize_t iom_dot_write(struct file *file, const char *buf, size_t line, loff_t *f_pos);
static ssize_t iom_dot_read(struct file *file, char *buf, size_t count, loff_t *f_pos);
static int iom_dot_open(struct inode *inode, struct file *file);
static int iom_dot_release(struct inode *inode, struct file *file);

struct file_operations iom_dot_fops = {
	.owner	=	THIS_MODULE,
	.open	=	iom_dot_open,
	.write	=	iom_dot_write,
	.read	=	iom_dot_read,
	.release=	iom_dot_release
};

static int iom_dot_open(struct inode *inode, struct file *file)
{
	if(dot_port_usage)
		return -EBUSY;
	dot_port_usage=1;
	return 0;
}

static int iom_dot_release(struct inode *inode, struct file *file)
{
	dot_port_usage = 0;
	return 0;
}

//to write full of dot matrix 
/*
static ssize_t iom_dot_write(struct file *file, const char *buf, size_t count, loff_t *f_pos)
{
	unsigned char value[IOM_DOT_MAX_ROW];
	unsigned short _s_value;
	int i;

	if(count >= IOM_DOT_MAX_ROW)
		count = IOM_DOT_MAX_ROW;

	if(copy_from_user(value, buf, count))
		return -EFAULT;

	for(i=0; i<count; i++)
	{
		_s_value = value[i] & 0x7F;
		outw(_s_value, (unsigned int)iom_fpga_dot_addr + i * 2);
	}
	return count;
}*/

//I use this. write only one line of dot matrix. 
static ssize_t iom_dot_write(struct file *file, const char *buf, size_t line, loff_t *f_pos)
{
	unsigned char value[1];
	unsigned short _s_value;

	if(copy_from_user(value, buf, sizeof(buf)))
		return -EFAULT;

	_s_value = value[0] & 0x7F;
	outw(_s_value, (unsigned int)iom_fpga_dot_addr + line * 2);

	return line;
}

//one line of dot matrix read.
static ssize_t iom_dot_read(struct file *file, char *buf, size_t line, loff_t *f_pos)
{
	unsigned char value[1];
	unsigned short _s_value;

	_s_value = inw((unsigned int)iom_fpga_dot_addr + line * 2);

	value[0] = _s_value & 0x7F;
	if(copy_to_user(buf, value, sizeof(value)))
		return -EFAULT;

	return line;
	
}

//-------------------- TEXT LCD SETTING --------------------

static int text_lcd_port_usage=0;
static unsigned char *iom_fpga_text_lcd_addr;

static ssize_t iom_text_lcd_write(struct file *file, const char *buf, size_t count, loff_t *f_pos);
static int iom_text_lcd_open(struct inode *inode, struct file *file);
static int iom_text_lcd_release(struct inode *inode, struct file *file);

struct file_operations iom_text_lcd_fops = {
	.owner	=	THIS_MODULE,
	.open	=	iom_text_lcd_open,
	.write	=	iom_text_lcd_write,
	.release=	iom_text_lcd_release
};

static int iom_text_lcd_open(struct inode *inode, struct file *file)
{
	if(text_lcd_port_usage)
		return -EBUSY;
	text_lcd_port_usage=1;
	return 0;
}

static int iom_text_lcd_release(struct inode *inode, struct file *file)
{
	text_lcd_port_usage = 0;
	return 0;
}

static ssize_t iom_text_lcd_write(struct file *file, const char *buf, size_t count, loff_t *f_pos)
{
	unsigned char value[IOM_TEXT_LCD_MAX_BUF+1];
	unsigned short _s_value;
	int i;

	if (count > IOM_TEXT_LCD_MAX_BUF)
		count = IOM_TEXT_LCD_MAX_BUF;
	if (copy_from_user(value, buf, count))
		return -EFAULT;
	value[count] = 0;
	printk("Text LCD driver: write(), length=%d / string=%s\n", count, value);

	for (i = 0; i < count; i += 2)
	{
		_s_value = (value[i] & 0xFF) << 8 | (value[i+1] & 0xFF);
		outw(_s_value, (unsigned int)iom_fpga_text_lcd_addr + i);
	}
	return count;
}

//-------------------- TEXT LCD SETTING --------------------

static int fnd_port_usage = 0;
static unsigned char *iom_fpga_fnd_addr;

static ssize_t iom_fnd_write(struct file *file, const char *buf, size_t count, loff_t *f_pos);
static ssize_t iom_fnd_read(struct file *file, char *buf, size_t count, loff_t *f_pos);
static int iom_fnd_open(struct inode *inode, struct file *file);
static int iom_fnd_release(struct inode *inode, struct file *file);

struct file_operations iom_fnd_fops = {
	.owner = THIS_MODULE,
	.open = iom_fnd_open,
	.write = iom_fnd_write,
	.read = iom_fnd_read,
	.release = iom_fnd_release
};

static int iom_fnd_open(struct inode *inode, struct file *file)
{
	if (fnd_port_usage)
		return -EBUSY;
	fnd_port_usage = 1;
	return 0;
}

static int iom_fnd_release(struct inode *inode, struct file *file)
{
	fnd_port_usage = 0;
	return 0;
}

static ssize_t iom_fnd_write(struct file *file, const char *buf, size_t count, loff_t
*f_pos)
{
	unsigned char value[4];
	unsigned short _s_value;
	if (count > 4)
		count = 4;
	if (copy_from_user(value, buf, 4))
		return -EFAULT;
	_s_value = value[0] << 12 | value[1] << 8 | value[2] << 4 | value[3];
	outw(_s_value, (unsigned int)iom_fpga_fnd_addr);

	return count;
}

static ssize_t iom_fnd_read(struct file *file, char *buf, size_t count, loff_t *f_pos)
{
	unsigned char value[4];
	unsigned short _s_value;
	_s_value = inw((unsigned int)iom_fpga_fnd_addr);
	value[0] = (_s_value >> 12) & 0xF;
	value[1] = (_s_value >> 8) & 0xF;
	value[2] = (_s_value >> 4) & 0xF;
	value[3] = (_s_value >> 0) & 0xF;
	if (copy_to_user(buf, value, 4))
		return -EFAULT;
	return count;
}

//-------------------- PUSH SWITCH SETTING --------------------

static int push_switch_port_usage=0;
static unsigned char *iom_fpga_push_switch_addr;

static int iom_push_switch_open(struct inode *inode, struct file *file);
static int iom_push_switch_release(struct inode *inode, struct file *file);
static ssize_t iom_push_switch_read(struct file *file, char *buf, size_t count, loff_t *f_pos);

struct file_operations iom_push_switch_fops = {
	.owner	=	THIS_MODULE,
	.open	=	iom_push_switch_open,
	.read	=	iom_push_switch_read,
	.release=	iom_push_switch_release
};

static int iom_push_switch_open(struct inode *inode, struct file *file)
{
	if(push_switch_port_usage)
		return -EBUSY;
	push_switch_port_usage=1;
	return 0;
}

static int iom_push_switch_release(struct inode *inode, struct file *file)
{
	push_switch_port_usage = 0;
	return 0;
}

static ssize_t iom_push_switch_read(struct file *file, char *buf, size_t count, loff_t *f_pos)
{
	unsigned char value[IOM_PUSH_SWITCH_MAX_BUTTON];
	unsigned short _s_value;
	int i;
	if (count > IOM_PUSH_SWITCH_MAX_BUTTON)
		count = IOM_PUSH_SWITCH_MAX_BUTTON;

	for (i = 0; i < count; i++)
	{
		_s_value = inw((unsigned int)iom_fpga_push_switch_addr + i * 2);
		value[i] = _s_value && 0xFF;
	}

	if(copy_to_user(buf, value, count))
		return -EFAULT;

	return count;
}

//-------------------- MODULE SETTING --------------------

static int iom_init(void) 
{
	int result = register_chrdev(IOM_BUZZER_MAJOR, IOM_BUZZER_NAME, &iom_buzzer_fops);
	if(result < 0) 
	{
		printk(KERN_WARNING "can't get buzzer number\n");
		return result;
	}
	result = register_chrdev(IOM_DOT_MAJOR, IOM_DOT_NAME, &iom_dot_fops);
	if(result < 0) 
	{
		printk(KERN_WARNING "can't get dot matrix number\n");
		return result;
	}
	result = register_chrdev(IOM_TEXT_LCD_MAJOR, IOM_TEXT_LCD_NAME, &iom_text_lcd_fops);
	if(result < 0) 
	{
		printk(KERN_WARNING "can't get text lcd number\n");
		return result;
	}
	result = register_chrdev(IOM_PUSH_SWITCH_MAJOR, IOM_PUSH_SWITCH_NAME, &iom_push_switch_fops);
	if(result < 0) 
	{
		printk(KERN_WARNING "can't get switch number\n");
		return result;
	}

	result = register_chrdev(IOM_FND_MAJOR, IOM_FND_NAME, &iom_fnd_fops);
	if (result < 0)
	{
		printk(KERN_WARNING "Can't get fnd number\n");
		return result;
	}
	iom_fpga_buzzer_addr = ioremap(IOM_BUZZER_ADDRESS, 0x1);
	iom_fpga_dot_addr = ioremap(IOM_DOT_ADDRESS, 0x14);
	iom_fpga_text_lcd_addr = ioremap(IOM_TEXT_LCD_ADDRESS, 0x20);
	iom_fpga_push_switch_addr = ioremap(IOM_PUSH_SWITCH_ADDRESS, 0x12);
	iom_fpga_fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4);
	return 0;
}

void __exit iom_exit(void)
{
	iounmap(iom_fpga_buzzer_addr);
	unregister_chrdev(IOM_BUZZER_MAJOR, IOM_BUZZER_NAME);	//BUZZER OFF

	iounmap(iom_fpga_dot_addr);
	unregister_chrdev(IOM_DOT_MAJOR, IOM_DOT_NAME);		//DOT MATRIX OFF

	iounmap(iom_fpga_text_lcd_addr);
	unregister_chrdev(IOM_TEXT_LCD_MAJOR, IOM_TEXT_LCD_NAME);		//TEXT LCD OFF

	iounmap(iom_fpga_push_switch_addr);
	unregister_chrdev(IOM_PUSH_SWITCH_MAJOR, IOM_PUSH_SWITCH_NAME); 	//SWITCH OFF

	iounmap(iom_fpga_fnd_addr);
	unregister_chrdev(IOM_FND_MAJOR, IOM_FND_NAME);		//FND OFF
}

module_init(iom_init);
module_exit(iom_exit);