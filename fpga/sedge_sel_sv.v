module sedge_sel_sv(
  input ain,
  input Clk,
  output edg
);

reg ain_d1,ain_d2,ain_d3;
always @(posedge Clk)
begin
  ain_d1 <= ain;
  ain_d2 <= ain_d1;
  ain_d3 <= ain_d2;
end

assign edg = ain_d2 & ~ain_d3;

endmodule
