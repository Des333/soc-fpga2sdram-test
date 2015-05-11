/**
 * File operations for /dev/etn-data char device.
 */
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/poll.h>
#include <asm/io.h>

#include "etn_main.h"
#include "etn_regs.h"
#include "etn_dma.h"


static int etn_data_dev_open(struct inode *inode, struct file *filp)
{
	struct fpga_dev *fpga = 
		container_of(filp->private_data, struct fpga_dev, data_dev);

	filp->private_data = fpga;

	return 0;
}

static int etn_data_dev_release(struct inode *inode, struct file *filp)
{
	return 0;
}


ssize_t etn_data_dev_read(struct file *filp, char __user *buf, 
                 size_t count, loff_t *f_pos)
{
        struct fpga_dev *fpga = filp->private_data;

        struct fpga_rx_buff *rx_buff = &fpga->buff;
        uint32_t size = rx_buff->size;
        
        ssize_t ret = 0;
        
        if (*f_pos >= size)
                goto out;

        if (*f_pos + count > size)
                count = size - *f_pos;

        if (copy_to_user(buf, rx_buff->addr + *f_pos, count)) {
                ret = -EFAULT;
                goto out;
        }

        *f_pos += count;
        ret = count;

out:
	return ret;
}

ssize_t etn_data_dev_write(struct file *filp, const char __user *buf, 
                  size_t count, loff_t *f_pos)
{
	return -EINVAL;
}


loff_t etn_data_dev_lseek (struct file *filp, loff_t off, int whence)
{
        struct fpga_dev *fpga = filp->private_data;
	
        loff_t newpos;
        uint32_t size;

        size = fpga->buff.size;

        switch(whence) {

        case SEEK_SET:
                newpos = off;
                break;

        case SEEK_CUR:
                newpos = filp->f_pos + off;
                break;

        case SEEK_END:
                newpos = size + off;
                break;

        default: 
                return -EINVAL;
        }

        if (newpos < 0) 
                return -EINVAL;

        if (newpos > size)
                newpos = size;  

        filp->f_pos = newpos;

        return newpos;
}


const struct file_operations etn_data_dev_fops = {
	.owner   = THIS_MODULE,
	.open    = etn_data_dev_open,
	.release = etn_data_dev_release,
	.read    = etn_data_dev_read,
	.write   = etn_data_dev_write,
        .llseek  = etn_data_dev_lseek,
};

