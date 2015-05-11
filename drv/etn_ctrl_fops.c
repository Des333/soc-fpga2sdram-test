/**
 * File operations for /dev/etn-ctrl char device.
 */

#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/poll.h>
#include <asm/io.h>

#include "etn_main.h"
#include "etn_regs.h"



static int etn_ctrl_open(struct inode *inode, struct file *filp)
{
	struct fpga_dev *fpga = 
		container_of(filp->private_data, struct fpga_dev, ctrl_dev);

	filp->private_data = fpga;

	return 0;
}


static int etn_ctrl_release(struct inode *inode, struct file *filp)
{
	return 0;
}


ssize_t etn_ctrl_read(struct file *filp, char __user *buf, 
                           size_t count, loff_t *f_pos)
{
	struct fpga_dev *fpga = filp->private_data;
        int ret;
        
        fpga->write_done = 0;

        fpga_fw_clear_bit(fpga, DMA_CTRL_CR, DMA_CTRL_CR_RUN_STB);
        fpga_fw_set_bit(fpga, DMA_CTRL_CR, DMA_CTRL_CR_RUN_STB);

        ret = wait_event_interruptible(fpga->data_queue, fpga->write_done == 1);
        if( ret ) {
                return ret;
        }


        return 0;
}

const struct file_operations etn_ctrl_dev_fops = {
	.owner   = THIS_MODULE,
	.open    = etn_ctrl_open,
	.release = etn_ctrl_release,
	.read    = etn_ctrl_read
};

