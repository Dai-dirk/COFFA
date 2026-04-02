`timescale 1ns / 1ps

module CSA_tree#(parameter WIDTH = 32,
                 parameter WIDTH32 = 32  
)(
    input [WIDTH-1:0] A,
    input [WIDTH-3:0] B,
    input [4:0] n,
    input float_flag,
    input [WIDTH32-1:0] B0,
    input [WIDTH-3:0] log_c1,
    input [WIDTH-3:0] log_c2,
    input [WIDTH-3:0] log_c3,
    input [WIDTH-3:0] log_c4,
    input [WIDTH-3:0] log_c5,
    input TRG,
    input VEC,
    output [9:0] zero_sign_k0,
    output [WIDTH+5:0] cpa_channel0,
    output [WIDTH+5:0] cpa_channel1,
    output [WIDTH+5:0] cpa_channel2,
    output [WIDTH+5:0] cpa_channel3,
    output [WIDTH+5:0] cpa_channel4
);

    // wire [27:0] pp0 [3:0];
    // wire [26:0] pp1 [3:0];
    // wire [27:0] pp2 [3:0];
    // wire [24:0] extend_A;
    // assign  extend_A = {A[23],  A[23:0]};

    // wire [24:0] rev_A;
    // assign  rev_A = ~extend_A + 1'b1;

    wire [33:0] pp0 [4:0];
    wire [32:0] pp1 [4:0];
    wire [33:0] pp2 [4:0];
    wire [30:0] extend_A;
    assign  extend_A = {A[29],  A[29:0]};

    wire [30:0] rev_A;
    assign  rev_A = ~extend_A + 1'b1;

    Booth_pp booth0 (
        .input_7({B[5:0], 1'b0}),
        .TRG(TRG),
        .A(extend_A),
        .rev_A(rev_A),
        .pp0(pp0[0]),
        .pp1(pp1[0]),
        .pp2(pp2[0])
    );

    Booth_pp booth1 (
        .input_7(B[11:5]),
        .TRG(TRG),
        .A(extend_A),
        .rev_A(rev_A),
        .pp0(pp0[1]),
        .pp1(pp1[1]),
        .pp2(pp2[1])
    );
    Booth_pp booth2 (
        .input_7(B[17:11]),
        .TRG(TRG),
        .A(extend_A),
        .rev_A(rev_A),
        .pp0(pp0[2]),
        .pp1(pp1[2]),
        .pp2(pp2[2])
    );
    Booth_pp booth3 (
        .input_7(B[23:17]),
        .TRG(TRG),
        .A(extend_A),
        .rev_A(rev_A),
        .pp0(pp0[3]),
        .pp1(pp1[3]),
        .pp2(pp2[3])
    );

    Booth_pp booth4 (
        .input_7(B[29:23]),
        .TRG(TRG),
        .A(extend_A),
        .rev_A(rev_A),
        .pp0(pp0[4]),
        .pp1(pp1[4]),
        .pp2(pp2[4])
    );

    reg [37:0] CSA0_op [2:0];
    reg [37:0] CSA1_op [2:0];
    reg [37:0] CSA2_op [2:0];
    reg [37:0] CSA3_op [2:0];
    reg [37:0] CSA4_op [2:0];

    always @(*) begin
        if(TRG==1'b1) begin
            CSA0_op[0] = {4'b0000, pp0[0]};
            CSA0_op[1] = {3'b000, pp1[0], 2'b00};
            CSA0_op[2] = {pp2[0], 4'b0000};

            CSA1_op[0] = {4'b0000, pp0[1]};
            CSA1_op[1] = {3'b000, pp1[1], 2'b00};
            CSA1_op[2] = {pp2[1], 4'b0000};

            CSA2_op[0] = {4'b0000, pp0[2]};
            CSA2_op[1] = {3'b000, pp1[2], 2'b00};
            CSA2_op[2] = {pp2[2], 4'b0000};

            CSA3_op[0] = {4'b0000, pp0[3]};
            CSA3_op[1] = {3'b000, pp1[3], 2'b00};
            CSA3_op[2] = {pp2[3], 4'b0000};

            CSA4_op[0] = {4'b0000, pp0[4]};
            CSA4_op[1] = {3'b000, pp1[4], 2'b00};
            CSA4_op[2] = {pp2[4], 4'b0000};
        end
        else begin
            CSA0_op[0] = {26'b0, pp0[0][33:22]};
            CSA0_op[1] = {25'b0, pp1[0][32:20]};
            CSA0_op[2] = {23'b0, pp2[0][32:18]};

            CSA1_op[0] = {21'b0, 1'b1, pp0[1][33], pp0[1][30:16]};
            CSA1_op[1] = {19'b0, pp1[1][32:14]};
            CSA1_op[2] = {17'b0, pp2[1][32:12]};

            CSA2_op[0] = {15'b0, 1'b1, pp0[2][33], pp0[2][30:10]};
            CSA2_op[1] = {13'b0, pp1[2][32:8]};
            CSA2_op[2] = {11'b0, pp2[2][32:6]};

            CSA3_op[0] = {9'b0, 1'b1, pp0[3][33], pp0[3][30:4]};
            CSA3_op[1] = {7'b0, pp1[3][32:2]};
            CSA3_op[2] = {5'b0, pp2[3][32:0]};

            CSA4_op[0] = {3'b0, 1'b1, pp0[4][33], pp0[4][30:0], 2'b0};
            CSA4_op[1] = {1'b0, pp1[4][32:0], 4'b0};
            CSA4_op[2] = {pp2[4][31:0], 6'b0};
        end
    end

    wire [37:0] extend_log_c0;
    wire [37:0] extend_log_c1;
    wire [37:0] extend_log_c2;
    wire [37:0] extend_log_c3;
    wire [37:0] extend_log_c4;
    assign extend_log_c0 = (log_c1[29] == 1'b0) ? {8'b00000000, log_c1} : {8'b11111111, log_c1};
    assign extend_log_c1 = (log_c2[29] == 1'b0) ? {8'b00000000, log_c2} : {8'b11111111, log_c2};
    assign extend_log_c2 = (log_c3[29] == 1'b0) ? {8'b00000000, log_c3} : {8'b11111111, log_c3};
    assign extend_log_c3 = (log_c4[29] == 1'b0) ? {8'b00000000, log_c4} : {8'b11111111, log_c4};
    assign extend_log_c4 = (log_c5[29] == 1'b0) ? {8'b00000000, log_c5} : {8'b11111111, log_c5};

    wire zero_signal_y0;
    wire sign_y0;
    // wire [5:0] k0;
    wire [7:0] k0;
    wire [21:0] A0_y0, A1_y0, A2_y0, A3_y0, b_y0;

    LOGC logc0(
        .y(B0),
        .n(n),
        .float_flag(float_flag),
        .zero_signal(zero_signal_y0),
        .sign(sign_y0),
        .k(k0),
        .A0(A0_y0),
        .A1(A1_y0),
        .A2(A2_y0),
        .A3(A3_y0),
        .b(b_y0)
    );

    assign zero_sign_k0 = {zero_signal_y0, sign_y0, k0};

    wire[37:0] CSA0_sum, CSA0_carry;
    wire[37:0] CSA0_operater [2:0];
    assign CSA0_operater[0] = (VEC == 1'b1) ? {16'b0, A0_y0} : CSA0_op[0];
    assign CSA0_operater[1] = (VEC == 1'b1) ? {16'b0, A1_y0} : CSA0_op[1];
    assign CSA0_operater[2] = (VEC == 1'b1) ? {16'b0, A2_y0} : CSA0_op[2];
    CSA_38 csa0(
        .a(CSA0_operater[0]),
        .b(CSA0_operater[1]),
        .c(CSA0_operater[2]),
        .sum(CSA0_sum),
        .carry(CSA0_carry)
    );

    wire[37:0] CSA1_sum, CSA1_carry;
    CSA_38 csa1(
        .a(CSA1_op[0]),
        .b(CSA1_op[1]),
        .c(CSA1_op[2]),
        .sum(CSA1_sum),
        .carry(CSA1_carry)
    );

    wire[37:0] CSA2_sum, CSA2_carry;
    CSA_38 csa2(
        .a(CSA2_op[0]),
        .b(CSA2_op[1]),
        .c(CSA2_op[2]),
        .sum(CSA2_sum),
        .carry(CSA2_carry)
    );

    wire[37:0] CSA3_sum, CSA3_carry;
    CSA_38 csa3(
        .a(CSA3_op[0]),
        .b(CSA3_op[1]),
        .c(CSA3_op[2]),
        .sum(CSA3_sum),
        .carry(CSA3_carry)
    );

    wire[37:0] CSA4_sum, CSA4_carry;
    CSA_38 csa4(
        .a(CSA4_op[0]),
        .b(CSA4_op[1]),
        .c(CSA4_op[2]),
        .sum(CSA4_sum),
        .carry(CSA4_carry)
    );

    wire[37:0] CSA5_sum, CSA5_carry;
    wire [37:0] CSA5_op_c;
    assign CSA5_op_c = (TRG == 1'b0) ? {CSA3_sum} : extend_log_c4;
    CSA_38 csa5(
        .a({CSA4_carry[36:0], 1'b0}),
        .b(CSA4_sum),
        .c(CSA5_op_c),
        .sum(CSA5_sum),
        .carry(CSA5_carry)
    );

    wire[37:0] CSA6_sum, CSA6_carry;
    wire [37:0] CSA6_op_c;
    assign CSA6_op_c = (TRG == 1'b0) ? {CSA3_carry[36:0], 1'b0} : extend_log_c2;
    CSA_38 csa6(
        .a(CSA2_sum),
        .b({CSA2_carry[36:0], 1'b0}),
        .c(CSA6_op_c),
        .sum(CSA6_sum),
        .carry(CSA6_carry)
    );

    wire[37:0] CSA7_sum, CSA7_carry;
    wire [37:0] CSA7_op_a;
    assign CSA7_op_a = (TRG == 1'b0) ? {CSA0_sum} : extend_log_c1;
    CSA_38 csa7(
        .a(CSA7_op_a),
        .b({CSA1_carry[36:0], 1'b0}),
        .c(CSA1_sum),
        .sum(CSA7_sum),
        .carry(CSA7_carry)
    );

    wire[37:0] CSA8_sum, CSA8_carry;
    wire [37:0] CSA8_op_a;
    assign CSA8_op_a = (TRG == 1'b0) ? {CSA6_sum} : {CSA3_carry[36:0], 1'b0};
    wire [37:0] CSA8_op_b;
    assign CSA8_op_b = (TRG == 1'b0) ? {CSA5_sum} : {CSA3_sum};
    wire [37:0] CSA8_op_c;
    assign CSA8_op_c = (TRG == 1'b0) ? {CSA5_carry[36:0], 1'b0} : extend_log_c3;
    CSA_38 csa8(
        .a(CSA8_op_a),
        .b(CSA8_op_b),
        .c(CSA8_op_c),
        .sum(CSA8_sum),
        .carry(CSA8_carry)
    );

    wire[37:0] CSA9_sum, CSA9_carry;
    wire [37:0] CSA9_op_b;
    assign CSA9_op_b = (TRG == 1'b0) ? {CSA7_carry[36:0], 1'b0} : {CSA0_sum};
    wire [37:0] CSA9_op_c;
    assign CSA9_op_c = (TRG == 1'b0) ? {CSA7_sum} : {(VEC == 1'b1) ? {16'b0, A3_y0} : {extend_log_c0}};
    CSA_38 csa9(
        .a({CSA0_carry[36:0], 1'b0}),
        .b(CSA9_op_b),
        .c(CSA9_op_c),
        .sum(CSA9_sum),
        .carry(CSA9_carry)
    );

    wire [37:0] CSA10_sum, CSA10_carry;
    CSA_38 csa10(
        .a({CSA6_carry[36:0], 1'b0}),
        .b({CSA8_sum}),
        .c({CSA8_carry[36:0], 1'b0}),
        .sum(CSA10_sum),
        .carry(CSA10_carry)
    );

    wire [37:0] CSA11_sum, CSA11_carry;
    CSA_38 csa11(
        .a(CSA9_sum),
        .b({CSA10_carry[36:0], 1'b0}),
        .c(CSA10_sum),
        .sum(CSA11_sum),
        .carry(CSA11_carry)
    );

    wire [37:0] CSA12_sum, CSA12_carry;
    wire [37:0] CSA12_op_b;
    assign CSA12_op_b = (VEC == 1'b1) ? {CSA9_sum} : {CSA11_carry[36:0], 1'b0};
    wire [37:0] CSA12_op_c;
    assign CSA12_op_c = (VEC == 1'b1) ? {16'b0, b_y0} : {CSA11_sum};
    CSA_38 csa12(
        .a({CSA9_carry[36:0], 1'b0}),
        .b(CSA12_op_b),
        .c(CSA12_op_c),
        .sum(CSA12_sum),
        .carry(CSA12_carry)
    );
    

    wire cpa0_select;
    assign cpa0_select = (TRG == 1'b0) || (VEC == 1'b1);
    wire [37:0] cpa0_a;
    wire [37:0] cpa0_b;
    assign cpa0_a = (cpa0_select == 1'b1) ? {CSA12_carry[36:0], 1'b0} : {CSA9_carry[36:0], 1'b0};
    assign cpa0_b = (cpa0_select == 1'b1) ? {CSA12_sum} : {CSA9_sum};
    wire [WIDTH+5:0] cpa_channel0_temp;

    CPA_38 cpa0(
        .a(cpa0_a),
        .b(cpa0_b),
        .sum(cpa_channel0_temp)
    );

    wire cpa_channel0_select;
    assign cpa_channel0_select = (VEC == 1'b1) && (A0_y0[21] == 1'b1) &&(cpa_channel0_temp[21]==1'b0);
    assign cpa_channel0 = (cpa_channel0_select == 1'b1) ? {cpa_channel0_temp[WIDTH+5:22], 22'b1111111111111111111111}: cpa_channel0_temp[WIDTH+5:0];

    CPA_38 cpa1(
        .a({CSA7_sum}),
        .b({CSA7_carry[36:0], 1'b0}),
        .sum(cpa_channel1)
    );

    CPA_38 cpa2(
        .a({CSA6_sum}),
        .b({CSA6_carry[36:0], 1'b0}),
        .sum(cpa_channel2)
    );

    CPA_38 cpa3(
        .a({CSA8_sum}),
        .b({CSA8_carry[36:0], 1'b0}),
        .sum(cpa_channel3)
    );

    CPA_38 cpa4(
        .a({CSA5_sum}),
        .b({CSA5_carry[36:0], 1'b0}),
        .sum(cpa_channel4)
    );
endmodule


module CSA_38 (
    input [37:0] a,
    input [37:0] b,
    input [37:0] c,
    output [37:0] sum,
    output [37:0] carry
);
    assign sum = a ^ b ^ c;          // XOR for sum
    assign carry = (a & b) | (b & c) | (c & a); // Majority logic for carry
endmodule

module CPA_38 (
    input [37:0] a,
    input [37:0] b,
    output [37:0] sum
);
    assign sum = a + b;
endmodule

module LOGC(
    input [31:0] y,
    input [4:0] n,
    input float_flag,
    output zero_signal,
    output sign,
    output [7:0] k,
    output [21:0] A0,
    output [21:0] A1,
    output [21:0] A2,
    output [21:0] A3,
    output [21:0] b
);
    wire [31:0] abs_y;
    wire [21:0] f;
    wire [4:0] k_;
    wire zero_signal0;

    ABS1 abs(
        .x_in(y),
        .sign(sign),
        .abs_x(abs_y)
    );

    LOD1 lod(
        .x_in(abs_y),
        .k(k_),
        .f(f),
        .zero_signal(zero_signal0)
    );
    wire [21:0] input_app_f;
    assign input_app_f = (float_flag == 1'b1) ? y[22:1] : f;

    APP_22_22_10_4_24_csa app(
        .f(input_app_f),
        .A0(A0),
        .A1(A1),
        .A2(A2),
        .A3(A3),
        .b(b)
    );

    wire [7:0] sub_a, sub_b;
    assign sub_a = (float_flag == 1'b0) ? {3'b0, k_} : y[30:23];
    assign sub_b = (float_flag == 1'b0) ? {3'b0, n} : 8'b01111111;
    assign k = sub_a - sub_b; 
    assign zero_signal = (float_flag == 1'b0 && zero_signal0==1'b1);
endmodule



module ABS1#(parameter WIDTH32 = 32  
)(
    input [WIDTH32-1:0] x_in,        // 输入 x 为 32 位定点数
    output reg sign,
    output reg [WIDTH32-1:0] abs_x // 输出 abs_x 为 x 的绝对值
);

    always @(*) begin
        if (x_in[WIDTH32-1] == 1'b1) begin      // 如果符号位为1，表示负数
            abs_x = ~x_in + 1'b1;  // 取反并加1，得到绝对值
            sign = 1'b1;
        end
        else
        begin
            abs_x = x_in;       // 如果符号位为0，表示正数，直接输出 x
            sign = 1'b0;
        end
    end
endmodule

//分组检测

module LOD1 #(parameter WIDTH32 = 32  
)(
    input [WIDTH32-1:0] x_in,
    output [4:0] k,
    output reg [21:0] f,
    output reg zero_signal
);
    CSA_leading_one_detector_32 leading_one_detector_1 (.in(x_in), .msb_pos(k));
    always @(*) begin
        if(x_in == 32'h000000)
        begin
            f = 22'b0;
            zero_signal = 1'b1;
        end
        else begin
            zero_signal = 1'b0;
            case (k) 
                5'b11111: f = x_in[30:9];
                5'b11110: f = x_in[29:8];
                5'b11101: f = x_in[28:7];
                5'b11100: f = x_in[27:6];
                5'b11011: f = x_in[26:5];
                5'b11010: f = x_in[25:4];
                5'b11001: f = x_in[24:3];
                5'b11000: f = x_in[23:2];
                5'b10111: f = x_in[22:1];
                5'b10110: f = x_in[21:0];
                5'b10101: f = {x_in[20:0], 1'b0};
                5'b10100: f = {x_in[19:0], 2'b0};
                5'b10011: f = {x_in[18:0], 3'b0};
                5'b10010: f = {x_in[17:0], 4'b0};
                5'b10001: f = {x_in[16:0], 5'b0};
                5'b10000: f = {x_in[15:0], 6'b0};
                5'b01111: f = {x_in[14:0], 7'b0};
                5'b01110: f = {x_in[13:0], 8'b0};
                5'b01101: f = {x_in[12:0], 9'b0};
                5'b01100: f = {x_in[11:0], 10'b0};
                5'b01011: f = {x_in[10:0], 11'b0};
                5'b01010: f = {x_in[9:0], 12'b0};
                5'b01001: f = {x_in[8:0], 13'b0};
                5'b01000: f = {x_in[7:0], 14'b0};
                5'b00111: f = {x_in[6:0], 15'b0};
                5'b00110: f = {x_in[5:0], 16'b0};
                5'b00101: f = {x_in[4:0], 17'b0};
                5'b00100: f = {x_in[3:0], 18'b0};
                5'b00011: f = {x_in[2:0], 19'b0};
                5'b00010: f = {x_in[1:0], 20'b0};
                5'b00001: f = {x_in[0], 21'b0};
                5'b00000: f = {22'b0};
                default:  f = {22'b0};
            endcase
        end
    end
endmodule



module CSA_leading_one_detector_4bit (
    input [3:0] in,      // 4-bit input
    output reg [1:0] msb_pos,  // MSB position within the 4 bits, 2-bit output (0-3)
    output reg signal
);
    always @(*) begin
        if(in == 4'b0000)
            signal = 1'b0;
        else
            signal = 1'b1;
        // casez (in)
        //     4'b1???: msb_pos = 2'd3;
        //     4'b01??: msb_pos = 2'd2;
        //     4'b001?: msb_pos = 2'd1;
        //     4'b0001: msb_pos = 2'd0;
        //     default: msb_pos = 2'd0;  // Default case (this should never happen)
        // endcase
        if(in[3]) msb_pos = 2'd3;
        else if(in[2]) msb_pos = 2'd2;
        else if(in[1]) msb_pos = 2'd1;
        else msb_pos = 2'd0;
    end
endmodule

module CSA_leading_one_detector_32 (
    input [31:0] in,     // 32-bit input
    output reg [4:0] msb_pos  // MSB position, 5-bit output (0-31)
);

    wire [1:0] msb_group_1, msb_group_2, msb_group_3, msb_group_4;
    wire [1:0] msb_group_5, msb_group_6, msb_group_7, msb_group_8;
    wire signal1, signal2, signal3, signal4;
    wire signal5, signal6, signal7, signal8;

    // Instantiate 8 4-bit Leading-One Detectors for each group
    CSA_leading_one_detector_4bit group1 (.in(in[31:28]), .msb_pos(msb_group_1), .signal(signal1));
    CSA_leading_one_detector_4bit group2 (.in(in[27:24]), .msb_pos(msb_group_2), .signal(signal2));
    CSA_leading_one_detector_4bit group3 (.in(in[23:20]), .msb_pos(msb_group_3), .signal(signal3));
    CSA_leading_one_detector_4bit group4 (.in(in[19:16]), .msb_pos(msb_group_4), .signal(signal4));
    CSA_leading_one_detector_4bit group5 (.in(in[15:12]), .msb_pos(msb_group_5), .signal(signal5));
    CSA_leading_one_detector_4bit group6 (.in(in[11:8]), .msb_pos(msb_group_6), .signal(signal6));
    CSA_leading_one_detector_4bit group7 (.in(in[7:4]), .msb_pos(msb_group_7), .signal(signal7));
    CSA_leading_one_detector_4bit group9 (.in(in[3:0]), .msb_pos(msb_group_8), .signal(signal8));
    always @(*) begin
        // Combine results from each group to determine the final MSB position
        if (signal1) msb_pos = {3'b111, msb_group_1};  // Position in the first group (0-3)
        else if (signal2) msb_pos = {3'b110, msb_group_2};
        else if (signal3) msb_pos = {3'b101, msb_group_3};
        else if (signal4) msb_pos = {3'b100, msb_group_4};
        else if (signal5) msb_pos = {3'b011, msb_group_5};
        else if (signal6) msb_pos = {3'b010, msb_group_6};
        else if (signal7) msb_pos = {3'b001, msb_group_7};
        else if (signal8) msb_pos = {3'b000, msb_group_8};
        else msb_pos = 5'b00000;  // Default case (this should never happen)
    end

endmodule


module  APP_22_22_10_4_24_csa(
    input [21:0] f,               // 22-bit input
    output [21:0] A0,          // 22-bit output A0
    output reg [21:0] A1,          // 22-bit output A1
    output reg [21:0] A2,          // 22-bit output A2
    output reg [21:0] A3,          // 22-bit output A3
    output reg [21:0] b            // 22-bit output b
);
    assign A0 = f;
    reg [9:0] segment;
    always @(*) begin
        case(f[21:12])
            10'b0000000000: begin segment = 5'd0; b = 22'b0000000000000000101011; end
            10'b0000000001: begin segment = 5'd0; b = 22'b0000000000000001111010; end
            10'b0000000010: begin segment = 5'd0; b = 22'b0000000000000011000100; end
            10'b0000000011: begin segment = 5'd0; b = 22'b0000000000000100001000; end
            10'b0000000100: begin segment = 5'd0; b = 22'b0000000000000101000110; end
            10'b0000000101: begin segment = 5'd0; b = 22'b0000000000000101111111; end
            10'b0000000110: begin segment = 5'd0; b = 22'b0000000000000110110010; end
            10'b0000000111: begin segment = 5'd0; b = 22'b0000000000000111011111; end
            10'b0000001000: begin segment = 5'd0; b = 22'b0000000000001000000110; end
            10'b0000001001: begin segment = 5'd0; b = 22'b0000000000001000101000; end
            10'b0000001010: begin segment = 5'd0; b = 22'b0000000000001001000100; end
            10'b0000001011: begin segment = 5'd0; b = 22'b0000000000001001011011; end
            10'b0000001100: begin segment = 5'd0; b = 22'b0000000000001001101100; end
            10'b0000001101: begin segment = 5'd0; b = 22'b0000000000001001110111; end
            10'b0000001110: begin segment = 5'd0; b = 22'b0000000000001001111101; end
            10'b0000001111: begin segment = 5'd0; b = 22'b0000000000001001111101; end
            10'b0000010000: begin segment = 5'd0; b = 22'b0000000000001001110111; end
            10'b0000010001: begin segment = 5'd0; b = 22'b0000000000001001101100; end
            10'b0000010010: begin segment = 5'd0; b = 22'b0000000000001001011011; end
            10'b0000010011: begin segment = 5'd0; b = 22'b0000000000001001000100; end
            10'b0000010100: begin segment = 5'd0; b = 22'b0000000000001000101000; end
            10'b0000010101: begin segment = 5'd0; b = 22'b0000000000001000000111; end
            10'b0000010110: begin segment = 5'd0; b = 22'b0000000000000111100000; end
            10'b0000010111: begin segment = 5'd0; b = 22'b0000000000000110110011; end
            10'b0000011000: begin segment = 5'd0; b = 22'b0000000000000110000001; end
            10'b0000011001: begin segment = 5'd0; b = 22'b0000000000000101001010; end
            10'b0000011010: begin segment = 5'd0; b = 22'b0000000000000100001101; end
            10'b0000011011: begin segment = 5'd0; b = 22'b0000000000000011001010; end
            10'b0000011100: begin segment = 5'd0; b = 22'b0000000000000010000010; end
            10'b0000011101: begin segment = 5'd0; b = 22'b0000000000000000110101; end
            10'b0000011110: begin segment = 5'd1; b = 22'b0000000001001011110010; end
            10'b0000011111: begin segment = 5'd1; b = 22'b0000000001001100111001; end
            10'b0000100000: begin segment = 5'd1; b = 22'b0000000001001101111100; end
            10'b0000100001: begin segment = 5'd1; b = 22'b0000000001001110111000; end
            10'b0000100010: begin segment = 5'd1; b = 22'b0000000001001111110000; end
            10'b0000100011: begin segment = 5'd1; b = 22'b0000000001010000100010; end
            10'b0000100100: begin segment = 5'd1; b = 22'b0000000001010001001110; end
            10'b0000100101: begin segment = 5'd1; b = 22'b0000000001010001110110; end
            10'b0000100110: begin segment = 5'd1; b = 22'b0000000001010010010111; end
            10'b0000100111: begin segment = 5'd1; b = 22'b0000000001010010110100; end
            10'b0000101000: begin segment = 5'd1; b = 22'b0000000001010011001011; end
            10'b0000101001: begin segment = 5'd1; b = 22'b0000000001010011011101; end
            10'b0000101010: begin segment = 5'd1; b = 22'b0000000001010011101001; end
            10'b0000101011: begin segment = 5'd1; b = 22'b0000000001010011110000; end
            10'b0000101100: begin segment = 5'd1; b = 22'b0000000001010011110010; end
            10'b0000101101: begin segment = 5'd1; b = 22'b0000000001010011101111; end
            10'b0000101110: begin segment = 5'd1; b = 22'b0000000001010011100110; end
            10'b0000101111: begin segment = 5'd1; b = 22'b0000000001010011011000; end
            10'b0000110000: begin segment = 5'd1; b = 22'b0000000001010011000101; end
            10'b0000110001: begin segment = 5'd1; b = 22'b0000000001010010101100; end
            10'b0000110010: begin segment = 5'd1; b = 22'b0000000001010010001110; end
            10'b0000110011: begin segment = 5'd1; b = 22'b0000000001010001101011; end
            10'b0000110100: begin segment = 5'd1; b = 22'b0000000001010001000011; end
            10'b0000110101: begin segment = 5'd1; b = 22'b0000000001010000010101; end
            10'b0000110110: begin segment = 5'd1; b = 22'b0000000001001111100010; end
            10'b0000110111: begin segment = 5'd1; b = 22'b0000000001001110101010; end
            10'b0000111000: begin segment = 5'd1; b = 22'b0000000001001101101101; end
            10'b0000111001: begin segment = 5'd1; b = 22'b0000000001001100101011; end
            10'b0000111010: begin segment = 5'd1; b = 22'b0000000001001011100011; end
            10'b0000111011: begin segment = 5'd1; b = 22'b0000000001001010010111; end
            10'b0000111100: begin segment = 5'd2; b = 22'b0000000011100000010101; end
            10'b0000111101: begin segment = 5'd2; b = 22'b0000000011100001011110; end
            10'b0000111110: begin segment = 5'd2; b = 22'b0000000011100010100010; end
            10'b0000111111: begin segment = 5'd2; b = 22'b0000000011100011100001; end
            10'b0001000000: begin segment = 5'd2; b = 22'b0000000011100100011011; end
            10'b0001000001: begin segment = 5'd2; b = 22'b0000000011100101001111; end
            10'b0001000010: begin segment = 5'd2; b = 22'b0000000011100101111111; end
            10'b0001000011: begin segment = 5'd2; b = 22'b0000000011100110101001; end
            10'b0001000100: begin segment = 5'd2; b = 22'b0000000011100111001111; end
            10'b0001000101: begin segment = 5'd2; b = 22'b0000000011100111101111; end
            10'b0001000110: begin segment = 5'd2; b = 22'b0000000011101000001010; end
            10'b0001000111: begin segment = 5'd2; b = 22'b0000000011101000100000; end
            10'b0001001000: begin segment = 5'd2; b = 22'b0000000011101000110001; end
            10'b0001001001: begin segment = 5'd2; b = 22'b0000000011101000111101; end
            10'b0001001010: begin segment = 5'd2; b = 22'b0000000011101001000100; end
            10'b0001001011: begin segment = 5'd2; b = 22'b0000000011101001000111; end
            10'b0001001100: begin segment = 5'd2; b = 22'b0000000011101001000011; end
            10'b0001001101: begin segment = 5'd2; b = 22'b0000000011101000111011; end
            10'b0001001110: begin segment = 5'd2; b = 22'b0000000011101000101110; end
            10'b0001001111: begin segment = 5'd2; b = 22'b0000000011101000011100; end
            10'b0001010000: begin segment = 5'd2; b = 22'b0000000011101000000101; end
            10'b0001010001: begin segment = 5'd2; b = 22'b0000000011100111101001; end
            10'b0001010010: begin segment = 5'd2; b = 22'b0000000011100111001001; end
            10'b0001010011: begin segment = 5'd2; b = 22'b0000000011100110100011; end
            10'b0001010100: begin segment = 5'd2; b = 22'b0000000011100101111000; end
            10'b0001010101: begin segment = 5'd2; b = 22'b0000000011100101001001; end
            10'b0001010110: begin segment = 5'd2; b = 22'b0000000011100100010100; end
            10'b0001010111: begin segment = 5'd2; b = 22'b0000000011100011011010; end
            10'b0001011000: begin segment = 5'd2; b = 22'b0000000011100010011100; end
            10'b0001011001: begin segment = 5'd2; b = 22'b0000000011100001011001; end
            10'b0001011010: begin segment = 5'd2; b = 22'b0000000011100000010001; end
            10'b0001011011: begin segment = 5'd2; b = 22'b0000000011011111000100; end
            10'b0001011100: begin segment = 5'd3; b = 22'b0000000111000101000010; end
            10'b0001011101: begin segment = 5'd3; b = 22'b0000000111000110001011; end
            10'b0001011110: begin segment = 5'd3; b = 22'b0000000111000111001111; end
            10'b0001011111: begin segment = 5'd3; b = 22'b0000000111001000001111; end
            10'b0001100000: begin segment = 5'd3; b = 22'b0000000111001001001010; end
            10'b0001100001: begin segment = 5'd3; b = 22'b0000000111001010000000; end
            10'b0001100010: begin segment = 5'd3; b = 22'b0000000111001010110001; end
            10'b0001100011: begin segment = 5'd3; b = 22'b0000000111001011011101; end
            10'b0001100100: begin segment = 5'd3; b = 22'b0000000111001100000101; end
            10'b0001100101: begin segment = 5'd3; b = 22'b0000000111001100100111; end
            10'b0001100110: begin segment = 5'd3; b = 22'b0000000111001101000101; end
            10'b0001100111: begin segment = 5'd3; b = 22'b0000000111001101011111; end
            10'b0001101000: begin segment = 5'd3; b = 22'b0000000111001101110011; end
            10'b0001101001: begin segment = 5'd3; b = 22'b0000000111001110000011; end
            10'b0001101010: begin segment = 5'd3; b = 22'b0000000111001110001110; end
            10'b0001101011: begin segment = 5'd3; b = 22'b0000000111001110010100; end
            10'b0001101100: begin segment = 5'd3; b = 22'b0000000111001110010110; end
            10'b0001101101: begin segment = 5'd3; b = 22'b0000000111001110010011; end
            10'b0001101110: begin segment = 5'd3; b = 22'b0000000111001110001011; end
            10'b0001101111: begin segment = 5'd3; b = 22'b0000000111001101111110; end
            10'b0001110000: begin segment = 5'd3; b = 22'b0000000111001101101101; end
            10'b0001110001: begin segment = 5'd3; b = 22'b0000000111001101010110; end
            10'b0001110010: begin segment = 5'd3; b = 22'b0000000111001100111100; end
            10'b0001110011: begin segment = 5'd3; b = 22'b0000000111001100011100; end
            10'b0001110100: begin segment = 5'd3; b = 22'b0000000111001011111000; end
            10'b0001110101: begin segment = 5'd3; b = 22'b0000000111001011001111; end
            10'b0001110110: begin segment = 5'd3; b = 22'b0000000111001010100010; end
            10'b0001110111: begin segment = 5'd3; b = 22'b0000000111001001110000; end
            10'b0001111000: begin segment = 5'd3; b = 22'b0000000111001000111010; end
            10'b0001111001: begin segment = 5'd3; b = 22'b0000000111000111111110; end
            10'b0001111010: begin segment = 5'd3; b = 22'b0000000111000110111111; end
            10'b0001111011: begin segment = 5'd3; b = 22'b0000000111000101111010; end
            10'b0001111100: begin segment = 5'd3; b = 22'b0000000111000100110001; end
            10'b0001111101: begin segment = 5'd4; b = 22'b0000001011101101101000; end
            10'b0001111110: begin segment = 5'd4; b = 22'b0000001011101110101110; end
            10'b0001111111: begin segment = 5'd4; b = 22'b0000001011101111101111; end
            10'b0010000000: begin segment = 5'd4; b = 22'b0000001011110000101100; end
            10'b0010000001: begin segment = 5'd4; b = 22'b0000001011110001100100; end
            10'b0010000010: begin segment = 5'd4; b = 22'b0000001011110010010111; end
            10'b0010000011: begin segment = 5'd4; b = 22'b0000001011110011000110; end
            10'b0010000100: begin segment = 5'd4; b = 22'b0000001011110011110001; end
            10'b0010000101: begin segment = 5'd4; b = 22'b0000001011110100010111; end
            10'b0010000110: begin segment = 5'd4; b = 22'b0000001011110100111000; end
            10'b0010000111: begin segment = 5'd4; b = 22'b0000001011110101010101; end
            10'b0010001000: begin segment = 5'd4; b = 22'b0000001011110101101110; end
            10'b0010001001: begin segment = 5'd4; b = 22'b0000001011110110000010; end
            10'b0010001010: begin segment = 5'd4; b = 22'b0000001011110110010001; end
            10'b0010001011: begin segment = 5'd4; b = 22'b0000001011110110011100; end
            10'b0010001100: begin segment = 5'd4; b = 22'b0000001011110110100011; end
            10'b0010001101: begin segment = 5'd4; b = 22'b0000001011110110100101; end
            10'b0010001110: begin segment = 5'd4; b = 22'b0000001011110110100010; end
            10'b0010001111: begin segment = 5'd4; b = 22'b0000001011110110011011; end
            10'b0010010000: begin segment = 5'd4; b = 22'b0000001011110110010000; end
            10'b0010010001: begin segment = 5'd4; b = 22'b0000001011110110000000; end
            10'b0010010010: begin segment = 5'd4; b = 22'b0000001011110101101100; end
            10'b0010010011: begin segment = 5'd4; b = 22'b0000001011110101010011; end
            10'b0010010100: begin segment = 5'd4; b = 22'b0000001011110100110110; end
            10'b0010010101: begin segment = 5'd4; b = 22'b0000001011110100010101; end
            10'b0010010110: begin segment = 5'd4; b = 22'b0000001011110011101111; end
            10'b0010010111: begin segment = 5'd4; b = 22'b0000001011110011000101; end
            10'b0010011000: begin segment = 5'd4; b = 22'b0000001011110010010110; end
            10'b0010011001: begin segment = 5'd4; b = 22'b0000001011110001100011; end
            10'b0010011010: begin segment = 5'd4; b = 22'b0000001011110000101100; end
            10'b0010011011: begin segment = 5'd4; b = 22'b0000001011101111110001; end
            10'b0010011100: begin segment = 5'd4; b = 22'b0000001011101110110001; end
            10'b0010011101: begin segment = 5'd4; b = 22'b0000001011101101101100; end
            10'b0010011110: begin segment = 5'd4; b = 22'b0000001011101100100100; end
            10'b0010011111: begin segment = 5'd5; b = 22'b0000010001100110001100; end
            10'b0010100000: begin segment = 5'd5; b = 22'b0000010001100111010011; end
            10'b0010100001: begin segment = 5'd5; b = 22'b0000010001101000010101; end
            10'b0010100010: begin segment = 5'd5; b = 22'b0000010001101001010011; end
            10'b0010100011: begin segment = 5'd5; b = 22'b0000010001101010001101; end
            10'b0010100100: begin segment = 5'd5; b = 22'b0000010001101011000011; end
            10'b0010100101: begin segment = 5'd5; b = 22'b0000010001101011110100; end
            10'b0010100110: begin segment = 5'd5; b = 22'b0000010001101100100001; end
            10'b0010100111: begin segment = 5'd5; b = 22'b0000010001101101001010; end
            10'b0010101000: begin segment = 5'd5; b = 22'b0000010001101101101110; end
            10'b0010101001: begin segment = 5'd5; b = 22'b0000010001101110001110; end
            10'b0010101010: begin segment = 5'd5; b = 22'b0000010001101110101010; end
            10'b0010101011: begin segment = 5'd5; b = 22'b0000010001101111000010; end
            10'b0010101100: begin segment = 5'd5; b = 22'b0000010001101111010101; end
            10'b0010101101: begin segment = 5'd5; b = 22'b0000010001101111100101; end
            10'b0010101110: begin segment = 5'd5; b = 22'b0000010001101111110000; end
            10'b0010101111: begin segment = 5'd5; b = 22'b0000010001101111110110; end
            10'b0010110000: begin segment = 5'd5; b = 22'b0000010001101111111001; end
            10'b0010110001: begin segment = 5'd5; b = 22'b0000010001101111110111; end
            10'b0010110010: begin segment = 5'd5; b = 22'b0000010001101111110010; end
            10'b0010110011: begin segment = 5'd5; b = 22'b0000010001101111101000; end
            10'b0010110100: begin segment = 5'd5; b = 22'b0000010001101111011001; end
            10'b0010110101: begin segment = 5'd5; b = 22'b0000010001101111000111; end
            10'b0010110110: begin segment = 5'd5; b = 22'b0000010001101110110001; end
            10'b0010110111: begin segment = 5'd5; b = 22'b0000010001101110010110; end
            10'b0010111000: begin segment = 5'd5; b = 22'b0000010001101101110111; end
            10'b0010111001: begin segment = 5'd5; b = 22'b0000010001101101010100; end
            10'b0010111010: begin segment = 5'd5; b = 22'b0000010001101100101101; end
            10'b0010111011: begin segment = 5'd5; b = 22'b0000010001101100000010; end
            10'b0010111100: begin segment = 5'd5; b = 22'b0000010001101011010010; end
            10'b0010111101: begin segment = 5'd5; b = 22'b0000010001101010011111; end
            10'b0010111110: begin segment = 5'd5; b = 22'b0000010001101001100111; end
            10'b0010111111: begin segment = 5'd5; b = 22'b0000010001101000101100; end
            10'b0011000000: begin segment = 5'd5; b = 22'b0000010001100111101100; end
            10'b0011000001: begin segment = 5'd6; b = 22'b0000011000011010000000; end
            10'b0011000010: begin segment = 5'd6; b = 22'b0000011000011011001000; end
            10'b0011000011: begin segment = 5'd6; b = 22'b0000011000011100001100; end
            10'b0011000100: begin segment = 5'd6; b = 22'b0000011000011101001100; end
            10'b0011000101: begin segment = 5'd6; b = 22'b0000011000011110001000; end
            10'b0011000110: begin segment = 5'd6; b = 22'b0000011000011111000000; end
            10'b0011000111: begin segment = 5'd6; b = 22'b0000011000011111110011; end
            10'b0011001000: begin segment = 5'd6; b = 22'b0000011000100000100011; end
            10'b0011001001: begin segment = 5'd6; b = 22'b0000011000100001001111; end
            10'b0011001010: begin segment = 5'd6; b = 22'b0000011000100001110111; end
            10'b0011001011: begin segment = 5'd6; b = 22'b0000011000100010011010; end
            10'b0011001100: begin segment = 5'd6; b = 22'b0000011000100010111010; end
            10'b0011001101: begin segment = 5'd6; b = 22'b0000011000100011010101; end
            10'b0011001110: begin segment = 5'd6; b = 22'b0000011000100011101101; end
            10'b0011001111: begin segment = 5'd6; b = 22'b0000011000100100000001; end
            10'b0011010000: begin segment = 5'd6; b = 22'b0000011000100100010000; end
            10'b0011010001: begin segment = 5'd6; b = 22'b0000011000100100011100; end
            10'b0011010010: begin segment = 5'd6; b = 22'b0000011000100100100011; end
            10'b0011010011: begin segment = 5'd6; b = 22'b0000011000100100100111; end
            10'b0011010100: begin segment = 5'd6; b = 22'b0000011000100100100111; end
            10'b0011010101: begin segment = 5'd6; b = 22'b0000011000100100100011; end
            10'b0011010110: begin segment = 5'd6; b = 22'b0000011000100100011010; end
            10'b0011010111: begin segment = 5'd6; b = 22'b0000011000100100001110; end
            10'b0011011000: begin segment = 5'd6; b = 22'b0000011000100011111110; end
            10'b0011011001: begin segment = 5'd6; b = 22'b0000011000100011101010; end
            10'b0011011010: begin segment = 5'd6; b = 22'b0000011000100011010010; end
            10'b0011011011: begin segment = 5'd6; b = 22'b0000011000100010110110; end
            10'b0011011100: begin segment = 5'd6; b = 22'b0000011000100010010110; end
            10'b0011011101: begin segment = 5'd6; b = 22'b0000011000100001110011; end
            10'b0011011110: begin segment = 5'd6; b = 22'b0000011000100001001011; end
            10'b0011011111: begin segment = 5'd6; b = 22'b0000011000100000011111; end
            10'b0011100000: begin segment = 5'd6; b = 22'b0000011000011111110000; end
            10'b0011100001: begin segment = 5'd6; b = 22'b0000011000011110111101; end
            10'b0011100010: begin segment = 5'd6; b = 22'b0000011000011110000110; end
            10'b0011100011: begin segment = 5'd6; b = 22'b0000011000011101001011; end
            10'b0011100100: begin segment = 5'd6; b = 22'b0000011000011100001100; end
            10'b0011100101: begin segment = 5'd7; b = 22'b0000011111100110001001; end
            10'b0011100110: begin segment = 5'd7; b = 22'b0000011111100111000011; end
            10'b0011100111: begin segment = 5'd7; b = 22'b0000011111100111111000; end
            10'b0011101000: begin segment = 5'd7; b = 22'b0000011111101000101010; end
            10'b0011101001: begin segment = 5'd7; b = 22'b0000011111101001011000; end
            10'b0011101010: begin segment = 5'd7; b = 22'b0000011111101010000010; end
            10'b0011101011: begin segment = 5'd7; b = 22'b0000011111101010101000; end
            10'b0011101100: begin segment = 5'd7; b = 22'b0000011111101011001011; end
            10'b0011101101: begin segment = 5'd7; b = 22'b0000011111101011101010; end
            10'b0011101110: begin segment = 5'd7; b = 22'b0000011111101100000100; end
            10'b0011101111: begin segment = 5'd7; b = 22'b0000011111101100011011; end
            10'b0011110000: begin segment = 5'd7; b = 22'b0000011111101100101111; end
            10'b0011110001: begin segment = 5'd7; b = 22'b0000011111101100111110; end
            10'b0011110010: begin segment = 5'd7; b = 22'b0000011111101101001010; end
            10'b0011110011: begin segment = 5'd7; b = 22'b0000011111101101010010; end
            10'b0011110100: begin segment = 5'd7; b = 22'b0000011111101101010110; end
            10'b0011110101: begin segment = 5'd7; b = 22'b0000011111101101010110; end
            10'b0011110110: begin segment = 5'd7; b = 22'b0000011111101101010011; end
            10'b0011110111: begin segment = 5'd7; b = 22'b0000011111101101001100; end
            10'b0011111000: begin segment = 5'd7; b = 22'b0000011111101101000001; end
            10'b0011111001: begin segment = 5'd7; b = 22'b0000011111101100110010; end
            10'b0011111010: begin segment = 5'd7; b = 22'b0000011111101100100000; end
            10'b0011111011: begin segment = 5'd7; b = 22'b0000011111101100001010; end
            10'b0011111100: begin segment = 5'd7; b = 22'b0000011111101011110000; end
            10'b0011111101: begin segment = 5'd7; b = 22'b0000011111101011010010; end
            10'b0011111110: begin segment = 5'd7; b = 22'b0000011111101010110001; end
            10'b0011111111: begin segment = 5'd7; b = 22'b0000011111101010001100; end
            10'b0100000000: begin segment = 5'd7; b = 22'b0000011111101001100100; end
            10'b0100000001: begin segment = 5'd7; b = 22'b0000011111101000110111; end
            10'b0100000010: begin segment = 5'd7; b = 22'b0000011111101000000111; end
            10'b0100000011: begin segment = 5'd7; b = 22'b0000011111100111010100; end
            10'b0100000100: begin segment = 5'd7; b = 22'b0000011111100110011100; end
            10'b0100000101: begin segment = 5'd7; b = 22'b0000011111100101100001; end
            10'b0100000110: begin segment = 5'd7; b = 22'b0000011111100100100011; end
            10'b0100000111: begin segment = 5'd7; b = 22'b0000011111100011100001; end
            10'b0100001000: begin segment = 5'd7; b = 22'b0000011111100010011011; end
            10'b0100001001: begin segment = 5'd7; b = 22'b0000011111100001010001; end
            10'b0100001010: begin segment = 5'd8; b = 22'b0000101000110011100010; end
            10'b0100001011: begin segment = 5'd8; b = 22'b0000101000110100100000; end
            10'b0100001100: begin segment = 5'd8; b = 22'b0000101000110101011011; end
            10'b0100001101: begin segment = 5'd8; b = 22'b0000101000110110010010; end
            10'b0100001110: begin segment = 5'd8; b = 22'b0000101000110111000101; end
            10'b0100001111: begin segment = 5'd8; b = 22'b0000101000110111110101; end
            10'b0100010000: begin segment = 5'd8; b = 22'b0000101000111000100001; end
            10'b0100010001: begin segment = 5'd8; b = 22'b0000101000111001001001; end
            10'b0100010010: begin segment = 5'd8; b = 22'b0000101000111001101110; end
            10'b0100010011: begin segment = 5'd8; b = 22'b0000101000111010001111; end
            10'b0100010100: begin segment = 5'd8; b = 22'b0000101000111010101101; end
            10'b0100010101: begin segment = 5'd8; b = 22'b0000101000111011000111; end
            10'b0100010110: begin segment = 5'd8; b = 22'b0000101000111011011110; end
            10'b0100010111: begin segment = 5'd8; b = 22'b0000101000111011110001; end
            10'b0100011000: begin segment = 5'd8; b = 22'b0000101000111100000000; end
            10'b0100011001: begin segment = 5'd8; b = 22'b0000101000111100001100; end
            10'b0100011010: begin segment = 5'd8; b = 22'b0000101000111100010100; end
            10'b0100011011: begin segment = 5'd8; b = 22'b0000101000111100011001; end
            10'b0100011100: begin segment = 5'd8; b = 22'b0000101000111100011010; end
            10'b0100011101: begin segment = 5'd8; b = 22'b0000101000111100010111; end
            10'b0100011110: begin segment = 5'd8; b = 22'b0000101000111100010001; end
            10'b0100011111: begin segment = 5'd8; b = 22'b0000101000111100001000; end
            10'b0100100000: begin segment = 5'd8; b = 22'b0000101000111011111011; end
            10'b0100100001: begin segment = 5'd8; b = 22'b0000101000111011101011; end
            10'b0100100010: begin segment = 5'd8; b = 22'b0000101000111011010111; end
            10'b0100100011: begin segment = 5'd8; b = 22'b0000101000111010111111; end
            10'b0100100100: begin segment = 5'd8; b = 22'b0000101000111010100100; end
            10'b0100100101: begin segment = 5'd8; b = 22'b0000101000111010000110; end
            10'b0100100110: begin segment = 5'd8; b = 22'b0000101000111001100100; end
            10'b0100100111: begin segment = 5'd8; b = 22'b0000101000111000111110; end
            10'b0100101000: begin segment = 5'd8; b = 22'b0000101000111000010110; end
            10'b0100101001: begin segment = 5'd8; b = 22'b0000101000110111101001; end
            10'b0100101010: begin segment = 5'd8; b = 22'b0000101000110110111001; end
            10'b0100101011: begin segment = 5'd8; b = 22'b0000101000110110000110; end
            10'b0100101100: begin segment = 5'd8; b = 22'b0000101000110101010000; end
            10'b0100101101: begin segment = 5'd8; b = 22'b0000101000110100010101; end
            10'b0100101110: begin segment = 5'd8; b = 22'b0000101000110011011000; end
            10'b0100101111: begin segment = 5'd9; b = 22'b0000110010010110001000; end
            10'b0100110000: begin segment = 5'd9; b = 22'b0000110010010111000100; end
            10'b0100110001: begin segment = 5'd9; b = 22'b0000110010010111111101; end
            10'b0100110010: begin segment = 5'd9; b = 22'b0000110010011000110011; end
            10'b0100110011: begin segment = 5'd9; b = 22'b0000110010011001100101; end
            10'b0100110100: begin segment = 5'd9; b = 22'b0000110010011010010100; end
            10'b0100110101: begin segment = 5'd9; b = 22'b0000110010011011000000; end
            10'b0100110110: begin segment = 5'd9; b = 22'b0000110010011011101000; end
            10'b0100110111: begin segment = 5'd9; b = 22'b0000110010011100001100; end
            10'b0100111000: begin segment = 5'd9; b = 22'b0000110010011100101110; end
            10'b0100111001: begin segment = 5'd9; b = 22'b0000110010011101001100; end
            10'b0100111010: begin segment = 5'd9; b = 22'b0000110010011101100110; end
            10'b0100111011: begin segment = 5'd9; b = 22'b0000110010011101111101; end
            10'b0100111100: begin segment = 5'd9; b = 22'b0000110010011110010001; end
            10'b0100111101: begin segment = 5'd9; b = 22'b0000110010011110100001; end
            10'b0100111110: begin segment = 5'd9; b = 22'b0000110010011110101110; end
            10'b0100111111: begin segment = 5'd9; b = 22'b0000110010011110111000; end
            10'b0101000000: begin segment = 5'd9; b = 22'b0000110010011110111110; end
            10'b0101000001: begin segment = 5'd9; b = 22'b0000110010011111000010; end
            10'b0101000010: begin segment = 5'd9; b = 22'b0000110010011111000001; end
            10'b0101000011: begin segment = 5'd9; b = 22'b0000110010011110111101; end
            10'b0101000100: begin segment = 5'd9; b = 22'b0000110010011110110110; end
            10'b0101000101: begin segment = 5'd9; b = 22'b0000110010011110101100; end
            10'b0101000110: begin segment = 5'd9; b = 22'b0000110010011110011110; end
            10'b0101000111: begin segment = 5'd9; b = 22'b0000110010011110001101; end
            10'b0101001000: begin segment = 5'd9; b = 22'b0000110010011101111000; end
            10'b0101001001: begin segment = 5'd9; b = 22'b0000110010011101100001; end
            10'b0101001010: begin segment = 5'd9; b = 22'b0000110010011101000110; end
            10'b0101001011: begin segment = 5'd9; b = 22'b0000110010011100101000; end
            10'b0101001100: begin segment = 5'd9; b = 22'b0000110010011100000110; end
            10'b0101001101: begin segment = 5'd9; b = 22'b0000110010011011100001; end
            10'b0101001110: begin segment = 5'd9; b = 22'b0000110010011010111001; end
            10'b0101001111: begin segment = 5'd9; b = 22'b0000110010011010001110; end
            10'b0101010000: begin segment = 5'd9; b = 22'b0000110010011001011111; end
            10'b0101010001: begin segment = 5'd9; b = 22'b0000110010011000101101; end
            10'b0101010010: begin segment = 5'd9; b = 22'b0000110010010111111000; end
            10'b0101010011: begin segment = 5'd9; b = 22'b0000110010010110111111; end
            10'b0101010100: begin segment = 5'd9; b = 22'b0000110010010110000100; end
            10'b0101010101: begin segment = 5'd9; b = 22'b0000110010010101000101; end
            10'b0101010110: begin segment = 5'd10; b = 22'b0000111101001011110000; end
            10'b0101010111: begin segment = 5'd10; b = 22'b0000111101001100101100; end
            10'b0101011000: begin segment = 5'd10; b = 22'b0000111101001101100110; end
            10'b0101011001: begin segment = 5'd10; b = 22'b0000111101001110011100; end
            10'b0101011010: begin segment = 5'd10; b = 22'b0000111101001111001111; end
            10'b0101011011: begin segment = 5'd10; b = 22'b0000111101001111111110; end
            10'b0101011100: begin segment = 5'd10; b = 22'b0000111101010000101011; end
            10'b0101011101: begin segment = 5'd10; b = 22'b0000111101010001010100; end
            10'b0101011110: begin segment = 5'd10; b = 22'b0000111101010001111010; end
            10'b0101011111: begin segment = 5'd10; b = 22'b0000111101010010011101; end
            10'b0101100000: begin segment = 5'd10; b = 22'b0000111101010010111100; end
            10'b0101100001: begin segment = 5'd10; b = 22'b0000111101010011011001; end
            10'b0101100010: begin segment = 5'd10; b = 22'b0000111101010011110010; end
            10'b0101100011: begin segment = 5'd10; b = 22'b0000111101010100001000; end
            10'b0101100100: begin segment = 5'd10; b = 22'b0000111101010100011011; end
            10'b0101100101: begin segment = 5'd10; b = 22'b0000111101010100101010; end
            10'b0101100110: begin segment = 5'd10; b = 22'b0000111101010100110111; end
            10'b0101100111: begin segment = 5'd10; b = 22'b0000111101010101000000; end
            10'b0101101000: begin segment = 5'd10; b = 22'b0000111101010101000111; end
            10'b0101101001: begin segment = 5'd10; b = 22'b0000111101010101001010; end
            10'b0101101010: begin segment = 5'd10; b = 22'b0000111101010101001010; end
            10'b0101101011: begin segment = 5'd10; b = 22'b0000111101010101000110; end
            10'b0101101100: begin segment = 5'd10; b = 22'b0000111101010101000000; end
            10'b0101101101: begin segment = 5'd10; b = 22'b0000111101010100110110; end
            10'b0101101110: begin segment = 5'd10; b = 22'b0000111101010100101010; end
            10'b0101101111: begin segment = 5'd10; b = 22'b0000111101010100011010; end
            10'b0101110000: begin segment = 5'd10; b = 22'b0000111101010100000111; end
            10'b0101110001: begin segment = 5'd10; b = 22'b0000111101010011110001; end
            10'b0101110010: begin segment = 5'd10; b = 22'b0000111101010011010111; end
            10'b0101110011: begin segment = 5'd10; b = 22'b0000111101010010111011; end
            10'b0101110100: begin segment = 5'd10; b = 22'b0000111101010010011100; end
            10'b0101110101: begin segment = 5'd10; b = 22'b0000111101010001111001; end
            10'b0101110110: begin segment = 5'd10; b = 22'b0000111101010001010100; end
            10'b0101110111: begin segment = 5'd10; b = 22'b0000111101010000101011; end
            10'b0101111000: begin segment = 5'd10; b = 22'b0000111101001111111111; end
            10'b0101111001: begin segment = 5'd10; b = 22'b0000111101001111010000; end
            10'b0101111010: begin segment = 5'd10; b = 22'b0000111101001110011110; end
            10'b0101111011: begin segment = 5'd10; b = 22'b0000111101001101101001; end
            10'b0101111100: begin segment = 5'd10; b = 22'b0000111101001100110001; end
            10'b0101111101: begin segment = 5'd10; b = 22'b0000111101001011110110; end
            10'b0101111110: begin segment = 5'd11; b = 22'b0001001000110101111100; end
            10'b0101111111: begin segment = 5'd11; b = 22'b0001001000110110111000; end
            10'b0110000000: begin segment = 5'd11; b = 22'b0001001000110111110001; end
            10'b0110000001: begin segment = 5'd11; b = 22'b0001001000111000100110; end
            10'b0110000010: begin segment = 5'd11; b = 22'b0001001000111001011001; end
            10'b0110000011: begin segment = 5'd11; b = 22'b0001001000111010001000; end
            10'b0110000100: begin segment = 5'd11; b = 22'b0001001000111010110101; end
            10'b0110000101: begin segment = 5'd11; b = 22'b0001001000111011011110; end
            10'b0110000110: begin segment = 5'd11; b = 22'b0001001000111100000101; end
            10'b0110000111: begin segment = 5'd11; b = 22'b0001001000111100101000; end
            10'b0110001000: begin segment = 5'd11; b = 22'b0001001000111101001001; end
            10'b0110001001: begin segment = 5'd11; b = 22'b0001001000111101100110; end
            10'b0110001010: begin segment = 5'd11; b = 22'b0001001000111110000000; end
            10'b0110001011: begin segment = 5'd11; b = 22'b0001001000111110011000; end
            10'b0110001100: begin segment = 5'd11; b = 22'b0001001000111110101100; end
            10'b0110001101: begin segment = 5'd11; b = 22'b0001001000111110111101; end
            10'b0110001110: begin segment = 5'd11; b = 22'b0001001000111111001100; end
            10'b0110001111: begin segment = 5'd11; b = 22'b0001001000111111010111; end
            10'b0110010000: begin segment = 5'd11; b = 22'b0001001000111111011111; end
            10'b0110010001: begin segment = 5'd11; b = 22'b0001001000111111100101; end
            10'b0110010010: begin segment = 5'd11; b = 22'b0001001000111111100111; end
            10'b0110010011: begin segment = 5'd11; b = 22'b0001001000111111100110; end
            10'b0110010100: begin segment = 5'd11; b = 22'b0001001000111111100010; end
            10'b0110010101: begin segment = 5'd11; b = 22'b0001001000111111011100; end
            10'b0110010110: begin segment = 5'd11; b = 22'b0001001000111111010010; end
            10'b0110010111: begin segment = 5'd11; b = 22'b0001001000111111000110; end
            10'b0110011000: begin segment = 5'd11; b = 22'b0001001000111110110111; end
            10'b0110011001: begin segment = 5'd11; b = 22'b0001001000111110100100; end
            10'b0110011010: begin segment = 5'd11; b = 22'b0001001000111110001111; end
            10'b0110011011: begin segment = 5'd11; b = 22'b0001001000111101110111; end
            10'b0110011100: begin segment = 5'd11; b = 22'b0001001000111101011011; end
            10'b0110011101: begin segment = 5'd11; b = 22'b0001001000111100111101; end
            10'b0110011110: begin segment = 5'd11; b = 22'b0001001000111100011100; end
            10'b0110011111: begin segment = 5'd11; b = 22'b0001001000111011111000; end
            10'b0110100000: begin segment = 5'd11; b = 22'b0001001000111011010001; end
            10'b0110100001: begin segment = 5'd11; b = 22'b0001001000111010101000; end
            10'b0110100010: begin segment = 5'd11; b = 22'b0001001000111001111011; end
            10'b0110100011: begin segment = 5'd11; b = 22'b0001001000111001001011; end
            10'b0110100100: begin segment = 5'd11; b = 22'b0001001000111000011001; end
            10'b0110100101: begin segment = 5'd11; b = 22'b0001001000110111100100; end
            10'b0110100110: begin segment = 5'd11; b = 22'b0001001000110110101011; end
            10'b0110100111: begin segment = 5'd11; b = 22'b0001001000110101110000; end
            10'b0110101000: begin segment = 5'd12; b = 22'b0001010101011010101100; end
            10'b0110101001: begin segment = 5'd12; b = 22'b0001010101011011100100; end
            10'b0110101010: begin segment = 5'd12; b = 22'b0001010101011100011010; end
            10'b0110101011: begin segment = 5'd12; b = 22'b0001010101011101001101; end
            10'b0110101100: begin segment = 5'd12; b = 22'b0001010101011101111101; end
            10'b0110101101: begin segment = 5'd12; b = 22'b0001010101011110101010; end
            10'b0110101110: begin segment = 5'd12; b = 22'b0001010101011111010100; end
            10'b0110101111: begin segment = 5'd12; b = 22'b0001010101011111111011; end
            10'b0110110000: begin segment = 5'd12; b = 22'b0001010101100000100000; end
            10'b0110110001: begin segment = 5'd12; b = 22'b0001010101100001000001; end
            10'b0110110010: begin segment = 5'd12; b = 22'b0001010101100001100001; end
            10'b0110110011: begin segment = 5'd12; b = 22'b0001010101100001111100; end
            10'b0110110100: begin segment = 5'd12; b = 22'b0001010101100010010110; end
            10'b0110110101: begin segment = 5'd12; b = 22'b0001010101100010101100; end
            10'b0110110110: begin segment = 5'd12; b = 22'b0001010101100010111111; end
            10'b0110110111: begin segment = 5'd12; b = 22'b0001010101100011010000; end
            10'b0110111000: begin segment = 5'd12; b = 22'b0001010101100011011110; end
            10'b0110111001: begin segment = 5'd12; b = 22'b0001010101100011101000; end
            10'b0110111010: begin segment = 5'd12; b = 22'b0001010101100011110001; end
            10'b0110111011: begin segment = 5'd12; b = 22'b0001010101100011110110; end
            10'b0110111100: begin segment = 5'd12; b = 22'b0001010101100011111001; end
            10'b0110111101: begin segment = 5'd12; b = 22'b0001010101100011111000; end
            10'b0110111110: begin segment = 5'd12; b = 22'b0001010101100011110101; end
            10'b0110111111: begin segment = 5'd12; b = 22'b0001010101100011101111; end
            10'b0111000000: begin segment = 5'd12; b = 22'b0001010101100011100111; end
            10'b0111000001: begin segment = 5'd12; b = 22'b0001010101100011011011; end
            10'b0111000010: begin segment = 5'd12; b = 22'b0001010101100011001101; end
            10'b0111000011: begin segment = 5'd12; b = 22'b0001010101100010111011; end
            10'b0111000100: begin segment = 5'd12; b = 22'b0001010101100010101000; end
            10'b0111000101: begin segment = 5'd12; b = 22'b0001010101100010010001; end
            10'b0111000110: begin segment = 5'd12; b = 22'b0001010101100001111000; end
            10'b0111000111: begin segment = 5'd12; b = 22'b0001010101100001011011; end
            10'b0111001000: begin segment = 5'd12; b = 22'b0001010101100000111101; end
            10'b0111001001: begin segment = 5'd12; b = 22'b0001010101100000011011; end
            10'b0111001010: begin segment = 5'd12; b = 22'b0001010101011111110111; end
            10'b0111001011: begin segment = 5'd12; b = 22'b0001010101011111001111; end
            10'b0111001100: begin segment = 5'd12; b = 22'b0001010101011110100110; end
            10'b0111001101: begin segment = 5'd12; b = 22'b0001010101011101111001; end
            10'b0111001110: begin segment = 5'd12; b = 22'b0001010101011101001001; end
            10'b0111001111: begin segment = 5'd12; b = 22'b0001010101011100010111; end
            10'b0111010000: begin segment = 5'd12; b = 22'b0001010101011011100010; end
            10'b0111010001: begin segment = 5'd12; b = 22'b0001010101011010101010; end
            10'b0111010010: begin segment = 5'd13; b = 22'b0001100010110010001110; end
            10'b0111010011: begin segment = 5'd13; b = 22'b0001100010110011000110; end
            10'b0111010100: begin segment = 5'd13; b = 22'b0001100010110011111100; end
            10'b0111010101: begin segment = 5'd13; b = 22'b0001100010110100101111; end
            10'b0111010110: begin segment = 5'd13; b = 22'b0001100010110101011111; end
            10'b0111010111: begin segment = 5'd13; b = 22'b0001100010110110001101; end
            10'b0111011000: begin segment = 5'd13; b = 22'b0001100010110110111000; end
            10'b0111011001: begin segment = 5'd13; b = 22'b0001100010110111100000; end
            10'b0111011010: begin segment = 5'd13; b = 22'b0001100010111000000101; end
            10'b0111011011: begin segment = 5'd13; b = 22'b0001100010111000101000; end
            10'b0111011100: begin segment = 5'd13; b = 22'b0001100010111001001000; end
            10'b0111011101: begin segment = 5'd13; b = 22'b0001100010111001100110; end
            10'b0111011110: begin segment = 5'd13; b = 22'b0001100010111010000000; end
            10'b0111011111: begin segment = 5'd13; b = 22'b0001100010111010011000; end
            10'b0111100000: begin segment = 5'd13; b = 22'b0001100010111010101110; end
            10'b0111100001: begin segment = 5'd13; b = 22'b0001100010111011000000; end
            10'b0111100010: begin segment = 5'd13; b = 22'b0001100010111011010000; end
            10'b0111100011: begin segment = 5'd13; b = 22'b0001100010111011011110; end
            10'b0111100100: begin segment = 5'd13; b = 22'b0001100010111011101000; end
            10'b0111100101: begin segment = 5'd13; b = 22'b0001100010111011110000; end
            10'b0111100110: begin segment = 5'd13; b = 22'b0001100010111011110110; end
            10'b0111100111: begin segment = 5'd13; b = 22'b0001100010111011111001; end
            10'b0111101000: begin segment = 5'd13; b = 22'b0001100010111011111001; end
            10'b0111101001: begin segment = 5'd13; b = 22'b0001100010111011110110; end
            10'b0111101010: begin segment = 5'd13; b = 22'b0001100010111011110000; end
            10'b0111101011: begin segment = 5'd13; b = 22'b0001100010111011101000; end
            10'b0111101100: begin segment = 5'd13; b = 22'b0001100010111011011110; end
            10'b0111101101: begin segment = 5'd13; b = 22'b0001100010111011010001; end
            10'b0111101110: begin segment = 5'd13; b = 22'b0001100010111011000001; end
            10'b0111101111: begin segment = 5'd13; b = 22'b0001100010111010101110; end
            10'b0111110000: begin segment = 5'd13; b = 22'b0001100010111010011001; end
            10'b0111110001: begin segment = 5'd13; b = 22'b0001100010111010000010; end
            10'b0111110010: begin segment = 5'd13; b = 22'b0001100010111001101000; end
            10'b0111110011: begin segment = 5'd13; b = 22'b0001100010111001001011; end
            10'b0111110100: begin segment = 5'd13; b = 22'b0001100010111000101011; end
            10'b0111110101: begin segment = 5'd13; b = 22'b0001100010111000001001; end
            10'b0111110110: begin segment = 5'd13; b = 22'b0001100010110111100100; end
            10'b0111110111: begin segment = 5'd13; b = 22'b0001100010110110111101; end
            10'b0111111000: begin segment = 5'd13; b = 22'b0001100010110110010011; end
            10'b0111111001: begin segment = 5'd13; b = 22'b0001100010110101100111; end
            10'b0111111010: begin segment = 5'd13; b = 22'b0001100010110100111000; end
            10'b0111111011: begin segment = 5'd13; b = 22'b0001100010110100000110; end
            10'b0111111100: begin segment = 5'd13; b = 22'b0001100010110011010010; end
            10'b0111111101: begin segment = 5'd13; b = 22'b0001100010110010011011; end
            10'b0111111110: begin segment = 5'd14; b = 22'b0001110000111110111000; end
            10'b0111111111: begin segment = 5'd14; b = 22'b0001110000111111101110; end
            10'b1000000000: begin segment = 5'd14; b = 22'b0001110001000000100001; end
            10'b1000000001: begin segment = 5'd14; b = 22'b0001110001000001010010; end
            10'b1000000010: begin segment = 5'd14; b = 22'b0001110001000010000001; end
            10'b1000000011: begin segment = 5'd14; b = 22'b0001110001000010101100; end
            10'b1000000100: begin segment = 5'd14; b = 22'b0001110001000011010110; end
            10'b1000000101: begin segment = 5'd14; b = 22'b0001110001000011111100; end
            10'b1000000110: begin segment = 5'd14; b = 22'b0001110001000100100001; end
            10'b1000000111: begin segment = 5'd14; b = 22'b0001110001000101000010; end
            10'b1000001000: begin segment = 5'd14; b = 22'b0001110001000101100001; end
            10'b1000001001: begin segment = 5'd14; b = 22'b0001110001000101111110; end
            10'b1000001010: begin segment = 5'd14; b = 22'b0001110001000110011000; end
            10'b1000001011: begin segment = 5'd14; b = 22'b0001110001000110110000; end
            10'b1000001100: begin segment = 5'd14; b = 22'b0001110001000111000100; end
            10'b1000001101: begin segment = 5'd14; b = 22'b0001110001000111010111; end
            10'b1000001110: begin segment = 5'd14; b = 22'b0001110001000111100111; end
            10'b1000001111: begin segment = 5'd14; b = 22'b0001110001000111110100; end
            10'b1000010000: begin segment = 5'd14; b = 22'b0001110001000111111111; end
            10'b1000010001: begin segment = 5'd14; b = 22'b0001110001001000001000; end
            10'b1000010010: begin segment = 5'd14; b = 22'b0001110001001000001110; end
            10'b1000010011: begin segment = 5'd14; b = 22'b0001110001001000010001; end
            10'b1000010100: begin segment = 5'd14; b = 22'b0001110001001000010010; end
            10'b1000010101: begin segment = 5'd14; b = 22'b0001110001001000010000; end
            10'b1000010110: begin segment = 5'd14; b = 22'b0001110001001000001100; end
            10'b1000010111: begin segment = 5'd14; b = 22'b0001110001001000000101; end
            10'b1000011000: begin segment = 5'd14; b = 22'b0001110001000111111100; end
            10'b1000011001: begin segment = 5'd14; b = 22'b0001110001000111110001; end
            10'b1000011010: begin segment = 5'd14; b = 22'b0001110001000111100011; end
            10'b1000011011: begin segment = 5'd14; b = 22'b0001110001000111010010; end
            10'b1000011100: begin segment = 5'd14; b = 22'b0001110001000110111111; end
            10'b1000011101: begin segment = 5'd14; b = 22'b0001110001000110101010; end
            10'b1000011110: begin segment = 5'd14; b = 22'b0001110001000110010010; end
            10'b1000011111: begin segment = 5'd14; b = 22'b0001110001000101110111; end
            10'b1000100000: begin segment = 5'd14; b = 22'b0001110001000101011010; end
            10'b1000100001: begin segment = 5'd14; b = 22'b0001110001000100111011; end
            10'b1000100010: begin segment = 5'd14; b = 22'b0001110001000100011001; end
            10'b1000100011: begin segment = 5'd14; b = 22'b0001110001000011110101; end
            10'b1000100100: begin segment = 5'd14; b = 22'b0001110001000011001110; end
            10'b1000100101: begin segment = 5'd14; b = 22'b0001110001000010100101; end
            10'b1000100110: begin segment = 5'd14; b = 22'b0001110001000001111010; end
            10'b1000100111: begin segment = 5'd14; b = 22'b0001110001000001001100; end
            10'b1000101000: begin segment = 5'd14; b = 22'b0001110001000000011011; end
            10'b1000101001: begin segment = 5'd14; b = 22'b0001110000111111101000; end
            10'b1000101010: begin segment = 5'd14; b = 22'b0001110000111110110011; end
            10'b1000101011: begin segment = 5'd15; b = 22'b0001111111111000101100; end
            10'b1000101100: begin segment = 5'd15; b = 22'b0001111111111001100000; end
            10'b1000101101: begin segment = 5'd15; b = 22'b0001111111111010010001; end
            10'b1000101110: begin segment = 5'd15; b = 22'b0001111111111011000000; end
            10'b1000101111: begin segment = 5'd15; b = 22'b0001111111111011101101; end
            10'b1000110000: begin segment = 5'd15; b = 22'b0001111111111100010111; end
            10'b1000110001: begin segment = 5'd15; b = 22'b0001111111111100111111; end
            10'b1000110010: begin segment = 5'd15; b = 22'b0001111111111101100100; end
            10'b1000110011: begin segment = 5'd15; b = 22'b0001111111111110000111; end
            10'b1000110100: begin segment = 5'd15; b = 22'b0001111111111110100111; end
            10'b1000110101: begin segment = 5'd15; b = 22'b0001111111111111000110; end
            10'b1000110110: begin segment = 5'd15; b = 22'b0001111111111111100001; end
            10'b1000110111: begin segment = 5'd15; b = 22'b0001111111111111111011; end
            10'b1000111000: begin segment = 5'd15; b = 22'b0010000000000000010010; end
            10'b1000111001: begin segment = 5'd15; b = 22'b0010000000000000100110; end
            10'b1000111010: begin segment = 5'd15; b = 22'b0010000000000000111000; end
            10'b1000111011: begin segment = 5'd15; b = 22'b0010000000000001001000; end
            10'b1000111100: begin segment = 5'd15; b = 22'b0010000000000001010110; end
            10'b1000111101: begin segment = 5'd15; b = 22'b0010000000000001100001; end
            10'b1000111110: begin segment = 5'd15; b = 22'b0010000000000001101001; end
            10'b1000111111: begin segment = 5'd15; b = 22'b0010000000000001110000; end
            10'b1001000000: begin segment = 5'd15; b = 22'b0010000000000001110100; end
            10'b1001000001: begin segment = 5'd15; b = 22'b0010000000000001110101; end
            10'b1001000010: begin segment = 5'd15; b = 22'b0010000000000001110100; end
            10'b1001000011: begin segment = 5'd15; b = 22'b0010000000000001110001; end
            10'b1001000100: begin segment = 5'd15; b = 22'b0010000000000001101100; end
            10'b1001000101: begin segment = 5'd15; b = 22'b0010000000000001100100; end
            10'b1001000110: begin segment = 5'd15; b = 22'b0010000000000001011010; end
            10'b1001000111: begin segment = 5'd15; b = 22'b0010000000000001001101; end
            10'b1001001000: begin segment = 5'd15; b = 22'b0010000000000000111110; end
            10'b1001001001: begin segment = 5'd15; b = 22'b0010000000000000101101; end
            10'b1001001010: begin segment = 5'd15; b = 22'b0010000000000000011001; end
            10'b1001001011: begin segment = 5'd15; b = 22'b0010000000000000000100; end
            10'b1001001100: begin segment = 5'd15; b = 22'b0001111111111111101011; end
            10'b1001001101: begin segment = 5'd15; b = 22'b0001111111111111010001; end
            10'b1001001110: begin segment = 5'd15; b = 22'b0001111111111110110100; end
            10'b1001001111: begin segment = 5'd15; b = 22'b0001111111111110010101; end
            10'b1001010000: begin segment = 5'd15; b = 22'b0001111111111101110011; end
            10'b1001010001: begin segment = 5'd15; b = 22'b0001111111111101001111; end
            10'b1001010010: begin segment = 5'd15; b = 22'b0001111111111100101001; end
            10'b1001010011: begin segment = 5'd15; b = 22'b0001111111111100000001; end
            10'b1001010100: begin segment = 5'd15; b = 22'b0001111111111011010110; end
            10'b1001010101: begin segment = 5'd15; b = 22'b0001111111111010101001; end
            10'b1001010110: begin segment = 5'd15; b = 22'b0001111111111001111010; end
            10'b1001010111: begin segment = 5'd15; b = 22'b0001111111111001001000; end
            10'b1001011000: begin segment = 5'd15; b = 22'b0001111111111000010100; end
            10'b1001011001: begin segment = 5'd16; b = 22'b0010001110010000100001; end
            10'b1001011010: begin segment = 5'd16; b = 22'b0010001110010001001010; end
            10'b1001011011: begin segment = 5'd16; b = 22'b0010001110010001110001; end
            10'b1001011100: begin segment = 5'd16; b = 22'b0010001110010010010110; end
            10'b1001011101: begin segment = 5'd16; b = 22'b0010001110010010111001; end
            10'b1001011110: begin segment = 5'd16; b = 22'b0010001110010011011001; end
            10'b1001011111: begin segment = 5'd16; b = 22'b0010001110010011110111; end
            10'b1001100000: begin segment = 5'd16; b = 22'b0010001110010100010011; end
            10'b1001100001: begin segment = 5'd16; b = 22'b0010001110010100101101; end
            10'b1001100010: begin segment = 5'd16; b = 22'b0010001110010101000100; end
            10'b1001100011: begin segment = 5'd16; b = 22'b0010001110010101011001; end
            10'b1001100100: begin segment = 5'd16; b = 22'b0010001110010101101100; end
            10'b1001100101: begin segment = 5'd16; b = 22'b0010001110010101111100; end
            10'b1001100110: begin segment = 5'd16; b = 22'b0010001110010110001010; end
            10'b1001100111: begin segment = 5'd16; b = 22'b0010001110010110010110; end
            10'b1001101000: begin segment = 5'd16; b = 22'b0010001110010110100000; end
            10'b1001101001: begin segment = 5'd16; b = 22'b0010001110010110100111; end
            10'b1001101010: begin segment = 5'd16; b = 22'b0010001110010110101101; end
            10'b1001101011: begin segment = 5'd16; b = 22'b0010001110010110110000; end
            10'b1001101100: begin segment = 5'd16; b = 22'b0010001110010110110000; end
            10'b1001101101: begin segment = 5'd16; b = 22'b0010001110010110101111; end
            10'b1001101110: begin segment = 5'd16; b = 22'b0010001110010110101011; end
            10'b1001101111: begin segment = 5'd16; b = 22'b0010001110010110100101; end
            10'b1001110000: begin segment = 5'd16; b = 22'b0010001110010110011101; end
            10'b1001110001: begin segment = 5'd16; b = 22'b0010001110010110010010; end
            10'b1001110010: begin segment = 5'd16; b = 22'b0010001110010110000110; end
            10'b1001110011: begin segment = 5'd16; b = 22'b0010001110010101110111; end
            10'b1001110100: begin segment = 5'd16; b = 22'b0010001110010101100110; end
            10'b1001110101: begin segment = 5'd16; b = 22'b0010001110010101010010; end
            10'b1001110110: begin segment = 5'd16; b = 22'b0010001110010100111101; end
            10'b1001110111: begin segment = 5'd16; b = 22'b0010001110010100100101; end
            10'b1001111000: begin segment = 5'd16; b = 22'b0010001110010100001011; end
            10'b1001111001: begin segment = 5'd16; b = 22'b0010001110010011101111; end
            10'b1001111010: begin segment = 5'd16; b = 22'b0010001110010011010001; end
            10'b1001111011: begin segment = 5'd16; b = 22'b0010001110010010110000; end
            10'b1001111100: begin segment = 5'd16; b = 22'b0010001110010010001101; end
            10'b1001111101: begin segment = 5'd16; b = 22'b0010001110010001101000; end
            10'b1001111110: begin segment = 5'd16; b = 22'b0010001110010001000001; end
            10'b1001111111: begin segment = 5'd16; b = 22'b0010001110010000011000; end
            10'b1010000000: begin segment = 5'd16; b = 22'b0010001110001111101100; end
            10'b1010000001: begin segment = 5'd16; b = 22'b0010001110001110111111; end
            10'b1010000010: begin segment = 5'd16; b = 22'b0010001110001110001111; end
            10'b1010000011: begin segment = 5'd16; b = 22'b0010001110001101011101; end
            10'b1010000100: begin segment = 5'd16; b = 22'b0010001110001100101000; end
            10'b1010000101: begin segment = 5'd16; b = 22'b0010001110001011110010; end
            10'b1010000110: begin segment = 5'd16; b = 22'b0010001110001010111001; end
            10'b1010000111: begin segment = 5'd16; b = 22'b0010001110001001111111; end
            10'b1010001000: begin segment = 5'd16; b = 22'b0010001110001001000010; end
            10'b1010001001: begin segment = 5'd17; b = 22'b0010100000100001010001; end
            10'b1010001010: begin segment = 5'd17; b = 22'b0010100000100010000100; end
            10'b1010001011: begin segment = 5'd17; b = 22'b0010100000100010110101; end
            10'b1010001100: begin segment = 5'd17; b = 22'b0010100000100011100011; end
            10'b1010001101: begin segment = 5'd17; b = 22'b0010100000100100010000; end
            10'b1010001110: begin segment = 5'd17; b = 22'b0010100000100100111010; end
            10'b1010001111: begin segment = 5'd17; b = 22'b0010100000100101100010; end
            10'b1010010000: begin segment = 5'd17; b = 22'b0010100000100110001000; end
            10'b1010010001: begin segment = 5'd17; b = 22'b0010100000100110101011; end
            10'b1010010010: begin segment = 5'd17; b = 22'b0010100000100111001101; end
            10'b1010010011: begin segment = 5'd17; b = 22'b0010100000100111101100; end
            10'b1010010100: begin segment = 5'd17; b = 22'b0010100000101000001001; end
            10'b1010010101: begin segment = 5'd17; b = 22'b0010100000101000100101; end
            10'b1010010110: begin segment = 5'd17; b = 22'b0010100000101000111110; end
            10'b1010010111: begin segment = 5'd17; b = 22'b0010100000101001010100; end
            10'b1010011000: begin segment = 5'd17; b = 22'b0010100000101001101001; end
            10'b1010011001: begin segment = 5'd17; b = 22'b0010100000101001111100; end
            10'b1010011010: begin segment = 5'd17; b = 22'b0010100000101010001100; end
            10'b1010011011: begin segment = 5'd17; b = 22'b0010100000101010011011; end
            10'b1010011100: begin segment = 5'd17; b = 22'b0010100000101010100111; end
            10'b1010011101: begin segment = 5'd17; b = 22'b0010100000101010110001; end
            10'b1010011110: begin segment = 5'd17; b = 22'b0010100000101010111001; end
            10'b1010011111: begin segment = 5'd17; b = 22'b0010100000101010111111; end
            10'b1010100000: begin segment = 5'd17; b = 22'b0010100000101011000011; end
            10'b1010100001: begin segment = 5'd17; b = 22'b0010100000101011000101; end
            10'b1010100010: begin segment = 5'd17; b = 22'b0010100000101011000100; end
            10'b1010100011: begin segment = 5'd17; b = 22'b0010100000101011000010; end
            10'b1010100100: begin segment = 5'd17; b = 22'b0010100000101010111101; end
            10'b1010100101: begin segment = 5'd17; b = 22'b0010100000101010110110; end
            10'b1010100110: begin segment = 5'd17; b = 22'b0010100000101010101110; end
            10'b1010100111: begin segment = 5'd17; b = 22'b0010100000101010100011; end
            10'b1010101000: begin segment = 5'd17; b = 22'b0010100000101010010110; end
            10'b1010101001: begin segment = 5'd17; b = 22'b0010100000101010000111; end
            10'b1010101010: begin segment = 5'd17; b = 22'b0010100000101001110110; end
            10'b1010101011: begin segment = 5'd17; b = 22'b0010100000101001100011; end
            10'b1010101100: begin segment = 5'd17; b = 22'b0010100000101001001110; end
            10'b1010101101: begin segment = 5'd17; b = 22'b0010100000101000110110; end
            10'b1010101110: begin segment = 5'd17; b = 22'b0010100000101000011101; end
            10'b1010101111: begin segment = 5'd17; b = 22'b0010100000101000000010; end
            10'b1010110000: begin segment = 5'd17; b = 22'b0010100000100111100100; end
            10'b1010110001: begin segment = 5'd17; b = 22'b0010100000100111000101; end
            10'b1010110010: begin segment = 5'd17; b = 22'b0010100000100110100011; end
            10'b1010110011: begin segment = 5'd17; b = 22'b0010100000100101111111; end
            10'b1010110100: begin segment = 5'd17; b = 22'b0010100000100101011010; end
            10'b1010110101: begin segment = 5'd17; b = 22'b0010100000100100110010; end
            10'b1010110110: begin segment = 5'd17; b = 22'b0010100000100100001000; end
            10'b1010110111: begin segment = 5'd17; b = 22'b0010100000100011011100; end
            10'b1010111000: begin segment = 5'd17; b = 22'b0010100000100010101110; end
            10'b1010111001: begin segment = 5'd17; b = 22'b0010100000100001111110; end
            10'b1010111010: begin segment = 5'd18; b = 22'b0010110001100100100110; end
            10'b1010111011: begin segment = 5'd18; b = 22'b0010110001100101010110; end
            10'b1010111100: begin segment = 5'd18; b = 22'b0010110001100110000100; end
            10'b1010111101: begin segment = 5'd18; b = 22'b0010110001100110110000; end
            10'b1010111110: begin segment = 5'd18; b = 22'b0010110001100111011010; end
            10'b1010111111: begin segment = 5'd18; b = 22'b0010110001101000000010; end
            10'b1011000000: begin segment = 5'd18; b = 22'b0010110001101000101000; end
            10'b1011000001: begin segment = 5'd18; b = 22'b0010110001101001001011; end
            10'b1011000010: begin segment = 5'd18; b = 22'b0010110001101001101101; end
            10'b1011000011: begin segment = 5'd18; b = 22'b0010110001101010001101; end
            10'b1011000100: begin segment = 5'd18; b = 22'b0010110001101010101011; end
            10'b1011000101: begin segment = 5'd18; b = 22'b0010110001101011000110; end
            10'b1011000110: begin segment = 5'd18; b = 22'b0010110001101011100000; end
            10'b1011000111: begin segment = 5'd18; b = 22'b0010110001101011111000; end
            10'b1011001000: begin segment = 5'd18; b = 22'b0010110001101100001101; end
            10'b1011001001: begin segment = 5'd18; b = 22'b0010110001101100100001; end
            10'b1011001010: begin segment = 5'd18; b = 22'b0010110001101100110011; end
            10'b1011001011: begin segment = 5'd18; b = 22'b0010110001101101000010; end
            10'b1011001100: begin segment = 5'd18; b = 22'b0010110001101101010000; end
            10'b1011001101: begin segment = 5'd18; b = 22'b0010110001101101011011; end
            10'b1011001110: begin segment = 5'd18; b = 22'b0010110001101101100101; end
            10'b1011001111: begin segment = 5'd18; b = 22'b0010110001101101101101; end
            10'b1011010000: begin segment = 5'd18; b = 22'b0010110001101101110010; end
            10'b1011010001: begin segment = 5'd18; b = 22'b0010110001101101110110; end
            10'b1011010010: begin segment = 5'd18; b = 22'b0010110001101101111000; end
            10'b1011010011: begin segment = 5'd18; b = 22'b0010110001101101111000; end
            10'b1011010100: begin segment = 5'd18; b = 22'b0010110001101101110101; end
            10'b1011010101: begin segment = 5'd18; b = 22'b0010110001101101110001; end
            10'b1011010110: begin segment = 5'd18; b = 22'b0010110001101101101011; end
            10'b1011010111: begin segment = 5'd18; b = 22'b0010110001101101100011; end
            10'b1011011000: begin segment = 5'd18; b = 22'b0010110001101101011000; end
            10'b1011011001: begin segment = 5'd18; b = 22'b0010110001101101001100; end
            10'b1011011010: begin segment = 5'd18; b = 22'b0010110001101100111110; end
            10'b1011011011: begin segment = 5'd18; b = 22'b0010110001101100101110; end
            10'b1011011100: begin segment = 5'd18; b = 22'b0010110001101100011100; end
            10'b1011011101: begin segment = 5'd18; b = 22'b0010110001101100001000; end
            10'b1011011110: begin segment = 5'd18; b = 22'b0010110001101011110010; end
            10'b1011011111: begin segment = 5'd18; b = 22'b0010110001101011011010; end
            10'b1011100000: begin segment = 5'd18; b = 22'b0010110001101011000000; end
            10'b1011100001: begin segment = 5'd18; b = 22'b0010110001101010100100; end
            10'b1011100010: begin segment = 5'd18; b = 22'b0010110001101010000111; end
            10'b1011100011: begin segment = 5'd18; b = 22'b0010110001101001100111; end
            10'b1011100100: begin segment = 5'd18; b = 22'b0010110001101001000101; end
            10'b1011100101: begin segment = 5'd18; b = 22'b0010110001101000100010; end
            10'b1011100110: begin segment = 5'd18; b = 22'b0010110001100111111100; end
            10'b1011100111: begin segment = 5'd18; b = 22'b0010110001100111010101; end
            10'b1011101000: begin segment = 5'd18; b = 22'b0010110001100110101011; end
            10'b1011101001: begin segment = 5'd18; b = 22'b0010110001100110000000; end
            10'b1011101010: begin segment = 5'd18; b = 22'b0010110001100101010011; end
            10'b1011101011: begin segment = 5'd18; b = 22'b0010110001100100100011; end
            10'b1011101100: begin segment = 5'd19; b = 22'b0011000100100100000110; end
            10'b1011101101: begin segment = 5'd19; b = 22'b0011000100100100111011; end
            10'b1011101110: begin segment = 5'd19; b = 22'b0011000100100101101110; end
            10'b1011101111: begin segment = 5'd19; b = 22'b0011000100100110011111; end
            10'b1011110000: begin segment = 5'd19; b = 22'b0011000100100111001110; end
            10'b1011110001: begin segment = 5'd19; b = 22'b0011000100100111111011; end
            10'b1011110010: begin segment = 5'd19; b = 22'b0011000100101000100111; end
            10'b1011110011: begin segment = 5'd19; b = 22'b0011000100101001010000; end
            10'b1011110100: begin segment = 5'd19; b = 22'b0011000100101001110111; end
            10'b1011110101: begin segment = 5'd19; b = 22'b0011000100101010011101; end
            10'b1011110110: begin segment = 5'd19; b = 22'b0011000100101011000001; end
            10'b1011110111: begin segment = 5'd19; b = 22'b0011000100101011100011; end
            10'b1011111000: begin segment = 5'd19; b = 22'b0011000100101100000010; end
            10'b1011111001: begin segment = 5'd19; b = 22'b0011000100101100100000; end
            10'b1011111010: begin segment = 5'd19; b = 22'b0011000100101100111100; end
            10'b1011111011: begin segment = 5'd19; b = 22'b0011000100101101010111; end
            10'b1011111100: begin segment = 5'd19; b = 22'b0011000100101101101111; end
            10'b1011111101: begin segment = 5'd19; b = 22'b0011000100101110000101; end
            10'b1011111110: begin segment = 5'd19; b = 22'b0011000100101110011010; end
            10'b1011111111: begin segment = 5'd19; b = 22'b0011000100101110101101; end
            10'b1100000000: begin segment = 5'd19; b = 22'b0011000100101110111101; end
            10'b1100000001: begin segment = 5'd19; b = 22'b0011000100101111001100; end
            10'b1100000010: begin segment = 5'd19; b = 22'b0011000100101111011001; end
            10'b1100000011: begin segment = 5'd19; b = 22'b0011000100101111100100; end
            10'b1100000100: begin segment = 5'd19; b = 22'b0011000100101111101110; end
            10'b1100000101: begin segment = 5'd19; b = 22'b0011000100101111110101; end
            10'b1100000110: begin segment = 5'd19; b = 22'b0011000100101111111010; end
            10'b1100000111: begin segment = 5'd19; b = 22'b0011000100101111111110; end
            10'b1100001000: begin segment = 5'd19; b = 22'b0011000100110000000000; end
            10'b1100001001: begin segment = 5'd19; b = 22'b0011000100110000000000; end
            10'b1100001010: begin segment = 5'd19; b = 22'b0011000100101111111110; end
            10'b1100001011: begin segment = 5'd19; b = 22'b0011000100101111111010; end
            10'b1100001100: begin segment = 5'd19; b = 22'b0011000100101111110100; end
            10'b1100001101: begin segment = 5'd19; b = 22'b0011000100101111101100; end
            10'b1100001110: begin segment = 5'd19; b = 22'b0011000100101111100011; end
            10'b1100001111: begin segment = 5'd19; b = 22'b0011000100101111010111; end
            10'b1100010000: begin segment = 5'd19; b = 22'b0011000100101111001010; end
            10'b1100010001: begin segment = 5'd19; b = 22'b0011000100101110111011; end
            10'b1100010010: begin segment = 5'd19; b = 22'b0011000100101110101010; end
            10'b1100010011: begin segment = 5'd19; b = 22'b0011000100101110011000; end
            10'b1100010100: begin segment = 5'd19; b = 22'b0011000100101110000011; end
            10'b1100010101: begin segment = 5'd19; b = 22'b0011000100101101101101; end
            10'b1100010110: begin segment = 5'd19; b = 22'b0011000100101101010100; end
            10'b1100010111: begin segment = 5'd19; b = 22'b0011000100101100111010; end
            10'b1100011000: begin segment = 5'd19; b = 22'b0011000100101100011110; end
            10'b1100011001: begin segment = 5'd19; b = 22'b0011000100101100000001; end
            10'b1100011010: begin segment = 5'd19; b = 22'b0011000100101011100001; end
            10'b1100011011: begin segment = 5'd19; b = 22'b0011000100101011000000; end
            10'b1100011100: begin segment = 5'd19; b = 22'b0011000100101010011100; end
            10'b1100011101: begin segment = 5'd19; b = 22'b0011000100101001110111; end
            10'b1100011110: begin segment = 5'd19; b = 22'b0011000100101001010001; end
            10'b1100011111: begin segment = 5'd19; b = 22'b0011000100101000101000; end
            10'b1100100000: begin segment = 5'd20; b = 22'b0011010111011000101110; end
            10'b1100100001: begin segment = 5'd20; b = 22'b0011010111011001100001; end
            10'b1100100010: begin segment = 5'd20; b = 22'b0011010111011010010011; end
            10'b1100100011: begin segment = 5'd20; b = 22'b0011010111011011000011; end
            10'b1100100100: begin segment = 5'd20; b = 22'b0011010111011011110010; end
            10'b1100100101: begin segment = 5'd20; b = 22'b0011010111011100011110; end
            10'b1100100110: begin segment = 5'd20; b = 22'b0011010111011101001001; end
            10'b1100100111: begin segment = 5'd20; b = 22'b0011010111011101110001; end
            10'b1100101000: begin segment = 5'd20; b = 22'b0011010111011110011000; end
            10'b1100101001: begin segment = 5'd20; b = 22'b0011010111011110111110; end
            10'b1100101010: begin segment = 5'd20; b = 22'b0011010111011111100001; end
            10'b1100101011: begin segment = 5'd20; b = 22'b0011010111100000000011; end
            10'b1100101100: begin segment = 5'd20; b = 22'b0011010111100000100010; end
            10'b1100101101: begin segment = 5'd20; b = 22'b0011010111100001000000; end
            10'b1100101110: begin segment = 5'd20; b = 22'b0011010111100001011101; end
            10'b1100101111: begin segment = 5'd20; b = 22'b0011010111100001110111; end
            10'b1100110000: begin segment = 5'd20; b = 22'b0011010111100010010000; end
            10'b1100110001: begin segment = 5'd20; b = 22'b0011010111100010100111; end
            10'b1100110010: begin segment = 5'd20; b = 22'b0011010111100010111100; end
            10'b1100110011: begin segment = 5'd20; b = 22'b0011010111100011001111; end
            10'b1100110100: begin segment = 5'd20; b = 22'b0011010111100011100000; end
            10'b1100110101: begin segment = 5'd20; b = 22'b0011010111100011110000; end
            10'b1100110110: begin segment = 5'd20; b = 22'b0011010111100011111110; end
            10'b1100110111: begin segment = 5'd20; b = 22'b0011010111100100001010; end
            10'b1100111000: begin segment = 5'd20; b = 22'b0011010111100100010101; end
            10'b1100111001: begin segment = 5'd20; b = 22'b0011010111100100011101; end
            10'b1100111010: begin segment = 5'd20; b = 22'b0011010111100100100100; end
            10'b1100111011: begin segment = 5'd20; b = 22'b0011010111100100101001; end
            10'b1100111100: begin segment = 5'd20; b = 22'b0011010111100100101101; end
            10'b1100111101: begin segment = 5'd20; b = 22'b0011010111100100101110; end
            10'b1100111110: begin segment = 5'd20; b = 22'b0011010111100100101110; end
            10'b1100111111: begin segment = 5'd20; b = 22'b0011010111100100101100; end
            10'b1101000000: begin segment = 5'd20; b = 22'b0011010111100100101000; end
            10'b1101000001: begin segment = 5'd20; b = 22'b0011010111100100100011; end
            10'b1101000010: begin segment = 5'd20; b = 22'b0011010111100100011100; end
            10'b1101000011: begin segment = 5'd20; b = 22'b0011010111100100010011; end
            10'b1101000100: begin segment = 5'd20; b = 22'b0011010111100100001000; end
            10'b1101000101: begin segment = 5'd20; b = 22'b0011010111100011111100; end
            10'b1101000110: begin segment = 5'd20; b = 22'b0011010111100011101101; end
            10'b1101000111: begin segment = 5'd20; b = 22'b0011010111100011011101; end
            10'b1101001000: begin segment = 5'd20; b = 22'b0011010111100011001100; end
            10'b1101001001: begin segment = 5'd20; b = 22'b0011010111100010111000; end
            10'b1101001010: begin segment = 5'd20; b = 22'b0011010111100010100011; end
            10'b1101001011: begin segment = 5'd20; b = 22'b0011010111100010001100; end
            10'b1101001100: begin segment = 5'd20; b = 22'b0011010111100001110011; end
            10'b1101001101: begin segment = 5'd20; b = 22'b0011010111100001011001; end
            10'b1101001110: begin segment = 5'd20; b = 22'b0011010111100000111101; end
            10'b1101001111: begin segment = 5'd20; b = 22'b0011010111100000011111; end
            10'b1101010000: begin segment = 5'd20; b = 22'b0011010111100000000000; end
            10'b1101010001: begin segment = 5'd20; b = 22'b0011010111011111011110; end
            10'b1101010010: begin segment = 5'd20; b = 22'b0011010111011110111011; end
            10'b1101010011: begin segment = 5'd20; b = 22'b0011010111011110010110; end
            10'b1101010100: begin segment = 5'd20; b = 22'b0011010111011101110000; end
            10'b1101010101: begin segment = 5'd20; b = 22'b0011010111011101001000; end
            10'b1101010110: begin segment = 5'd21; b = 22'b0011101011011110001110; end
            10'b1101010111: begin segment = 5'd21; b = 22'b0011101011011111000010; end
            10'b1101011000: begin segment = 5'd21; b = 22'b0011101011011111110101; end
            10'b1101011001: begin segment = 5'd21; b = 22'b0011101011100000100110; end
            10'b1101011010: begin segment = 5'd21; b = 22'b0011101011100001010101; end
            10'b1101011011: begin segment = 5'd21; b = 22'b0011101011100010000011; end
            10'b1101011100: begin segment = 5'd21; b = 22'b0011101011100010101111; end
            10'b1101011101: begin segment = 5'd21; b = 22'b0011101011100011011001; end
            10'b1101011110: begin segment = 5'd21; b = 22'b0011101011100100000001; end
            10'b1101011111: begin segment = 5'd21; b = 22'b0011101011100100101000; end
            10'b1101100000: begin segment = 5'd21; b = 22'b0011101011100101001101; end
            10'b1101100001: begin segment = 5'd21; b = 22'b0011101011100101110000; end
            10'b1101100010: begin segment = 5'd21; b = 22'b0011101011100110010010; end
            10'b1101100011: begin segment = 5'd21; b = 22'b0011101011100110110010; end
            10'b1101100100: begin segment = 5'd21; b = 22'b0011101011100111010000; end
            10'b1101100101: begin segment = 5'd21; b = 22'b0011101011100111101101; end
            10'b1101100110: begin segment = 5'd21; b = 22'b0011101011101000001000; end
            10'b1101100111: begin segment = 5'd21; b = 22'b0011101011101000100001; end
            10'b1101101000: begin segment = 5'd21; b = 22'b0011101011101000111000; end
            10'b1101101001: begin segment = 5'd21; b = 22'b0011101011101001001110; end
            10'b1101101010: begin segment = 5'd21; b = 22'b0011101011101001100010; end
            10'b1101101011: begin segment = 5'd21; b = 22'b0011101011101001110101; end
            10'b1101101100: begin segment = 5'd21; b = 22'b0011101011101010000101; end
            10'b1101101101: begin segment = 5'd21; b = 22'b0011101011101010010101; end
            10'b1101101110: begin segment = 5'd21; b = 22'b0011101011101010100010; end
            10'b1101101111: begin segment = 5'd21; b = 22'b0011101011101010101110; end
            10'b1101110000: begin segment = 5'd21; b = 22'b0011101011101010111000; end
            10'b1101110001: begin segment = 5'd21; b = 22'b0011101011101011000000; end
            10'b1101110010: begin segment = 5'd21; b = 22'b0011101011101011000111; end
            10'b1101110011: begin segment = 5'd21; b = 22'b0011101011101011001100; end
            10'b1101110100: begin segment = 5'd21; b = 22'b0011101011101011010000; end
            10'b1101110101: begin segment = 5'd21; b = 22'b0011101011101011010001; end
            10'b1101110110: begin segment = 5'd21; b = 22'b0011101011101011010010; end
            10'b1101110111: begin segment = 5'd21; b = 22'b0011101011101011010000; end
            10'b1101111000: begin segment = 5'd21; b = 22'b0011101011101011001101; end
            10'b1101111001: begin segment = 5'd21; b = 22'b0011101011101011001000; end
            10'b1101111010: begin segment = 5'd21; b = 22'b0011101011101011000001; end
            10'b1101111011: begin segment = 5'd21; b = 22'b0011101011101010111001; end
            10'b1101111100: begin segment = 5'd21; b = 22'b0011101011101010101111; end
            10'b1101111101: begin segment = 5'd21; b = 22'b0011101011101010100100; end
            10'b1101111110: begin segment = 5'd21; b = 22'b0011101011101010010111; end
            10'b1101111111: begin segment = 5'd21; b = 22'b0011101011101010001000; end
            10'b1110000000: begin segment = 5'd21; b = 22'b0011101011101001111000; end
            10'b1110000001: begin segment = 5'd21; b = 22'b0011101011101001100110; end
            10'b1110000010: begin segment = 5'd21; b = 22'b0011101011101001010010; end
            10'b1110000011: begin segment = 5'd21; b = 22'b0011101011101000111101; end
            10'b1110000100: begin segment = 5'd21; b = 22'b0011101011101000100110; end
            10'b1110000101: begin segment = 5'd21; b = 22'b0011101011101000001101; end
            10'b1110000110: begin segment = 5'd21; b = 22'b0011101011100111110011; end
            10'b1110000111: begin segment = 5'd21; b = 22'b0011101011100111010111; end
            10'b1110001000: begin segment = 5'd21; b = 22'b0011101011100110111010; end
            10'b1110001001: begin segment = 5'd21; b = 22'b0011101011100110011011; end
            10'b1110001010: begin segment = 5'd21; b = 22'b0011101011100101111010; end
            10'b1110001011: begin segment = 5'd21; b = 22'b0011101011100101011000; end
            10'b1110001100: begin segment = 5'd21; b = 22'b0011101011100100110100; end
            10'b1110001101: begin segment = 5'd22; b = 22'b0011111101110001100001; end
            10'b1110001110: begin segment = 5'd22; b = 22'b0011111101110010001100; end
            10'b1110001111: begin segment = 5'd22; b = 22'b0011111101110010110101; end
            10'b1110010000: begin segment = 5'd22; b = 22'b0011111101110011011101; end
            10'b1110010001: begin segment = 5'd22; b = 22'b0011111101110100000011; end
            10'b1110010010: begin segment = 5'd22; b = 22'b0011111101110100100111; end
            10'b1110010011: begin segment = 5'd22; b = 22'b0011111101110101001010; end
            10'b1110010100: begin segment = 5'd22; b = 22'b0011111101110101101011; end
            10'b1110010101: begin segment = 5'd22; b = 22'b0011111101110110001010; end
            10'b1110010110: begin segment = 5'd22; b = 22'b0011111101110110101000; end
            10'b1110010111: begin segment = 5'd22; b = 22'b0011111101110111000101; end
            10'b1110011000: begin segment = 5'd22; b = 22'b0011111101110111011111; end
            10'b1110011001: begin segment = 5'd22; b = 22'b0011111101110111111000; end
            10'b1110011010: begin segment = 5'd22; b = 22'b0011111101111000010000; end
            10'b1110011011: begin segment = 5'd22; b = 22'b0011111101111000100110; end
            10'b1110011100: begin segment = 5'd22; b = 22'b0011111101111000111010; end
            10'b1110011101: begin segment = 5'd22; b = 22'b0011111101111001001101; end
            10'b1110011110: begin segment = 5'd22; b = 22'b0011111101111001011110; end
            10'b1110011111: begin segment = 5'd22; b = 22'b0011111101111001101101; end
            10'b1110100000: begin segment = 5'd22; b = 22'b0011111101111001111011; end
            10'b1110100001: begin segment = 5'd22; b = 22'b0011111101111010001000; end
            10'b1110100010: begin segment = 5'd22; b = 22'b0011111101111010010011; end
            10'b1110100011: begin segment = 5'd22; b = 22'b0011111101111010011100; end
            10'b1110100100: begin segment = 5'd22; b = 22'b0011111101111010100011; end
            10'b1110100101: begin segment = 5'd22; b = 22'b0011111101111010101001; end
            10'b1110100110: begin segment = 5'd22; b = 22'b0011111101111010101110; end
            10'b1110100111: begin segment = 5'd22; b = 22'b0011111101111010110001; end
            10'b1110101000: begin segment = 5'd22; b = 22'b0011111101111010110010; end
            10'b1110101001: begin segment = 5'd22; b = 22'b0011111101111010110010; end
            10'b1110101010: begin segment = 5'd22; b = 22'b0011111101111010110000; end
            10'b1110101011: begin segment = 5'd22; b = 22'b0011111101111010101100; end
            10'b1110101100: begin segment = 5'd22; b = 22'b0011111101111010101000; end
            10'b1110101101: begin segment = 5'd22; b = 22'b0011111101111010100001; end
            10'b1110101110: begin segment = 5'd22; b = 22'b0011111101111010011001; end
            10'b1110101111: begin segment = 5'd22; b = 22'b0011111101111010001111; end
            10'b1110110000: begin segment = 5'd22; b = 22'b0011111101111010000100; end
            10'b1110110001: begin segment = 5'd22; b = 22'b0011111101111001110111; end
            10'b1110110010: begin segment = 5'd22; b = 22'b0011111101111001101001; end
            10'b1110110011: begin segment = 5'd22; b = 22'b0011111101111001011001; end
            10'b1110110100: begin segment = 5'd22; b = 22'b0011111101111001000111; end
            10'b1110110101: begin segment = 5'd22; b = 22'b0011111101111000110100; end
            10'b1110110110: begin segment = 5'd22; b = 22'b0011111101111000100000; end
            10'b1110110111: begin segment = 5'd22; b = 22'b0011111101111000001001; end
            10'b1110111000: begin segment = 5'd22; b = 22'b0011111101110111110010; end
            10'b1110111001: begin segment = 5'd22; b = 22'b0011111101110111011000; end
            10'b1110111010: begin segment = 5'd22; b = 22'b0011111101110110111110; end
            10'b1110111011: begin segment = 5'd22; b = 22'b0011111101110110100001; end
            10'b1110111100: begin segment = 5'd22; b = 22'b0011111101110110000011; end
            10'b1110111101: begin segment = 5'd22; b = 22'b0011111101110101100100; end
            10'b1110111110: begin segment = 5'd22; b = 22'b0011111101110101000011; end
            10'b1110111111: begin segment = 5'd22; b = 22'b0011111101110100100001; end
            10'b1111000000: begin segment = 5'd22; b = 22'b0011111101110011111100; end
            10'b1111000001: begin segment = 5'd22; b = 22'b0011111101110011010111; end
            10'b1111000010: begin segment = 5'd22; b = 22'b0011111101110010110000; end
            10'b1111000011: begin segment = 5'd22; b = 22'b0011111101110010000111; end
            10'b1111000100: begin segment = 5'd22; b = 22'b0011111101110001011101; end
            10'b1111000101: begin segment = 5'd23; b = 22'b0100010010000010001011; end
            10'b1111000110: begin segment = 5'd23; b = 22'b0100010010000010110100; end
            10'b1111000111: begin segment = 5'd23; b = 22'b0100010010000011011011; end
            10'b1111001000: begin segment = 5'd23; b = 22'b0100010010000100000001; end
            10'b1111001001: begin segment = 5'd23; b = 22'b0100010010000100100101; end
            10'b1111001010: begin segment = 5'd23; b = 22'b0100010010000101000111; end
            10'b1111001011: begin segment = 5'd23; b = 22'b0100010010000101101000; end
            10'b1111001100: begin segment = 5'd23; b = 22'b0100010010000110001000; end
            10'b1111001101: begin segment = 5'd23; b = 22'b0100010010000110100110; end
            10'b1111001110: begin segment = 5'd23; b = 22'b0100010010000111000011; end
            10'b1111001111: begin segment = 5'd23; b = 22'b0100010010000111011110; end
            10'b1111010000: begin segment = 5'd23; b = 22'b0100010010000111110111; end
            10'b1111010001: begin segment = 5'd23; b = 22'b0100010010001000001111; end
            10'b1111010010: begin segment = 5'd23; b = 22'b0100010010001000100110; end
            10'b1111010011: begin segment = 5'd23; b = 22'b0100010010001000111011; end
            10'b1111010100: begin segment = 5'd23; b = 22'b0100010010001001001110; end
            10'b1111010101: begin segment = 5'd23; b = 22'b0100010010001001100000; end
            10'b1111010110: begin segment = 5'd23; b = 22'b0100010010001001110001; end
            10'b1111010111: begin segment = 5'd23; b = 22'b0100010010001010000000; end
            10'b1111011000: begin segment = 5'd23; b = 22'b0100010010001010001101; end
            10'b1111011001: begin segment = 5'd23; b = 22'b0100010010001010011001; end
            10'b1111011010: begin segment = 5'd23; b = 22'b0100010010001010100100; end
            10'b1111011011: begin segment = 5'd23; b = 22'b0100010010001010101101; end
            10'b1111011100: begin segment = 5'd23; b = 22'b0100010010001010110100; end
            10'b1111011101: begin segment = 5'd23; b = 22'b0100010010001010111010; end
            10'b1111011110: begin segment = 5'd23; b = 22'b0100010010001010111111; end
            10'b1111011111: begin segment = 5'd23; b = 22'b0100010010001011000010; end
            10'b1111100000: begin segment = 5'd23; b = 22'b0100010010001011000011; end
            10'b1111100001: begin segment = 5'd23; b = 22'b0100010010001011000011; end
            10'b1111100010: begin segment = 5'd23; b = 22'b0100010010001011000010; end
            10'b1111100011: begin segment = 5'd23; b = 22'b0100010010001010111111; end
            10'b1111100100: begin segment = 5'd23; b = 22'b0100010010001010111010; end
            10'b1111100101: begin segment = 5'd23; b = 22'b0100010010001010110100; end
            10'b1111100110: begin segment = 5'd23; b = 22'b0100010010001010101101; end
            10'b1111100111: begin segment = 5'd23; b = 22'b0100010010001010100100; end
            10'b1111101000: begin segment = 5'd23; b = 22'b0100010010001010011010; end
            10'b1111101001: begin segment = 5'd23; b = 22'b0100010010001010001110; end
            10'b1111101010: begin segment = 5'd23; b = 22'b0100010010001010000001; end
            10'b1111101011: begin segment = 5'd23; b = 22'b0100010010001001110010; end
            10'b1111101100: begin segment = 5'd23; b = 22'b0100010010001001100010; end
            10'b1111101101: begin segment = 5'd23; b = 22'b0100010010001001010000; end
            10'b1111101110: begin segment = 5'd23; b = 22'b0100010010001000111101; end
            10'b1111101111: begin segment = 5'd23; b = 22'b0100010010001000101000; end
            10'b1111110000: begin segment = 5'd23; b = 22'b0100010010001000010010; end
            10'b1111110001: begin segment = 5'd23; b = 22'b0100010010000111111011; end
            10'b1111110010: begin segment = 5'd23; b = 22'b0100010010000111100010; end
            10'b1111110011: begin segment = 5'd23; b = 22'b0100010010000111000111; end
            10'b1111110100: begin segment = 5'd23; b = 22'b0100010010000110101011; end
            10'b1111110101: begin segment = 5'd23; b = 22'b0100010010000110001110; end
            10'b1111110110: begin segment = 5'd23; b = 22'b0100010010000101101111; end
            10'b1111110111: begin segment = 5'd23; b = 22'b0100010010000101001111; end
            10'b1111111000: begin segment = 5'd23; b = 22'b0100010010000100101101; end
            10'b1111111001: begin segment = 5'd23; b = 22'b0100010010000100001010; end
            10'b1111111010: begin segment = 5'd23; b = 22'b0100010010000011100101; end
            10'b1111111011: begin segment = 5'd23; b = 22'b0100010010000010111111; end
            10'b1111111100: begin segment = 5'd23; b = 22'b0100010010000010010111; end
            10'b1111111101: begin segment = 5'd23; b = 22'b0100010010000001101110; end
            10'b1111111110: begin segment = 5'd23; b = 22'b0100010010000001000100; end
            10'b1111111111: begin segment = 5'd23; b = 22'b0100010010000000011000; end
      endcase
    end

// Auto-generated neg assignments for WIDTH=22
    wire neg_1;
    assign neg_1 = ~f[21];
    wire [1:0] neg_2;
    assign neg_2 = ~f[21:20];
    wire [2:0] neg_3;
    assign neg_3 = ~f[21:19];
    wire [3:0] neg_4;
    assign neg_4 = ~f[21:18];
    wire [4:0] neg_5;
    assign neg_5 = ~f[21:17];
    wire [5:0] neg_6;
    assign neg_6 = ~f[21:16];
    wire [6:0] neg_7;
    assign neg_7 = ~f[21:15];
    wire [7:0] neg_8;
    assign neg_8 = ~f[21:14];
    wire [8:0] neg_9;
    assign neg_9 = ~f[21:13];
    wire [9:0] neg_10;
    assign neg_10 = ~f[21:12];
    wire [10:0] neg_11;
    assign neg_11 = ~f[21:11];
    wire [11:0] neg_12;
    assign neg_12 = ~f[21:10];
    wire [12:0] neg_13;
    assign neg_13 = ~f[21:9];
    wire [13:0] neg_14;
    assign neg_14 = ~f[21:8];
    wire [14:0] neg_15;
    assign neg_15 = ~f[21:7];
    wire [15:0] neg_16;
    assign neg_16 = ~f[21:6];
    wire [16:0] neg_17;
    assign neg_17 = ~f[21:5];
    wire [17:0] neg_18;
    assign neg_18 = ~f[21:4];
    wire [18:0] neg_19;
    assign neg_19 = ~f[21:3];
    wire [19:0] neg_20;
    assign neg_20 = ~f[21:2];
    wire [20:0] neg_21;
    assign neg_21 = ~f[21:1];
// End of auto-generated section

    always @(*) begin
        case(segment)
            24'd0: begin A1 = {1'b0, f[21:1]}; A2 = {4'b1111, neg_18}; A3 = {6'b111111, neg_16}; end
            24'd1: begin A1 = {1'b0, f[21:1]}; A2 = {3'b111, neg_19}; A3 = {7'b0000000, f[21:7]}; end
            24'd2: begin A1 = {1'b0, f[21:1]}; A2 = {3'b111, neg_19}; A3 = {5'b11111, neg_17}; end
            24'd3: begin A1 = {2'b00, f[21:2]}; A2 = {4'b0000, f[21:4]}; A3 = {7'b1111111, neg_15}; end
            24'd4: begin A1 = {2'b00, f[21:2]}; A2 = {6'b000000, f[21:6]}; A3 = {9'b000000000, f[21:9]}; end
            24'd5: begin A1 = {2'b00, f[21:2]}; A2 = {6'b111111, neg_16}; A3 = {8'b11111111, neg_14}; end
            24'd6: begin A1 = {2'b00, f[21:2]}; A2 = {4'b1111, neg_18}; A3 = {7'b0000000, f[21:7]}; end
            24'd7: begin A1 = {3'b000, f[21:3]}; A2 = {5'b00000, f[21:5]}; A3 = {7'b0000000, f[21:7]}; end
            24'd8: begin A1 = {3'b000, f[21:3]}; A2 = {8'b00000000, f[21:8]}; A3 = {12'b000000000000, f[21:12]}; end
            24'd9: begin A1 = {3'b000, f[21:3]}; A2 = {5'b11111, neg_17}; A3 = {8'b00000000, f[21:8]}; end
            24'd10: begin A1 = {4'b0000, f[21:4]}; A2 = {8'b00000000, f[21:8]}; A3 = {11'b11111111111, neg_11}; end
            24'd11: begin A1 = {5'b00000, f[21:5]}; A2 = {8'b00000000, f[21:8]}; A3 = {12'b000000000000, f[21:12]}; end
            24'd12: begin A1 = {7'b0000000, f[21:7]}; A2 = {9'b111111111, neg_13}; A3 = {13'b1111111111111, neg_9}; end
            24'd13: begin A1 = {5'b11111, neg_17}; A2 = {7'b0000000, f[21:7]}; A3 = {11'b00000000000, f[21:11]}; end
            24'd14: begin A1 = {4'b1111, neg_18}; A2 = {6'b000000, f[21:6]}; A3 = {8'b11111111, neg_14}; end
            24'd15: begin A1 = {4'b1111, neg_18}; A2 = {6'b111111, neg_16}; A3 = {11'b00000000000, f[21:11]}; end
            24'd16: begin A1 = {3'b111, neg_19}; A2 = {5'b00000, f[21:5]}; A3 = {7'b1111111, neg_15}; end
            24'd17: begin A1 = {3'b111, neg_19}; A2 = {8'b11111111, neg_14}; A3 = {10'b1111111111, neg_12}; end
            24'd18: begin A1 = {3'b111, neg_19}; A2 = {5'b11111, neg_17}; A3 = {9'b000000000, f[21:9]}; end
            24'd19: begin A1 = {2'b11, neg_20}; A2 = {4'b0000, f[21:4]}; A3 = {7'b0000000, f[21:7]}; end
            24'd20: begin A1 = {2'b11, neg_20}; A2 = {4'b0000, f[21:4]}; A3 = {6'b111111, neg_16}; end
            24'd21: begin A1 = {2'b11, neg_20}; A2 = {5'b00000, f[21:5]}; A3 = {7'b1111111, neg_15}; end
            24'd22: begin A1 = {2'b11, neg_20}; A2 = {8'b00000000, f[21:8]}; A3 = {11'b11111111111, neg_11}; end
            24'd23: begin A1 = {2'b11, neg_20}; A2 = {6'b111111, neg_16}; A3 = {9'b111111111, neg_13}; end
      endcase
    end
endmodule


module Booth_pp(
    input [6:0] input_7,
    input TRG,
    input [30:0] A,
    input [30:0] rev_A,
    output reg [33:0] pp0,
    output reg [32:0] pp1,
    output reg [33:0] pp2
);

    wire [2:0] onehot [3:0];
    assign  onehot[0] = (TRG == 1'b0) ? input_7[2:0] : {input_7[2:1], 1'b0};
    assign  onehot[1] = input_7[4:2];
    assign  onehot[2] = input_7[6:4];
    
    always @(*) begin
        case (onehot[0])
            3'b000, 3'b111: begin
                pp0 = {3'b100, 31'b0};
            end
            3'b001, 3'b010: begin
                if(A[30] == 1'b0)
                    pp0 = {3'b100, A};
                else
                    pp0 = {3'b011, A};
            end
            3'b011: begin
                if(A[30] == 1'b0)
                    pp0 = {3'b100, A[29:0], 1'b0};
                else
                    pp0 = {3'b011, A[29:0], 1'b0};
            end
            3'b100: begin
                if(A[30] == 1'b0)
                    pp0 = {3'b011, rev_A[30], rev_A[28:0], 1'b0}; 
                else
                    pp0 = {3'b100, rev_A[30], rev_A[28:0], 1'b0}; 
            end
            3'b101, 3'b110: begin
                if(A[30] == 1'b0)
                    pp0 = {3'b011, rev_A};
                else
                    pp0 = {3'b100, rev_A};
            end
        endcase
    end
    wire S;
    wire S_;
    assign S = 1'b1^A[30];
    assign S_ = 1'b0^A[30];

    always @(*) begin
        case (onehot[1])
            3'b000, 3'b111: begin
                pp1 = {2'b11, 31'b0};
            end
            3'b001, 3'b010: begin
                pp1 = {1'b1, S, A};
            end
            3'b011: begin
                pp1 = {1'b1, S, A[29:0], 1'b0};
            end
            3'b100: begin
                pp1 = {1'b1, S_, rev_A[30], rev_A[28:0], 1'b0}; 
            end
            3'b101, 3'b110: begin
                pp1 = {1'b1,  S_, rev_A};
            end
        endcase
    end
    always @(*) begin
        case (onehot[2])
            3'b000, 3'b111: begin
                pp2 = {2'b11, 1'b1, 31'b0};
            end
            3'b001, 3'b010: begin
                pp2 = {2'b11, S, A};
            end
            3'b011: begin
                pp2 = {2'b11, S, A[29:0], 1'b0};
            end
            3'b100: begin
                pp2 = {2'b11, S_, rev_A[30], rev_A[28:0], 1'b0}; 
            end
            3'b101, 3'b110: begin
                pp2 = {2'b11, S_, rev_A};
            end
        endcase
    end



endmodule
