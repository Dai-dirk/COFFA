module myMax ();

wire [31:0] internal_wire_0;
wire [31:0] internal_wire_1;
wire [31:0] internal_wire_2;
wire internal_wire_fg_3;

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


\SGT #(
    .A_WIDTH(32),
    .B_WIDTH(32),
    .Y_WIDTH(1)
) Sgt_1(
    .A(internal_wire_0),
    .B(internal_wire_1),
    .Y(internal_wire_fg_3)
);


\SEL #(
    .A_WIDTH(32),
    .B_WIDTH(32),
    .C_WIDTH(1),
    .Y_WIDTH(32)
) Sel_1(
    .A(internal_wire_0),
    .B(internal_wire_1),
    .C(internal_wire_fg_3),
    .Y(internal_wire_2)
);

\OUTPUT #(
    .A_WIDTH(32)
) output_0 (
    .A(internal_wire_2)
);

endmodule
