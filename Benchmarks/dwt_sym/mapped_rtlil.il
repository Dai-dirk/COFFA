
autoidx 91

module \wtmath_dwt_sym_4096cycles_new_new

  attribute \keep "coarse_wire_10_0"
  wire width 32 $10_0

  attribute \keep "coarse_wire_11_0"
  wire width 32 $11_0

  wire $12_0

  attribute \keep "coarse_wire_13_0"
  wire width 32 $13_0

  attribute \keep "coarse_wire_14_0"
  wire width 32 $14_0

  attribute \keep "coarse_wire_15_0"
  wire width 32 $15_0

  attribute \keep "coarse_wire_16_0"
  wire width 32 $16_0

  wire $17_0

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

  attribute \keep "coarse_wire_25_0"
  wire width 32 $25_0

  attribute \keep "coarse_wire_26_0"
  wire width 32 $26_0

  attribute \keep "coarse_wire_27_0"
  wire width 32 $27_0

  wire $28_0

  attribute \keep "coarse_wire_2_0"
  wire width 32 $2_0

  wire $30_0

  attribute \keep "coarse_wire_33_0"
  wire width 32 $33_0

  attribute \keep "coarse_wire_34_0"
  wire width 32 $34_0

  attribute \keep "coarse_wire_35_0"
  wire width 32 $35_0

  attribute \keep "coarse_wire_36_0"
  wire width 32 $36_0

  attribute \keep "coarse_wire_37_0"
  wire width 32 $37_0

  wire $38_0

  attribute \keep "coarse_wire_40_0"
  wire width 32 $40_0

  attribute \keep "coarse_wire_41_0"
  wire width 32 $41_0

  attribute \keep "coarse_wire_44_0"
  wire width 32 $44_0

  attribute \keep "coarse_wire_45_0"
  wire width 32 $45_0

  attribute \keep "coarse_wire_46_0"
  wire width 32 $46_0

  wire $47_0

  attribute \keep "coarse_wire_5_0"
  wire width 32 $5_0

  attribute \keep "coarse_wire_6_0"
  wire width 32 $6_0

  attribute \keep "coarse_wire_7_0"
  wire width 32 $7_0

  wire $8_0

  attribute \keep "coarse_wire_9_0"
  wire width 32 $9_0

  attribute \keep "coarse_node_33"
  cell \ACC $ACC79
    parameter \A_WIDTH 32
    parameter \Y_WIDTH 32
    parameter \acc_first "0"
    parameter \acc_params "0,64,64,1"
    connect \A $23_0
    connect \Y $33_0
  end

  attribute \keep "coarse_node_34"
  cell \ACC $ACC83
    parameter \A_WIDTH 32
    parameter \Y_WIDTH 32
    parameter \acc_first "0"
    parameter \acc_params "0,64,1,64"
    connect \A $25_0
    connect \Y $34_0
  end

  attribute \keep "coarse_node_13"
  cell \ADD $ADD40
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $34_0
    connect \B $2_0
    connect \Y $13_0
  end

  attribute \keep "node_35"
  cell \CACC $CACC85
    parameter \A_WIDTH 32
    parameter \B_WIDTH 1
    parameter \Y_WIDTH 32
    parameter \acc_first "1"
    parameter \acc_params "0,64,1,64"
    connect \A $44_0
    connect \B $47_0
    connect \Y $35_0
  end

  attribute \keep "node_36"
  cell \CACC $CACC87
    parameter \A_WIDTH 32
    parameter \B_WIDTH 1
    parameter \Y_WIDTH 32
    parameter \acc_first "1"
    parameter \acc_params "0,64,1,64"
    connect \A $40_0
    connect \B $47_0
    connect \Y $36_0
  end

  attribute \keep "node_9"
  cell \CINPUT $CInput32
    parameter \A_WIDTH 1
    parameter \Y_WIDTH 32
    parameter \multiport "0"
    parameter \offset "0,4"
    parameter \pattern "-4,64,260,64"
    parameter \ref_name "inp"
    parameter \size "4096"
    connect \A $8_0
    connect \Y $9_0
  end

  attribute \keep "node_18"
  cell \CINPUT $CInput52
    parameter \A_WIDTH 1
    parameter \Y_WIDTH 32
    parameter \multiport "0"
    parameter \offset "0,504"
    parameter \pattern "4,64,-260,64"
    parameter \ref_name "inp"
    parameter \size "4096"
    connect \A $30_0
    connect \Y $18_0
  end

  attribute \keep "node_14"
  cell \CLOAD $CLOAD42
    parameter \A_WIDTH 32
    parameter \B_WIDTH 1
    parameter \Y_WIDTH 32
    parameter \cycles "4096"
    parameter \multiport "0"
    parameter \offset "0,0"
    parameter \ref_name "inp"
    parameter \size "4096"
    connect \A $45_0
    connect \B $28_0
    connect \Y $14_0
  end

  attribute \keep "coarse_node_46"
  cell \CONST $CONST111
    parameter \VALUE 4
    parameter \Y_WIDTH 32
    connect \Y $46_0
  end

  attribute \keep "coarse_node_21"
  cell \CONST $CONST59
    parameter \VALUE 1
    parameter \Y_WIDTH 32
    connect \Y $21_0
  end

  attribute \keep "coarse_node_22"
  cell \CONST $CONST61
    parameter \VALUE 32'11111111111111111111111111111110
    parameter \Y_WIDTH 32
    connect \Y $22_0
  end

  attribute \keep "coarse_node_23"
  cell \CONST $CONST62
    parameter \VALUE 2
    parameter \Y_WIDTH 32
    connect \Y $23_0
  end

  attribute \keep "coarse_node_24"
  cell \CONST $CONST64
    parameter \VALUE 64
    parameter \Y_WIDTH 32
    connect \Y $24_0
  end

  attribute \keep "coarse_node_25"
  cell \CONST $CONST65
    parameter \VALUE 1
    parameter \Y_WIDTH 32
    connect \Y $25_0
  end

  attribute \keep "coarse_node_26"
  cell \CONST $CONST67
    parameter \VALUE 0
    parameter \Y_WIDTH 32
    connect \Y $26_0
  end

  attribute \keep "coarse_node_27"
  cell \CONST $CONST68
    parameter \VALUE 63
    parameter \Y_WIDTH 32
    connect \Y $27_0
  end

  attribute \keep "coarse_node_5"
  cell \INPUT $Input20
    parameter \Y_WIDTH 32
    parameter \multiport "0"
    parameter \offset "0,0"
    parameter \pattern "4,64,-252,64"
    parameter \ref_name "lpd"
    parameter \size "256"
    connect \Y $5_0
  end

  attribute \keep "coarse_node_6"
  cell \INPUT $Input22
    parameter \Y_WIDTH 32
    parameter \multiport "0"
    parameter \offset "0,0"
    parameter \pattern "4,64,-252,64"
    parameter \ref_name "hpd"
    parameter \size "256"
    connect \Y $6_0
  end

  attribute \keep "coarse_node_45"
  cell \MUL $MUL110
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $13_0
    connect \B $46_0
    connect \Y $45_0
  end

  attribute \keep "coarse_node_10"
  cell \MUL $MUL33
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $9_0
    connect \B $5_0
    connect \Y $10_0
  end

  attribute \keep "coarse_node_11"
  cell \MUL $MUL35
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $9_0
    connect \B $6_0
    connect \Y $11_0
  end

  attribute \keep "coarse_node_15"
  cell \MUL $MUL43
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $14_0
    connect \B $5_0
    connect \Y $15_0
  end

  attribute \keep "coarse_node_16"
  cell \MUL $MUL45
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $14_0
    connect \B $6_0
    connect \Y $16_0
  end

  attribute \keep "coarse_node_19"
  cell \MUL $MUL53
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $18_0
    connect \B $5_0
    connect \Y $19_0
  end

  attribute \keep "coarse_node_20"
  cell \MUL $MUL55
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $18_0
    connect \B $6_0
    connect \Y $20_0
  end

  attribute \keep "coarse_node_1"
  cell \OR $OR3
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $33_0
    connect \B $21_0
    connect \Y $1_0
  end

  attribute \keep "coarse_node_3"
  cell \OUTPUT $Output10
    parameter \A_WIDTH 32
    parameter \multiport "0"
    parameter \offset "0,0"
    parameter \pattern "0,64,8,64"
    parameter \ref_name "cA"
    parameter \size "508"
    connect \A $36_0
  end

  attribute \keep "coarse_node_4"
  cell \OUTPUT $Output12
    parameter \A_WIDTH 32
    parameter \multiport "0"
    parameter \offset "0,0"
    parameter \pattern "0,64,8,64"
    parameter \ref_name "cD"
    parameter \size "508"
    connect \A $35_0
  end

  attribute \keep "node_37"
  cell \SEL $SEL88
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \C_WIDTH 1
    parameter \Y_WIDTH 32
    connect \A $10_0
    connect \B $15_0
    connect \C $8_0
    connect \Y $37_0
  end

  attribute \keep "node_40"
  cell \SEL $SEL93
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \C_WIDTH 1
    parameter \Y_WIDTH 32
    connect \A $37_0
    connect \B $19_0
    connect \C $38_0
    connect \Y $40_0
  end

  attribute \keep "node_41"
  cell \SEL $SEL94
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \C_WIDTH 1
    parameter \Y_WIDTH 32
    connect \A $11_0
    connect \B $16_0
    connect \C $8_0
    connect \Y $41_0
  end

  attribute \keep "node_44"
  cell \SEL $SEL99
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \C_WIDTH 1
    parameter \Y_WIDTH 32
    connect \A $41_0
    connect \B $20_0
    connect \C $38_0
    connect \Y $44_0
  end

  attribute \keep "node_12"
  cell \SLT $SLT38
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 1
    connect \A $7_0
    connect \B $26_0
    connect \Y $12_0
  end

  attribute \keep "node_17"
  cell \SLT $SLT48
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 1
    connect \A $27_0
    connect \B $7_0
    connect \Y $17_0
  end

  attribute \keep "coarse_node_7"
  cell \SUB $SUB23
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $1_0
    connect \B $34_0
    connect \Y $7_0
  end

  attribute \keep "node_8"
  cell \ULT $ULT24
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 1
    connect \A $7_0
    connect \B $24_0
    connect \Y $8_0
  end

  attribute \keep "coarse_node_2"
  cell \XOR $XOR5
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $33_0
    connect \B $22_0
    connect \Y $2_0
  end

  cell $lut $abc$86$auto$blifparse.cc:518:parse_blif$87
    parameter \LUT 4'0100
    parameter \WIDTH 2
    connect \A { $12_0 $8_0 }
    connect \Y $28_0
  end

  cell $lut $abc$86$auto$blifparse.cc:518:parse_blif$88
    parameter \LUT 8'00010000
    parameter \WIDTH 3
    connect \A { $17_0 $8_0 $12_0 }
    connect \Y $30_0
  end

  cell $lut $abc$86$auto$blifparse.cc:518:parse_blif$89
    parameter \LUT 4'1110
    parameter \WIDTH 2
    connect \A { $8_0 $12_0 }
    connect \Y $38_0
  end

  cell $lut $abc$86$auto$blifparse.cc:518:parse_blif$90
    parameter \LUT 8'11111110
    parameter \WIDTH 3
    connect \A { $8_0 $17_0 $12_0 }
    connect \Y $47_0
  end
end
