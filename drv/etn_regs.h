
#ifndef _ETN_REGS_H
#define _ETN_REGS_H


void fpga_write_flush(struct fpga_dev *fpga);

void fpga_fw_write_reg(struct fpga_dev *fpga, int reg, u16 val);
u16 fpga_fw_read_reg(struct fpga_dev *fpga, int reg);

int fpga_fw_wait(struct fpga_dev *fpga, int reg, int bit, int sense);
int fpga_fw_test_bit(struct fpga_dev *fpga, int reg, int bit);

void fpga_fw_set_bit(struct fpga_dev *fpga, int reg, int bit);
void fpga_fw_clear_bit(struct fpga_dev *fpga, int reg, int bit);

void fpga_fw_write_pos(struct fpga_dev *fpga, int reg, int msb, int lsb, u16 val);

u16 fpga_fw_read_pos(struct fpga_dev *fpga, int reg, int msb, int lsb);




#endif

