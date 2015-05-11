#include <linux/platform_device.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>

#include "etn_main.h"
#include "etn_regs.h"
#include "etn_dma.h"



int fpga_add_rx_buffer(struct fpga_dev *fpga, uint32_t size)
{
        void *addr;
        dma_addr_t dma_addr;

        addr = dmam_alloc_coherent(&fpga->pdev->dev, size, &dma_addr, GFP_KERNEL);

	if (unlikely(addr == NULL)) {
		printk("Failed to allocate RX buffer\n");
		return -ENOMEM;
	}

        printk( KERN_DEBUG "Add RX buff, addr=0x%p, dma_addr=0x%p\n", 
                            addr, (void *)dma_addr);

	fpga->buff.addr = addr;
        fpga->buff.dma_addr = dma_addr;
        fpga->buff.size = size;

	if (dma_mapping_error(&fpga->pdev->dev, fpga->buff.dma_addr)) {
		printk("Failed to map RX buffer\n");
		fpga->buff.addr = NULL;
		fpga->buff.dma_addr = 0;
		return -ENOMEM;
	}

        // For 128-bit FPGA2SDRAM interface.
        // We set address in FPGA2SDRAM in words.
        dma_addr = dma_addr / 16;
        size = size / 16;

        // Write DMA address to FPGA
        fpga_fw_write_reg(fpga, DMA_ADDR_CR0, dma_addr & 0xFFFF);
        fpga_fw_write_reg(fpga, DMA_ADDR_CR1, dma_addr >> 16);
        
        // Write size to FPGA
        fpga_fw_write_reg(fpga, DMA_SIZE_CR0, size & 0xFFFF);
        fpga_fw_write_reg(fpga, DMA_SIZE_CR1, size >> 16);

	return 0;
}


irqreturn_t fpga_isr(int irq, void *dev_id)
{
	struct fpga_dev *fpga = dev_id;

        printk( KERN_DEBUG "Get interrupt\n" );

        fpga->write_done = 1;

        wake_up_interruptible(&fpga->data_queue);
        
	return IRQ_HANDLED;
}




