# SPDLOG_LEVEL=trace, debug, info, warn, err, critical, off
./build/fgra-compiler SPDLOG_LEVEL=off\
		 -c true -m true -o true\
                -t 6000000\
                -i 15\
                -q true\
                -v true\
                -C true\
                -p "../../fgra-mg/src/main/resources/operations.json" \
                -a "../../fgra-mg/src/main/resources/fgra_adg.json" \
                -d "../../benchmarks/soc_test/Fusion_broad_journal/adi/affine.json"
	
	#-d "../../benchmark/Fusion_benchmark/cnn/dai-custom.json ../../benchmark/Fusion_benchmark/csr_matvec/csr_matvec_custom.json ../../benchmark/Fusion_benchmark/lpdc_matching/lpdc_matching.json ../../benchmark/Fusion_benchmark/machsuite/machsuit_crs.json ../../benchmark/Fusion_benchmark/versa_detector/detector.json ../../benchmark/Fusion_benchmark/versa_generator/generator.json ../../benchmark/Fusion_benchmark/wavelib_dwt_sym/wtmath_dwt_sym_4096cycles_new_new.json ../../benchmark/Fusion_benchmark/wavelib_per/per_128_128.json ../../benchmark/Fusion_benchmark/wavelib_sym/sym_128_128.json"
	# -d "../benchmarks/test/arf/arf.json ../benchmarks/test/centro-fir/centro-fir.json ../benchmarks/test/cosine1/cosine1.json ../benchmarks/test/ewf/ewf.json ../benchmarks/test/fft/fft.json ../benchmarks/test/fir1/fir1.json ../benchmarks/test/resnet1/resnet1.json"
	# -d "../benchmarks/test/cosine2/cosine2.json ../benchmarks/test/fir/fir.json ../benchmarks/test/md/md.json ../benchmarks/test/resnet2/resnet2.json ../benchmarks/test/stencil3d/stencil3d.json"	
	# -d "../benchmarks/test/cosine2/cosine2.json ../benchmarks/test/resnet2/resnet2.json ../benchmarks/test/stencil3d/stencil3d.json"
	# -d "../benchmarks/test/arf/arf.json ../benchmarks/test/centro-fir/centro-fir.json ../benchmarks/test/cosine1/cosine1.json ../benchmarks/test/ewf/ewf.json ../benchmarks/test/fft/fft.json ../benchmarks/test/fir/fir.json ../benchmarks/test/fir1/fir1.json ../benchmarks/test/resnet1/resnet1.json"
	# -d "../benchmarks/test/ewf/ewf.json"
	# -d "../benchmarks/test2/conv3/conv3.json ../benchmarks/test2/matrix.mul/matrixmul.json ../benchmarks/test2/simple/simple.json"
