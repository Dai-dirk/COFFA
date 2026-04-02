yosys -import

# handle arguments
set lut 4
set infiles ""
set outfile ""
set arg_state flag
foreach arg $argv {
	switch $arg_state {
		flag {
			switch -- $arg {
				~i	{ set arg_state in  }
				~o	{ set arg_state out }
				~l	{ set arg_state lut }				
				~v	{ set arg_state viz }
			}
		}
		in {
			lappend infiles $arg
			set arg_state flag
		}		
		lut {
			set lut $arg
			set arg_state flag
		}		
		out {
			set outfile $arg
			set arg_state flag
		}		
		viz {
			set viz $arg
			set arg_state flag
		}
	}
}


# step 1: load internal cells library
read_verilog -lib ./Syn/lib.v

# step 2: load the rtlil file of the design
read_rtlil $infiles

# step 3: early opptimization
opt

# step 4: check the problem in the design
check

# step 5: mapping the fine-grain cells to Yosys internal abc cells
techmap -map +/techmap.v

# step 6: using abc for technology mapping
abc -lut $lut

# step 6: optimization
opt

# step 7: check the problem in the design
check

# step 8: print the statistics of the design
stat

# step 9: dump the mapped rtlil file
dump -o $outfile *

# generate the schematics
if {$viz == "true"} {
	show
}
