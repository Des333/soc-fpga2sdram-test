#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/of_irq.h>
#include <linux/phy.h>

#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <asm/io.h>

#include "etn_main.h"
#include "etn_regs.h"

static int size = 4096;
module_param(size, int, S_IRUGO);


/**
 *  Map FPGA memory for H2F interface.
 *  
 *  @param fpga -  ETN device pointer
 *  @return 0 on success, -ENXIO on error.
 */
static int map_hw_mem(struct fpga_dev *fpga)
{
	if (!request_mem_region(H2F_ADDR, H2F_LEN, DRV_NAME)) {
		printk("Failed to request mem region for H2F interface\n");
		goto err;
	}

	fpga->H2F = ioremap(H2F_ADDR, H2F_LEN);
	if (!fpga->H2F) {
		printk("Failed to map H2F address space\n");
		goto err_release_h2f;
	}

	return 0;

err_release_h2f:
	if (fpga->H2F) {
		iounmap(fpga->H2F);
	}
	release_mem_region(H2F_ADDR, H2F_LEN);

err:
	return -ENXIO;
}

/** 
 * Initialize hardware interfaces.
 *
 * 1. Request memory mappings for configuration modules (RSTMGR, L3REGS)
 * 2. Disable reset in RSTMGR's brgmodrst register.
 * 3. Enable remapping in L3REGS's remap register.
 * 4. Request memory mappings for H2F interface.
 * 5. Disable reset for FPGA2SDRAM ports
 * 6. Unmap RSTMGR and L3REGS memory.
 *
 * @param fpga - fpga device descriptor
 * @return 0 on success, -ENXIO on error.
 */ 
static int init_hw_interface(struct fpga_dev *fpga)
{
	void __iomem *RSTMGR = NULL;
	void __iomem *L3REGS = NULL;
	void __iomem *PORTRSTN = NULL;
	uint32_t brgmodrst = 0;
	uint32_t test = 0;
	uint32_t remap = 0;
	int ret = 0;

	// ------------------------------------------
	// First, map configuration registers regions
	// ------------------------------------------
        /*
	if (!request_mem_region(RSTMGR_ADDR, RSTMGR_LEN, DRV_NAME)) {
		printk("Failed to request mem region for RSTMGR\n");
		ret = -ENXIO;
		goto err;
	}
        */

	RSTMGR = ioremap(RSTMGR_ADDR, RSTMGR_LEN);
	if (!RSTMGR) {
		printk("Failed to map RSTMGR address space \n");
		ret = -ENXIO;
		goto err_release_rstmgr;
	}

	if (!request_mem_region(L3REGS_ADDR, L3REGS_LEN, DRV_NAME)) {
		printk("Failed to request mem region for L3 registers\n");
		ret = -ENXIO;
		goto err_release_rstmgr;
	}

	L3REGS = ioremap(L3REGS_ADDR, L3REGS_LEN);
	if (!L3REGS) {
		printk("Failed to map L3REGS ddress space \n");
		ret = -ENXIO;
		goto err_release_l3regs;
	}


	if (!request_mem_region(PORTRSTN_ADDR, PORTRSTN_LEN, DRV_NAME)) {
		printk("Failed to request mem region for FPGA2SDRAM reset registers\n");
		ret = -ENXIO;
		goto err_release_portrstn;
	}

	PORTRSTN = ioremap(PORTRSTN_ADDR, PORTRSTN_LEN);
	if (!PORTRSTN) {
		printk("Failed to map PORTRSTN address space \n");
		ret = -ENXIO;
		goto err_release_portrstn;
	}


	// ---------------------------------------
	// Now do mapping related hw configuration
	// ---------------------------------------

	// 1. Disable reset
	brgmodrst = ~(BRGMODRST_H2F_BIT | BRGMODRST_LWH2F_BIT | BRGMODRST_F2H_BIT) & BRGMODRST_MASK;
	iowrite32(brgmodrst, RSTMGR + BRGMODRST_REG_OFF);

	// Check how it was written
	test = ioread32(RSTMGR + BRGMODRST_REG_OFF);
	if (test != brgmodrst) {
		printk("Failed to disable H2F, LWH2F and F2H reset.\n");
		printk("brgmodrst is 0x%4X, must be 0x%4X\n", test, brgmodrst);
		ret = -ENXIO;
		goto err_release_l3regs;
	}

	// 2. Enable remaping
	remap = L3REGS_REMAP_LWH2F_BIT | L3REGS_REMAP_H2F_BIT;
	iowrite32(remap, L3REGS + L3REGS_REMAP_OFF);
	// remap register is write-only, so we can't check it.


	// Map FPGA memory
	if (map_hw_mem(fpga)) {
		printk("Failed to map H2F and LWH2F memory\n");
		ret = -ENXIO;
	}

	// Disable reset for FPGA2SDRAM ports
	iowrite32(ALL_PORT_EN, PORTRSTN);
        
	// We don't need L3 regs, reset management and FPGA2SDRAM registers anymore
err_release_portrstn:
	if (PORTRSTN) {
		iounmap(PORTRSTN);
	}
	release_mem_region(PORTRSTN_ADDR, PORTRSTN_LEN);
       


err_release_l3regs:
	if (L3REGS) {
		iounmap(L3REGS);
	}
	release_mem_region(L3REGS_ADDR, L3REGS_LEN);

err_release_rstmgr:
	if (RSTMGR) {
		iounmap(RSTMGR);
	}
	//release_mem_region(RSTMGR_ADDR, RSTMGR_LEN);

	printk( KERN_DEBUG "init_hw_interface ret = %d\n", ret);
	return ret;
}

/**
 * Unmap memory requested for H2F and LWH2F interfaces
 */
static void unmap_hw_mem(struct fpga_dev *fpga)
{
	if (fpga->H2F) {
		iounmap(fpga->H2F);
		fpga->H2F = NULL;
		release_mem_region(H2F_ADDR, H2F_LEN);
	}
}

/**
 * Hardware interface deinitialization. Just unmaps memory
 */
static void deinit_hw_interface(struct fpga_dev *fpga)
{
	unmap_hw_mem(fpga);
}


static const struct miscdevice etn_ctrl_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = "etn-ctrl",
	.fops  = &etn_ctrl_dev_fops,
};


static const struct miscdevice etn_data_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = "etn-data",
	.fops  = &etn_data_dev_fops,
};




int etn_register(struct fpga_dev *fpga)
{
        int err = 0;

        BUG_ON(fpga == NULL);

        printk( KERN_DEBUG "FPGA is ready, starting register\n");

        err = fpga_add_rx_buffer(fpga, size);
        if (err) {
	       	printk("Failed to initialize RX buffer\n");
	        goto err;
        }
        

        fpga->ctrl_dev = etn_ctrl_dev;

        if ((err = misc_register(&fpga->ctrl_dev))) {
		printk("Cannot register control misc device\n");
		goto err;
	}
       

        fpga->data_dev = etn_data_dev;

        if ((err = misc_register(&fpga->data_dev))) {
		printk("Cannot register data misc device\n");
		goto err_ctrl_dev;
	}
       
        
        init_waitqueue_head(&fpga->data_queue);

        // Register interrupt handler
        err = request_irq(fpga->irq, fpga_isr, IRQF_SHARED, DRV_NAME, fpga);
        if (err) {
		printk("Cannot allocate interrupt %d\n", fpga->irq);
		goto err_data_dev;
	}
        
        printk( KERN_DEBUG "Register is successfully done\n");

	return 0;


err_data_dev:
	misc_deregister(&fpga->data_dev);
	fpga->data_dev.this_device = NULL;

err_ctrl_dev:
	misc_deregister(&fpga->ctrl_dev);
	fpga->ctrl_dev.this_device = NULL;

err:
	return err;
}


void etn_unregister(struct fpga_dev *fpga)
{
	if (fpga) {

                if (fpga->ctrl_dev.this_device) {
	                misc_deregister(&fpga->ctrl_dev);
                        fpga->ctrl_dev.this_device = NULL;
                }

                if (fpga->data_dev.this_device) {
	                misc_deregister(&fpga->data_dev);
                        fpga->data_dev.this_device = NULL;
                        
                        free_irq(fpga->irq, fpga);
                }
                                
	}
}


void etn_free(struct fpga_dev *fpga)
{
	if (fpga) {
		etn_unregister(fpga);

		deinit_hw_interface(fpga);

		kfree(fpga);
	}
}

int etn_remove(struct platform_device *pdev)
{
	struct fpga_dev *fpga;

	fpga = platform_get_drvdata(pdev);

	etn_free(fpga);
	platform_set_drvdata(pdev, NULL);

	return 0;      
}

#ifdef CONFIG_OF
static const struct of_device_id etn_of_match[] = {
	{ .compatible = "mtk,etn", },
	{},
};

MODULE_DEVICE_TABLE(of, etn_of_match);
#endif

static struct platform_driver etn_driver = {
	.remove = etn_remove,
	.driver = {
		.name	= "etn",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(etn_of_match),
	},
};


static u64 platform_dma_mask = DMA_BIT_MASK(24);



int etn_probe(struct platform_device *pdev)
{
        int err = 0;
	struct fpga_dev *fpga;

	fpga = kzalloc(sizeof(*fpga), GFP_KERNEL);
	if (!fpga)
		return -ENOMEM;

	// Init hardware interfaces
	if ((err = init_hw_interface(fpga))) {
		printk("Hardware interface configuration failed\n");
		goto err_free_fpgadev;
	}
        
	pdev->dev.dma_mask = &platform_dma_mask;

	fpga->pdev = pdev;
		
        fpga->irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
        printk( KERN_DEBUG "IRQ from dts: %u\n", fpga->irq );

	if ((err = etn_register(fpga))) {
		dev_err(&pdev->dev,"Cannot register devices\n");
		goto err_free_fpgadev;
	}

	// Save handle to our meaty structure
	platform_set_drvdata(pdev, fpga);
        
	return 0;

err_free_fpgadev:
	if (fpga)
		kfree(fpga);

	return err;
}


static int __init etn_init(void)
{
	if (platform_driver_probe(&etn_driver, etn_probe)) {
		printk("Failed to probe ETN platform driver\n");
		return -ENXIO;
	}

	return 0;
}

static void __exit etn_exit(void)
{
	platform_driver_unregister(&etn_driver);
}


MODULE_AUTHOR("Alex Dzyoba <a.dzyoba@metrotek.spb.ru>");
MODULE_AUTHOR("Denis Gabidullin <d.gabidullin@metrotek.spb.ru>");
MODULE_DESCRIPTION("Driver for testing FPGA2SDRAM interface");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);

module_init(etn_init);
module_exit(etn_exit);
