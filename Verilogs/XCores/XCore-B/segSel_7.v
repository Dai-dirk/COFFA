`timescale 1ns / 1ps
//@yuan: adding support for float-point number
//TODO: adding support for NaN, Inf
module segSel_7 #(parameter WIDTH32 = 32  
)(
    input [WIDTH32 - 1:0] x_in,
    input is_fp,
    input [6*WIDTH32 - 1:0] break_points_in,
    output [5:0] log2_out
);
    reg [WIDTH32 - 1:0] break_point [5:0];
    reg break_point_sign [5:0];
    integer i;
    always@(*)begin
        for(i = 0; i < 6; i = i + 1)begin
            if(is_fp)begin
                break_point[i] <= {1'b0, break_points_in[(i+1)*WIDTH32 - 2 -: (WIDTH32 - 1)]} ; //@yuan: for float-point, we only need the significand/fraction field
            end 
            else begin
                break_point[i] <= break_points_in[(i+1)*WIDTH32 - 1 -: WIDTH32];
            end 
            break_point_sign[i] <= break_points_in[(i+1)*WIDTH32 - 1];
        end
    end

    wire [WIDTH32 - 1:0] x_op;
    assign x_op = is_fp ? {1'b0, x_in[WIDTH32 - 2 : 0]} : x_in;
    
    genvar j;
    wire [5:0] is_both_zero;
    wire [5:0] is_same_sign;
    generate
        for(j = 0; j < 6; j = j + 1)begin: break_is_zero
            assign is_both_zero[j] = (~|break_point[j]) & (~|x_op);
            assign is_same_sign[j] = break_point_sign[j] == x_in[WIDTH32 - 1];
        end    
    endgenerate

    wire [5:0] cmp_result;
    generate
        for(j = 0; j < 6; j = j + 1)begin: cmp//@yuan: reusing the comparator
            assign cmp_result[j] = $signed(x_op) > $signed(break_point[j]) ? 1'b1 : 1'b0 ;
        end    
    endgenerate

    
    //if the break point is 0, the segment should be selected the right side
    generate
        for(j = 0; j < 6; j = j + 1)begin: result
            assign log2_out[j] = is_fp ?  is_both_zero[j] ? 1'b1 : is_same_sign[j] ? x_in[WIDTH32 - 1] ^ cmp_result[j] : !x_in[WIDTH32 - 1] : cmp_result[j];
        end    
    endgenerate

    // assign log2_out[0] = is_fp ?  is_both_zero[0] ? 1'b1 : is_same_sign[0] ? x_in[WIDTH32 - 1] ^ cmp_result[0] : !x_in[WIDTH32 - 1] : cmp_result[0];
    // assign log2_out[1] = is_fp ?  is_both_zero[1] ? 1'b1 : is_same_sign[1] ? x_in[WIDTH32 - 1] ^ cmp_result[1] : !x_in[WIDTH32 - 1] : cmp_result[1];
    // assign log2_out[2] = is_fp ?  is_both_zero[2] ? 1'b1 : is_same_sign[2] ? x_in[WIDTH32 - 1] ^ cmp_result[2] : !x_in[WIDTH32 - 1] : cmp_result[2];
    // assign log2_out[3] = is_fp ?  is_both_zero[3] ? 1'b1 : is_same_sign[3] ? x_in[WIDTH32 - 1] ^ cmp_result[3] : !x_in[WIDTH32 - 1] : cmp_result[3];
    // assign log2_out[4] = is_fp ?  is_both_zero[4] ? 1'b1 : is_same_sign[4] ? x_in[WIDTH32 - 1] ^ cmp_result[4] : !x_in[WIDTH32 - 1] : cmp_result[4];
endmodule 
