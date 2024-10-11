module \lpdc_matching

  wire width 1 $1_0

  wire width 1 $2_0

  attribute \keep "coarse_wire_3_0"
  wire width 32 $3_0

  wire width 1 $4_0

  attribute \keep "coarse_wire_5_0"
  wire width 32 $5_0

  attribute \keep "coarse_wire_7_0"
  wire width 32 $7_0

  attribute \keep "coarse_wire_8_0"
  wire width 32 $8_0

  wire width 1 $9_0

  wire width 1 $10_0

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

  wire width 1 $21_0

  wire width 1 $22_0

  wire width 1 $23_0

  wire width 1 $24_0

  wire width 1 $25_0

  attribute \keep "coarse_wire_26_0"
  wire width 32 $26_0

  attribute \keep "coarse_wire_27_0"
  wire width 32 $27_0

  attribute \keep "coarse_wire_28_0"
  wire width 32 $28_0

  attribute \keep "coarse_wire_29_0"
  wire width 32 $29_0

  wire width 1 $30_0

  wire width 1 $31_0

  wire width 1 $32_0


  attribute \keep "node_1"
  cell \ULT $ULT2
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 1
     connect \A $26_0
     connect \B $12_0
     connect \Y $1_0
  end

  attribute \keep "node_2"
  cell \ULT $ULT3
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 1
     connect \A $13_0
     connect \B $26_0
     connect \Y $2_0
  end

  attribute \keep "coarse_node_3"
  cell \ADD $ADD4
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $26_0
     connect \B $14_0
     connect \Y $3_0
  end

  attribute \keep "node_4"
  cell \ULT $ULT5
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 1
     connect \A $3_0
     connect \B $15_0
     connect \Y $4_0
  end

  attribute \keep "coarse_node_5"
  cell \ASHR $ASHR8
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $28_0
     connect \B $16_0
     connect \Y $5_0
  end

  attribute \keep "coarse_node_6"
  cell \OUTPUT $Output10
     parameter \multiport "0"
     parameter \offset "0,0"
     parameter \pattern "0,3,4,660"
     parameter \ref_name "W"
     parameter \size "2640"
     parameter \A_WIDTH 32
     connect \A $5_0
  end

  attribute \keep "coarse_node_7"
  cell \ADD $ADD17
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $27_0
     connect \B $26_0
     connect \Y $7_0
  end

  attribute \keep "node_8"
  cell \CINPUT $CInput24
     parameter \multiport "0"
     parameter \offset "-40,40"
     parameter \pattern "2600,3,-5196,660"
     parameter \ref_name "fint"
     parameter \size "7880"
     parameter \A_WIDTH 1
     parameter \Y_WIDTH 32
     connect \A $1_0
     connect \Y $8_0
  end

  attribute \keep "node_9"
  cell \ULT $ULT27
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 1
     connect \A $7_0
     connect \B $19_0
     connect \Y $9_0
  end

  cell $and $AND28
     parameter \A_SIGNED 0
     parameter \B_SIGNED 0
     parameter \A_WIDTH 1
     parameter \B_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $2_0
     connect \B $9_0
     connect \Y $10_0
  end

  attribute \keep "node_11"
  cell \CINPUT $CInput32
     parameter \multiport "0"
     parameter \offset "-40,0"
     parameter \pattern "2600,3,-5196,660"
     parameter \ref_name "fint"
     parameter \size "7880"
     parameter \A_WIDTH 1
     parameter \Y_WIDTH 32
     connect \A $21_0
     connect \Y $11_0
  end

  attribute \keep "coarse_node_12"
  cell \CONST $CONST37
     parameter \Y_WIDTH 32
     parameter \VALUE 350
     connect \Y $12_0
  end

  attribute \keep "coarse_node_13"
  cell \CONST $CONST38
     parameter \Y_WIDTH 32
     parameter \VALUE 359
     connect \Y $13_0
  end

  attribute \keep "coarse_node_14"
  cell \CONST $CONST39
     parameter \Y_WIDTH 32
     parameter \VALUE -350
     connect \Y $14_0
  end

  attribute \keep "coarse_node_15"
  cell \CONST $CONST40
     parameter \Y_WIDTH 32
     parameter \VALUE 10
     connect \Y $15_0
  end

  attribute \keep "coarse_node_16"
  cell \CONST $CONST41
     parameter \Y_WIDTH 32
     parameter \VALUE 2
     connect \Y $16_0
  end

  attribute \keep "coarse_node_17"
  cell \CONST $CONST42
     parameter \Y_WIDTH 32
     parameter \VALUE 1
     connect \Y $17_0
  end

  attribute \keep "coarse_node_18"
  cell \CONST $CONST45
     parameter \Y_WIDTH 32
     parameter \VALUE 650
     connect \Y $18_0
  end

  attribute \keep "coarse_node_19"
  cell \CONST $CONST47
     parameter \Y_WIDTH 32
     parameter \VALUE 1802
     connect \Y $19_0
  end

  attribute \keep "coarse_node_20"
  cell \CONST $CONST49
     parameter \Y_WIDTH 32
     parameter \VALUE 129
     connect \Y $20_0
  end

  cell $and $AND50
     parameter \A_SIGNED 0
     parameter \B_SIGNED 0
     parameter \A_WIDTH 1
     parameter \B_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $10_0
     connect \B $24_0
     connect \Y $21_0
  end

  cell $and $AND51
     parameter \A_SIGNED 0
     parameter \B_SIGNED 0
     parameter \A_WIDTH 1
     parameter \B_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $25_0
     connect \B $24_0
     connect \Y $22_0
  end

  cell $and $AND52
     parameter \A_SIGNED 0
     parameter \B_SIGNED 0
     parameter \A_WIDTH 1
     parameter \B_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $4_0
     connect \B $22_0
     connect \Y $23_0
  end

  cell $not $NOT54
     parameter \A_SIGNED 0
     parameter \A_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $1_0
     connect \Y $24_0
  end

  cell $not $NOT56
     parameter \A_SIGNED 0
     parameter \A_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $10_0
     connect \Y $25_0
  end

  attribute \keep "coarse_node_26"
  cell \ACC $ACC59
     parameter \acc_first "0"
     parameter \acc_params "0,660,3,1"
     parameter \A_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $17_0
     connect \Y $26_0
  end

  attribute \keep "coarse_node_27"
  cell \ACC $ACC63
     parameter \acc_first "0"
     parameter \acc_params "0,3,1,660"
     parameter \A_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $18_0
     connect \Y $27_0
  end

  attribute \keep "node_28"
  cell \CDIACC $CDIACC65
     parameter \acc_first "1"
     parameter \acc_params "0,3,1,660"
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \C_WIDTH 1
     parameter \D_WIDTH 1
     parameter \Y_WIDTH 32
     connect \A $29_0
     connect \B $20_0
     connect \C $31_0
     connect \D $32_0
     connect \Y $28_0
  end

  attribute \keep "node_29"
  cell \SEL $SEL66
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \C_WIDTH 1
     parameter \Y_WIDTH 32
     connect \A $8_0
     connect \B $11_0
     connect \C $1_0
     connect \Y $29_0
  end

  cell $and $AND69
     parameter \A_SIGNED 0
     parameter \B_SIGNED 0
     parameter \A_WIDTH 1
     parameter \B_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $24_0
     connect \B $22_0
     connect \Y $30_0
  end

  cell $not $NOT79
     parameter \A_SIGNED 0
     parameter \A_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $30_0
     connect \Y $31_0
  end

  cell $and $AND80
     parameter \A_SIGNED 0
     parameter \B_SIGNED 0
     parameter \A_WIDTH 1
     parameter \B_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $23_0
     connect \B $30_0
     connect \Y $32_0
  end
end
