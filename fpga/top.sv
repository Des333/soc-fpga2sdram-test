
module top(
		output wire [14:0] memory_mem_a,                
		output wire [2:0]  memory_mem_ba,               
		output wire        memory_mem_ck,               
		output wire        memory_mem_ck_n,             
		output wire        memory_mem_cke,              
		output wire        memory_mem_cs_n,             
		output wire        memory_mem_ras_n,            
		output wire        memory_mem_cas_n,            
		output wire        memory_mem_we_n,             
		output wire        memory_mem_reset_n,          
		inout  wire [31:0] memory_mem_dq,               
		inout  wire [3:0]  memory_mem_dqs,              
		inout  wire [3:0]  memory_mem_dqs_n,            
		output wire        memory_mem_odt,              
		output wire [3:0]  memory_mem_dm,               
		input  wire        memory_oct_rzqin,            
		inout  wire        hps_io_hps_io_sdio_inst_CMD, 
		inout  wire        hps_io_hps_io_sdio_inst_D0,  
		inout  wire        hps_io_hps_io_sdio_inst_D1,  
		output wire        hps_io_hps_io_sdio_inst_CLK, 
		inout  wire        hps_io_hps_io_sdio_inst_D2,  
		inout  wire        hps_io_hps_io_sdio_inst_D3,  
		input  wire        hps_io_hps_io_uart0_inst_RX, 
		output wire        hps_io_hps_io_uart0_inst_TX  
);


//***********************************************************
// Local parameters & defines
//***********************************************************
localparam CR_CNT = 5;
localparam SR_CNT = 3;
localparam DATA_W = 16;
localparam ADDR_W = 10;


// Control registers
`define DMA_CTRL_CR        0
        `define DMA_CTRL_CR_RUN_STB      0

`define DMA_ADDR_CR0       1
`define DMA_ADDR_CR1       2


`define DMA_SIZE_CR0       3
`define DMA_SIZE_CR1       4

// Status registers
`define DMA_STAT_SR        0
        `define DMA_STAT_SR_BUSY         0

`define DMA_CYCLE_CNT_SR0  1
`define DMA_CYCLE_CNT_SR1  2


//***********************************************************
// Signals
//***********************************************************
logic               clk_w;
logic [31:0]        irq0_w;

logic               reg64_waitrequest;
logic [63:0]        reg64_readdata;
logic               reg64_readdatavalid;
logic [0:0]         reg64_burstcount;
logic [63:0]        reg64_writedata;
logic [9:0]         reg64_address;
logic               reg64_write;
logic               reg64_read;
logic [7:0]         reg64_byteenable;
logic               reg64_debugaccess;


logic               reg16_write;
logic               reg16_read;
logic [11:0]        reg16_address;
logic [15:0]        reg16_writedata;
logic [1:0]         reg16_byteenable;
logic [15:0]        reg16_readdata;
logic               reg16_readdatavalid;
logic               reg16_waitrequest;


logic [27:0]        sdram0_address;
logic [7:0]         sdram0_burstcount;
logic               sdram0_waitrequest;
logic [127:0]       sdram0_readdata;
logic               sdram0_readdatavalid;
logic               sdram0_read;
logic [127:0]       sdram0_writedata;
logic [15:0]        sdram0_byteenable;
logic               sdram0_write;


//***********************************************************
// SOC instance
//***********************************************************

soc soc(
  .memory_mem_a                           ( memory_mem_a                     ),
  .memory_mem_ba                          ( memory_mem_ba                    ),
  .memory_mem_ck                          ( memory_mem_ck                    ),
  .memory_mem_ck_n                        ( memory_mem_ck_n                  ),
  .memory_mem_cke                         ( memory_mem_cke                   ),
  .memory_mem_cs_n                        ( memory_mem_cs_n                  ),
  .memory_mem_ras_n                       ( memory_mem_ras_n                 ),
  .memory_mem_cas_n                       ( memory_mem_cas_n                 ),
  .memory_mem_we_n                        ( memory_mem_we_n                  ),
  .memory_mem_reset_n                     ( memory_mem_reset_n               ),
  .memory_mem_dq                          ( memory_mem_dq                    ),
  .memory_mem_dqs                         ( memory_mem_dqs                   ),
  .memory_mem_dqs_n                       ( memory_mem_dqs_n                 ),
  .memory_mem_odt                         ( memory_mem_odt                   ),
  .memory_mem_dm                          ( memory_mem_dm                    ),
  .memory_oct_rzqin                       ( memory_oct_rzqin                 ),
  .hps_io_hps_io_sdio_inst_CMD            ( hps_io_hps_io_sdio_inst_CMD      ),
  .hps_io_hps_io_sdio_inst_D0             ( hps_io_hps_io_sdio_inst_D0       ),
  .hps_io_hps_io_sdio_inst_D1             ( hps_io_hps_io_sdio_inst_D1       ),
  .hps_io_hps_io_sdio_inst_CLK            ( hps_io_hps_io_sdio_inst_CLK      ),
  .hps_io_hps_io_sdio_inst_D2             ( hps_io_hps_io_sdio_inst_D2       ),
  .hps_io_hps_io_sdio_inst_D3             ( hps_io_hps_io_sdio_inst_D3       ),
  .hps_io_hps_io_uart0_inst_RX            ( hps_io_hps_io_uart0_inst_RX      ),
  .hps_io_hps_io_uart0_inst_TX            ( hps_io_hps_io_uart0_inst_TX      ),
  .reg_waitrequest                        ( reg64_waitrequest                ),
  .reg_readdata                           ( reg64_readdata                   ),
  .reg_readdatavalid                      ( reg64_readdatavalid              ),
  .reg_burstcount                         ( reg64_burstcount                 ),
  .reg_writedata                          ( reg64_writedata                  ),
  .reg_address                            ( reg64_address                    ),
  .reg_write                              ( reg64_write                      ),
  .reg_read                               ( reg64_read                       ),
  .reg_byteenable                         ( reg64_byteenable                 ),
  .reg_debugaccess                        ( reg64_debugaccess                ),
  .sdram0_address                         ( sdram0_address                   ),
  .sdram0_burstcount                      ( sdram0_burstcount                ),
  .sdram0_waitrequest                     ( sdram0_waitrequest               ),
  .sdram0_readdata                        ( sdram0_readdata                  ),
  .sdram0_readdatavalid                   ( sdram0_readdatavalid             ),
  .sdram0_read                            ( sdram0_read                      ),
  .sdram0_writedata                       ( sdram0_writedata                 ),
  .sdram0_byteenable                      ( sdram0_byteenable                ),
  .sdram0_write                           ( sdram0_write                     ),
  .irq0_irq                               ( irq0_w                           ),
  .clk_clk                                ( clk_w                            )
);


avalon_width_adapter avalon_width_adapter_regs(
  .clk_i                                  ( clk_w                            ),
  .rst_i                                  ( 1'b0                             ),

  // Wide IF
  .wide_writedata_i                       ( reg64_writedata                  ),
  .wide_byteenable_i                      ( reg64_byteenable                 ),
  .wide_write_i                           ( reg64_write                      ),
  .wide_read_i                            ( reg64_read                       ),
  .wide_address_i                         ( reg64_address                    ),

  .wide_readdata_o                        ( reg64_readdata                   ),
  .wide_waitrequest_o                     ( reg64_waitrequest                ),
  .wide_datavalid_o                       ( reg64_readdatavalid              ),


  // Narrow IF
  .narrow_writedata_o                     ( reg16_writedata                  ),
  .narrow_byteenable_o                    ( reg16_byteenable                 ),
  .narrow_write_o                         ( reg16_write                      ),
  .narrow_read_o                          ( reg16_read                       ),
  .narrow_address_o                       ( reg16_address                    ),

  .narrow_readdata_i                      ( reg16_readdata                   ),
  .narrow_datavalid_i                     ( reg16_readdatavalid              ),
  .narrow_waitrequest_i                   ( reg16_waitrequest                )
);

// We count registers in items.
defparam avalon_width_adapter_regs.SLAVE_ADDR_IS_BYTE = 0;

// 64 / 16 = 4
defparam avalon_width_adapter_regs.WIDTH_RATIO    = 4;
defparam avalon_width_adapter_regs.NARROW_IF_BE_W = 2;
defparam avalon_width_adapter_regs.WIDE_IF_ADDR_W = 10;




//***********************************************************
// Control & Status Registers
//***********************************************************

logic [DATA_W-1:0] cregs_w [CR_CNT-1:0];
logic [DATA_W-1:0] sregs_w [SR_CNT-1:0];

regfile_with_be #( 
  .CTRL_CNT                               ( CR_CNT                           ), 
  .STAT_CNT                               ( SR_CNT                           ), 
  .ADDR_W                                 ( ADDR_W                           ), 
  .DATA_W                                 ( DATA_W                           ), 
  .SEL_SR_BY_MSB                          ( 0                                )
) fpga_id_regfile (
  .clk_i                                  ( clk_w                            ),
  .rst_i                                  ( 1'b0                             ),

  .data_i                                 ( reg16_writedata                  ),
  .wren_i                                 ( reg16_write                      ),
  .addr_i                                 ( reg16_address                    ),
  .be_i                                   ( reg16_byteenable                 ),
  .sreg_i                                 ( sregs_w                          ),
  .data_o                                 ( reg16_readdata                   ),
  .creg_o                                 ( cregs_w                          )
); 

// Reading from registers have 0 cycles delay
assign reg16_readdatavalid = reg16_read; 

// This iface never blocks
assign reg16_waitrequest = 1'b0;


//***********************************************************
// SDRAM interface
//***********************************************************

logic run_test;
logic run_test_stb;

logic [31:0] dma_data_size;
logic [31:0] dma_addr;


// Control from CPU -- bit for start, DMA buffer address and transaction size.
assign run_test       = cregs_w[`DMA_CTRL_CR][`DMA_CTRL_CR_RUN_STB];
assign dma_addr       = { cregs_w[`DMA_ADDR_CR1], cregs_w[`DMA_ADDR_CR0] };
assign dma_data_size  = { cregs_w[`DMA_SIZE_CR1], cregs_w[`DMA_SIZE_CR0] };

// Status for CPU -- current state and overall cycles count.
assign sregs_w[`DMA_STAT_SR][`DMA_STAT_SR_BUSY] = test_is_running;
assign { sregs_w[`DMA_CYCLE_CNT_SR1], sregs_w[`DMA_CYCLE_CNT_SR0] } = cycle_cnt;

// Get strob from level
sedge_sel_sv run_test_sel(
  .Clk                         ( clk_w         ),
  .ain                         ( run_test      ),
  .edg                         ( run_test_stb  )
);

// Current state -- '1' if transaction in progress, '0' otherwise
logic test_is_running;


// Set 1 when CPU start transaction, set 0 when transaction finished
always_ff @( posedge clk_w )
  if( run_test_stb )
    test_is_running <= 1'b1;
  else
    if( test_finished )
      test_is_running <= 1'b0;


// For emulate data
logic [63:0] data_cnt;

// Current address on SDRAM iface
logic [31:0] addr_cnt;

// Overall cycles count. 
logic [31:0] cycle_cnt;


always_ff @( posedge clk_w )
  if( run_test_stb )
    cycle_cnt <= '0;
  else
    if( test_is_running )
      cycle_cnt <= cycle_cnt + 1;


// Form pseudo-data 
always_ff @( posedge clk_w )
  if( !test_is_running )
    data_cnt <= '0;
  else
    if( !sdram0_waitrequest )
      if( data_cnt != ( dma_data_size - 1 ) )
        data_cnt <= data_cnt + 1;


// Increase address if no waitrequest
always_ff @( posedge clk_w )
  if( run_test_stb )
    addr_cnt <= dma_addr;
  else
    if( !sdram0_waitrequest )
      addr_cnt <= addr_cnt + 1;


logic test_finished;

assign test_finished = ( data_cnt == ( dma_data_size - 1 ) ) && !sdram0_waitrequest;

// Rise interrupt to CPU
assign irq0_w[0] = test_finished;
   

// SDRAM IF
assign sdram0_writedata  = { ~data_cnt, data_cnt };
assign sdram0_write      = test_is_running;
assign sdram0_byteenable = 16'hffff;
assign sdram0_burstcount = 1'b1;
assign sdram0_address    = addr_cnt;


endmodule
