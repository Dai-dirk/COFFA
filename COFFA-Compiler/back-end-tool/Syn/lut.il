
autoidx 82

module \ex

  wire width 16 $edge1

  wire $edge11

  wire $edge12

  wire width 16 $edge2

  wire $edge3

  wire $edge4

  wire width 16 $edge5

  wire width 16 $edge6

  wire width 16 $edge7

  wire width 16 $edge8

  wire $edge9

  wire width 16 input 1 \a

  wire width 16 input 2 \b

  wire width 16 input 3 \c

  wire width 16 input 4 \d

  wire input 5 \e

  wire input 6 \f

  wire output 8 \g

  wire output 7 \o

  cell \INPUT $Input1
    parameter \A_WIDTH 16
    parameter \Y_WIDTH 16
    connect \A \c
    connect \Y $edge5
  end

  cell \INPUT $Input2
    parameter \A_WIDTH 16
    parameter \Y_WIDTH 16
    connect \A \d
    connect \Y $edge6
  end

  cell \INPUT $Input3
    parameter \A_WIDTH 16
    parameter \Y_WIDTH 16
    connect \A \a
    connect \Y $edge7
  end

  cell \INPUT $Input4
    parameter \A_WIDTH 16
    parameter \Y_WIDTH 16
    connect \A \b
    connect \Y $edge8
  end

  cell \INPUT $Input5
    parameter \A_WIDTH 1
    parameter \Y_WIDTH 1
    connect \A \e
    connect \Y $edge9
  end

  cell \INPUT $Input6
    parameter \A_WIDTH 1
    parameter \Y_WIDTH 1
    connect \A \f
    connect \Y $edge11
  end

  cell \OUTPUT $Output1
    parameter \A_WIDTH 1
    parameter \Y_WIDTH 1
    connect \A $edge12
    connect \Y \o
  end

  cell \ADD $add$/home/dai-dirk/ex/ex_rtlil/mull.v:33$1
    parameter \A_WIDTH 16
    parameter \B_WIDTH 16
    parameter \Y_WIDTH 16
    connect \A $edge5
    connect \B $edge6
    connect \Y $edge2
  end

  cell $_OR_ $auto$simplemap.cc:86:simplemap_bitop$79
    connect \A $edge9
    connect \B $edge3
    connect \Y $edge4
  end

  cell $_XOR_ $auto$simplemap.cc:86:simplemap_bitop$80
    connect \A $edge4
    connect \B $edge3
    connect \Y \g
  end

  cell $_AND_ $auto$simplemap.cc:86:simplemap_bitop$81
    connect \A $edge11
    connect \B $edge4
    connect \Y $edge12
  end

  cell \EQ $eq$/home/dai-dirk/ex/ex_rtlil/mull.v:39$3
    parameter \A_WIDTH 16
    parameter \B_WIDTH 16
    parameter \Y_WIDTH 1
    connect \A $edge1
    connect \B $edge2
    connect \Y $edge3
  end

  cell \MUL $mul$/home/dai-dirk/ex/ex_rtlil/mull.v:33$1
    parameter \A_WIDTH 16
    parameter \B_WIDTH 16
    parameter \Y_WIDTH 16
    connect \A $edge7
    connect \B $edge8
    connect \Y $edge1
  end
end
