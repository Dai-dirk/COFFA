
autoidx 83

module \cnn

  wire $10_0

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

  attribute \keep "coarse_wire_1_0"
  wire width 32 $1_0

  attribute \keep "coarse_wire_20_0"
  wire width 32 $20_0

  attribute \keep "coarse_wire_21_0"
  wire width 32 $21_0

  attribute \keep "coarse_wire_22_0"
  wire width 32 $22_0

  attribute \keep "coarse_wire_23_0"
  wire width 32 $23_0

  attribute \keep "coarse_wire_24_0"
  wire width 32 $24_0

  wire $25_0

  attribute \keep "coarse_wire_26_0"
  wire width 32 $26_0

  attribute \keep "coarse_wire_27_0"
  wire width 32 $27_0

  attribute \keep "coarse_wire_28_0"
  wire width 32 $28_0

  attribute \keep "coarse_wire_29_0"
  wire width 32 $29_0

  attribute \keep "coarse_wire_2_0"
  wire width 32 $2_0

  attribute \keep "coarse_wire_30_0"
  wire width 32 $30_0

  attribute \keep "coarse_wire_31_0"
  wire width 32 $31_0

  attribute \keep "coarse_wire_32_0"
  wire width 32 $32_0

  attribute \keep "coarse_wire_33_0"
  wire width 32 $33_0

  attribute \keep "coarse_wire_34_0"
  wire width 32 $34_0

  attribute \keep "coarse_wire_4_0"
  wire width 32 $4_0

  attribute \keep "coarse_wire_6_0"
  wire width 32 $6_0

  wire $7_0

  attribute \keep "coarse_wire_8_0"
  wire width 32 $8_0

  wire $9_0

  attribute \keep "coarse_node_26"
  cell \ACC $ACC72
    parameter \A_WIDTH 32
    parameter \Y_WIDTH 32
    parameter \acc_first "0"
    parameter \acc_params "0,256,121,1"
    connect \A $19_0
    connect \Y $26_0
  end

  attribute \keep "coarse_node_27"
  cell \ACC $ACC76
    parameter \A_WIDTH 32
    parameter \Y_WIDTH 32
    parameter \acc_first "1"
    parameter \acc_params "0,121,1,256"
    connect \A $12_0
    connect \Y $27_0
  end

  attribute \keep "coarse_node_28"
  cell \ACC $ACC78
    parameter \A_WIDTH 32
    parameter \Y_WIDTH 32
    parameter \acc_first "1"
    parameter \acc_params "0,121,1,256"
    connect \A $13_0
    connect \Y $28_0
  end

  attribute \keep "coarse_node_29"
  cell \ACC $ACC80
    parameter \A_WIDTH 32
    parameter \Y_WIDTH 32
    parameter \acc_first "0"
    parameter \acc_params "0,11,11,256"
    connect \A $22_0
    connect \Y $29_0
  end

  attribute \keep "coarse_node_30"
  cell \ACC $ACC86
    parameter \A_WIDTH 32
    parameter \Y_WIDTH 32
    parameter \acc_first "0"
    parameter \acc_params "0,11,1,2816"
    connect \A $24_0
    connect \Y $30_0
  end

  attribute \keep "coarse_node_6"
  cell \ADD $ADD19
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $29_0
    connect \B $20_0
    connect \Y $6_0
  end

  attribute \keep "coarse_node_1"
  cell \ADD $ADD2
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $26_0
    connect \B $16_0
    connect \Y $1_0
  end

  attribute \keep "coarse_node_8"
  cell \ADD $ADD30
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $1_0
    connect \B $30_0
    connect \Y $8_0
  end

  attribute \keep "node_14"
  cell \CINPUT $CInput48
    parameter \A_WIDTH 1
    parameter \Y_WIDTH 32
    parameter \multiport "0"
    parameter \offset "0,-100"
    parameter \pattern "16,11,-156.000000,11,-184,256"
    parameter \ref_name "uniformH"
    parameter \size "4184"
    connect \A $25_0
    connect \Y $14_0
  end

  attribute \keep "node_15"
  cell \CINPUT $CInput50
    parameter \A_WIDTH 1
    parameter \Y_WIDTH 32
    parameter \multiport "0"
    parameter \offset "0,-100"
    parameter \pattern "16,11,-156.000000,11,-184,256"
    parameter \ref_name "uniformV"
    parameter \size "4184"
    connect \A $25_0
    connect \Y $15_0
  end

  attribute \keep "coarse_node_16"
  cell \CONST $CONST52
    parameter \VALUE 32'11111111111111111111111111111011
    parameter \Y_WIDTH 32
    connect \Y $16_0
  end

  attribute \keep "coarse_node_17"
  cell \CONST $CONST54
    parameter \VALUE 15
    parameter \Y_WIDTH 32
    connect \Y $17_0
  end

  attribute \keep "coarse_node_18"
  cell \CONST $CONST55
    parameter \VALUE 15
    parameter \Y_WIDTH 32
    connect \Y $18_0
  end

  attribute \keep "coarse_node_19"
  cell \CONST $CONST56
    parameter \VALUE 1
    parameter \Y_WIDTH 32
    connect \Y $19_0
  end

  attribute \keep "coarse_node_20"
  cell \CONST $CONST58
    parameter \VALUE 32'11111111111111111111111111111011
    parameter \Y_WIDTH 32
    connect \Y $20_0
  end

  attribute \keep "coarse_node_21"
  cell \CONST $CONST59
    parameter \VALUE 3
    parameter \Y_WIDTH 32
    connect \Y $21_0
  end

  attribute \keep "coarse_node_22"
  cell \CONST $CONST60
    parameter \VALUE 1
    parameter \Y_WIDTH 32
    connect \Y $22_0
  end

  attribute \keep "coarse_node_23"
  cell \CONST $CONST62
    parameter \VALUE 255
    parameter \Y_WIDTH 32
    connect \Y $23_0
  end

  attribute \keep "coarse_node_24"
  cell \CONST $CONST63
    parameter \VALUE 1
    parameter \Y_WIDTH 32
    connect \Y $24_0
  end

  attribute \keep "coarse_node_31"
  cell \CONST $CONST87
    parameter \VALUE 0
    parameter \Y_WIDTH 32
    connect \Y $31_0
  end

  attribute \keep "coarse_node_33"
  cell \CONST $CONST89
    parameter \VALUE 0
    parameter \Y_WIDTH 32
    connect \Y $33_0
  end

  attribute \keep "coarse_node_11"
  cell \INPUT $Input37
    parameter \Y_WIDTH 32
    parameter \multiport "0"
    parameter \offset "0,0"
    parameter \pattern "64,11,-636,11,-680,256"
    parameter \ref_name "m_GaussianKernel"
    parameter \size "684"
    connect \Y $11_0
  end

  attribute \keep "coarse_node_4"
  cell \MUL $MUL10
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $28_0
    connect \B $18_0
    connect \Y $4_0
  end

  attribute \keep "coarse_node_12"
  cell \MUL $MUL38
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $11_0
    connect \B $34_0
    connect \Y $12_0
  end

  attribute \keep "coarse_node_13"
  cell \MUL $MUL40
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $11_0
    connect \B $32_0
    connect \Y $13_0
  end

  attribute \keep "coarse_node_2"
  cell \MUL $MUL7
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $27_0
    connect \B $17_0
    connect \Y $2_0
  end

  attribute \keep "coarse_node_5"
  cell \OUTPUT $Output12
    parameter \A_WIDTH 32
    parameter \multiport "0"
    parameter \offset "0,0"
    parameter \pattern "0,11,0,11,16,256"
    parameter \ref_name "m_DispV"
    parameter \size "4084"
    connect \A $4_0
  end

  attribute \keep "coarse_node_3"
  cell \OUTPUT $Output9
    parameter \A_WIDTH 32
    parameter \multiport "0"
    parameter \offset "0,0"
    parameter \pattern "0,11,0,11,16,256"
    parameter \ref_name "m_DispH"
    parameter \size "4084"
    connect \A $2_0
  end

  attribute \keep "node_32"
  cell \SEL $SEL88
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \C_WIDTH 1
    parameter \Y_WIDTH 32
    connect \A $31_0
    connect \B $15_0
    connect \C $10_0
    connect \Y $32_0
  end

  attribute \keep "node_34"
  cell \SEL $SEL90
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \C_WIDTH 1
    parameter \Y_WIDTH 32
    connect \A $33_0
    connect \B $14_0
    connect \C $10_0
    connect \Y $34_0
  end

  attribute \keep "node_7"
  cell \ULT $ULT20
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 1
    connect \A $21_0
    connect \B $6_0
    connect \Y $7_0
  end

  attribute \keep "node_9"
  cell \ULT $ULT31
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 1
    connect \A $23_0
    connect \B $8_0
    connect \Y $9_0
  end

  cell $lut $abc$80$auto$blifparse.cc:518:parse_blif$81
    parameter \LUT 4'0001
    parameter \WIDTH 2
    connect \A { $9_0 $7_0 }
    connect \Y $25_0
  end

  cell $lut $abc$80$auto$blifparse.cc:518:parse_blif$82
    parameter \LUT 4'1110
    parameter \WIDTH 2
    connect \A { $9_0 $7_0 }
    connect \Y $10_0
  end
end
