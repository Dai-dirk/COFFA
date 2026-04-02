; ModuleID = 'softmax_3.c'
source_filename = "softmax_3.c"
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
@input = common dso_local local_unnamed_addr global [2048 x float] zeroinitializer, align 4
@sum_arr = common dso_local local_unnamed_addr global [1 x float] zeroinitializer, align 4
@output = common dso_local local_unnamed_addr global [2048 x float] zeroinitializer, align 4
@input_buf = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 4
@output_buf = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 4

; Function Attrs: nounwind
define dso_local void @kernel() local_unnamed_addr #0 {
entry:
  %call = tail call i32 bitcast (i32 (...)* @loop_begin to i32 ()*)() #2
  %0 = load float, float* getelementptr inbounds ([1 x float], [1 x float]* @sum_arr, i32 0, i32 0), align 4, !tbaa !3
  %1 = insertelement <4 x float> undef, float %0, i32 0
  %2 = shufflevector <4 x float> %1, <4 x float> undef, <4 x i32> zeroinitializer
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %call2 = tail call i32 bitcast (i32 (...)* @loop_end to i32 ()*)() #2
  ret void

for.body:                                         ; preds = %for.body, %entry
  %i.07 = phi i32 [ 0, %entry ], [ %inc.3, %for.body ]
  %arrayidx = getelementptr inbounds [2048 x float], [2048 x float]* @input, i32 0, i32 %i.07
  %arrayidx1 = getelementptr inbounds [2048 x float], [2048 x float]* @output, i32 0, i32 %i.07
  %3 = bitcast float* %arrayidx to <4 x float>*
  %4 = load <4 x float>, <4 x float>* %3, align 4, !tbaa !3
  %5 = fdiv <4 x float> %4, %2
  %6 = bitcast float* %arrayidx1 to <4 x float>*
  store <4 x float> %5, <4 x float>* %6, align 4, !tbaa !3
  %inc.3 = add nuw nsw i32 %i.07, 4
  %exitcond.3 = icmp eq i32 %inc.3, 2048
  br i1 %exitcond.3, label %for.cond.cleanup, label %for.body, !llvm.loop !7
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
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.unroll.disable"}
