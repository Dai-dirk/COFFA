; ModuleID = 'softmax_gvn.ll'
source_filename = "softmax.c"
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
@max_arr = dso_local local_unnamed_addr global float 0.000000e+00, align 4
@input = dso_local local_unnamed_addr global [2048 x float] zeroinitializer, align 4
@output = dso_local local_unnamed_addr global [2048 x float] zeroinitializer, align 4
@input_buf = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 4
@output_buf = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 4
@sum_arr = dso_local local_unnamed_addr global float 0.000000e+00, align 4

; Function Attrs: nounwind uwtable
define dso_local void @kernel(ptr nocapture noundef readonly %input, ptr nocapture noundef readnone %output) local_unnamed_addr #0 {
entry:
  %0 = load float, ptr @max_arr, align 4, !tbaa !6
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %maxx.1.lcssa = phi float [ %maxx.1, %for.body ]
  store float %maxx.1.lcssa, ptr @max_arr, align 4, !tbaa !6
  ret void

for.body:                                         ; preds = %for.body, %entry
  %i.010 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %maxx.09 = phi float [ %0, %entry ], [ %maxx.1, %for.body ]
  %arrayidx = getelementptr inbounds float, ptr %input, i32 %i.010
  %1 = load float, ptr %arrayidx, align 4, !tbaa !6
  %cmp1 = fcmp ogt float %1, %maxx.09
  %maxx.1 = select i1 %cmp1, float %1, float %maxx.09
  %inc = add nuw nsw i32 %i.010, 1
  %exitcond.not = icmp eq i32 %inc, 2048
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body, !llvm.loop !10
}

declare i32 @loop_begin(...) local_unnamed_addr #1

declare i32 @loop_end(...) local_unnamed_addr #1

attributes #0 = { nounwind uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="i686" "target-features"="+cx8,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="i686" "target-features"="+cx8,+x87" "tune-cpu"="generic" }

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
