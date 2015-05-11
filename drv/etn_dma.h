#ifndef _ETN_DMA_H
#define _ETN_DMA_H


// Control registers
#define DMA_CTRL_CR        0
        #define DMA_CTRL_CR_RUN_STB      0

#define DMA_ADDR_CR0       1
#define DMA_ADDR_CR1       2

#define DMA_SIZE_CR0       3
#define DMA_SIZE_CR1       4

// Status registers
#define DMA_STAT_SR        0
        #define DMA_STAT_SR_BUSY         0

#define DMA_CYCLE_CNT_SR0  1
#define DMA_CYCLE_CNT_SR1  2




struct fpga_rx_buff {
	void *addr;
	dma_addr_t dma_addr;
        uint32_t size;
};

struct fpga_dev;

int fpga_add_rx_buffer(struct fpga_dev *fpga, uint32_t size);
irqreturn_t fpga_isr(int irq, void *dev_id);


#endif
