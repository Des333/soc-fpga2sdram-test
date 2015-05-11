#ifndef _ETN_H
#define _ETN_H

#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/fpga.h>
#include "etn_dma.h"

#define DRV_NAME "etn"

#ifndef DRV_VERSION
#define DRV_VERSION "0.0.1"
#endif


// Default size is 4M.
// TODO:
//   Move it to module params
#define RX_BUF_SIZE (4*1024*1024)

struct fpga_feature {
	int cr;
	int sr;
};

struct fpga_dev {
	struct platform_device *pdev;

	struct miscdevice ctrl_dev;
	struct miscdevice data_dev;

        wait_queue_head_t data_queue;

        ktime_t start;

        int write_done;

        int irq;

	struct fpga_rx_buff buff;

	void __iomem *H2F;  

};

extern const struct file_operations etn_ctrl_dev_fops;
extern const struct file_operations etn_data_dev_fops;

int etn_register(struct fpga_dev *fpga);
void etn_unregister(struct fpga_dev *fpga);

// Address mappings
#define   H2F_ADDR     0xC0000000
#define   H2F_LEN      0x4088


// FPGA to SDRAM port enables
#define   PORTRSTN_ADDR       0xFFC25080
#define   PORTRSTN_LEN        4

#define   ALL_PORT_EN         0x3fff

// Reset management registers (for now only brgmodbrst)
#define   RSTMGR_ADDR         0xFFD05000
#define   RSTMGR_LEN          0x24
#define   BRGMODRST_REG_OFF   0x1C

// BRGMODRST fields
#define   BRGMODRST_MASK       0x7 // 3 bits
#define   BRGMODRST_F2H_BIT    1 << 2
#define   BRGMODRST_LWH2F_BIT  1 << 1
#define   BRGMODRST_H2F_BIT    1 << 0

// L3 GPV registers (for now only remap)
#define   L3REGS_ADDR        0xFF800000
#define   L3REGS_LEN         0x100000
#define   L3REGS_REMAP_OFF   0x0  // Remap register
#define   L3REGS_SECUR_OFF   0x20 // Security settings register (lwhps2fpgaregs)

// remap fields
#define   L3REGS_REMAP_LWH2F_BIT  1 << 4
#define   L3REGS_REMAP_H2F_BIT    1 << 3


#endif // _ETN_H
