; ModuleID = 'svm.ll'
source_filename = "svm.c"
target datalayout = "e-m:e-p:32:32-p270:32:32-p271:32:32-p272:64:64-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

@sum = dso_local local_unnamed_addr global i32 0, align 4
@test_vector = common dso_local local_unnamed_addr global [64 x float] zeroinitializer, align 4
@sup_vectors = common dso_local local_unnamed_addr global [64 x [64 x float]] zeroinitializer, align 4
@sv_coeff = common dso_local local_unnamed_addr global [64 x float] zeroinitializer, align 4

; Function Attrs: nounwind
define dso_local void @kernel() local_unnamed_addr #0 {
entry:
  %call = tail call i32 bitcast (i32 (...)* @loop_begin to i32 ()*)() #2
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %i.034 = phi i32 [ 0, %entry ], [ %inc13, %for.cond.cleanup3 ]
  %tmp.033 = phi i32 [ undef, %entry ], [ %add11, %for.cond.cleanup3 ]
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  %add11.lcssa = phi i32 [ %add11, %for.cond.cleanup3 ]
  store i32 %add11.lcssa, i32* @sum, align 4, !tbaa !3
  %call15 = tail call i32 bitcast (i32 (...)* @loop_end to i32 ()*)() #2
  ret void

for.cond.cleanup3:                                ; preds = %for.body4
  %add.3.lcssa = phi float [ %add.3, %for.body4 ]
  %mul7 = fmul float %add.3.lcssa, 0xBFA99999A0000000
  %mul.i = fmul float %mul7, 0x3FF7154760000000
  %conv.i = fptosi float %mul.i to i32
  %conv1.i = sitofp i32 %conv.i to float
  %mul2.i = fmul float %conv1.i, 0x3FE62E4300000000
  %sub.i = fsub float %mul7, %mul2.i
  %add.i = shl i32 %conv.i, 23
  %shl.i = add i32 %add.i, 1065353216
  %0 = bitcast i32 %shl.i to float
  %mul6.i = fmul float %sub.i, 0x3FA5555560000000
  %add7.i = fadd float %mul6.i, 0x3FC5555560000000
  %mul8.i = fmul float %sub.i, %add7.i
  %add9.i = fadd float %mul8.i, 5.000000e-01
  %mul10.i = fmul float %sub.i, %add9.i
  %add11.i = fadd float %mul10.i, 1.000000e+00
  %mul12.i = fmul float %sub.i, %add11.i
  %add13.i = fadd float %mul12.i, 1.000000e+00
  %mul14.i = fmul float %add13.i, %0
  %arrayidx9 = getelementptr inbounds [64 x float], [64 x float]* @sv_coeff, i32 0, i32 %i.034
  %1 = load float, float* %arrayidx9, align 4, !tbaa !7
  %mul10 = fmul float %mul14.i, %1
  %conv = fptosi float %mul10 to i32
  %add11 = add nsw i32 %tmp.033, %conv
  %inc13 = add nuw nsw i32 %i.034, 1
  %exitcond = icmp eq i32 %inc13, 64
  br i1 %exitcond, label %for.cond.cleanup, label %for.cond1.preheader

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %j.032 = phi i32 [ 0, %for.cond1.preheader ], [ %inc.3, %for.body4 ]
  %norma.131 = phi float [ 0.000000e+00, %for.cond1.preheader ], [ %add.3, %for.body4 ]
  %arrayidx = getelementptr inbounds [64 x float], [64 x float]* @test_vector, i32 0, i32 %j.032
  %2 = load float, float* %arrayidx, align 4, !tbaa !7
  %arrayidx6 = getelementptr inbounds [64 x [64 x float]], [64 x [64 x float]]* @sup_vectors, i32 0, i32 %j.032, i32 %i.034
  %3 = load float, float* %arrayidx6, align 4, !tbaa !7
  %sub = fsub float %2, %3
  %mul = fmul float %sub, %sub
  %add = fadd float %norma.131, %mul
  %inc = or i32 %j.032, 1
  %arrayidx.1 = getelementptr inbounds [64 x float], [64 x float]* @test_vector, i32 0, i32 %inc
  %arrayidx6.1 = getelementptr inbounds [64 x [64 x float]], [64 x [64 x float]]* @sup_vectors, i32 0, i32 %inc, i32 %i.034
  %4 = load float, float* %arrayidx6.1, align 4, !tbaa !7
  %inc.1 = or i32 %j.032, 2
  %5 = bitcast float* %arrayidx.1 to <2 x float>*
  %6 = load <2 x float>, <2 x float>* %5, align 4, !tbaa !7
  %arrayidx6.2 = getelementptr inbounds [64 x [64 x float]], [64 x [64 x float]]* @sup_vectors, i32 0, i32 %inc.1, i32 %i.034
  %7 = load float, float* %arrayidx6.2, align 4, !tbaa !7
  %8 = insertelement <2 x float> undef, float %4, i32 0
  %9 = insertelement <2 x float> %8, float %7, i32 1
  %10 = fsub <2 x float> %6, %9
  %11 = fmul <2 x float> %10, %10
  %12 = extractelement <2 x float> %11, i32 0
  %add.1 = fadd float %add, %12
  %13 = extractelement <2 x float> %11, i32 1
  %add.2 = fadd float %add.1, %13
  %inc.2 = or i32 %j.032, 3
  %arrayidx.3 = getelementptr inbounds [64 x float], [64 x float]* @test_vector, i32 0, i32 %inc.2
  %14 = load float, float* %arrayidx.3, align 4, !tbaa !7
  %arrayidx6.3 = getelementptr inbounds [64 x [64 x float]], [64 x [64 x float]]* @sup_vectors, i32 0, i32 %inc.2, i32 %i.034
  %15 = load float, float* %arrayidx6.3, align 4, !tbaa !7
  %sub.3 = fsub float %14, %15
  %mul.3 = fmul float %sub.3, %sub.3
  %add.3 = fadd float %add.2, %mul.3
  %inc.3 = add nuw nsw i32 %j.032, 4
  %exitcond.3 = icmp eq i32 %inc.3, 64
  br i1 %exitcond.3, label %for.cond.cleanup3, label %for.body4, !llvm.loop !9
}

declare dso_local i32 @loop_begin(...) local_unnamed_addr #1

declare dso_local i32 @loop_end(...) local_unnamed_addr #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"NumRegisterParameters", i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 10.0.0 "}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"float", !5, i64 0}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.unroll.disable"}
