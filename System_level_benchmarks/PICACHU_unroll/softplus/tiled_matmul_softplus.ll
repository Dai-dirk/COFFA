; ModuleID = 'tiled_matmul_softplus.c'
source_filename = "tiled_matmul_softplus.c"
target datalayout = "e-m:e-p:32:32-p270:32:32-p271:32:32-p272:64:64-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

@beta = dso_local local_unnamed_addr constant float 3.000000e+00, align 4
@const1 = dso_local local_unnamed_addr constant float 1.000000e+00, align 4
@A = common dso_local local_unnamed_addr global [32 x [32 x float]] zeroinitializer, align 4
@B = common dso_local local_unnamed_addr global [32 x [32 x float]] zeroinitializer, align 4
@D = common dso_local local_unnamed_addr global [32 x [32 x float]] zeroinitializer, align 4
@igelu_C = common dso_local local_unnamed_addr global [32 x [32 x float]] zeroinitializer, align 4
@C = common dso_local local_unnamed_addr global [32 x [32 x float]] zeroinitializer, align 4

; Function Attrs: nounwind
define dso_local void @kernel() local_unnamed_addr #0 {
entry:
  %Packed_B868 = alloca [1048576 x float], align 64
  %Packed_A870 = alloca [49152 x float], align 64
  %call = tail call i32 bitcast (i32 (...)* @loop_begin to i32 ()*)() #3
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) bitcast ([32 x [32 x float]]* @igelu_C to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 128) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 256) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 384) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 512) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 640) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 768) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 896) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 1024) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 1152) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 1280) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 1408) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 1536) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 1664) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 1792) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 1920) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 2048) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 2176) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 2304) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 2432) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 2560) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 2688) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 2816) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 2944) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 3072) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 3200) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 3328) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 3456) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 3584) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 3712) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 3840) to i32) to i8*), i8 0, i32 128, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull align 4 dereferenceable(128) inttoptr (i32 trunc (i64 add (i64 zext (i32 ptrtoint ([32 x [32 x float]]* @igelu_C to i32) to i64), i64 3968) to i32) to i8*), i8 0, i32 128, i1 false)
  br label %polly.loop_header64

polly.exiting:                                    ; preds = %polly.loop_exit852
  %call28 = tail call i32 bitcast (i32 (...)* @loop_end to i32 ()*)() #3
  ret void

polly.loop_header64:                              ; preds = %entry, %polly.loop_exit72
  %polly.indvar67 = phi i64 [ %polly.indvar_next68, %polly.loop_exit72 ], [ 0, %entry ]
  %pexp.p_div_q = lshr i64 %polly.indvar67, 3
  %polly.access.mul.Packed_B = shl i64 %pexp.p_div_q, 9
  %pexp.pdiv_r = and i64 %polly.indvar67, 7
  br label %polly.loop_header70

polly.loop_exit72:                                ; preds = %polly.loop_header70
  %polly.indvar_next68 = add nuw nsw i64 %polly.indvar67, 1
  %polly.loop_cond69 = icmp ult i64 %polly.indvar67, 31
  br i1 %polly.loop_cond69, label %polly.loop_header64, label %polly.loop_header78

polly.loop_header70:                              ; preds = %polly.loop_header70, %polly.loop_header64
  %polly.indvar73 = phi i64 [ 0, %polly.loop_header64 ], [ %polly.indvar_next74, %polly.loop_header70 ]
  %polly.access.mul.B = shl i64 %polly.indvar73, 5
  %polly.access.add.B = add nuw nsw i64 %polly.access.mul.B, %polly.indvar67
  %0 = trunc i64 %polly.access.add.B to i32
  %polly.access.B = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @B, i32 0, i32 0, i32 %0
  %1 = bitcast float* %polly.access.B to i32*
  %polly.access.B.load867 = load i32, i32* %1, align 4, !alias.scope !3, !noalias !5
  %polly.access.add.Packed_B = add nuw nsw i64 %polly.indvar73, %polly.access.mul.Packed_B
  %polly.access.mul.Packed_B76 = shl nsw i64 %polly.access.add.Packed_B, 3
  %polly.access.add.Packed_B77 = or i64 %polly.access.mul.Packed_B76, %pexp.pdiv_r
  %2 = trunc i64 %polly.access.add.Packed_B77 to i32
  %polly.access.Packed_B = getelementptr [1048576 x float], [1048576 x float]* %Packed_B868, i32 0, i32 %2
  %3 = bitcast float* %polly.access.Packed_B to i32*
  store i32 %polly.access.B.load867, i32* %3, align 4, !alias.scope !9, !noalias !11
  %polly.indvar_next74 = add nuw nsw i64 %polly.indvar73, 1
  %polly.loop_cond75 = icmp ult i64 %polly.indvar73, 31
  br i1 %polly.loop_cond75, label %polly.loop_header70, label %polly.loop_exit72

polly.loop_header78:                              ; preds = %polly.loop_exit72, %polly.loop_header78
  %polly.indvar81 = phi i64 [ %polly.indvar_next82, %polly.loop_header78 ], [ 0, %polly.loop_exit72 ]
  %polly.access.mul.A = shl i64 %polly.indvar81, 5
  %pexp.p_div_q90 = lshr i64 %polly.indvar81, 2
  %polly.access.mul.Packed_A = shl i64 %pexp.p_div_q90, 11
  %pexp.pdiv_r92 = and i64 %polly.indvar81, 3
  %4 = trunc i64 %polly.access.mul.A to i32
  %polly.access.A = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %4
  %5 = bitcast float* %polly.access.A to i32*
  %polly.access.A.load869 = load i32, i32* %5, align 4, !alias.scope !6, !noalias !12
  %polly.access.mul.Packed_A91 = shl i64 %pexp.p_div_q90, 11
  %polly.access.add.Packed_A93 = or i64 %polly.access.mul.Packed_A91, %pexp.pdiv_r92
  %6 = trunc i64 %polly.access.add.Packed_A93 to i32
  %polly.access.Packed_A = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %6
  %7 = bitcast float* %polly.access.Packed_A to i32*
  store i32 %polly.access.A.load869, i32* %7, align 4, !alias.scope !10, !noalias !13
  %8 = trunc i64 %polly.access.mul.A to i32
  %9 = or i32 %8, 1
  %polly.access.A.1 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %9
  %10 = bitcast float* %polly.access.A.1 to i32*
  %polly.access.A.load869.1 = load i32, i32* %10, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.1 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.1 = or i64 %polly.access.add.Packed_A.1, %pexp.pdiv_r92
  %11 = trunc i64 %polly.access.mul.Packed_A91.1 to i32
  %12 = or i32 %11, 4
  %polly.access.Packed_A.1 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %12
  %13 = bitcast float* %polly.access.Packed_A.1 to i32*
  store i32 %polly.access.A.load869.1, i32* %13, align 4, !alias.scope !10, !noalias !13
  %14 = trunc i64 %polly.access.mul.A to i32
  %15 = or i32 %14, 2
  %polly.access.A.2 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %15
  %16 = bitcast float* %polly.access.A.2 to i32*
  %polly.access.A.load869.2 = load i32, i32* %16, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.2 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.2 = or i64 %polly.access.add.Packed_A.2, %pexp.pdiv_r92
  %17 = trunc i64 %polly.access.mul.Packed_A91.2 to i32
  %18 = or i32 %17, 8
  %polly.access.Packed_A.2 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %18
  %19 = bitcast float* %polly.access.Packed_A.2 to i32*
  store i32 %polly.access.A.load869.2, i32* %19, align 4, !alias.scope !10, !noalias !13
  %20 = trunc i64 %polly.access.mul.A to i32
  %21 = or i32 %20, 3
  %polly.access.A.3 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %21
  %22 = bitcast float* %polly.access.A.3 to i32*
  %polly.access.A.load869.3 = load i32, i32* %22, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.3 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.3 = or i64 %polly.access.add.Packed_A.3, %pexp.pdiv_r92
  %23 = trunc i64 %polly.access.mul.Packed_A91.3 to i32
  %24 = or i32 %23, 12
  %polly.access.Packed_A.3 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %24
  %25 = bitcast float* %polly.access.Packed_A.3 to i32*
  store i32 %polly.access.A.load869.3, i32* %25, align 4, !alias.scope !10, !noalias !13
  %26 = trunc i64 %polly.access.mul.A to i32
  %27 = or i32 %26, 4
  %polly.access.A.4 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %27
  %28 = bitcast float* %polly.access.A.4 to i32*
  %polly.access.A.load869.4 = load i32, i32* %28, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.4 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.4 = or i64 %polly.access.add.Packed_A.4, %pexp.pdiv_r92
  %29 = trunc i64 %polly.access.mul.Packed_A91.4 to i32
  %30 = or i32 %29, 16
  %polly.access.Packed_A.4 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %30
  %31 = bitcast float* %polly.access.Packed_A.4 to i32*
  store i32 %polly.access.A.load869.4, i32* %31, align 4, !alias.scope !10, !noalias !13
  %32 = trunc i64 %polly.access.mul.A to i32
  %33 = or i32 %32, 5
  %polly.access.A.5 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %33
  %34 = bitcast float* %polly.access.A.5 to i32*
  %polly.access.A.load869.5 = load i32, i32* %34, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.5 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.5 = or i64 %polly.access.add.Packed_A.5, %pexp.pdiv_r92
  %35 = trunc i64 %polly.access.mul.Packed_A91.5 to i32
  %36 = or i32 %35, 20
  %polly.access.Packed_A.5 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %36
  %37 = bitcast float* %polly.access.Packed_A.5 to i32*
  store i32 %polly.access.A.load869.5, i32* %37, align 4, !alias.scope !10, !noalias !13
  %38 = trunc i64 %polly.access.mul.A to i32
  %39 = or i32 %38, 6
  %polly.access.A.6 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %39
  %40 = bitcast float* %polly.access.A.6 to i32*
  %polly.access.A.load869.6 = load i32, i32* %40, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.6 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.6 = or i64 %polly.access.add.Packed_A.6, %pexp.pdiv_r92
  %41 = trunc i64 %polly.access.mul.Packed_A91.6 to i32
  %42 = or i32 %41, 24
  %polly.access.Packed_A.6 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %42
  %43 = bitcast float* %polly.access.Packed_A.6 to i32*
  store i32 %polly.access.A.load869.6, i32* %43, align 4, !alias.scope !10, !noalias !13
  %44 = trunc i64 %polly.access.mul.A to i32
  %45 = or i32 %44, 7
  %polly.access.A.7 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %45
  %46 = bitcast float* %polly.access.A.7 to i32*
  %polly.access.A.load869.7 = load i32, i32* %46, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.7 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.7 = or i64 %polly.access.add.Packed_A.7, %pexp.pdiv_r92
  %47 = trunc i64 %polly.access.mul.Packed_A91.7 to i32
  %48 = or i32 %47, 28
  %polly.access.Packed_A.7 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %48
  %49 = bitcast float* %polly.access.Packed_A.7 to i32*
  store i32 %polly.access.A.load869.7, i32* %49, align 4, !alias.scope !10, !noalias !13
  %50 = trunc i64 %polly.access.mul.A to i32
  %51 = or i32 %50, 8
  %polly.access.A.8 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %51
  %52 = bitcast float* %polly.access.A.8 to i32*
  %polly.access.A.load869.8 = load i32, i32* %52, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.8 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.8 = or i64 %polly.access.add.Packed_A.8, %pexp.pdiv_r92
  %53 = trunc i64 %polly.access.mul.Packed_A91.8 to i32
  %54 = or i32 %53, 32
  %polly.access.Packed_A.8 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %54
  %55 = bitcast float* %polly.access.Packed_A.8 to i32*
  store i32 %polly.access.A.load869.8, i32* %55, align 4, !alias.scope !10, !noalias !13
  %56 = trunc i64 %polly.access.mul.A to i32
  %57 = or i32 %56, 9
  %polly.access.A.9 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %57
  %58 = bitcast float* %polly.access.A.9 to i32*
  %polly.access.A.load869.9 = load i32, i32* %58, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.9 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.9 = or i64 %polly.access.add.Packed_A.9, %pexp.pdiv_r92
  %59 = trunc i64 %polly.access.mul.Packed_A91.9 to i32
  %60 = or i32 %59, 36
  %polly.access.Packed_A.9 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %60
  %61 = bitcast float* %polly.access.Packed_A.9 to i32*
  store i32 %polly.access.A.load869.9, i32* %61, align 4, !alias.scope !10, !noalias !13
  %62 = trunc i64 %polly.access.mul.A to i32
  %63 = or i32 %62, 10
  %polly.access.A.10 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %63
  %64 = bitcast float* %polly.access.A.10 to i32*
  %polly.access.A.load869.10 = load i32, i32* %64, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.10 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.10 = or i64 %polly.access.add.Packed_A.10, %pexp.pdiv_r92
  %65 = trunc i64 %polly.access.mul.Packed_A91.10 to i32
  %66 = or i32 %65, 40
  %polly.access.Packed_A.10 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %66
  %67 = bitcast float* %polly.access.Packed_A.10 to i32*
  store i32 %polly.access.A.load869.10, i32* %67, align 4, !alias.scope !10, !noalias !13
  %68 = trunc i64 %polly.access.mul.A to i32
  %69 = or i32 %68, 11
  %polly.access.A.11 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %69
  %70 = bitcast float* %polly.access.A.11 to i32*
  %polly.access.A.load869.11 = load i32, i32* %70, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.11 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.11 = or i64 %polly.access.add.Packed_A.11, %pexp.pdiv_r92
  %71 = trunc i64 %polly.access.mul.Packed_A91.11 to i32
  %72 = or i32 %71, 44
  %polly.access.Packed_A.11 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %72
  %73 = bitcast float* %polly.access.Packed_A.11 to i32*
  store i32 %polly.access.A.load869.11, i32* %73, align 4, !alias.scope !10, !noalias !13
  %74 = trunc i64 %polly.access.mul.A to i32
  %75 = or i32 %74, 12
  %polly.access.A.12 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %75
  %76 = bitcast float* %polly.access.A.12 to i32*
  %polly.access.A.load869.12 = load i32, i32* %76, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.12 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.12 = or i64 %polly.access.add.Packed_A.12, %pexp.pdiv_r92
  %77 = trunc i64 %polly.access.mul.Packed_A91.12 to i32
  %78 = or i32 %77, 48
  %polly.access.Packed_A.12 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %78
  %79 = bitcast float* %polly.access.Packed_A.12 to i32*
  store i32 %polly.access.A.load869.12, i32* %79, align 4, !alias.scope !10, !noalias !13
  %80 = trunc i64 %polly.access.mul.A to i32
  %81 = or i32 %80, 13
  %polly.access.A.13 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %81
  %82 = bitcast float* %polly.access.A.13 to i32*
  %polly.access.A.load869.13 = load i32, i32* %82, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.13 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.13 = or i64 %polly.access.add.Packed_A.13, %pexp.pdiv_r92
  %83 = trunc i64 %polly.access.mul.Packed_A91.13 to i32
  %84 = or i32 %83, 52
  %polly.access.Packed_A.13 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %84
  %85 = bitcast float* %polly.access.Packed_A.13 to i32*
  store i32 %polly.access.A.load869.13, i32* %85, align 4, !alias.scope !10, !noalias !13
  %86 = trunc i64 %polly.access.mul.A to i32
  %87 = or i32 %86, 14
  %polly.access.A.14 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %87
  %88 = bitcast float* %polly.access.A.14 to i32*
  %polly.access.A.load869.14 = load i32, i32* %88, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.14 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.14 = or i64 %polly.access.add.Packed_A.14, %pexp.pdiv_r92
  %89 = trunc i64 %polly.access.mul.Packed_A91.14 to i32
  %90 = or i32 %89, 56
  %polly.access.Packed_A.14 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %90
  %91 = bitcast float* %polly.access.Packed_A.14 to i32*
  store i32 %polly.access.A.load869.14, i32* %91, align 4, !alias.scope !10, !noalias !13
  %92 = trunc i64 %polly.access.mul.A to i32
  %93 = or i32 %92, 15
  %polly.access.A.15 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %93
  %94 = bitcast float* %polly.access.A.15 to i32*
  %polly.access.A.load869.15 = load i32, i32* %94, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.15 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.15 = or i64 %polly.access.add.Packed_A.15, %pexp.pdiv_r92
  %95 = trunc i64 %polly.access.mul.Packed_A91.15 to i32
  %96 = or i32 %95, 60
  %polly.access.Packed_A.15 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %96
  %97 = bitcast float* %polly.access.Packed_A.15 to i32*
  store i32 %polly.access.A.load869.15, i32* %97, align 4, !alias.scope !10, !noalias !13
  %98 = trunc i64 %polly.access.mul.A to i32
  %99 = or i32 %98, 16
  %polly.access.A.16 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %99
  %100 = bitcast float* %polly.access.A.16 to i32*
  %polly.access.A.load869.16 = load i32, i32* %100, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.16 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.16 = or i64 %polly.access.add.Packed_A.16, %pexp.pdiv_r92
  %101 = trunc i64 %polly.access.mul.Packed_A91.16 to i32
  %102 = or i32 %101, 64
  %polly.access.Packed_A.16 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %102
  %103 = bitcast float* %polly.access.Packed_A.16 to i32*
  store i32 %polly.access.A.load869.16, i32* %103, align 4, !alias.scope !10, !noalias !13
  %104 = trunc i64 %polly.access.mul.A to i32
  %105 = or i32 %104, 17
  %polly.access.A.17 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %105
  %106 = bitcast float* %polly.access.A.17 to i32*
  %polly.access.A.load869.17 = load i32, i32* %106, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.17 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.17 = or i64 %polly.access.add.Packed_A.17, %pexp.pdiv_r92
  %107 = trunc i64 %polly.access.mul.Packed_A91.17 to i32
  %108 = or i32 %107, 68
  %polly.access.Packed_A.17 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %108
  %109 = bitcast float* %polly.access.Packed_A.17 to i32*
  store i32 %polly.access.A.load869.17, i32* %109, align 4, !alias.scope !10, !noalias !13
  %110 = trunc i64 %polly.access.mul.A to i32
  %111 = or i32 %110, 18
  %polly.access.A.18 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %111
  %112 = bitcast float* %polly.access.A.18 to i32*
  %polly.access.A.load869.18 = load i32, i32* %112, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.18 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.18 = or i64 %polly.access.add.Packed_A.18, %pexp.pdiv_r92
  %113 = trunc i64 %polly.access.mul.Packed_A91.18 to i32
  %114 = or i32 %113, 72
  %polly.access.Packed_A.18 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %114
  %115 = bitcast float* %polly.access.Packed_A.18 to i32*
  store i32 %polly.access.A.load869.18, i32* %115, align 4, !alias.scope !10, !noalias !13
  %116 = trunc i64 %polly.access.mul.A to i32
  %117 = or i32 %116, 19
  %polly.access.A.19 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %117
  %118 = bitcast float* %polly.access.A.19 to i32*
  %polly.access.A.load869.19 = load i32, i32* %118, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.19 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.19 = or i64 %polly.access.add.Packed_A.19, %pexp.pdiv_r92
  %119 = trunc i64 %polly.access.mul.Packed_A91.19 to i32
  %120 = or i32 %119, 76
  %polly.access.Packed_A.19 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %120
  %121 = bitcast float* %polly.access.Packed_A.19 to i32*
  store i32 %polly.access.A.load869.19, i32* %121, align 4, !alias.scope !10, !noalias !13
  %122 = trunc i64 %polly.access.mul.A to i32
  %123 = or i32 %122, 20
  %polly.access.A.20 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %123
  %124 = bitcast float* %polly.access.A.20 to i32*
  %polly.access.A.load869.20 = load i32, i32* %124, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.20 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.20 = or i64 %polly.access.add.Packed_A.20, %pexp.pdiv_r92
  %125 = trunc i64 %polly.access.mul.Packed_A91.20 to i32
  %126 = or i32 %125, 80
  %polly.access.Packed_A.20 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %126
  %127 = bitcast float* %polly.access.Packed_A.20 to i32*
  store i32 %polly.access.A.load869.20, i32* %127, align 4, !alias.scope !10, !noalias !13
  %128 = trunc i64 %polly.access.mul.A to i32
  %129 = or i32 %128, 21
  %polly.access.A.21 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %129
  %130 = bitcast float* %polly.access.A.21 to i32*
  %polly.access.A.load869.21 = load i32, i32* %130, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.21 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.21 = or i64 %polly.access.add.Packed_A.21, %pexp.pdiv_r92
  %131 = trunc i64 %polly.access.mul.Packed_A91.21 to i32
  %132 = or i32 %131, 84
  %polly.access.Packed_A.21 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %132
  %133 = bitcast float* %polly.access.Packed_A.21 to i32*
  store i32 %polly.access.A.load869.21, i32* %133, align 4, !alias.scope !10, !noalias !13
  %134 = trunc i64 %polly.access.mul.A to i32
  %135 = or i32 %134, 22
  %polly.access.A.22 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %135
  %136 = bitcast float* %polly.access.A.22 to i32*
  %polly.access.A.load869.22 = load i32, i32* %136, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.22 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.22 = or i64 %polly.access.add.Packed_A.22, %pexp.pdiv_r92
  %137 = trunc i64 %polly.access.mul.Packed_A91.22 to i32
  %138 = or i32 %137, 88
  %polly.access.Packed_A.22 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %138
  %139 = bitcast float* %polly.access.Packed_A.22 to i32*
  store i32 %polly.access.A.load869.22, i32* %139, align 4, !alias.scope !10, !noalias !13
  %140 = trunc i64 %polly.access.mul.A to i32
  %141 = or i32 %140, 23
  %polly.access.A.23 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %141
  %142 = bitcast float* %polly.access.A.23 to i32*
  %polly.access.A.load869.23 = load i32, i32* %142, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.23 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.23 = or i64 %polly.access.add.Packed_A.23, %pexp.pdiv_r92
  %143 = trunc i64 %polly.access.mul.Packed_A91.23 to i32
  %144 = or i32 %143, 92
  %polly.access.Packed_A.23 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %144
  %145 = bitcast float* %polly.access.Packed_A.23 to i32*
  store i32 %polly.access.A.load869.23, i32* %145, align 4, !alias.scope !10, !noalias !13
  %146 = trunc i64 %polly.access.mul.A to i32
  %147 = or i32 %146, 24
  %polly.access.A.24 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %147
  %148 = bitcast float* %polly.access.A.24 to i32*
  %polly.access.A.load869.24 = load i32, i32* %148, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.24 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.24 = or i64 %polly.access.add.Packed_A.24, %pexp.pdiv_r92
  %149 = trunc i64 %polly.access.mul.Packed_A91.24 to i32
  %150 = or i32 %149, 96
  %polly.access.Packed_A.24 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %150
  %151 = bitcast float* %polly.access.Packed_A.24 to i32*
  store i32 %polly.access.A.load869.24, i32* %151, align 4, !alias.scope !10, !noalias !13
  %152 = trunc i64 %polly.access.mul.A to i32
  %153 = or i32 %152, 25
  %polly.access.A.25 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %153
  %154 = bitcast float* %polly.access.A.25 to i32*
  %polly.access.A.load869.25 = load i32, i32* %154, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.25 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.25 = or i64 %polly.access.add.Packed_A.25, %pexp.pdiv_r92
  %155 = trunc i64 %polly.access.mul.Packed_A91.25 to i32
  %156 = or i32 %155, 100
  %polly.access.Packed_A.25 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %156
  %157 = bitcast float* %polly.access.Packed_A.25 to i32*
  store i32 %polly.access.A.load869.25, i32* %157, align 4, !alias.scope !10, !noalias !13
  %158 = trunc i64 %polly.access.mul.A to i32
  %159 = or i32 %158, 26
  %polly.access.A.26 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %159
  %160 = bitcast float* %polly.access.A.26 to i32*
  %polly.access.A.load869.26 = load i32, i32* %160, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.26 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.26 = or i64 %polly.access.add.Packed_A.26, %pexp.pdiv_r92
  %161 = trunc i64 %polly.access.mul.Packed_A91.26 to i32
  %162 = or i32 %161, 104
  %polly.access.Packed_A.26 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %162
  %163 = bitcast float* %polly.access.Packed_A.26 to i32*
  store i32 %polly.access.A.load869.26, i32* %163, align 4, !alias.scope !10, !noalias !13
  %164 = trunc i64 %polly.access.mul.A to i32
  %165 = or i32 %164, 27
  %polly.access.A.27 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %165
  %166 = bitcast float* %polly.access.A.27 to i32*
  %polly.access.A.load869.27 = load i32, i32* %166, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.27 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.27 = or i64 %polly.access.add.Packed_A.27, %pexp.pdiv_r92
  %167 = trunc i64 %polly.access.mul.Packed_A91.27 to i32
  %168 = or i32 %167, 108
  %polly.access.Packed_A.27 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %168
  %169 = bitcast float* %polly.access.Packed_A.27 to i32*
  store i32 %polly.access.A.load869.27, i32* %169, align 4, !alias.scope !10, !noalias !13
  %170 = trunc i64 %polly.access.mul.A to i32
  %171 = or i32 %170, 28
  %polly.access.A.28 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %171
  %172 = bitcast float* %polly.access.A.28 to i32*
  %polly.access.A.load869.28 = load i32, i32* %172, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.28 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.28 = or i64 %polly.access.add.Packed_A.28, %pexp.pdiv_r92
  %173 = trunc i64 %polly.access.mul.Packed_A91.28 to i32
  %174 = or i32 %173, 112
  %polly.access.Packed_A.28 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %174
  %175 = bitcast float* %polly.access.Packed_A.28 to i32*
  store i32 %polly.access.A.load869.28, i32* %175, align 4, !alias.scope !10, !noalias !13
  %176 = trunc i64 %polly.access.mul.A to i32
  %177 = or i32 %176, 29
  %polly.access.A.29 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %177
  %178 = bitcast float* %polly.access.A.29 to i32*
  %polly.access.A.load869.29 = load i32, i32* %178, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.29 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.29 = or i64 %polly.access.add.Packed_A.29, %pexp.pdiv_r92
  %179 = trunc i64 %polly.access.mul.Packed_A91.29 to i32
  %180 = or i32 %179, 116
  %polly.access.Packed_A.29 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %180
  %181 = bitcast float* %polly.access.Packed_A.29 to i32*
  store i32 %polly.access.A.load869.29, i32* %181, align 4, !alias.scope !10, !noalias !13
  %182 = trunc i64 %polly.access.mul.A to i32
  %183 = or i32 %182, 30
  %polly.access.A.30 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %183
  %184 = bitcast float* %polly.access.A.30 to i32*
  %polly.access.A.load869.30 = load i32, i32* %184, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.30 = shl i64 %pexp.p_div_q90, 11
  %polly.access.mul.Packed_A91.30 = or i64 %polly.access.add.Packed_A.30, %pexp.pdiv_r92
  %185 = trunc i64 %polly.access.mul.Packed_A91.30 to i32
  %186 = or i32 %185, 120
  %polly.access.Packed_A.30 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %186
  %187 = bitcast float* %polly.access.Packed_A.30 to i32*
  store i32 %polly.access.A.load869.30, i32* %187, align 4, !alias.scope !10, !noalias !13
  %188 = trunc i64 %polly.access.mul.A to i32
  %189 = or i32 %188, 31
  %polly.access.A.31 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @A, i32 0, i32 0, i32 %189
  %190 = bitcast float* %polly.access.A.31 to i32*
  %polly.access.A.load869.31 = load i32, i32* %190, align 4, !alias.scope !6, !noalias !12
  %polly.access.add.Packed_A.31 = or i64 %polly.access.mul.Packed_A, %pexp.pdiv_r92
  %191 = trunc i64 %polly.access.add.Packed_A.31 to i32
  %192 = or i32 %191, 124
  %polly.access.Packed_A.31 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %192
  %193 = bitcast float* %polly.access.Packed_A.31 to i32*
  store i32 %polly.access.A.load869.31, i32* %193, align 4, !alias.scope !10, !noalias !13
  %polly.indvar_next82 = add nuw nsw i64 %polly.indvar81, 1
  %polly.loop_cond83 = icmp ult i64 %polly.indvar81, 31
  br i1 %polly.loop_cond83, label %polly.loop_header78, label %polly.loop_header94

polly.loop_header94:                              ; preds = %polly.loop_header78, %polly.loop_exit102
  %polly.indvar97 = phi i64 [ %polly.indvar_next98, %polly.loop_exit102 ], [ 0, %polly.loop_header78 ]
  %194 = shl nsw i64 %polly.indvar97, 3
  %polly.access.mul.Packed_B122 = shl i64 %polly.indvar97, 9
  %195 = or i64 %194, 4
  br label %polly.loop_header100

polly.loop_exit102:                               ; preds = %polly.loop_exit108
  %polly.indvar_next98 = add nuw nsw i64 %polly.indvar97, 1
  %polly.loop_cond99 = icmp ult i64 %polly.indvar97, 3
  br i1 %polly.loop_cond99, label %polly.loop_header94, label %polly.loop_header844

polly.loop_header100:                             ; preds = %polly.loop_exit108, %polly.loop_header94
  %polly.indvar103 = phi i64 [ 0, %polly.loop_header94 ], [ %polly.indvar_next104, %polly.loop_exit108 ]
  %196 = shl i64 %polly.indvar103, 7
  %polly.access.add.igelu_C113 = add nuw nsw i64 %196, %194
  %197 = trunc i64 %polly.access.add.igelu_C113 to i32
  %polly.access.igelu_C114 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @igelu_C, i32 0, i32 0, i32 %197
  %polly.access.mul.Packed_A116 = shl i64 %polly.indvar103, 9
  %polly.access.add.igelu_C202 = add nuw nsw i64 %196, %195
  %198 = trunc i64 %polly.access.add.igelu_C202 to i32
  %polly.access.igelu_C203 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @igelu_C, i32 0, i32 0, i32 %198
  %polly.access.mul.igelu_C293 = or i64 %196, 32
  %polly.access.add.igelu_C294 = add nuw nsw i64 %polly.access.mul.igelu_C293, %194
  %199 = trunc i64 %polly.access.add.igelu_C294 to i32
  %polly.access.igelu_C295 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @igelu_C, i32 0, i32 0, i32 %199
  %polly.access.add.igelu_C386 = add nuw nsw i64 %polly.access.mul.igelu_C293, %195
  %200 = trunc i64 %polly.access.add.igelu_C386 to i32
  %polly.access.igelu_C387 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @igelu_C, i32 0, i32 0, i32 %200
  %polly.access.mul.igelu_C477 = or i64 %196, 64
  %polly.access.add.igelu_C478 = add nuw nsw i64 %polly.access.mul.igelu_C477, %194
  %201 = trunc i64 %polly.access.add.igelu_C478 to i32
  %polly.access.igelu_C479 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @igelu_C, i32 0, i32 0, i32 %201
  %polly.access.add.igelu_C570 = add nuw nsw i64 %polly.access.mul.igelu_C477, %195
  %202 = trunc i64 %polly.access.add.igelu_C570 to i32
  %polly.access.igelu_C571 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @igelu_C, i32 0, i32 0, i32 %202
  %203 = or i64 %196, 96
  %polly.access.add.igelu_C662 = add nuw nsw i64 %203, %194
  %204 = trunc i64 %polly.access.add.igelu_C662 to i32
  %polly.access.igelu_C663 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @igelu_C, i32 0, i32 0, i32 %204
  %polly.access.add.igelu_C754 = add nuw nsw i64 %203, %195
  %205 = trunc i64 %polly.access.add.igelu_C754 to i32
  %polly.access.igelu_C755 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @igelu_C, i32 0, i32 0, i32 %205
  %206 = bitcast float* %polly.access.igelu_C114 to <4 x float>*
  %207 = load <4 x float>, <4 x float>* %206, align 4, !alias.scope !14, !noalias !19
  %208 = bitcast float* %polly.access.igelu_C203 to <4 x float>*
  %209 = load <4 x float>, <4 x float>* %208, align 4, !alias.scope !20, !noalias !25
  %210 = bitcast float* %polly.access.igelu_C295 to <4 x float>*
  %211 = load <4 x float>, <4 x float>* %210, align 4, !alias.scope !26, !noalias !31
  %212 = bitcast float* %polly.access.igelu_C387 to <4 x float>*
  %213 = load <4 x float>, <4 x float>* %212, align 4, !alias.scope !32, !noalias !37
  %214 = bitcast float* %polly.access.igelu_C479 to <4 x float>*
  %215 = load <4 x float>, <4 x float>* %214, align 4, !alias.scope !38, !noalias !43
  %216 = bitcast float* %polly.access.igelu_C571 to <4 x float>*
  %217 = load <4 x float>, <4 x float>* %216, align 4, !alias.scope !44, !noalias !49
  %218 = bitcast float* %polly.access.igelu_C663 to <4 x float>*
  %219 = load <4 x float>, <4 x float>* %218, align 4, !alias.scope !50, !noalias !55
  %220 = bitcast float* %polly.access.igelu_C755 to <4 x float>*
  %221 = load <4 x float>, <4 x float>* %220, align 4, !alias.scope !56, !noalias !61
  br label %polly.loop_header106

polly.loop_exit108:                               ; preds = %polly.loop_header106
  %222 = bitcast float* %polly.access.igelu_C114 to <4 x float>*
  store <4 x float> %245, <4 x float>* %222, align 4, !alias.scope !14, !noalias !19
  %223 = bitcast float* %polly.access.igelu_C203 to <4 x float>*
  store <4 x float> %250, <4 x float>* %223, align 4, !alias.scope !20, !noalias !25
  %224 = bitcast float* %polly.access.igelu_C295 to <4 x float>*
  store <4 x float> %255, <4 x float>* %224, align 4, !alias.scope !26, !noalias !31
  %225 = bitcast float* %polly.access.igelu_C387 to <4 x float>*
  store <4 x float> %257, <4 x float>* %225, align 4, !alias.scope !32, !noalias !37
  %226 = bitcast float* %polly.access.igelu_C479 to <4 x float>*
  store <4 x float> %262, <4 x float>* %226, align 4, !alias.scope !38, !noalias !43
  %227 = bitcast float* %polly.access.igelu_C571 to <4 x float>*
  store <4 x float> %264, <4 x float>* %227, align 4, !alias.scope !44, !noalias !49
  %228 = bitcast float* %polly.access.igelu_C663 to <4 x float>*
  store <4 x float> %269, <4 x float>* %228, align 4, !alias.scope !50, !noalias !55
  %229 = bitcast float* %polly.access.igelu_C755 to <4 x float>*
  store <4 x float> %271, <4 x float>* %229, align 4, !alias.scope !56, !noalias !61
  %polly.indvar_next104 = add nuw nsw i64 %polly.indvar103, 1
  %polly.loop_cond105 = icmp ult i64 %polly.indvar103, 7
  br i1 %polly.loop_cond105, label %polly.loop_header100, label %polly.loop_exit102

polly.loop_header106:                             ; preds = %polly.loop_header106, %polly.loop_header100
  %polly.indvar109 = phi i64 [ 0, %polly.loop_header100 ], [ %polly.indvar_next110, %polly.loop_header106 ]
  %230 = phi <4 x float> [ %219, %polly.loop_header100 ], [ %269, %polly.loop_header106 ]
  %231 = phi <4 x float> [ %221, %polly.loop_header100 ], [ %271, %polly.loop_header106 ]
  %232 = phi <4 x float> [ %215, %polly.loop_header100 ], [ %262, %polly.loop_header106 ]
  %233 = phi <4 x float> [ %217, %polly.loop_header100 ], [ %264, %polly.loop_header106 ]
  %234 = phi <4 x float> [ %211, %polly.loop_header100 ], [ %255, %polly.loop_header106 ]
  %235 = phi <4 x float> [ %213, %polly.loop_header100 ], [ %257, %polly.loop_header106 ]
  %236 = phi <4 x float> [ %207, %polly.loop_header100 ], [ %245, %polly.loop_header106 ]
  %237 = phi <4 x float> [ %209, %polly.loop_header100 ], [ %250, %polly.loop_header106 ]
  %polly.access.add.Packed_A117 = add nuw nsw i64 %polly.indvar109, %polly.access.mul.Packed_A116
  %polly.access.add.Packed_A117.tr = trunc i64 %polly.access.add.Packed_A117 to i32
  %238 = shl i32 %polly.access.add.Packed_A117.tr, 2
  %polly.access.Packed_A120 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %238
  %_p_scalar_ = load float, float* %polly.access.Packed_A120, align 16, !alias.scope !10, !noalias !13
  %polly.access.add.Packed_B123 = add nuw nsw i64 %polly.indvar109, %polly.access.mul.Packed_B122
  %polly.access.add.Packed_B123.tr = trunc i64 %polly.access.add.Packed_B123 to i32
  %239 = shl i32 %polly.access.add.Packed_B123.tr, 3
  %polly.access.Packed_B126 = getelementptr [1048576 x float], [1048576 x float]* %Packed_B868, i32 0, i32 %239
  %240 = bitcast float* %polly.access.Packed_B126 to <4 x float>*
  %241 = load <4 x float>, <4 x float>* %240, align 32, !alias.scope !9, !noalias !11
  %242 = insertelement <4 x float> undef, float %_p_scalar_, i32 0
  %243 = shufflevector <4 x float> %242, <4 x float> undef, <4 x i32> zeroinitializer
  %244 = fmul <4 x float> %243, %241
  %245 = fadd <4 x float> %236, %244
  %246 = or i32 %239, 4
  %polly.access.Packed_B216 = getelementptr [1048576 x float], [1048576 x float]* %Packed_B868, i32 0, i32 %246
  %247 = bitcast float* %polly.access.Packed_B216 to <4 x float>*
  %248 = load <4 x float>, <4 x float>* %247, align 16, !alias.scope !9, !noalias !11
  %249 = fmul <4 x float> %243, %248
  %250 = fadd <4 x float> %237, %249
  %251 = or i32 %238, 1
  %polly.access.Packed_A301 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %251
  %_p_scalar_302 = load float, float* %polly.access.Packed_A301, align 4, !alias.scope !10, !noalias !13
  %252 = insertelement <4 x float> undef, float %_p_scalar_302, i32 0
  %253 = shufflevector <4 x float> %252, <4 x float> undef, <4 x i32> zeroinitializer
  %254 = fmul <4 x float> %241, %253
  %255 = fadd <4 x float> %234, %254
  %256 = fmul <4 x float> %248, %253
  %257 = fadd <4 x float> %256, %235
  %258 = or i32 %238, 2
  %polly.access.Packed_A485 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %258
  %_p_scalar_486 = load float, float* %polly.access.Packed_A485, align 8, !alias.scope !10, !noalias !13
  %259 = insertelement <4 x float> undef, float %_p_scalar_486, i32 0
  %260 = shufflevector <4 x float> %259, <4 x float> undef, <4 x i32> zeroinitializer
  %261 = fmul <4 x float> %241, %260
  %262 = fadd <4 x float> %232, %261
  %263 = fmul <4 x float> %248, %260
  %264 = fadd <4 x float> %263, %233
  %265 = or i32 %238, 3
  %polly.access.Packed_A669 = getelementptr [49152 x float], [49152 x float]* %Packed_A870, i32 0, i32 %265
  %_p_scalar_670 = load float, float* %polly.access.Packed_A669, align 4, !alias.scope !10, !noalias !13
  %266 = insertelement <4 x float> undef, float %_p_scalar_670, i32 0
  %267 = shufflevector <4 x float> %266, <4 x float> undef, <4 x i32> zeroinitializer
  %268 = fmul <4 x float> %241, %267
  %269 = fadd <4 x float> %230, %268
  %270 = fmul <4 x float> %248, %267
  %271 = fadd <4 x float> %270, %231
  %polly.indvar_next110 = add nuw nsw i64 %polly.indvar109, 1
  %polly.loop_cond111 = icmp ult i64 %polly.indvar109, 31
  br i1 %polly.loop_cond111, label %polly.loop_header106, label %polly.loop_exit108, !llvm.loop !62

polly.loop_header844:                             ; preds = %polly.loop_exit102, %polly.loop_exit852
  %polly.indvar847 = phi i64 [ %polly.indvar_next848, %polly.loop_exit852 ], [ 0, %polly.loop_exit102 ]
  %272 = trunc i64 %polly.indvar847 to i32
  %polly.access.mul.igelu_C856 = shl i64 %polly.indvar847, 5
  br label %polly.loop_header850

polly.loop_exit852:                               ; preds = %polly.loop_header850
  %polly.indvar_next848 = add nuw nsw i64 %polly.indvar847, 1
  %polly.loop_cond849 = icmp ult i64 %polly.indvar847, 31
  br i1 %polly.loop_cond849, label %polly.loop_header844, label %polly.exiting

polly.loop_header850:                             ; preds = %polly.loop_header850, %polly.loop_header844
  %polly.indvar853 = phi i64 [ 0, %polly.loop_header844 ], [ %polly.indvar_next854, %polly.loop_header850 ]
  %polly.access.add.igelu_C857 = add nuw nsw i64 %polly.indvar853, %polly.access.mul.igelu_C856
  %273 = trunc i64 %polly.access.add.igelu_C857 to i32
  %polly.access.igelu_C858 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @igelu_C, i32 0, i32 0, i32 %273
  %polly.access.igelu_C858.reload = load float, float* %polly.access.igelu_C858, align 4, !alias.scope !64, !noalias !65
  %274 = trunc i64 %polly.indvar853 to i32
  %scevgep859 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @D, i32 0, i32 %272, i32 %274
  %_p_scalar_860 = load float, float* %scevgep859, align 4, !alias.scope !7, !noalias !66
  %p_add14 = fadd float %polly.access.igelu_C858.reload, %_p_scalar_860
  %p_mul15 = fmul float %p_add14, 3.000000e+00
  %p_mul.i = fmul float %p_mul15, 0x3FF7154760000000
  %p_conv.i = fptosi float %p_mul.i to i32
  %p_conv1.i = sitofp i32 %p_conv.i to float
  %p_mul2.i = fmul float %p_conv1.i, 0x3FE62E4300000000
  %p_sub.i = fsub float %p_mul15, %p_mul2.i
  %p_add.i = shl i32 %p_conv.i, 23
  %p_shl.i = add i32 %p_add.i, 1065353216
  %p_ = bitcast i32 %p_shl.i to float
  %p_mul6.i = fmul float %p_sub.i, 0x3FA5555560000000
  %p_add7.i = fadd float %p_mul6.i, 0x3FC5555560000000
  %p_mul8.i = fmul float %p_sub.i, %p_add7.i
  %p_add9.i = fadd float %p_mul8.i, 5.000000e-01
  %p_mul10.i = fmul float %p_sub.i, %p_add9.i
  %p_add11.i = fadd float %p_mul10.i, 1.000000e+00
  %p_mul12.i = fmul float %p_sub.i, %p_add11.i
  %p_add13.i = fadd float %p_mul12.i, 1.000000e+00
  %p_mul14.i = fmul float %p_add13.i, %p_
  %p_add17 = fadd float %p_mul14.i, 1.000000e+00
  %p_861 = bitcast float %p_add17 to i32
  %p_862 = lshr i32 %p_861, 23
  %p_and.i = and i32 %p_862, 255
  %p_sub.i47 = add nsw i32 %p_and.i, -127
  %p_and1.i = and i32 %p_861, 8388607
  %p_or.i = or i32 %p_and1.i, 1065353216
  %p_863 = bitcast i32 %p_or.i to float
  %p_sub2.i = fadd float %p_863, -1.000000e+00
  %p_mul.i48 = fmul float %p_sub2.i, 2.500000e-01
  %p_864 = fsub float 0x3FD5555560000000, %p_mul.i48
  %p_mul3.i = fmul float %p_sub2.i, %p_864
  %p_add4.i = fadd float %p_mul3.i, -5.000000e-01
  %p_mul5.i = fmul float %p_sub2.i, %p_add4.i
  %p_add6.i = fadd float %p_mul5.i, 1.000000e+00
  %p_mul7.i = fmul float %p_sub2.i, %p_add6.i
  %p_conv.i49 = sitofp i32 %p_sub.i47 to float
  %p_mul8.i50 = fmul float %p_mul7.i, 0x3FF7154760000000
  %p_add9.i51 = fadd float %p_mul8.i50, %p_conv.i49
  %p_mul19 = fmul float %p_add9.i51, 0x3FD5555560000000
  %scevgep866 = getelementptr [32 x [32 x float]], [32 x [32 x float]]* @igelu_C, i32 0, i32 %272, i32 %274
  store float %p_mul19, float* %scevgep866, align 4, !alias.scope !64, !noalias !65
  %polly.indvar_next854 = add nuw nsw i64 %polly.indvar853, 1
  %polly.loop_cond855 = icmp ult i64 %polly.indvar853, 31
  br i1 %polly.loop_cond855, label %polly.loop_header850, label %polly.loop_exit852
}

declare dso_local i32 @loop_begin(...) local_unnamed_addr #1

declare dso_local i32 @loop_end(...) local_unnamed_addr #1

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.memset.p0i8.i32(i8* nocapture writeonly, i8, i32, i1 immarg) #2

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "polly-optimized" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind willreturn }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"NumRegisterParameters", i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 10.0.0 "}
!3 = distinct !{!3, !4, !"polly.alias.scope.MemRef2"}
!4 = distinct !{!4, !"polly.alias.scope.domain"}
!5 = !{!6, !7, !8, !9, !10}
!6 = distinct !{!6, !4, !"polly.alias.scope.MemRef1"}
!7 = distinct !{!7, !4, !"polly.alias.scope.MemRef4"}
!8 = distinct !{!8, !4, !"polly.alias.scope.MemRef5"}
!9 = distinct !{!9, !4, !"polly.alias.scope.Packed_B"}
!10 = distinct !{!10, !4, !"polly.alias.scope.Packed_A"}
!11 = !{!6, !3, !7, !8, !10}
!12 = !{!3, !7, !8, !9, !10}
!13 = !{!6, !3, !7, !8, !9}
!14 = !{!15, !8, !"second level alias metadata", !16, !17, !18}
!15 = distinct !{!15, !8, !"second level alias metadata"}
!16 = distinct !{!16, !8, !"second level alias metadata"}
!17 = distinct !{!17, !8, !"second level alias metadata"}
!18 = distinct !{!18, !8, !"second level alias metadata"}
!19 = !{!6, !3, !7, !9, !10}
!20 = !{!21, !8, !"second level alias metadata", !22, !23, !24}
!21 = distinct !{!21, !8, !"second level alias metadata"}
!22 = distinct !{!22, !8, !"second level alias metadata"}
!23 = distinct !{!23, !8, !"second level alias metadata"}
!24 = distinct !{!24, !8, !"second level alias metadata"}
!25 = !{!6, !3, !7, !9, !10, !15, !16, !17, !18}
!26 = !{!27, !8, !"second level alias metadata", !28, !29, !30}
!27 = distinct !{!27, !8, !"second level alias metadata"}
!28 = distinct !{!28, !8, !"second level alias metadata"}
!29 = distinct !{!29, !8, !"second level alias metadata"}
!30 = distinct !{!30, !8, !"second level alias metadata"}
!31 = !{!6, !3, !7, !9, !10, !15, !16, !17, !18, !21, !22, !23, !24}
!32 = !{!33, !8, !"second level alias metadata", !34, !35, !36}
!33 = distinct !{!33, !8, !"second level alias metadata"}
!34 = distinct !{!34, !8, !"second level alias metadata"}
!35 = distinct !{!35, !8, !"second level alias metadata"}
!36 = distinct !{!36, !8, !"second level alias metadata"}
!37 = !{!6, !3, !7, !9, !10, !15, !16, !17, !18, !21, !22, !23, !24, !27, !28, !29, !30}
!38 = !{!39, !8, !"second level alias metadata", !40, !41, !42}
!39 = distinct !{!39, !8, !"second level alias metadata"}
!40 = distinct !{!40, !8, !"second level alias metadata"}
!41 = distinct !{!41, !8, !"second level alias metadata"}
!42 = distinct !{!42, !8, !"second level alias metadata"}
!43 = !{!6, !3, !7, !9, !10, !15, !16, !17, !18, !21, !22, !23, !24, !27, !28, !29, !30, !33, !34, !35, !36}
!44 = !{!45, !8, !"second level alias metadata", !46, !47, !48}
!45 = distinct !{!45, !8, !"second level alias metadata"}
!46 = distinct !{!46, !8, !"second level alias metadata"}
!47 = distinct !{!47, !8, !"second level alias metadata"}
!48 = distinct !{!48, !8, !"second level alias metadata"}
!49 = !{!6, !3, !7, !9, !10, !15, !16, !17, !18, !21, !22, !23, !24, !27, !28, !29, !30, !33, !34, !35, !36, !39, !40, !41, !42}
!50 = !{!51, !8, !"second level alias metadata", !52, !53, !54}
!51 = distinct !{!51, !8, !"second level alias metadata"}
!52 = distinct !{!52, !8, !"second level alias metadata"}
!53 = distinct !{!53, !8, !"second level alias metadata"}
!54 = distinct !{!54, !8, !"second level alias metadata"}
!55 = !{!6, !3, !7, !9, !10, !15, !16, !17, !18, !21, !22, !23, !24, !27, !28, !29, !30, !33, !34, !35, !36, !39, !40, !41, !42, !45, !46, !47, !48}
!56 = !{!57, !8, !"second level alias metadata", !58, !59, !60}
!57 = distinct !{!57, !8, !"second level alias metadata"}
!58 = distinct !{!58, !8, !"second level alias metadata"}
!59 = distinct !{!59, !8, !"second level alias metadata"}
!60 = distinct !{!60, !8, !"second level alias metadata"}
!61 = !{!6, !3, !7, !9, !10, !15, !16, !17, !18, !21, !22, !23, !24, !27, !28, !29, !30, !33, !34, !35, !36, !39, !40, !41, !42, !45, !46, !47, !48, !51, !52, !53, !54}
!62 = distinct !{!62, !63}
!63 = !{!"llvm.loop.vectorize.enable", i1 false}
!64 = distinct !{!64, !8, !"second level alias metadata"}
!65 = !{!6, !3, !7, !9, !10, !15, !16, !17, !18, !21, !22, !23, !24, !27, !28, !29, !30, !33, !34, !35, !36, !39, !40, !41, !42, !45, !46, !47, !48, !51, !52, !53, !54, !57, !58, !59, !60}
!66 = !{!6, !3, !8, !9, !10}
