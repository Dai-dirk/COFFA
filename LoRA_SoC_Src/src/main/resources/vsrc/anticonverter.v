module anticonverter#(parameter WIDTH = 32,
                          parameter WIDTH32 = 32  
)(
    input [WIDTH-1:0] x,
    input float_flag,
    input Tri_overflow_one,
    input [4:0] n,
    output [WIDTH32-1:0] x_output
);

    wire [22:0] f_e2_out;
    APP_22_20_10_4_32_anticonv4 app_anti(
        .f(x[21:0]),
        .f_e2_out(f_e2_out)
    );
    wire [7:0] shift;
    wire [WIDTH32-1:0] result;
    //相当于f_e2_out还是24位，只是后面位全部为0
    // assign  shift = {3'b0, n} + 8'b11101001 + x[29:22];
    assign  shift = {3'b0, n} + x[29:22];
    // assign  shift = x[21:16] + 1'b1;
    wire [22:0] app_result;
    assign app_result = (f_e2_out[22]==1'b1) ? f_e2_out : {1'b1, 22'b0};
    new_Shift_operation32 shift_op(
        .shift(shift),
        .e({app_result, 1'b0}),  // 24-bit signed input signal
        .result(result)
    );
    wire [7:0] float_e_temp;
    assign float_e_temp = 8'b01111111 + x[29:22];
    wire [7:0] float_e;
    assign float_e = (float_e_temp == 8'b11111111) ? 8'b00000000 : float_e_temp;
    wire [WIDTH32-1:0] final_result_fix;
    assign  final_result_fix = (x[30]== 1'b1) ? (~result + 1'b1) : result;
    wire [WIDTH32-1:0] final_result_float;
    assign final_result_float = {x[30], float_e, app_result[21:0], 1'b0};
    wire [WIDTH32-1:0] final_result;
    assign final_result = (float_flag == 1'b1) ? final_result_float: final_result_fix;
    assign  x_output = (x[31] == 1'b1 || Tri_overflow_one == 1'b1) ? 32'd0 : final_result;
endmodule


module  APP_22_20_10_4_32_anticonv4(
    input [21:0] f,               // 22-bit input
    output [22:0] f_e2_out     // 23-bit output 
);
    reg [5:0] segment;
    reg [20:0] b;
    always @(*) begin
        case(f[21:12])
            10'b0000000000: begin segment = 6'd0; b = 21'b011111111111111111100; end
            10'b0000000001: begin segment = 6'd0; b = 21'b011111111111111110010; end
            10'b0000000010: begin segment = 6'd0; b = 21'b011111111111111101001; end
            10'b0000000011: begin segment = 6'd0; b = 21'b011111111111111100000; end
            10'b0000000100: begin segment = 6'd0; b = 21'b011111111111111011000; end
            10'b0000000101: begin segment = 6'd0; b = 21'b011111111111111010000; end
            10'b0000000110: begin segment = 6'd0; b = 21'b011111111111111001001; end
            10'b0000000111: begin segment = 6'd0; b = 21'b011111111111111000010; end
            10'b0000001000: begin segment = 6'd0; b = 21'b011111111111110111100; end
            10'b0000001001: begin segment = 6'd0; b = 21'b011111111111110110110; end
            10'b0000001010: begin segment = 6'd0; b = 21'b011111111111110110000; end
            10'b0000001011: begin segment = 6'd0; b = 21'b011111111111110101100; end
            10'b0000001100: begin segment = 6'd0; b = 21'b011111111111110100111; end
            10'b0000001101: begin segment = 6'd0; b = 21'b011111111111110100011; end
            10'b0000001110: begin segment = 6'd0; b = 21'b011111111111110100000; end
            10'b0000001111: begin segment = 6'd0; b = 21'b011111111111110011101; end
            10'b0000010000: begin segment = 6'd0; b = 21'b011111111111110011010; end
            10'b0000010001: begin segment = 6'd0; b = 21'b011111111111110011000; end
            10'b0000010010: begin segment = 6'd0; b = 21'b011111111111110010111; end
            10'b0000010011: begin segment = 6'd0; b = 21'b011111111111110010110; end
            10'b0000010100: begin segment = 6'd0; b = 21'b011111111111110010101; end
            10'b0000010101: begin segment = 6'd0; b = 21'b011111111111110010101; end
            10'b0000010110: begin segment = 6'd0; b = 21'b011111111111110010110; end
            10'b0000010111: begin segment = 6'd0; b = 21'b011111111111110010110; end
            10'b0000011000: begin segment = 6'd0; b = 21'b011111111111110011000; end
            10'b0000011001: begin segment = 6'd0; b = 21'b011111111111110011010; end
            10'b0000011010: begin segment = 6'd0; b = 21'b011111111111110011100; end
            10'b0000011011: begin segment = 6'd0; b = 21'b011111111111110011111; end
            10'b0000011100: begin segment = 6'd0; b = 21'b011111111111110100010; end
            10'b0000011101: begin segment = 6'd0; b = 21'b011111111111110100110; end
            10'b0000011110: begin segment = 6'd0; b = 21'b011111111111110101011; end
            10'b0000011111: begin segment = 6'd0; b = 21'b011111111111110101111; end
            10'b0000100000: begin segment = 6'd0; b = 21'b011111111111110110101; end
            10'b0000100001: begin segment = 6'd0; b = 21'b011111111111110111011; end
            10'b0000100010: begin segment = 6'd0; b = 21'b011111111111111000001; end
            10'b0000100011: begin segment = 6'd1; b = 21'b011111111110110000111; end
            10'b0000100100: begin segment = 6'd1; b = 21'b011111111110101111101; end
            10'b0000100101: begin segment = 6'd1; b = 21'b011111111110101110101; end
            10'b0000100110: begin segment = 6'd1; b = 21'b011111111110101101110; end
            10'b0000100111: begin segment = 6'd1; b = 21'b011111111110101100110; end
            10'b0000101000: begin segment = 6'd1; b = 21'b011111111110101011111; end
            10'b0000101001: begin segment = 6'd1; b = 21'b011111111110101011000; end
            10'b0000101010: begin segment = 6'd1; b = 21'b011111111110101010011; end
            10'b0000101011: begin segment = 6'd1; b = 21'b011111111110101001101; end
            10'b0000101100: begin segment = 6'd1; b = 21'b011111111110101001000; end
            10'b0000101101: begin segment = 6'd1; b = 21'b011111111110101000011; end
            10'b0000101110: begin segment = 6'd1; b = 21'b011111111110101000000; end
            10'b0000101111: begin segment = 6'd1; b = 21'b011111111110100111100; end
            10'b0000110000: begin segment = 6'd1; b = 21'b011111111110100111000; end
            10'b0000110001: begin segment = 6'd1; b = 21'b011111111110100110110; end
            10'b0000110010: begin segment = 6'd1; b = 21'b011111111110100110100; end
            10'b0000110011: begin segment = 6'd1; b = 21'b011111111110100110011; end
            10'b0000110100: begin segment = 6'd1; b = 21'b011111111110100110001; end
            10'b0000110101: begin segment = 6'd1; b = 21'b011111111110100110001; end
            10'b0000110110: begin segment = 6'd1; b = 21'b011111111110100110001; end
            10'b0000110111: begin segment = 6'd1; b = 21'b011111111110100110010; end
            10'b0000111000: begin segment = 6'd1; b = 21'b011111111110100110010; end
            10'b0000111001: begin segment = 6'd1; b = 21'b011111111110100110100; end
            10'b0000111010: begin segment = 6'd1; b = 21'b011111111110100110110; end
            10'b0000111011: begin segment = 6'd1; b = 21'b011111111110100111001; end
            10'b0000111100: begin segment = 6'd1; b = 21'b011111111110100111011; end
            10'b0000111101: begin segment = 6'd1; b = 21'b011111111110100111111; end
            10'b0000111110: begin segment = 6'd1; b = 21'b011111111110101000011; end
            10'b0000111111: begin segment = 6'd1; b = 21'b011111111110101001000; end
            10'b0001000000: begin segment = 6'd1; b = 21'b011111111110101001100; end
            10'b0001000001: begin segment = 6'd1; b = 21'b011111111110101010001; end
            10'b0001000010: begin segment = 6'd1; b = 21'b011111111110101011000; end
            10'b0001000011: begin segment = 6'd1; b = 21'b011111111110101011110; end
            10'b0001000100: begin segment = 6'd1; b = 21'b011111111110101100101; end
            10'b0001000101: begin segment = 6'd1; b = 21'b011111111110101101100; end
            10'b0001000110: begin segment = 6'd1; b = 21'b011111111110101110101; end
            10'b0001000111: begin segment = 6'd1; b = 21'b011111111110101111101; end
            10'b0001001000: begin segment = 6'd1; b = 21'b011111111110110000110; end
            10'b0001001001: begin segment = 6'd2; b = 21'b011111111011111100100; end
            10'b0001001010: begin segment = 6'd2; b = 21'b011111111011111011010; end
            10'b0001001011: begin segment = 6'd2; b = 21'b011111111011111010001; end
            10'b0001001100: begin segment = 6'd2; b = 21'b011111111011111001000; end
            10'b0001001101: begin segment = 6'd2; b = 21'b011111111011111000000; end
            10'b0001001110: begin segment = 6'd2; b = 21'b011111111011110111000; end
            10'b0001001111: begin segment = 6'd2; b = 21'b011111111011110110001; end
            10'b0001010000: begin segment = 6'd2; b = 21'b011111111011110101010; end
            10'b0001010001: begin segment = 6'd2; b = 21'b011111111011110100100; end
            10'b0001010010: begin segment = 6'd2; b = 21'b011111111011110011110; end
            10'b0001010011: begin segment = 6'd2; b = 21'b011111111011110011001; end
            10'b0001010100: begin segment = 6'd2; b = 21'b011111111011110010100; end
            10'b0001010101: begin segment = 6'd2; b = 21'b011111111011110010000; end
            10'b0001010110: begin segment = 6'd2; b = 21'b011111111011110001100; end
            10'b0001010111: begin segment = 6'd2; b = 21'b011111111011110001001; end
            10'b0001011000: begin segment = 6'd2; b = 21'b011111111011110000111; end
            10'b0001011001: begin segment = 6'd2; b = 21'b011111111011110000100; end
            10'b0001011010: begin segment = 6'd2; b = 21'b011111111011110000011; end
            10'b0001011011: begin segment = 6'd2; b = 21'b011111111011110000010; end
            10'b0001011100: begin segment = 6'd2; b = 21'b011111111011110000001; end
            10'b0001011101: begin segment = 6'd2; b = 21'b011111111011110000001; end
            10'b0001011110: begin segment = 6'd2; b = 21'b011111111011110000001; end
            10'b0001011111: begin segment = 6'd2; b = 21'b011111111011110000010; end
            10'b0001100000: begin segment = 6'd2; b = 21'b011111111011110000100; end
            10'b0001100001: begin segment = 6'd2; b = 21'b011111111011110000110; end
            10'b0001100010: begin segment = 6'd2; b = 21'b011111111011110001000; end
            10'b0001100011: begin segment = 6'd2; b = 21'b011111111011110001011; end
            10'b0001100100: begin segment = 6'd2; b = 21'b011111111011110001111; end
            10'b0001100101: begin segment = 6'd2; b = 21'b011111111011110010011; end
            10'b0001100110: begin segment = 6'd2; b = 21'b011111111011110010111; end
            10'b0001100111: begin segment = 6'd2; b = 21'b011111111011110011100; end
            10'b0001101000: begin segment = 6'd2; b = 21'b011111111011110100010; end
            10'b0001101001: begin segment = 6'd2; b = 21'b011111111011110101000; end
            10'b0001101010: begin segment = 6'd2; b = 21'b011111111011110101110; end
            10'b0001101011: begin segment = 6'd2; b = 21'b011111111011110110110; end
            10'b0001101100: begin segment = 6'd2; b = 21'b011111111011110111101; end
            10'b0001101101: begin segment = 6'd2; b = 21'b011111111011111000101; end
            10'b0001101110: begin segment = 6'd3; b = 21'b011111111000000001001; end
            10'b0001101111: begin segment = 6'd3; b = 21'b011111111000000000000; end
            10'b0001110000: begin segment = 6'd3; b = 21'b011111110111111111000; end
            10'b0001110001: begin segment = 6'd3; b = 21'b011111110111111110000; end
            10'b0001110010: begin segment = 6'd3; b = 21'b011111110111111101001; end
            10'b0001110011: begin segment = 6'd3; b = 21'b011111110111111100010; end
            10'b0001110100: begin segment = 6'd3; b = 21'b011111110111111011100; end
            10'b0001110101: begin segment = 6'd3; b = 21'b011111110111111010110; end
            10'b0001110110: begin segment = 6'd3; b = 21'b011111110111111010001; end
            10'b0001110111: begin segment = 6'd3; b = 21'b011111110111111001100; end
            10'b0001111000: begin segment = 6'd3; b = 21'b011111110111111001000; end
            10'b0001111001: begin segment = 6'd3; b = 21'b011111110111111000100; end
            10'b0001111010: begin segment = 6'd3; b = 21'b011111110111111000001; end
            10'b0001111011: begin segment = 6'd3; b = 21'b011111110111110111111; end
            10'b0001111100: begin segment = 6'd3; b = 21'b011111110111110111101; end
            10'b0001111101: begin segment = 6'd3; b = 21'b011111110111110111011; end
            10'b0001111110: begin segment = 6'd3; b = 21'b011111110111110111010; end
            10'b0001111111: begin segment = 6'd3; b = 21'b011111110111110111001; end
            10'b0010000000: begin segment = 6'd3; b = 21'b011111110111110111001; end
            10'b0010000001: begin segment = 6'd3; b = 21'b011111110111110111010; end
            10'b0010000010: begin segment = 6'd3; b = 21'b011111110111110111011; end
            10'b0010000011: begin segment = 6'd3; b = 21'b011111110111110111101; end
            10'b0010000100: begin segment = 6'd3; b = 21'b011111110111110111111; end
            10'b0010000101: begin segment = 6'd3; b = 21'b011111110111111000010; end
            10'b0010000110: begin segment = 6'd3; b = 21'b011111110111111000101; end
            10'b0010000111: begin segment = 6'd3; b = 21'b011111110111111001000; end
            10'b0010001000: begin segment = 6'd3; b = 21'b011111110111111001101; end
            10'b0010001001: begin segment = 6'd3; b = 21'b011111110111111010001; end
            10'b0010001010: begin segment = 6'd3; b = 21'b011111110111111010111; end
            10'b0010001011: begin segment = 6'd3; b = 21'b011111110111111011100; end
            10'b0010001100: begin segment = 6'd3; b = 21'b011111110111111100011; end
            10'b0010001101: begin segment = 6'd3; b = 21'b011111110111111101010; end
            10'b0010001110: begin segment = 6'd3; b = 21'b011111110111111110001; end
            10'b0010001111: begin segment = 6'd3; b = 21'b011111110111111111001; end
            10'b0010010000: begin segment = 6'd3; b = 21'b011111111000000000001; end
            10'b0010010001: begin segment = 6'd3; b = 21'b011111111000000001010; end
            10'b0010010010: begin segment = 6'd3; b = 21'b011111111000000010100; end
            10'b0010010011: begin segment = 6'd4; b = 21'b011111110010110111111; end
            10'b0010010100: begin segment = 6'd4; b = 21'b011111110010110111000; end
            10'b0010010101: begin segment = 6'd4; b = 21'b011111110010110110001; end
            10'b0010010110: begin segment = 6'd4; b = 21'b011111110010110101010; end
            10'b0010010111: begin segment = 6'd4; b = 21'b011111110010110100100; end
            10'b0010011000: begin segment = 6'd4; b = 21'b011111110010110011111; end
            10'b0010011001: begin segment = 6'd4; b = 21'b011111110010110011010; end
            10'b0010011010: begin segment = 6'd4; b = 21'b011111110010110010110; end
            10'b0010011011: begin segment = 6'd4; b = 21'b011111110010110010010; end
            10'b0010011100: begin segment = 6'd4; b = 21'b011111110010110001111; end
            10'b0010011101: begin segment = 6'd4; b = 21'b011111110010110001101; end
            10'b0010011110: begin segment = 6'd4; b = 21'b011111110010110001011; end
            10'b0010011111: begin segment = 6'd4; b = 21'b011111110010110001001; end
            10'b0010100000: begin segment = 6'd4; b = 21'b011111110010110001000; end
            10'b0010100001: begin segment = 6'd4; b = 21'b011111110010110000111; end
            10'b0010100010: begin segment = 6'd4; b = 21'b011111110010110000111; end
            10'b0010100011: begin segment = 6'd4; b = 21'b011111110010110001000; end
            10'b0010100100: begin segment = 6'd4; b = 21'b011111110010110001001; end
            10'b0010100101: begin segment = 6'd4; b = 21'b011111110010110001011; end
            10'b0010100110: begin segment = 6'd4; b = 21'b011111110010110001101; end
            10'b0010100111: begin segment = 6'd4; b = 21'b011111110010110010000; end
            10'b0010101000: begin segment = 6'd4; b = 21'b011111110010110010011; end
            10'b0010101001: begin segment = 6'd4; b = 21'b011111110010110010111; end
            10'b0010101010: begin segment = 6'd4; b = 21'b011111110010110011011; end
            10'b0010101011: begin segment = 6'd4; b = 21'b011111110010110100000; end
            10'b0010101100: begin segment = 6'd4; b = 21'b011111110010110100101; end
            10'b0010101101: begin segment = 6'd4; b = 21'b011111110010110101011; end
            10'b0010101110: begin segment = 6'd4; b = 21'b011111110010110110010; end
            10'b0010101111: begin segment = 6'd4; b = 21'b011111110010110111001; end
            10'b0010110000: begin segment = 6'd4; b = 21'b011111110010111000001; end
            10'b0010110001: begin segment = 6'd4; b = 21'b011111110010111001001; end
            10'b0010110010: begin segment = 6'd4; b = 21'b011111110010111010001; end
            10'b0010110011: begin segment = 6'd4; b = 21'b011111110010111011011; end
            10'b0010110100: begin segment = 6'd4; b = 21'b011111110010111100100; end
            10'b0010110101: begin segment = 6'd4; b = 21'b011111110010111101111; end
            10'b0010110110: begin segment = 6'd4; b = 21'b011111110010111111010; end
            10'b0010110111: begin segment = 6'd5; b = 21'b011111101010011010001; end
            10'b0010111000: begin segment = 6'd5; b = 21'b011111101010011000101; end
            10'b0010111001: begin segment = 6'd5; b = 21'b011111101010010111001; end
            10'b0010111010: begin segment = 6'd5; b = 21'b011111101010010101110; end
            10'b0010111011: begin segment = 6'd5; b = 21'b011111101010010100100; end
            10'b0010111100: begin segment = 6'd5; b = 21'b011111101010010011010; end
            10'b0010111101: begin segment = 6'd5; b = 21'b011111101010010010001; end
            10'b0010111110: begin segment = 6'd5; b = 21'b011111101010010001000; end
            10'b0010111111: begin segment = 6'd5; b = 21'b011111101010010000000; end
            10'b0011000000: begin segment = 6'd5; b = 21'b011111101010001111000; end
            10'b0011000001: begin segment = 6'd5; b = 21'b011111101010001110001; end
            10'b0011000010: begin segment = 6'd5; b = 21'b011111101010001101010; end
            10'b0011000011: begin segment = 6'd5; b = 21'b011111101010001100100; end
            10'b0011000100: begin segment = 6'd5; b = 21'b011111101010001011110; end
            10'b0011000101: begin segment = 6'd5; b = 21'b011111101010001011010; end
            10'b0011000110: begin segment = 6'd5; b = 21'b011111101010001010101; end
            10'b0011000111: begin segment = 6'd5; b = 21'b011111101010001010001; end
            10'b0011001000: begin segment = 6'd5; b = 21'b011111101010001001110; end
            10'b0011001001: begin segment = 6'd5; b = 21'b011111101010001001011; end
            10'b0011001010: begin segment = 6'd5; b = 21'b011111101010001001001; end
            10'b0011001011: begin segment = 6'd5; b = 21'b011111101010001000111; end
            10'b0011001100: begin segment = 6'd5; b = 21'b011111101010001000110; end
            10'b0011001101: begin segment = 6'd5; b = 21'b011111101010001000110; end
            10'b0011001110: begin segment = 6'd5; b = 21'b011111101010001000110; end
            10'b0011001111: begin segment = 6'd5; b = 21'b011111101010001000110; end
            10'b0011010000: begin segment = 6'd5; b = 21'b011111101010001000111; end
            10'b0011010001: begin segment = 6'd5; b = 21'b011111101010001001001; end
            10'b0011010010: begin segment = 6'd5; b = 21'b011111101010001001011; end
            10'b0011010011: begin segment = 6'd5; b = 21'b011111101010001001110; end
            10'b0011010100: begin segment = 6'd5; b = 21'b011111101010001010001; end
            10'b0011010101: begin segment = 6'd5; b = 21'b011111101010001010101; end
            10'b0011010110: begin segment = 6'd5; b = 21'b011111101010001011001; end
            10'b0011010111: begin segment = 6'd5; b = 21'b011111101010001011110; end
            10'b0011011000: begin segment = 6'd5; b = 21'b011111101010001100100; end
            10'b0011011001: begin segment = 6'd5; b = 21'b011111101010001101010; end
            10'b0011011010: begin segment = 6'd5; b = 21'b011111101010001110001; end
            10'b0011011011: begin segment = 6'd6; b = 21'b011111100010111100100; end
            10'b0011011100: begin segment = 6'd6; b = 21'b011111100010111011011; end
            10'b0011011101: begin segment = 6'd6; b = 21'b011111100010111010010; end
            10'b0011011110: begin segment = 6'd6; b = 21'b011111100010111001010; end
            10'b0011011111: begin segment = 6'd6; b = 21'b011111100010111000010; end
            10'b0011100000: begin segment = 6'd6; b = 21'b011111100010110111011; end
            10'b0011100001: begin segment = 6'd6; b = 21'b011111100010110110101; end
            10'b0011100010: begin segment = 6'd6; b = 21'b011111100010110101111; end
            10'b0011100011: begin segment = 6'd6; b = 21'b011111100010110101010; end
            10'b0011100100: begin segment = 6'd6; b = 21'b011111100010110100101; end
            10'b0011100101: begin segment = 6'd6; b = 21'b011111100010110100001; end
            10'b0011100110: begin segment = 6'd6; b = 21'b011111100010110011101; end
            10'b0011100111: begin segment = 6'd6; b = 21'b011111100010110011010; end
            10'b0011101000: begin segment = 6'd6; b = 21'b011111100010110011000; end
            10'b0011101001: begin segment = 6'd6; b = 21'b011111100010110010110; end
            10'b0011101010: begin segment = 6'd6; b = 21'b011111100010110010100; end
            10'b0011101011: begin segment = 6'd6; b = 21'b011111100010110010100; end
            10'b0011101100: begin segment = 6'd6; b = 21'b011111100010110010011; end
            10'b0011101101: begin segment = 6'd6; b = 21'b011111100010110010100; end
            10'b0011101110: begin segment = 6'd6; b = 21'b011111100010110010100; end
            10'b0011101111: begin segment = 6'd6; b = 21'b011111100010110010110; end
            10'b0011110000: begin segment = 6'd6; b = 21'b011111100010110011000; end
            10'b0011110001: begin segment = 6'd6; b = 21'b011111100010110011010; end
            10'b0011110010: begin segment = 6'd6; b = 21'b011111100010110011110; end
            10'b0011110011: begin segment = 6'd6; b = 21'b011111100010110100001; end
            10'b0011110100: begin segment = 6'd6; b = 21'b011111100010110100110; end
            10'b0011110101: begin segment = 6'd6; b = 21'b011111100010110101010; end
            10'b0011110110: begin segment = 6'd6; b = 21'b011111100010110110000; end
            10'b0011110111: begin segment = 6'd6; b = 21'b011111100010110110110; end
            10'b0011111000: begin segment = 6'd6; b = 21'b011111100010110111100; end
            10'b0011111001: begin segment = 6'd6; b = 21'b011111100010111000011; end
            10'b0011111010: begin segment = 6'd6; b = 21'b011111100010111001011; end
            10'b0011111011: begin segment = 6'd6; b = 21'b011111100010111010011; end
            10'b0011111100: begin segment = 6'd6; b = 21'b011111100010111011100; end
            10'b0011111101: begin segment = 6'd6; b = 21'b011111100010111100101; end
            10'b0011111110: begin segment = 6'd7; b = 21'b011111011011100000110; end
            10'b0011111111: begin segment = 6'd7; b = 21'b011111011011100000001; end
            10'b0100000000: begin segment = 6'd7; b = 21'b011111011011011111101; end
            10'b0100000001: begin segment = 6'd7; b = 21'b011111011011011111010; end
            10'b0100000010: begin segment = 6'd7; b = 21'b011111011011011110111; end
            10'b0100000011: begin segment = 6'd7; b = 21'b011111011011011110101; end
            10'b0100000100: begin segment = 6'd7; b = 21'b011111011011011110011; end
            10'b0100000101: begin segment = 6'd7; b = 21'b011111011011011110010; end
            10'b0100000110: begin segment = 6'd7; b = 21'b011111011011011110010; end
            10'b0100000111: begin segment = 6'd7; b = 21'b011111011011011110010; end
            10'b0100001000: begin segment = 6'd7; b = 21'b011111011011011110011; end
            10'b0100001001: begin segment = 6'd7; b = 21'b011111011011011110100; end
            10'b0100001010: begin segment = 6'd7; b = 21'b011111011011011110110; end
            10'b0100001011: begin segment = 6'd7; b = 21'b011111011011011111000; end
            10'b0100001100: begin segment = 6'd7; b = 21'b011111011011011111011; end
            10'b0100001101: begin segment = 6'd7; b = 21'b011111011011011111111; end
            10'b0100001110: begin segment = 6'd7; b = 21'b011111011011100000011; end
            10'b0100001111: begin segment = 6'd7; b = 21'b011111011011100000111; end
            10'b0100010000: begin segment = 6'd7; b = 21'b011111011011100001101; end
            10'b0100010001: begin segment = 6'd7; b = 21'b011111011011100010010; end
            10'b0100010010: begin segment = 6'd7; b = 21'b011111011011100011001; end
            10'b0100010011: begin segment = 6'd7; b = 21'b011111011011100100000; end
            10'b0100010100: begin segment = 6'd7; b = 21'b011111011011100100111; end
            10'b0100010101: begin segment = 6'd7; b = 21'b011111011011100110000; end
            10'b0100010110: begin segment = 6'd7; b = 21'b011111011011100111000; end
            10'b0100010111: begin segment = 6'd7; b = 21'b011111011011101000010; end
            10'b0100011000: begin segment = 6'd7; b = 21'b011111011011101001100; end
            10'b0100011001: begin segment = 6'd7; b = 21'b011111011011101010110; end
            10'b0100011010: begin segment = 6'd7; b = 21'b011111011011101100001; end
            10'b0100011011: begin segment = 6'd7; b = 21'b011111011011101101101; end
            10'b0100011100: begin segment = 6'd7; b = 21'b011111011011101111001; end
            10'b0100011101: begin segment = 6'd7; b = 21'b011111011011110000110; end
            10'b0100011110: begin segment = 6'd7; b = 21'b011111011011110010011; end
            10'b0100011111: begin segment = 6'd7; b = 21'b011111011011110100001; end
            10'b0100100000: begin segment = 6'd7; b = 21'b011111011011110110000; end
            10'b0100100001: begin segment = 6'd8; b = 21'b011111001110010011011; end
            10'b0100100010: begin segment = 6'd8; b = 21'b011111001110010010011; end
            10'b0100100011: begin segment = 6'd8; b = 21'b011111001110010001011; end
            10'b0100100100: begin segment = 6'd8; b = 21'b011111001110010000100; end
            10'b0100100101: begin segment = 6'd8; b = 21'b011111001110001111101; end
            10'b0100100110: begin segment = 6'd8; b = 21'b011111001110001110111; end
            10'b0100100111: begin segment = 6'd8; b = 21'b011111001110001110010; end
            10'b0100101000: begin segment = 6'd8; b = 21'b011111001110001101101; end
            10'b0100101001: begin segment = 6'd8; b = 21'b011111001110001101001; end
            10'b0100101010: begin segment = 6'd8; b = 21'b011111001110001100110; end
            10'b0100101011: begin segment = 6'd8; b = 21'b011111001110001100011; end
            10'b0100101100: begin segment = 6'd8; b = 21'b011111001110001100000; end
            10'b0100101101: begin segment = 6'd8; b = 21'b011111001110001011110; end
            10'b0100101110: begin segment = 6'd8; b = 21'b011111001110001011101; end
            10'b0100101111: begin segment = 6'd8; b = 21'b011111001110001011100; end
            10'b0100110000: begin segment = 6'd8; b = 21'b011111001110001011100; end
            10'b0100110001: begin segment = 6'd8; b = 21'b011111001110001011101; end
            10'b0100110010: begin segment = 6'd8; b = 21'b011111001110001011110; end
            10'b0100110011: begin segment = 6'd8; b = 21'b011111001110001100000; end
            10'b0100110100: begin segment = 6'd8; b = 21'b011111001110001100010; end
            10'b0100110101: begin segment = 6'd8; b = 21'b011111001110001100101; end
            10'b0100110110: begin segment = 6'd8; b = 21'b011111001110001101001; end
            10'b0100110111: begin segment = 6'd8; b = 21'b011111001110001101101; end
            10'b0100111000: begin segment = 6'd8; b = 21'b011111001110001110001; end
            10'b0100111001: begin segment = 6'd8; b = 21'b011111001110001110111; end
            10'b0100111010: begin segment = 6'd8; b = 21'b011111001110001111101; end
            10'b0100111011: begin segment = 6'd8; b = 21'b011111001110010000011; end
            10'b0100111100: begin segment = 6'd8; b = 21'b011111001110010001010; end
            10'b0100111101: begin segment = 6'd8; b = 21'b011111001110010010010; end
            10'b0100111110: begin segment = 6'd8; b = 21'b011111001110010011010; end
            10'b0100111111: begin segment = 6'd8; b = 21'b011111001110010100011; end
            10'b0101000000: begin segment = 6'd8; b = 21'b011111001110010101100; end
            10'b0101000001: begin segment = 6'd8; b = 21'b011111001110010110110; end
            10'b0101000010: begin segment = 6'd8; b = 21'b011111001110011000001; end
            10'b0101000011: begin segment = 6'd8; b = 21'b011111001110011001100; end
            10'b0101000100: begin segment = 6'd9; b = 21'b011111000000011001101; end
            10'b0101000101: begin segment = 6'd9; b = 21'b011111000000011000011; end
            10'b0101000110: begin segment = 6'd9; b = 21'b011111000000010111010; end
            10'b0101000111: begin segment = 6'd9; b = 21'b011111000000010110010; end
            10'b0101001000: begin segment = 6'd9; b = 21'b011111000000010101001; end
            10'b0101001001: begin segment = 6'd9; b = 21'b011111000000010100010; end
            10'b0101001010: begin segment = 6'd9; b = 21'b011111000000010011011; end
            10'b0101001011: begin segment = 6'd9; b = 21'b011111000000010010101; end
            10'b0101001100: begin segment = 6'd9; b = 21'b011111000000010010000; end
            10'b0101001101: begin segment = 6'd9; b = 21'b011111000000010001011; end
            10'b0101001110: begin segment = 6'd9; b = 21'b011111000000010000111; end
            10'b0101001111: begin segment = 6'd9; b = 21'b011111000000010000011; end
            10'b0101010000: begin segment = 6'd9; b = 21'b011111000000001111111; end
            10'b0101010001: begin segment = 6'd9; b = 21'b011111000000001111101; end
            10'b0101010010: begin segment = 6'd9; b = 21'b011111000000001111011; end
            10'b0101010011: begin segment = 6'd9; b = 21'b011111000000001111010; end
            10'b0101010100: begin segment = 6'd9; b = 21'b011111000000001111010; end
            10'b0101010101: begin segment = 6'd9; b = 21'b011111000000001111010; end
            10'b0101010110: begin segment = 6'd9; b = 21'b011111000000001111011; end
            10'b0101010111: begin segment = 6'd9; b = 21'b011111000000001111100; end
            10'b0101011000: begin segment = 6'd9; b = 21'b011111000000001111101; end
            10'b0101011001: begin segment = 6'd9; b = 21'b011111000000001111111; end
            10'b0101011010: begin segment = 6'd9; b = 21'b011111000000010000011; end
            10'b0101011011: begin segment = 6'd9; b = 21'b011111000000010000110; end
            10'b0101011100: begin segment = 6'd9; b = 21'b011111000000010001011; end
            10'b0101011101: begin segment = 6'd9; b = 21'b011111000000010010000; end
            10'b0101011110: begin segment = 6'd9; b = 21'b011111000000010010101; end
            10'b0101011111: begin segment = 6'd9; b = 21'b011111000000010011011; end
            10'b0101100000: begin segment = 6'd9; b = 21'b011111000000010100001; end
            10'b0101100001: begin segment = 6'd9; b = 21'b011111000000010101000; end
            10'b0101100010: begin segment = 6'd9; b = 21'b011111000000010110000; end
            10'b0101100011: begin segment = 6'd9; b = 21'b011111000000010111001; end
            10'b0101100100: begin segment = 6'd9; b = 21'b011111000000011000010; end
            10'b0101100101: begin segment = 6'd9; b = 21'b011111000000011001100; end
            10'b0101100110: begin segment = 6'd10; b = 21'b011110110001000110011; end
            10'b0101100111: begin segment = 6'd10; b = 21'b011110110001000101000; end
            10'b0101101000: begin segment = 6'd10; b = 21'b011110110001000011110; end
            10'b0101101001: begin segment = 6'd10; b = 21'b011110110001000010100; end
            10'b0101101010: begin segment = 6'd10; b = 21'b011110110001000001011; end
            10'b0101101011: begin segment = 6'd10; b = 21'b011110110001000000010; end
            10'b0101101100: begin segment = 6'd10; b = 21'b011110110000111111010; end
            10'b0101101101: begin segment = 6'd10; b = 21'b011110110000111110011; end
            10'b0101101110: begin segment = 6'd10; b = 21'b011110110000111101101; end
            10'b0101101111: begin segment = 6'd10; b = 21'b011110110000111100111; end
            10'b0101110000: begin segment = 6'd10; b = 21'b011110110000111100001; end
            10'b0101110001: begin segment = 6'd10; b = 21'b011110110000111011100; end
            10'b0101110010: begin segment = 6'd10; b = 21'b011110110000111011000; end
            10'b0101110011: begin segment = 6'd10; b = 21'b011110110000111010101; end
            10'b0101110100: begin segment = 6'd10; b = 21'b011110110000111010010; end
            10'b0101110101: begin segment = 6'd10; b = 21'b011110110000111001111; end
            10'b0101110110: begin segment = 6'd10; b = 21'b011110110000111001110; end
            10'b0101110111: begin segment = 6'd10; b = 21'b011110110000111001100; end
            10'b0101111000: begin segment = 6'd10; b = 21'b011110110000111001100; end
            10'b0101111001: begin segment = 6'd10; b = 21'b011110110000111001100; end
            10'b0101111010: begin segment = 6'd10; b = 21'b011110110000111001101; end
            10'b0101111011: begin segment = 6'd10; b = 21'b011110110000111001110; end
            10'b0101111100: begin segment = 6'd10; b = 21'b011110110000111010000; end
            10'b0101111101: begin segment = 6'd10; b = 21'b011110110000111010011; end
            10'b0101111110: begin segment = 6'd10; b = 21'b011110110000111010110; end
            10'b0101111111: begin segment = 6'd10; b = 21'b011110110000111011010; end
            10'b0110000000: begin segment = 6'd10; b = 21'b011110110000111011110; end
            10'b0110000001: begin segment = 6'd10; b = 21'b011110110000111100011; end
            10'b0110000010: begin segment = 6'd10; b = 21'b011110110000111101001; end
            10'b0110000011: begin segment = 6'd10; b = 21'b011110110000111110000; end
            10'b0110000100: begin segment = 6'd10; b = 21'b011110110000111110110; end
            10'b0110000101: begin segment = 6'd10; b = 21'b011110110000111111110; end
            10'b0110000110: begin segment = 6'd10; b = 21'b011110110001000000110; end
            10'b0110000111: begin segment = 6'd10; b = 21'b011110110001000001111; end
            10'b0110001000: begin segment = 6'd11; b = 21'b011110100001101101110; end
            10'b0110001001: begin segment = 6'd11; b = 21'b011110100001101100100; end
            10'b0110001010: begin segment = 6'd11; b = 21'b011110100001101011011; end
            10'b0110001011: begin segment = 6'd11; b = 21'b011110100001101010011; end
            10'b0110001100: begin segment = 6'd11; b = 21'b011110100001101001011; end
            10'b0110001101: begin segment = 6'd11; b = 21'b011110100001101000011; end
            10'b0110001110: begin segment = 6'd11; b = 21'b011110100001100111101; end
            10'b0110001111: begin segment = 6'd11; b = 21'b011110100001100110110; end
            10'b0110010000: begin segment = 6'd11; b = 21'b011110100001100110001; end
            10'b0110010001: begin segment = 6'd11; b = 21'b011110100001100101100; end
            10'b0110010010: begin segment = 6'd11; b = 21'b011110100001100101000; end
            10'b0110010011: begin segment = 6'd11; b = 21'b011110100001100100100; end
            10'b0110010100: begin segment = 6'd11; b = 21'b011110100001100100001; end
            10'b0110010101: begin segment = 6'd11; b = 21'b011110100001100011111; end
            10'b0110010110: begin segment = 6'd11; b = 21'b011110100001100011101; end
            10'b0110010111: begin segment = 6'd11; b = 21'b011110100001100011100; end
            10'b0110011000: begin segment = 6'd11; b = 21'b011110100001100011100; end
            10'b0110011001: begin segment = 6'd11; b = 21'b011110100001100011100; end
            10'b0110011010: begin segment = 6'd11; b = 21'b011110100001100011101; end
            10'b0110011011: begin segment = 6'd11; b = 21'b011110100001100011110; end
            10'b0110011100: begin segment = 6'd11; b = 21'b011110100001100100000; end
            10'b0110011101: begin segment = 6'd11; b = 21'b011110100001100100011; end
            10'b0110011110: begin segment = 6'd11; b = 21'b011110100001100100110; end
            10'b0110011111: begin segment = 6'd11; b = 21'b011110100001100101010; end
            10'b0110100000: begin segment = 6'd11; b = 21'b011110100001100101111; end
            10'b0110100001: begin segment = 6'd11; b = 21'b011110100001100110100; end
            10'b0110100010: begin segment = 6'd11; b = 21'b011110100001100111010; end
            10'b0110100011: begin segment = 6'd11; b = 21'b011110100001101000001; end
            10'b0110100100: begin segment = 6'd11; b = 21'b011110100001101001000; end
            10'b0110100101: begin segment = 6'd11; b = 21'b011110100001101010000; end
            10'b0110100110: begin segment = 6'd11; b = 21'b011110100001101011000; end
            10'b0110100111: begin segment = 6'd11; b = 21'b011110100001101100001; end
            10'b0110101000: begin segment = 6'd11; b = 21'b011110100001101101011; end
            10'b0110101001: begin segment = 6'd12; b = 21'b011110010000010001110; end
            10'b0110101010: begin segment = 6'd12; b = 21'b011110010000010000100; end
            10'b0110101011: begin segment = 6'd12; b = 21'b011110010000001111011; end
            10'b0110101100: begin segment = 6'd12; b = 21'b011110010000001110010; end
            10'b0110101101: begin segment = 6'd12; b = 21'b011110010000001101010; end
            10'b0110101110: begin segment = 6'd12; b = 21'b011110010000001100010; end
            10'b0110101111: begin segment = 6'd12; b = 21'b011110010000001011100; end
            10'b0110110000: begin segment = 6'd12; b = 21'b011110010000001010101; end
            10'b0110110001: begin segment = 6'd12; b = 21'b011110010000001010000; end
            10'b0110110010: begin segment = 6'd12; b = 21'b011110010000001001011; end
            10'b0110110011: begin segment = 6'd12; b = 21'b011110010000001000111; end
            10'b0110110100: begin segment = 6'd12; b = 21'b011110010000001000011; end
            10'b0110110101: begin segment = 6'd12; b = 21'b011110010000001000000; end
            10'b0110110110: begin segment = 6'd12; b = 21'b011110010000000111110; end
            10'b0110110111: begin segment = 6'd12; b = 21'b011110010000000111101; end
            10'b0110111000: begin segment = 6'd12; b = 21'b011110010000000111100; end
            10'b0110111001: begin segment = 6'd12; b = 21'b011110010000000111011; end
            10'b0110111010: begin segment = 6'd12; b = 21'b011110010000000111100; end
            10'b0110111011: begin segment = 6'd12; b = 21'b011110010000000111101; end
            10'b0110111100: begin segment = 6'd12; b = 21'b011110010000000111110; end
            10'b0110111101: begin segment = 6'd12; b = 21'b011110010000001000000; end
            10'b0110111110: begin segment = 6'd12; b = 21'b011110010000001000011; end
            10'b0110111111: begin segment = 6'd12; b = 21'b011110010000001000111; end
            10'b0111000000: begin segment = 6'd12; b = 21'b011110010000001001011; end
            10'b0111000001: begin segment = 6'd12; b = 21'b011110010000001010000; end
            10'b0111000010: begin segment = 6'd12; b = 21'b011110010000001010110; end
            10'b0111000011: begin segment = 6'd12; b = 21'b011110010000001011100; end
            10'b0111000100: begin segment = 6'd12; b = 21'b011110010000001100011; end
            10'b0111000101: begin segment = 6'd12; b = 21'b011110010000001101010; end
            10'b0111000110: begin segment = 6'd12; b = 21'b011110010000001110010; end
            10'b0111000111: begin segment = 6'd12; b = 21'b011110010000001111011; end
            10'b0111001000: begin segment = 6'd12; b = 21'b011110010000010000101; end
            10'b0111001001: begin segment = 6'd12; b = 21'b011110010000010001111; end
            10'b0111001010: begin segment = 6'd13; b = 21'b011101111101011111101; end
            10'b0111001011: begin segment = 6'd13; b = 21'b011101111101011110011; end
            10'b0111001100: begin segment = 6'd13; b = 21'b011101111101011101010; end
            10'b0111001101: begin segment = 6'd13; b = 21'b011101111101011100010; end
            10'b0111001110: begin segment = 6'd13; b = 21'b011101111101011011010; end
            10'b0111001111: begin segment = 6'd13; b = 21'b011101111101011010011; end
            10'b0111010000: begin segment = 6'd13; b = 21'b011101111101011001101; end
            10'b0111010001: begin segment = 6'd13; b = 21'b011101111101011000111; end
            10'b0111010010: begin segment = 6'd13; b = 21'b011101111101011000010; end
            10'b0111010011: begin segment = 6'd13; b = 21'b011101111101010111110; end
            10'b0111010100: begin segment = 6'd13; b = 21'b011101111101010111010; end
            10'b0111010101: begin segment = 6'd13; b = 21'b011101111101010110111; end
            10'b0111010110: begin segment = 6'd13; b = 21'b011101111101010110101; end
            10'b0111010111: begin segment = 6'd13; b = 21'b011101111101010110011; end
            10'b0111011000: begin segment = 6'd13; b = 21'b011101111101010110010; end
            10'b0111011001: begin segment = 6'd13; b = 21'b011101111101010110010; end
            10'b0111011010: begin segment = 6'd13; b = 21'b011101111101010110010; end
            10'b0111011011: begin segment = 6'd13; b = 21'b011101111101010110011; end
            10'b0111011100: begin segment = 6'd13; b = 21'b011101111101010110101; end
            10'b0111011101: begin segment = 6'd13; b = 21'b011101111101010110111; end
            10'b0111011110: begin segment = 6'd13; b = 21'b011101111101010111010; end
            10'b0111011111: begin segment = 6'd13; b = 21'b011101111101010111110; end
            10'b0111100000: begin segment = 6'd13; b = 21'b011101111101011000010; end
            10'b0111100001: begin segment = 6'd13; b = 21'b011101111101011000111; end
            10'b0111100010: begin segment = 6'd13; b = 21'b011101111101011001101; end
            10'b0111100011: begin segment = 6'd13; b = 21'b011101111101011010011; end
            10'b0111100100: begin segment = 6'd13; b = 21'b011101111101011011010; end
            10'b0111100101: begin segment = 6'd13; b = 21'b011101111101011100001; end
            10'b0111100110: begin segment = 6'd13; b = 21'b011101111101011101010; end
            10'b0111100111: begin segment = 6'd13; b = 21'b011101111101011110011; end
            10'b0111101000: begin segment = 6'd13; b = 21'b011101111101011111100; end
            10'b0111101001: begin segment = 6'd13; b = 21'b011101111101100000111; end
            10'b0111101010: begin segment = 6'd13; b = 21'b011101111101100010010; end
            10'b0111101011: begin segment = 6'd14; b = 21'b011101100111111101010; end
            10'b0111101100: begin segment = 6'd14; b = 21'b011101100111111100000; end
            10'b0111101101: begin segment = 6'd14; b = 21'b011101100111111010110; end
            10'b0111101110: begin segment = 6'd14; b = 21'b011101100111111001101; end
            10'b0111101111: begin segment = 6'd14; b = 21'b011101100111111000101; end
            10'b0111110000: begin segment = 6'd14; b = 21'b011101100111110111110; end
            10'b0111110001: begin segment = 6'd14; b = 21'b011101100111110110111; end
            10'b0111110010: begin segment = 6'd14; b = 21'b011101100111110110001; end
            10'b0111110011: begin segment = 6'd14; b = 21'b011101100111110101011; end
            10'b0111110100: begin segment = 6'd14; b = 21'b011101100111110100110; end
            10'b0111110101: begin segment = 6'd14; b = 21'b011101100111110100010; end
            10'b0111110110: begin segment = 6'd14; b = 21'b011101100111110011111; end
            10'b0111110111: begin segment = 6'd14; b = 21'b011101100111110011100; end
            10'b0111111000: begin segment = 6'd14; b = 21'b011101100111110011010; end
            10'b0111111001: begin segment = 6'd14; b = 21'b011101100111110011001; end
            10'b0111111010: begin segment = 6'd14; b = 21'b011101100111110011000; end
            10'b0111111011: begin segment = 6'd14; b = 21'b011101100111110011000; end
            10'b0111111100: begin segment = 6'd14; b = 21'b011101100111110011000; end
            10'b0111111101: begin segment = 6'd14; b = 21'b011101100111110011010; end
            10'b0111111110: begin segment = 6'd14; b = 21'b011101100111110011011; end
            10'b0111111111: begin segment = 6'd14; b = 21'b011101100111110011110; end
            10'b1000000000: begin segment = 6'd14; b = 21'b011101100111110100001; end
            10'b1000000001: begin segment = 6'd14; b = 21'b011101100111110100110; end
            10'b1000000010: begin segment = 6'd14; b = 21'b011101100111110101010; end
            10'b1000000011: begin segment = 6'd14; b = 21'b011101100111110110000; end
            10'b1000000100: begin segment = 6'd14; b = 21'b011101100111110110101; end
            10'b1000000101: begin segment = 6'd14; b = 21'b011101100111110111100; end
            10'b1000000110: begin segment = 6'd14; b = 21'b011101100111111000011; end
            10'b1000000111: begin segment = 6'd14; b = 21'b011101100111111001100; end
            10'b1000001000: begin segment = 6'd14; b = 21'b011101100111111010100; end
            10'b1000001001: begin segment = 6'd14; b = 21'b011101100111111011110; end
            10'b1000001010: begin segment = 6'd14; b = 21'b011101100111111100111; end
            10'b1000001011: begin segment = 6'd15; b = 21'b011101010001101011000; end
            10'b1000001100: begin segment = 6'd15; b = 21'b011101010001101001101; end
            10'b1000001101: begin segment = 6'd15; b = 21'b011101010001101000100; end
            10'b1000001110: begin segment = 6'd15; b = 21'b011101010001100111011; end
            10'b1000001111: begin segment = 6'd15; b = 21'b011101010001100110011; end
            10'b1000010000: begin segment = 6'd15; b = 21'b011101010001100101010; end
            10'b1000010001: begin segment = 6'd15; b = 21'b011101010001100100011; end
            10'b1000010010: begin segment = 6'd15; b = 21'b011101010001100011101; end
            10'b1000010011: begin segment = 6'd15; b = 21'b011101010001100011000; end
            10'b1000010100: begin segment = 6'd15; b = 21'b011101010001100010011; end
            10'b1000010101: begin segment = 6'd15; b = 21'b011101010001100001111; end
            10'b1000010110: begin segment = 6'd15; b = 21'b011101010001100001100; end
            10'b1000010111: begin segment = 6'd15; b = 21'b011101010001100001001; end
            10'b1000011000: begin segment = 6'd15; b = 21'b011101010001100000110; end
            10'b1000011001: begin segment = 6'd15; b = 21'b011101010001100000101; end
            10'b1000011010: begin segment = 6'd15; b = 21'b011101010001100000101; end
            10'b1000011011: begin segment = 6'd15; b = 21'b011101010001100000101; end
            10'b1000011100: begin segment = 6'd15; b = 21'b011101010001100000101; end
            10'b1000011101: begin segment = 6'd15; b = 21'b011101010001100000111; end
            10'b1000011110: begin segment = 6'd15; b = 21'b011101010001100001010; end
            10'b1000011111: begin segment = 6'd15; b = 21'b011101010001100001100; end
            10'b1000100000: begin segment = 6'd15; b = 21'b011101010001100001110; end
            10'b1000100001: begin segment = 6'd15; b = 21'b011101010001100010011; end
            10'b1000100010: begin segment = 6'd15; b = 21'b011101010001100011000; end
            10'b1000100011: begin segment = 6'd15; b = 21'b011101010001100011110; end
            10'b1000100100: begin segment = 6'd15; b = 21'b011101010001100100011; end
            10'b1000100101: begin segment = 6'd15; b = 21'b011101010001100101011; end
            10'b1000100110: begin segment = 6'd15; b = 21'b011101010001100110011; end
            10'b1000100111: begin segment = 6'd15; b = 21'b011101010001100111011; end
            10'b1000101000: begin segment = 6'd15; b = 21'b011101010001101000100; end
            10'b1000101001: begin segment = 6'd15; b = 21'b011101010001101001110; end
            10'b1000101010: begin segment = 6'd15; b = 21'b011101010001101011000; end
            10'b1000101011: begin segment = 6'd16; b = 21'b011100111001000100111; end
            10'b1000101100: begin segment = 6'd16; b = 21'b011100111001000011100; end
            10'b1000101101: begin segment = 6'd16; b = 21'b011100111001000010010; end
            10'b1000101110: begin segment = 6'd16; b = 21'b011100111001000001000; end
            10'b1000101111: begin segment = 6'd16; b = 21'b011100111001000000000; end
            10'b1000110000: begin segment = 6'd16; b = 21'b011100111000111110111; end
            10'b1000110001: begin segment = 6'd16; b = 21'b011100111000111110000; end
            10'b1000110010: begin segment = 6'd16; b = 21'b011100111000111101001; end
            10'b1000110011: begin segment = 6'd16; b = 21'b011100111000111100100; end
            10'b1000110100: begin segment = 6'd16; b = 21'b011100111000111011110; end
            10'b1000110101: begin segment = 6'd16; b = 21'b011100111000111011010; end
            10'b1000110110: begin segment = 6'd16; b = 21'b011100111000111010110; end
            10'b1000110111: begin segment = 6'd16; b = 21'b011100111000111010011; end
            10'b1000111000: begin segment = 6'd16; b = 21'b011100111000111010001; end
            10'b1000111001: begin segment = 6'd16; b = 21'b011100111000111001111; end
            10'b1000111010: begin segment = 6'd16; b = 21'b011100111000111001110; end
            10'b1000111011: begin segment = 6'd16; b = 21'b011100111000111001110; end
            10'b1000111100: begin segment = 6'd16; b = 21'b011100111000111001111; end
            10'b1000111101: begin segment = 6'd16; b = 21'b011100111000111010000; end
            10'b1000111110: begin segment = 6'd16; b = 21'b011100111000111010010; end
            10'b1000111111: begin segment = 6'd16; b = 21'b011100111000111010100; end
            10'b1001000000: begin segment = 6'd16; b = 21'b011100111000111010111; end
            10'b1001000001: begin segment = 6'd16; b = 21'b011100111000111011011; end
            10'b1001000010: begin segment = 6'd16; b = 21'b011100111000111100000; end
            10'b1001000011: begin segment = 6'd16; b = 21'b011100111000111100101; end
            10'b1001000100: begin segment = 6'd16; b = 21'b011100111000111101011; end
            10'b1001000101: begin segment = 6'd16; b = 21'b011100111000111110010; end
            10'b1001000110: begin segment = 6'd16; b = 21'b011100111000111111010; end
            10'b1001000111: begin segment = 6'd16; b = 21'b011100111001000000010; end
            10'b1001001000: begin segment = 6'd16; b = 21'b011100111001000001011; end
            10'b1001001001: begin segment = 6'd16; b = 21'b011100111001000010100; end
            10'b1001001010: begin segment = 6'd16; b = 21'b011100111001000011111; end
            10'b1001001011: begin segment = 6'd17; b = 21'b011100011110101100001; end
            10'b1001001100: begin segment = 6'd17; b = 21'b011100011110101010110; end
            10'b1001001101: begin segment = 6'd17; b = 21'b011100011110101001100; end
            10'b1001001110: begin segment = 6'd17; b = 21'b011100011110101000010; end
            10'b1001001111: begin segment = 6'd17; b = 21'b011100011110100111001; end
            10'b1001010000: begin segment = 6'd17; b = 21'b011100011110100110000; end
            10'b1001010001: begin segment = 6'd17; b = 21'b011100011110100101001; end
            10'b1001010010: begin segment = 6'd17; b = 21'b011100011110100100010; end
            10'b1001010011: begin segment = 6'd17; b = 21'b011100011110100011100; end
            10'b1001010100: begin segment = 6'd17; b = 21'b011100011110100010110; end
            10'b1001010101: begin segment = 6'd17; b = 21'b011100011110100010001; end
            10'b1001010110: begin segment = 6'd17; b = 21'b011100011110100001101; end
            10'b1001010111: begin segment = 6'd17; b = 21'b011100011110100001010; end
            10'b1001011000: begin segment = 6'd17; b = 21'b011100011110100000111; end
            10'b1001011001: begin segment = 6'd17; b = 21'b011100011110100000101; end
            10'b1001011010: begin segment = 6'd17; b = 21'b011100011110100000100; end
            10'b1001011011: begin segment = 6'd17; b = 21'b011100011110100000100; end
            10'b1001011100: begin segment = 6'd17; b = 21'b011100011110100000100; end
            10'b1001011101: begin segment = 6'd17; b = 21'b011100011110100000101; end
            10'b1001011110: begin segment = 6'd17; b = 21'b011100011110100000111; end
            10'b1001011111: begin segment = 6'd17; b = 21'b011100011110100001001; end
            10'b1001100000: begin segment = 6'd17; b = 21'b011100011110100001100; end
            10'b1001100001: begin segment = 6'd17; b = 21'b011100011110100010000; end
            10'b1001100010: begin segment = 6'd17; b = 21'b011100011110100010101; end
            10'b1001100011: begin segment = 6'd17; b = 21'b011100011110100011010; end
            10'b1001100100: begin segment = 6'd17; b = 21'b011100011110100100001; end
            10'b1001100101: begin segment = 6'd17; b = 21'b011100011110100100111; end
            10'b1001100110: begin segment = 6'd17; b = 21'b011100011110100101111; end
            10'b1001100111: begin segment = 6'd17; b = 21'b011100011110100110111; end
            10'b1001101000: begin segment = 6'd17; b = 21'b011100011110101000000; end
            10'b1001101001: begin segment = 6'd17; b = 21'b011100011110101001010; end
            10'b1001101010: begin segment = 6'd18; b = 21'b011100000100001111010; end
            10'b1001101011: begin segment = 6'd18; b = 21'b011100000100001101111; end
            10'b1001101100: begin segment = 6'd18; b = 21'b011100000100001100101; end
            10'b1001101101: begin segment = 6'd18; b = 21'b011100000100001011100; end
            10'b1001101110: begin segment = 6'd18; b = 21'b011100000100001010011; end
            10'b1001101111: begin segment = 6'd18; b = 21'b011100000100001001100; end
            10'b1001110000: begin segment = 6'd18; b = 21'b011100000100001000101; end
            10'b1001110001: begin segment = 6'd18; b = 21'b011100000100000111111; end
            10'b1001110010: begin segment = 6'd18; b = 21'b011100000100000111001; end
            10'b1001110011: begin segment = 6'd18; b = 21'b011100000100000110100; end
            10'b1001110100: begin segment = 6'd18; b = 21'b011100000100000110000; end
            10'b1001110101: begin segment = 6'd18; b = 21'b011100000100000101100; end
            10'b1001110110: begin segment = 6'd18; b = 21'b011100000100000101010; end
            10'b1001110111: begin segment = 6'd18; b = 21'b011100000100000101000; end
            10'b1001111000: begin segment = 6'd18; b = 21'b011100000100000101000; end
            10'b1001111001: begin segment = 6'd18; b = 21'b011100000100000100111; end
            10'b1001111010: begin segment = 6'd18; b = 21'b011100000100000101000; end
            10'b1001111011: begin segment = 6'd18; b = 21'b011100000100000101001; end
            10'b1001111100: begin segment = 6'd18; b = 21'b011100000100000101010; end
            10'b1001111101: begin segment = 6'd18; b = 21'b011100000100000101101; end
            10'b1001111110: begin segment = 6'd18; b = 21'b011100000100000110000; end
            10'b1001111111: begin segment = 6'd18; b = 21'b011100000100000110100; end
            10'b1010000000: begin segment = 6'd18; b = 21'b011100000100000111001; end
            10'b1010000001: begin segment = 6'd18; b = 21'b011100000100000111111; end
            10'b1010000010: begin segment = 6'd18; b = 21'b011100000100001000101; end
            10'b1010000011: begin segment = 6'd18; b = 21'b011100000100001001100; end
            10'b1010000100: begin segment = 6'd18; b = 21'b011100000100001010011; end
            10'b1010000101: begin segment = 6'd18; b = 21'b011100000100001011100; end
            10'b1010000110: begin segment = 6'd18; b = 21'b011100000100001100101; end
            10'b1010000111: begin segment = 6'd18; b = 21'b011100000100001101111; end
            10'b1010001000: begin segment = 6'd18; b = 21'b011100000100001111010; end
            10'b1010001001: begin segment = 6'd19; b = 21'b011011101000001100011; end
            10'b1010001010: begin segment = 6'd19; b = 21'b011011101000001011001; end
            10'b1010001011: begin segment = 6'd19; b = 21'b011011101000001010000; end
            10'b1010001100: begin segment = 6'd19; b = 21'b011011101000001000111; end
            10'b1010001101: begin segment = 6'd19; b = 21'b011011101000001000000; end
            10'b1010001110: begin segment = 6'd19; b = 21'b011011101000000111001; end
            10'b1010001111: begin segment = 6'd19; b = 21'b011011101000000110011; end
            10'b1010010000: begin segment = 6'd19; b = 21'b011011101000000101101; end
            10'b1010010001: begin segment = 6'd19; b = 21'b011011101000000101000; end
            10'b1010010010: begin segment = 6'd19; b = 21'b011011101000000100101; end
            10'b1010010011: begin segment = 6'd19; b = 21'b011011101000000100001; end
            10'b1010010100: begin segment = 6'd19; b = 21'b011011101000000011111; end
            10'b1010010101: begin segment = 6'd19; b = 21'b011011101000000011101; end
            10'b1010010110: begin segment = 6'd19; b = 21'b011011101000000011100; end
            10'b1010010111: begin segment = 6'd19; b = 21'b011011101000000011100; end
            10'b1010011000: begin segment = 6'd19; b = 21'b011011101000000011101; end
            10'b1010011001: begin segment = 6'd19; b = 21'b011011101000000011110; end
            10'b1010011010: begin segment = 6'd19; b = 21'b011011101000000100000; end
            10'b1010011011: begin segment = 6'd19; b = 21'b011011101000000100011; end
            10'b1010011100: begin segment = 6'd19; b = 21'b011011101000000100110; end
            10'b1010011101: begin segment = 6'd19; b = 21'b011011101000000101011; end
            10'b1010011110: begin segment = 6'd19; b = 21'b011011101000000110000; end
            10'b1010011111: begin segment = 6'd19; b = 21'b011011101000000110110; end
            10'b1010100000: begin segment = 6'd19; b = 21'b011011101000000111100; end
            10'b1010100001: begin segment = 6'd19; b = 21'b011011101000001000100; end
            10'b1010100010: begin segment = 6'd19; b = 21'b011011101000001001100; end
            10'b1010100011: begin segment = 6'd19; b = 21'b011011101000001010101; end
            10'b1010100100: begin segment = 6'd19; b = 21'b011011101000001011110; end
            10'b1010100101: begin segment = 6'd19; b = 21'b011011101000001101001; end
            10'b1010100110: begin segment = 6'd19; b = 21'b011011101000001110100; end
            10'b1010100111: begin segment = 6'd19; b = 21'b011011101000010000000; end
            10'b1010101000: begin segment = 6'd20; b = 21'b011011001000000010110; end
            10'b1010101001: begin segment = 6'd20; b = 21'b011011001000000001011; end
            10'b1010101010: begin segment = 6'd20; b = 21'b011011001000000000010; end
            10'b1010101011: begin segment = 6'd20; b = 21'b011011000111111111000; end
            10'b1010101100: begin segment = 6'd20; b = 21'b011011000111111101111; end
            10'b1010101101: begin segment = 6'd20; b = 21'b011011000111111101000; end
            10'b1010101110: begin segment = 6'd20; b = 21'b011011000111111100001; end
            10'b1010101111: begin segment = 6'd20; b = 21'b011011000111111011011; end
            10'b1010110000: begin segment = 6'd20; b = 21'b011011000111111010101; end
            10'b1010110001: begin segment = 6'd20; b = 21'b011011000111111010000; end
            10'b1010110010: begin segment = 6'd20; b = 21'b011011000111111001101; end
            10'b1010110011: begin segment = 6'd20; b = 21'b011011000111111001010; end
            10'b1010110100: begin segment = 6'd20; b = 21'b011011000111111000111; end
            10'b1010110101: begin segment = 6'd20; b = 21'b011011000111111000101; end
            10'b1010110110: begin segment = 6'd20; b = 21'b011011000111111000101; end
            10'b1010110111: begin segment = 6'd20; b = 21'b011011000111111000101; end
            10'b1010111000: begin segment = 6'd20; b = 21'b011011000111111000101; end
            10'b1010111001: begin segment = 6'd20; b = 21'b011011000111111000110; end
            10'b1010111010: begin segment = 6'd20; b = 21'b011011000111111001001; end
            10'b1010111011: begin segment = 6'd20; b = 21'b011011000111111001100; end
            10'b1010111100: begin segment = 6'd20; b = 21'b011011000111111001111; end
            10'b1010111101: begin segment = 6'd20; b = 21'b011011000111111010100; end
            10'b1010111110: begin segment = 6'd20; b = 21'b011011000111111011001; end
            10'b1010111111: begin segment = 6'd20; b = 21'b011011000111111100000; end
            10'b1011000000: begin segment = 6'd20; b = 21'b011011000111111100110; end
            10'b1011000001: begin segment = 6'd20; b = 21'b011011000111111101110; end
            10'b1011000010: begin segment = 6'd20; b = 21'b011011000111111110110; end
            10'b1011000011: begin segment = 6'd20; b = 21'b011011001000000000000; end
            10'b1011000100: begin segment = 6'd20; b = 21'b011011001000000001001; end
            10'b1011000101: begin segment = 6'd20; b = 21'b011011001000000010100; end
            10'b1011000110: begin segment = 6'd21; b = 21'b011010100111110011000; end
            10'b1011000111: begin segment = 6'd21; b = 21'b011010100111110001101; end
            10'b1011001000: begin segment = 6'd21; b = 21'b011010100111110000011; end
            10'b1011001001: begin segment = 6'd21; b = 21'b011010100111101111010; end
            10'b1011001010: begin segment = 6'd21; b = 21'b011010100111101110001; end
            10'b1011001011: begin segment = 6'd21; b = 21'b011010100111101101001; end
            10'b1011001100: begin segment = 6'd21; b = 21'b011010100111101100010; end
            10'b1011001101: begin segment = 6'd21; b = 21'b011010100111101011100; end
            10'b1011001110: begin segment = 6'd21; b = 21'b011010100111101010110; end
            10'b1011001111: begin segment = 6'd21; b = 21'b011010100111101010001; end
            10'b1011010000: begin segment = 6'd21; b = 21'b011010100111101001110; end
            10'b1011010001: begin segment = 6'd21; b = 21'b011010100111101001010; end
            10'b1011010010: begin segment = 6'd21; b = 21'b011010100111101001000; end
            10'b1011010011: begin segment = 6'd21; b = 21'b011010100111101000110; end
            10'b1011010100: begin segment = 6'd21; b = 21'b011010100111101000110; end
            10'b1011010101: begin segment = 6'd21; b = 21'b011010100111101000101; end
            10'b1011010110: begin segment = 6'd21; b = 21'b011010100111101000111; end
            10'b1011010111: begin segment = 6'd21; b = 21'b011010100111101001000; end
            10'b1011011000: begin segment = 6'd21; b = 21'b011010100111101001011; end
            10'b1011011001: begin segment = 6'd21; b = 21'b011010100111101001101; end
            10'b1011011010: begin segment = 6'd21; b = 21'b011010100111101010010; end
            10'b1011011011: begin segment = 6'd21; b = 21'b011010100111101010110; end
            10'b1011011100: begin segment = 6'd21; b = 21'b011010100111101011100; end
            10'b1011011101: begin segment = 6'd21; b = 21'b011010100111101100001; end
            10'b1011011110: begin segment = 6'd21; b = 21'b011010100111101101001; end
            10'b1011011111: begin segment = 6'd21; b = 21'b011010100111101110000; end
            10'b1011100000: begin segment = 6'd21; b = 21'b011010100111101111001; end
            10'b1011100001: begin segment = 6'd21; b = 21'b011010100111110000010; end
            10'b1011100010: begin segment = 6'd21; b = 21'b011010100111110001101; end
            10'b1011100011: begin segment = 6'd21; b = 21'b011010100111110011000; end
            10'b1011100100: begin segment = 6'd22; b = 21'b011010000101110101010; end
            10'b1011100101: begin segment = 6'd22; b = 21'b011010000101110011111; end
            10'b1011100110: begin segment = 6'd22; b = 21'b011010000101110010101; end
            10'b1011100111: begin segment = 6'd22; b = 21'b011010000101110001100; end
            10'b1011101000: begin segment = 6'd22; b = 21'b011010000101110000011; end
            10'b1011101001: begin segment = 6'd22; b = 21'b011010000101101111011; end
            10'b1011101010: begin segment = 6'd22; b = 21'b011010000101101110100; end
            10'b1011101011: begin segment = 6'd22; b = 21'b011010000101101101110; end
            10'b1011101100: begin segment = 6'd22; b = 21'b011010000101101101001; end
            10'b1011101101: begin segment = 6'd22; b = 21'b011010000101101100100; end
            10'b1011101110: begin segment = 6'd22; b = 21'b011010000101101100001; end
            10'b1011101111: begin segment = 6'd22; b = 21'b011010000101101011110; end
            10'b1011110000: begin segment = 6'd22; b = 21'b011010000101101011100; end
            10'b1011110001: begin segment = 6'd22; b = 21'b011010000101101011010; end
            10'b1011110010: begin segment = 6'd22; b = 21'b011010000101101011010; end
            10'b1011110011: begin segment = 6'd22; b = 21'b011010000101101011010; end
            10'b1011110100: begin segment = 6'd22; b = 21'b011010000101101011011; end
            10'b1011110101: begin segment = 6'd22; b = 21'b011010000101101011101; end
            10'b1011110110: begin segment = 6'd22; b = 21'b011010000101101011111; end
            10'b1011110111: begin segment = 6'd22; b = 21'b011010000101101100011; end
            10'b1011111000: begin segment = 6'd22; b = 21'b011010000101101100111; end
            10'b1011111001: begin segment = 6'd22; b = 21'b011010000101101101100; end
            10'b1011111010: begin segment = 6'd22; b = 21'b011010000101101110010; end
            10'b1011111011: begin segment = 6'd22; b = 21'b011010000101101111001; end
            10'b1011111100: begin segment = 6'd22; b = 21'b011010000101110000000; end
            10'b1011111101: begin segment = 6'd22; b = 21'b011010000101110001000; end
            10'b1011111110: begin segment = 6'd22; b = 21'b011010000101110010010; end
            10'b1011111111: begin segment = 6'd22; b = 21'b011010000101110011011; end
            10'b1100000000: begin segment = 6'd22; b = 21'b011010000101110100110; end
            10'b1100000001: begin segment = 6'd22; b = 21'b011010000101110110010; end
            10'b1100000010: begin segment = 6'd23; b = 21'b011001100000010000000; end
            10'b1100000011: begin segment = 6'd23; b = 21'b011001100000001110100; end
            10'b1100000100: begin segment = 6'd23; b = 21'b011001100000001101001; end
            10'b1100000101: begin segment = 6'd23; b = 21'b011001100000001011110; end
            10'b1100000110: begin segment = 6'd23; b = 21'b011001100000001010101; end
            10'b1100000111: begin segment = 6'd23; b = 21'b011001100000001001100; end
            10'b1100001000: begin segment = 6'd23; b = 21'b011001100000001000101; end
            10'b1100001001: begin segment = 6'd23; b = 21'b011001100000000111110; end
            10'b1100001010: begin segment = 6'd23; b = 21'b011001100000000110111; end
            10'b1100001011: begin segment = 6'd23; b = 21'b011001100000000110010; end
            10'b1100001100: begin segment = 6'd23; b = 21'b011001100000000101101; end
            10'b1100001101: begin segment = 6'd23; b = 21'b011001100000000101010; end
            10'b1100001110: begin segment = 6'd23; b = 21'b011001100000000100111; end
            10'b1100001111: begin segment = 6'd23; b = 21'b011001100000000100101; end
            10'b1100010000: begin segment = 6'd23; b = 21'b011001100000000100011; end
            10'b1100010001: begin segment = 6'd23; b = 21'b011001100000000100011; end
            10'b1100010010: begin segment = 6'd23; b = 21'b011001100000000100011; end
            10'b1100010011: begin segment = 6'd23; b = 21'b011001100000000100100; end
            10'b1100010100: begin segment = 6'd23; b = 21'b011001100000000100110; end
            10'b1100010101: begin segment = 6'd23; b = 21'b011001100000000101001; end
            10'b1100010110: begin segment = 6'd23; b = 21'b011001100000000101101; end
            10'b1100010111: begin segment = 6'd23; b = 21'b011001100000000110001; end
            10'b1100011000: begin segment = 6'd23; b = 21'b011001100000000110110; end
            10'b1100011001: begin segment = 6'd23; b = 21'b011001100000000111101; end
            10'b1100011010: begin segment = 6'd23; b = 21'b011001100000001000011; end
            10'b1100011011: begin segment = 6'd23; b = 21'b011001100000001001011; end
            10'b1100011100: begin segment = 6'd23; b = 21'b011001100000001010100; end
            10'b1100011101: begin segment = 6'd23; b = 21'b011001100000001011101; end
            10'b1100011110: begin segment = 6'd23; b = 21'b011001100000001100111; end
            10'b1100011111: begin segment = 6'd24; b = 21'b011000111010101111110; end
            10'b1100100000: begin segment = 6'd24; b = 21'b011000111010101110010; end
            10'b1100100001: begin segment = 6'd24; b = 21'b011000111010101100111; end
            10'b1100100010: begin segment = 6'd24; b = 21'b011000111010101011100; end
            10'b1100100011: begin segment = 6'd24; b = 21'b011000111010101010011; end
            10'b1100100100: begin segment = 6'd24; b = 21'b011000111010101001010; end
            10'b1100100101: begin segment = 6'd24; b = 21'b011000111010101000010; end
            10'b1100100110: begin segment = 6'd24; b = 21'b011000111010100111011; end
            10'b1100100111: begin segment = 6'd24; b = 21'b011000111010100110100; end
            10'b1100101000: begin segment = 6'd24; b = 21'b011000111010100101111; end
            10'b1100101001: begin segment = 6'd24; b = 21'b011000111010100101010; end
            10'b1100101010: begin segment = 6'd24; b = 21'b011000111010100100110; end
            10'b1100101011: begin segment = 6'd24; b = 21'b011000111010100100011; end
            10'b1100101100: begin segment = 6'd24; b = 21'b011000111010100100001; end
            10'b1100101101: begin segment = 6'd24; b = 21'b011000111010100100000; end
            10'b1100101110: begin segment = 6'd24; b = 21'b011000111010100011111; end
            10'b1100101111: begin segment = 6'd24; b = 21'b011000111010100011111; end
            10'b1100110000: begin segment = 6'd24; b = 21'b011000111010100100000; end
            10'b1100110001: begin segment = 6'd24; b = 21'b011000111010100100010; end
            10'b1100110010: begin segment = 6'd24; b = 21'b011000111010100100101; end
            10'b1100110011: begin segment = 6'd24; b = 21'b011000111010100101001; end
            10'b1100110100: begin segment = 6'd24; b = 21'b011000111010100101101; end
            10'b1100110101: begin segment = 6'd24; b = 21'b011000111010100110011; end
            10'b1100110110: begin segment = 6'd24; b = 21'b011000111010100111001; end
            10'b1100110111: begin segment = 6'd24; b = 21'b011000111010101000000; end
            10'b1100111000: begin segment = 6'd24; b = 21'b011000111010101001000; end
            10'b1100111001: begin segment = 6'd24; b = 21'b011000111010101010000; end
            10'b1100111010: begin segment = 6'd24; b = 21'b011000111010101011010; end
            10'b1100111011: begin segment = 6'd24; b = 21'b011000111010101100100; end
            10'b1100111100: begin segment = 6'd25; b = 21'b011000010011111000011; end
            10'b1100111101: begin segment = 6'd25; b = 21'b011000010011110110111; end
            10'b1100111110: begin segment = 6'd25; b = 21'b011000010011110101100; end
            10'b1100111111: begin segment = 6'd25; b = 21'b011000010011110100010; end
            10'b1101000000: begin segment = 6'd25; b = 21'b011000010011110011000; end
            10'b1101000001: begin segment = 6'd25; b = 21'b011000010011110010000; end
            10'b1101000010: begin segment = 6'd25; b = 21'b011000010011110001000; end
            10'b1101000011: begin segment = 6'd25; b = 21'b011000010011110000001; end
            10'b1101000100: begin segment = 6'd25; b = 21'b011000010011101111011; end
            10'b1101000101: begin segment = 6'd25; b = 21'b011000010011101110110; end
            10'b1101000110: begin segment = 6'd25; b = 21'b011000010011101110010; end
            10'b1101000111: begin segment = 6'd25; b = 21'b011000010011101101110; end
            10'b1101001000: begin segment = 6'd25; b = 21'b011000010011101101011; end
            10'b1101001001: begin segment = 6'd25; b = 21'b011000010011101101010; end
            10'b1101001010: begin segment = 6'd25; b = 21'b011000010011101101001; end
            10'b1101001011: begin segment = 6'd25; b = 21'b011000010011101101000; end
            10'b1101001100: begin segment = 6'd25; b = 21'b011000010011101101001; end
            10'b1101001101: begin segment = 6'd25; b = 21'b011000010011101101011; end
            10'b1101001110: begin segment = 6'd25; b = 21'b011000010011101101101; end
            10'b1101001111: begin segment = 6'd25; b = 21'b011000010011101110000; end
            10'b1101010000: begin segment = 6'd25; b = 21'b011000010011101110101; end
            10'b1101010001: begin segment = 6'd25; b = 21'b011000010011101111010; end
            10'b1101010010: begin segment = 6'd25; b = 21'b011000010011101111111; end
            10'b1101010011: begin segment = 6'd25; b = 21'b011000010011110000110; end
            10'b1101010100: begin segment = 6'd25; b = 21'b011000010011110001110; end
            10'b1101010101: begin segment = 6'd25; b = 21'b011000010011110010110; end
            10'b1101010110: begin segment = 6'd25; b = 21'b011000010011110011111; end
            10'b1101010111: begin segment = 6'd25; b = 21'b011000010011110101001; end
            10'b1101011000: begin segment = 6'd25; b = 21'b011000010011110110100; end
            10'b1101011001: begin segment = 6'd26; b = 21'b010111101011000011011; end
            10'b1101011010: begin segment = 6'd26; b = 21'b010111101011000001111; end
            10'b1101011011: begin segment = 6'd26; b = 21'b010111101011000000100; end
            10'b1101011100: begin segment = 6'd26; b = 21'b010111101010111111010; end
            10'b1101011101: begin segment = 6'd26; b = 21'b010111101010111110000; end
            10'b1101011110: begin segment = 6'd26; b = 21'b010111101010111101000; end
            10'b1101011111: begin segment = 6'd26; b = 21'b010111101010111100001; end
            10'b1101100000: begin segment = 6'd26; b = 21'b010111101010111011011; end
            10'b1101100001: begin segment = 6'd26; b = 21'b010111101010111010101; end
            10'b1101100010: begin segment = 6'd26; b = 21'b010111101010111010000; end
            10'b1101100011: begin segment = 6'd26; b = 21'b010111101010111001100; end
            10'b1101100100: begin segment = 6'd26; b = 21'b010111101010111001001; end
            10'b1101100101: begin segment = 6'd26; b = 21'b010111101010111000111; end
            10'b1101100110: begin segment = 6'd26; b = 21'b010111101010111000101; end
            10'b1101100111: begin segment = 6'd26; b = 21'b010111101010111000101; end
            10'b1101101000: begin segment = 6'd26; b = 21'b010111101010111000110; end
            10'b1101101001: begin segment = 6'd26; b = 21'b010111101010111001000; end
            10'b1101101010: begin segment = 6'd26; b = 21'b010111101010111001001; end
            10'b1101101011: begin segment = 6'd26; b = 21'b010111101010111001101; end
            10'b1101101100: begin segment = 6'd26; b = 21'b010111101010111010000; end
            10'b1101101101: begin segment = 6'd26; b = 21'b010111101010111010101; end
            10'b1101101110: begin segment = 6'd26; b = 21'b010111101010111011010; end
            10'b1101101111: begin segment = 6'd26; b = 21'b010111101010111100001; end
            10'b1101110000: begin segment = 6'd26; b = 21'b010111101010111101000; end
            10'b1101110001: begin segment = 6'd26; b = 21'b010111101010111110001; end
            10'b1101110010: begin segment = 6'd26; b = 21'b010111101010111111001; end
            10'b1101110011: begin segment = 6'd26; b = 21'b010111101011000000100; end
            10'b1101110100: begin segment = 6'd26; b = 21'b010111101011000001110; end
            10'b1101110101: begin segment = 6'd26; b = 21'b010111101011000011010; end
            10'b1101110110: begin segment = 6'd27; b = 21'b010111000010001010111; end
            10'b1101110111: begin segment = 6'd27; b = 21'b010111000010001001101; end
            10'b1101111000: begin segment = 6'd27; b = 21'b010111000010001000100; end
            10'b1101111001: begin segment = 6'd27; b = 21'b010111000010000111011; end
            10'b1101111010: begin segment = 6'd27; b = 21'b010111000010000110100; end
            10'b1101111011: begin segment = 6'd27; b = 21'b010111000010000101101; end
            10'b1101111100: begin segment = 6'd27; b = 21'b010111000010000100111; end
            10'b1101111101: begin segment = 6'd27; b = 21'b010111000010000100010; end
            10'b1101111110: begin segment = 6'd27; b = 21'b010111000010000011110; end
            10'b1101111111: begin segment = 6'd27; b = 21'b010111000010000011011; end
            10'b1110000000: begin segment = 6'd27; b = 21'b010111000010000011001; end
            10'b1110000001: begin segment = 6'd27; b = 21'b010111000010000011000; end
            10'b1110000010: begin segment = 6'd27; b = 21'b010111000010000010111; end
            10'b1110000011: begin segment = 6'd27; b = 21'b010111000010000011000; end
            10'b1110000100: begin segment = 6'd27; b = 21'b010111000010000011001; end
            10'b1110000101: begin segment = 6'd27; b = 21'b010111000010000011011; end
            10'b1110000110: begin segment = 6'd27; b = 21'b010111000010000011110; end
            10'b1110000111: begin segment = 6'd27; b = 21'b010111000010000100010; end
            10'b1110001000: begin segment = 6'd27; b = 21'b010111000010000100111; end
            10'b1110001001: begin segment = 6'd27; b = 21'b010111000010000101101; end
            10'b1110001010: begin segment = 6'd27; b = 21'b010111000010000110011; end
            10'b1110001011: begin segment = 6'd27; b = 21'b010111000010000111010; end
            10'b1110001100: begin segment = 6'd27; b = 21'b010111000010001000011; end
            10'b1110001101: begin segment = 6'd27; b = 21'b010111000010001001100; end
            10'b1110001110: begin segment = 6'd27; b = 21'b010111000010001010110; end
            10'b1110001111: begin segment = 6'd27; b = 21'b010111000010001100001; end
            10'b1110010000: begin segment = 6'd27; b = 21'b010111000010001101101; end
            10'b1110010001: begin segment = 6'd27; b = 21'b010111000010001111010; end
            10'b1110010010: begin segment = 6'd28; b = 21'b010110010111011001100; end
            10'b1110010011: begin segment = 6'd28; b = 21'b010110010111011000010; end
            10'b1110010100: begin segment = 6'd28; b = 21'b010110010111010111010; end
            10'b1110010101: begin segment = 6'd28; b = 21'b010110010111010110010; end
            10'b1110010110: begin segment = 6'd28; b = 21'b010110010111010101100; end
            10'b1110010111: begin segment = 6'd28; b = 21'b010110010111010100110; end
            10'b1110011000: begin segment = 6'd28; b = 21'b010110010111010100001; end
            10'b1110011001: begin segment = 6'd28; b = 21'b010110010111010011101; end
            10'b1110011010: begin segment = 6'd28; b = 21'b010110010111010011010; end
            10'b1110011011: begin segment = 6'd28; b = 21'b010110010111010010111; end
            10'b1110011100: begin segment = 6'd28; b = 21'b010110010111010010110; end
            10'b1110011101: begin segment = 6'd28; b = 21'b010110010111010010110; end
            10'b1110011110: begin segment = 6'd28; b = 21'b010110010111010010110; end
            10'b1110011111: begin segment = 6'd28; b = 21'b010110010111010010111; end
            10'b1110100000: begin segment = 6'd28; b = 21'b010110010111010011010; end
            10'b1110100001: begin segment = 6'd28; b = 21'b010110010111010011101; end
            10'b1110100010: begin segment = 6'd28; b = 21'b010110010111010100001; end
            10'b1110100011: begin segment = 6'd28; b = 21'b010110010111010100110; end
            10'b1110100100: begin segment = 6'd28; b = 21'b010110010111010101100; end
            10'b1110100101: begin segment = 6'd28; b = 21'b010110010111010110010; end
            10'b1110100110: begin segment = 6'd28; b = 21'b010110010111010111010; end
            10'b1110100111: begin segment = 6'd28; b = 21'b010110010111011000011; end
            10'b1110101000: begin segment = 6'd28; b = 21'b010110010111011001100; end
            10'b1110101001: begin segment = 6'd28; b = 21'b010110010111011010111; end
            10'b1110101010: begin segment = 6'd28; b = 21'b010110010111011100010; end
            10'b1110101011: begin segment = 6'd28; b = 21'b010110010111011101110; end
            10'b1110101100: begin segment = 6'd28; b = 21'b010110010111011111011; end
            10'b1110101101: begin segment = 6'd28; b = 21'b010110010111100001001; end
            10'b1110101110: begin segment = 6'd29; b = 21'b010101011100101000111; end
            10'b1110101111: begin segment = 6'd29; b = 21'b010101011100100110111; end
            10'b1110110000: begin segment = 6'd29; b = 21'b010101011100100101000; end
            10'b1110110001: begin segment = 6'd29; b = 21'b010101011100100011010; end
            10'b1110110010: begin segment = 6'd29; b = 21'b010101011100100001100; end
            10'b1110110011: begin segment = 6'd29; b = 21'b010101011100100000000; end
            10'b1110110100: begin segment = 6'd29; b = 21'b010101011100011110100; end
            10'b1110110101: begin segment = 6'd29; b = 21'b010101011100011101001; end
            10'b1110110110: begin segment = 6'd29; b = 21'b010101011100011100000; end
            10'b1110110111: begin segment = 6'd29; b = 21'b010101011100011010111; end
            10'b1110111000: begin segment = 6'd29; b = 21'b010101011100011001111; end
            10'b1110111001: begin segment = 6'd29; b = 21'b010101011100011001000; end
            10'b1110111010: begin segment = 6'd29; b = 21'b010101011100011000010; end
            10'b1110111011: begin segment = 6'd29; b = 21'b010101011100010111101; end
            10'b1110111100: begin segment = 6'd29; b = 21'b010101011100010111000; end
            10'b1110111101: begin segment = 6'd29; b = 21'b010101011100010110101; end
            10'b1110111110: begin segment = 6'd29; b = 21'b010101011100010110011; end
            10'b1110111111: begin segment = 6'd29; b = 21'b010101011100010110001; end
            10'b1111000000: begin segment = 6'd29; b = 21'b010101011100010110001; end
            10'b1111000001: begin segment = 6'd29; b = 21'b010101011100010110001; end
            10'b1111000010: begin segment = 6'd29; b = 21'b010101011100010110010; end
            10'b1111000011: begin segment = 6'd29; b = 21'b010101011100010110100; end
            10'b1111000100: begin segment = 6'd29; b = 21'b010101011100010110111; end
            10'b1111000101: begin segment = 6'd29; b = 21'b010101011100010111011; end
            10'b1111000110: begin segment = 6'd29; b = 21'b010101011100011000000; end
            10'b1111000111: begin segment = 6'd29; b = 21'b010101011100011000110; end
            10'b1111001000: begin segment = 6'd29; b = 21'b010101011100011001101; end
            10'b1111001001: begin segment = 6'd29; b = 21'b010101011100011010100; end
            10'b1111001010: begin segment = 6'd30; b = 21'b010100111110000110101; end
            10'b1111001011: begin segment = 6'd30; b = 21'b010100111110000101111; end
            10'b1111001100: begin segment = 6'd30; b = 21'b010100111110000101001; end
            10'b1111001101: begin segment = 6'd30; b = 21'b010100111110000100101; end
            10'b1111001110: begin segment = 6'd30; b = 21'b010100111110000100001; end
            10'b1111001111: begin segment = 6'd30; b = 21'b010100111110000011110; end
            10'b1111010000: begin segment = 6'd30; b = 21'b010100111110000011100; end
            10'b1111010001: begin segment = 6'd30; b = 21'b010100111110000011011; end
            10'b1111010010: begin segment = 6'd30; b = 21'b010100111110000011011; end
            10'b1111010011: begin segment = 6'd30; b = 21'b010100111110000011100; end
            10'b1111010100: begin segment = 6'd30; b = 21'b010100111110000011110; end
            10'b1111010101: begin segment = 6'd30; b = 21'b010100111110000100001; end
            10'b1111010110: begin segment = 6'd30; b = 21'b010100111110000100101; end
            10'b1111010111: begin segment = 6'd30; b = 21'b010100111110000101010; end
            10'b1111011000: begin segment = 6'd30; b = 21'b010100111110000101111; end
            10'b1111011001: begin segment = 6'd30; b = 21'b010100111110000110110; end
            10'b1111011010: begin segment = 6'd30; b = 21'b010100111110000111101; end
            10'b1111011011: begin segment = 6'd30; b = 21'b010100111110001000110; end
            10'b1111011100: begin segment = 6'd30; b = 21'b010100111110001001111; end
            10'b1111011101: begin segment = 6'd30; b = 21'b010100111110001011010; end
            10'b1111011110: begin segment = 6'd30; b = 21'b010100111110001100101; end
            10'b1111011111: begin segment = 6'd30; b = 21'b010100111110001110001; end
            10'b1111100000: begin segment = 6'd30; b = 21'b010100111110001111110; end
            10'b1111100001: begin segment = 6'd30; b = 21'b010100111110010001100; end
            10'b1111100010: begin segment = 6'd30; b = 21'b010100111110010011011; end
            10'b1111100011: begin segment = 6'd30; b = 21'b010100111110010101011; end
            10'b1111100100: begin segment = 6'd30; b = 21'b010100111110010111100; end
            10'b1111100101: begin segment = 6'd31; b = 21'b010100000010000000100; end
            10'b1111100110: begin segment = 6'd31; b = 21'b010100000001111110111; end
            10'b1111100111: begin segment = 6'd31; b = 21'b010100000001111101100; end
            10'b1111101000: begin segment = 6'd31; b = 21'b010100000001111100010; end
            10'b1111101001: begin segment = 6'd31; b = 21'b010100000001111011001; end
            10'b1111101010: begin segment = 6'd31; b = 21'b010100000001111010000; end
            10'b1111101011: begin segment = 6'd31; b = 21'b010100000001111001001; end
            10'b1111101100: begin segment = 6'd31; b = 21'b010100000001111000010; end
            10'b1111101101: begin segment = 6'd31; b = 21'b010100000001110111101; end
            10'b1111101110: begin segment = 6'd31; b = 21'b010100000001110111000; end
            10'b1111101111: begin segment = 6'd31; b = 21'b010100000001110110100; end
            10'b1111110000: begin segment = 6'd31; b = 21'b010100000001110110001; end
            10'b1111110001: begin segment = 6'd31; b = 21'b010100000001110110000; end
            10'b1111110010: begin segment = 6'd31; b = 21'b010100000001110101111; end
            10'b1111110011: begin segment = 6'd31; b = 21'b010100000001110101111; end
            10'b1111110100: begin segment = 6'd31; b = 21'b010100000001110110000; end
            10'b1111110101: begin segment = 6'd31; b = 21'b010100000001110110010; end
            10'b1111110110: begin segment = 6'd31; b = 21'b010100000001110110101; end
            10'b1111110111: begin segment = 6'd31; b = 21'b010100000001110111001; end
            10'b1111111000: begin segment = 6'd31; b = 21'b010100000001110111110; end
            10'b1111111001: begin segment = 6'd31; b = 21'b010100000001111000100; end
            10'b1111111010: begin segment = 6'd31; b = 21'b010100000001111001011; end
            10'b1111111011: begin segment = 6'd31; b = 21'b010100000001111010011; end
            10'b1111111100: begin segment = 6'd31; b = 21'b010100000001111011011; end
            10'b1111111101: begin segment = 6'd31; b = 21'b010100000001111100101; end
            10'b1111111110: begin segment = 6'd31; b = 21'b010100000001111110000; end
            10'b1111111111: begin segment = 6'd31; b = 21'b010100000001111111011; end
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


// Auto-generated register declarations for 4 registers
    reg [20:0] A1;
    reg [20:0] A2;
    reg [20:0] A3;
    reg [20:0] A4;
// End of auto-generated section

    always @(*) begin
        case(segment)
            6'd0: begin A1 = {3'b111, neg_18}; A2 = {5'b11111, neg_16}; A3 = {7'b0000000, f[21:8]}; end
            6'd1: begin A1 = {3'b111, neg_18}; A2 = {6'b111111, neg_15}; A3 = {13'b0000000000000, f[21:14]}; end
            6'd2: begin A1 = {3'b111, neg_18}; A2 = {7'b1111111, neg_14}; A3 = {9'b000000000, f[21:10]}; end
            6'd3: begin A1 = {3'b111, neg_18}; A2 = {8'b00000000, f[21:9]}; A3 = {10'b1111111111, neg_11}; end
            6'd4: begin A1 = {3'b111, neg_18}; A2 = {6'b000000, f[21:7]}; A3 = {8'b11111111, neg_13}; end
            6'd5: begin A1 = {3'b111, neg_18}; A2 = {5'b00000, f[21:6]}; A3 = {7'b1111111, neg_14}; end
            6'd6: begin A1 = {3'b111, neg_18}; A2 = {5'b00000, f[21:6]}; A3 = {11'b00000000000, f[21:12]}; end
            6'd7: begin A1 = {3'b111, neg_18}; A2 = {5'b00000, f[21:6]}; A3 = {7'b0000000, f[21:8]}; end
            6'd8: begin A1 = {4'b1111, neg_17}; A2 = {6'b111111, neg_15}; A3 = {8'b00000000, f[21:9]}; end
            6'd9: begin A1 = {4'b1111, neg_17}; A2 = {10'b1111111111, neg_11}; A3 = {14'b00000000000000, f[21:15]}; end
            6'd10: begin A1 = {4'b1111, neg_17}; A2 = {7'b0000000, f[21:8]}; A3 = {9'b000000000, f[21:10]}; end
            6'd11: begin A1 = {4'b1111, neg_17}; A2 = {6'b000000, f[21:7]}; A3 = {8'b00000000, f[21:9]}; end
            6'd12: begin A1 = {5'b11111, neg_16}; A2 = {9'b111111111, neg_12}; A3 = {11'b00000000000, f[21:12]}; end
            6'd13: begin A1 = {5'b11111, neg_16}; A2 = {7'b0000000, f[21:8]}; A3 = {10'b0000000000, f[21:11]}; end
            6'd14: begin A1 = {6'b111111, neg_15}; A2 = {8'b00000000, f[21:9]}; A3 = {12'b000000000000, f[21:13]}; end
            6'd15: begin A1 = {10'b1111111111, neg_11}; A2 = {13'b0000000000000, f[21:14]}; A3 = {15'b000000000000000, f[21:16]}; end
            6'd16: begin A1 = {7'b0000000, f[21:8]}; A2 = {9'b000000000, f[21:10]}; A3 = {11'b00000000000, f[21:12]}; end
            6'd17: begin A1 = {5'b00000, f[21:6]}; A2 = {7'b1111111, neg_14}; A3 = {9'b111111111, neg_12}; end
            6'd18: begin A1 = {5'b00000, f[21:6]}; A2 = {10'b0000000000, f[21:11]}; A3 = {14'b11111111111111, neg_7}; end
            6'd19: begin A1 = {4'b0000, f[21:5]}; A2 = {6'b111111, neg_15}; A3 = {8'b11111111, neg_13}; end
            6'd20: begin A1 = {4'b0000, f[21:5]}; A2 = {7'b1111111, neg_14}; A3 = {13'b0000000000000, f[21:14]}; end
            6'd21: begin A1 = {4'b0000, f[21:5]}; A2 = {8'b00000000, f[21:9]}; A3 = {12'b111111111111, neg_9}; end
            6'd22: begin A1 = {4'b0000, f[21:5]}; A2 = {6'b000000, f[21:7]}; A3 = {11'b11111111111, neg_10}; end
            6'd23: begin A1 = {3'b000, f[21:4]}; A2 = {5'b11111, neg_16}; A3 = {8'b11111111, neg_13}; end
            6'd24: begin A1 = {3'b000, f[21:4]}; A2 = {5'b11111, neg_16}; A3 = {7'b0000000, f[21:8]}; end
            6'd25: begin A1 = {3'b000, f[21:4]}; A2 = {6'b111111, neg_15}; A3 = {8'b00000000, f[21:9]}; end
            6'd26: begin A1 = {3'b000, f[21:4]}; A2 = {12'b000000000000, f[21:13]}; A3 = {14'b11111111111111, neg_7}; end
            6'd27: begin A1 = {3'b000, f[21:4]}; A2 = {6'b000000, f[21:7]}; A3 = {8'b11111111, neg_13}; end
            6'd28: begin A1 = {3'b000, f[21:4]}; A2 = {5'b00000, f[21:6]}; A3 = {7'b1111111, neg_14}; end
            6'd29: begin A1 = {3'b000, f[21:4]}; A2 = {5'b00000, f[21:6]}; A3 = {7'b0000000, f[21:8]}; end
            6'd30: begin A1 = {2'b00, f[21:3]}; A2 = {4'b1111, neg_17}; A3 = {6'b111111, neg_15}; end
            6'd31: begin A1 = {2'b00, f[21:3]}; A2 = {4'b1111, neg_17}; A3 = {11'b11111111111, neg_10}; end
      endcase
    end

// Auto-generated CSA tree for final_N=4, final_M=22, final_add_M=20
    wire [20:0] csa1_carry, csa1_sum;
    wire [20:0] csa2_carry, csa2_sum;
    wire [20:0] csa3_carry, csa3_sum;
    wire [20:0] final_sum;

    CSA_anticonv4 csa1 (
        .a({1'b0, f[21:2]}),  // f的高20位
        .b(A1),
        .c(A2),
        .sum(csa1_sum),
        .carry(csa1_carry)
    );

    CSA_anticonv4 csa2 (
        .a(csa1_sum),
        .b(A3),
        .c({csa1_carry[19:0], 1'b0}),  // 左移1位
        .sum(csa2_sum),
        .carry(csa2_carry)
    );

    CSA_anticonv4 csa3 (
        .a(csa2_sum),
         .b(b),
        .c({csa2_carry[19:0], 1'b0}),
        .sum(csa3_sum),
        .carry(csa3_carry)
    );

    CPA_anticonv4 cpa (
        .a(csa3_sum),
        .b({csa3_carry[19:0], 1'b0}),  // 左移1位
        .sum(final_sum)
    );

    assign f_e2_out = {final_sum, f[1:0]};  // 拼接高位和原始低位
// End of auto-generated CSA tree

endmodule
module CSA_anticonv4 #(parameter ADD_WIDTH = 21
)(
    input [ADD_WIDTH-1:0] a,
    input [ADD_WIDTH-1:0] b,
    input [ADD_WIDTH-1:0] c,
    output [ADD_WIDTH-1:0] sum,
    output [ADD_WIDTH-1:0] carry
);
    assign sum = a ^ b ^ c;          // XOR for sum
    assign carry = (a & b) | (b & c) | (c & a); // Majority logic for carry
endmodule

// Carry-Propagate Adder (CPA) module
module CPA_anticonv4 #(parameter ADD_WIDTH = 21
)(
    input [ADD_WIDTH-1:0] a,
    input [ADD_WIDTH-1:0] b,
    output [ADD_WIDTH-1:0] sum
);
    assign sum = a + b;  // Simple binary addition
endmodule

module Shift_operation32(
    input [7:0] shift,    // 6-bit signed shift signal
    input [23:0] e,       // 24-bit signed input signal
    output [31:0] result     // 32-bit output result
);
    reg [31:0] result0;
    reg sel;
    always @(*) begin
        case(shift)
            8'b00000000: begin result0 = {8'b00000000, e}; sel = 1'b0; end
            8'b00000001: begin result0 = {7'b0000000, e, 1'b0}; sel = 1'b0; end
            8'b00000010: begin result0 = {6'b000000, e, 2'b00};sel = 1'b0; end
            8'b00000011: begin result0 = {5'b00000, e, 3'b000};sel = 1'b0; end
            8'b00000100: begin result0 = {4'b0000, e, 4'b0000};sel = 1'b0; end
            8'b00000101: begin result0 = {3'b000, e, 5'b00000};sel = 1'b0; end
            8'b00000110: begin result0 = {2'b00, e, 6'b000000};sel = 1'b0; end
            8'b00000111: begin result0 = {1'b0, e, 7'b0000000};sel = 1'b0; end
            8'b00001000: begin result0 = {e, 8'b00000000}; sel = 1'b0; end
            8'b00001001: begin result0 = {e[22:0], 9'b000000000}; sel = 1'b0; end
            8'b00001010: begin result0 = {e[21:0], 10'b0000000000}; sel = 1'b0; end
            8'b00001011: begin result0 = {e[20:0], 11'b00000000000}; sel = 1'b0; end
            8'b00001100: begin result0 = {e[19:0], 12'b000000000000}; sel = 1'b0; end
            8'b00001101: begin result0 = {e[18:0], 13'b0000000000000}; sel = 1'b0; end
            8'b00001110: begin result0 = {e[17:0], 14'b00000000000000}; sel = 1'b0; end
            8'b00001111: begin result0 = {e[16:0], 15'b000000000000000}; sel = 1'b0; end
            8'b00010000: begin result0 = {e[15:0], 16'b0000000000000000}; sel = 1'b0; end
            8'b00010001: begin result0 = {e[14:0], 17'b00000000000000000}; sel = 1'b0; end
            8'b00010010: begin result0 = {e[13:0], 18'b000000000000000000}; sel = 1'b0; end
            8'b00010011: begin result0 = {e[12:0], 19'b0000000000000000000}; sel = 1'b0; end
            8'b00010100: begin result0 = {e[11:0], 20'b00000000000000000000}; sel = 1'b0; end
            8'b00010101: begin result0 = {e[10:0], 21'b000000000000000000000}; sel = 1'b0; end
            8'b00010110: begin result0 = {e[9:0], 22'b0000000000000000000000}; sel = 1'b0; end
            8'b00010111: begin result0 = {e[8:0], 23'b00000000000000000000000}; sel = 1'b0; end
            8'b00011000: begin result0 = {e[7:0], 24'b000000000000000000000000}; sel = 1'b0; end
            8'b00011001: begin result0 = {e[6:0], 25'b0000000000000000000000000}; sel = 1'b0; end
            8'b00011010: begin result0 = {e[5:0], 26'b00000000000000000000000000}; sel = 1'b0; end
            8'b00011011: begin result0 = {e[4:0], 27'b000000000000000000000000000}; sel = 1'b0; end
            8'b00011100: begin result0 = {e[3:0], 28'b0000000000000000000000000000}; sel = 1'b0; end
            8'b00011101: begin result0 = {e[2:0], 29'b00000000000000000000000000000}; sel = 1'b0; end
            8'b00011110: begin result0 = {e[1:0], 30'b000000000000000000000000000000}; sel = 1'b0; end
            8'b00011111: begin result0 = {e[0], 31'b0000000000000000000000000000000}; sel = 1'b0; end
            8'b11111111: begin result0 = {9'b000000000, e[23:1]};  sel = e[0]; end
            8'b11111110: begin result0 = {10'b0000000000, e[23:2]}; sel = e[1]; end
            8'b11111101: begin result0 = {11'b00000000000, e[23:3]}; sel = e[2]; end
            8'b11111100: begin result0 = {12'b000000000000, e[23:4]}; sel = e[3]; end
            8'b11111011: begin result0 = {13'b0000000000000, e[23:5]}; sel = e[4]; end
            8'b11111010: begin result0 = {14'b00000000000000, e[23:6]}; sel = e[5]; end
            8'b11111001: begin result0 = {15'b000000000000000, e[23:7]}; sel = e[6]; end
            8'b11111000: begin result0 = {16'b0000000000000000, e[23:8]}; sel = e[7]; end
            8'b11110111: begin result0 = {17'b00000000000000000, e[23:9]}; sel = e[8]; end
            8'b11110110: begin result0 = {18'b000000000000000000, e[23:10]}; sel = e[9]; end
            8'b11110101: begin result0 = {19'b0000000000000000000, e[23:11]}; sel = e[10]; end
            8'b11110100: begin result0 = {20'b00000000000000000000, e[23:12]}; sel = e[11]; end
            8'b11110011: begin result0 = {21'b000000000000000000000, e[23:13]}; sel = e[12]; end
            8'b11110010: begin result0 = {22'b0000000000000000000000, e[23:14]}; sel = e[13]; end
            8'b11110001: begin result0 = {23'b00000000000000000000000, e[23:15]}; sel = e[14]; end
            8'b11110000: begin result0 = {24'b000000000000000000000000, e[23:16]}; sel = e[15]; end
            8'b11101111: begin result0 = {25'b0000000000000000000000000, e[23:17]}; sel = e[16]; end
            8'b11101110: begin result0 = {26'b00000000000000000000000000, e[23:18]}; sel = e[17]; end
            8'b11101101: begin result0 = {27'b000000000000000000000000000, e[23:19]}; sel = e[18]; end
            8'b11101100: begin result0 = {28'b0000000000000000000000000000, e[23:20]}; sel = e[19]; end
            8'b11101011: begin result0 = {29'b00000000000000000000000000000, e[23:21]}; sel = e[20]; end
            8'b11101010: begin result0 = {30'b00000000000000000000000000000, e[23:22]}; sel = e[21]; end
            8'b11101001: begin result0 = {31'b000000000000000000000000000000, e[23]}; sel = e[22]; end
            default : result0 = 32'd0;
        endcase
    end
    assign result = result0 + sel;
endmodule

module new_Shift_operation32(
    input [7:0] shift,    // 6-bit signed shift signal
    input [23:0] e,       // 24-bit signed input signal
    output [31:0] result     // 32-bit output result
);
    reg [31:0] result0;
    reg sel;
    always @(*) begin
        case(shift)
            8'b00010111: begin result0 = {8'b00000000, e}; sel = 1'b0; end
            8'b00011000: begin result0 = {7'b0000000, e, 1'b0}; sel = 1'b0; end
            8'b00011001: begin result0 = {6'b000000, e, 2'b00};sel = 1'b0; end
            8'b00011010: begin result0 = {5'b00000, e, 3'b000};sel = 1'b0; end
            8'b00011011: begin result0 = {4'b0000, e, 4'b0000};sel = 1'b0; end
            8'b00011100: begin result0 = {3'b000, e, 5'b00000};sel = 1'b0; end
            8'b00011101: begin result0 = {2'b00, e, 6'b000000};sel = 1'b0; end
            8'b00011110: begin result0 = {1'b0, e, 7'b0000000};sel = 1'b0; end
            8'b00011111: begin result0 = {e, 8'b00000000}; sel = 1'b0; end
            8'b00100000: begin result0 = {e[22:0], 9'b000000000}; sel = 1'b0; end
            8'b00100001: begin result0 = {e[21:0], 10'b0000000000}; sel = 1'b0; end
            8'b00100010: begin result0 = {e[20:0], 11'b00000000000}; sel = 1'b0; end
            8'b00100011: begin result0 = {e[19:0], 12'b000000000000}; sel = 1'b0; end
            8'b00100100: begin result0 = {e[18:0], 13'b0000000000000}; sel = 1'b0; end
            8'b00100101: begin result0 = {e[17:0], 14'b00000000000000}; sel = 1'b0; end
            8'b00100110: begin result0 = {e[16:0], 15'b000000000000000}; sel = 1'b0; end
            8'b00100111: begin result0 = {e[15:0], 16'b0000000000000000}; sel = 1'b0; end
            8'b00101000: begin result0 = {e[14:0], 17'b00000000000000000}; sel = 1'b0; end
            8'b00101001: begin result0 = {e[13:0], 18'b000000000000000000}; sel = 1'b0; end
            8'b00101010: begin result0 = {e[12:0], 19'b0000000000000000000}; sel = 1'b0; end
            8'b00101011: begin result0 = {e[11:0], 20'b00000000000000000000}; sel = 1'b0; end
            8'b00101100: begin result0 = {e[10:0], 21'b000000000000000000000}; sel = 1'b0; end
            8'b00101101: begin result0 = {e[9:0], 22'b0000000000000000000000}; sel = 1'b0; end
            8'b00101110: begin result0 = {e[8:0], 23'b00000000000000000000000}; sel = 1'b0; end
            8'b00101111: begin result0 = {e[7:0], 24'b000000000000000000000000}; sel = 1'b0; end
            8'b00110000: begin result0 = {e[6:0], 25'b0000000000000000000000000}; sel = 1'b0; end
            8'b00110001: begin result0 = {e[5:0], 26'b00000000000000000000000000}; sel = 1'b0; end
            8'b00110010: begin result0 = {e[4:0], 27'b000000000000000000000000000}; sel = 1'b0; end
            8'b00110011: begin result0 = {e[3:0], 28'b0000000000000000000000000000}; sel = 1'b0; end
            8'b00110100: begin result0 = {e[2:0], 29'b00000000000000000000000000000}; sel = 1'b0; end
            8'b00110101: begin result0 = {e[1:0], 30'b000000000000000000000000000000}; sel = 1'b0; end
            8'b00110110: begin result0 = {e[0], 31'b0000000000000000000000000000000}; sel = 1'b0; end
            8'b00010110: begin result0 = {9'b000000000, e[23:1]};  sel = e[0]; end
            8'b00010101: begin result0 = {10'b0000000000, e[23:2]}; sel = e[1]; end
            8'b00010100: begin result0 = {11'b00000000000, e[23:3]}; sel = e[2]; end
            8'b00010011: begin result0 = {12'b000000000000, e[23:4]}; sel = e[3]; end
            8'b00010010: begin result0 = {13'b0000000000000, e[23:5]}; sel = e[4]; end
            8'b00010001: begin result0 = {14'b00000000000000, e[23:6]}; sel = e[5]; end
            8'b00010000: begin result0 = {15'b000000000000000, e[23:7]}; sel = e[6]; end
            8'b00001111: begin result0 = {16'b0000000000000000, e[23:8]}; sel = e[7]; end
            8'b00001110: begin result0 = {17'b00000000000000000, e[23:9]}; sel = e[8]; end
            8'b00001101: begin result0 = {18'b000000000000000000, e[23:10]}; sel = e[9]; end
            8'b00001100: begin result0 = {19'b0000000000000000000, e[23:11]}; sel = e[10]; end
            8'b00001011: begin result0 = {20'b00000000000000000000, e[23:12]}; sel = e[11]; end
            8'b00001010: begin result0 = {21'b000000000000000000000, e[23:13]}; sel = e[12]; end
            8'b00001001: begin result0 = {22'b0000000000000000000000, e[23:14]}; sel = e[13]; end
            8'b00001000: begin result0 = {23'b00000000000000000000000, e[23:15]}; sel = e[14]; end
            8'b00000111: begin result0 = {24'b000000000000000000000000, e[23:16]}; sel = e[15]; end
            8'b00000110: begin result0 = {25'b0000000000000000000000000, e[23:17]}; sel = e[16]; end
            8'b00000101: begin result0 = {26'b00000000000000000000000000, e[23:18]}; sel = e[17]; end
            8'b00000100: begin result0 = {27'b000000000000000000000000000, e[23:19]}; sel = e[18]; end
            8'b00000011: begin result0 = {28'b0000000000000000000000000000, e[23:20]}; sel = e[19]; end
            8'b00000010: begin result0 = {29'b00000000000000000000000000000, e[23:21]}; sel = e[20]; end
            8'b00000001: begin result0 = {30'b00000000000000000000000000000, e[23:22]}; sel = e[21]; end
            8'b00000000: begin result0 = {31'b000000000000000000000000000000, e[23]}; sel = e[22]; end
            default : result0 = 32'd0;
        endcase
    end
    assign result = result0 + sel;
endmodule

module  APP_22_17_8_4_28_anticonv1(
    input [21:0] f,               // 22-bit input
    output [22:0] f_e2_out     // 23-bit output 
);
    reg [4:0] segment;
    reg [17:0] b;
    always @(*) begin
        case(f[21:14])
            8'b00000000: begin segment = 5'd0; b = 18'b011111111111111111; end
            8'b00000001: begin segment = 5'd0; b = 18'b011111111111111011; end
            8'b00000010: begin segment = 5'd0; b = 18'b011111111111110111; end
            8'b00000011: begin segment = 5'd0; b = 18'b011111111111110101; end
            8'b00000100: begin segment = 5'd0; b = 18'b011111111111110100; end
            8'b00000101: begin segment = 5'd0; b = 18'b011111111111110100; end
            8'b00000110: begin segment = 5'd0; b = 18'b011111111111110100; end
            8'b00000111: begin segment = 5'd0; b = 18'b011111111111110110; end
            8'b00001000: begin segment = 5'd0; b = 18'b011111111111111001; end
            8'b00001001: begin segment = 5'd0; b = 18'b011111111111111100; end
            8'b00001010: begin segment = 5'd1; b = 18'b011111111110011000; end
            8'b00001011: begin segment = 5'd1; b = 18'b011111111110010100; end
            8'b00001100: begin segment = 5'd1; b = 18'b011111111110010000; end
            8'b00001101: begin segment = 5'd1; b = 18'b011111111110001110; end
            8'b00001110: begin segment = 5'd1; b = 18'b011111111110001100; end
            8'b00001111: begin segment = 5'd1; b = 18'b011111111110001100; end
            8'b00010000: begin segment = 5'd1; b = 18'b011111111110001101; end
            8'b00010001: begin segment = 5'd1; b = 18'b011111111110001110; end
            8'b00010010: begin segment = 5'd1; b = 18'b011111111110010001; end
            8'b00010011: begin segment = 5'd1; b = 18'b011111111110010100; end
            8'b00010100: begin segment = 5'd1; b = 18'b011111111110011001; end
            8'b00010101: begin segment = 5'd2; b = 18'b011111111010110010; end
            8'b00010110: begin segment = 5'd2; b = 18'b011111111010101110; end
            8'b00010111: begin segment = 5'd2; b = 18'b011111111010101011; end
            8'b00011000: begin segment = 5'd2; b = 18'b011111111010101000; end
            8'b00011001: begin segment = 5'd2; b = 18'b011111111010100111; end
            8'b00011010: begin segment = 5'd2; b = 18'b011111111010100111; end
            8'b00011011: begin segment = 5'd2; b = 18'b011111111010101000; end
            8'b00011100: begin segment = 5'd2; b = 18'b011111111010101001; end
            8'b00011101: begin segment = 5'd2; b = 18'b011111111010101100; end
            8'b00011110: begin segment = 5'd2; b = 18'b011111111010110000; end
            8'b00011111: begin segment = 5'd3; b = 18'b011111110101111010; end
            8'b00100000: begin segment = 5'd3; b = 18'b011111110101110110; end
            8'b00100001: begin segment = 5'd3; b = 18'b011111110101110011; end
            8'b00100010: begin segment = 5'd3; b = 18'b011111110101110001; end
            8'b00100011: begin segment = 5'd3; b = 18'b011111110101110001; end
            8'b00100100: begin segment = 5'd3; b = 18'b011111110101110001; end
            8'b00100101: begin segment = 5'd3; b = 18'b011111110101110010; end
            8'b00100110: begin segment = 5'd3; b = 18'b011111110101110101; end
            8'b00100111: begin segment = 5'd3; b = 18'b011111110101111000; end
            8'b00101000: begin segment = 5'd3; b = 18'b011111110101111101; end
            8'b00101001: begin segment = 5'd4; b = 18'b011111101110111001; end
            8'b00101010: begin segment = 5'd4; b = 18'b011111101110110101; end
            8'b00101011: begin segment = 5'd4; b = 18'b011111101110110010; end
            8'b00101100: begin segment = 5'd4; b = 18'b011111101110101111; end
            8'b00101101: begin segment = 5'd4; b = 18'b011111101110101110; end
            8'b00101110: begin segment = 5'd4; b = 18'b011111101110101110; end
            8'b00101111: begin segment = 5'd4; b = 18'b011111101110110000; end
            8'b00110000: begin segment = 5'd4; b = 18'b011111101110110010; end
            8'b00110001: begin segment = 5'd4; b = 18'b011111101110110101; end
            8'b00110010: begin segment = 5'd4; b = 18'b011111101110111001; end
            8'b00110011: begin segment = 5'd5; b = 18'b011111100110111100; end
            8'b00110100: begin segment = 5'd5; b = 18'b011111100110111000; end
            8'b00110101: begin segment = 5'd5; b = 18'b011111100110110110; end
            8'b00110110: begin segment = 5'd5; b = 18'b011111100110110101; end
            8'b00110111: begin segment = 5'd5; b = 18'b011111100110110100; end
            8'b00111000: begin segment = 5'd5; b = 18'b011111100110110101; end
            8'b00111001: begin segment = 5'd5; b = 18'b011111100110111000; end
            8'b00111010: begin segment = 5'd5; b = 18'b011111100110111011; end
            8'b00111011: begin segment = 5'd5; b = 18'b011111100110111111; end
            8'b00111100: begin segment = 5'd5; b = 18'b011111100111000101; end
            8'b00111101: begin segment = 5'd6; b = 18'b011111011011101001; end
            8'b00111110: begin segment = 5'd6; b = 18'b011111011011100101; end
            8'b00111111: begin segment = 5'd6; b = 18'b011111011011100010; end
            8'b01000000: begin segment = 5'd6; b = 18'b011111011011100000; end
            8'b01000001: begin segment = 5'd6; b = 18'b011111011011011111; end
            8'b01000010: begin segment = 5'd6; b = 18'b011111011011100000; end
            8'b01000011: begin segment = 5'd6; b = 18'b011111011011100001; end
            8'b01000100: begin segment = 5'd6; b = 18'b011111011011100100; end
            8'b01000101: begin segment = 5'd6; b = 18'b011111011011100111; end
            8'b01000110: begin segment = 5'd6; b = 18'b011111011011101100; end
            8'b01000111: begin segment = 5'd6; b = 18'b011111011011110011; end
            8'b01001000: begin segment = 5'd7; b = 18'b011111001110010100; end
            8'b01001001: begin segment = 5'd7; b = 18'b011111001110010000; end
            8'b01001010: begin segment = 5'd7; b = 18'b011111001110001110; end
            8'b01001011: begin segment = 5'd7; b = 18'b011111001110001101; end
            8'b01001100: begin segment = 5'd7; b = 18'b011111001110001101; end
            8'b01001101: begin segment = 5'd7; b = 18'b011111001110001110; end
            8'b01001110: begin segment = 5'd7; b = 18'b011111001110010000; end
            8'b01001111: begin segment = 5'd7; b = 18'b011111001110010100; end
            8'b01010000: begin segment = 5'd7; b = 18'b011111001110011000; end
            8'b01010001: begin segment = 5'd7; b = 18'b011111001110011110; end
            8'b01010010: begin segment = 5'd8; b = 18'b011110111101100000; end
            8'b01010011: begin segment = 5'd8; b = 18'b011110111101011011; end
            8'b01010100: begin segment = 5'd8; b = 18'b011110111101010111; end
            8'b01010101: begin segment = 5'd8; b = 18'b011110111101010101; end
            8'b01010110: begin segment = 5'd8; b = 18'b011110111101010100; end
            8'b01010111: begin segment = 5'd8; b = 18'b011110111101010100; end
            8'b01011000: begin segment = 5'd8; b = 18'b011110111101010101; end
            8'b01011001: begin segment = 5'd8; b = 18'b011110111101010111; end
            8'b01011010: begin segment = 5'd8; b = 18'b011110111101011011; end
            8'b01011011: begin segment = 5'd8; b = 18'b011110111101100000; end
            8'b01011100: begin segment = 5'd9; b = 18'b011110101110000011; end
            8'b01011101: begin segment = 5'd9; b = 18'b011110101110000000; end
            8'b01011110: begin segment = 5'd9; b = 18'b011110101101111110; end
            8'b01011111: begin segment = 5'd9; b = 18'b011110101101111101; end
            8'b01100000: begin segment = 5'd9; b = 18'b011110101101111101; end
            8'b01100001: begin segment = 5'd9; b = 18'b011110101101111110; end
            8'b01100010: begin segment = 5'd9; b = 18'b011110101110000001; end
            8'b01100011: begin segment = 5'd9; b = 18'b011110101110000101; end
            8'b01100100: begin segment = 5'd9; b = 18'b011110101110001010; end
            8'b01100101: begin segment = 5'd10; b = 18'b011110011010110101; end
            8'b01100110: begin segment = 5'd10; b = 18'b011110011010110001; end
            8'b01100111: begin segment = 5'd10; b = 18'b011110011010101110; end
            8'b01101000: begin segment = 5'd10; b = 18'b011110011010101011; end
            8'b01101001: begin segment = 5'd10; b = 18'b011110011010101011; end
            8'b01101010: begin segment = 5'd10; b = 18'b011110011010101100; end
            8'b01101011: begin segment = 5'd10; b = 18'b011110011010101110; end
            8'b01101100: begin segment = 5'd10; b = 18'b011110011010110001; end
            8'b01101101: begin segment = 5'd10; b = 18'b011110011010110101; end
            8'b01101110: begin segment = 5'd11; b = 18'b011110000110110110; end
            8'b01101111: begin segment = 5'd11; b = 18'b011110000110110001; end
            8'b01110000: begin segment = 5'd11; b = 18'b011110000110101111; end
            8'b01110001: begin segment = 5'd11; b = 18'b011110000110101101; end
            8'b01110010: begin segment = 5'd11; b = 18'b011110000110101100; end
            8'b01110011: begin segment = 5'd11; b = 18'b011110000110101101; end
            8'b01110100: begin segment = 5'd11; b = 18'b011110000110101111; end
            8'b01110101: begin segment = 5'd11; b = 18'b011110000110110010; end
            8'b01110110: begin segment = 5'd11; b = 18'b011110000110110110; end
            8'b01110111: begin segment = 5'd12; b = 18'b011101110000101010; end
            8'b01111000: begin segment = 5'd12; b = 18'b011101110000100110; end
            8'b01111001: begin segment = 5'd12; b = 18'b011101110000100010; end
            8'b01111010: begin segment = 5'd12; b = 18'b011101110000100000; end
            8'b01111011: begin segment = 5'd12; b = 18'b011101110000100000; end
            8'b01111100: begin segment = 5'd12; b = 18'b011101110000100001; end
            8'b01111101: begin segment = 5'd12; b = 18'b011101110000100011; end
            8'b01111110: begin segment = 5'd12; b = 18'b011101110000100110; end
            8'b01111111: begin segment = 5'd12; b = 18'b011101110000101010; end
            8'b10000000: begin segment = 5'd13; b = 18'b011101010110110010; end
            8'b10000001: begin segment = 5'd13; b = 18'b011101010110101100; end
            8'b10000010: begin segment = 5'd13; b = 18'b011101010110101000; end
            8'b10000011: begin segment = 5'd13; b = 18'b011101010110100101; end
            8'b10000100: begin segment = 5'd13; b = 18'b011101010110100100; end
            8'b10000101: begin segment = 5'd13; b = 18'b011101010110100100; end
            8'b10000110: begin segment = 5'd13; b = 18'b011101010110100101; end
            8'b10000111: begin segment = 5'd13; b = 18'b011101010110101000; end
            8'b10001000: begin segment = 5'd13; b = 18'b011101010110101101; end
            8'b10001001: begin segment = 5'd13; b = 18'b011101010110110010; end
            8'b10001010: begin segment = 5'd14; b = 18'b011100111010001100; end
            8'b10001011: begin segment = 5'd14; b = 18'b011100111010000111; end
            8'b10001100: begin segment = 5'd14; b = 18'b011100111010000100; end
            8'b10001101: begin segment = 5'd14; b = 18'b011100111010000010; end
            8'b10001110: begin segment = 5'd14; b = 18'b011100111010000010; end
            8'b10001111: begin segment = 5'd14; b = 18'b011100111010000010; end
            8'b10010000: begin segment = 5'd14; b = 18'b011100111010000100; end
            8'b10010001: begin segment = 5'd14; b = 18'b011100111010001000; end
            8'b10010010: begin segment = 5'd14; b = 18'b011100111010001101; end
            8'b10010011: begin segment = 5'd15; b = 18'b011100011100010110; end
            8'b10010100: begin segment = 5'd15; b = 18'b011100011100010001; end
            8'b10010101: begin segment = 5'd15; b = 18'b011100011100001101; end
            8'b10010110: begin segment = 5'd15; b = 18'b011100011100001011; end
            8'b10010111: begin segment = 5'd15; b = 18'b011100011100001010; end
            8'b10011000: begin segment = 5'd15; b = 18'b011100011100001011; end
            8'b10011001: begin segment = 5'd15; b = 18'b011100011100001101; end
            8'b10011010: begin segment = 5'd15; b = 18'b011100011100010001; end
            8'b10011011: begin segment = 5'd15; b = 18'b011100011100010110; end
            8'b10011100: begin segment = 5'd16; b = 18'b011011111100011111; end
            8'b10011101: begin segment = 5'd16; b = 18'b011011111100011010; end
            8'b10011110: begin segment = 5'd16; b = 18'b011011111100010111; end
            8'b10011111: begin segment = 5'd16; b = 18'b011011111100010100; end
            8'b10100000: begin segment = 5'd16; b = 18'b011011111100010011; end
            8'b10100001: begin segment = 5'd16; b = 18'b011011111100010100; end
            8'b10100010: begin segment = 5'd16; b = 18'b011011111100010110; end
            8'b10100011: begin segment = 5'd16; b = 18'b011011111100011010; end
            8'b10100100: begin segment = 5'd16; b = 18'b011011111100011111; end
            8'b10100101: begin segment = 5'd17; b = 18'b011011011000100011; end
            8'b10100110: begin segment = 5'd17; b = 18'b011011011000011110; end
            8'b10100111: begin segment = 5'd17; b = 18'b011011011000011010; end
            8'b10101000: begin segment = 5'd17; b = 18'b011011011000010111; end
            8'b10101001: begin segment = 5'd17; b = 18'b011011011000010110; end
            8'b10101010: begin segment = 5'd17; b = 18'b011011011000010110; end
            8'b10101011: begin segment = 5'd17; b = 18'b011011011000011000; end
            8'b10101100: begin segment = 5'd17; b = 18'b011011011000011011; end
            8'b10101101: begin segment = 5'd17; b = 18'b011011011000100000; end
            8'b10101110: begin segment = 5'd18; b = 18'b011010110110100001; end
            8'b10101111: begin segment = 5'd18; b = 18'b011010110110011101; end
            8'b10110000: begin segment = 5'd18; b = 18'b011010110110011010; end
            8'b10110001: begin segment = 5'd18; b = 18'b011010110110011001; end
            8'b10110010: begin segment = 5'd18; b = 18'b011010110110011000; end
            8'b10110011: begin segment = 5'd18; b = 18'b011010110110011011; end
            8'b10110100: begin segment = 5'd18; b = 18'b011010110110011110; end
            8'b10110101: begin segment = 5'd18; b = 18'b011010110110100011; end
            8'b10110110: begin segment = 5'd19; b = 18'b011010010000001001; end
            8'b10110111: begin segment = 5'd19; b = 18'b011010010000000100; end
            8'b10111000: begin segment = 5'd19; b = 18'b011010010000000000; end
            8'b10111001: begin segment = 5'd19; b = 18'b011010001111111110; end
            8'b10111010: begin segment = 5'd19; b = 18'b011010001111111101; end
            8'b10111011: begin segment = 5'd19; b = 18'b011010001111111101; end
            8'b10111100: begin segment = 5'd19; b = 18'b011010010000000000; end
            8'b10111101: begin segment = 5'd19; b = 18'b011010010000000100; end
            8'b10111110: begin segment = 5'd20; b = 18'b011001101100011100; end
            8'b10111111: begin segment = 5'd20; b = 18'b011001101100010111; end
            8'b11000000: begin segment = 5'd20; b = 18'b011001101100010100; end
            8'b11000001: begin segment = 5'd20; b = 18'b011001101100010010; end
            8'b11000010: begin segment = 5'd20; b = 18'b011001101100010010; end
            8'b11000011: begin segment = 5'd20; b = 18'b011001101100010100; end
            8'b11000100: begin segment = 5'd20; b = 18'b011001101100010111; end
            8'b11000101: begin segment = 5'd20; b = 18'b011001101100011100; end
            8'b11000110: begin segment = 5'd20; b = 18'b011001101100100011; end
            8'b11000111: begin segment = 5'd21; b = 18'b011000111010110011; end
            8'b11001000: begin segment = 5'd21; b = 18'b011000111010101101; end
            8'b11001001: begin segment = 5'd21; b = 18'b011000111010101001; end
            8'b11001010: begin segment = 5'd21; b = 18'b011000111010100110; end
            8'b11001011: begin segment = 5'd21; b = 18'b011000111010100101; end
            8'b11001100: begin segment = 5'd21; b = 18'b011000111010100110; end
            8'b11001101: begin segment = 5'd21; b = 18'b011000111010101000; end
            8'b11001110: begin segment = 5'd21; b = 18'b011000111010101100; end
            8'b11001111: begin segment = 5'd22; b = 18'b011000010011110111; end
            8'b11010000: begin segment = 5'd22; b = 18'b011000010011110010; end
            8'b11010001: begin segment = 5'd22; b = 18'b011000010011101111; end
            8'b11010010: begin segment = 5'd22; b = 18'b011000010011101110; end
            8'b11010011: begin segment = 5'd22; b = 18'b011000010011101111; end
            8'b11010100: begin segment = 5'd22; b = 18'b011000010011110001; end
            8'b11010101: begin segment = 5'd22; b = 18'b011000010011110100; end
            8'b11010110: begin segment = 5'd22; b = 18'b011000010011111010; end
            8'b11010111: begin segment = 5'd23; b = 18'b010111100100011101; end
            8'b11011000: begin segment = 5'd23; b = 18'b010111100100010111; end
            8'b11011001: begin segment = 5'd23; b = 18'b010111100100010100; end
            8'b11011010: begin segment = 5'd23; b = 18'b010111100100010010; end
            8'b11011011: begin segment = 5'd23; b = 18'b010111100100010011; end
            8'b11011100: begin segment = 5'd23; b = 18'b010111100100010101; end
            8'b11011101: begin segment = 5'd23; b = 18'b010111100100011000; end
            8'b11011110: begin segment = 5'd23; b = 18'b010111100100011101; end
            8'b11011111: begin segment = 5'd24; b = 18'b010110110011001111; end
            8'b11100000: begin segment = 5'd24; b = 18'b010110110011001001; end
            8'b11100001: begin segment = 5'd24; b = 18'b010110110011000110; end
            8'b11100010: begin segment = 5'd24; b = 18'b010110110011000100; end
            8'b11100011: begin segment = 5'd24; b = 18'b010110110011000101; end
            8'b11100100: begin segment = 5'd24; b = 18'b010110110011000110; end
            8'b11100101: begin segment = 5'd24; b = 18'b010110110011001001; end
            8'b11100110: begin segment = 5'd24; b = 18'b010110110011001111; end
            8'b11100111: begin segment = 5'd25; b = 18'b010110000001100111; end
            8'b11101000: begin segment = 5'd25; b = 18'b010110000001100010; end
            8'b11101001: begin segment = 5'd25; b = 18'b010110000001011111; end
            8'b11101010: begin segment = 5'd25; b = 18'b010110000001011110; end
            8'b11101011: begin segment = 5'd25; b = 18'b010110000001011110; end
            8'b11101100: begin segment = 5'd25; b = 18'b010110000001100000; end
            8'b11101101: begin segment = 5'd25; b = 18'b010110000001100101; end
            8'b11101110: begin segment = 5'd25; b = 18'b010110000001101011; end
            8'b11101111: begin segment = 5'd26; b = 18'b010100111110011100; end
            8'b11110000: begin segment = 5'd26; b = 18'b010100111110010011; end
            8'b11110001: begin segment = 5'd26; b = 18'b010100111110001101; end
            8'b11110010: begin segment = 5'd26; b = 18'b010100111110001000; end
            8'b11110011: begin segment = 5'd26; b = 18'b010100111110000101; end
            8'b11110100: begin segment = 5'd26; b = 18'b010100111110000100; end
            8'b11110101: begin segment = 5'd26; b = 18'b010100111110000101; end
            8'b11110110: begin segment = 5'd26; b = 18'b010100111110001000; end
            8'b11110111: begin segment = 5'd27; b = 18'b010100001111110011; end
            8'b11111000: begin segment = 5'd27; b = 18'b010100001111101110; end
            8'b11111001: begin segment = 5'd27; b = 18'b010100001111101010; end
            8'b11111010: begin segment = 5'd27; b = 18'b010100001111101000; end
            8'b11111011: begin segment = 5'd27; b = 18'b010100001111101000; end
            8'b11111100: begin segment = 5'd27; b = 18'b010100001111101011; end
            8'b11111101: begin segment = 5'd27; b = 18'b010100001111101111; end
            8'b11111110: begin segment = 5'd27; b = 18'b010100001111110101; end
            8'b11111111: begin segment = 5'd27; b = 18'b010100001111111101; end
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


// Auto-generated register declarations for 4 registers
    reg [17:0] A1;
    reg [17:0] A2;
    reg [17:0] A3;
    reg [17:0] A4;
// End of auto-generated section

    always @(*) begin
        case(segment)
            5'd0: begin A1 = {3'b111, neg_15}; A2 = {5'b11111, neg_13}; A3 = {7'b0000000, f[21:11]}; end
            5'd1: begin A1 = {3'b111, neg_15}; A2 = {6'b111111, neg_12}; A3 = {9'b000000000, f[21:13]}; end
            5'd2: begin A1 = {3'b111, neg_15}; A2 = {8'b11111111, neg_10}; A3 = {10'b0000000000, f[21:14]}; end
            5'd3: begin A1 = {3'b111, neg_15}; A2 = {7'b0000000, f[21:11]}; A3 = {10'b1111111111, neg_8}; end
            5'd4: begin A1 = {3'b111, neg_15}; A2 = {6'b000000, f[21:10]}; A3 = {9'b000000000, f[21:13]}; end
            5'd5: begin A1 = {3'b111, neg_15}; A2 = {5'b00000, f[21:9]}; A3 = {8'b11111111, neg_10}; end
            5'd6: begin A1 = {3'b111, neg_15}; A2 = {5'b00000, f[21:9]}; A3 = {7'b0000000, f[21:11]}; end
            5'd7: begin A1 = {4'b1111, neg_14}; A2 = {6'b111111, neg_12}; A3 = {8'b00000000, f[21:12]}; end
            5'd8: begin A1 = {4'b1111, neg_14}; A2 = {10'b0000000000, f[21:14]}; A3 = {12'b000000000000, f[21:16]}; end
            5'd9: begin A1 = {4'b1111, neg_14}; A2 = {6'b000000, f[21:10]}; A3 = {8'b11111111, neg_10}; end
            5'd10: begin A1 = {5'b11111, neg_13}; A2 = {7'b1111111, neg_11}; A3 = {12'b000000000000, f[21:16]}; end
            5'd11: begin A1 = {5'b11111, neg_13}; A2 = {8'b00000000, f[21:12]}; A3 = {13'b1111111111111, neg_5}; end
            5'd12: begin A1 = {6'b111111, neg_12}; A2 = {12'b111111111111, neg_6}; A3 = {14'b00000000000000, f[21:18]}; end
            5'd13: begin A1 = {8'b11111111, neg_10}; A2 = {10'b0000000000, f[21:14]}; A3 = {12'b111111111111, neg_6}; end
            5'd14: begin A1 = {7'b0000000, f[21:11]}; A2 = {9'b000000000, f[21:13]}; A3 = {18'b000000000000000000};end
            5'd15: begin A1 = {5'b00000, f[21:9]}; A2 = {7'b1111111, neg_11}; A3 = {10'b1111111111, neg_8}; end
            5'd16: begin A1 = {5'b00000, f[21:9]}; A2 = {8'b00000000, f[21:12]}; A3 = {14'b00000000000000, f[21:18]}; end
            5'd17: begin A1 = {4'b0000, f[21:8]}; A2 = {6'b111111, neg_12}; A3 = {9'b000000000, f[21:13]}; end
            5'd18: begin A1 = {4'b0000, f[21:8]}; A2 = {9'b111111111, neg_9}; A3 = {11'b00000000000, f[21:15]}; end
            5'd19: begin A1 = {4'b0000, f[21:8]}; A2 = {6'b000000, f[21:10]}; A3 = {8'b11111111, neg_10}; end
            5'd20: begin A1 = {3'b000, f[21:7]}; A2 = {5'b11111, neg_13}; A3 = {7'b1111111, neg_11}; end
            5'd21: begin A1 = {3'b000, f[21:7]}; A2 = {5'b11111, neg_13}; A3 = {7'b0000000, f[21:11]}; end
            5'd22: begin A1 = {3'b000, f[21:7]}; A2 = {6'b111111, neg_12}; A3 = {8'b00000000, f[21:12]}; end
            5'd23: begin A1 = {3'b000, f[21:7]}; A2 = {9'b000000000, f[21:13]}; A3 = {13'b0000000000000, f[21:17]}; end
            5'd24: begin A1 = {3'b000, f[21:7]}; A2 = {6'b000000, f[21:10]}; A3 = {12'b000000000000, f[21:16]}; end
            5'd25: begin A1 = {3'b000, f[21:7]}; A2 = {5'b00000, f[21:9]}; A3 = {9'b111111111, neg_9}; end
            5'd26: begin A1 = {2'b00, f[21:6]}; A2 = {4'b1111, neg_14}; A3 = {6'b111111, neg_12}; end
            5'd27: begin A1 = {2'b00, f[21:6]}; A2 = {4'b1111, neg_14}; A3 = {8'b11111111, neg_10}; end
      endcase
    end

// Auto-generated CSA tree for final_N=4, final_M=22, final_add_M=17
    wire [17:0] csa1_carry, csa1_sum;
    wire [17:0] csa2_carry, csa2_sum;
    wire [17:0] csa3_carry, csa3_sum;
    wire [17:0] final_sum;

    CSA_anticonv1 csa1 (
        .a({1'b0, f[21:5]}),  // f的高17位
        .b(A1),
        .c(A2),
        .sum(csa1_sum),
        .carry(csa1_carry)
    );

    CSA_anticonv1 csa2 (
        .a(csa1_sum),
        .b(A3),
        .c({csa1_carry[16:0], 1'b0}),  // 左移1位
        .sum(csa2_sum),
        .carry(csa2_carry)
    );

    CSA_anticonv1 csa3 (
        .a(csa2_sum),
         .b(b),
        .c({csa2_carry[16:0], 1'b0}),
        .sum(csa3_sum),
        .carry(csa3_carry)
    );

    CPA_anticonv1 cpa (
        .a(csa3_sum),
        .b({csa3_carry[16:0], 1'b0}),  // 左移1位
        .sum(final_sum)
    );

    assign f_e2_out = {final_sum, f[4:0]};  // 拼接高位和原始低位
// End of auto-generated CSA tree

endmodule
module CSA_anticonv1 #(parameter ADD_WIDTH = 18
)(
    input [ADD_WIDTH-1:0] a,
    input [ADD_WIDTH-1:0] b,
    input [ADD_WIDTH-1:0] c,
    output [ADD_WIDTH-1:0] sum,
    output [ADD_WIDTH-1:0] carry
);
    assign sum = a ^ b ^ c;          // XOR for sum
    assign carry = (a & b) | (b & c) | (c & a); // Majority logic for carry
endmodule

// Carry-Propagate Adder (CPA) module
module CPA_anticonv1 #(parameter ADD_WIDTH = 18
)(
    input [ADD_WIDTH-1:0] a,
    input [ADD_WIDTH-1:0] b,
    output [ADD_WIDTH-1:0] sum
);
    assign sum = a + b;  // Simple binary addition
endmodule

module  APP_22_18_9_4_32_anticonv2(
    input [21:0] f,               // 22-bit input
    output [22:0] f_e2_out     // 23-bit output 
);
    reg [5:0] segment;
    reg [18:0] b;
    always @(*) begin
        case(f[21:13])
            9'b000000000: begin segment = 6'd0; b = 19'b0111111111111111111; end
            9'b000000001: begin segment = 6'd0; b = 19'b0111111111111111010; end
            9'b000000010: begin segment = 6'd0; b = 19'b0111111111111110110; end
            9'b000000011: begin segment = 6'd0; b = 19'b0111111111111110010; end
            9'b000000100: begin segment = 6'd0; b = 19'b0111111111111101111; end
            9'b000000101: begin segment = 6'd0; b = 19'b0111111111111101100; end
            9'b000000110: begin segment = 6'd0; b = 19'b0111111111111101010; end
            9'b000000111: begin segment = 6'd0; b = 19'b0111111111111101000; end
            9'b000001000: begin segment = 6'd0; b = 19'b0111111111111100111; end
            9'b000001001: begin segment = 6'd0; b = 19'b0111111111111100110; end
            9'b000001010: begin segment = 6'd0; b = 19'b0111111111111100110; end
            9'b000001011: begin segment = 6'd0; b = 19'b0111111111111100110; end
            9'b000001100: begin segment = 6'd0; b = 19'b0111111111111100111; end
            9'b000001101: begin segment = 6'd0; b = 19'b0111111111111101000; end
            9'b000001110: begin segment = 6'd0; b = 19'b0111111111111101010; end
            9'b000001111: begin segment = 6'd0; b = 19'b0111111111111101100; end
            9'b000010000: begin segment = 6'd1; b = 19'b0111111111101110011; end
            9'b000010001: begin segment = 6'd1; b = 19'b0111111111101101110; end
            9'b000010010: begin segment = 6'd1; b = 19'b0111111111101101011; end
            9'b000010011: begin segment = 6'd1; b = 19'b0111111111101100111; end
            9'b000010100: begin segment = 6'd1; b = 19'b0111111111101100101; end
            9'b000010101: begin segment = 6'd1; b = 19'b0111111111101100010; end
            9'b000010110: begin segment = 6'd1; b = 19'b0111111111101100001; end
            9'b000010111: begin segment = 6'd1; b = 19'b0111111111101011111; end
            9'b000011000: begin segment = 6'd1; b = 19'b0111111111101011110; end
            9'b000011001: begin segment = 6'd1; b = 19'b0111111111101011110; end
            9'b000011010: begin segment = 6'd1; b = 19'b0111111111101011110; end
            9'b000011011: begin segment = 6'd1; b = 19'b0111111111101011111; end
            9'b000011100: begin segment = 6'd1; b = 19'b0111111111101100000; end
            9'b000011101: begin segment = 6'd1; b = 19'b0111111111101100001; end
            9'b000011110: begin segment = 6'd1; b = 19'b0111111111101100011; end
            9'b000011111: begin segment = 6'd1; b = 19'b0111111111101100110; end
            9'b000100000: begin segment = 6'd1; b = 19'b0111111111101101001; end
            9'b000100001: begin segment = 6'd1; b = 19'b0111111111101101101; end
            9'b000100010: begin segment = 6'd1; b = 19'b0111111111101110001; end
            9'b000100011: begin segment = 6'd2; b = 19'b0111111111000100100; end
            9'b000100100: begin segment = 6'd2; b = 19'b0111111111000100000; end
            9'b000100101: begin segment = 6'd2; b = 19'b0111111111000011100; end
            9'b000100110: begin segment = 6'd2; b = 19'b0111111111000011000; end
            9'b000100111: begin segment = 6'd2; b = 19'b0111111111000010101; end
            9'b000101000: begin segment = 6'd2; b = 19'b0111111111000010011; end
            9'b000101001: begin segment = 6'd2; b = 19'b0111111111000010001; end
            9'b000101010: begin segment = 6'd2; b = 19'b0111111111000010000; end
            9'b000101011: begin segment = 6'd2; b = 19'b0111111111000001111; end
            9'b000101100: begin segment = 6'd2; b = 19'b0111111111000001111; end
            9'b000101101: begin segment = 6'd2; b = 19'b0111111111000001111; end
            9'b000101110: begin segment = 6'd2; b = 19'b0111111111000010000; end
            9'b000101111: begin segment = 6'd2; b = 19'b0111111111000010001; end
            9'b000110000: begin segment = 6'd2; b = 19'b0111111111000010010; end
            9'b000110001: begin segment = 6'd2; b = 19'b0111111111000010101; end
            9'b000110010: begin segment = 6'd2; b = 19'b0111111111000010111; end
            9'b000110011: begin segment = 6'd2; b = 19'b0111111111000011011; end
            9'b000110100: begin segment = 6'd2; b = 19'b0111111111000011110; end
            9'b000110101: begin segment = 6'd2; b = 19'b0111111111000100011; end
            9'b000110110: begin segment = 6'd3; b = 19'b0111111110000100010; end
            9'b000110111: begin segment = 6'd3; b = 19'b0111111110000011101; end
            9'b000111000: begin segment = 6'd3; b = 19'b0111111110000011010; end
            9'b000111001: begin segment = 6'd3; b = 19'b0111111110000010111; end
            9'b000111010: begin segment = 6'd3; b = 19'b0111111110000010100; end
            9'b000111011: begin segment = 6'd3; b = 19'b0111111110000010010; end
            9'b000111100: begin segment = 6'd3; b = 19'b0111111110000010000; end
            9'b000111101: begin segment = 6'd3; b = 19'b0111111110000010000; end
            9'b000111110: begin segment = 6'd3; b = 19'b0111111110000001111; end
            9'b000111111: begin segment = 6'd3; b = 19'b0111111110000001111; end
            9'b001000000: begin segment = 6'd3; b = 19'b0111111110000001111; end
            9'b001000001: begin segment = 6'd3; b = 19'b0111111110000010001; end
            9'b001000010: begin segment = 6'd3; b = 19'b0111111110000010010; end
            9'b001000011: begin segment = 6'd3; b = 19'b0111111110000010100; end
            9'b001000100: begin segment = 6'd3; b = 19'b0111111110000010111; end
            9'b001000101: begin segment = 6'd3; b = 19'b0111111110000011010; end
            9'b001000110: begin segment = 6'd3; b = 19'b0111111110000011101; end
            9'b001000111: begin segment = 6'd3; b = 19'b0111111110000100010; end
            9'b001001000: begin segment = 6'd4; b = 19'b0111111100101110110; end
            9'b001001001: begin segment = 6'd4; b = 19'b0111111100101110001; end
            9'b001001010: begin segment = 6'd4; b = 19'b0111111100101101110; end
            9'b001001011: begin segment = 6'd4; b = 19'b0111111100101101011; end
            9'b001001100: begin segment = 6'd4; b = 19'b0111111100101101000; end
            9'b001001101: begin segment = 6'd4; b = 19'b0111111100101100110; end
            9'b001001110: begin segment = 6'd4; b = 19'b0111111100101100100; end
            9'b001001111: begin segment = 6'd4; b = 19'b0111111100101100011; end
            9'b001010000: begin segment = 6'd4; b = 19'b0111111100101100011; end
            9'b001010001: begin segment = 6'd4; b = 19'b0111111100101100011; end
            9'b001010010: begin segment = 6'd4; b = 19'b0111111100101100011; end
            9'b001010011: begin segment = 6'd4; b = 19'b0111111100101100100; end
            9'b001010100: begin segment = 6'd4; b = 19'b0111111100101100110; end
            9'b001010101: begin segment = 6'd4; b = 19'b0111111100101101000; end
            9'b001010110: begin segment = 6'd4; b = 19'b0111111100101101011; end
            9'b001010111: begin segment = 6'd4; b = 19'b0111111100101101110; end
            9'b001011000: begin segment = 6'd4; b = 19'b0111111100101110010; end
            9'b001011001: begin segment = 6'd4; b = 19'b0111111100101110110; end
            9'b001011010: begin segment = 6'd5; b = 19'b0111111010100111101; end
            9'b001011011: begin segment = 6'd5; b = 19'b0111111010100110111; end
            9'b001011100: begin segment = 6'd5; b = 19'b0111111010100110001; end
            9'b001011101: begin segment = 6'd5; b = 19'b0111111010100101011; end
            9'b001011110: begin segment = 6'd5; b = 19'b0111111010100100110; end
            9'b001011111: begin segment = 6'd5; b = 19'b0111111010100100010; end
            9'b001100000: begin segment = 6'd5; b = 19'b0111111010100011110; end
            9'b001100001: begin segment = 6'd5; b = 19'b0111111010100011011; end
            9'b001100010: begin segment = 6'd5; b = 19'b0111111010100011000; end
            9'b001100011: begin segment = 6'd5; b = 19'b0111111010100010110; end
            9'b001100100: begin segment = 6'd5; b = 19'b0111111010100010100; end
            9'b001100101: begin segment = 6'd5; b = 19'b0111111010100010011; end
            9'b001100110: begin segment = 6'd5; b = 19'b0111111010100010010; end
            9'b001100111: begin segment = 6'd5; b = 19'b0111111010100010010; end
            9'b001101000: begin segment = 6'd5; b = 19'b0111111010100010011; end
            9'b001101001: begin segment = 6'd5; b = 19'b0111111010100010100; end
            9'b001101010: begin segment = 6'd5; b = 19'b0111111010100010110; end
            9'b001101011: begin segment = 6'd5; b = 19'b0111111010100011000; end
            9'b001101100: begin segment = 6'd6; b = 19'b0111111000111010010; end
            9'b001101101: begin segment = 6'd6; b = 19'b0111111000111001101; end
            9'b001101110: begin segment = 6'd6; b = 19'b0111111000111001001; end
            9'b001101111: begin segment = 6'd6; b = 19'b0111111000111000110; end
            9'b001110000: begin segment = 6'd6; b = 19'b0111111000111000100; end
            9'b001110001: begin segment = 6'd6; b = 19'b0111111000111000001; end
            9'b001110010: begin segment = 6'd6; b = 19'b0111111000110111111; end
            9'b001110011: begin segment = 6'd6; b = 19'b0111111000110111110; end
            9'b001110100: begin segment = 6'd6; b = 19'b0111111000110111110; end
            9'b001110101: begin segment = 6'd6; b = 19'b0111111000110111110; end
            9'b001110110: begin segment = 6'd6; b = 19'b0111111000110111110; end
            9'b001110111: begin segment = 6'd6; b = 19'b0111111000110111111; end
            9'b001111000: begin segment = 6'd6; b = 19'b0111111000111000010; end
            9'b001111001: begin segment = 6'd6; b = 19'b0111111000111000100; end
            9'b001111010: begin segment = 6'd6; b = 19'b0111111000111000111; end
            9'b001111011: begin segment = 6'd6; b = 19'b0111111000111001010; end
            9'b001111100: begin segment = 6'd6; b = 19'b0111111000111001110; end
            9'b001111101: begin segment = 6'd6; b = 19'b0111111000111010011; end
            9'b001111110: begin segment = 6'd7; b = 19'b0111110110111000100; end
            9'b001111111: begin segment = 6'd7; b = 19'b0111110110111000010; end
            9'b010000000: begin segment = 6'd7; b = 19'b0111110110111000000; end
            9'b010000001: begin segment = 6'd7; b = 19'b0111110110110111110; end
            9'b010000010: begin segment = 6'd7; b = 19'b0111110110110111101; end
            9'b010000011: begin segment = 6'd7; b = 19'b0111110110110111101; end
            9'b010000100: begin segment = 6'd7; b = 19'b0111110110110111110; end
            9'b010000101: begin segment = 6'd7; b = 19'b0111110110110111110; end
            9'b010000110: begin segment = 6'd7; b = 19'b0111110110111000000; end
            9'b010000111: begin segment = 6'd7; b = 19'b0111110110111000010; end
            9'b010001000: begin segment = 6'd7; b = 19'b0111110110111000101; end
            9'b010001001: begin segment = 6'd7; b = 19'b0111110110111001000; end
            9'b010001010: begin segment = 6'd7; b = 19'b0111110110111001100; end
            9'b010001011: begin segment = 6'd7; b = 19'b0111110110111010000; end
            9'b010001100: begin segment = 6'd7; b = 19'b0111110110111010101; end
            9'b010001101: begin segment = 6'd7; b = 19'b0111110110111011011; end
            9'b010001110: begin segment = 6'd7; b = 19'b0111110110111100001; end
            9'b010001111: begin segment = 6'd7; b = 19'b0111110110111100111; end
            9'b010010000: begin segment = 6'd8; b = 19'b0111110011100101001; end
            9'b010010001: begin segment = 6'd8; b = 19'b0111110011100100100; end
            9'b010010010: begin segment = 6'd8; b = 19'b0111110011100100001; end
            9'b010010011: begin segment = 6'd8; b = 19'b0111110011100011110; end
            9'b010010100: begin segment = 6'd8; b = 19'b0111110011100011100; end
            9'b010010101: begin segment = 6'd8; b = 19'b0111110011100011010; end
            9'b010010110: begin segment = 6'd8; b = 19'b0111110011100011001; end
            9'b010010111: begin segment = 6'd8; b = 19'b0111110011100011000; end
            9'b010011000: begin segment = 6'd8; b = 19'b0111110011100011000; end
            9'b010011001: begin segment = 6'd8; b = 19'b0111110011100011001; end
            9'b010011010: begin segment = 6'd8; b = 19'b0111110011100011010; end
            9'b010011011: begin segment = 6'd8; b = 19'b0111110011100011011; end
            9'b010011100: begin segment = 6'd8; b = 19'b0111110011100011110; end
            9'b010011101: begin segment = 6'd8; b = 19'b0111110011100100001; end
            9'b010011110: begin segment = 6'd8; b = 19'b0111110011100100100; end
            9'b010011111: begin segment = 6'd8; b = 19'b0111110011100101000; end
            9'b010100000: begin segment = 6'd8; b = 19'b0111110011100101101; end
            9'b010100001: begin segment = 6'd9; b = 19'b0111110000010010011; end
            9'b010100010: begin segment = 6'd9; b = 19'b0111110000010001110; end
            9'b010100011: begin segment = 6'd9; b = 19'b0111110000010001010; end
            9'b010100100: begin segment = 6'd9; b = 19'b0111110000010000111; end
            9'b010100101: begin segment = 6'd9; b = 19'b0111110000010000100; end
            9'b010100110: begin segment = 6'd9; b = 19'b0111110000010000010; end
            9'b010100111: begin segment = 6'd9; b = 19'b0111110000010000000; end
            9'b010101000: begin segment = 6'd9; b = 19'b0111110000001111111; end
            9'b010101001: begin segment = 6'd9; b = 19'b0111110000001111111; end
            9'b010101010: begin segment = 6'd9; b = 19'b0111110000001111111; end
            9'b010101011: begin segment = 6'd9; b = 19'b0111110000010000000; end
            9'b010101100: begin segment = 6'd9; b = 19'b0111110000010000010; end
            9'b010101101: begin segment = 6'd9; b = 19'b0111110000010000100; end
            9'b010101110: begin segment = 6'd9; b = 19'b0111110000010000110; end
            9'b010101111: begin segment = 6'd9; b = 19'b0111110000010001010; end
            9'b010110000: begin segment = 6'd9; b = 19'b0111110000010001101; end
            9'b010110001: begin segment = 6'd9; b = 19'b0111110000010010010; end
            9'b010110010: begin segment = 6'd10; b = 19'b0111101100101000100; end
            9'b010110011: begin segment = 6'd10; b = 19'b0111101100100111111; end
            9'b010110100: begin segment = 6'd10; b = 19'b0111101100100111011; end
            9'b010110101: begin segment = 6'd10; b = 19'b0111101100100111000; end
            9'b010110110: begin segment = 6'd10; b = 19'b0111101100100110101; end
            9'b010110111: begin segment = 6'd10; b = 19'b0111101100100110011; end
            9'b010111000: begin segment = 6'd10; b = 19'b0111101100100110001; end
            9'b010111001: begin segment = 6'd10; b = 19'b0111101100100110000; end
            9'b010111010: begin segment = 6'd10; b = 19'b0111101100100101111; end
            9'b010111011: begin segment = 6'd10; b = 19'b0111101100100110000; end
            9'b010111100: begin segment = 6'd10; b = 19'b0111101100100110000; end
            9'b010111101: begin segment = 6'd10; b = 19'b0111101100100110010; end
            9'b010111110: begin segment = 6'd10; b = 19'b0111101100100110100; end
            9'b010111111: begin segment = 6'd10; b = 19'b0111101100100110110; end
            9'b011000000: begin segment = 6'd10; b = 19'b0111101100100111010; end
            9'b011000001: begin segment = 6'd10; b = 19'b0111101100100111101; end
            9'b011000010: begin segment = 6'd10; b = 19'b0111101100101000010; end
            9'b011000011: begin segment = 6'd11; b = 19'b0111101000011100000; end
            9'b011000100: begin segment = 6'd11; b = 19'b0111101000011011011; end
            9'b011000101: begin segment = 6'd11; b = 19'b0111101000011010110; end
            9'b011000110: begin segment = 6'd11; b = 19'b0111101000011010010; end
            9'b011000111: begin segment = 6'd11; b = 19'b0111101000011001111; end
            9'b011001000: begin segment = 6'd11; b = 19'b0111101000011001100; end
            9'b011001001: begin segment = 6'd11; b = 19'b0111101000011001010; end
            9'b011001010: begin segment = 6'd11; b = 19'b0111101000011001001; end
            9'b011001011: begin segment = 6'd11; b = 19'b0111101000011001000; end
            9'b011001100: begin segment = 6'd11; b = 19'b0111101000011001000; end
            9'b011001101: begin segment = 6'd11; b = 19'b0111101000011001000; end
            9'b011001110: begin segment = 6'd11; b = 19'b0111101000011001001; end
            9'b011001111: begin segment = 6'd11; b = 19'b0111101000011001011; end
            9'b011010000: begin segment = 6'd11; b = 19'b0111101000011001101; end
            9'b011010001: begin segment = 6'd11; b = 19'b0111101000011010000; end
            9'b011010010: begin segment = 6'd11; b = 19'b0111101000011010100; end
            9'b011010011: begin segment = 6'd11; b = 19'b0111101000011011000; end
            9'b011010100: begin segment = 6'd12; b = 19'b0111100100000100101; end
            9'b011010101: begin segment = 6'd12; b = 19'b0111100100000100001; end
            9'b011010110: begin segment = 6'd12; b = 19'b0111100100000011100; end
            9'b011010111: begin segment = 6'd12; b = 19'b0111100100000011001; end
            9'b011011000: begin segment = 6'd12; b = 19'b0111100100000010101; end
            9'b011011001: begin segment = 6'd12; b = 19'b0111100100000010011; end
            9'b011011010: begin segment = 6'd12; b = 19'b0111100100000010001; end
            9'b011011011: begin segment = 6'd12; b = 19'b0111100100000010000; end
            9'b011011100: begin segment = 6'd12; b = 19'b0111100100000001111; end
            9'b011011101: begin segment = 6'd12; b = 19'b0111100100000010000; end
            9'b011011110: begin segment = 6'd12; b = 19'b0111100100000010001; end
            9'b011011111: begin segment = 6'd12; b = 19'b0111100100000010010; end
            9'b011100000: begin segment = 6'd12; b = 19'b0111100100000010100; end
            9'b011100001: begin segment = 6'd12; b = 19'b0111100100000010111; end
            9'b011100010: begin segment = 6'd12; b = 19'b0111100100000011011; end
            9'b011100011: begin segment = 6'd12; b = 19'b0111100100000011111; end
            9'b011100100: begin segment = 6'd12; b = 19'b0111100100000100011; end
            9'b011100101: begin segment = 6'd13; b = 19'b0111011111010111110; end
            9'b011100110: begin segment = 6'd13; b = 19'b0111011111010111010; end
            9'b011100111: begin segment = 6'd13; b = 19'b0111011111010110110; end
            9'b011101000: begin segment = 6'd13; b = 19'b0111011111010110011; end
            9'b011101001: begin segment = 6'd13; b = 19'b0111011111010110001; end
            9'b011101010: begin segment = 6'd13; b = 19'b0111011111010101111; end
            9'b011101011: begin segment = 6'd13; b = 19'b0111011111010101110; end
            9'b011101100: begin segment = 6'd13; b = 19'b0111011111010101101; end
            9'b011101101: begin segment = 6'd13; b = 19'b0111011111010101110; end
            9'b011101110: begin segment = 6'd13; b = 19'b0111011111010101110; end
            9'b011101111: begin segment = 6'd13; b = 19'b0111011111010110000; end
            9'b011110000: begin segment = 6'd13; b = 19'b0111011111010110010; end
            9'b011110001: begin segment = 6'd13; b = 19'b0111011111010110101; end
            9'b011110010: begin segment = 6'd13; b = 19'b0111011111010111000; end
            9'b011110011: begin segment = 6'd13; b = 19'b0111011111010111100; end
            9'b011110100: begin segment = 6'd13; b = 19'b0111011111011000001; end
            9'b011110101: begin segment = 6'd14; b = 19'b0111011010001011001; end
            9'b011110110: begin segment = 6'd14; b = 19'b0111011010001010100; end
            9'b011110111: begin segment = 6'd14; b = 19'b0111011010001010000; end
            9'b011111000: begin segment = 6'd14; b = 19'b0111011010001001101; end
            9'b011111001: begin segment = 6'd14; b = 19'b0111011010001001010; end
            9'b011111010: begin segment = 6'd14; b = 19'b0111011010001001000; end
            9'b011111011: begin segment = 6'd14; b = 19'b0111011010001000111; end
            9'b011111100: begin segment = 6'd14; b = 19'b0111011010001000110; end
            9'b011111101: begin segment = 6'd14; b = 19'b0111011010001000110; end
            9'b011111110: begin segment = 6'd14; b = 19'b0111011010001000110; end
            9'b011111111: begin segment = 6'd14; b = 19'b0111011010001001000; end
            9'b100000000: begin segment = 6'd14; b = 19'b0111011010001001010; end
            9'b100000001: begin segment = 6'd14; b = 19'b0111011010001001101; end
            9'b100000010: begin segment = 6'd14; b = 19'b0111011010001010000; end
            9'b100000011: begin segment = 6'd14; b = 19'b0111011010001010100; end
            9'b100000100: begin segment = 6'd14; b = 19'b0111011010001011001; end
            9'b100000101: begin segment = 6'd15; b = 19'b0111010100100110010; end
            9'b100000110: begin segment = 6'd15; b = 19'b0111010100100101101; end
            9'b100000111: begin segment = 6'd15; b = 19'b0111010100100101001; end
            9'b100001000: begin segment = 6'd15; b = 19'b0111010100100100110; end
            9'b100001001: begin segment = 6'd15; b = 19'b0111010100100100011; end
            9'b100001010: begin segment = 6'd15; b = 19'b0111010100100100001; end
            9'b100001011: begin segment = 6'd15; b = 19'b0111010100100011111; end
            9'b100001100: begin segment = 6'd15; b = 19'b0111010100100011111; end
            9'b100001101: begin segment = 6'd15; b = 19'b0111010100100011111; end
            9'b100001110: begin segment = 6'd15; b = 19'b0111010100100100000; end
            9'b100001111: begin segment = 6'd15; b = 19'b0111010100100100001; end
            9'b100010000: begin segment = 6'd15; b = 19'b0111010100100100011; end
            9'b100010001: begin segment = 6'd15; b = 19'b0111010100100100110; end
            9'b100010010: begin segment = 6'd15; b = 19'b0111010100100101001; end
            9'b100010011: begin segment = 6'd15; b = 19'b0111010100100101101; end
            9'b100010100: begin segment = 6'd15; b = 19'b0111010100100110011; end
            9'b100010101: begin segment = 6'd16; b = 19'b0111001110100010111; end
            9'b100010110: begin segment = 6'd16; b = 19'b0111001110100010010; end
            9'b100010111: begin segment = 6'd16; b = 19'b0111001110100001110; end
            9'b100011000: begin segment = 6'd16; b = 19'b0111001110100001010; end
            9'b100011001: begin segment = 6'd16; b = 19'b0111001110100000111; end
            9'b100011010: begin segment = 6'd16; b = 19'b0111001110100000101; end
            9'b100011011: begin segment = 6'd16; b = 19'b0111001110100000100; end
            9'b100011100: begin segment = 6'd16; b = 19'b0111001110100000100; end
            9'b100011101: begin segment = 6'd16; b = 19'b0111001110100000100; end
            9'b100011110: begin segment = 6'd16; b = 19'b0111001110100000100; end
            9'b100011111: begin segment = 6'd16; b = 19'b0111001110100000110; end
            9'b100100000: begin segment = 6'd16; b = 19'b0111001110100001000; end
            9'b100100001: begin segment = 6'd16; b = 19'b0111001110100001011; end
            9'b100100010: begin segment = 6'd16; b = 19'b0111001110100001110; end
            9'b100100011: begin segment = 6'd16; b = 19'b0111001110100010010; end
            9'b100100100: begin segment = 6'd16; b = 19'b0111001110100010111; end
            9'b100100101: begin segment = 6'd17; b = 19'b0111000111101011011; end
            9'b100100110: begin segment = 6'd17; b = 19'b0111000111101010101; end
            9'b100100111: begin segment = 6'd17; b = 19'b0111000111101010000; end
            9'b100101000: begin segment = 6'd17; b = 19'b0111000111101001100; end
            9'b100101001: begin segment = 6'd17; b = 19'b0111000111101001000; end
            9'b100101010: begin segment = 6'd17; b = 19'b0111000111101000110; end
            9'b100101011: begin segment = 6'd17; b = 19'b0111000111101000100; end
            9'b100101100: begin segment = 6'd17; b = 19'b0111000111101000010; end
            9'b100101101: begin segment = 6'd17; b = 19'b0111000111101000010; end
            9'b100101110: begin segment = 6'd17; b = 19'b0111000111101000010; end
            9'b100101111: begin segment = 6'd17; b = 19'b0111000111101000011; end
            9'b100110000: begin segment = 6'd17; b = 19'b0111000111101000100; end
            9'b100110001: begin segment = 6'd17; b = 19'b0111000111101000111; end
            9'b100110010: begin segment = 6'd17; b = 19'b0111000111101001010; end
            9'b100110011: begin segment = 6'd17; b = 19'b0111000111101001110; end
            9'b100110100: begin segment = 6'd17; b = 19'b0111000111101010010; end
            9'b100110101: begin segment = 6'd18; b = 19'b0111000001001011000; end
            9'b100110110: begin segment = 6'd18; b = 19'b0111000001001010011; end
            9'b100110111: begin segment = 6'd18; b = 19'b0111000001001001111; end
            9'b100111000: begin segment = 6'd18; b = 19'b0111000001001001100; end
            9'b100111001: begin segment = 6'd18; b = 19'b0111000001001001001; end
            9'b100111010: begin segment = 6'd18; b = 19'b0111000001001000111; end
            9'b100111011: begin segment = 6'd18; b = 19'b0111000001001000110; end
            9'b100111100: begin segment = 6'd18; b = 19'b0111000001001000110; end
            9'b100111101: begin segment = 6'd18; b = 19'b0111000001001000110; end
            9'b100111110: begin segment = 6'd18; b = 19'b0111000001001000111; end
            9'b100111111: begin segment = 6'd18; b = 19'b0111000001001001001; end
            9'b101000000: begin segment = 6'd18; b = 19'b0111000001001001100; end
            9'b101000001: begin segment = 6'd18; b = 19'b0111000001001001111; end
            9'b101000010: begin segment = 6'd18; b = 19'b0111000001001010011; end
            9'b101000011: begin segment = 6'd18; b = 19'b0111000001001011000; end
            9'b101000100: begin segment = 6'd19; b = 19'b0110111010000011011; end
            9'b101000101: begin segment = 6'd19; b = 19'b0110111010000010110; end
            9'b101000110: begin segment = 6'd19; b = 19'b0110111010000010010; end
            9'b101000111: begin segment = 6'd19; b = 19'b0110111010000001110; end
            9'b101001000: begin segment = 6'd19; b = 19'b0110111010000001100; end
            9'b101001001: begin segment = 6'd19; b = 19'b0110111010000001010; end
            9'b101001010: begin segment = 6'd19; b = 19'b0110111010000001000; end
            9'b101001011: begin segment = 6'd19; b = 19'b0110111010000001000; end
            9'b101001100: begin segment = 6'd19; b = 19'b0110111010000001000; end
            9'b101001101: begin segment = 6'd19; b = 19'b0110111010000001001; end
            9'b101001110: begin segment = 6'd19; b = 19'b0110111010000001011; end
            9'b101001111: begin segment = 6'd19; b = 19'b0110111010000001101; end
            9'b101010000: begin segment = 6'd19; b = 19'b0110111010000010001; end
            9'b101010001: begin segment = 6'd19; b = 19'b0110111010000010101; end
            9'b101010010: begin segment = 6'd19; b = 19'b0110111010000011010; end
            9'b101010011: begin segment = 6'd20; b = 19'b0110110010110001001; end
            9'b101010100: begin segment = 6'd20; b = 19'b0110110010110000100; end
            9'b101010101: begin segment = 6'd20; b = 19'b0110110010110000000; end
            9'b101010110: begin segment = 6'd20; b = 19'b0110110010101111101; end
            9'b101010111: begin segment = 6'd20; b = 19'b0110110010101111011; end
            9'b101011000: begin segment = 6'd20; b = 19'b0110110010101111001; end
            9'b101011001: begin segment = 6'd20; b = 19'b0110110010101111000; end
            9'b101011010: begin segment = 6'd20; b = 19'b0110110010101111000; end
            9'b101011011: begin segment = 6'd20; b = 19'b0110110010101111001; end
            9'b101011100: begin segment = 6'd20; b = 19'b0110110010101111010; end
            9'b101011101: begin segment = 6'd20; b = 19'b0110110010101111101; end
            9'b101011110: begin segment = 6'd20; b = 19'b0110110010110000000; end
            9'b101011111: begin segment = 6'd20; b = 19'b0110110010110000100; end
            9'b101100000: begin segment = 6'd20; b = 19'b0110110010110001000; end
            9'b101100001: begin segment = 6'd20; b = 19'b0110110010110001101; end
            9'b101100010: begin segment = 6'd21; b = 19'b0110101010011110101; end
            9'b101100011: begin segment = 6'd21; b = 19'b0110101010011110000; end
            9'b101100100: begin segment = 6'd21; b = 19'b0110101010011101100; end
            9'b101100101: begin segment = 6'd21; b = 19'b0110101010011101000; end
            9'b101100110: begin segment = 6'd21; b = 19'b0110101010011100101; end
            9'b101100111: begin segment = 6'd21; b = 19'b0110101010011100011; end
            9'b101101000: begin segment = 6'd21; b = 19'b0110101010011100010; end
            9'b101101001: begin segment = 6'd21; b = 19'b0110101010011100010; end
            9'b101101010: begin segment = 6'd21; b = 19'b0110101010011100010; end
            9'b101101011: begin segment = 6'd21; b = 19'b0110101010011100011; end
            9'b101101100: begin segment = 6'd21; b = 19'b0110101010011100101; end
            9'b101101101: begin segment = 6'd21; b = 19'b0110101010011101000; end
            9'b101101110: begin segment = 6'd21; b = 19'b0110101010011101011; end
            9'b101101111: begin segment = 6'd21; b = 19'b0110101010011110000; end
            9'b101110000: begin segment = 6'd21; b = 19'b0110101010011110101; end
            9'b101110001: begin segment = 6'd22; b = 19'b0110100001110101001; end
            9'b101110010: begin segment = 6'd22; b = 19'b0110100001110100011; end
            9'b101110011: begin segment = 6'd22; b = 19'b0110100001110011111; end
            9'b101110100: begin segment = 6'd22; b = 19'b0110100001110011011; end
            9'b101110101: begin segment = 6'd22; b = 19'b0110100001110011000; end
            9'b101110110: begin segment = 6'd22; b = 19'b0110100001110010110; end
            9'b101110111: begin segment = 6'd22; b = 19'b0110100001110010100; end
            9'b101111000: begin segment = 6'd22; b = 19'b0110100001110010100; end
            9'b101111001: begin segment = 6'd22; b = 19'b0110100001110010100; end
            9'b101111010: begin segment = 6'd22; b = 19'b0110100001110010101; end
            9'b101111011: begin segment = 6'd22; b = 19'b0110100001110010111; end
            9'b101111100: begin segment = 6'd22; b = 19'b0110100001110011001; end
            9'b101111101: begin segment = 6'd22; b = 19'b0110100001110011101; end
            9'b101111110: begin segment = 6'd22; b = 19'b0110100001110100001; end
            9'b101111111: begin segment = 6'd22; b = 19'b0110100001110100110; end
            9'b110000000: begin segment = 6'd23; b = 19'b0110011000000100110; end
            9'b110000001: begin segment = 6'd23; b = 19'b0110011000000011111; end
            9'b110000010: begin segment = 6'd23; b = 19'b0110011000000011010; end
            9'b110000011: begin segment = 6'd23; b = 19'b0110011000000010101; end
            9'b110000100: begin segment = 6'd23; b = 19'b0110011000000010001; end
            9'b110000101: begin segment = 6'd23; b = 19'b0110011000000001110; end
            9'b110000110: begin segment = 6'd23; b = 19'b0110011000000001100; end
            9'b110000111: begin segment = 6'd23; b = 19'b0110011000000001010; end
            9'b110001000: begin segment = 6'd23; b = 19'b0110011000000001010; end
            9'b110001001: begin segment = 6'd23; b = 19'b0110011000000001010; end
            9'b110001010: begin segment = 6'd23; b = 19'b0110011000000001011; end
            9'b110001011: begin segment = 6'd23; b = 19'b0110011000000001100; end
            9'b110001100: begin segment = 6'd23; b = 19'b0110011000000001111; end
            9'b110001101: begin segment = 6'd23; b = 19'b0110011000000010011; end
            9'b110001110: begin segment = 6'd23; b = 19'b0110011000000010111; end
            9'b110001111: begin segment = 6'd24; b = 19'b0110001110101100010; end
            9'b110010000: begin segment = 6'd24; b = 19'b0110001110101011100; end
            9'b110010001: begin segment = 6'd24; b = 19'b0110001110101010111; end
            9'b110010010: begin segment = 6'd24; b = 19'b0110001110101010010; end
            9'b110010011: begin segment = 6'd24; b = 19'b0110001110101001111; end
            9'b110010100: begin segment = 6'd24; b = 19'b0110001110101001100; end
            9'b110010101: begin segment = 6'd24; b = 19'b0110001110101001010; end
            9'b110010110: begin segment = 6'd24; b = 19'b0110001110101001001; end
            9'b110010111: begin segment = 6'd24; b = 19'b0110001110101001001; end
            9'b110011000: begin segment = 6'd24; b = 19'b0110001110101001001; end
            9'b110011001: begin segment = 6'd24; b = 19'b0110001110101001011; end
            9'b110011010: begin segment = 6'd24; b = 19'b0110001110101001101; end
            9'b110011011: begin segment = 6'd24; b = 19'b0110001110101010000; end
            9'b110011100: begin segment = 6'd24; b = 19'b0110001110101010100; end
            9'b110011101: begin segment = 6'd24; b = 19'b0110001110101011001; end
            9'b110011110: begin segment = 6'd25; b = 19'b0110000100111110000; end
            9'b110011111: begin segment = 6'd25; b = 19'b0110000100111101010; end
            9'b110100000: begin segment = 6'd25; b = 19'b0110000100111100110; end
            9'b110100001: begin segment = 6'd25; b = 19'b0110000100111100010; end
            9'b110100010: begin segment = 6'd25; b = 19'b0110000100111011111; end
            9'b110100011: begin segment = 6'd25; b = 19'b0110000100111011101; end
            9'b110100100: begin segment = 6'd25; b = 19'b0110000100111011011; end
            9'b110100101: begin segment = 6'd25; b = 19'b0110000100111011011; end
            9'b110100110: begin segment = 6'd25; b = 19'b0110000100111011011; end
            9'b110100111: begin segment = 6'd25; b = 19'b0110000100111011100; end
            9'b110101000: begin segment = 6'd25; b = 19'b0110000100111011111; end
            9'b110101001: begin segment = 6'd25; b = 19'b0110000100111100001; end
            9'b110101010: begin segment = 6'd25; b = 19'b0110000100111100101; end
            9'b110101011: begin segment = 6'd25; b = 19'b0110000100111101010; end
            9'b110101100: begin segment = 6'd26; b = 19'b0101111011010100010; end
            9'b110101101: begin segment = 6'd26; b = 19'b0101111011010011101; end
            9'b110101110: begin segment = 6'd26; b = 19'b0101111011010011001; end
            9'b110101111: begin segment = 6'd26; b = 19'b0101111011010010101; end
            9'b110110000: begin segment = 6'd26; b = 19'b0101111011010010011; end
            9'b110110001: begin segment = 6'd26; b = 19'b0101111011010010001; end
            9'b110110010: begin segment = 6'd26; b = 19'b0101111011010010000; end
            9'b110110011: begin segment = 6'd26; b = 19'b0101111011010010000; end
            9'b110110100: begin segment = 6'd26; b = 19'b0101111011010010001; end
            9'b110110101: begin segment = 6'd26; b = 19'b0101111011010010010; end
            9'b110110110: begin segment = 6'd26; b = 19'b0101111011010010110; end
            9'b110110111: begin segment = 6'd26; b = 19'b0101111011010011001; end
            9'b110111000: begin segment = 6'd26; b = 19'b0101111011010011101; end
            9'b110111001: begin segment = 6'd26; b = 19'b0101111011010100010; end
            9'b110111010: begin segment = 6'd27; b = 19'b0101110000100011011; end
            9'b110111011: begin segment = 6'd27; b = 19'b0101110000100010101; end
            9'b110111100: begin segment = 6'd27; b = 19'b0101110000100010001; end
            9'b110111101: begin segment = 6'd27; b = 19'b0101110000100001101; end
            9'b110111110: begin segment = 6'd27; b = 19'b0101110000100001010; end
            9'b110111111: begin segment = 6'd27; b = 19'b0101110000100001000; end
            9'b111000000: begin segment = 6'd27; b = 19'b0101110000100000111; end
            9'b111000001: begin segment = 6'd27; b = 19'b0101110000100000111; end
            9'b111000010: begin segment = 6'd27; b = 19'b0101110000100000111; end
            9'b111000011: begin segment = 6'd27; b = 19'b0101110000100001001; end
            9'b111000100: begin segment = 6'd27; b = 19'b0101110000100001011; end
            9'b111000101: begin segment = 6'd27; b = 19'b0101110000100001110; end
            9'b111000110: begin segment = 6'd27; b = 19'b0101110000100010011; end
            9'b111000111: begin segment = 6'd27; b = 19'b0101110000100011000; end
            9'b111001000: begin segment = 6'd28; b = 19'b0101100101110111000; end
            9'b111001001: begin segment = 6'd28; b = 19'b0101100101110110011; end
            9'b111001010: begin segment = 6'd28; b = 19'b0101100101110101110; end
            9'b111001011: begin segment = 6'd28; b = 19'b0101100101110101011; end
            9'b111001100: begin segment = 6'd28; b = 19'b0101100101110101000; end
            9'b111001101: begin segment = 6'd28; b = 19'b0101100101110100111; end
            9'b111001110: begin segment = 6'd28; b = 19'b0101100101110100110; end
            9'b111001111: begin segment = 6'd28; b = 19'b0101100101110100110; end
            9'b111010000: begin segment = 6'd28; b = 19'b0101100101110101000; end
            9'b111010001: begin segment = 6'd28; b = 19'b0101100101110101010; end
            9'b111010010: begin segment = 6'd28; b = 19'b0101100101110101101; end
            9'b111010011: begin segment = 6'd28; b = 19'b0101100101110110000; end
            9'b111010100: begin segment = 6'd28; b = 19'b0101100101110110101; end
            9'b111010101: begin segment = 6'd28; b = 19'b0101100101110111011; end
            9'b111010110: begin segment = 6'd29; b = 19'b0101011010110110011; end
            9'b111010111: begin segment = 6'd29; b = 19'b0101011010110101110; end
            9'b111011000: begin segment = 6'd29; b = 19'b0101011010110101011; end
            9'b111011001: begin segment = 6'd29; b = 19'b0101011010110101000; end
            9'b111011010: begin segment = 6'd29; b = 19'b0101011010110100110; end
            9'b111011011: begin segment = 6'd29; b = 19'b0101011010110100110; end
            9'b111011100: begin segment = 6'd29; b = 19'b0101011010110100110; end
            9'b111011101: begin segment = 6'd29; b = 19'b0101011010110100111; end
            9'b111011110: begin segment = 6'd29; b = 19'b0101011010110101000; end
            9'b111011111: begin segment = 6'd29; b = 19'b0101011010110101011; end
            9'b111100000: begin segment = 6'd29; b = 19'b0101011010110101111; end
            9'b111100001: begin segment = 6'd29; b = 19'b0101011010110110100; end
            9'b111100010: begin segment = 6'd29; b = 19'b0101011010110111001; end
            9'b111100011: begin segment = 6'd29; b = 19'b0101011010111000000; end
            9'b111100100: begin segment = 6'd30; b = 19'b0101001111100010001; end
            9'b111100101: begin segment = 6'd30; b = 19'b0101001111100001101; end
            9'b111100110: begin segment = 6'd30; b = 19'b0101001111100001011; end
            9'b111100111: begin segment = 6'd30; b = 19'b0101001111100001001; end
            9'b111101000: begin segment = 6'd30; b = 19'b0101001111100001000; end
            9'b111101001: begin segment = 6'd30; b = 19'b0101001111100001000; end
            9'b111101010: begin segment = 6'd30; b = 19'b0101001111100001001; end
            9'b111101011: begin segment = 6'd30; b = 19'b0101001111100001011; end
            9'b111101100: begin segment = 6'd30; b = 19'b0101001111100001101; end
            9'b111101101: begin segment = 6'd30; b = 19'b0101001111100010001; end
            9'b111101110: begin segment = 6'd30; b = 19'b0101001111100010110; end
            9'b111101111: begin segment = 6'd30; b = 19'b0101001111100011100; end
            9'b111110000: begin segment = 6'd30; b = 19'b0101001111100100010; end
            9'b111110001: begin segment = 6'd30; b = 19'b0101001111100101010; end
            9'b111110010: begin segment = 6'd31; b = 19'b0101000000111111101; end
            9'b111110011: begin segment = 6'd31; b = 19'b0101000000111110111; end
            9'b111110100: begin segment = 6'd31; b = 19'b0101000000111110010; end
            9'b111110101: begin segment = 6'd31; b = 19'b0101000000111101111; end
            9'b111110110: begin segment = 6'd31; b = 19'b0101000000111101100; end
            9'b111110111: begin segment = 6'd31; b = 19'b0101000000111101010; end
            9'b111111000: begin segment = 6'd31; b = 19'b0101000000111101001; end
            9'b111111001: begin segment = 6'd31; b = 19'b0101000000111101001; end
            9'b111111010: begin segment = 6'd31; b = 19'b0101000000111101010; end
            9'b111111011: begin segment = 6'd31; b = 19'b0101000000111101100; end
            9'b111111100: begin segment = 6'd31; b = 19'b0101000000111101111; end
            9'b111111101: begin segment = 6'd31; b = 19'b0101000000111110011; end
            9'b111111110: begin segment = 6'd31; b = 19'b0101000000111111000; end
            9'b111111111: begin segment = 6'd31; b = 19'b0101000000111111110; end
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


// Auto-generated register declarations for 4 registers
    reg [18:0] A1;
    reg [18:0] A2;
    reg [18:0] A3;
    reg [18:0] A4;
// End of auto-generated section

    always @(*) begin
        case(segment)
            6'd0: begin A1 = {3'b111, neg_16}; A2 = {5'b11111, neg_14}; A3 = {7'b0000000, f[21:10]}; end
            6'd1: begin A1 = {3'b111, neg_16}; A2 = {6'b111111, neg_13}; A3 = {11'b11111111111, neg_8}; end
            6'd2: begin A1 = {3'b111, neg_16}; A2 = {7'b1111111, neg_12}; A3 = {10'b0000000000, f[21:13]}; end
            6'd3: begin A1 = {3'b111, neg_16}; A2 = {9'b000000000, f[21:12]}; A3 = {11'b00000000000, f[21:14]}; end
            6'd4: begin A1 = {3'b111, neg_16}; A2 = {6'b000000, f[21:9]}; A3 = {8'b11111111, neg_11}; end
            6'd5: begin A1 = {3'b111, neg_16}; A2 = {5'b00000, f[21:8]}; A3 = {7'b1111111, neg_12}; end
            6'd6: begin A1 = {3'b111, neg_16}; A2 = {5'b00000, f[21:8]}; A3 = {12'b111111111111, neg_7}; end
            6'd7: begin A1 = {3'b111, neg_16}; A2 = {5'b00000, f[21:8]}; A3 = {7'b0000000, f[21:10]}; end
            6'd8: begin A1 = {4'b1111, neg_15}; A2 = {6'b111111, neg_13}; A3 = {8'b00000000, f[21:11]}; end
            6'd9: begin A1 = {4'b1111, neg_15}; A2 = {9'b111111111, neg_10}; A3 = {11'b00000000000, f[21:14]}; end
            6'd10: begin A1 = {4'b1111, neg_15}; A2 = {7'b0000000, f[21:10]}; A3 = {10'b0000000000, f[21:13]}; end
            6'd11: begin A1 = {4'b1111, neg_15}; A2 = {6'b000000, f[21:9]}; A3 = {8'b00000000, f[21:11]}; end
            6'd12: begin A1 = {5'b11111, neg_14}; A2 = {9'b111111111, neg_10}; A3 = {11'b00000000000, f[21:14]}; end
            6'd13: begin A1 = {5'b11111, neg_14}; A2 = {7'b0000000, f[21:10]}; A3 = {10'b0000000000, f[21:13]}; end
            6'd14: begin A1 = {6'b111111, neg_13}; A2 = {8'b00000000, f[21:11]}; A3 = {13'b1111111111111, neg_6}; end
            6'd15: begin A1 = {10'b1111111111, neg_9}; A2 = {12'b111111111111, neg_7}; A3 = {14'b00000000000000, f[21:17]}; end
            6'd16: begin A1 = {7'b0000000, f[21:10]}; A2 = {9'b000000000, f[21:12]}; A3 = {19'b0000000000000000000};end
            6'd17: begin A1 = {5'b00000, f[21:8]}; A2 = {7'b1111111, neg_12}; A3 = {9'b111111111, neg_10}; end
            6'd18: begin A1 = {5'b00000, f[21:8]}; A2 = {10'b0000000000, f[21:13]}; A3 = {12'b111111111111, neg_7}; end
            6'd19: begin A1 = {4'b0000, f[21:7]}; A2 = {6'b111111, neg_13}; A3 = {8'b11111111, neg_11}; end
            6'd20: begin A1 = {4'b0000, f[21:7]}; A2 = {7'b1111111, neg_12}; A3 = {10'b1111111111, neg_9}; end
            6'd21: begin A1 = {4'b0000, f[21:7]}; A2 = {8'b00000000, f[21:11]}; A3 = {10'b1111111111, neg_9}; end
            6'd22: begin A1 = {4'b0000, f[21:7]}; A2 = {6'b000000, f[21:9]}; A3 = {10'b1111111111, neg_9}; end
            6'd23: begin A1 = {3'b000, f[21:6]}; A2 = {5'b11111, neg_14}; A3 = {8'b11111111, neg_11}; end
            6'd24: begin A1 = {3'b000, f[21:6]}; A2 = {5'b11111, neg_14}; A3 = {7'b0000000, f[21:10]}; end
            6'd25: begin A1 = {3'b000, f[21:6]}; A2 = {6'b111111, neg_13}; A3 = {8'b00000000, f[21:11]}; end
            6'd26: begin A1 = {3'b000, f[21:6]}; A2 = {11'b11111111111, neg_8}; A3 = {15'b000000000000000, f[21:18]}; end
            6'd27: begin A1 = {3'b000, f[21:6]}; A2 = {6'b000000, f[21:9]}; A3 = {8'b11111111, neg_11}; end
            6'd28: begin A1 = {3'b000, f[21:6]}; A2 = {5'b00000, f[21:8]}; A3 = {7'b1111111, neg_12}; end
            6'd29: begin A1 = {3'b000, f[21:6]}; A2 = {5'b00000, f[21:8]}; A3 = {8'b00000000, f[21:11]}; end
            6'd30: begin A1 = {2'b00, f[21:5]}; A2 = {4'b1111, neg_15}; A3 = {6'b111111, neg_13}; end
            6'd31: begin A1 = {2'b00, f[21:5]}; A2 = {4'b1111, neg_15}; A3 = {10'b1111111111, neg_9}; end
      endcase
    end

// Auto-generated CSA tree for final_N=4, final_M=22, final_add_M=18
    wire [18:0] csa1_carry, csa1_sum;
    wire [18:0] csa2_carry, csa2_sum;
    wire [18:0] csa3_carry, csa3_sum;
    wire [18:0] final_sum;

    CSA_anticonv2 csa1 (
        .a({1'b0, f[21:4]}),  // f的高18位
        .b(A1),
        .c(A2),
        .sum(csa1_sum),
        .carry(csa1_carry)
    );

    CSA_anticonv2 csa2 (
        .a(csa1_sum),
        .b(A3),
        .c({csa1_carry[17:0], 1'b0}),  // 左移1位
        .sum(csa2_sum),
        .carry(csa2_carry)
    );

    CSA_anticonv2 csa3 (
        .a(csa2_sum),
         .b(b),
        .c({csa2_carry[17:0], 1'b0}),
        .sum(csa3_sum),
        .carry(csa3_carry)
    );

    CPA_anticonv2 cpa (
        .a(csa3_sum),
        .b({csa3_carry[17:0], 1'b0}),  // 左移1位
        .sum(final_sum)
    );

    assign f_e2_out = {final_sum, f[3:0]};  // 拼接高位和原始低位
// End of auto-generated CSA tree

endmodule
module CSA_anticonv2 #(parameter ADD_WIDTH = 19
)(
    input [ADD_WIDTH-1:0] a,
    input [ADD_WIDTH-1:0] b,
    input [ADD_WIDTH-1:0] c,
    output [ADD_WIDTH-1:0] sum,
    output [ADD_WIDTH-1:0] carry
);
    assign sum = a ^ b ^ c;          // XOR for sum
    assign carry = (a & b) | (b & c) | (c & a); // Majority logic for carry
endmodule

// Carry-Propagate Adder (CPA) module
module CPA_anticonv2 #(parameter ADD_WIDTH = 19
)(
    input [ADD_WIDTH-1:0] a,
    input [ADD_WIDTH-1:0] b,
    output [ADD_WIDTH-1:0] sum
);
    assign sum = a + b;  // Simple binary addition
endmodule

module  APP_22_19_9_5_32_anticonv3(
    input [21:0] f,               // 22-bit input
    output [22:0] f_e2_out     // 23-bit output 
);
    reg [5:0] segment;
    reg [19:0] b;
    always @(*) begin
        case(f[21:13])
            9'b000000000: begin segment = 6'd0; b = 20'b01111111111111111110; end
            9'b000000001: begin segment = 6'd0; b = 20'b01111111111111110110; end
            9'b000000010: begin segment = 6'd0; b = 20'b01111111111111110000; end
            9'b000000011: begin segment = 6'd0; b = 20'b01111111111111101011; end
            9'b000000100: begin segment = 6'd0; b = 20'b01111111111111100110; end
            9'b000000101: begin segment = 6'd0; b = 20'b01111111111111100011; end
            9'b000000110: begin segment = 6'd0; b = 20'b01111111111111100001; end
            9'b000000111: begin segment = 6'd0; b = 20'b01111111111111011111; end
            9'b000001000: begin segment = 6'd0; b = 20'b01111111111111011111; end
            9'b000001001: begin segment = 6'd0; b = 20'b01111111111111011111; end
            9'b000001010: begin segment = 6'd0; b = 20'b01111111111111100001; end
            9'b000001011: begin segment = 6'd0; b = 20'b01111111111111100011; end
            9'b000001100: begin segment = 6'd0; b = 20'b01111111111111100111; end
            9'b000001101: begin segment = 6'd0; b = 20'b01111111111111101011; end
            9'b000001110: begin segment = 6'd0; b = 20'b01111111111111110000; end
            9'b000001111: begin segment = 6'd0; b = 20'b01111111111111110111; end
            9'b000010000: begin segment = 6'd1; b = 20'b01111111111011101010; end
            9'b000010001: begin segment = 6'd1; b = 20'b01111111111011100001; end
            9'b000010010: begin segment = 6'd1; b = 20'b01111111111011011010; end
            9'b000010011: begin segment = 6'd1; b = 20'b01111111111011010011; end
            9'b000010100: begin segment = 6'd1; b = 20'b01111111111011001110; end
            9'b000010101: begin segment = 6'd1; b = 20'b01111111111011001001; end
            9'b000010110: begin segment = 6'd1; b = 20'b01111111111011000110; end
            9'b000010111: begin segment = 6'd1; b = 20'b01111111111011000011; end
            9'b000011000: begin segment = 6'd1; b = 20'b01111111111011000010; end
            9'b000011001: begin segment = 6'd1; b = 20'b01111111111011000001; end
            9'b000011010: begin segment = 6'd1; b = 20'b01111111111011000010; end
            9'b000011011: begin segment = 6'd1; b = 20'b01111111111011000100; end
            9'b000011100: begin segment = 6'd1; b = 20'b01111111111011000110; end
            9'b000011101: begin segment = 6'd1; b = 20'b01111111111011001010; end
            9'b000011110: begin segment = 6'd1; b = 20'b01111111111011001110; end
            9'b000011111: begin segment = 6'd1; b = 20'b01111111111011010011; end
            9'b000100000: begin segment = 6'd1; b = 20'b01111111111011011010; end
            9'b000100001: begin segment = 6'd1; b = 20'b01111111111011100001; end
            9'b000100010: begin segment = 6'd1; b = 20'b01111111111011101010; end
            9'b000100011: begin segment = 6'd2; b = 20'b01111111110001001100; end
            9'b000100100: begin segment = 6'd2; b = 20'b01111111110001000011; end
            9'b000100101: begin segment = 6'd2; b = 20'b01111111110000111011; end
            9'b000100110: begin segment = 6'd2; b = 20'b01111111110000110101; end
            9'b000100111: begin segment = 6'd2; b = 20'b01111111110000101111; end
            9'b000101000: begin segment = 6'd2; b = 20'b01111111110000101011; end
            9'b000101001: begin segment = 6'd2; b = 20'b01111111110000100111; end
            9'b000101010: begin segment = 6'd2; b = 20'b01111111110000100101; end
            9'b000101011: begin segment = 6'd2; b = 20'b01111111110000100011; end
            9'b000101100: begin segment = 6'd2; b = 20'b01111111110000100010; end
            9'b000101101: begin segment = 6'd2; b = 20'b01111111110000100011; end
            9'b000101110: begin segment = 6'd2; b = 20'b01111111110000100100; end
            9'b000101111: begin segment = 6'd2; b = 20'b01111111110000100111; end
            9'b000110000: begin segment = 6'd2; b = 20'b01111111110000101011; end
            9'b000110001: begin segment = 6'd2; b = 20'b01111111110000101111; end
            9'b000110010: begin segment = 6'd2; b = 20'b01111111110000110101; end
            9'b000110011: begin segment = 6'd2; b = 20'b01111111110000111011; end
            9'b000110100: begin segment = 6'd2; b = 20'b01111111110001000011; end
            9'b000110101: begin segment = 6'd2; b = 20'b01111111110001001100; end
            9'b000110110: begin segment = 6'd3; b = 20'b01111111100001000011; end
            9'b000110111: begin segment = 6'd3; b = 20'b01111111100000111011; end
            9'b000111000: begin segment = 6'd3; b = 20'b01111111100000110011; end
            9'b000111001: begin segment = 6'd3; b = 20'b01111111100000101101; end
            9'b000111010: begin segment = 6'd3; b = 20'b01111111100000101000; end
            9'b000111011: begin segment = 6'd3; b = 20'b01111111100000100100; end
            9'b000111100: begin segment = 6'd3; b = 20'b01111111100000100001; end
            9'b000111101: begin segment = 6'd3; b = 20'b01111111100000011111; end
            9'b000111110: begin segment = 6'd3; b = 20'b01111111100000011110; end
            9'b000111111: begin segment = 6'd3; b = 20'b01111111100000011110; end
            9'b001000000: begin segment = 6'd3; b = 20'b01111111100000011111; end
            9'b001000001: begin segment = 6'd3; b = 20'b01111111100000100001; end
            9'b001000010: begin segment = 6'd3; b = 20'b01111111100000100100; end
            9'b001000011: begin segment = 6'd3; b = 20'b01111111100000101000; end
            9'b001000100: begin segment = 6'd3; b = 20'b01111111100000101110; end
            9'b001000101: begin segment = 6'd3; b = 20'b01111111100000110100; end
            9'b001000110: begin segment = 6'd3; b = 20'b01111111100000111011; end
            9'b001000111: begin segment = 6'd3; b = 20'b01111111100001000100; end
            9'b001001000: begin segment = 6'd4; b = 20'b01111111001011100110; end
            9'b001001001: begin segment = 6'd4; b = 20'b01111111001011011110; end
            9'b001001010: begin segment = 6'd4; b = 20'b01111111001011010111; end
            9'b001001011: begin segment = 6'd4; b = 20'b01111111001011010000; end
            9'b001001100: begin segment = 6'd4; b = 20'b01111111001011001011; end
            9'b001001101: begin segment = 6'd4; b = 20'b01111111001011000111; end
            9'b001001110: begin segment = 6'd4; b = 20'b01111111001011000100; end
            9'b001001111: begin segment = 6'd4; b = 20'b01111111001011000001; end
            9'b001010000: begin segment = 6'd4; b = 20'b01111111001010111111; end
            9'b001010001: begin segment = 6'd4; b = 20'b01111111001010111111; end
            9'b001010010: begin segment = 6'd4; b = 20'b01111111001011000001; end
            9'b001010011: begin segment = 6'd4; b = 20'b01111111001011000011; end
            9'b001010100: begin segment = 6'd4; b = 20'b01111111001011000110; end
            9'b001010101: begin segment = 6'd4; b = 20'b01111111001011001010; end
            9'b001010110: begin segment = 6'd4; b = 20'b01111111001011010000; end
            9'b001010111: begin segment = 6'd4; b = 20'b01111111001011010110; end
            9'b001011000: begin segment = 6'd4; b = 20'b01111111001011011110; end
            9'b001011001: begin segment = 6'd4; b = 20'b01111111001011100111; end
            9'b001011010: begin segment = 6'd5; b = 20'b01111110101111100100; end
            9'b001011011: begin segment = 6'd5; b = 20'b01111110101111011011; end
            9'b001011100: begin segment = 6'd5; b = 20'b01111110101111010011; end
            9'b001011101: begin segment = 6'd5; b = 20'b01111110101111001100; end
            9'b001011110: begin segment = 6'd5; b = 20'b01111110101111000110; end
            9'b001011111: begin segment = 6'd5; b = 20'b01111110101111000001; end
            9'b001100000: begin segment = 6'd5; b = 20'b01111110101110111101; end
            9'b001100001: begin segment = 6'd5; b = 20'b01111110101110111011; end
            9'b001100010: begin segment = 6'd5; b = 20'b01111110101110111001; end
            9'b001100011: begin segment = 6'd5; b = 20'b01111110101110111001; end
            9'b001100100: begin segment = 6'd5; b = 20'b01111110101110111001; end
            9'b001100101: begin segment = 6'd5; b = 20'b01111110101110111011; end
            9'b001100110: begin segment = 6'd5; b = 20'b01111110101110111110; end
            9'b001100111: begin segment = 6'd5; b = 20'b01111110101111000010; end
            9'b001101000: begin segment = 6'd5; b = 20'b01111110101111000111; end
            9'b001101001: begin segment = 6'd5; b = 20'b01111110101111001110; end
            9'b001101010: begin segment = 6'd5; b = 20'b01111110101111010101; end
            9'b001101011: begin segment = 6'd5; b = 20'b01111110101111011101; end
            9'b001101100: begin segment = 6'd6; b = 20'b01111110001110010110; end
            9'b001101101: begin segment = 6'd6; b = 20'b01111110001110001100; end
            9'b001101110: begin segment = 6'd6; b = 20'b01111110001110000101; end
            9'b001101111: begin segment = 6'd6; b = 20'b01111110001101111110; end
            9'b001110000: begin segment = 6'd6; b = 20'b01111110001101111000; end
            9'b001110001: begin segment = 6'd6; b = 20'b01111110001101110011; end
            9'b001110010: begin segment = 6'd6; b = 20'b01111110001101110000; end
            9'b001110011: begin segment = 6'd6; b = 20'b01111110001101101101; end
            9'b001110100: begin segment = 6'd6; b = 20'b01111110001101101101; end
            9'b001110101: begin segment = 6'd6; b = 20'b01111110001101101101; end
            9'b001110110: begin segment = 6'd6; b = 20'b01111110001101101110; end
            9'b001110111: begin segment = 6'd6; b = 20'b01111110001101110000; end
            9'b001111000: begin segment = 6'd6; b = 20'b01111110001101110011; end
            9'b001111001: begin segment = 6'd6; b = 20'b01111110001101110111; end
            9'b001111010: begin segment = 6'd6; b = 20'b01111110001101111110; end
            9'b001111011: begin segment = 6'd6; b = 20'b01111110001110000100; end
            9'b001111100: begin segment = 6'd6; b = 20'b01111110001110001100; end
            9'b001111101: begin segment = 6'd6; b = 20'b01111110001110010101; end
            9'b001111110: begin segment = 6'd7; b = 20'b01111101100110001110; end
            9'b001111111: begin segment = 6'd7; b = 20'b01111101100110000100; end
            9'b010000000: begin segment = 6'd7; b = 20'b01111101100101111101; end
            9'b010000001: begin segment = 6'd7; b = 20'b01111101100101110110; end
            9'b010000010: begin segment = 6'd7; b = 20'b01111101100101110000; end
            9'b010000011: begin segment = 6'd7; b = 20'b01111101100101101100; end
            9'b010000100: begin segment = 6'd7; b = 20'b01111101100101101001; end
            9'b010000101: begin segment = 6'd7; b = 20'b01111101100101100110; end
            9'b010000110: begin segment = 6'd7; b = 20'b01111101100101100101; end
            9'b010000111: begin segment = 6'd7; b = 20'b01111101100101100110; end
            9'b010001000: begin segment = 6'd7; b = 20'b01111101100101100111; end
            9'b010001001: begin segment = 6'd7; b = 20'b01111101100101101001; end
            9'b010001010: begin segment = 6'd7; b = 20'b01111101100101101101; end
            9'b010001011: begin segment = 6'd7; b = 20'b01111101100101110010; end
            9'b010001100: begin segment = 6'd7; b = 20'b01111101100101110111; end
            9'b010001101: begin segment = 6'd7; b = 20'b01111101100101111111; end
            9'b010001110: begin segment = 6'd7; b = 20'b01111101100110000111; end
            9'b010001111: begin segment = 6'd7; b = 20'b01111101100110010000; end
            9'b010010000: begin segment = 6'd8; b = 20'b01111100111000001000; end
            9'b010010001: begin segment = 6'd8; b = 20'b01111100111000000000; end
            9'b010010010: begin segment = 6'd8; b = 20'b01111100110111111000; end
            9'b010010011: begin segment = 6'd8; b = 20'b01111100110111110010; end
            9'b010010100: begin segment = 6'd8; b = 20'b01111100110111101100; end
            9'b010010101: begin segment = 6'd8; b = 20'b01111100110111101001; end
            9'b010010110: begin segment = 6'd8; b = 20'b01111100110111100101; end
            9'b010010111: begin segment = 6'd8; b = 20'b01111100110111100100; end
            9'b010011000: begin segment = 6'd8; b = 20'b01111100110111100011; end
            9'b010011001: begin segment = 6'd8; b = 20'b01111100110111100100; end
            9'b010011010: begin segment = 6'd8; b = 20'b01111100110111100101; end
            9'b010011011: begin segment = 6'd8; b = 20'b01111100110111101001; end
            9'b010011100: begin segment = 6'd8; b = 20'b01111100110111101101; end
            9'b010011101: begin segment = 6'd8; b = 20'b01111100110111110010; end
            9'b010011110: begin segment = 6'd8; b = 20'b01111100110111111001; end
            9'b010011111: begin segment = 6'd8; b = 20'b01111100111000000001; end
            9'b010100000: begin segment = 6'd8; b = 20'b01111100111000001001; end
            9'b010100001: begin segment = 6'd9; b = 20'b01111100000100111001; end
            9'b010100010: begin segment = 6'd9; b = 20'b01111100000100110000; end
            9'b010100011: begin segment = 6'd9; b = 20'b01111100000100101000; end
            9'b010100100: begin segment = 6'd9; b = 20'b01111100000100100001; end
            9'b010100101: begin segment = 6'd9; b = 20'b01111100000100011100; end
            9'b010100110: begin segment = 6'd9; b = 20'b01111100000100011000; end
            9'b010100111: begin segment = 6'd9; b = 20'b01111100000100010100; end
            9'b010101000: begin segment = 6'd9; b = 20'b01111100000100010011; end
            9'b010101001: begin segment = 6'd9; b = 20'b01111100000100010011; end
            9'b010101010: begin segment = 6'd9; b = 20'b01111100000100010011; end
            9'b010101011: begin segment = 6'd9; b = 20'b01111100000100010101; end
            9'b010101100: begin segment = 6'd9; b = 20'b01111100000100011000; end
            9'b010101101: begin segment = 6'd9; b = 20'b01111100000100011100; end
            9'b010101110: begin segment = 6'd9; b = 20'b01111100000100100001; end
            9'b010101111: begin segment = 6'd9; b = 20'b01111100000100101000; end
            9'b010110000: begin segment = 6'd9; b = 20'b01111100000100110000; end
            9'b010110001: begin segment = 6'd9; b = 20'b01111100000100111001; end
            9'b010110010: begin segment = 6'd10; b = 20'b01111011001011100010; end
            9'b010110011: begin segment = 6'd10; b = 20'b01111011001011011000; end
            9'b010110100: begin segment = 6'd10; b = 20'b01111011001011010001; end
            9'b010110101: begin segment = 6'd10; b = 20'b01111011001011001010; end
            9'b010110110: begin segment = 6'd10; b = 20'b01111011001011000101; end
            9'b010110111: begin segment = 6'd10; b = 20'b01111011001011000000; end
            9'b010111000: begin segment = 6'd10; b = 20'b01111011001010111110; end
            9'b010111001: begin segment = 6'd10; b = 20'b01111011001010111100; end
            9'b010111010: begin segment = 6'd10; b = 20'b01111011001010111100; end
            9'b010111011: begin segment = 6'd10; b = 20'b01111011001010111100; end
            9'b010111100: begin segment = 6'd10; b = 20'b01111011001010111111; end
            9'b010111101: begin segment = 6'd10; b = 20'b01111011001011000001; end
            9'b010111110: begin segment = 6'd10; b = 20'b01111011001011000110; end
            9'b010111111: begin segment = 6'd10; b = 20'b01111011001011001100; end
            9'b011000000: begin segment = 6'd10; b = 20'b01111011001011010011; end
            9'b011000001: begin segment = 6'd10; b = 20'b01111011001011011011; end
            9'b011000010: begin segment = 6'd10; b = 20'b01111011001011100101; end
            9'b011000011: begin segment = 6'd11; b = 20'b01111010001010000100; end
            9'b011000100: begin segment = 6'd11; b = 20'b01111010001001111010; end
            9'b011000101: begin segment = 6'd11; b = 20'b01111010001001110010; end
            9'b011000110: begin segment = 6'd11; b = 20'b01111010001001101011; end
            9'b011000111: begin segment = 6'd11; b = 20'b01111010001001100101; end
            9'b011001000: begin segment = 6'd11; b = 20'b01111010001001100001; end
            9'b011001001: begin segment = 6'd11; b = 20'b01111010001001011110; end
            9'b011001010: begin segment = 6'd11; b = 20'b01111010001001011100; end
            9'b011001011: begin segment = 6'd11; b = 20'b01111010001001011011; end
            9'b011001100: begin segment = 6'd11; b = 20'b01111010001001011011; end
            9'b011001101: begin segment = 6'd11; b = 20'b01111010001001011101; end
            9'b011001110: begin segment = 6'd11; b = 20'b01111010001001100000; end
            9'b011001111: begin segment = 6'd11; b = 20'b01111010001001100101; end
            9'b011010000: begin segment = 6'd11; b = 20'b01111010001001101010; end
            9'b011010001: begin segment = 6'd11; b = 20'b01111010001001110001; end
            9'b011010010: begin segment = 6'd11; b = 20'b01111010001001111001; end
            9'b011010011: begin segment = 6'd11; b = 20'b01111010001010000011; end
            9'b011010100: begin segment = 6'd12; b = 20'b01111001000010000000; end
            9'b011010101: begin segment = 6'd12; b = 20'b01111001000001110110; end
            9'b011010110: begin segment = 6'd12; b = 20'b01111001000001101101; end
            9'b011010111: begin segment = 6'd12; b = 20'b01111001000001100110; end
            9'b011011000: begin segment = 6'd12; b = 20'b01111001000001100001; end
            9'b011011001: begin segment = 6'd12; b = 20'b01111001000001011100; end
            9'b011011010: begin segment = 6'd12; b = 20'b01111001000001011001; end
            9'b011011011: begin segment = 6'd12; b = 20'b01111001000001010110; end
            9'b011011100: begin segment = 6'd12; b = 20'b01111001000001010110; end
            9'b011011101: begin segment = 6'd12; b = 20'b01111001000001010111; end
            9'b011011110: begin segment = 6'd12; b = 20'b01111001000001011000; end
            9'b011011111: begin segment = 6'd12; b = 20'b01111001000001011100; end
            9'b011100000: begin segment = 6'd12; b = 20'b01111001000001100000; end
            9'b011100001: begin segment = 6'd12; b = 20'b01111001000001100110; end
            9'b011100010: begin segment = 6'd12; b = 20'b01111001000001101101; end
            9'b011100011: begin segment = 6'd12; b = 20'b01111001000001110101; end
            9'b011100100: begin segment = 6'd12; b = 20'b01111001000001111111; end
            9'b011100101: begin segment = 6'd13; b = 20'b01110111110101000011; end
            9'b011100110: begin segment = 6'd13; b = 20'b01110111110100111010; end
            9'b011100111: begin segment = 6'd13; b = 20'b01110111110100110010; end
            9'b011101000: begin segment = 6'd13; b = 20'b01110111110100101100; end
            9'b011101001: begin segment = 6'd13; b = 20'b01110111110100100111; end
            9'b011101010: begin segment = 6'd13; b = 20'b01110111110100100011; end
            9'b011101011: begin segment = 6'd13; b = 20'b01110111110100100000; end
            9'b011101100: begin segment = 6'd13; b = 20'b01110111110100011111; end
            9'b011101101: begin segment = 6'd13; b = 20'b01110111110100011111; end
            9'b011101110: begin segment = 6'd13; b = 20'b01110111110100100001; end
            9'b011101111: begin segment = 6'd13; b = 20'b01110111110100100100; end
            9'b011110000: begin segment = 6'd13; b = 20'b01110111110100100111; end
            9'b011110001: begin segment = 6'd13; b = 20'b01110111110100101100; end
            9'b011110010: begin segment = 6'd13; b = 20'b01110111110100110011; end
            9'b011110011: begin segment = 6'd13; b = 20'b01110111110100111100; end
            9'b011110100: begin segment = 6'd13; b = 20'b01110111110101000100; end
            9'b011110101: begin segment = 6'd14; b = 20'b01110110100010110101; end
            9'b011110110: begin segment = 6'd14; b = 20'b01110110100010101011; end
            9'b011110111: begin segment = 6'd14; b = 20'b01110110100010100011; end
            9'b011111000: begin segment = 6'd14; b = 20'b01110110100010011101; end
            9'b011111001: begin segment = 6'd14; b = 20'b01110110100010010111; end
            9'b011111010: begin segment = 6'd14; b = 20'b01110110100010010011; end
            9'b011111011: begin segment = 6'd14; b = 20'b01110110100010010000; end
            9'b011111100: begin segment = 6'd14; b = 20'b01110110100010001111; end
            9'b011111101: begin segment = 6'd14; b = 20'b01110110100010001111; end
            9'b011111110: begin segment = 6'd14; b = 20'b01110110100010010000; end
            9'b011111111: begin segment = 6'd14; b = 20'b01110110100010010011; end
            9'b100000000: begin segment = 6'd14; b = 20'b01110110100010011000; end
            9'b100000001: begin segment = 6'd14; b = 20'b01110110100010011101; end
            9'b100000010: begin segment = 6'd14; b = 20'b01110110100010100100; end
            9'b100000011: begin segment = 6'd14; b = 20'b01110110100010101100; end
            9'b100000100: begin segment = 6'd14; b = 20'b01110110100010110110; end
            9'b100000101: begin segment = 6'd15; b = 20'b01110101001001100011; end
            9'b100000110: begin segment = 6'd15; b = 20'b01110101001001011010; end
            9'b100000111: begin segment = 6'd15; b = 20'b01110101001001010001; end
            9'b100001000: begin segment = 6'd15; b = 20'b01110101001001001010; end
            9'b100001001: begin segment = 6'd15; b = 20'b01110101001001000100; end
            9'b100001010: begin segment = 6'd15; b = 20'b01110101001001000000; end
            9'b100001011: begin segment = 6'd15; b = 20'b01110101001000111101; end
            9'b100001100: begin segment = 6'd15; b = 20'b01110101001000111101; end
            9'b100001101: begin segment = 6'd15; b = 20'b01110101001000111101; end
            9'b100001110: begin segment = 6'd15; b = 20'b01110101001000111110; end
            9'b100001111: begin segment = 6'd15; b = 20'b01110101001001000001; end
            9'b100010000: begin segment = 6'd15; b = 20'b01110101001001000101; end
            9'b100010001: begin segment = 6'd15; b = 20'b01110101001001001010; end
            9'b100010010: begin segment = 6'd15; b = 20'b01110101001001010001; end
            9'b100010011: begin segment = 6'd15; b = 20'b01110101001001011010; end
            9'b100010100: begin segment = 6'd15; b = 20'b01110101001001100100; end
            9'b100010101: begin segment = 6'd16; b = 20'b01110011101000101110; end
            9'b100010110: begin segment = 6'd16; b = 20'b01110011101000100100; end
            9'b100010111: begin segment = 6'd16; b = 20'b01110011101000011011; end
            9'b100011000: begin segment = 6'd16; b = 20'b01110011101000010100; end
            9'b100011001: begin segment = 6'd16; b = 20'b01110011101000001111; end
            9'b100011010: begin segment = 6'd16; b = 20'b01110011101000001011; end
            9'b100011011: begin segment = 6'd16; b = 20'b01110011101000001000; end
            9'b100011100: begin segment = 6'd16; b = 20'b01110011101000000111; end
            9'b100011101: begin segment = 6'd16; b = 20'b01110011101000000111; end
            9'b100011110: begin segment = 6'd16; b = 20'b01110011101000001000; end
            9'b100011111: begin segment = 6'd16; b = 20'b01110011101000001011; end
            9'b100100000: begin segment = 6'd16; b = 20'b01110011101000001111; end
            9'b100100001: begin segment = 6'd16; b = 20'b01110011101000010101; end
            9'b100100010: begin segment = 6'd16; b = 20'b01110011101000011100; end
            9'b100100011: begin segment = 6'd16; b = 20'b01110011101000100100; end
            9'b100100100: begin segment = 6'd16; b = 20'b01110011101000101110; end
            9'b100100101: begin segment = 6'd17; b = 20'b01110001111111011010; end
            9'b100100110: begin segment = 6'd17; b = 20'b01110001111111010000; end
            9'b100100111: begin segment = 6'd17; b = 20'b01110001111111000111; end
            9'b100101000: begin segment = 6'd17; b = 20'b01110001111111000000; end
            9'b100101001: begin segment = 6'd17; b = 20'b01110001111110111010; end
            9'b100101010: begin segment = 6'd17; b = 20'b01110001111110110101; end
            9'b100101011: begin segment = 6'd17; b = 20'b01110001111110110010; end
            9'b100101100: begin segment = 6'd17; b = 20'b01110001111110110001; end
            9'b100101101: begin segment = 6'd17; b = 20'b01110001111110110001; end
            9'b100101110: begin segment = 6'd17; b = 20'b01110001111110110010; end
            9'b100101111: begin segment = 6'd17; b = 20'b01110001111110110101; end
            9'b100110000: begin segment = 6'd17; b = 20'b01110001111110111001; end
            9'b100110001: begin segment = 6'd17; b = 20'b01110001111110111111; end
            9'b100110010: begin segment = 6'd17; b = 20'b01110001111111000110; end
            9'b100110011: begin segment = 6'd17; b = 20'b01110001111111001111; end
            9'b100110100: begin segment = 6'd17; b = 20'b01110001111111011001; end
            9'b100110101: begin segment = 6'd18; b = 20'b01110000010010111001; end
            9'b100110110: begin segment = 6'd18; b = 20'b01110000010010101111; end
            9'b100110111: begin segment = 6'd18; b = 20'b01110000010010100111; end
            9'b100111000: begin segment = 6'd18; b = 20'b01110000010010100001; end
            9'b100111001: begin segment = 6'd18; b = 20'b01110000010010011011; end
            9'b100111010: begin segment = 6'd18; b = 20'b01110000010010011000; end
            9'b100111011: begin segment = 6'd18; b = 20'b01110000010010010101; end
            9'b100111100: begin segment = 6'd18; b = 20'b01110000010010010101; end
            9'b100111101: begin segment = 6'd18; b = 20'b01110000010010010101; end
            9'b100111110: begin segment = 6'd18; b = 20'b01110000010010011000; end
            9'b100111111: begin segment = 6'd18; b = 20'b01110000010010011011; end
            9'b101000000: begin segment = 6'd18; b = 20'b01110000010010100010; end
            9'b101000001: begin segment = 6'd18; b = 20'b01110000010010101000; end
            9'b101000010: begin segment = 6'd18; b = 20'b01110000010010110001; end
            9'b101000011: begin segment = 6'd18; b = 20'b01110000010010111010; end
            9'b101000100: begin segment = 6'd19; b = 20'b01101110100010000111; end
            9'b101000101: begin segment = 6'd19; b = 20'b01101110100001111101; end
            9'b101000110: begin segment = 6'd19; b = 20'b01101110100001110100; end
            9'b101000111: begin segment = 6'd19; b = 20'b01101110100001101101; end
            9'b101001000: begin segment = 6'd19; b = 20'b01101110100001101001; end
            9'b101001001: begin segment = 6'd19; b = 20'b01101110100001100101; end
            9'b101001010: begin segment = 6'd19; b = 20'b01101110100001100011; end
            9'b101001011: begin segment = 6'd19; b = 20'b01101110100001100010; end
            9'b101001100: begin segment = 6'd19; b = 20'b01101110100001100011; end
            9'b101001101: begin segment = 6'd19; b = 20'b01101110100001100101; end
            9'b101001110: begin segment = 6'd19; b = 20'b01101110100001101001; end
            9'b101001111: begin segment = 6'd19; b = 20'b01101110100001101110; end
            9'b101010000: begin segment = 6'd19; b = 20'b01101110100001110110; end
            9'b101010001: begin segment = 6'd19; b = 20'b01101110100001111110; end
            9'b101010010: begin segment = 6'd19; b = 20'b01101110100010000111; end
            9'b101010011: begin segment = 6'd20; b = 20'b01101100101001100111; end
            9'b101010100: begin segment = 6'd20; b = 20'b01101100101001011101; end
            9'b101010101: begin segment = 6'd20; b = 20'b01101100101001010101; end
            9'b101010110: begin segment = 6'd20; b = 20'b01101100101001001110; end
            9'b101010111: begin segment = 6'd20; b = 20'b01101100101001001001; end
            9'b101011000: begin segment = 6'd20; b = 20'b01101100101001000101; end
            9'b101011001: begin segment = 6'd20; b = 20'b01101100101001000011; end
            9'b101011010: begin segment = 6'd20; b = 20'b01101100101001000010; end
            9'b101011011: begin segment = 6'd20; b = 20'b01101100101001000100; end
            9'b101011100: begin segment = 6'd20; b = 20'b01101100101001000110; end
            9'b101011101: begin segment = 6'd20; b = 20'b01101100101001001010; end
            9'b101011110: begin segment = 6'd20; b = 20'b01101100101001001111; end
            9'b101011111: begin segment = 6'd20; b = 20'b01101100101001010111; end
            9'b101100000: begin segment = 6'd20; b = 20'b01101100101001011111; end
            9'b101100001: begin segment = 6'd20; b = 20'b01101100101001101001; end
            9'b101100010: begin segment = 6'd21; b = 20'b01101010101000010110; end
            9'b101100011: begin segment = 6'd21; b = 20'b01101010101000001100; end
            9'b101100100: begin segment = 6'd21; b = 20'b01101010101000000100; end
            9'b101100101: begin segment = 6'd21; b = 20'b01101010100111111100; end
            9'b101100110: begin segment = 6'd21; b = 20'b01101010100111110111; end
            9'b101100111: begin segment = 6'd21; b = 20'b01101010100111110011; end
            9'b101101000: begin segment = 6'd21; b = 20'b01101010100111110001; end
            9'b101101001: begin segment = 6'd21; b = 20'b01101010100111110000; end
            9'b101101010: begin segment = 6'd21; b = 20'b01101010100111110001; end
            9'b101101011: begin segment = 6'd21; b = 20'b01101010100111110011; end
            9'b101101100: begin segment = 6'd21; b = 20'b01101010100111110111; end
            9'b101101101: begin segment = 6'd21; b = 20'b01101010100111111101; end
            9'b101101110: begin segment = 6'd21; b = 20'b01101010101000000100; end
            9'b101101111: begin segment = 6'd21; b = 20'b01101010101000001100; end
            9'b101110000: begin segment = 6'd21; b = 20'b01101010101000010111; end
            9'b101110001: begin segment = 6'd22; b = 20'b01101000011110101101; end
            9'b101110010: begin segment = 6'd22; b = 20'b01101000011110100010; end
            9'b101110011: begin segment = 6'd22; b = 20'b01101000011110011001; end
            9'b101110100: begin segment = 6'd22; b = 20'b01101000011110010011; end
            9'b101110101: begin segment = 6'd22; b = 20'b01101000011110001101; end
            9'b101110110: begin segment = 6'd22; b = 20'b01101000011110001000; end
            9'b101110111: begin segment = 6'd22; b = 20'b01101000011110000110; end
            9'b101111000: begin segment = 6'd22; b = 20'b01101000011110000110; end
            9'b101111001: begin segment = 6'd22; b = 20'b01101000011110000110; end
            9'b101111010: begin segment = 6'd22; b = 20'b01101000011110001000; end
            9'b101111011: begin segment = 6'd22; b = 20'b01101000011110001100; end
            9'b101111100: begin segment = 6'd22; b = 20'b01101000011110010010; end
            9'b101111101: begin segment = 6'd22; b = 20'b01101000011110011001; end
            9'b101111110: begin segment = 6'd22; b = 20'b01101000011110100001; end
            9'b101111111: begin segment = 6'd22; b = 20'b01101000011110101011; end
            9'b110000000: begin segment = 6'd23; b = 20'b01100110001101001100; end
            9'b110000001: begin segment = 6'd23; b = 20'b01100110001101000001; end
            9'b110000010: begin segment = 6'd23; b = 20'b01100110001100111000; end
            9'b110000011: begin segment = 6'd23; b = 20'b01100110001100110000; end
            9'b110000100: begin segment = 6'd23; b = 20'b01100110001100101011; end
            9'b110000101: begin segment = 6'd23; b = 20'b01100110001100100110; end
            9'b110000110: begin segment = 6'd23; b = 20'b01100110001100100100; end
            9'b110000111: begin segment = 6'd23; b = 20'b01100110001100100011; end
            9'b110001000: begin segment = 6'd23; b = 20'b01100110001100100100; end
            9'b110001001: begin segment = 6'd23; b = 20'b01100110001100100110; end
            9'b110001010: begin segment = 6'd23; b = 20'b01100110001100101010; end
            9'b110001011: begin segment = 6'd23; b = 20'b01100110001100110000; end
            9'b110001100: begin segment = 6'd23; b = 20'b01100110001100110111; end
            9'b110001101: begin segment = 6'd23; b = 20'b01100110001101000000; end
            9'b110001110: begin segment = 6'd23; b = 20'b01100110001101001011; end
            9'b110001111: begin segment = 6'd24; b = 20'b01100011110111100010; end
            9'b110010000: begin segment = 6'd24; b = 20'b01100011110111011000; end
            9'b110010001: begin segment = 6'd24; b = 20'b01100011110111010000; end
            9'b110010010: begin segment = 6'd24; b = 20'b01100011110111001001; end
            9'b110010011: begin segment = 6'd24; b = 20'b01100011110111000100; end
            9'b110010100: begin segment = 6'd24; b = 20'b01100011110111000000; end
            9'b110010101: begin segment = 6'd24; b = 20'b01100011110110111110; end
            9'b110010110: begin segment = 6'd24; b = 20'b01100011110110111110; end
            9'b110010111: begin segment = 6'd24; b = 20'b01100011110111000000; end
            9'b110011000: begin segment = 6'd24; b = 20'b01100011110111000011; end
            9'b110011001: begin segment = 6'd24; b = 20'b01100011110111001000; end
            9'b110011010: begin segment = 6'd24; b = 20'b01100011110111001110; end
            9'b110011011: begin segment = 6'd24; b = 20'b01100011110111010110; end
            9'b110011100: begin segment = 6'd24; b = 20'b01100011110111100000; end
            9'b110011101: begin segment = 6'd24; b = 20'b01100011110111101100; end
            9'b110011110: begin segment = 6'd25; b = 20'b01100001010101111110; end
            9'b110011111: begin segment = 6'd25; b = 20'b01100001010101110100; end
            9'b110100000: begin segment = 6'd25; b = 20'b01100001010101101100; end
            9'b110100001: begin segment = 6'd25; b = 20'b01100001010101100101; end
            9'b110100010: begin segment = 6'd25; b = 20'b01100001010101100000; end
            9'b110100011: begin segment = 6'd25; b = 20'b01100001010101011100; end
            9'b110100100: begin segment = 6'd25; b = 20'b01100001010101011011; end
            9'b110100101: begin segment = 6'd25; b = 20'b01100001010101011011; end
            9'b110100110: begin segment = 6'd25; b = 20'b01100001010101011101; end
            9'b110100111: begin segment = 6'd25; b = 20'b01100001010101100000; end
            9'b110101000: begin segment = 6'd25; b = 20'b01100001010101100101; end
            9'b110101001: begin segment = 6'd25; b = 20'b01100001010101101100; end
            9'b110101010: begin segment = 6'd25; b = 20'b01100001010101110101; end
            9'b110101011: begin segment = 6'd25; b = 20'b01100001010101111111; end
            9'b110101100: begin segment = 6'd26; b = 20'b01011110110101001000; end
            9'b110101101: begin segment = 6'd26; b = 20'b01011110110100111101; end
            9'b110101110: begin segment = 6'd26; b = 20'b01011110110100110101; end
            9'b110101111: begin segment = 6'd26; b = 20'b01011110110100101110; end
            9'b110110000: begin segment = 6'd26; b = 20'b01011110110100101000; end
            9'b110110001: begin segment = 6'd26; b = 20'b01011110110100100100; end
            9'b110110010: begin segment = 6'd26; b = 20'b01011110110100100011; end
            9'b110110011: begin segment = 6'd26; b = 20'b01011110110100100011; end
            9'b110110100: begin segment = 6'd26; b = 20'b01011110110100100100; end
            9'b110110101: begin segment = 6'd26; b = 20'b01011110110100101000; end
            9'b110110110: begin segment = 6'd26; b = 20'b01011110110100101101; end
            9'b110110111: begin segment = 6'd26; b = 20'b01011110110100110100; end
            9'b110111000: begin segment = 6'd26; b = 20'b01011110110100111101; end
            9'b110111001: begin segment = 6'd26; b = 20'b01011110110101000111; end
            9'b110111010: begin segment = 6'd27; b = 20'b01011100001100010011; end
            9'b110111011: begin segment = 6'd27; b = 20'b01011100001100001000; end
            9'b110111100: begin segment = 6'd27; b = 20'b01011100001011111111; end
            9'b110111101: begin segment = 6'd27; b = 20'b01011100001011111000; end
            9'b110111110: begin segment = 6'd27; b = 20'b01011100001011110011; end
            9'b110111111: begin segment = 6'd27; b = 20'b01011100001011101111; end
            9'b111000000: begin segment = 6'd27; b = 20'b01011100001011101110; end
            9'b111000001: begin segment = 6'd27; b = 20'b01011100001011101101; end
            9'b111000010: begin segment = 6'd27; b = 20'b01011100001011110000; end
            9'b111000011: begin segment = 6'd27; b = 20'b01011100001011110011; end
            9'b111000100: begin segment = 6'd27; b = 20'b01011100001011111000; end
            9'b111000101: begin segment = 6'd27; b = 20'b01011100001011111111; end
            9'b111000110: begin segment = 6'd27; b = 20'b01011100001100001000; end
            9'b111000111: begin segment = 6'd27; b = 20'b01011100001100010010; end
            9'b111001000: begin segment = 6'd28; b = 20'b01011001011010001010; end
            9'b111001001: begin segment = 6'd28; b = 20'b01011001011010000000; end
            9'b111001010: begin segment = 6'd28; b = 20'b01011001011001110111; end
            9'b111001011: begin segment = 6'd28; b = 20'b01011001011001110000; end
            9'b111001100: begin segment = 6'd28; b = 20'b01011001011001101010; end
            9'b111001101: begin segment = 6'd28; b = 20'b01011001011001100111; end
            9'b111001110: begin segment = 6'd28; b = 20'b01011001011001100100; end
            9'b111001111: begin segment = 6'd28; b = 20'b01011001011001100101; end
            9'b111010000: begin segment = 6'd28; b = 20'b01011001011001100110; end
            9'b111010001: begin segment = 6'd28; b = 20'b01011001011001101010; end
            9'b111010010: begin segment = 6'd28; b = 20'b01011001011001101111; end
            9'b111010011: begin segment = 6'd28; b = 20'b01011001011001110111; end
            9'b111010100: begin segment = 6'd28; b = 20'b01011001011001111111; end
            9'b111010101: begin segment = 6'd28; b = 20'b01011001011010001011; end
            9'b111010110: begin segment = 6'd29; b = 20'b01010110011110111000; end
            9'b111010111: begin segment = 6'd29; b = 20'b01010110011110101101; end
            9'b111011000: begin segment = 6'd29; b = 20'b01010110011110100100; end
            9'b111011001: begin segment = 6'd29; b = 20'b01010110011110011100; end
            9'b111011010: begin segment = 6'd29; b = 20'b01010110011110010111; end
            9'b111011011: begin segment = 6'd29; b = 20'b01010110011110010011; end
            9'b111011100: begin segment = 6'd29; b = 20'b01010110011110010010; end
            9'b111011101: begin segment = 6'd29; b = 20'b01010110011110010010; end
            9'b111011110: begin segment = 6'd29; b = 20'b01010110011110010011; end
            9'b111011111: begin segment = 6'd29; b = 20'b01010110011110010111; end
            9'b111100000: begin segment = 6'd29; b = 20'b01010110011110011100; end
            9'b111100001: begin segment = 6'd29; b = 20'b01010110011110100100; end
            9'b111100010: begin segment = 6'd29; b = 20'b01010110011110101101; end
            9'b111100011: begin segment = 6'd29; b = 20'b01010110011110111000; end
            9'b111100100: begin segment = 6'd30; b = 20'b01010011011010001111; end
            9'b111100101: begin segment = 6'd30; b = 20'b01010011011010000100; end
            9'b111100110: begin segment = 6'd30; b = 20'b01010011011001111011; end
            9'b111100111: begin segment = 6'd30; b = 20'b01010011011001110011; end
            9'b111101000: begin segment = 6'd30; b = 20'b01010011011001101101; end
            9'b111101001: begin segment = 6'd30; b = 20'b01010011011001101001; end
            9'b111101010: begin segment = 6'd30; b = 20'b01010011011001100111; end
            9'b111101011: begin segment = 6'd30; b = 20'b01010011011001100111; end
            9'b111101100: begin segment = 6'd30; b = 20'b01010011011001101000; end
            9'b111101101: begin segment = 6'd30; b = 20'b01010011011001101100; end
            9'b111101110: begin segment = 6'd30; b = 20'b01010011011001110001; end
            9'b111101111: begin segment = 6'd30; b = 20'b01010011011001111001; end
            9'b111110000: begin segment = 6'd30; b = 20'b01010011011010000010; end
            9'b111110001: begin segment = 6'd30; b = 20'b01010011011010001101; end
            9'b111110010: begin segment = 6'd31; b = 20'b01010000001101111100; end
            9'b111110011: begin segment = 6'd31; b = 20'b01010000001101110001; end
            9'b111110100: begin segment = 6'd31; b = 20'b01010000001101100111; end
            9'b111110101: begin segment = 6'd31; b = 20'b01010000001101011111; end
            9'b111110110: begin segment = 6'd31; b = 20'b01010000001101011010; end
            9'b111110111: begin segment = 6'd31; b = 20'b01010000001101010110; end
            9'b111111000: begin segment = 6'd31; b = 20'b01010000001101010011; end
            9'b111111001: begin segment = 6'd31; b = 20'b01010000001101010011; end
            9'b111111010: begin segment = 6'd31; b = 20'b01010000001101010110; end
            9'b111111011: begin segment = 6'd31; b = 20'b01010000001101011010; end
            9'b111111100: begin segment = 6'd31; b = 20'b01010000001101011111; end
            9'b111111101: begin segment = 6'd31; b = 20'b01010000001101100111; end
            9'b111111110: begin segment = 6'd31; b = 20'b01010000001101110000; end
            9'b111111111: begin segment = 6'd31; b = 20'b01010000001101111100; end
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


// Auto-generated register declarations for 5 registers
    reg [19:0] A1;
    reg [19:0] A2;
    reg [19:0] A3;
    reg [19:0] A4;
    reg [19:0] A5;
// End of auto-generated section

    always @(*) begin
        case(segment)
            6'd0: begin A1 = {3'b111, neg_17}; A2 = {5'b11111, neg_15}; A3 = {7'b0000000, f[21:9]}; A4 = {10'b1111111111, neg_10}; end
            6'd1: begin A1 = {3'b111, neg_17}; A2 = {6'b111111, neg_14}; A3 = {11'b11111111111, neg_9}; A4 = {13'b1111111111111, neg_7}; end
            6'd2: begin A1 = {3'b111, neg_17}; A2 = {7'b1111111, neg_13}; A3 = {10'b0000000000, f[21:12]}; A4 = {14'b11111111111111, neg_6}; end
            6'd3: begin A1 = {3'b111, neg_17}; A2 = {9'b000000000, f[21:11]}; A3 = {11'b00000000000, f[21:13]}; A4 = {19'b1111111111111111111, neg_1}; end
            6'd4: begin A1 = {3'b111, neg_17}; A2 = {6'b000000, f[21:8]}; A3 = {8'b11111111, neg_12}; A4 = {15'b000000000000000, f[21:17]}; end
            6'd5: begin A1 = {3'b111, neg_17}; A2 = {5'b00000, f[21:7]}; A3 = {7'b1111111, neg_13}; A4 = {9'b111111111, neg_11}; end
            6'd6: begin A1 = {3'b111, neg_17}; A2 = {5'b00000, f[21:7]}; A3 = {12'b111111111111, neg_8}; A4 = {14'b00000000000000, f[21:16]}; end
            6'd7: begin A1 = {3'b111, neg_17}; A2 = {5'b00000, f[21:7]}; A3 = {7'b0000000, f[21:9]}; A4 = {9'b000000000, f[21:11]}; end
            6'd8: begin A1 = {4'b1111, neg_16}; A2 = {6'b111111, neg_14}; A3 = {8'b00000000, f[21:10]}; A4 = {12'b000000000000, f[21:14]}; end
            6'd9: begin A1 = {4'b1111, neg_16}; A2 = {9'b111111111, neg_11}; A3 = {11'b00000000000, f[21:13]}; A4 = {14'b11111111111111, neg_6}; end
            6'd10: begin A1 = {4'b1111, neg_16}; A2 = {7'b0000000, f[21:9]}; A3 = {10'b0000000000, f[21:12]}; A4 = {12'b111111111111, neg_8}; end
            6'd11: begin A1 = {4'b1111, neg_16}; A2 = {6'b000000, f[21:8]}; A3 = {8'b00000000, f[21:10]}; A4 = {11'b11111111111, neg_9}; end
            6'd12: begin A1 = {5'b11111, neg_15}; A2 = {9'b111111111, neg_11}; A3 = {11'b00000000000, f[21:13]}; A4 = {13'b1111111111111, neg_7}; end
            6'd13: begin A1 = {5'b11111, neg_15}; A2 = {7'b0000000, f[21:9]}; A3 = {10'b0000000000, f[21:12]}; A4 = {13'b0000000000000, f[21:15]}; end
            6'd14: begin A1 = {6'b111111, neg_14}; A2 = {8'b00000000, f[21:10]}; A3 = {13'b1111111111111, neg_7}; A4 = {17'b11111111111111111, neg_3}; end
            6'd15: begin A1 = {10'b1111111111, neg_10}; A2 = {12'b111111111111, neg_8}; A3 = {14'b00000000000000, f[21:16]}; A4 = {20'b00000000000000000000};end
            6'd16: begin A1 = {7'b0000000, f[21:9]}; A2 = {9'b000000000, f[21:11]}; A3 = {20'b00000000000000000000};A4 = {20'b00000000000000000000};end
            6'd17: begin A1 = {5'b00000, f[21:7]}; A2 = {7'b1111111, neg_13}; A3 = {9'b111111111, neg_11}; A4 = {11'b11111111111, neg_9}; end
            6'd18: begin A1 = {5'b00000, f[21:7]}; A2 = {10'b0000000000, f[21:12]}; A3 = {12'b111111111111, neg_8}; A4 = {16'b1111111111111111, neg_4}; end
            6'd19: begin A1 = {4'b0000, f[21:6]}; A2 = {6'b111111, neg_14}; A3 = {8'b11111111, neg_12}; A4 = {13'b1111111111111, neg_7}; end
            6'd20: begin A1 = {4'b0000, f[21:6]}; A2 = {7'b1111111, neg_13}; A3 = {10'b1111111111, neg_10}; A4 = {12'b000000000000, f[21:14]}; end
            6'd21: begin A1 = {4'b0000, f[21:6]}; A2 = {8'b00000000, f[21:10]}; A3 = {10'b1111111111, neg_10}; A4 = {14'b11111111111111, neg_6}; end
            6'd22: begin A1 = {4'b0000, f[21:6]}; A2 = {6'b000000, f[21:8]}; A3 = {10'b1111111111, neg_10}; A4 = {13'b1111111111111, neg_7}; end
            6'd23: begin A1 = {3'b000, f[21:5]}; A2 = {5'b11111, neg_15}; A3 = {8'b11111111, neg_12}; A4 = {10'b1111111111, neg_10}; end
            6'd24: begin A1 = {3'b000, f[21:5]}; A2 = {5'b11111, neg_15}; A3 = {7'b0000000, f[21:9]}; A4 = {10'b1111111111, neg_10}; end
            6'd25: begin A1 = {3'b000, f[21:5]}; A2 = {6'b111111, neg_14}; A3 = {8'b00000000, f[21:10]}; A4 = {11'b11111111111, neg_9}; end
            6'd26: begin A1 = {3'b000, f[21:5]}; A2 = {11'b11111111111, neg_9}; A3 = {15'b000000000000000, f[21:17]}; A4 = {18'b111111111111111111, neg_2}; end
            6'd27: begin A1 = {3'b000, f[21:5]}; A2 = {6'b000000, f[21:8]}; A3 = {8'b11111111, neg_12}; A4 = {12'b111111111111, neg_8}; end
            6'd28: begin A1 = {3'b000, f[21:5]}; A2 = {5'b00000, f[21:7]}; A3 = {7'b1111111, neg_13}; A4 = {12'b000000000000, f[21:14]}; end
            6'd29: begin A1 = {3'b000, f[21:5]}; A2 = {5'b00000, f[21:7]}; A3 = {8'b00000000, f[21:10]}; A4 = {10'b0000000000, f[21:12]}; end
            6'd30: begin A1 = {2'b00, f[21:4]}; A2 = {4'b1111, neg_16}; A3 = {6'b111111, neg_14}; A4 = {9'b000000000, f[21:11]}; end
            6'd31: begin A1 = {2'b00, f[21:4]}; A2 = {4'b1111, neg_16}; A3 = {10'b1111111111, neg_10}; A4 = {13'b0000000000000, f[21:15]}; end
      endcase
    end

// Auto-generated CSA tree for final_N=5, final_M=22, final_add_M=19
    wire [19:0] csa1_carry, csa1_sum;
    wire [19:0] csa2_carry, csa2_sum;
    wire [19:0] csa3_carry, csa3_sum;
    wire [19:0] csa4_carry, csa4_sum;
    wire [19:0] final_sum;

    CSA_anticonv3 csa1 (
        .a({1'b0, f[21:3]}),  // f的高19位
        .b(A1),
        .c(A2),
        .sum(csa1_sum),
        .carry(csa1_carry)
    );

    CSA_anticonv3 csa2 (
        .a(csa1_sum),
        .b(A3),
        .c({csa1_carry[18:0], 1'b0}),  // 左移1位
        .sum(csa2_sum),
        .carry(csa2_carry)
    );

    CSA_anticonv3 csa3 (
        .a(csa2_sum),
        .b(A4),
        .c({csa2_carry[18:0], 1'b0}),  // 左移1位
        .sum(csa3_sum),
        .carry(csa3_carry)
    );

    CSA_anticonv3 csa4 (
        .a(csa3_sum),
         .b(b),
        .c({csa3_carry[18:0], 1'b0}),
        .sum(csa4_sum),
        .carry(csa4_carry)
    );

    CPA_anticonv3 cpa (
        .a(csa4_sum),
        .b({csa4_carry[18:0], 1'b0}),  // 左移1位
        .sum(final_sum)
    );

    assign f_e2_out = {final_sum, f[2:0]};  // 拼接高位和原始低位
// End of auto-generated CSA tree

endmodule
module CSA_anticonv3 #(parameter ADD_WIDTH = 20
)(
    input [ADD_WIDTH-1:0] a,
    input [ADD_WIDTH-1:0] b,
    input [ADD_WIDTH-1:0] c,
    output [ADD_WIDTH-1:0] sum,
    output [ADD_WIDTH-1:0] carry
);
    assign sum = a ^ b ^ c;          // XOR for sum
    assign carry = (a & b) | (b & c) | (c & a); // Majority logic for carry
endmodule

// Carry-Propagate Adder (CPA) module
module CPA_anticonv3 #(parameter ADD_WIDTH = 20
)(
    input [ADD_WIDTH-1:0] a,
    input [ADD_WIDTH-1:0] b,
    output [ADD_WIDTH-1:0] sum
);
    assign sum = a + b;  // Simple binary addition
endmodule


