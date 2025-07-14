//Yosys cell library, which are the coarse-grained operations. 
//Yosys treats them as black boxes and doesn't change them during the synthesis process.

//OPC:0
(* blackbox *)
module \PASS (A, Y);
parameter A_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
output [Y_WIDTH-1:0] Y;

endmodule

//OPC:1
(* blackbox *)
module \ADD (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
        
endmodule

//OPC:2
(* blackbox *)
module \SUB (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [A_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;

endmodule

//OPC:3
(* blackbox *)
module \MUL (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

//OPC:4
(* blackbox *)
module \AND (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

//OPC:5
(* blackbox *)
module \OR (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

//OPC:6
(* blackbox *)
module \XOR (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

//OPC:7
(* blackbox *)
module \SHL (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

//OPC:8
(* blackbox *)
module \LSHR (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

//OPC:9
(* blackbox *)
module \ASHR (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

//OPC:10
(* blackbox *)
module \EQ (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

//OPC:11
(* blackbox *)
module \NE (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

//OPC:12
(* blackbox *)
module \LT (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

//OPC:13
(* blackbox *)
module \LE (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

//OPC:14
(* blackbox *)
module \IMUL (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

//OPC:15
(* blackbox *)
module \ILE (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

//OPC:16
(* blackbox *)
module \ILT (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

//OPC:17
(* blackbox *)
module \SEL (A, B, C, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter C_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
input [C_WIDTH-1:0] C;
output [Y_WIDTH-1:0] Y;
    
endmodule

//OPC:18
(* blackbox *)
module \LOAD (A, Y);
parameter A_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
output [Y_WIDTH-1:0] Y;
    
endmodule

//OPC:19
(* blackbox *)
module \STORE (A, B);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
    
endmodule

//OPC:20
(* blackbox *)
module \NOT (A, Y);
parameter A_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \UDIV (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
        
endmodule

(* blackbox *)
module \SLT (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \INPUT (Y);
parameter Y_WIDTH = 16;
    
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \OUTPUT (A);
parameter A_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
    
endmodule


(* blackbox *)
module \CLOAD (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule


(* blackbox *)
module \CSTORE (A, B, C);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter C_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
input [C_WIDTH-1:0] C;
    
endmodule

(* blackbox *)
module \CINPUT (A, Y);
parameter A_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \COUTPUT (A, B);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;    
endmodule

(* blackbox *)
module \TLOAD (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule


(* blackbox *)
module \TSTORE (A, B, C);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter C_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
input [C_WIDTH-1:0] C;
    
endmodule

(* blackbox *)
module \TCLOAD (A, B, C, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter C_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
input [C_WIDTH-1:0] C;
output [Y_WIDTH-1:0] Y;
    
endmodule


(* blackbox *)
module \TCSTORE (A, B, C, D);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter C_WIDTH = 16;
parameter D_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
input [C_WIDTH-1:0] C;
input [D_WIDTH-1:0] D;
    
endmodule


(* blackbox *)
module \CONST (Y);
parameter Y_WIDTH = 16;
parameter VALUE = 16;
output [Y_WIDTH-1:0] Y;


endmodule


(* blackbox *)
module \ULE (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \ULT (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \ACC (A, Y);
parameter A_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \ASUB (A, Y);
parameter A_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \AMUL (A, Y);
parameter A_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \ADIV (A, Y);
parameter A_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \AMOD (A, Y);
parameter A_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \AAND (A, Y);
parameter A_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \AOR (A, Y);
parameter A_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \AXOR (A, Y);
parameter A_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \ASHL (A, Y);
parameter A_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \ALSHR (A, Y);
parameter A_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \AASHR (A, Y);
parameter A_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CACC (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CASUB (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CAMUL (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CADIV (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CAMOD (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CAAND (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CAOR (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CAXOR (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CASHL (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CALSHR (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CAASHR (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CIACC (A, B, C, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter C_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
input [C_WIDTH-1:0] C;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CIASUB (A, B, C, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter C_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
input [C_WIDTH-1:0] C;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CIAMUL (A, B, C, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter C_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
input [C_WIDTH-1:0] C;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CIADIV (A, B, C, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter C_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
input [C_WIDTH-1:0] C;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CIAMOD (A, B, C, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter C_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
input [C_WIDTH-1:0] C;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CIAAND (A, B, C, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter C_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
input [C_WIDTH-1:0] C;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CIAAND (A, B, C, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter C_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
input [C_WIDTH-1:0] C;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CIAOR (A, B, C, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter C_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
input [C_WIDTH-1:0] C;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CIAXOR (A, B, C, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter C_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
input [C_WIDTH-1:0] C;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CIASHL (A, B, C, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter C_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
input [C_WIDTH-1:0] C;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CIALSHR (A, B, C, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter C_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
input [C_WIDTH-1:0] C;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CIAASHR (A, B, C, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter C_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
input [C_WIDTH-1:0] C;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CDIACC (A, B, C, D, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter C_WIDTH = 16;
parameter D_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
input [C_WIDTH-1:0] C;
input [D_WIDTH-1:0] D;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \ISEL (A, B, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \CISEL (A, B, C, Y);
parameter A_WIDTH = 16;
parameter B_WIDTH = 16;
parameter C_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
input [B_WIDTH-1:0] B;
input [C_WIDTH-1:0] C;
output [Y_WIDTH-1:0] Y;
    
endmodule

(* blackbox *)
module \SEXT (A, Y);
parameter A_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
output [Y_WIDTH-1:0] Y;
    
endmodule


(* blackbox *)
module \ZEXT (A, Y);
parameter A_WIDTH = 16;
parameter Y_WIDTH = 16;
    
input [A_WIDTH-1:0] A;
output [Y_WIDTH-1:0] Y;
    
endmodule