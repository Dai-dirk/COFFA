module \per_128_128

  attribute \keep "coarse_wire_3_0"
  wire width 32 $3_0

  attribute \keep "coarse_wire_4_0"
  wire width 32 $4_0

  attribute \keep "coarse_wire_5_0"
  wire width 32 $5_0

  attribute \keep "coarse_wire_6_0"
  wire width 32 $6_0

  attribute \keep "coarse_wire_7_0"
  wire width 32 $7_0

  wire width 1 $8_0

  attribute \keep "coarse_wire_9_0"
  wire width 32 $9_0

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

  attribute \keep "coarse_wire_24_0"
  wire width 32 $24_0

  attribute \keep "coarse_wire_25_0"
  wire width 32 $25_0

  wire width 1 $26_0

  attribute \keep "coarse_wire_27_0"
  wire width 32 $27_0

  attribute \keep "coarse_wire_28_0"
  wire width 32 $28_0

  attribute \keep "coarse_wire_29_0"
  wire width 32 $29_0

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

  wire width 1 $35_0

  attribute \keep "coarse_wire_36_0"
  wire width 32 $36_0

  attribute \keep "coarse_wire_37_0"
  wire width 32 $37_0

  attribute \keep "coarse_wire_38_0"
  wire width 32 $38_0

  attribute \keep "coarse_wire_39_0"
  wire width 32 $39_0

  attribute \keep "coarse_wire_40_0"
  wire width 32 $40_0

  attribute \keep "coarse_wire_41_0"
  wire width 32 $41_0

  wire width 1 $42_0

  wire width 1 $43_0

  wire width 1 $44_0

  wire width 1 $45_0

  wire width 1 $46_0

  attribute \keep "coarse_wire_47_0"
  wire width 32 $47_0

  attribute \keep "coarse_wire_48_0"
  wire width 32 $48_0

  attribute \keep "coarse_wire_49_0"
  wire width 32 $49_0

  attribute \keep "coarse_wire_50_0"
  wire width 32 $50_0

  attribute \keep "coarse_wire_51_0"
  wire width 32 $51_0

  wire width 1 $52_0

  wire width 1 $53_0

  attribute \keep "coarse_wire_54_0"
  wire width 32 $54_0

  attribute \keep "coarse_wire_55_0"
  wire width 32 $55_0

  wire width 1 $56_0

  wire width 1 $57_0

  attribute \keep "coarse_wire_58_0"
  wire width 32 $58_0

  wire width 1 $59_0

  wire width 1 $60_0


  attribute \keep "coarse_node_1"
  cell \OUTPUT $Output13
     parameter \multiport "1"
     parameter \offset "0,0"
     parameter \pattern "0,64,16,191"
     parameter \ref_name "X"
     parameter \size "3052"
     parameter \A_WIDTH 32
     connect \A $50_0
  end

  attribute \keep "coarse_node_2"
  cell \OUTPUT $Output15
     parameter \multiport "1"
     parameter \offset "0,8"
     parameter \pattern "0,64,16,191"
     parameter \ref_name "X"
     parameter \size "3052"
     parameter \A_WIDTH 32
     connect \A $49_0
  end

  attribute \keep "coarse_node_3"
  cell \INPUT $Input24
     parameter \multiport "1"
     parameter \offset "0,0"
     parameter \pattern "8,64,-504.000000,191"
     parameter \ref_name "lpr"
     parameter \size "512"
     parameter \Y_WIDTH 32
     connect \Y $3_0
  end

  attribute \keep "coarse_node_4"
  cell \INPUT $Input27
     parameter \multiport "1"
     parameter \offset "0,4"
     parameter \pattern "8,64,-504.000000,191"
     parameter \ref_name "lpr"
     parameter \size "512"
     parameter \Y_WIDTH 32
     connect \Y $4_0
  end

  attribute \keep "coarse_node_5"
  cell \INPUT $Input29
     parameter \multiport "1"
     parameter \offset "0,0"
     parameter \pattern "8,64,-504.000000,191"
     parameter \ref_name "hpr"
     parameter \size "512"
     parameter \Y_WIDTH 32
     connect \Y $5_0
  end

  attribute \keep "coarse_node_6"
  cell \INPUT $Input31
     parameter \multiport "1"
     parameter \offset "0,4"
     parameter \pattern "8,64,-504.000000,191"
     parameter \ref_name "hpr"
     parameter \size "512"
     parameter \Y_WIDTH 32
     connect \Y $6_0
  end

  attribute \keep "coarse_node_7"
  cell \SUB $SUB32
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $47_0
     connect \B $48_0
     connect \Y $7_0
  end

  attribute \keep "node_8"
  cell \ULT $ULT33
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 1
     connect \A $7_0
     connect \B $37_0
     connect \Y $8_0
  end

  attribute \keep "node_9"
  cell \CINPUT $CInput42
     parameter \multiport "0"
     parameter \offset "0,512"
     parameter \pattern "-4,64,256,191"
     parameter \ref_name "cA"
     parameter \size "1796"
     parameter \A_WIDTH 1
     parameter \Y_WIDTH 32
     connect \A $43_0
     connect \Y $9_0
  end

  attribute \keep "coarse_node_10"
  cell \MUL $MUL43
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $9_0
     connect \B $3_0
     connect \Y $10_0
  end

  attribute \keep "node_11"
  cell \CINPUT $CInput45
     parameter \multiport "0"
     parameter \offset "0,512"
     parameter \pattern "-4,64,256,191"
     parameter \ref_name "cD"
     parameter \size "1796"
     parameter \A_WIDTH 1
     parameter \Y_WIDTH 32
     connect \A $43_0
     connect \Y $11_0
  end

  attribute \keep "coarse_node_12"
  cell \MUL $MUL46
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $11_0
     connect \B $5_0
     connect \Y $12_0
  end

  attribute \keep "coarse_node_13"
  cell \ADD $ADD47
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $10_0
     connect \B $12_0
     connect \Y $13_0
  end

  attribute \keep "coarse_node_14"
  cell \MUL $MUL49
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $9_0
     connect \B $4_0
     connect \Y $14_0
  end

  attribute \keep "coarse_node_15"
  cell \MUL $MUL50
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $11_0
     connect \B $6_0
     connect \Y $15_0
  end

  attribute \keep "coarse_node_16"
  cell \ADD $ADD51
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $14_0
     connect \B $15_0
     connect \Y $16_0
  end

  attribute \keep "node_17"
  cell \CINPUT $CInput55
     parameter \multiport "0"
     parameter \offset "0,0"
     parameter \pattern "-4,64,256,191"
     parameter \ref_name "cA"
     parameter \size "1796"
     parameter \A_WIDTH 1
     parameter \Y_WIDTH 32
     connect \A $8_0
     connect \Y $17_0
  end

  attribute \keep "coarse_node_18"
  cell \MUL $MUL56
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $17_0
     connect \B $3_0
     connect \Y $18_0
  end

  attribute \keep "node_19"
  cell \CINPUT $CInput58
     parameter \multiport "0"
     parameter \offset "0,0"
     parameter \pattern "-4,64,256,191"
     parameter \ref_name "cD"
     parameter \size "1796"
     parameter \A_WIDTH 1
     parameter \Y_WIDTH 32
     connect \A $8_0
     connect \Y $19_0
  end

  attribute \keep "coarse_node_20"
  cell \MUL $MUL59
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $19_0
     connect \B $5_0
     connect \Y $20_0
  end

  attribute \keep "coarse_node_21"
  cell \ADD $ADD60
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $18_0
     connect \B $20_0
     connect \Y $21_0
  end

  attribute \keep "coarse_node_22"
  cell \MUL $MUL62
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $17_0
     connect \B $4_0
     connect \Y $22_0
  end

  attribute \keep "coarse_node_23"
  cell \MUL $MUL63
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $19_0
     connect \B $6_0
     connect \Y $23_0
  end

  attribute \keep "coarse_node_24"
  cell \ADD $ADD64
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $22_0
     connect \B $23_0
     connect \Y $24_0
  end

  attribute \keep "coarse_node_25"
  cell \ADD $ADD67
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $7_0
     connect \B $39_0
     connect \Y $25_0
  end

  attribute \keep "node_26"
  cell \ULT $ULT68
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 1
     connect \A $25_0
     connect \B $40_0
     connect \Y $26_0
  end

  attribute \keep "node_27"
  cell \CINPUT $CInput71
     parameter \multiport "0"
     parameter \offset "0,-512"
     parameter \pattern "-4,64,256,191"
     parameter \ref_name "cA"
     parameter \size "1796"
     parameter \A_WIDTH 1
     parameter \Y_WIDTH 32
     connect \A $44_0
     connect \Y $27_0
  end

  attribute \keep "coarse_node_28"
  cell \MUL $MUL72
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $27_0
     connect \B $3_0
     connect \Y $28_0
  end

  attribute \keep "node_29"
  cell \CINPUT $CInput74
     parameter \multiport "0"
     parameter \offset "0,-512"
     parameter \pattern "-4,64,256,191"
     parameter \ref_name "cD"
     parameter \size "1796"
     parameter \A_WIDTH 1
     parameter \Y_WIDTH 32
     connect \A $44_0
     connect \Y $29_0
  end

  attribute \keep "coarse_node_30"
  cell \MUL $MUL75
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $29_0
     connect \B $5_0
     connect \Y $30_0
  end

  attribute \keep "coarse_node_31"
  cell \ADD $ADD76
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $28_0
     connect \B $30_0
     connect \Y $31_0
  end

  attribute \keep "coarse_node_32"
  cell \MUL $MUL78
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $27_0
     connect \B $4_0
     connect \Y $32_0
  end

  attribute \keep "coarse_node_33"
  cell \MUL $MUL79
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $29_0
     connect \B $6_0
     connect \Y $33_0
  end

  attribute \keep "coarse_node_34"
  cell \ADD $ADD80
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $32_0
     connect \B $33_0
     connect \Y $34_0
  end

  attribute \keep "node_35"
  cell \ULT $ULT83
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 1
     connect \A $41_0
     connect \B $7_0
     connect \Y $35_0
  end

  attribute \keep "coarse_node_36"
  cell \CONST $CONST90
     parameter \Y_WIDTH 32
     parameter \VALUE 1
     connect \Y $36_0
  end

  attribute \keep "coarse_node_37"
  cell \CONST $CONST94
     parameter \Y_WIDTH 32
     parameter \VALUE 128
     connect \Y $37_0
  end

  attribute \keep "coarse_node_38"
  cell \CONST $CONST95
     parameter \Y_WIDTH 32
     parameter \VALUE 1
     connect \Y $38_0
  end

  attribute \keep "coarse_node_39"
  cell \CONST $CONST97
     parameter \Y_WIDTH 32
     parameter \VALUE -128
     connect \Y $39_0
  end

  attribute \keep "coarse_node_40"
  cell \CONST $CONST98
     parameter \Y_WIDTH 32
     parameter \VALUE 127
     connect \Y $40_0
  end

  attribute \keep "coarse_node_41"
  cell \CONST $CONST99
     parameter \Y_WIDTH 32
     parameter \VALUE -64
     connect \Y $41_0
  end

  cell $and $AND100
     parameter \A_SIGNED 0
     parameter \B_SIGNED 0
     parameter \A_WIDTH 1
     parameter \B_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $46_0
     connect \B $45_0
     connect \Y $42_0
  end

  cell $and $AND101
     parameter \A_SIGNED 0
     parameter \B_SIGNED 0
     parameter \A_WIDTH 1
     parameter \B_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $35_0
     connect \B $42_0
     connect \Y $43_0
  end

  cell $and $AND102
     parameter \A_SIGNED 0
     parameter \B_SIGNED 0
     parameter \A_WIDTH 1
     parameter \B_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $26_0
     connect \B $45_0
     connect \Y $44_0
  end

  cell $not $NOT104
     parameter \A_SIGNED 0
     parameter \A_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $8_0
     connect \Y $45_0
  end

  cell $not $NOT106
     parameter \A_SIGNED 0
     parameter \A_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $26_0
     connect \Y $46_0
  end

  attribute \keep "coarse_node_47"
  cell \ACC $ACC110
     parameter \acc_first "0"
     parameter \acc_params "0,191,64,1"
     parameter \A_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $36_0
     connect \Y $47_0
  end

  attribute \keep "coarse_node_48"
  cell \ACC $ACC118
     parameter \acc_first "0"
     parameter \acc_params "0,64,1,191"
     parameter \A_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $38_0
     connect \Y $48_0
  end

  attribute \keep "node_49"
  cell \CACC $CACC120
     parameter \acc_first "1"
     parameter \acc_params "0,64,1,191"
     parameter \A_WIDTH 32
     parameter \B_WIDTH 1
     parameter \Y_WIDTH 32
     connect \A $58_0
     connect \B $59_0
     connect \Y $49_0
  end

  attribute \keep "node_50"
  cell \CACC $CACC122
     parameter \acc_first "1"
     parameter \acc_params "0,64,1,191"
     parameter \A_WIDTH 32
     parameter \B_WIDTH 1
     parameter \Y_WIDTH 32
     connect \A $54_0
     connect \B $60_0
     connect \Y $50_0
  end

  attribute \keep "node_51"
  cell \SEL $SEL123
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \C_WIDTH 1
     parameter \Y_WIDTH 32
     connect \A $21_0
     connect \B $31_0
     connect \C $8_0
     connect \Y $51_0
  end

  cell $or $OR124
     parameter \A_SIGNED 0
     parameter \B_SIGNED 0
     parameter \A_WIDTH 1
     parameter \B_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $8_0
     connect \B $44_0
     connect \Y $52_0
  end

  cell $and $AND126
     parameter \A_SIGNED 0
     parameter \B_SIGNED 0
     parameter \A_WIDTH 1
     parameter \B_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $42_0
     connect \B $43_0
     connect \Y $53_0
  end

  attribute \keep "node_54"
  cell \SEL $SEL128
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \C_WIDTH 1
     parameter \Y_WIDTH 32
     connect \A $51_0
     connect \B $13_0
     connect \C $52_0
     connect \Y $54_0
  end

  attribute \keep "node_55"
  cell \SEL $SEL129
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \C_WIDTH 1
     parameter \Y_WIDTH 32
     connect \A $24_0
     connect \B $34_0
     connect \C $8_0
     connect \Y $55_0
  end

  cell $or $OR130
     parameter \A_SIGNED 0
     parameter \B_SIGNED 0
     parameter \A_WIDTH 1
     parameter \B_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $8_0
     connect \B $44_0
     connect \Y $56_0
  end

  cell $and $AND132
     parameter \A_SIGNED 0
     parameter \B_SIGNED 0
     parameter \A_WIDTH 1
     parameter \B_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $42_0
     connect \B $43_0
     connect \Y $57_0
  end

  attribute \keep "node_58"
  cell \SEL $SEL134
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \C_WIDTH 1
     parameter \Y_WIDTH 32
     connect \A $55_0
     connect \B $16_0
     connect \C $56_0
     connect \Y $58_0
  end

  cell $or $OR168
     parameter \A_SIGNED 0
     parameter \B_SIGNED 0
     parameter \A_WIDTH 1
     parameter \B_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $57_0
     connect \B $56_0
     connect \Y $59_0
  end

  cell $or $OR172
     parameter \A_SIGNED 0
     parameter \B_SIGNED 0
     parameter \A_WIDTH 1
     parameter \B_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $53_0
     connect \B $52_0
     connect \Y $60_0
  end
end
