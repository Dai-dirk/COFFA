`timescale 1ns / 1ps
module CPA_tree_with_MAD#(parameter WIDTH32 = 32  
)(
    input clk,
    input rstn,
    input [WIDTH32-1:0] t0,
    input [WIDTH32-1:0] t1,
    input [WIDTH32-1:0] t2,
    input [WIDTH32-1:0] t3,
    input [WIDTH32-1:0] t4,
    input float_flag,
    input [WIDTH32-1:0] bias,

    output float_overflow,
    output float_underflow,
    output cpa_overflow,
    output [WIDTH32-1:0] tri_result
);
    //@zou 默认执行求和方式
    // wire [WIDTH32-1:0] channel0_CPA;
    // wire [WIDTH32-1:0] channel1_CPA;
    // wire [WIDTH32-1:0] channel2_CPA;
    // wire [WIDTH32-1:0] channel3_CPA;


    wire [WIDTH32-1:0] CPA0_a;
    wire [WIDTH32-1:0] CPA0_b;
    assign CPA0_a = t0;
    assign CPA0_b = t1;
    wire CPA0_overflow;
    wire CPA0_cpa_overflow;
    wire CPA0_underflow;
    wire [WIDTH32-1:0] CPA0_result;

    CPA_float_add cpa_add0(
        .clk(clk),
        .rstn(rstn),
        .float_flag(float_flag),
        .input_x0(CPA0_a),
        .input_x1(CPA0_b),
        .TRi(1'b0),
        .final_result(CPA0_result),
        .cpa_overflow(CPA0_cpa_overflow),
        .overflow(CPA0_overflow),
        .underflow(CPA0_underflow)
    );


    wire [WIDTH32-1:0] CPA1_a;
    wire [WIDTH32-1:0] CPA1_b;
    assign CPA1_a = t2;
    assign CPA1_b = t3;
    wire CPA1_overflow;
    wire CPA1_cpa_overflow;
    wire CPA1_underflow;
    wire [WIDTH32-1:0] CPA1_result;

    CPA_float_add cpa_add1(
        .clk(clk),
        .rstn(rstn),
        .float_flag(float_flag),
        .input_x0(CPA1_a),
        .input_x1(CPA1_b),
        .TRi(1'b0),
        .final_result(CPA1_result),
        .cpa_overflow(CPA1_cpa_overflow),
        .overflow(CPA1_overflow),
        .underflow(CPA1_underflow)
    );


    wire [WIDTH32-1:0] CPA2_a;
    wire [WIDTH32-1:0] CPA2_b;
    assign CPA2_a = t4;
    assign CPA2_b = bias;
    wire CPA2_overflow;
    wire CPA2_cpa_overflow;
    wire CPA2_underflow;
    wire [WIDTH32-1:0] CPA2_result;

    CPA_float_add cpa_add2(
        .clk(clk),
        .rstn(rstn),
        .float_flag(float_flag),
        .input_x0(CPA2_a),
        .input_x1(CPA2_b),
        .TRi(1'b0),
        .final_result(CPA2_result),
        .cpa_overflow(CPA2_cpa_overflow),
        .overflow(CPA2_overflow),
        .underflow(CPA2_underflow)
    );




    wire [WIDTH32-1:0] CPA3_a;
    wire [WIDTH32-1:0] CPA3_b;
    assign CPA3_a = CPA0_result;
    assign CPA3_b = CPA1_result;
    wire CPA3_overflow;
    wire CPA3_cpa_overflow;
    wire CPA3_underflow;
    wire [WIDTH32-1:0] CPA3_result;
    CPA_float_add_all_reg cpa_add3(
        .clk(clk),
        .rstn(rstn),
        .float_flag(float_flag),
        .input_x0(CPA3_a),
        .input_x1(CPA3_b),
        .TRi(1'b0),
        .final_result(CPA3_result),
        .cpa_overflow(CPA3_cpa_overflow),
        .overflow(CPA3_overflow),
        .underflow(CPA3_underflow)
    );

    wire [WIDTH32-1:0] CPA4_a;
    wire [WIDTH32-1:0] CPA4_b;
    //@yuan: we should delay the result of CPA2 to make data being synchronized
    reg [WIDTH32-1:0] CPA2_result_reg;
    always@(posedge clk or negedge rstn)begin
        if(!rstn)begin
            CPA2_result_reg <= 0;
        end else begin
            CPA2_result_reg <= CPA2_result;
        end
    end
    assign CPA4_a = CPA3_result;
    assign CPA4_b = CPA2_result_reg;
    wire CPA4_overflow;
    wire CPA4_cpa_overflow;
    wire CPA4_underflow;
    wire [WIDTH32-1:0] CPA4_result;
    CPA_float_add_all_reg cpa_add4(
        .clk(clk),
        .rstn(rstn),
        .float_flag(float_flag),
        .input_x0(CPA4_a),
        .input_x1(CPA4_b),
        .TRi(1'b0),
        .final_result(CPA4_result),
        .cpa_overflow(CPA4_cpa_overflow),
        .overflow(CPA4_overflow),
        .underflow(CPA4_underflow)
    );
    
    assign float_overflow = (float_flag==1'b1)&&(CPA0_overflow|CPA1_overflow|CPA2_overflow|CPA3_overflow|CPA4_overflow);
    assign float_underflow = (float_flag==1'b1)&&(CPA0_underflow|CPA1_underflow|CPA2_underflow|CPA3_underflow|CPA4_underflow);
    assign cpa_overflow = CPA0_cpa_overflow | CPA1_cpa_overflow | CPA2_cpa_overflow | CPA3_cpa_overflow | CPA4_cpa_overflow;
    assign tri_result = CPA4_result;
endmodule





module CPA_cpatree #(parameter N = 32)(
    input [N-1:0] a,
    input [N-1:0] b,
    input TRi,
    output [N-1:0] sum,
    output overflow
);
    assign sum = a + b + TRi;  // Simple binary addition
    assign overflow = (a[N-1] == b[N-1]) && (sum[N-1] != a[N-1]);
endmodule

module CPA_float_add#(parameter WIDTH32 = 32
)(
    input clk,
    inout rstn,
    input float_flag,
    input [WIDTH32-1:0] input_x0,
    input [WIDTH32-1:0] input_x1,
    input TRi,
    output [WIDTH32-1:0] final_result,
    output cpa_overflow,
    output reg overflow,
    output reg underflow
);
    
    //对阶
    wire [8:0] subexponet;
    wire [8:0] exponent0;
    wire [8:0] exponent1;
    assign exponent0 = {1'b0, input_x0[30:23]};
    assign exponent1 = {1'b1, ~input_x1[30:23]};
    assign subexponet = exponent0 + exponent1 + 1'b1;

    wire [7:0] normal_exponent = (subexponet[8]==1'b0) ? input_x0[30:23] : input_x1[30:23];//取较大的指数

    wire [22:0] Mantissa0;
    wire [22:0] Mantissa1;
    assign Mantissa0 = input_x0[22:0];
    assign Mantissa1 = input_x1[22:0];
    wire [8:0] abs_subexponet;
    assign abs_subexponet = (subexponet[8]==1'b0)?subexponet:(~subexponet+1'b1);

    wire [22:0] a;
    wire [23:0] b;
    assign a = (subexponet[8]==1'b1) ? Mantissa1 : Mantissa0;//不移动的尾数
    //@yuan: for the Denormalized （specially, the 0）
    wire [23:0] frac0, frac1;
    assign frac0 = (input_x0[30:23] == 8'b0) ? {1'b0, Mantissa0} : {1'b1, Mantissa0};
    assign frac1 = (input_x1[30:23] == 8'b0) ? {1'b0, Mantissa1} : {1'b1, Mantissa1};
    wire [23:0] input_shift;
    // assign input_shift = (subexponet[8]==1'b0) ? {1'b1, Mantissa1} : {1'b1, Mantissa0};
    assign input_shift = (subexponet[8]==1'b0) ? frac1 : frac0;

    wire [4:0] shift_b;
    assign shift_b = (abs_subexponet[8:5] == 4'b0000) ? abs_subexponet[4:0] : 5'b11111;
    assign b = input_shift>>shift_b;//对指数较小的尾数进行右移动，由于只有24bit，所以只取后面5bit即可
    // wire over_subexponent;
    // assign over_subexponent = ~(abs_subexponet[8:5] == 4'b0);//指数过大的情况，意味着，两个数相差过大
    //尾数求和
    // reg [25:0] add_a;
    reg [31:0] add_b;
    reg sub;
    wire [1:0] x_sign;
    assign x_sign = {input_x0[31],input_x1[31]};
    always @(*) begin
        case(x_sign)
            // 2'b00,2'b11:begin add_a = {3'b001, a}; add_b = {8'b00, b}; sub = 1'b0; end
            // 2'b01,2'b10:begin add_a = {3'b001, a}; add_b = {8'b11111111, ~b}; sub = 1'b1; end
            2'b00,2'b11:begin add_b = {8'b0, b}; sub = 1'b0; end
            2'b01,2'b10:begin add_b = {8'b11111111, ~b}; sub = 1'b1; end
        endcase
    end
    wire [31:0] result;
    wire [31:0] fixed_point_result;
    wire [31:0] cpa_a, cpa_b;
    wire cpa_TRi;
    wire cpa_overflow_mantissa;
    wire cpa_overflow_fixed_point;
    assign cpa_TRi = sub;
    assign cpa_a = {8'b0, 1'b1, a};
    assign cpa_b = add_b;
    //@yuan: for the mantissa addition
    CPA_cpatree cpa_0(
        .a(cpa_a),
        .b(cpa_b),
        .TRi(cpa_TRi),
        .sum(result),
        .overflow(cpa_overflow_mantissa)
    );
    //for the fixed point addition
    CPA_cpatree cpa_fixed_adder(
        .a(input_x0),
        .b(input_x1),
        .TRi(TRi),
        .sum(fixed_point_result),
        .overflow(cpa_overflow_fixed_point)
    );
    //规范化
    wire [25:0] abs_result;
    assign abs_result = (result[25]==1'b0) ? result[25:0] : (~result[25:0] + 1'b1);
    wire [5:0] k;//高位是符号位
    wire [22:0] new_result;
    FLOAT_ADD_LOD1 lod(
        .x_in(abs_result[24:0]),
        .k(k),
        .result(new_result)
    );
    wire [7:0] final_result_exponent;
    wire [22:0] final_result_Mantissa;
    wire [8:0] final_sub_exponent;
    wire [8:0] final_sub_k;
    //@yuan: the underflow and overflow signals should be delayed 1 cycle, as the same as the final result
    wire underflow_temp, overflow_temp;
    assign final_sub_k = ~{{3{k[5]}}, k} + 1'b1;
    assign final_sub_exponent = {1'b0, normal_exponent}+final_sub_k;
    assign underflow_temp = (input_x0[31] ^ input_x1[31]==1'b1) && (final_sub_exponent[8] ==1'b1 || k==6'b011111);//相减之后为0，或者指数减去k后小于0。

    assign overflow_temp = (normal_exponent[7:0] == 8'b11111111) && (input_x0[31] ^ input_x1[31]==1'b0);
    assign final_result_exponent = (underflow_temp==1'b1)? 8'b0:((overflow_temp==1'b1)? 8'b11111111: final_sub_exponent[7:0]);//下溢出时取最小的，上溢出取最大的
    assign final_result_Mantissa = (underflow_temp==1'b1)? 23'b0:((overflow_temp==1'b1)? 23'b11111111111111111111111 : new_result);
    reg final_sign;
    always @(*) begin
        case({input_x0[31], input_x1[31], subexponet[8], result[24]})
            4'b0000: final_sign=1'b0;
            4'b0001: final_sign=1'b0;
            4'b0010: final_sign=1'b0;
            4'b0011: final_sign=1'b0;
            4'b1100: final_sign=1'b1;
            4'b1101: final_sign=1'b1;
            4'b1110: final_sign=1'b1;
            4'b1111: final_sign=1'b1;
            4'b0100: final_sign=1'b0;
            4'b0101: final_sign=1'b1;
            4'b0110: final_sign=1'b1;
            4'b0111: final_sign=1'b0;
            4'b1000: final_sign=1'b1;
            4'b1001: final_sign=1'b0;
            4'b1010: final_sign=1'b0;
            4'b1011: final_sign=1'b1;
        endcase
    end
    wire [31:0] final_result_float;
    assign final_result_float = {final_sign, final_result_exponent, final_result_Mantissa};
    //TODO: Adding a Round module
    //@yuan: add one register at the ouput of float-addition to improving timing performance
    reg [WIDTH32 - 1 : 0] final_result_float_reg;
    reg cpa_overflow_mantissa_reg;
    always@(posedge clk or negedge rstn)begin
        if(!rstn)begin
            final_result_float_reg <= 0;
            overflow <= 1'b0;
            underflow <= 1'b0;
            cpa_overflow_mantissa_reg <= 1'b0;
        end else begin
            final_result_float_reg <= final_result_float;
            overflow <= overflow_temp;
            underflow <= underflow_temp;
            cpa_overflow_mantissa_reg <= cpa_overflow_mantissa;
        end
    end
    assign cpa_overflow = (float_flag==1'b1) ? cpa_overflow_mantissa_reg : cpa_overflow_fixed_point;
    assign final_result = (float_flag==1'b1) ? final_result_float_reg : fixed_point_result;
endmodule

module CPA_float_add_all_reg#(parameter WIDTH32 = 32
)(
    input clk,
    inout rstn,
    input float_flag,
    input [WIDTH32-1:0] input_x0,
    input [WIDTH32-1:0] input_x1,
    input TRi,
    output [WIDTH32-1:0] final_result,
    output reg cpa_overflow,
    output reg overflow,
    output reg underflow
);
    
    //对阶
    wire [8:0] subexponet;
    wire [8:0] exponent0;
    wire [8:0] exponent1;
    assign exponent0 = {1'b0, input_x0[30:23]};
    assign exponent1 = {1'b1, ~input_x1[30:23]};
    assign subexponet = exponent0 + exponent1 + 1'b1;

    wire [7:0] normal_exponent = (subexponet[8]==1'b0) ? input_x0[30:23] : input_x1[30:23];//取较大的指数

    wire [22:0] Mantissa0;
    wire [22:0] Mantissa1;
    assign Mantissa0 = input_x0[22:0];
    assign Mantissa1 = input_x1[22:0];
    wire [8:0] abs_subexponet;
    assign abs_subexponet = (subexponet[8]==1'b0)?subexponet:(~subexponet+1'b1);

    wire [22:0] a;
    wire [23:0] b;
    assign a = (subexponet[8]==1'b1) ? Mantissa1 : Mantissa0;//不移动的尾数
    //@yuan: for the Denormalized （specially, the 0）
    wire [23:0] frac0, frac1;
    assign frac0 = (input_x0[30:23] == 8'b0) ? {1'b0, Mantissa0} : {1'b1, Mantissa0};
    assign frac1 = (input_x1[30:23] == 8'b0) ? {1'b0, Mantissa1} : {1'b1, Mantissa1};
    wire [23:0] input_shift;
    // assign input_shift = (subexponet[8]==1'b0) ? {1'b1, Mantissa1} : {1'b1, Mantissa0};
    assign input_shift = (subexponet[8]==1'b0) ? frac1 : frac0;

    wire [4:0] shift_b;
    assign shift_b = (abs_subexponet[8:5] == 4'b0000) ? abs_subexponet[4:0] : 5'b11111;
    assign b = input_shift>>shift_b;//对指数较小的尾数进行右移动，由于只有24bit，所以只取后面5bit即可
    // assign b = input_shift>>abs_subexponet;//对指数较小的尾数进行右移动，由于只有24为，所以只取后面5bit尽可
    // wire over_subexponent;
    // assign over_subexponent = ~(abs_subexponet[8:5] == 4'b0);//指数过大的情况，意味着，两个数相差过大
    //尾数求和
    // reg [25:0] add_a;
    reg [31:0] add_b;
    reg sub;
    wire [1:0] x_sign;
    assign x_sign = {input_x0[31],input_x1[31]};
    always @(*) begin
        case(x_sign)
            // 2'b00,2'b11:begin add_a = {3'b001, a}; add_b = {8'b00, b}; sub = 1'b0; end
            // 2'b01,2'b10:begin add_a = {3'b001, a}; add_b = {8'b11111111, ~b}; sub = 1'b1; end
            2'b00,2'b11:begin add_b = {8'b0, b}; sub = 1'b0; end
            2'b01,2'b10:begin add_b = {8'b11111111, ~b}; sub = 1'b1; end
        endcase
    end
    wire [31:0] result;
    wire [31:0] cpa_a, cpa_b;
    wire cpa_TRi;
    wire cpa_overflow_temp;
    assign cpa_TRi = (float_flag==1'b1) ? sub : TRi;
    assign cpa_a = (float_flag==1'b1) ? {8'b0, 1'b1, a} : input_x0;
    assign cpa_b = (float_flag==1'b1) ? add_b : input_x1;

    CPA_cpatree cpa_0(
        .a(cpa_a),
        .b(cpa_b),
        .TRi(cpa_TRi),
        .sum(result),
        .overflow(cpa_overflow_temp)
    );

    //规范化
    wire [25:0] abs_result;
    assign abs_result = (result[25]==1'b0) ? result[25:0] : (~result[25:0] + 1'b1);
    wire [5:0] k;//高位是符号位
    wire [22:0] new_result;
    FLOAT_ADD_LOD1 lod(
        .x_in(abs_result[24:0]),
        .k(k),
        .result(new_result)
    );
    wire [7:0] final_result_exponent;
    wire [22:0] final_result_Mantissa;
    wire [8:0] final_sub_exponent;
    wire [8:0] final_sub_k;
    //@yuan: the underflow and overflow signals should be delayed 1 cycle, as the same as the final result
    wire underflow_temp, overflow_temp;
    assign final_sub_k = ~{{3{k[5]}}, k} + 1'b1;
    assign final_sub_exponent = {1'b0, normal_exponent}+final_sub_k;
    assign underflow_temp = (input_x0[31] ^ input_x1[31]==1'b1) && (final_sub_exponent[8] ==1'b1 || k==6'b011111);//相减之后为0，或者指数减去k后小于0。

    assign overflow_temp = (normal_exponent[7:0] == 8'b11111111) && (input_x0[31] ^ input_x1[31]==1'b0);
    assign final_result_exponent = (underflow_temp==1'b1)? 8'b0:((overflow_temp==1'b1)? 8'b11111111: final_sub_exponent[7:0]);//下溢出时取最小的，上溢出取最大的
    assign final_result_Mantissa = (underflow_temp==1'b1)? 23'b0:((overflow_temp==1'b1)? 23'b11111111111111111111111 : new_result);
    reg final_sign;
    always @(*) begin
        case({input_x0[31], input_x1[31], subexponet[8], result[24]})
            4'b0000: final_sign=1'b0;
            4'b0001: final_sign=1'b0;
            4'b0010: final_sign=1'b0;
            4'b0011: final_sign=1'b0;
            4'b1100: final_sign=1'b1;
            4'b1101: final_sign=1'b1;
            4'b1110: final_sign=1'b1;
            4'b1111: final_sign=1'b1;
            4'b0100: final_sign=1'b0;
            4'b0101: final_sign=1'b1;
            4'b0110: final_sign=1'b1;
            4'b0111: final_sign=1'b0;
            4'b1000: final_sign=1'b1;
            4'b1001: final_sign=1'b0;
            4'b1010: final_sign=1'b0;
            4'b1011: final_sign=1'b1;
        endcase
    end
    wire [31:0] final_result_float;
    assign final_result_float = {final_sign, final_result_exponent, final_result_Mantissa};
    //TODO: Adding a Round module
    //@yuan: add one register at the ouput of float-addition to improving timing performance
    reg [WIDTH32 - 1 : 0] final_result_float_reg;
    reg [WIDTH32 - 1 : 0] fixed_point_result_reg;
    always@(posedge clk or negedge rstn)begin
        if(!rstn)begin
            final_result_float_reg <= 0;
            fixed_point_result_reg <= 0;
            overflow <= 1'b0;
            underflow <= 1'b0;
            cpa_overflow <= 1'b0;
        end else begin
            final_result_float_reg <= final_result_float;
            fixed_point_result_reg <= result;
            overflow <= overflow_temp;
            underflow <= underflow_temp;
            cpa_overflow <= cpa_overflow_temp;
        end
    end
    // assign cpa_overflow = (float_flag==1'b1) ? cpa_overflow_mantissa_reg : cpa_overflow_fixed_point_reg;
    assign final_result = (float_flag==1'b1) ? final_result_float_reg : fixed_point_result_reg;
endmodule

// module CPA_float_add_last#(parameter WIDTH32 = 32
// )(
//     input clk,
//     inout rstn,
//     input float_flag,
//     input [WIDTH32-1:0] input_x0,
//     input [WIDTH32-1:0] input_x1,
//     input TRi,
//     output [WIDTH32-1:0] final_result,
//     output cpa_overflow,
//     output reg overflow,
//     output reg underflow
// );
    
//     //对阶
//     wire [8:0] subexponet;
//     wire [8:0] exponent0;
//     wire [8:0] exponent1;
//     assign exponent0 = {1'b0, input_x0[30:23]};
//     assign exponent1 = {1'b1, ~input_x1[30:23]};
//     assign subexponet = exponent0 + exponent1 + 1'b1;

//     wire [7:0] normal_exponent = (subexponet[8]==1'b0) ? input_x0[30:23] : input_x1[30:23];//取较大的指数

//     wire [22:0] Mantissa0;
//     wire [22:0] Mantissa1;
//     assign Mantissa0 = input_x0[22:0];
//     assign Mantissa1 = input_x1[22:0];
//     wire [8:0] abs_subexponet;
//     assign abs_subexponet = (subexponet[8]==1'b0)?subexponet:(~subexponet+1'b1);

//     wire [22:0] a;
//     wire [23:0] b;
//     assign a = (subexponet[8]==1'b1) ? Mantissa1 : Mantissa0;//不移动的尾数
//     //@yuan: for the Denormalized （specially, the 0）
//     wire [23:0] frac0, frac1;
//     assign frac0 = (input_x0[30:23] == 8'b0) ? {1'b0, Mantissa0} : {1'b1, Mantissa0};
//     assign frac1 = (input_x1[30:23] == 8'b0) ? {1'b0, Mantissa1} : {1'b1, Mantissa1};
//     wire [23:0] input_shift;
//     // assign input_shift = (subexponet[8]==1'b0) ? {1'b1, Mantissa1} : {1'b1, Mantissa0};
//     assign input_shift = (subexponet[8]==1'b0) ? frac1 : frac0;
//     assign b = input_shift>>abs_subexponet;//对指数较小的尾数进行右移动，由于只有24为，所以只取后面5bit尽可
//     wire over_subexponent;
//     assign over_subexponent = ~(abs_subexponet[8:5] == 4'b0);//指数过大的情况，意味着，两个数相差过大
//     //尾数求和
//     reg [25:0] add_a;
//     reg [31:0] add_b;
//     reg sub;
//     wire [1:0] x_sign;
//     assign x_sign = {input_x0[31],input_x1[31]};
//     always @(*) begin
//         case(x_sign)
//             2'b00,2'b11:begin add_a = {3'b001, a}; add_b = {8'b00, b}; sub = 1'b0; end
//             2'b01,2'b10:begin add_a = {3'b001, a}; add_b = {8'b11111111, ~b}; sub = 1'b1; end
//         endcase
//     end
//     wire [31:0] result;
//     wire [31:0] fixed_point_result;
//     wire [31:0] cpa_a, cpa_b;
//     wire cpa_TRi;
//     wire cpa_overflow_mantissa;
//     wire cpa_overflow_fixed_point;
//     assign cpa_TRi = sub;
//     assign cpa_a = {6'b0, add_a};
//     assign cpa_b = add_b;
//     //@yuan: for the mantissa addition
//     CPA_cpatree cpa_0(
//         .a(cpa_a),
//         .b(cpa_b),
//         .TRi(cpa_TRi),
//         .sum(result),
//         .overflow(cpa_overflow_mantissa)
//     );
//     //for the fixed point addition
//     CPA_cpatree cpa_fixed_adder(
//         .a(input_x0),
//         .b(input_x1),
//         .TRi(TRi),
//         .sum(fixed_point_result),
//         .overflow(cpa_overflow_fixed_point)
//     );
//     //规范化
//     wire [25:0] abs_result;
//     assign abs_result = (result[25]==1'b0) ? result[25:0] : (~result[25:0] + 1'b1);
//     wire [5:0] k;//高位是符号位
//     wire [22:0] new_result;
//     FLOAT_ADD_LOD1 lod(
//         .x_in(abs_result[24:0]),
//         .k(k),
//         .result(new_result)
//     );
//     wire [7:0] final_result_exponent;
//     wire [22:0] final_result_Mantissa;
//     wire [8:0] final_sub_exponent;
//     wire [8:0] final_sub_k;
//     //@yuan: the underflow and overflow signals should be delayed 1 cycle, as the same as the final result
//     wire underflow_temp, overflow_temp;
//     assign final_sub_k = ~{{3{k[5]}}, k} + 1'b1;
//     assign final_sub_exponent = {1'b0, normal_exponent}+final_sub_k;
//     assign underflow_temp = (input_x0[31] ^ input_x1[31]==1'b1) && (final_sub_exponent[8] ==1'b1 || k==6'b011111);//相减之后为0，或者指数减去k后小于0。

//     assign overflow_temp = (normal_exponent[7:0] == 8'b11111111) && (input_x0[31] ^ input_x1[31]==1'b0);
//     assign final_result_exponent = (underflow_temp==1'b1)? 8'b0:((overflow_temp==1'b1)? 8'b11111111: final_sub_exponent[7:0]);//下溢出时取最小的，上溢出取最大的
//     assign final_result_Mantissa = (underflow_temp==1'b1)? 23'b0:((overflow_temp==1'b1)? 23'b11111111111111111111111 : new_result);
//     reg final_sign;
//     always @(*) begin
//         case({input_x0[31], input_x1[31], subexponet[8], result[24]})
//             4'b0000: final_sign=1'b0;
//             4'b0001: final_sign=1'b0;
//             4'b0010: final_sign=1'b0;
//             4'b0011: final_sign=1'b0;
//             4'b1100: final_sign=1'b1;
//             4'b1101: final_sign=1'b1;
//             4'b1110: final_sign=1'b1;
//             4'b1111: final_sign=1'b1;
//             4'b0100: final_sign=1'b0;
//             4'b0101: final_sign=1'b1;
//             4'b0110: final_sign=1'b1;
//             4'b0111: final_sign=1'b0;
//             4'b1000: final_sign=1'b1;
//             4'b1001: final_sign=1'b0;
//             4'b1010: final_sign=1'b0;
//             4'b1011: final_sign=1'b1;
//         endcase
//     end
//     wire [31:0] final_result_float;
//     assign final_result_float = {final_sign, final_result_exponent, final_result_Mantissa};
//     //TODO: Adding a Round module
//     //@yuan: add one register at the ouput of float-addition to improving timing performance
//     reg [WIDTH32 - 1 : 0] final_result_float_reg;
//     reg [WIDTH32 - 1 : 0] fixed_point_result_reg;
//     reg cpa_overflow_mantissa_reg;
//     reg cpa_overflow_fixed_point_reg;
//     always@(posedge clk or negedge rstn)begin
//         if(!rstn)begin
//             final_result_float_reg <= 0;
//             fixed_point_result_reg <= 0;
//             overflow <= 1'b0;
//             underflow <= 1'b0;
//             cpa_overflow_fixed_point_reg <= 1'b0;
//             cpa_overflow_mantissa_reg <= 1'b0;
//         end else begin
//             final_result_float_reg <= final_result_float;
//             fixed_point_result_reg <= fixed_point_result;
//             overflow <= overflow_temp;
//             underflow <= underflow_temp;
//             cpa_overflow_fixed_point_reg <= cpa_overflow_fixed_point;
//             cpa_overflow_mantissa_reg <= cpa_overflow_mantissa;
//         end
//     end
//     assign cpa_overflow = (float_flag==1'b1) ? cpa_overflow_mantissa_reg : cpa_overflow_fixed_point_reg;
//     assign final_result = (float_flag==1'b1) ? final_result_float_reg : fixed_point_result_reg;
// endmodule

// module CPA_float_add#(parameter WIDTH32 = 32
// )(
//     input clk,
//     inout rstn,
//     input float_flag,
//     input [WIDTH32-1:0] input_x0,
//     input [WIDTH32-1:0] input_x1,
//     input TRi,
//     output [WIDTH32-1:0] final_result,
//     output cpa_overflow,
//     output overflow,
//     output underflow
// );
    
//     //对阶
//     wire [8:0] subexponet;
//     wire [8:0] exponent0;
//     wire [8:0] exponent1;
//     assign exponent0 = {1'b0, input_x0[30:23]};
//     assign exponent1 = {1'b1, ~input_x1[30:23]};
//     assign subexponet = exponent0 + exponent1 + 1'b1;

//     wire [7:0] normal_exponent = (subexponet[8]==1'b0) ? input_x0[30:23] : input_x1[30:23];//取较大的指数

//     wire [22:0] Mantissa0;
//     wire [22:0] Mantissa1;
//     assign Mantissa0 = input_x0[22:0];
//     assign Mantissa1 = input_x1[22:0];
//     wire [8:0] abs_subexponet;
//     assign abs_subexponet = (subexponet[8]==1'b0)?subexponet:(~subexponet+1'b1);

//     wire [22:0] a;
//     wire [23:0] b;
//     assign a = (subexponet[8]==1'b1) ? Mantissa1 : Mantissa0;//不移动的尾数
//     //@yuan: for the Denormalized （specially, the 0）
//     wire [23:0] frac0, frac1;
//     assign frac0 = (input_x0[30:23] == 8'b0) ? {1'b0, Mantissa0} : {1'b1, Mantissa0};
//     assign frac1 = (input_x1[30:23] == 8'b0) ? {1'b0, Mantissa1} : {1'b1, Mantissa1};
//     wire [23:0] input_shift;
//     // assign input_shift = (subexponet[8]==1'b0) ? {1'b1, Mantissa1} : {1'b1, Mantissa0};
//     assign input_shift = (subexponet[8]==1'b0) ? frac1 : frac0;
//     assign b = input_shift>>abs_subexponet;//对指数较小的尾数进行右移动，由于只有24为，所以只取后面5bit尽可
//     wire over_subexponent;
//     assign over_subexponent = ~(abs_subexponet[8:5] == 4'b0);//指数过大的情况，意味着，两个数相差过大
//     //尾数求和
//     reg [25:0] add_a;
//     reg [31:0] add_b;
//     reg sub;
//     wire [1:0] x_sign;
//     assign x_sign = {input_x0[31],input_x1[31]};
//     always @(*) begin
//         case(x_sign)
//             2'b00,2'b11:begin add_a = {3'b001, a}; add_b = {8'b00, b}; sub = 1'b0; end
//             2'b01,2'b10:begin add_a = {3'b001, a}; add_b = {8'b11111111, ~b}; sub = 1'b1; end
//         endcase
//     end
//     wire [31:0] result;
//     wire [31:0] cpa_a, cpa_b;
//     wire cpa_TRi;
//     assign cpa_TRi = (float_flag==1'b1) ? sub : TRi;
//     assign cpa_a = (float_flag==1'b1) ? {6'b0, add_a} : input_x0;
//     assign cpa_b = (float_flag==1'b1) ? add_b : input_x1;

//     CPA_cpatree cpa_0(
//         .a(cpa_a),
//         .b(cpa_b),
//         .TRi(cpa_TRi),
//         .sum(result),
//         .overflow(cpa_overflow)
//     );

//     //规范化
//     wire [25:0] abs_result;
//     assign abs_result = (result[25]==1'b0) ? result[25:0] : (~result[25:0] + 1'b1);
//     wire [5:0] k;//高位是符号位
//     wire [22:0] new_result;
//     FLOAT_ADD_LOD1 lod(
//         .x_in(abs_result[24:0]),
//         .k(k),
//         .result(new_result)
//     );
//     wire [7:0] final_result_exponent;
//     wire [22:0] final_result_Mantissa;
//     wire [8:0] final_sub_exponent;
//     wire [8:0] final_sub_k;
//     assign final_sub_k = ~{{3{k[5]}}, k} + 1'b1;
//     assign final_sub_exponent = {1'b0, normal_exponent}+final_sub_k;
//     assign underflow = (input_x0[31] ^ input_x1[31]==1'b1) && (final_sub_exponent[8] ==1'b1 || k==6'b011111);//相减之后为0，或者指数减去k后小于0。

//     assign overflow = (normal_exponent[7:0] == 8'b11111111) && (input_x0[31] ^ input_x1[31]==1'b0);
//     assign final_result_exponent = (underflow==1'b1)? 8'b0:((overflow==1'b1)? 8'b11111111: final_sub_exponent[7:0]);//下溢出时取最小的，上溢出取最大的
//     assign final_result_Mantissa = (underflow==1'b1)? 23'b0:((overflow==1'b1)? 23'b11111111111111111111111 : new_result);
//     reg final_sign;
//     always @(*) begin
//         case({input_x0[31], input_x1[31], subexponet[8], result[24]})
//             4'b0000: final_sign=1'b0;
//             4'b0001: final_sign=1'b0;
//             4'b0010: final_sign=1'b0;
//             4'b0011: final_sign=1'b0;
//             4'b1100: final_sign=1'b1;
//             4'b1101: final_sign=1'b1;
//             4'b1110: final_sign=1'b1;
//             4'b1111: final_sign=1'b1;
//             4'b0100: final_sign=1'b0;
//             4'b0101: final_sign=1'b1;
//             4'b0110: final_sign=1'b1;
//             4'b0111: final_sign=1'b0;
//             4'b1000: final_sign=1'b1;
//             4'b1001: final_sign=1'b0;
//             4'b1010: final_sign=1'b0;
//             4'b1011: final_sign=1'b1;
//         endcase
//     end
//     wire [31:0] final_result_float;
//     assign final_result_float = {final_sign, final_result_exponent, final_result_Mantissa};
//     //TODO: Adding a Round module
//     //@yuan: add one register at the ouput of float-addition to improving timing performance
//     // reg [WIDTH32 - 1 : 0] final_result_float_reg;
//     // always@(posedge clk or negedge rstn)begin
//     //     if(!rstn)begin
//     //         final_result_float_reg <= 0;
//     //     end else begin
//     //         final_result_float_reg <= final_result_float;
//     //     end
//     // end
//     assign final_result = (float_flag==1'b1) ? final_result_float : result;
// endmodule

//分组检测

module FLOAT_ADD_LOD1 #(parameter WIDTH = 24
)(
    input [WIDTH:0] x_in,
    output [5:0] k,
    output [22:0] result
);
    wire [4:0] k_;
    reg [22:0] result_;
    FLOAT_ADD_leading_one_detector_24 leading_one_detector_1 (.in(x_in[23:0]), .msb_pos(k_));
    always @(*) begin
        case (k_)
            5'd0: result_ = x_in[22:0];
            5'd1: result_ = {x_in[21:0], 1'b0};
            5'd2: result_ = {x_in[20:0], 2'b0};
            5'd3: result_ = {x_in[19:0], 3'b0};
            5'd4: result_ = {x_in[18:0], 4'b0};
            5'd5: result_ = {x_in[17:0], 5'b0};
            5'd6: result_ = {x_in[16:0], 6'b0};
            5'd7: result_ = {x_in[15:0], 7'b0};
            5'd8: result_ = {x_in[14:0], 8'b0};
            5'd9: result_ = {x_in[13:0], 9'b0};
            5'd10: result_ = {x_in[12:0], 10'b0};
            5'd11: result_ = {x_in[11:0], 11'b0};
            5'd12: result_ = {x_in[10:0], 12'b0};
            5'd13: result_ = {x_in[9:0], 13'b0};
            5'd14: result_ = {x_in[8:0], 14'b0};
            5'd15: result_ = {x_in[7:0], 15'b0};
            5'd16: result_ = {x_in[6:0], 16'b0};
            5'd17: result_ = {x_in[5:0], 17'b0};
            5'd18: result_ = {x_in[4:0], 18'b0};
            5'd19: result_ = {x_in[3:0], 19'b0};
            5'd20: result_ = {x_in[2:0], 20'b0};
            5'd21: result_ = {x_in[1:0], 21'b0};
            5'd22: result_ = {x_in[0], 22'b0};
            default:  result_ = {23'b0};
        endcase
    end
    assign result = (x_in[24] == 1'b1)?x_in[23:1]:result_;
    assign k = (x_in[24] == 1'b1) ? 6'b111111 : {1'b0, k_};

endmodule



module FLOAT_ADD_leading_one_detector_4bit (
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
        //     4'b1???: msb_pos = 2'd0;
        //     4'b01??: msb_pos = 2'd1;
        //     4'b001?: msb_pos = 2'd2;
        //     4'b0001: msb_pos = 2'd3;
        //     default: msb_pos = 2'd0;  // Default case (this should never happen)
        // endcase
        if(in[3]) msb_pos = 2'd0;
        else if(in[2]) msb_pos = 2'd1;
        else if(in[1]) msb_pos = 2'd2;
        else msb_pos = 2'd3;
    end
endmodule


module FLOAT_ADD_leading_one_detector_24 (
    input [23:0] in,     // 32-bit input
    output reg [4:0] msb_pos  // MSB position, 5-bit output (0-31)
);

    wire [1:0] msb_group_1, msb_group_2, msb_group_3, msb_group_4;
    wire [1:0] msb_group_5, msb_group_6;
    wire signal1, signal2, signal3, signal4;
    wire  signal5, signal6;

    // Instantiate 8 4-bit Leading-One Detectors for each group
    FLOAT_ADD_leading_one_detector_4bit group1 (.in(in[23:20]), .msb_pos(msb_group_1), .signal(signal1));
    FLOAT_ADD_leading_one_detector_4bit group2 (.in(in[19:16]), .msb_pos(msb_group_2), .signal(signal2));
    FLOAT_ADD_leading_one_detector_4bit group3 (.in(in[15:12]), .msb_pos(msb_group_3), .signal(signal3));
    FLOAT_ADD_leading_one_detector_4bit group4 (.in(in[11:8]), .msb_pos(msb_group_4), .signal(signal4));
    FLOAT_ADD_leading_one_detector_4bit group5 (.in(in[7:4]), .msb_pos(msb_group_5), .signal(signal5));
    FLOAT_ADD_leading_one_detector_4bit group6 (.in(in[3:0]), .msb_pos(msb_group_6), .signal(signal6));
    always @(*) begin
        // Combine results from each group to determine the final MSB position
        if (signal1) msb_pos = {3'b000, msb_group_1};  // Position in the first group (0-3)
        else if (signal2) msb_pos = {3'b001, msb_group_2};
        else if (signal3) msb_pos = {3'b010, msb_group_3};
        else if (signal4) msb_pos = {3'b011, msb_group_4};
        else if (signal5) msb_pos = {3'b100, msb_group_5};
        else if (signal6) msb_pos = {3'b101, msb_group_6};
        else msb_pos = 5'b11111;  // Default case (this should never happen)
    end
endmodule
