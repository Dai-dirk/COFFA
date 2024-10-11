
autoidx 85

module \csr_matvec_custom

  attribute \keep "coarse_wire_10_0"
  wire width 32 $10_0

  wire $11_0

  wire $12_0

  wire $13_0

  attribute \keep "coarse_wire_14_0"
  wire width 32 $14_0

  attribute \keep "back_wire $15_0 1 0 0"
  wire $15_0

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

  attribute \keep "coarse_wire_24_0"
  wire width 32 $24_0

  attribute \keep "coarse_wire_25_0"
  wire width 32 $25_0

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

  attribute \keep "coarse_wire_35_0"
  wire width 32 $35_0

  attribute \keep "coarse_wire_36_0"
  wire width 32 $36_0

  attribute \keep "coarse_wire_37_0"
  wire width 32 $37_0

  attribute \keep "coarse_wire_38_0"
  wire width 32 $38_0

  attribute \keep "coarse_wire_39_0"
  wire width 32 $39_0

  attribute \keep "coarse_wire_3_0"
  wire width 32 $3_0

  attribute \keep "coarse_wire_40_0"
  wire width 32 $40_0

  attribute \keep "coarse_wire_41_0"
  wire width 32 $41_0

  attribute \keep "coarse_wire_42_0"
  wire width 32 $42_0

  attribute \keep "coarse_wire_43_0"
  wire width 32 $43_0

  attribute \keep "coarse_wire_44_0"
  wire width 32 $44_0

  attribute \keep "coarse_wire_45_0"
  wire width 32 $45_0

  attribute \keep "coarse_wire_46_0"
  wire width 32 $46_0

  attribute \keep "coarse_wire_47_0"
  wire width 32 $47_0

  attribute \keep "coarse_wire_48_0"
  wire width 32 $48_0

  attribute \keep "coarse_wire_49_0"
  wire width 32 $49_0

  attribute \keep "coarse_wire_4_0"
  wire width 32 $4_0

  attribute \keep "back_wire $50_0 1 0 0"
  wire $50_0

  attribute \keep "back_wire $51_0 1 0 0"
  wire $51_0

  wire $52_0

  wire $5_0

  attribute \keep "coarse_wire_6_0"
  wire width 32 $6_0

  wire $7_0

  attribute \keep "coarse_wire_8_0"
  wire width 32 $8_0

  attribute \keep "coarse_wire_9_0"
  wire width 32 $9_0

  attribute \keep "coarse_node_6"
  cell \ADD $ADD14
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $35_0
    connect \B $27_0
    connect \Y $6_0
  end

  attribute \keep "coarse_node_9"
  cell \ADD $ADD18
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $8_0
    connect \B $4_0
    connect \Y $9_0
  end

  attribute \keep "coarse_node_14"
  cell \ADD $ADD29
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $49_0
    connect \B $32_0
    connect \Y $14_0
  end

  attribute \keep "coarse_node_20"
  cell \ADD $ADD37
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $19_0
    connect \B $8_0
    connect \Y $20_0
  end

  attribute \keep "coarse_node_2"
  cell \ADD $ADD4
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $34_0
    connect \B $24_0
    connect \Y $2_0
  end

  attribute \keep "coarse_node_22"
  cell \ADD $ADD40
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $21_0
    connect \B $18_0
    connect \Y $22_0
  end

  attribute \keep "coarse_node_49"
  cell \ADD $ADD81
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $1_0
    connect \B $36_0
    connect \Y $49_0
  end

  attribute \keep "node_34"
  cell \CACC $CACC63
    parameter \A_WIDTH 32
    parameter \B_WIDTH 1
    parameter \Y_WIDTH 32
    parameter \acc_first "0"
    parameter \acc_params "0,4096,1,1"
    parameter \backedge "B,1,0,0"
    connect \A $24_0
    connect \B $50_0
    connect \Y $34_0
  end

  attribute \keep "node_35"
  cell \CIACC $CIACC66
    parameter \A_WIDTH 32
    parameter \B_WIDTH 1
    parameter \C_WIDTH 1
    parameter \Y_WIDTH 32
    parameter \acc_first "0"
    parameter \acc_params "0,4096,1,1"
    parameter \backedge "B,1,0,0 C,1,0,0"
    connect \A $27_0
    connect \B $15_0
    connect \C $50_0
    connect \Y $35_0
  end

  attribute \keep "node_36"
  cell \CIACC $CIACC68
    parameter \A_WIDTH 32
    parameter \B_WIDTH 1
    parameter \C_WIDTH 1
    parameter \Y_WIDTH 32
    parameter \acc_first "0"
    parameter \acc_params "0,4096,1,1"
    parameter \backedge "C,1,0,0"
    connect \A $32_0
    connect \B $52_0
    connect \C $15_0
    connect \Y $36_0
  end

  attribute \keep "coarse_node_24"
  cell \CONST $CONST43
    parameter \VALUE 1
    parameter \Y_WIDTH 32
    connect \Y $24_0
  end

  attribute \keep "coarse_node_25"
  cell \CONST $CONST44
    parameter \VALUE 3
    parameter \Y_WIDTH 32
    connect \Y $25_0
  end

  attribute \keep "coarse_node_26"
  cell \CONST $CONST45
    parameter \VALUE 32
    parameter \Y_WIDTH 32
    connect \Y $26_0
  end

  attribute \keep "coarse_node_27"
  cell \CONST $CONST46
    parameter \VALUE 1
    parameter \Y_WIDTH 32
    connect \Y $27_0
  end

  attribute \keep "coarse_node_28"
  cell \CONST $CONST47
    parameter \VALUE 32
    parameter \Y_WIDTH 32
    connect \Y $28_0
  end

  attribute \keep "coarse_node_29"
  cell \CONST $CONST48
    parameter \VALUE 4
    parameter \Y_WIDTH 32
    connect \Y $29_0
  end

  attribute \keep "coarse_node_30"
  cell \CONST $CONST49
    parameter \VALUE 0
    parameter \Y_WIDTH 32
    connect \Y $30_0
  end

  attribute \keep "coarse_node_31"
  cell \CONST $CONST50
    parameter \VALUE 64
    parameter \Y_WIDTH 32
    connect \Y $31_0
  end

  attribute \keep "coarse_node_32"
  cell \CONST $CONST51
    parameter \VALUE 1
    parameter \Y_WIDTH 32
    connect \Y $32_0
  end

  attribute \keep "coarse_node_33"
  cell \CONST $CONST52
    parameter \VALUE 1
    parameter \Y_WIDTH 32
    connect \Y $33_0
  end

  attribute \keep "coarse_node_38"
  cell \CONST $CONST70
    parameter \VALUE 4
    parameter \Y_WIDTH 32
    connect \Y $38_0
  end

  attribute \keep "coarse_node_40"
  cell \CONST $CONST72
    parameter \VALUE 4
    parameter \Y_WIDTH 32
    connect \Y $40_0
  end

  attribute \keep "coarse_node_42"
  cell \CONST $CONST74
    parameter \VALUE 4
    parameter \Y_WIDTH 32
    connect \Y $42_0
  end

  attribute \keep "coarse_node_44"
  cell \CONST $CONST76
    parameter \VALUE 4
    parameter \Y_WIDTH 32
    connect \Y $44_0
  end

  attribute \keep "coarse_node_46"
  cell \CONST $CONST78
    parameter \VALUE 4
    parameter \Y_WIDTH 32
    connect \Y $46_0
  end

  attribute \keep "coarse_node_48"
  cell \CONST $CONST80
    parameter \VALUE 4
    parameter \Y_WIDTH 32
    connect \Y $48_0
  end

  attribute \keep "fine_node_52"
  cell \CONST $CONST84
    parameter \VALUE 1
    parameter \Y_WIDTH 1
    connect \Y $52_0
  end

  attribute \keep "node_5"
  cell \EQ $EQ10
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 1
    connect \A $2_0
    connect \B $26_0
    connect \Y $5_0
  end

  attribute \keep "node_7"
  cell \EQ $EQ15
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 1
    connect \A $6_0
    connect \B $28_0
    connect \Y $7_0
  end

  attribute \keep "node_15"
  cell \EQ $EQ30
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 1
    connect \A $14_0
    connect \B $3_0
    connect \Y $15_0
  end

  attribute \keep "coarse_node_18"
  cell \MUL $MUL35
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $17_0
    connect \B $16_0
    connect \Y $18_0
  end

  attribute \keep "coarse_node_37"
  cell \MUL $MUL69
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $34_0
    connect \B $38_0
    connect \Y $37_0
  end

  attribute \keep "coarse_node_39"
  cell \MUL $MUL71
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $2_0
    connect \B $40_0
    connect \Y $39_0
  end

  attribute \keep "coarse_node_41"
  cell \MUL $MUL73
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $9_0
    connect \B $42_0
    connect \Y $41_0
  end

  attribute \keep "coarse_node_43"
  cell \MUL $MUL75
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $49_0
    connect \B $44_0
    connect \Y $43_0
  end

  attribute \keep "coarse_node_45"
  cell \MUL $MUL77
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $49_0
    connect \B $46_0
    connect \Y $45_0
  end

  attribute \keep "coarse_node_47"
  cell \MUL $MUL79
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $20_0
    connect \B $48_0
    connect \Y $47_0
  end

  attribute \keep "coarse_node_8"
  cell \SHL $SHL17
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $35_0
    connect \B $29_0
    connect \Y $8_0
  end

  attribute \keep "coarse_node_19"
  cell \SHL $SHL36
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $10_0
    connect \B $33_0
    connect \Y $19_0
  end

  attribute \keep "coarse_node_4"
  cell \SHL $SHL8
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 32
    connect \A $34_0
    connect \B $25_0
    connect \Y $4_0
  end

  attribute \keep "node_11"
  cell \SLT $SLT24
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 1
    connect \A $30_0
    connect \B $10_0
    connect \Y $11_0
  end

  attribute \keep "node_12"
  cell \SLT $SLT25
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \Y_WIDTH 1
    connect \A $10_0
    connect \B $31_0
    connect \Y $12_0
  end

  attribute \keep "node_16"
  cell \TCLOAD $TCLOAD33
    parameter \A_WIDTH 32
    parameter \B_WIDTH 1
    parameter \C_WIDTH 1
    parameter \Y_WIDTH 32
    parameter \backedge "C,1,0,0"
    parameter \cycles "4096"
    parameter \multiport "0"
    parameter \offset "0,0"
    parameter \ref_name "A_data"
    parameter \size "512"
    connect \A $45_0
    connect \B $13_0
    connect \C $51_0
    connect \Y $16_0
  end

  attribute \keep "node_17"
  cell \TCLOAD $TCLOAD34
    parameter \A_WIDTH 32
    parameter \B_WIDTH 1
    parameter \C_WIDTH 1
    parameter \Y_WIDTH 32
    parameter \backedge "C,1,0,0"
    parameter \cycles "4096"
    parameter \multiport "0"
    parameter \offset "0,0"
    parameter \ref_name "x_data"
    parameter \size "3072"
    connect \A $41_0
    connect \B $13_0
    connect \C $51_0
    connect \Y $17_0
  end

  attribute \keep "node_21"
  cell \TCLOAD $TCLOAD39
    parameter \A_WIDTH 32
    parameter \B_WIDTH 1
    parameter \C_WIDTH 1
    parameter \Y_WIDTH 32
    parameter \backedge "C,1,0,0"
    parameter \cycles "4096"
    parameter \multiport "1"
    parameter \offset "0,0"
    parameter \ref_name "y_data"
    parameter \size "2560"
    connect \A $47_0
    connect \B $13_0
    connect \C $51_0
    connect \Y $21_0
  end

  attribute \keep "node_23"
  cell \TCSTORE $TCSTORE41
    parameter \A_WIDTH 32
    parameter \B_WIDTH 32
    parameter \C_WIDTH 1
    parameter \D_WIDTH 1
    parameter \backedge "D,1,0,0"
    parameter \cycles "4096"
    parameter \multiport "1"
    parameter \offset "0,0"
    parameter \ref_name "y_data"
    parameter \size "2560"
    connect \A $22_0
    connect \B $47_0
    connect \C $13_0
    connect \D $51_0
  end

  attribute \keep "node_10"
  cell \TLOAD $TLOAD23
    parameter \A_WIDTH 32
    parameter \B_WIDTH 1
    parameter \Y_WIDTH 32
    parameter \backedge "B,1,0,0"
    parameter \cycles "4096"
    parameter \multiport "0"
    parameter \offset "0,0"
    parameter \ref_name "A_j"
    parameter \size "256"
    connect \A $43_0
    connect \B $51_0
    connect \Y $10_0
  end

  attribute \keep "node_1"
  cell \TLOAD $TLOAD3
    parameter \A_WIDTH 32
    parameter \B_WIDTH 1
    parameter \Y_WIDTH 32
    parameter \backedge "B,1,0,0"
    parameter \cycles "4096"
    parameter \multiport "0"
    parameter \offset "0,0"
    parameter \ref_name "A_i"
    parameter \size "192"
    connect \A $37_0
    connect \B $51_0
    connect \Y $1_0
  end

  attribute \keep "node_3"
  cell \TLOAD $TLOAD6
    parameter \A_WIDTH 32
    parameter \B_WIDTH 1
    parameter \Y_WIDTH 32
    parameter \backedge "B,1,0,0"
    parameter \cycles "4096"
    parameter \multiport "0"
    parameter \offset "0,0"
    parameter \ref_name "B_i"
    parameter \size "192"
    connect \A $39_0
    connect \B $51_0
    connect \Y $3_0
  end

  cell $lut $abc$81$auto$blifparse.cc:518:parse_blif$82
    parameter \LUT 4'1000
    parameter \WIDTH 2
    connect \A { $12_0 $11_0 }
    connect \Y $13_0
  end

  cell $lut $abc$81$auto$blifparse.cc:518:parse_blif$83
    parameter \LUT 4'1000
    parameter \WIDTH 2
    connect \A { $7_0 $15_0 }
    connect \Y $50_0
  end

  cell $lut $abc$81$auto$blifparse.cc:518:parse_blif$84
    parameter \LUT 8'10000000
    parameter \WIDTH 3
    connect \A { $5_0 $7_0 $15_0 }
    connect \Y $51_0
  end
end
