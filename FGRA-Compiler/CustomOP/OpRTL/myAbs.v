module myAbs ();

wire [31:0] internal_wire_0;
wire [31:0] internal_wire_1;
wire [31:0] internal_wire_2;
wire [31:0] internal_wire_3;
wire [31:0] internal_wire_4;
wire [31:0] internal_wire_5;
wire [31:0] internal_wire_6;
wire internal_wire_fg_7;

\INPUT #(
    .Y_WIDTH(32)
) input_0 (
    .Y(internal_wire_0)
);


\CONST #(
    .Y_WIDTH(32),
    .VALUE(0)
) const_0 (
    .Y(internal_wire_1)
);

\CONST #(
    .Y_WIDTH(32),
    .VALUE(1)
) const_1 (
    .Y(internal_wire_2)
);

\CONST #(
    .Y_WIDTH(32),
    .VALUE(32'hffffffff)
) const_2 (
    .Y(internal_wire_3)
);

\SGE #(
    .A_WIDTH(32),
    .B_WIDTH(32),
    .Y_WIDTH(1)
) Sgt_1(
    .A(internal_wire_0),
    .B(internal_wire_1),
    .Y(internal_wire_fg_7)
);

\XOR #(
    .A_WIDTH(32),
    .B_WIDTH(32),
    .Y_WIDTH(32)
) Xor_1(
    .A(internal_wire_0),
    .B(internal_wire_3),
    .Y(internal_wire_4)
);

\ADD #(
    .A_WIDTH(32),
    .B_WIDTH(32),
    .Y_WIDTH(32)
) Add_1(
    .A(internal_wire_2),
    .B(internal_wire_4),
    .Y(internal_wire_5)
);

\SEL #(
    .A_WIDTH(32),
    .B_WIDTH(32),
    .C_WIDTH(1),
    .Y_WIDTH(32)
) Sel_1(
    .A(internal_wire_0),
    .B(internal_wire_5),
    .C(internal_wire_fg_7),
    .Y(internal_wire_6)
);

\OUTPUT #(
    .A_WIDTH(32)
) output_0 (
    .A(internal_wire_6)
);

endmodule
