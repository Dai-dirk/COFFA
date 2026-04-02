; ModuleID = 'mish.c'
source_filename = "mish.c"
target datalayout = "e-m:e-p:32:32-p270:32:32-p271:32:32-p272:64:64-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

@c1 = dso_local local_unnamed_addr constant float 5.000000e+00, align 4
@c2 = dso_local local_unnamed_addr constant float 6.000000e+00, align 4
@c3 = dso_local local_unnamed_addr constant float 1.000000e+00, align 4
@logc1 = dso_local local_unnamed_addr constant float 1.100000e+01, align 4
@logc2 = dso_local local_unnamed_addr constant float 1.200000e+01, align 4
@logc3 = dso_local local_unnamed_addr constant float 1.300000e+01, align 4
@log2e = dso_local local_unnamed_addr constant float 1.000000e+01, align 4
@bias = dso_local local_unnamed_addr constant float 1.270000e+02, align 4
@pi2 = dso_local local_unnamed_addr constant float 7.000000e+00, align 4
@pi = dso_local local_unnamed_addr constant float 8.000000e+00, align 4
@pi_2 = dso_local local_unnamed_addr constant float 9.000000e+00, align 4
@beta1 = dso_local local_unnamed_addr constant float 3.000000e+00, align 4
@beta2 = dso_local local_unnamed_addr constant float 5.000000e+00, align 4
@const1 = dso_local local_unnamed_addr constant float 1.000000e+00, align 4
@const2 = dso_local local_unnamed_addr constant float -3.000000e+00, align 4
@input = dso_local global [2048 x float] zeroinitializer, align 4
@output = dso_local local_unnamed_addr global [2048 x float] zeroinitializer, align 4

; Function Attrs: nounwind uwtable
define dso_local void @kernel() local_unnamed_addr #0 {
entry:
  %call = tail call i32 @loop_begin() #3
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %call9 = tail call i32 @loop_end() #3
  ret void

for.body:                                         ; preds = %entry, %for.body
  %i.024 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds [2048 x float], ptr @input, i32 0, i32 %i.024
  %0 = load volatile float, ptr %arrayidx, align 4, !tbaa !6
  %mul = fmul float %0, 5.000000e+00
  %mul.i = fmul float %mul, 0x3FF7154760000000
  %conv.i = fptosi float %mul.i to i32
  %conv1.i = sitofp i32 %conv.i to float
  %neg.i = fneg float %conv1.i
  %1 = tail call float @llvm.fmuladd.f32(float %neg.i, float 0x3FE62E4300000000, float %mul)
  %add.i = shl i32 %conv.i, 23
  %shl.i = add i32 %add.i, 1065353216
  %2 = bitcast i32 %shl.i to float
  %3 = tail call float @llvm.fmuladd.f32(float %1, float 0x3FA5555560000000, float 0x3FC5555560000000)
  %4 = tail call float @llvm.fmuladd.f32(float %1, float %3, float 5.000000e-01)
  %5 = tail call float @llvm.fmuladd.f32(float %1, float %4, float 1.000000e+00)
  %6 = tail call float @llvm.fmuladd.f32(float %1, float %5, float 1.000000e+00)
  %mul9.i = fmul float %6, %2
  %add = fadd float %mul9.i, 1.000000e+00
  %7 = bitcast float %add to i32
  %8 = lshr i32 %7, 23
  %and.i = and i32 %8, 255
  %sub.i = add nsw i32 %and.i, -127
  %and1.i = and i32 %7, 8388607
  %or.i = or i32 %and1.i, 1065353216
  %9 = bitcast i32 %or.i to float
  %sub2.i = fadd float %9, -1.000000e+00
  %10 = tail call float @llvm.fmuladd.f32(float %sub2.i, float -2.500000e-01, float 0x3FD5555560000000)
  %11 = tail call float @llvm.fmuladd.f32(float %sub2.i, float %10, float -5.000000e-01)
  %12 = tail call float @llvm.fmuladd.f32(float %sub2.i, float %11, float 1.000000e+00)
  %mul.i15 = fmul float %sub2.i, %12
  %conv.i16 = sitofp i32 %sub.i to float
  %13 = tail call float @llvm.fmuladd.f32(float %mul.i15, float 0x3FF7154760000000, float %conv.i16)
  %mul3 = fmul float %13, -3.000000e+00
  %mul.i17 = fmul float %mul3, 0x3FF7154760000000
  %conv.i18 = fptosi float %mul.i17 to i32
  %conv1.i19 = sitofp i32 %conv.i18 to float
  %neg.i20 = fneg float %conv1.i19
  %14 = tail call float @llvm.fmuladd.f32(float %neg.i20, float 0x3FE62E4300000000, float %mul3)
  %add.i21 = shl i32 %conv.i18, 23
  %shl.i22 = add i32 %add.i21, 1065353216
  %15 = bitcast i32 %shl.i22 to float
  %16 = tail call float @llvm.fmuladd.f32(float %14, float 0x3FA5555560000000, float 0x3FC5555560000000)
  %17 = tail call float @llvm.fmuladd.f32(float %14, float %16, float 5.000000e-01)
  %18 = tail call float @llvm.fmuladd.f32(float %14, float %17, float 1.000000e+00)
  %19 = tail call float @llvm.fmuladd.f32(float %14, float %18, float 1.000000e+00)
  %mul9.i23 = fmul float %19, %15
  %20 = load volatile float, ptr %arrayidx, align 4, !tbaa !6
  %sub = fsub float 1.000000e+00, %mul9.i23
  %mul6 = fmul float %20, %sub
  %add7 = fadd float %mul9.i23, 1.000000e+00
  %div = fdiv float %mul6, %add7
  %arrayidx8 = getelementptr inbounds [2048 x float], ptr @output, i32 0, i32 %i.024
  store float %div, ptr %arrayidx8, align 4, !tbaa !6
  %inc = add nuw nsw i32 %i.024, 1
  %exitcond.not = icmp eq i32 %inc, 2048
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body, !llvm.loop !10
}

declare i32 @loop_begin(...) local_unnamed_addr #1

declare i32 @loop_end(...) local_unnamed_addr #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind readnone speculatable willreturn
declare float @llvm.fmuladd.f32(float, float, float) #2

attributes #0 = { nounwind uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="i686" "target-features"="+cx8,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="i686" "target-features"="+cx8,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"NumRegisterParameters", i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{i32 7, !"PIE Level", i32 2}
!4 = !{i32 7, !"uwtable", i32 2}
!5 = !{!"Debian clang version 15.0.6"}
!6 = !{!7, !7, i64 0}
!7 = !{!"float", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = distinct !{!10, !11, !12}
!11 = !{!"llvm.loop.mustprogress"}
!12 = !{!"llvm.loop.unroll.disable"}
