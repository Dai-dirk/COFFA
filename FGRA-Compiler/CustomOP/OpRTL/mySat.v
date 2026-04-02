module mySat ();

wire [31:0] internal_wire_0;
wire [31:0] internal_wire_1;
wire [31:0] internal_wire_2;
wire [31:0] internal_wire_3;
wire [31:0] internal_wire_4;
wire [31:0] internal_wire_5;
wire [31:0] internal_wire_6;
wire [31:0] internal_wire_7;
wire [31:0] internal_wire_8;
wire [31:0] internal_wire_9;
wire [31:0] internal_wire_10;
wire [31:0] internal_wire_11;
wire  internal_wire_12_fg;
wire  internal_wire_13_fg;

\INPUT #(
    .Y_WIDTH(32)
) input_0 (
    .Y(internal_wire_0)
);

\INPUT #(
    .Y_WIDTH(32)
) input_1 (
    .Y(internal_wire_1)
);

\CONST #(
    .Y_WIDTH(32),
    .VALUE(1)
) const_0 (
    .Y(internal_wire_2)
);


\CONST #(
    .Y_WIDTH(32),
    .VALUE(1)
) const_1 (
    .Y(internal_wire_4)
);

\CONST #(
    .Y_WIDTH(32),
    .VALUE(1)
) const_2 (
    .Y(internal_wire_6)
);


\CONST #(
    .Y_WIDTH(32),
    .VALUE(32'hffffffff)
) const_3 (
    .Y(internal_wire_7)
);

\SUB #(
    .A_WIDTH(32),
    .B_WIDTH(32),
    .Y_WIDTH(32)
) Sub_1(
    .A(internal_wire_1),
    .B(internal_wire_2),
    .Y(internal_wire_3)
);


\SHL #(
    .A_WIDTH(32),
    .B_WIDTH(32),
    .Y_WIDTH(32)
) Shl_1(
    .A(internal_wire_4),
    .B(internal_wire_3),
    .Y(internal_wire_5)
);

\SUB #(
    .A_WIDTH(32),
    .B_WIDTH(32),
    .Y_WIDTH(32)
) Sub_2(
    .A(internal_wire_5),
    .B(internal_wire_6),
    .Y(internal_wire_8)
);




\XOR #(
    .A_WIDTH(32),
    .B_WIDTH(32),
    .Y_WIDTH(32)
) Xor_1(
    .A(internal_wire_5),
    .B(internal_wire_7),
    .Y(internal_wire_9)
);



\SLT #(
    .A_WIDTH(32),
    .B_WIDTH(32),
    .Y_WIDTH(1)
) Slt_1(
    .A(internal_wire_0),
    .B(internal_wire_9),
    .Y(internal_wire_12_fg)
);

\SGT #(
    .A_WIDTH(32),
    .B_WIDTH(32),
    .Y_WIDTH(1)
) Sgt_1(
    .A(internal_wire_0),
    .B(internal_wire_8),
    .Y(internal_wire_13_fg)
);



\SEL #(
    .A_WIDTH(32),
    .B_WIDTH(32),
    .C_WIDTH(1),
    .Y_WIDTH(32)
) Sel_1(
    .A(internal_wire_0),
    .B(internal_wire_9),
    .C(internal_wire_12_fg),
    .Y(internal_wire_10)
);


\SEL #(
    .A_WIDTH(32),
    .B_WIDTH(32),
    .C_WIDTH(1),
    .Y_WIDTH(32)
) Sel_2(
    .A(internal_wire_10),
    .B(internal_wire_8),
    .C(internal_wire_13_fg),
    .Y(internal_wire_11)
);

\OUTPUT #(
    .A_WIDTH(32)
) output_0 (
    .A(internal_wire_11)
);

endmodule
