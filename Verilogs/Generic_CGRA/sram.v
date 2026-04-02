`define ASIC

module dp_sram_32_8_ip #(
    parameter width = 32,
    parameter lgDepth = 8
)(
    input                clk,
    input                a_en,
    input                a_we,
    input  [lgDepth-1:0] a_addr,
    input  [width-1:0]   a_din,
    output [width-1:0]   a_dout,
    input                b_en,
    input                b_we,
    input  [lgDepth-1:0] b_addr,
    input  [width-1:0]   b_din,
    output [width-1:0]   b_dout
);
`ifdef ASIC
dp_sram_32_8 sram (
  .QA    ( a_dout ),
  .ADRA  ( a_addr ),
  .DA    ( a_din  ),
  .WEA   ( a_we   ),
  .MEA   ( a_en   ),
  .CLKA  ( clk    ),
  .TEST1A( 1'h0   ),
  .RMEA  ( 1'h0   ),
  .RMA   ( 4'h0   ),
  .LS    ( 1'h0   ),
  .QB    ( b_dout ),
  .ADRB  ( b_addr ),
  .DB    ( b_din  ),
  .WEB   ( b_we   ),
  .MEB   ( b_en   ),
  .CLKB  ( clk    ),
  .TEST1B( 1'h0   ),
  .RMEB  ( 1'h0   ),
  .RMB   ( 4'h0   )
);
`else
dp_sram_32_8 sram (
  .clka (clk   ),
  .ena  (a_en  ),
  .wea  (a_we  ),
  .addra(a_addr),
  .dina (a_din ),
  .douta(a_dout),
  .clkb (clk  ),
  .enb  (b_en  ),
  .web  (b_we  ),
  .addrb(b_addr),
  .dinb (b_din ),
  .doutb(b_dout)
);
`endif


endmodule

module dp_sram_32_9_ip #(
    parameter width = 32,
    parameter lgDepth = 9
)(
    input                clk,
    input                a_en,
    input                a_we,
    input  [lgDepth-1:0] a_addr,
    input  [width-1:0]   a_din,
    output [width-1:0]   a_dout,
    input                b_en,
    input                b_we,
    input  [lgDepth-1:0] b_addr,
    input  [width-1:0]   b_din,
    output [width-1:0]   b_dout
);
`ifdef ASIC
dp_sram_32_9 sram (
  .QA    ( a_dout ),
  .ADRA  ( a_addr ),
  .DA    ( a_din  ),
  .WEA   ( a_we   ),
  .MEA   ( a_en   ),
  .CLKA  ( clk    ),
  .TEST1A( 1'h0   ),
  .RMEA  ( 1'h0   ),
  .RMA   ( 4'h0   ),
  .LS    ( 1'h0   ),
  .QB    ( b_dout ),
  .ADRB  ( b_addr ),
  .DB    ( b_din  ),
  .WEB   ( b_we   ),
  .MEB   ( b_en   ),
  .CLKB  ( clk    ),
  .TEST1B( 1'h0   ),
  .RMEB  ( 1'h0   ),
  .RMB   ( 4'h0   )
);
`else
dp_sram_32_9 sram (
  .clka (clk   ),
  .ena  (a_en  ),
  .wea  (a_we  ),
  .addra(a_addr),
  .dina (a_din ),
  .douta(a_dout),
  .clkb (clk  ),
  .enb  (b_en  ),
  .web  (b_we  ),
  .addrb(b_addr),
  .dinb (b_din ),
  .doutb(b_dout)
);
`endif


endmodule


module dp_sram_32_10_ip #(
    parameter width = 32,
    parameter lgDepth = 10
)(
    input                clk,
    input                a_en,
    input                a_we,
    input  [lgDepth-1:0] a_addr,
    input  [width-1:0]   a_din,
    output [width-1:0]   a_dout,
    input                b_en,
    input                b_we,
    input  [lgDepth-1:0] b_addr,
    input  [width-1:0]   b_din,
    output [width-1:0]   b_dout
);
`ifdef ASIC
dp_sram_32_10 sram (
  .QA    ( a_dout ),
  .ADRA  ( a_addr ),
  .DA    ( a_din  ),
  .WEA   ( a_we   ),
  .MEA   ( a_en   ),
  .CLKA  ( clk    ),
  .TEST1A( 1'h0   ),
  .RMEA  ( 1'h0   ),
  .RMA   ( 4'h0   ),
  .LS    ( 1'h0   ),
  .QB    ( b_dout ),
  .ADRB  ( b_addr ),
  .DB    ( b_din  ),
  .WEB   ( b_we   ),
  .MEB   ( b_en   ),
  .CLKB  ( clk    ),
  .TEST1B( 1'h0   ),
  .RMEB  ( 1'h0   ),
  .RMB   ( 4'h0   )
);
`else
dp_sram_32_10 sram (
  .clka (clk   ),
  .ena  (a_en  ),
  .wea  (a_we  ),
  .addra(a_addr),
  .dina (a_din ),
  .douta(a_dout),
  .clkb (clk  ),
  .enb  (b_en  ),
  .web  (b_we  ),
  .addrb(b_addr),
  .dinb (b_din ),
  .doutb(b_dout)
);
`endif


endmodule


module dp_sram_128_7_ip #(
    parameter width = 128,
    parameter lgDepth = 7
)(
    input                clk,
    input                a_en,
    input                a_we,
    input  [lgDepth-1:0] a_addr,
    input  [width-1:0]   a_din,
    output [width-1:0]   a_dout,
    input                b_en,
    input                b_we,
    input  [lgDepth-1:0] b_addr,
    input  [width-1:0]   b_din,
    output [width-1:0]   b_dout
);

`ifdef ASIC
dp_sram_128_7 sram (
  .QA    ( a_dout ),
  .ADRA  ( a_addr ),
  .DA    ( a_din  ),
  .WEA   ( a_we   ),
  .MEA   ( a_en   ),
  .CLKA  ( clk    ),
  .TEST1A( 1'h0   ),
  .RMEA  ( 1'h0   ),
  .RMA   ( 4'h0   ),
  .LS    ( 1'h0   ),
  .QB    ( b_dout ),
  .ADRB  ( b_addr ),
  .DB    ( b_din  ),
  .WEB   ( b_we   ),
  .MEB   ( b_en   ),
  .CLKB  ( clk    ),
  .TEST1B( 1'h0   ),
  .RMEB  ( 1'h0   ),
  .RMB   ( 4'h0   )
);
`else
dp_sram_128_7 sram (
  .clka (clk   ),
  .ena  (a_en  ),
  .wea  (a_we  ),
  .addra(a_addr),
  .dina (a_din ),
  .douta(a_dout),
  .clkb (clk  ),
  .enb  (b_en  ),
  .web  (b_we  ),
  .addrb(b_addr),
  .dinb (b_din ),
  .doutb(b_dout)
);
`endif


endmodule


module dp_sram_128_6_ip #(
    parameter width = 128,
    parameter lgDepth = 6
)(
    input                clk,
    input                a_en,
    input                a_we,
    input  [lgDepth-1:0] a_addr,
    input  [width-1:0]   a_din,
    output [width-1:0]   a_dout,
    input                b_en,
    input                b_we,
    input  [lgDepth-1:0] b_addr,
    input  [width-1:0]   b_din,
    output [width-1:0]   b_dout
);

`ifdef ASIC
dp_sram_128_6 sram (
  .QA    ( a_dout ),
  .ADRA  ( a_addr ),
  .DA    ( a_din  ),
  .WEA   ( a_we   ),
  .MEA   ( a_en   ),
  .CLKA  ( clk    ),
  .TEST1A( 1'h0   ),
  .RMEA  ( 1'h0   ),
  .RMA   ( 4'h0   ),
  .LS    ( 1'h0   ),
  .QB    ( b_dout ),
  .ADRB  ( b_addr ),
  .DB    ( b_din  ),
  .WEB   ( b_we   ),
  .MEB   ( b_en   ),
  .CLKB  ( clk    ),
  .TEST1B( 1'h0   ),
  .RMEB  ( 1'h0   ),
  .RMB   ( 4'h0   )
);
`else
dp_sram_128_6 sram (
  .clka (clk   ),
  .ena  (a_en  ),
  .wea  (a_we  ),
  .addra(a_addr),
  .dina (a_din ),
  .douta(a_dout),
  .clkb (clk  ),
  .enb  (b_en  ),
  .web  (b_we  ),
  .addrb(b_addr),
  .dinb (b_din ),
  .doutb(b_dout)
);
`endif


endmodule


module dp_sram_128_5_ip #(
    parameter width = 128,
    parameter lgDepth = 5
)(
    input                clk,
    input                a_en,
    input                a_we,
    input  [lgDepth-1:0] a_addr,
    input  [width-1:0]   a_din,
    output [width-1:0]   a_dout,
    input                b_en,
    input                b_we,
    input  [lgDepth-1:0] b_addr,
    input  [width-1:0]   b_din,
    output [width-1:0]   b_dout
);

`ifdef ASIC
dp_sram_128_5 sram (
  .QA    ( a_dout ),
  .ADRA  ( a_addr ),
  .DA    ( a_din  ),
  .WEA   ( a_we   ),
  .MEA   ( a_en   ),
  .CLKA  ( clk    ),
  .TEST1A( 1'h0   ),
  .RMEA  ( 1'h0   ),
  .RMA   ( 4'h0   ),
  .LS    ( 1'h0   ),
  .QB    ( b_dout ),
  .ADRB  ( b_addr ),
  .DB    ( b_din  ),
  .WEB   ( b_we   ),
  .MEB   ( b_en   ),
  .CLKB  ( clk    ),
  .TEST1B( 1'h0   ),
  .RMEB  ( 1'h0   ),
  .RMB   ( 4'h0   )
);
`else
dp_sram_128_5 sram (
  .clka (clk   ),
  .ena  (a_en  ),
  .wea  (a_we  ),
  .addra(a_addr),
  .dina (a_din ),
  .douta(a_dout),
  .clkb (clk  ),
  .enb  (b_en  ),
  .web  (b_we  ),
  .addrb(b_addr),
  .dinb (b_din ),
  .doutb(b_dout)
);
`endif


endmodule


module dp_sram_128_8_ip #(
    parameter width = 128,
    parameter lgDepth = 8
)(
    input                clk,
    input                a_en,
    input                a_we,
    input  [lgDepth-1:0] a_addr,
    input  [width-1:0]   a_din,
    output [width-1:0]   a_dout,
    input                b_en,
    input                b_we,
    input  [lgDepth-1:0] b_addr,
    input  [width-1:0]   b_din,
    output [width-1:0]   b_dout
);

`ifdef ASIC
dp_sram_128_8 sram (
  .QA    ( a_dout ),
  .ADRA  ( a_addr ),
  .DA    ( a_din  ),
  .WEA   ( a_we   ),
  .MEA   ( a_en   ),
  .CLKA  ( clk    ),
  .TEST1A( 1'h0   ),
  .RMEA  ( 1'h0   ),
  .RMA   ( 4'h0   ),
  .LS    ( 1'h0   ),
  .QB    ( b_dout ),
  .ADRB  ( b_addr ),
  .DB    ( b_din  ),
  .WEB   ( b_we   ),
  .MEB   ( b_en   ),
  .CLKB  ( clk    ),
  .TEST1B( 1'h0   ),
  .RMEB  ( 1'h0   ),
  .RMB   ( 4'h0   )
);
`else
dp_sram_128_8 sram (
  .clka (clk   ),
  .ena  (a_en  ),
  .wea  (a_we  ),
  .addra(a_addr),
  .dina (a_din ),
  .douta(a_dout),
  .clkb (clk  ),
  .enb  (b_en  ),
  .web  (b_we  ),
  .addrb(b_addr),
  .dinb (b_din ),
  .doutb(b_dout)
);
`endif


endmodule

module dp_sram_128_4_ip #(
    parameter width = 128,
    parameter lgDepth = 4
)(
    input                clk,
    input                a_en,
    input                a_we,
    input  [lgDepth-1:0] a_addr,
    input  [width-1:0]   a_din,
    output [width-1:0]   a_dout,
    input                b_en,
    input                b_we,
    input  [lgDepth-1:0] b_addr,
    input  [width-1:0]   b_din,
    output [width-1:0]   b_dout
);

`ifdef ASIC
wire [lgDepth : 0] addr_a;
wire [lgDepth : 0] addr_b;
assign addr_a = {1'b0, a_addr};
assign addr_b = {1'b0, b_addr};
dp_sram_128_4 sram (
  .QA    ( a_dout ),
  .ADRA  ( addr_a ),
  .DA    ( a_din  ),
  .WEA   ( a_we   ),
  .MEA   ( a_en   ),
  .CLKA  ( clk    ),
  .TEST1A( 1'h0   ),
  .RMEA  ( 1'h0   ),
  .RMA   ( 4'h0   ),
  .LS    ( 1'h0   ),
  .QB    ( b_dout ),
  .ADRB  ( addr_b ),
  .DB    ( b_din  ),
  .WEB   ( b_we   ),
  .MEB   ( b_en   ),
  .CLKB  ( clk    ),
  .TEST1B( 1'h0   ),
  .RMEB  ( 1'h0   ),
  .RMB   ( 4'h0   )
);
`else
dp_sram_128_8 sram (
  .clka (clk   ),
  .ena  (a_en  ),
  .wea  (a_we  ),
  .addra(a_addr),
  .dina (a_din ),
  .douta(a_dout),
  .clkb (clk  ),
  .enb  (b_en  ),
  .web  (b_we  ),
  .addrb(b_addr),
  .dinb (b_din ),
  .doutb(b_dout)
);
`endif


endmodule

module dp_sram_128_10 #(
    parameter width = 128,
    parameter lgDepth = 10
)(
    input                clk,
    input                a_en,
    input                a_we,
    input  [lgDepth-1:0] a_addr,
    input  [width-1:0]   a_din,
    output [width-1:0]   a_dout,
    input                b_en,
    input                b_we,
    input  [lgDepth-1:0] b_addr,
    input  [width-1:0]   b_din,
    output [width-1:0]   b_dout
);

`ifdef ASIC
dp_sram_128_10_ip sram (
  .QA    ( a_dout ),
  .ADRA  ( a_addr ),
  .DA    ( a_din  ),
  .WEA   ( a_we   ),
  .MEA   ( a_en   ),
  .CLKA  ( clk    ),
  .TEST1A( 1'h0   ),
  .RMEA  ( 1'h0   ),
  .RMA   ( 4'h0   ),
  .LS    ( 1'h0   ),
  .QB    ( b_dout ),
  .ADRB  ( b_addr ),
  .DB    ( b_din  ),
  .WEB   ( b_we   ),
  .MEB   ( b_en   ),
  .CLKB  ( clk    ),
  .TEST1B( 1'h0   ),
  .RMEB  ( 1'h0   ),
  .RMB   ( 4'h0   )
);
`else
dp_sram_128_8 sram (
  .clka (clk   ),
  .ena  (a_en  ),
  .wea  (a_we  ),
  .addra(a_addr),
  .dina (a_din ),
  .douta(a_dout),
  .clkb (clk  ),
  .enb  (b_en  ),
  .web  (b_we  ),
  .addrb(b_addr),
  .dinb (b_din ),
  .doutb(b_dout)
);
`endif


endmodule