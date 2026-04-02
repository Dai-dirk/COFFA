module myMulSub ();

wire [31:0] internal_wire_0;
wire [31:0] internal_wire_1;
wire [31:0] internal_wire_2;
wire [31:0] internal_wire_3;
wire [31:0] internal_wire_4;

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
\INPUT #(
    .Y_WIDTH(32)
) input_2 (
    .Y(internal_wire_2)
);

\MUL #(
    .A_WIDTH(32),
    .B_WIDTH(32),
    .Y_WIDTH(32)
)Mul_1(
    .A(internal_wire_0),
    .B(internal_wire_1),
    .Y(internal_wire_3)
);

\SUB #(
    .A_WIDTH(32),
    .B_WIDTH(32),
    .Y_WIDTH(32)
)Sub_1(
    .A(internal_wire_3),
    .B(internal_wire_2),
    .Y(internal_wire_4)
);


\OUTPUT #(
    .A_WIDTH(32)
) output_0 (
    .A(internal_wire_4)
);
endmodule
