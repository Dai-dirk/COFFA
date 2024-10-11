
autoidx 79

module \sym_128_128

  attribute \keep "coarse_wire_10_0"
  wire width 32 $10_0

  attribute \keep "coarse_wire_11_0"
  wire width 32 $11_0

  attribute \keep "coarse_wire_12_0"
  wire width 32 $12_0

  attribute \keep "coarse_wire_13_0"
  wire width 32 $13_0

  attribute \keep "coarse_wire_14_0"
  wire width 32 $14_0

  attribute \keep "coarse_wire_15_0"
  wire width 32 $15_0

  attribute \keep "coarse_wire_16_0"
  wire width 32 $16_0

  attribute \keep "coarse_wire_17_0"
  wire width 32 $17_0

  attribute \keep "coarse_wire_18_0"
  wire width 32 $18_0

  attribute \keep "coarse_wire_19_0"
  wire width 32 $19_0

  attribute \keep "coarse_wire_20_0"
  wire width 32 $20_0

  attribute \keep "coarse_wire_21_0"
  wire width 32 $21_0

  attribute \keep "coarse_wire_22_0"
  wire width 32 $22_0

  attribute \keep "coarse_wire_23_0"
  wire width 32 $23_0

  attribute \keep "coarse_wire_3_0"
  wire width 32 $3_0

  wire $4_0

  attribute \keep "coarse_wire_5_0"
  wire width 32 $5_0

  attribute \keep "coarse_wire_6_0"
  wire width 32 $6_0

  attribute \keep "coarse_wire_7_0"
  wire width 32 $7_0

  attribute \keep "coarse_wire_8_0"
  wire width 32 $8_0

  attribute \keep "coarse_wire_9_0"
  wire width 32 $9_0

  attribute \keep "coarse_node_20"
  cell \ACC $ACC72
    parameter \A_WIDTH 32
    parameter \Y_WIDTH 32
    parameter \acc_first "0"
    parameter \acc_params "0,128,64,1"
    connect \A $17_0
    connect \Y $20_0
  end

  attribute \keep "coarse_node_21"
  cell \ACC $ACC76
    parameter \A_WIDTH 32
    parameter \Y_WIDTH 32
    parameter \acc_first "0"
    parameter \acc_params "0,64,1,128"
    connect \A $19_0
    connect \Y $21_0
  end

  attribute \keep "coarse_node_11"
  cell \ADD $ADD40
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $7_0
    connect \B $10_0
    connect \Y $11_0
  end

  attribute \keep "coarse_node_16"
  cell \ADD $ADD49
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $13_0
    connect \B $15_0
    connect \Y $16_0
  end

  attribute \keep "node_22"
  cell \CACC $CACC78
    parameter \A_WIDTH 32
    parameter \B_WIDTH 1
    parameter \Y_WIDTH 32
    parameter \acc_first "1"
    parameter \acc_params "0,64,1,128"
    connect \A $16_0
    connect \B $4_0
    connect \Y $22_0
  end

  attribute \keep "node_23"
  cell \CACC $CACC80
    parameter \A_WIDTH 32
    parameter \B_WIDTH 1
    parameter \Y_WIDTH 32
    parameter \acc_first "1"
    parameter \acc_params "0,64,1,128"
    connect \A $11_0
    connect \B $4_0
    connect \Y $23_0
  end

  attribute \keep "node_5"
  cell \CINPUT $CInput31
    parameter \A_WIDTH 1
    parameter \Y_WIDTH 32
    parameter \multiport "1"
    parameter \offset "0,0"
    parameter \pattern "8,64,-504.000000,128"
    parameter \ref_name "lpr"
    parameter \size "512"
    connect \A $4_0
    connect \Y $5_0
  end

  attribute \keep "node_6"
  cell \CINPUT $CInput33
    parameter \A_WIDTH 1
    parameter \Y_WIDTH 32
    parameter \multiport "1"
    parameter \offset "0,0"
    parameter \pattern "-4,64,256,128"
    parameter \ref_name "cA"
    parameter \size "512"
    connect \A $4_0
    connect \Y $6_0
  end

  attribute \keep "node_8"
  cell \CINPUT $CInput36
    parameter \A_WIDTH 1
    parameter \Y_WIDTH 32
    parameter \multiport "1"
    parameter \offset "0,0"
    parameter \pattern "8,64,-504.000000,128"
    parameter \ref_name "hpr"
    parameter \size "512"
    connect \A $4_0
    connect \Y $8_0
  end

  attribute \keep "node_9"
  cell \CINPUT $CInput38
    parameter \A_WIDTH 1
    parameter \Y_WIDTH 32
    parameter \multiport "1"
    parameter \offset "0,0"
    parameter \pattern "-4,64,256,128"
    parameter \ref_name "cD"
    parameter \size "512"
    connect \A $4_0
    connect \Y $9_0
  end

  attribute \keep "node_12"
  cell \CINPUT $CInput44
    parameter \A_WIDTH 1
    parameter \Y_WIDTH 32
    parameter \multiport "1"
    parameter \offset "0,4"
    parameter \pattern "8,64,-504.000000,128"
    parameter \ref_name "lpr"
    parameter \size "512"
    connect \A $4_0
    connect \Y $12_0
  end

  attribute \keep "node_14"
  cell \CINPUT $CInput47
    parameter \A_WIDTH 1
    parameter \Y_WIDTH 32
    parameter \multiport "1"
    parameter \offset "0,4"
    parameter \pattern "8,64,-504.000000,128"
    parameter \ref_name "hpr"
    parameter \size "512"
    connect \A $4_0
    connect \Y $14_0
  end

  attribute \keep "coarse_node_17"
  cell \CONST $CONST56
    parameter \VALUE 1
    parameter \Y_WIDTH 32
    connect \Y $17_0
  end

  attribute \keep "coarse_node_18"
  cell \CONST $CONST59
    parameter \VALUE 128
    parameter \Y_WIDTH 32
    connect \Y $18_0
  end

  attribute \keep "coarse_node_19"
  cell \CONST $CONST60
    parameter \VALUE 1
    parameter \Y_WIDTH 32
    connect \Y $19_0
  end

  attribute \keep "coarse_node_7"
  cell \MUL $MUL34
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $6_0
    connect \B $5_0
    connect \Y $7_0
  end

  attribute \keep "coarse_node_10"
  cell \MUL $MUL39
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $9_0
    connect \B $8_0
    connect \Y $10_0
  end

  attribute \keep "coarse_node_13"
  cell \MUL $MUL45
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $12_0
    connect \B $6_0
    connect \Y $13_0
  end

  attribute \keep "coarse_node_15"
  cell \MUL $MUL48
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $14_0
    connect \B $9_0
    connect \Y $15_0
  end

  attribute \keep "coarse_node_1"
  cell \OUTPUT $Output12
    parameter \A_WIDTH 32
    parameter \multiport "1"
    parameter \offset "0,0"
    parameter \pattern "0,64,16,128"
    parameter \ref_name "X"
    parameter \size "2044"
    connect \A $23_0
  end

  attribute \keep "coarse_node_2"
  cell \OUTPUT $Output14
    parameter \A_WIDTH 32
    parameter \multiport "1"
    parameter \offset "0,8"
    parameter \pattern "0,64,16,128"
    parameter \ref_name "X"
    parameter \size "2044"
    connect \A $22_0
  end

  attribute \keep "coarse_node_3"
  cell \SUB $SUB22
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $20_0
    connect \B $21_0
    connect \Y $3_0
  end

  attribute \keep "node_4"
  cell \ULT $ULT23
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 1
    connect \A $3_0
    connect \B $18_0
    connect \Y $4_0
  end
end
