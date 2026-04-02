module \fine

  attribute \keep "coarse_input_1"
  wire width 32 input 1 \input1

  attribute \keep "coarse_input_2"
  wire width 32 input 2 \input2

  attribute \keep "coarse_input_3"
  wire width 32 input 3 \input6

  wire width 1 input 4 \input9

  attribute \keep "coarse_input_5"
  wire width 32 input 5 \input17

  wire width 1 input 6 \input31

  attribute \keep "coarse_output_7"
  wire width 32 output 7 \output24

  wire width 1 output 8 \output30

  wire width 1 output 9 \output37

  attribute \keep "coarse_wire_1_0"
  wire width 32 $1_0

  attribute \keep "coarse_wire_2_0"
  wire width 32 $2_0

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

  attribute \keep "coarse_wire_8_0"
  wire width 32 $8_0

  wire width 1 $9_0

  attribute \keep "coarse_wire_10_0"
  wire width 32 $10_0

  wire width 1 $11_0

  attribute \keep "coarse_wire_12_0"
  wire width 32 $12_0

  attribute \keep "coarse_wire_13_0"
  wire width 32 $13_0

  attribute \keep "coarse_wire_14_0"
  wire width 32 $14_0

  wire width 1 $15_0

  attribute \keep "coarse_wire_16_0"
  wire width 32 $16_0

  attribute \keep "coarse_wire_17_0"
  wire width 32 $17_0

  wire width 1 $18_0

  wire width 1 $19_0

  wire width 1 $20_0

  attribute \keep "coarse_wire_21_0"
  wire width 32 $21_0

  attribute \keep "coarse_wire_22_0"
  wire width 32 $22_0

  attribute \keep "coarse_wire_23_0"
  wire width 32 $23_0

  attribute \keep "coarse_wire_25_0"
  wire width 32 $25_0

  wire width 1 $27_0

  wire width 1 $28_0

  wire width 1 $29_0

  wire width 1 $31_0

  wire width 1 $32_0

  attribute \keep "coarse_wire_33_0"
  wire width 32 $33_0

  wire width 1 $35_0

  wire width 1 $36_0


  attribute \keep "coarse_node_1"
  cell \INPUT $input1
     parameter \A_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A \input1
     connect \Y $1_0
  end

  attribute \keep "coarse_node_2"
  cell \INPUT $input2
     parameter \A_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A \input2
     connect \Y $2_0
  end

  attribute \keep "coarse_node_3"
  cell \ADD $add3
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $1_0
     connect \B $2_0
     connect \Y $3_0
  end

  attribute \keep "coarse_node_4"
  cell \LOAD $load4
     parameter \Y_WIDTH 32
     connect \Y $4_0
  end

  attribute \keep "coarse_node_5"
  cell \MUL $mul5
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $3_0
     connect \B $4_0
     connect \Y $5_0
  end

  attribute \keep "coarse_node_6"
  cell \INPUT $input6
     parameter \A_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A \input6
     connect \Y $6_0
  end

  attribute \keep "coarse_node_7"
  cell \LOAD $load7
     parameter \Y_WIDTH 32
     connect \Y $7_0
  end

  attribute \keep "coarse_node_8"
  cell \AND $and8
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $6_0
     connect \B $7_0
     connect \Y $8_0
  end

  attribute \keep "fine_node_9"
  cell \INPUT $input9
     parameter \A_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A \input9
     connect \Y $9_0
  end

  attribute \keep "coarse_node_10"
  cell \SHL $shl10
     parameter \A_WIDTH 32
     parameter \B_WIDTH 1
     parameter \Y_WIDTH 32
     connect \A $5_0
     connect \B $9_0
     connect \Y $10_0
  end

  attribute \keep "coarse_node_11"
  cell \EQ $eq11
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 1
     connect \A $8_0
     connect \B $10_0
     connect \Y $11_0
  end

  attribute \keep "coarse_node_12"
  cell \LOAD $load12
     parameter \Y_WIDTH 32
     connect \Y $12_0
  end

  attribute \keep "coarse_node_13"
  cell \LOAD $load13
     parameter \Y_WIDTH 32
     connect \Y $13_0
  end

  attribute \keep "coarse_node_14"
  cell \SUB $sub14
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $13_0
     connect \B $12_0
     connect \Y $14_0
  end

  attribute \keep "coarse_node_15"
  cell \LT $lt15
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 1
     connect \A $14_0
     connect \B $10_0
     connect \Y $15_0
  end

  attribute \keep "coarse_node_16"
  cell \OR $or16
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $8_0
     connect \B $10_0
     connect \Y $16_0
  end

  attribute \keep "coarse_node_17"
  cell \INPUT $input17
     parameter \A_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A \input17
     connect \Y $17_0
  end

  attribute \keep "coarse_node_18"
  cell \ILE $ile18
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \Y_WIDTH 1
     connect \A $17_0
     connect \B $16_0
     connect \Y $18_0
  end

  cell $or $or19
     parameter \A_SIGNED 0
     parameter \B_SIGNED 0
     parameter \A_WIDTH 1
     parameter \B_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $15_0
     connect \B $11_0
     connect \Y $19_0
  end

  cell $and $and20
     parameter \A_SIGNED 0
     parameter \B_SIGNED 0
     parameter \A_WIDTH 1
     parameter \B_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $18_0
     connect \B $19_0
     connect \Y $20_0
  end

  attribute \keep "coarse_node_21"
  cell \LOAD $load21
     parameter \Y_WIDTH 32
     connect \Y $21_0
  end

  attribute \keep "coarse_node_22"
  cell \LOAD $load22
     parameter \Y_WIDTH 32
     connect \Y $22_0
  end

  attribute \keep "coarse_node_23"
  cell \SEL $sel23
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     parameter \C_WIDTH 1
     parameter \Y_WIDTH 32
     connect \A $21_0
     connect \B $22_0
     connect \C $20_0
     connect \Y $23_0
  end

  attribute \keep "coarse_node_24"
  cell \OUTPUT $output24
     parameter \A_WIDTH 32
     parameter \Y_WIDTH 32
     connect \A $23_0
     connect \Y \output24
  end

  attribute \keep "coarse_node_25"
  cell \CONST $const25
     parameter \Y_WIDTH 32
     connect \Y $25_0
  end

  attribute \keep "coarse_node_26"
  cell \STORE $store26
     parameter \A_WIDTH 32
     parameter \B_WIDTH 32
     connect \A $23_0
     connect \B $25_0
  end

  attribute \keep "fine_node_27"
  cell \LOAD $load27
     parameter \Y_WIDTH 1
     connect \Y $27_0
  end

  cell $xor $xor28
     parameter \A_SIGNED 0
     parameter \B_SIGNED 0
     parameter \A_WIDTH 1
     parameter \B_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $19_0
     connect \B $27_0
     connect \Y $28_0
  end

  cell $not $not29
     parameter \A_SIGNED 0
     parameter \A_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $28_0
     connect \Y $29_0
  end

  attribute \keep "fine_node_30"
  cell \OUTPUT $output30
     parameter \A_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $29_0
     connect \Y \output30
  end

  attribute \keep "fine_node_31"
  cell \INPUT $input31
     parameter \A_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A \input31
     connect \Y $31_0
  end

  cell $eq $eq32
     parameter \A_SIGNED 0
     parameter \B_SIGNED 0
     parameter \A_WIDTH 1
     parameter \B_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $28_0
     connect \B $31_0
     connect \Y $32_0
  end

  attribute \keep "coarse_node_33"
  cell \CONST $const33
     parameter \Y_WIDTH 32
     connect \Y $33_0
  end

  attribute \keep "coarse_node_34"
  cell \STORE $store34
     parameter \A_WIDTH 32
     parameter \B_WIDTH 1
     connect \A $33_0
     connect \B $32_0
  end

  attribute \keep "fine_node_35"
  cell \CONST $const35
     parameter \Y_WIDTH 1
     connect \Y $35_0
  end

  cell $or $or36
     parameter \A_SIGNED 0
     parameter \B_SIGNED 0
     parameter \A_WIDTH 1
     parameter \B_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $20_0
     connect \B $35_0
     connect \Y $36_0
  end

  attribute \keep "fine_node_37"
  cell \OUTPUT $output37
     parameter \A_WIDTH 1
     parameter \Y_WIDTH 1
     connect \A $36_0
     connect \Y \output37
  end
end
