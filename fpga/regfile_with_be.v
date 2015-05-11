module regfile_with_be #( 
  // Status register count
  parameter STAT_CNT = 32, 
  // Control register count
  parameter CTRL_CNT = 32, 
  // Address width
  parameter ADDR_W = 7, 
  // Data width
  parameter DATA_W = 8,

  // If 1 -- selected status register using MSB,
  // otherwise -- by registers count.
  parameter SEL_SR_BY_MSB = 1 
) (
  input                           clk_i,
  input                           rst_i,
  input        [DATA_W-1:0]       data_i,          
  input                           wren_i,          
//FIXME: add normal parametrization!
  input        [1:0]              be_i,            
  input        [ADDR_W-1:0]       addr_i,          
  input        [DATA_W-1:0]       sreg_i [STAT_CNT-1:0], 
  output logic [DATA_W-1:0]       data_o,          
  output logic [DATA_W-1:0]       creg_o [CTRL_CNT-1:0]  
); 

localparam CR_CNT_WIDTH = ( CTRL_CNT == 1 ) ? ( 1 ) : $clog2( CTRL_CNT );
localparam SR_CNT_WIDTH = ( STAT_CNT == 1 ) ? ( 1 ) : $clog2( STAT_CNT );

// Inner address of status or conrrol register
logic [CR_CNT_WIDTH-1:0] cr_local_addr;
logic [SR_CNT_WIDTH-1:0] sr_local_addr;

// Array of control registers
logic [DATA_W-1:0] ctrl_regs [CTRL_CNT-1:0]; 

logic status_is_sel;




// Write to conotrl register operation
always_ff @( posedge clk_i, posedge rst_i )
  if( rst_i )
    begin
      int i;
      for( i = 0; i < CTRL_CNT; i++ )
        ctrl_regs[i] <= '0;
    end
  else
    if( wren_i & !status_is_sel )
      //FIXME: add normal parametrization!
      case( be_i )
        3'b01:
          ctrl_regs[cr_local_addr][7:0] <= data_i[7:0]; 

        3'b10:
          ctrl_regs[cr_local_addr][15:8] <= data_i[15:8]; 

        3'b11:
          ctrl_regs[cr_local_addr] <= data_i; 
      endcase      


// SEL_SR_BY_MSB == 1:
//   If MSB of address == 1, we access to status registers
//   otherwise we access to control registers.
// SEL_SR_BY_MSB == 0:
//   We used address for select registers type.
assign status_is_sel = ( SEL_SR_BY_MSB == 1 ) ? ( addr_i[ADDR_W-1]   ) : ( addr_i >= CTRL_CNT       );

assign cr_local_addr = ( SEL_SR_BY_MSB == 1 ) ? ( addr_i[ADDR_W-2:0] ) : ( addr_i[CR_CNT_WIDTH-1:0] );
assign sr_local_addr = ( SEL_SR_BY_MSB == 1 ) ? ( addr_i[ADDR_W-2:0] ) : ( addr_i - CTRL_CNT        );


// Read operation.
always_comb
  if( status_is_sel ) 
    data_o = sreg_i[sr_local_addr];    
  else       
    data_o = ctrl_regs[cr_local_addr];

// Output assigning
always_comb
  creg_o = ctrl_regs;

endmodule







