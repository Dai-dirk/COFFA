`timescale 1ns / 1ps
//@zou:多项式系数通过配置输入，最大支持段数为6段
module LNS_Top_7 #(parameter WIDTH = 32,
             parameter WIDTH32 = 32  
)(
    input clk,
    input rstn,
    input [4:0] n,
    input float_flag,
    input [WIDTH32 - 1:0] x_0,
    input [WIDTH32 - 1:0] y_0,
    // input [2:0] TRI_select,//@yuan: 选择具体的函数
    input VEC,//向量计算，或者乘除法计算
    input qi,//是否开方
    input Div,//是否除法
    //@zou：五个channel
    input [5*WIDTH-1:0] logc_in_0,//@yuan: 第一段多项式展开的logci输入
    input [WIDTH-3:0] K_in_0,//@yuan: 第一段多项式展开的ki输入
    input [5*WIDTH-1:0] logc_in_1,//@yuan: 第二段多项式展开的logci输入
    input [WIDTH-3:0] K_in_1,//@yuan: 第二段多项式展开的ki输入
    input [5*WIDTH-1:0] logc_in_2,//@yuan: 第三段多项式展开的logci输入
    input [WIDTH-3:0] K_in_2,//@yuan: 第三段多项式展开的ki输入
    input [5*WIDTH-1:0] logc_in_3,//@yuan: 第四段多项式展开的logci输入
    input [WIDTH-3:0] K_in_3,//@yuan: 第四段多项式展开的ki输入
    input [5*WIDTH-1:0] logc_in_4,//@yuan: 第五段多项式展开的logci输入
    input [WIDTH-3:0] K_in_4,//@yuan: 第五段多项式展开的ki输入
    input [5*WIDTH-1:0] logc_in_5,//@yuan: 第六段多项式展开的logci输入
    input [WIDTH-3:0] K_in_5,//@yuan: 第六段多项式展开的ki输入
    input [5*WIDTH-1:0] logc_in_6,//@yuan: 第七段多项式展开的logci输入
    input [WIDTH-3:0] K_in_6,//@yuan: 第七段多项式展开的ki输入
    input [13:0] bias_sel, //@yuan: 各段的偏移选择信号；每段 2-bit * 5段；每段： 00 无偏移， 01 偏移量为1， 10 偏移量为x
    input [6*WIDTH32-1 : 0] break_points, //@yuan: 因为最大的分段数是6，所以潜在的断点是5个；对于分段数不足6个的情况，对应的断点设置为当前数据格式下对应的正数的最大值：7FFFFFFF (由编译器决定)
    input [7*WIDTH32-1 : 0] constant_bias_in, //@yuan: 输入的每一段的bias

    input TRG,//@zou：booth计算5个6*Q8.22还是一个Q8.22*Q8.22
    input power,//指数函数
    input TRi,//三角函数
    output TRG_overflow,//24*24的溢出
    output VEC_overflow,
    output CPA_float_overflow,
    output CPA_float_underflow,
    output CPA_cpa_overflow,
    output [WIDTH32 - 1:0] channel0_log_result,
    output [WIDTH32 - 1:0] channel2_tri_result,
    output [WIDTH32 - 1:0] channel0_VEC_Power_result //@zou,每个类型都只有一个输出，且都转化成了Qm.n输出格式
);
    //wire power_sign;
    //@zou：默认计算指数时不会出现负数为底数且指数为小数的不正确情况
    //assign power_sign = ((float_flag == 1'b0 )&&(power == 1'b1) && (y_0[n] == 1'b0)) ? 1'b1 : 1'b0;//为1，则指数计算结果为正数，否则为x发符号
    //@yuan:取消bias_one在内部，这个可以通过常数项的配置烧入
    // reg [WIDTH32 - 1:0] fix_bias_one;
    // always @(*) begin
    //     fix_bias_one = (32'b1 << n);//@yuan: 使用移位即可
    // end
    // wire [WIDTH32 - 1:0] bias_one;//$zou, 浮点增加
    // assign bias_one = (float_flag==1'b1) ? 32'b00111111_10000000_00000000_00000000 : fix_bias_one;

    integer j;

    //@zou: 计算段系数选择
    //@yuan: 在第一个cycle进行段系数的选择，目的是节约寄存器的数量
    wire [5:0] segSel_Out;
    //@yuan: 根据输入选择对应范围段的系数
    reg [5*WIDTH-1:0] logc_stage_logc_custom;
    reg [WIDTH-3:0] logc_stage_K_custom;
    reg [WIDTH - 1:0] segment_bias_constant [6 : 0];

    //@yuan: each segment has its own constant bias
    always@(*)begin
        for(j = 0; j < 7; j = j + 1)begin
            segment_bias_constant[j] <= constant_bias_in[(j + 1)*WIDTH32 - 1 -: WIDTH32];
        end
    end
    // reg [WIDTH32-1:0] bias_custom;
    segSel_7 #(.WIDTH32(WIDTH32)) sel (
        .x_in(x_0),
        .is_fp(float_flag),
        .break_points_in(break_points),
        .log2_out(segSel_Out)
    );
    always @(*) begin
        casez(segSel_Out)
            6'b1?????: begin logc_stage_logc_custom = logc_in_6; logc_stage_K_custom = K_in_6; end
            6'b01????: begin logc_stage_logc_custom = logc_in_5; logc_stage_K_custom = K_in_5; end
            6'b001???: begin logc_stage_logc_custom = logc_in_4; logc_stage_K_custom = K_in_4; end
            6'b0001??: begin logc_stage_logc_custom = logc_in_3; logc_stage_K_custom = K_in_3; end
            6'b00001?: begin logc_stage_logc_custom = logc_in_2; logc_stage_K_custom = K_in_2; end
            6'b000001: begin logc_stage_logc_custom = logc_in_1; logc_stage_K_custom = K_in_1; end
            6'b000000: begin logc_stage_logc_custom = logc_in_0; logc_stage_K_custom = K_in_0; end
            default: begin logc_stage_logc_custom = logc_in_0; logc_stage_K_custom = K_in_0; end
        endcase
    end
    reg [WIDTH32 - 1:0] bias_custom;
    reg [WIDTH32 - 1:0] bias_seg [6:0];
    always@(*)begin
        for(j = 0; j < 7; j = j + 1)begin
            case(bias_sel[(j+1)*2 - 1 -: 2])
                2'b00: bias_seg[j] <= 32'b0;
                2'b01: bias_seg[j] <= segment_bias_constant[j];
                2'b10: bias_seg[j] <= x_0;
                2'b11: bias_seg[j] <= y_0; //@yuan: used by cascaed LNS
                default: bias_seg[j] <= 32'b0;
            endcase
        end
    end
    always @(*) begin
        casez(segSel_Out)
            6'b1?????: begin bias_custom = bias_seg[6]; end
            6'b01????: begin bias_custom = bias_seg[5]; end
            6'b001???: begin bias_custom = bias_seg[4]; end 
            6'b0001??: begin bias_custom = bias_seg[3]; end
            6'b00001?: begin bias_custom = bias_seg[2]; end
            6'b000001: begin bias_custom = bias_seg[1]; end
            6'b000000: begin bias_custom = bias_seg[0]; end
            default: begin bias_custom = bias_seg[0]; end
        endcase
    end

    //@zou: 流水线寄存器
    reg [WIDTH32 - 1:0] input_x;
    reg [WIDTH32 - 1:0] input_y;
    reg logc_stage_TRG;
    reg logc_stage_TRi;
    //reg logc_stage_power_sign;
    reg logc_stage_power;
    reg logc_stage_VEC;
    reg logc_stage_qi;
    reg logc_stage_Div;
    //@yuan: 段系数寄存器
    reg [5*WIDTH-1:0] logc_stage_logc_custom_reg;
    reg [WIDTH-3:0] logc_stage_K_custom_reg;
    reg [WIDTH32 - 1:0] logc_stage_bias;
    // reg [WIDTH32 - 1:0] bias_one_reg;
    // reg [11:0] bias_sel_reg;
    reg [4:0] segSel_Out_reg;
    // reg [2:0] logc_stage_TRI_select;
    reg [4:0] input_n;
    reg logc_stage_float_flag;

    always @(posedge clk or negedge rstn) begin
        if (rstn == 1'b0) begin
            input_x <= 32'b0;
            input_y <= 32'b0;
            input_n <= 5'b0;
            logc_stage_TRG <= 1'b1;
            logc_stage_TRi <= 1'b0;
            //logc_stage_power_sign <= 1'b0;
            logc_stage_power <= 1'b0;
            logc_stage_VEC <= 1'b0;
            logc_stage_qi <= 1'b1;
            logc_stage_Div <= 1'b0;
            logc_stage_logc_custom_reg <= 160'b0;
            logc_stage_K_custom_reg <= 31'b0;
            // bias_one_reg <= 32'b0;
            // bias_sel_reg <= 12'b0;
            logc_stage_bias <= 32'b0;
            segSel_Out_reg <= 5'b0;
            logc_stage_float_flag <= 1'b0;
        end 
        else begin
            // 在复位信号为高时，input_x 和 input_y 被赋值为 x_0 和 y_0
            input_x <= x_0;
            input_y <= y_0;
            input_n <= n;
            logc_stage_TRG <= TRG;
            logc_stage_TRi <= TRi;
            //logc_stage_power_sign <= power_sign;
            logc_stage_power <= power;
            logc_stage_VEC <= VEC;
            logc_stage_qi <= qi;
            logc_stage_Div <= Div;
            logc_stage_logc_custom_reg <= logc_stage_logc_custom;
            logc_stage_K_custom_reg <= logc_stage_K_custom;
            // bias_one_reg <= bias_one;
            // bias_sel_reg <= bias_sel;
            logc_stage_bias <= bias_custom;
            segSel_Out_reg <= segSel_Out;
            logc_stage_float_flag <= float_flag;
        end
    end

    //对数转换
    wire [WIDTH - 1:0] log_x;
    wire [WIDTH32-1:0] absx0;
    genvar i; // 定义生成变量
    converter logconv_x0 (
        .x_in(input_x),
        .n(input_n),
        .float_flag(logc_stage_float_flag),
        .abs_x(absx0),
        .log2_out(log_x)
    );

    //指数运算会转换成Q8.22*Q8.22的乘法运算，需要对y的数据格式进行转换
    wire [WIDTH-3:0] fix_new_y0;
    wire [WIDTH-3:0] float_new_y0;
    wire float_power_y0_overflow;
    wire float_power_y0_underflow;
    Tran_to_822 tran_to_822_0(
        .B0(input_y),
        .n(input_n),
        .new_B0(fix_new_y0)
    );
    FLOAT_TRAN_TO_Q822 float_tran_to_q822(
        .float_y(input_y),
        .Q822y(float_new_y0),
        .overflow_y(float_power_y0_overflow),
        .underflow_y(float_power_y0_underflow)
    );
    wire [WIDTH-3:0] new_y0;
    assign new_y0 = (logc_stage_float_flag==1'b1) ? float_new_y0 : fix_new_y0;

    wire logc_stage_power_sign;
    assign logc_stage_power_sign = ((logc_stage_power == 1'b1) && (new_y0[22] == 1'b0)) ? 1'b1 : 1'b0;//为1，则指数计算结果为正数，否则为x的符号

    wire [WIDTH-3:0] logc_stage_input_CSA_B;
    assign logc_stage_input_CSA_B = (logc_stage_TRG == 1'b0) ? new_y0 : logc_stage_K_custom_reg;//TRG为0有效，表示进行指数函数

    reg [WIDTH - 1:0] input_logA;
    reg [WIDTH32 - 1:0] input_B0;
    reg [WIDTH32 - 1:0] lns_stage_bias;
    reg lns_stage_TRG;
    reg lns_stage_TRi;
    reg lns_stage_power_sign;
    reg lns_stage_VEC;
    reg lns_stage_qi;
    reg lns_stage_Div;
    reg [5*WIDTH - 1:0] lns_stage_logc;
    // reg [WIDTH - 3:0] lns_stage_B;
    reg [WIDTH - 3:0] lns_stage_input_CSA_B;
    reg [4:0] lns_stage_n;
    reg lns_stage_float_flag;  

    //每一项泰勒展开：logci + ki * logx; logc_stage_logc_custom_reg： logci; logc_stage_K_custom_reg: ki
    always @(posedge clk or negedge rstn) begin
        if (~rstn) begin
            input_logA <= 32'b0;
            input_B0 <= 32'b0;
            lns_stage_logc <= 159'b0;
            // lns_stage_B <= 30'b0;
            lns_stage_input_CSA_B <= 30'b0;
            lns_stage_bias <= 32'b0;

            lns_stage_TRG <= 1'b1;
            lns_stage_TRi <= 1'b0;
            lns_stage_power_sign <= 1'b0;
            lns_stage_VEC <= 1'b0;
            lns_stage_qi <= 1'b1;
            lns_stage_Div <= 1'b0;
            lns_stage_n <= 5'b0;
            lns_stage_float_flag <= 1'b0;
        end else begin
            input_logA <= log_x;
            input_B0 <= input_y;//用于指数函数、对数转换
            lns_stage_logc <= logc_stage_logc_custom_reg;
            // lns_stage_B <= logc_stage_K_custom_reg;
            lns_stage_input_CSA_B <= logc_stage_input_CSA_B;

            lns_stage_bias <= logc_stage_bias;
            lns_stage_TRG <= logc_stage_TRG;
            lns_stage_TRi <= logc_stage_TRi;
            lns_stage_power_sign <= logc_stage_power_sign;
            lns_stage_VEC <= logc_stage_VEC;
            lns_stage_qi <= logc_stage_qi;
            lns_stage_Div <= logc_stage_Div;
            lns_stage_n <= input_n;
            lns_stage_float_flag <= logc_stage_float_flag;
        end
    end

    wire [9:0] y_zero_sign_k0;//@zou, CSA只处理一个y0
    wire [WIDTH + 5:0] cpa_channel0, cpa_channel1, cpa_channel2, cpa_channel3, cpa_channel4;
    CSA_tree csa_tree(
        .A(input_logA),
        .B(lns_stage_input_CSA_B),
        .n(lns_stage_n),
        .float_flag(lns_stage_float_flag),
        .B0(input_B0),
        
        .log_c1(lns_stage_logc[5*WIDTH-3:4*WIDTH]),
        .log_c2(lns_stage_logc[4*WIDTH-3:3*WIDTH]),
        .log_c3(lns_stage_logc[3*WIDTH-3:2*WIDTH]),
        .log_c4(lns_stage_logc[2*WIDTH-3:WIDTH]),
        .log_c5(lns_stage_logc[WIDTH-3:0]),
        .TRG(lns_stage_TRG),
        .VEC(lns_stage_VEC),

        .zero_sign_k0(y_zero_sign_k0),
        .cpa_channel0(cpa_channel0),
        .cpa_channel1(cpa_channel1),
        .cpa_channel2(cpa_channel2),
        .cpa_channel3(cpa_channel3),
        .cpa_channel4(cpa_channel4)
    );

    reg alogc_stage_float_flag; 
    reg alogc_stage_TRi;
    reg alogc_stage_TRG;
    reg alogc_stage_VEC;
    reg alogc_stage_qi;
    reg alogc_stage_Div;
    reg alogc_stage_power_sign;
    reg alogc_stage_tri_sign [4:0];

    reg alogc_stage_logA_sign;
    reg alogc_stage_B_sign [4:0];//指数项是否为偶数
    reg [WIDTH32-1:0] alogc_stage_bias;
    reg [WIDTH-1:0] alogc_logA;//@yuan: 只有一个输入通道
    reg [9:0] alogc_y_zero_sign_k0;
    reg [WIDTH+5:0] alogc_cpa_channel [4:0];
    reg [1:0] alogc_stage_logc_zero_sign [4:0];
    reg [4:0] alogc_stage_n;
    reg alogc_stage_TRG_sign_B;

    always @(posedge clk or negedge rstn) begin
        if (~rstn) begin
            // 当复位信号为低时，所有 input_x 和 input_y 都被清零
            alogc_logA <= 32'b0;//@yuan: 只有一个输入通道
            for (j = 0; j < 5; j = j + 1) begin
                alogc_stage_tri_sign[j] <= 1'b0;
                alogc_stage_B_sign[j] <= 1'b0;
                alogc_cpa_channel[j] <= 32'b0;
                alogc_stage_logc_zero_sign[j] <= 2'b00;
            end
            alogc_y_zero_sign_k0 <= 10'b0;
            alogc_stage_qi <= 1'b1;
            alogc_stage_Div <= 1'b0;
            alogc_stage_bias <= 32'b0;
            alogc_stage_TRi <= 1'b0;
            alogc_stage_TRG <= 1'b1;
            alogc_stage_VEC <= 1'b0;
            alogc_stage_power_sign <= 1'b0;
            alogc_stage_logA_sign <= 1'b0;
            alogc_stage_n <= 5'b0;  
            alogc_stage_TRG_sign_B <= 1'b0;
            alogc_stage_float_flag <= 1'b0;

        end else begin
            // 在复位信号为高时，input_x 和 input_y 被赋值为 x_0 和 y_0
            alogc_logA <= input_logA;
            for (j = 0; j < 5; j = j + 1) begin
                alogc_stage_tri_sign[j] <= lns_stage_input_CSA_B[6*j];//@zou:分段近似指数项是否为偶数
                alogc_stage_B_sign[j] <= lns_stage_input_CSA_B[6*j+5];//@zou:分段近似指数项是否为正数
            end
            alogc_stage_logc_zero_sign[0] <= lns_stage_logc[5*WIDTH-1:5*WIDTH-2];
            alogc_stage_logc_zero_sign[1] <= lns_stage_logc[4*WIDTH-1:4*WIDTH-2];
            alogc_stage_logc_zero_sign[2] <= lns_stage_logc[3*WIDTH-1:3*WIDTH-2];
            alogc_stage_logc_zero_sign[3] <= lns_stage_logc[2*WIDTH-1:2*WIDTH-2];
            alogc_stage_logc_zero_sign[4] <= lns_stage_logc[WIDTH-1:WIDTH-2];

            alogc_cpa_channel[0] <= cpa_channel0;
            alogc_cpa_channel[1] <= cpa_channel1;
            alogc_cpa_channel[2] <= cpa_channel2;
            alogc_cpa_channel[3] <= cpa_channel3;
            alogc_cpa_channel[4] <= cpa_channel4;

            alogc_y_zero_sign_k0 <= y_zero_sign_k0;
            
            alogc_stage_qi <= lns_stage_qi;
            alogc_stage_Div <= lns_stage_Div;
            alogc_stage_bias <= lns_stage_bias;
            alogc_stage_TRi <= lns_stage_TRi;
            alogc_stage_TRG <= lns_stage_TRG;
            alogc_stage_VEC <= lns_stage_VEC;
            alogc_stage_power_sign <= lns_stage_power_sign;
            alogc_stage_logA_sign <= input_logA[WIDTH - 3];
            alogc_stage_n <= lns_stage_n;
            alogc_stage_TRG_sign_B <= input_B0[31];
            alogc_stage_float_flag <= lns_stage_float_flag;
        end
    end

    // 补一位的原因在于防止溢出，溢出在sat中判断；但实际可以直接在CPA中判断
    wire [WIDTH-2: 0] shift_log_y;
    assign shift_log_y = (alogc_stage_qi == 1'b0) ? {{2{alogc_y_zero_sign_k0[7]}}, alogc_y_zero_sign_k0[7:0], alogc_cpa_channel[0][21:1]}: {alogc_y_zero_sign_k0[7], alogc_y_zero_sign_k0[7:0], alogc_cpa_channel[0][21:0]};
    wire [WIDTH-2:0] CPA_ALOGC_a;
    assign CPA_ALOGC_a = {alogc_logA[WIDTH-3], alogc_logA[WIDTH-3 : 0]};
    wire [WIDTH-2:0] CPA_ALOGC_sum;
    //@yuan: 指数的加减也只有一个通道
    CPA_ALOGC CPA_ALOGC_inst0 (
        .a(CPA_ALOGC_a),
        .b(shift_log_y),
        .Div(alogc_stage_Div),
        .sum(CPA_ALOGC_sum)
    );

    wire [WIDTH+5:0] input_sat [4:0];
    assign input_sat[0] = (alogc_stage_VEC == 1'b1) ? ((CPA_ALOGC_sum[WIDTH-2] == 1'b0) ? {7'b0000000, CPA_ALOGC_sum} : {7'b1111111, CPA_ALOGC_sum}) : alogc_cpa_channel[0];
    assign input_sat[1] = alogc_cpa_channel[1];
    assign input_sat[2] = alogc_cpa_channel[2];
    assign input_sat[3] = alogc_cpa_channel[3];
    assign input_sat[4] = alogc_cpa_channel[4];

    wire [WIDTH-1:0] pre_input_anti [4:0];
    wire Tri_overflow_one [4:0];
    wire SAT_VEC_overflow;

    SAT_with_TRG_VEC_overflow sat_inst0(
        .input_sat(input_sat[0]),
        .TRi(alogc_stage_TRi),
        .TRG(alogc_stage_TRG),
        .VEC(alogc_stage_VEC),
        .TRG_B_sign(alogc_stage_TRG_sign_B), //计算power时，y的符号
        .Tri_sign(alogc_stage_tri_sign[0]), //三角函数计算时，判断如果指数是偶数，则结果是正数
        .alogc_stage_power_sign(alogc_stage_power_sign),//计算power时，判断如果指数是偶数，则结果是正数
        .input_logA_zero_sign(alogc_logA[WIDTH-1:WIDTH-2]),//logA，A的零标志和负标志
        .input_logB_zero_sign(alogc_y_zero_sign_k0[9:8]), //VEC计算时，B的零标志和负标志
        .input_logC_zero_sign(alogc_stage_logc_zero_sign[0]),//三角函数计算式，ci的零标志和负标志
        .alogc_stage_logA_sign(alogc_stage_logA_sign),//logA的正负标志
        .alogc_stage_B_sign(alogc_stage_B_sign[0]),//三角函数计算时，指数的正负标志

        .Tri_overflow_one(Tri_overflow_one[0]),
        .TRG_overflow(TRG_overflow),//TOP输出
        .VEC_overflow(SAT_VEC_overflow),
        .input_anti(pre_input_anti[0])
    );

    generate
        for (i = 1; i < 5; i = i + 1) begin: sat_gen // 给生成的代码块命名为 sat_gen
            SAT_with_VEC_overflow sat_inst (
                .input_sat(input_sat[i]),
                .TRi(alogc_stage_TRi),
                .Tri_sign(alogc_stage_tri_sign[i]),
                .input_logA_zero_sign(alogc_logA[WIDTH-1:WIDTH-2]),
                .input_logC_zero_sign(alogc_stage_logc_zero_sign[i]), //应该是logc的zero和符号位
                .alogc_stage_logA_sign(alogc_stage_logA_sign),
                .alogc_stage_B_sign(alogc_stage_B_sign[i]),
                .Tri_overflow_one(Tri_overflow_one[i]),
                .input_anti(pre_input_anti[i])
            );
        end
    endgenerate


    wire [WIDTH32-1:0] alogc_stage_log_result0;
    wire [WIDTH32-1:0] alogc_stage_log_result0_fix;
    wire [WIDTH32-1:0] alogc_stage_log_result0_float;
    Tran_log_to_mn tran_log_to_mn_0(
        .log30(pre_input_anti[0][WIDTH-3:0]),
        .n(alogc_stage_n),
        .log32(alogc_stage_log_result0_fix)
    );
    Tran_Q822_to_float tran_q822_to_float0(
        .log30(pre_input_anti[0][WIDTH-3:0]),
        .log32(alogc_stage_log_result0_float)
    );
    assign alogc_stage_log_result0 = (alogc_stage_float_flag==1'b1) ? alogc_stage_log_result0_float : alogc_stage_log_result0_fix;

    wire [WIDTH32-1:0] output_anti [4:0];
    generate
        for (i = 0; i < 5; i = i + 1) begin: anticonv_gen // 给生成的代码块命名为 sat_gen
            anticonverter anticonv_inst (
                .x(pre_input_anti[i]),
                .float_flag(alogc_stage_float_flag),
                .Tri_overflow_one(Tri_overflow_one[i]),
                .n(alogc_stage_n),
                .x_output(output_anti[i])
            );
        end
    endgenerate

    //指数对数在第4周期输出
    reg [WIDTH32-1:0] channel0_log_result0;
    reg [WIDTH32-1:0] CPA_tree_input_t [4:0];
    reg [WIDTH32-1:0] fxp_stage_bias;
    reg fxp_stage_TRi;
    reg fxp_stage_VEC;
    reg fxp_stage_SAT_VEC_overflow;
    reg fxp_stage_float_flag;

    always @(posedge clk or negedge rstn) begin
        if (~rstn) begin
            for (j = 0; j < 5; j = j + 1) begin
                CPA_tree_input_t[j] <= 32'b0; 
            end
            fxp_stage_SAT_VEC_overflow <= 1'b0;
            fxp_stage_bias <= 32'b0;
            fxp_stage_TRi <= 1'b0;
            fxp_stage_VEC <= 1'b0;
            channel0_log_result0 <= 32'b0;
            fxp_stage_float_flag <= 1'b0;
        end
        else begin
            for (j = 0; j < 5; j = j + 1) begin
                CPA_tree_input_t[j] <= output_anti[j];
            end
            fxp_stage_SAT_VEC_overflow <= SAT_VEC_overflow;
            fxp_stage_TRi <= alogc_stage_TRi;
            fxp_stage_VEC <= alogc_stage_VEC;
            fxp_stage_float_flag <= alogc_stage_float_flag;

            fxp_stage_bias <= alogc_stage_bias;
            channel0_log_result0 <= alogc_stage_log_result0;
            
        end
    end

    assign VEC_overflow = fxp_stage_VEC & fxp_stage_SAT_VEC_overflow;
    wire [WIDTH32-1:0] channel2_tri_result0;
    CPA_tree_with_MAD cpa_tree(
        .clk(clk),
        .rstn(rstn),
        .t0(CPA_tree_input_t[0]),
        .t1(CPA_tree_input_t[1]),
        .t2(CPA_tree_input_t[2]),
        .t3(CPA_tree_input_t[3]),
        .t4(CPA_tree_input_t[4]),
        .float_flag(fxp_stage_float_flag),
        .bias(fxp_stage_bias),

        .float_overflow(CPA_float_overflow),
        .float_underflow(CPA_float_underflow),
        .cpa_overflow(CPA_cpa_overflow),
        .tri_result(channel2_tri_result0)
    );

    //VEC、指数、对数在第四个clk后输出
    assign channel0_VEC_Power_result = CPA_tree_input_t[0]; //@yuan: 指数和VEC结果

    assign channel0_log_result = channel0_log_result0;// 对数结果

    assign channel2_tri_result = channel2_tri_result0;//三角函数结果



endmodule


module Tran_to_822 #(parameter WIDTH = 32,
                    parameter WIDTH32 = 32)
(
    input [WIDTH32-1:0] B0,
    input [4:0] n,
    output reg [WIDTH-3:0] new_B0
);
//保留符号位
    always @(*) begin
        case(n)
            5'd0: new_B0 = {B0[WIDTH32-1], B0[6:0], 22'b0};
            5'd1: new_B0 = {B0[WIDTH32-1], B0[7:0], 21'b0};
            5'd2: new_B0 = {B0[WIDTH32-1], B0[8:0], 20'b0};
            5'd3: new_B0 = {B0[WIDTH32-1], B0[9:0], 19'b0};
            5'd4: new_B0 = {B0[WIDTH32-1], B0[10:0], 18'b0};
            5'd5: new_B0 = {B0[WIDTH32-1], B0[11:0], 17'b0};
            5'd6: new_B0 = {B0[WIDTH32-1], B0[12:0], 16'b0};
            5'd7: new_B0 = {B0[WIDTH32-1], B0[13:0], 15'b0};
            5'd8: new_B0 = {B0[WIDTH32-1], B0[14:0], 14'b0};
            5'd9: new_B0 = {B0[WIDTH32-1], B0[15:0], 13'b0};
            5'd10: new_B0 = {B0[WIDTH32-1], B0[16:0], 12'b0};
            5'd11: new_B0 = {B0[WIDTH32-1], B0[17:0], 11'b0};
            5'd12: new_B0 = {B0[WIDTH32-1], B0[18:0], 10'b0};
            5'd13: new_B0 = {B0[WIDTH32-1], B0[19:0], 9'b0};
            5'd14: new_B0 = {B0[WIDTH32-1], B0[20:0], 8'b0};
            5'd15: new_B0 = {B0[WIDTH32-1], B0[21:0], 7'b0};
            5'd16: new_B0 = {B0[WIDTH32-1], B0[22:0], 6'b0};
            5'd17: new_B0 = {B0[WIDTH32-1], B0[23:0], 5'b0};
            5'd18: new_B0 = {B0[WIDTH32-1], B0[24:0], 4'b0};
            5'd19: new_B0 = {B0[WIDTH32-1], B0[25:0], 3'b0};
            5'd20: new_B0 = {B0[WIDTH32-1], B0[26:0], 2'b0};
            5'd21: new_B0 = {B0[WIDTH32-1], B0[27:0], 1'b0};
            5'd22: new_B0 = {B0[WIDTH32-1], B0[28:0]};
            5'd23: new_B0 = {B0[WIDTH32-1], B0[29:1]};
            5'd24: new_B0 = {B0[WIDTH32-1], B0[30:2]};
            5'd25: new_B0 = {B0[31], B0[31:3]};
            5'd26: new_B0 = {{2{B0[31]}}, B0[31:4]};
            5'd27: new_B0 = {{3{B0[31]}}, B0[31:5]};
            5'd28: new_B0 = {{4{B0[31]}}, B0[31:6]};
            5'd29: new_B0 = {{5{B0[31]}}, B0[31:7]};
            5'd30: new_B0 = {{6{B0[31]}}, B0[31:8]};
            5'd31: new_B0 = {{7{B0[31]}}, B0[31:9]};
            default: new_B0 = B0[31:2];
        endcase
    end
endmodule


module FLOAT_TRAN_TO_Q822(
    input [31:0] float_y,
    output reg [29:0] Q822y,
    output overflow_y,
    output underflow_y
);

    wire [7:0] exponent;
    assign exponent = {1'b0, float_y[30:23]} + 8'b10000001;
    wire [24:0] Q2_23;
    assign Q2_23 = (float_y[31] == 1'b0) ? {1'b0, 1'b1, float_y[22:0]} : {1'b1, 1'b0, ~float_y[22:0]} + 1'b1;
    reg incase;
    always @(*) begin
        case(exponent)
            8'b00000000: begin Q822y = {{6{Q2_23[24]}}, Q2_23[24:1]}; incase = 1'b1; end
            8'b00000001: begin Q822y = {{5{Q2_23[24]}}, Q2_23[24:0]}; incase = 1'b1; end
            8'b00000010: begin Q822y = {{4{Q2_23[24]}}, Q2_23[24:0], 1'b0}; incase = 1'b1; end
            8'b00000011: begin Q822y = {{3{Q2_23[24]}}, Q2_23[24:0], 2'b0}; incase = 1'b1; end
            8'b00000100: begin Q822y = {{2{Q2_23[24]}}, Q2_23[24:0], 3'b0}; incase = 1'b1; end
            8'b00000101: begin Q822y = {{1{Q2_23[24]}}, Q2_23[24:0], 4'b0}; incase = 1'b1; end
            8'b00000110: begin Q822y = {Q2_23[24:0], 4'b0}; incase = 1'b1; end
            8'b11111111: begin Q822y = {{7{Q2_23[24]}}, Q2_23[24:2]}; incase = 1'b1; end
            8'b11111110: begin Q822y = {{8{Q2_23[24]}}, Q2_23[24:3]}; incase = 1'b1; end
            8'b11111101: begin Q822y = {{9{Q2_23[24]}}, Q2_23[24:4]}; incase = 1'b1; end
            8'b11111100: begin Q822y = {{10{Q2_23[24]}}, Q2_23[24:5]}; incase = 1'b1; end
            8'b11111011: begin Q822y = {{11{Q2_23[24]}}, Q2_23[24:6]}; incase = 1'b1; end
            8'b11111010: begin Q822y = {{12{Q2_23[24]}}, Q2_23[24:7]}; incase = 1'b1; end
            8'b11111001: begin Q822y = {{13{Q2_23[24]}}, Q2_23[24:8]}; incase = 1'b1; end
            8'b11111000: begin Q822y = {{14{Q2_23[24]}}, Q2_23[24:9]}; incase = 1'b1; end
            8'b11110111: begin Q822y = {{15{Q2_23[24]}}, Q2_23[24:10]}; incase = 1'b1; end
            8'b11110110: begin Q822y = {{16{Q2_23[24]}}, Q2_23[24:11]}; incase = 1'b1; end
            8'b11110101: begin Q822y = {{17{Q2_23[24]}}, Q2_23[24:12]}; incase = 1'b1; end
            8'b11110100: begin Q822y = {{18{Q2_23[24]}}, Q2_23[24:13]}; incase = 1'b1; end
            8'b11110011: begin Q822y = {{19{Q2_23[24]}}, Q2_23[24:14]}; incase = 1'b1; end
            8'b11110010: begin Q822y = {{20{Q2_23[24]}}, Q2_23[24:15]}; incase = 1'b1; end
            8'b11110001: begin Q822y = {{21{Q2_23[24]}}, Q2_23[24:16]}; incase = 1'b1; end
            8'b11110000: begin Q822y = {{22{Q2_23[24]}}, Q2_23[24:17]}; incase = 1'b1; end
            default: begin Q822y = {30'b0}; incase = 1'b0; end
        endcase
    end
    assign overflow_y = (exponent[7] == 1'b0 && incase == 1'b0);
    assign underflow_y = (exponent[7] == 1'b1 && incase == 1'b0);

endmodule


module SAT_with_TRG_VEC_overflow #(parameter WIDTH = 32)(
    input [WIDTH+5:0] input_sat,
    input TRi,
    input TRG,
    input VEC,
    input TRG_B_sign,//计算power时，y的符号
    input Tri_sign, //三角函数计算时，判断如果指数是偶数，则结果是正数
    input alogc_stage_power_sign,//计算power时，判断如果指数是偶数，则结果是正数
    input [1:0] input_logA_zero_sign,//logA，A的零标志和负标志
    input [1:0] input_logB_zero_sign,//VEC计算时，B的零标志和负标志
    input [1:0] input_logC_zero_sign,//三角函数计算式，ci的零标志和负标志
    input alogc_stage_logA_sign,//logA的正负标志
    input alogc_stage_B_sign,//三角函数计算时，指数的正负标志
    output reg Tri_overflow_one,
    output reg TRG_overflow,
    output reg VEC_overflow,
    output reg [WIDTH-1:0] input_anti
);
always @(*) begin
    if(VEC == 1'b1)
        input_anti = {input_logA_zero_sign[1]|input_logB_zero_sign[1], input_logA_zero_sign[0]^input_logB_zero_sign[0], input_sat[WIDTH-3:0]};
        //这里对于开方的情况，没有进行考虑，即如果输入的B是负数也可以进行开方，相当于B的符号在开方符号外，所以要求开方输入的值不能是负数
    else if(alogc_stage_power_sign == 1'b1)
        input_anti = {input_logA_zero_sign[1], 1'b0, input_sat[WIDTH-3:0]};
    else if(TRG==1'b0 && input_logB_zero_sign[1] == 1'b1)
        input_anti = 32'b0;
    else if(TRG==1'b0)
        input_anti = {input_logA_zero_sign[1], input_logA_zero_sign[0], input_sat[WIDTH-3:0]};
    else if(TRi == 1'b1 && Tri_sign == 1'b0)
        // input_anti = {input_logC_zero_sign[1] | (input_logA_zero_sign[1] & (~input_logB_zero_sign[1])), 1'b0^input_logC_zero_sign[0], input_sat[WIDTH-3:0]};//@yuan: we treat 0^0 = 1 here
        input_anti = {input_logA_zero_sign[1]|input_logC_zero_sign[1], 1'b0^input_logC_zero_sign[0], input_sat[WIDTH-3:0]};
    else
        input_anti = {input_logA_zero_sign[1]|input_logC_zero_sign[1], input_logA_zero_sign[0]^input_logC_zero_sign[0], input_sat[WIDTH-3:0]};
    
    if(TRi == 1'b0 || (TRi==1'b1 && ( (input_sat[WIDTH+5:WIDTH-3] == 9'b111111111) || (input_sat[WIDTH+5:WIDTH-3] == 9'b000000000))))
        Tri_overflow_one = 1'b0;
    else
        Tri_overflow_one = 1'b1;
    
    if(TRG == 1'b1 || (TRG==1'b0 && ((alogc_stage_logA_sign^TRG_B_sign == 1'b1 && input_sat[WIDTH+5:WIDTH-3] == 9'b111111111) || (alogc_stage_logA_sign^TRG_B_sign == 1'b0 && input_sat[WIDTH+5:WIDTH-3] == 9'b000000000))))
        TRG_overflow = 1'b0;
    else
        TRG_overflow = 1'b1;
    
    if(VEC==1'b0 || (VEC == 1'b1 && (input_sat[WIDTH+5:WIDTH-3] == 9'b111111111 || input_sat[WIDTH+5:WIDTH-3] == 9'b000000000)))
        VEC_overflow = 1'b0;
    else
        VEC_overflow = 1'b1;
end
endmodule


module SAT_with_VEC_overflow #(parameter WIDTH = 32)(
    input [WIDTH+5:0] input_sat,
    input TRi,
    input [1:0] input_logA_zero_sign,
    input [1:0] input_logC_zero_sign,
    input alogc_stage_logA_sign,
    input alogc_stage_B_sign,
    input Tri_sign,
    output reg Tri_overflow_one,
    output reg [WIDTH-1:0] input_anti

);

always @(*) begin
    if(TRi == 1'b1 && Tri_sign == 1'b0)
        input_anti = {input_logA_zero_sign[1]|input_logC_zero_sign[1], 1'b0^input_logC_zero_sign[0], input_sat[WIDTH-3:0]};
    else
        input_anti = {input_logA_zero_sign[1]|input_logC_zero_sign[1], input_logA_zero_sign[0]^input_logC_zero_sign[0], input_sat[WIDTH-3:0]};
    
    if(TRi == 1'b0 || (TRi==1'b1 && ( (input_sat[WIDTH+5:WIDTH-3] == 9'b111111111) || (input_sat[WIDTH+5:WIDTH-3] == 9'b000000000))))
        Tri_overflow_one = 1'b0;
    else
        Tri_overflow_one = 1'b1;
end
endmodule

module Tran_log_to_mn #(parameter WIDTH = 32,
                    parameter WIDTH32 = 32)
(
        input [WIDTH-3:0] log30,
        input [4:0] n,
        output reg [WIDTH32-1:0] log32
);
always @(*) begin
    case(n)
        5'd0: log32 = { {24{log30[WIDTH-3]}}, log30[WIDTH-3:22]};
        5'd1: log32 = { {23{log30[WIDTH-3]}}, log30[WIDTH-3:21]};
        5'd2: log32 = { {22{log30[WIDTH-3]}}, log30[WIDTH-3:20]};
        5'd3: log32 = { {21{log30[WIDTH-3]}}, log30[WIDTH-3:19]};
        5'd4: log32 = { {20{log30[WIDTH-3]}}, log30[WIDTH-3:18]};
        5'd5: log32 = { {19{log30[WIDTH-3]}}, log30[WIDTH-3:17]};
        5'd6: log32 = { {18{log30[WIDTH-3]}}, log30[WIDTH-3:16]};
        5'd7: log32 = { {17{log30[WIDTH-3]}}, log30[WIDTH-3:15]};
        5'd8: log32 = { {16{log30[WIDTH-3]}}, log30[WIDTH-3:14]};
        5'd9: log32 = { {15{log30[WIDTH-3]}}, log30[WIDTH-3:13]};
        5'd10: log32 = { {14{log30[WIDTH-3]}}, log30[WIDTH-3:12]};
        5'd11: log32 = { {13{log30[WIDTH-3]}}, log30[WIDTH-3:11]};
        5'd12: log32 = { {12{log30[WIDTH-3]}}, log30[WIDTH-3:10]};
        5'd13: log32 = { {11{log30[WIDTH-3]}}, log30[WIDTH-3:9]};
        5'd14: log32 = { {10{log30[WIDTH-3]}}, log30[WIDTH-3:8]};
        5'd15: log32 = { {9{log30[WIDTH-3]}}, log30[WIDTH-3:7]};
        5'd16: log32 = { {8{log30[WIDTH-3]}}, log30[WIDTH-3:6]};
        5'd17: log32 = { {7{log30[WIDTH-3]}}, log30[WIDTH-3:5]};
        5'd18: log32 = { {6{log30[WIDTH-3]}}, log30[WIDTH-3:4]};
        5'd19: log32 = { {5{log30[WIDTH-3]}}, log30[WIDTH-3:3]};
        5'd20: log32 = { {4{log30[WIDTH-3]}}, log30[WIDTH-3:2]};
        5'd21: log32 = { {3{log30[WIDTH-3]}}, log30[WIDTH-3:1]};
        5'd22: log32 = { {2{log30[WIDTH-3]}}, log30[WIDTH-3:0]};
        5'd23: log32 = { {1{log30[WIDTH-3]}}, log30[WIDTH-3:0], 1'b0};
        5'd24: log32 = {log30[WIDTH-3:0], 2'b0};
        5'd25: log32 = {log30[WIDTH-3], log30[WIDTH-5:0], 3'b0};
        5'd26: log32 = {log30[WIDTH-3], log30[WIDTH-6:0], 4'b0};
        5'd27: log32 = {log30[WIDTH-3], log30[WIDTH-7:0], 5'b0};
        5'd28: log32 = {log30[WIDTH-3], log30[WIDTH-8:0], 6'b0};
        5'd29: log32 = {log30[WIDTH-3], log30[WIDTH-9:0], 7'b0};
        5'd30: log32 = {log30[WIDTH-3], log30[WIDTH-10:0], 8'b0};
        5'd31: log32 = {log30[WIDTH-3], log30[WIDTH-11:0], 9'b0};
        default: log32 = {{2{log30[WIDTH-3]}}, log30[WIDTH-3:0]};
    endcase
end
endmodule


module Tran_Q822_to_float(
    input [29:0] log30,
    output reg [31:0] log32
);
    wire sign;
    wire [29:0] abs_x30;
    ABS0_Tran_Q822_to_float abs0_Tran_Q822_to_float0(
        .x_in(log30),
        .sign(sign),
        .abs_x(abs_x30)
    );
    wire [4:0] k;
    LOD1_Tran_Q822_to_float_24 lod1_Tran_Q822_to_float_24_0(
        .in(abs_x30),
        .msb_pos(k)  
    );

    always @(*) begin
        case(k)
            5'd2: log32 = {sign, 8'b10000110, 23'b00000000000000000000000};
            5'd3: log32 = {sign, 8'b10000101, abs_x30[27:5]};
            5'd4: log32 = {sign, 8'b10000100, abs_x30[26:4]};
            5'd5: log32 = {sign, 8'b10000011, abs_x30[25:3]};
            5'd6: log32 = {sign, 8'b10000010, abs_x30[24:2]};
            5'd7: log32 = {sign, 8'b10000001, abs_x30[23:1]};
            5'd8: log32 = {sign, 8'b10000000, abs_x30[22:0]};
            5'd9: log32 = {sign, 8'b01111111, abs_x30[21:0], 1'b0};
            5'd10: log32 = {sign, 8'b01111110, abs_x30[20:0], 2'b0};
            5'd11: log32 = {sign, 8'b01111101, abs_x30[19:0], 3'b0};
            5'd12: log32 = {sign, 8'b01111100, abs_x30[18:0], 4'b0};
            5'd13: log32 = {sign, 8'b01111011, abs_x30[17:0], 5'b0};
            5'd14: log32 = {sign, 8'b01111010, abs_x30[16:0], 6'b0};
            5'd15: log32 = {sign, 8'b01111001, abs_x30[15:0], 7'b0};
            5'd16: log32 = {sign, 8'b01111000, abs_x30[14:0], 8'b0};
            5'd17: log32 = {sign, 8'b01110111, abs_x30[13:0], 9'b0};
            5'd18: log32 = {sign, 8'b01110110, abs_x30[12:0], 10'b0};
            5'd19: log32 = {sign, 8'b01110101, abs_x30[11:0], 11'b0};
            5'd20: log32 = {sign, 8'b01110100, abs_x30[10:0], 12'b0};
            5'd21: log32 = {sign, 8'b01110011, abs_x30[9:0], 13'b0};
            5'd22: log32 = {sign, 8'b01110010, abs_x30[8:0], 14'b0};
            5'd23: log32 = {sign, 8'b01110001, abs_x30[7:0], 15'b0};
            5'd24: log32 = {sign, 8'b01110000, abs_x30[6:0], 16'b0};
            5'd25: log32 = {sign, 8'b01101111, abs_x30[5:0], 17'b0};
            5'd26: log32 = {sign, 8'b01101110, abs_x30[4:0], 18'b0};
            5'd27: log32 = {sign, 8'b01101101, abs_x30[3:0], 19'b0};
            5'd28: log32 = {sign, 8'b01101100, abs_x30[2:0], 20'b0};
            5'd29: log32 = {sign, 8'b01101011, abs_x30[1:0], 21'b0};
            5'd30: log32 = {sign, 8'b01101010, abs_x30[0], 22'b0};
            5'd31: log32 = {sign, 8'b01101001, 23'b0};
            default: log32 = {1'b0, 8'b0, 23'b0};
        endcase
    end


endmodule


module ABS0_Tran_Q822_to_float(
    input [29:0] x_in,        // 输入 x 为 32 位定点数
    output reg sign,
    output reg [29:0] abs_x // 输出 abs_x 为 x 的绝对值
);

    always @(*) begin
        if (x_in[29] == 1'b1) begin      // 如果符号位为1，表示负数
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

module leading_one_detector_4bit_Tran_Q816_to_float (
    input [3:0] in,      // 4-bit input
    output reg [1:0] msb_pos,  // MSB position within the 4 bits, 2-bit output (0-3)
    output reg signal
);
    always @(*) begin
        if(in == 4'b0000)
            signal = 1'b0;
        else
            signal = 1'b1;
        casez (in)
            4'b1???: msb_pos = 2'd0;
            4'b01??: msb_pos = 2'd1;
            4'b001?: msb_pos = 2'd2;
            4'b0001: msb_pos = 2'd3;
            default: msb_pos = 2'd0;  // Default case (this should never happen)
        endcase
    end
endmodule


module LOD1_Tran_Q822_to_float_24 (
    input [29:0] in,     // 32-bit input
    output reg [4:0] msb_pos  // MSB position, 5-bit output (0-31)
);

    wire [1:0] msb_group_1, msb_group_2, msb_group_3, msb_group_4;
    wire [1:0] msb_group_5, msb_group_6, msb_group_7, msb_group_8;
    wire signal1, signal2, signal3, signal4;
    wire  signal5, signal6, signal7, signal8;

    // Instantiate 8 4-bit Leading-One Detectors for each group
    leading_one_detector_4bit_Tran_Q816_to_float group1 (.in({2'b00, in[29:28]}), .msb_pos(msb_group_1), .signal(signal1));
    leading_one_detector_4bit_Tran_Q816_to_float group2 (.in(in[27:24]), .msb_pos(msb_group_2), .signal(signal2));
    leading_one_detector_4bit_Tran_Q816_to_float group3 (.in(in[23:20]), .msb_pos(msb_group_3), .signal(signal3));
    leading_one_detector_4bit_Tran_Q816_to_float group4 (.in(in[19:16]), .msb_pos(msb_group_4), .signal(signal4));
    leading_one_detector_4bit_Tran_Q816_to_float group5 (.in(in[15:12]), .msb_pos(msb_group_5), .signal(signal5));
    leading_one_detector_4bit_Tran_Q816_to_float group6 (.in(in[11:8]), .msb_pos(msb_group_6), .signal(signal6));
    leading_one_detector_4bit_Tran_Q816_to_float group7 (.in(in[7:4]), .msb_pos(msb_group_7), .signal(signal7));
    leading_one_detector_4bit_Tran_Q816_to_float group8 (.in(in[3:0]), .msb_pos(msb_group_8), .signal(signal8));
    always @(*) begin
        // Combine results from each group to determine the final MSB position
        //@zou 若in全为0，则会使得ms_pos为0，对于浮点是不正确的，会输出最小的浮点数
        if (signal1) msb_pos = {3'b000, msb_group_1};  // Position in the first group (0-3)
        else if (signal2) msb_pos = {3'b001, msb_group_2};
        else if (signal3) msb_pos = {3'b010, msb_group_3};
        else if (signal4) msb_pos = {3'b011, msb_group_4};
        else if (signal5) msb_pos = {3'b100, msb_group_5};
        else if (signal6) msb_pos = {3'b101, msb_group_6};
        else if (signal7) msb_pos = {3'b110, msb_group_7};
        else if (signal8) msb_pos = {3'b111, msb_group_8};
        else msb_pos = 5'b00000;  // Default case (this should never happen)
    end

endmodule


module CPA_ALOGC (
    input [30:0] a,
    input [30:0] b,
    input Div,
    output [30:0] sum
);
    wire [30:0] rev_b;
    assign rev_b = ~b;
    wire [30:0] operator_b;
    assign operator_b = (Div == 1'b0) ? b : rev_b;
    assign sum = a + operator_b + Div;  // Simple binary addition
endmodule
