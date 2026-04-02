; ModuleID = 'swiglu.ll'
source_filename = "swiglu.c"
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
@a = dso_local local_unnamed_addr global [2048 x float] zeroinitializer, align 4
@b = dso_local local_unnamed_addr global [2048 x float] zeroinitializer, align 4
@output = dso_local local_unnamed_addr global [2048 x float] zeroinitializer, align 4

; Function Attrs: nounwind uwtable
define dso_local void @kernel() local_unnamed_addr #0 {
entry:
  %call = tail call i32 @loop_begin() #3
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %call9 = tail call i32 @loop_end() #3
  ret void

for.body:                                         ; preds = %for.body, %entry
  %i.022 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds [2048 x float], ptr @a, i32 0, i32 %i.022
  %0 = load float, ptr %arrayidx, align 4, !tbaa !6
  %arrayidx1 = getelementptr inbounds [2048 x float], ptr @b, i32 0, i32 %i.022
  %1 = load float, ptr %arrayidx1, align 4, !tbaa !6
  %fneg = fneg float %0
  %mul.i = fmul float %0, 0xBFF7154760000000
  %conv.i = fptosi float %mul.i to i32
  %conv1.i = sitofp i32 %conv.i to float
  %neg.i = fneg float %conv1.i
  %2 = tail call float @llvm.fmuladd.f32(float %neg.i, float 0x3FE62E4300000000, float %fneg)
  %add.i = shl i32 %conv.i, 23
  %shl.i = add i32 %add.i, 1065353216
  %3 = bitcast i32 %shl.i to float
  %4 = tail call float @llvm.fmuladd.f32(float %2, float 0x3FA5555560000000, float 0x3FC5555560000000)
  %5 = tail call float @llvm.fmuladd.f32(float %2, float %4, float 5.000000e-01)
  %6 = tail call float @llvm.fmuladd.f32(float %2, float %5, float 1.000000e+00)
  %7 = tail call float @llvm.fmuladd.f32(float %2, float %6, float 1.000000e+00)
  %mul9.i = fmul float %7, %3
  %add = fadd float %mul9.i, 1.000000e+00
  %div = fdiv float 1.000000e+00, %add
  %fneg3 = fmul float %div, %fneg
  %mul.i15 = fmul float %fneg3, 0x3FF7154760000000
  %conv.i16 = fptosi float %mul.i15 to i32
  %conv1.i17 = sitofp i32 %conv.i16 to float
  %neg.i18 = fneg float %conv1.i17
  %8 = tail call float @llvm.fmuladd.f32(float %neg.i18, float 0x3FE62E4300000000, float %fneg3)
  %add.i19 = shl i32 %conv.i16, 23
  %shl.i20 = add i32 %add.i19, 1065353216
  %9 = bitcast i32 %shl.i20 to float
  %10 = tail call float @llvm.fmuladd.f32(float %8, float 0x3FA5555560000000, float 0x3FC5555560000000)
  %11 = tail call float @llvm.fmuladd.f32(float %8, float %10, float 5.000000e-01)
  %12 = tail call float @llvm.fmuladd.f32(float %8, float %11, float 1.000000e+00)
  %13 = tail call float @llvm.fmuladd.f32(float %8, float %12, float 1.000000e+00)
  %mul9.i21 = fmul float %13, %9
  %add5 = fadd float %mul9.i21, 1.000000e+00
  %div6 = fdiv float 1.000000e+00, %add5
  %mul7 = fmul float %1, %div6
  %arrayidx8 = getelementptr inbounds [2048 x float], ptr @output, i32 0, i32 %i.022
  store float %mul7, ptr %arrayidx8, align 4, !tbaa !6
  %inc = add nuw nsw i32 %i.022, 1
  %exitcond.not = icmp eq i32 %inc, 2048
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body, !llvm.loop !10
}

declare i32 @loop_begin(...) local_unnamed_addr #1

declare i32 @loop_end(...) local_unnamed_addr #1

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare float @llvm.fmuladd.f32(float, float, float) #2

attributes #0 = { nounwind uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="i686" "target-features"="+cx8,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="i686" "target-features"="+cx8,+x87" "tune-cpu"="generic" }
attributes #2 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
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
