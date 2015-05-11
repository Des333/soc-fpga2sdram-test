
`define BITS_IN_BYTE   8 

module avalon_width_adapter #(
//*************************************************
// For user config
//*************************************************
  // wide_width / narrow_width
  parameter WIDTH_RATIO        = 2,

  // narrow BE = 2, narrow DATA_W = 16,
  // wide   BE = 8, wide   DATA_W = 64,
  parameter NARROW_IF_BE_W     = 4,   

  // wide addr_cnt = 1024, narrow addr_cnt = 1024
  parameter WIDE_IF_ADDR_W     = 12,
  parameter SLAVE_ADDR_IS_BYTE = 0,

//*************************************************
// Not for user config
//*************************************************
  parameter WIDE_IF_BE_W      = WIDTH_RATIO * NARROW_IF_BE_W, 
  parameter NARROW_IF_DATA_W  = NARROW_IF_BE_W * `BITS_IN_BYTE, 
  parameter WIDE_IF_DATA_W    = WIDTH_RATIO * NARROW_IF_DATA_W, 
  parameter NARROW_IF_ADDR_W  = SLAVE_ADDR_IS_BYTE ? WIDE_IF_ADDR_W : ( WIDE_IF_ADDR_W - $clog2( NARROW_IF_BE_W ) )
) (
  input                                 clk_i,
  input                                 rst_i,
  

  // Wide IF
  input  [WIDE_IF_DATA_W-1:0]           wide_writedata_i,
  input  [WIDE_IF_BE_W-1:0]             wide_byteenable_i,
  input                                 wide_write_i,
  input                                 wide_read_i,
  input  [WIDE_IF_ADDR_W-1:0]           wide_address_i,

  output logic [WIDE_IF_DATA_W-1:0]     wide_readdata_o,
  output logic                          wide_waitrequest_o,
  output logic                          wide_datavalid_o,


// Narrow IF
  output logic [NARROW_IF_DATA_W-1:0]   narrow_writedata_o,
  output logic [NARROW_IF_BE_W-1:0]     narrow_byteenable_o,
  output logic                          narrow_write_o,
  output logic                          narrow_read_o,
  output logic [NARROW_IF_ADDR_W-1:0]   narrow_address_o,

  input [NARROW_IF_DATA_W-1:0]          narrow_readdata_i,
  input                                 narrow_datavalid_i,
  input                                 narrow_waitrequest_i
);

logic read_is_finished[1:0];
logic write_is_finished;

//*************************************************
// FSM
//*************************************************
enum logic [2:0] { IDLE_S         = 3'd1,
                   READ_S         = 3'd2,
                   WRITE_S        = 3'd4
                  } state, next_state;

always_ff @( posedge clk_i or posedge rst_i )
  if( rst_i )
    state <= IDLE_S;
  else
    state <= next_state;


always_comb
  begin
    next_state = state;
    case( state )

      IDLE_S:
        begin
          // Reading and writing can't happen simultaneously
          if( wide_read_i )
            next_state = READ_S;
          else  
            if( wide_write_i )
              next_state = WRITE_S;
        end
      
      READ_S:
        begin
          // Waiting real reading finish 
          if( read_is_finished[1] )
            next_state = IDLE_S;
        end
      
      WRITE_S:
        begin
          if( write_is_finished )
            next_state = IDLE_S;
        end
  
    endcase
  end


//*************************************************
// Latch tarnsaction parameters 
//*************************************************
logic [WIDTH_RATIO-1:0][NARROW_IF_DATA_W-1:0]  wide_writedata_d1;
logic [WIDTH_RATIO-1:0][NARROW_IF_BE_W-1:0]    wide_byteenable_d1;
logic [WIDE_IF_ADDR_W-1:0]                     wide_address_d1;

always_ff @( posedge clk_i )
  if( ( state == IDLE_S ) && ( next_state != IDLE_S ) )
    begin
      wide_writedata_d1  <= wide_writedata_i;
      wide_byteenable_d1 <= wide_byteenable_i;
      wide_address_d1    <= wide_address_i;
    end
   


//*************************************************
// Takt counters
//*************************************************
logic [$clog2(WIDTH_RATIO):0] takt_cnt;
logic [$clog2(WIDTH_RATIO)-1:0] rd_takt_cnt;

always_ff @( posedge clk_i )
  if( next_state == IDLE_S )
    takt_cnt <= '0;
  else
    if( !narrow_waitrequest_i )  
      if( takt_cnt != WIDTH_RATIO )
        takt_cnt <= takt_cnt + 1'd1;

always_ff @( posedge clk_i )
  if( next_state != READ_S )
    rd_takt_cnt <= '0;
  else
    if( narrow_datavalid_i )
      rd_takt_cnt <= rd_takt_cnt + 1'd1;



//*************************************************
// Forming transaction on narrow interface
//*************************************************
always_comb
  if( ( state == IDLE_S ) && ( next_state != IDLE_S ) )
    begin
      narrow_writedata_o  = wide_writedata_i[NARROW_IF_DATA_W-1:0];
      narrow_byteenable_o = wide_byteenable_i[NARROW_IF_BE_W-1:0];
    end
  else
    begin
      narrow_writedata_o  = wide_writedata_d1[ takt_cnt ];
      narrow_byteenable_o = wide_byteenable_d1[ takt_cnt ];
    end  


logic [WIDE_IF_ADDR_W-1:0] wide_address_w;

always_comb
  if( ( state == IDLE_S ) && ( next_state != IDLE_S ) )
    wide_address_w = wide_address_i;
  else  
    wide_address_w = wide_address_d1;

generate 
  if( SLAVE_ADDR_IS_BYTE )
    assign narrow_address_o = wide_address_w + ( takt_cnt << $clog2( NARROW_IF_BE_W )  );
  else
    assign narrow_address_o = ( wide_address_w >> $clog2( NARROW_IF_BE_W ) ) + takt_cnt;
endgenerate


assign narrow_write_o = ( next_state == WRITE_S ) || ( state == WRITE_S );
assign narrow_read_o  = ( ( next_state == READ_S  ) || ( state == READ_S  ) ) && ( takt_cnt < WIDTH_RATIO );


//*************************************************
// Forming data on wide IF
//*************************************************

assign read_is_finished[0]  = narrow_datavalid_i && ( rd_takt_cnt == WIDTH_RATIO - 1 ) && ( state == READ_S  );
assign write_is_finished    = !narrow_waitrequest_i && ( takt_cnt == WIDTH_RATIO - 1 ) && ( state == WRITE_S );

always_ff @( posedge clk_i )
  read_is_finished[1] <= read_is_finished[0];

logic [WIDTH_RATIO-1:0][NARROW_IF_DATA_W-1:0] wide_readdata;

always_ff @( posedge clk_i )
  wide_readdata[ rd_takt_cnt ] <= narrow_readdata_i;

assign wide_readdata_o    = wide_readdata;
assign wide_datavalid_o   = read_is_finished[1]; 
assign wide_waitrequest_o = ( next_state != IDLE_S ); 


endmodule
