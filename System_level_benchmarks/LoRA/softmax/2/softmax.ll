; ModuleID = 'softmax.c'
source_filename = "softmax.c"
target datalayout = "e-m:e-p:32:32-p270:32:32-p271:32:32-p272:64:64-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

@.str = private unnamed_addr constant [30 x i8] c"___CGRA_HARDWARE_OP__myExp___\00", section "llvm.metadata"
@.str.1 = private unnamed_addr constant [10 x i8] c"softmax.c\00", section "llvm.metadata"
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
@sum_arr = dso_local local_unnamed_addr global float 0.000000e+00, align 4
@max_arr = dso_local local_unnamed_addr global float 0.000000e+00, align 4
@input = dso_local local_unnamed_addr global [2048 x float] zeroinitializer, align 4
@output = dso_local local_unnamed_addr global [2048 x float] zeroinitializer, align 4
@input_buf = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 4
@output_buf = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 4
@llvm.global.annotations = appending global [1 x { ptr, ptr, ptr, i32, ptr }] [{ ptr, ptr, ptr, i32, ptr } { ptr @myExp, ptr @.str, ptr @.str.1, i32 2, ptr null }], section "llvm.metadata"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local float @myExp(float noundef %exp) #0 {
entry:
  %exp.addr = alloca float, align 4
  store float %exp, ptr %exp.addr, align 4, !tbaa !6
  %call = call i32 @__hardware_imp()
  %0 = inttoptr i32 %call to ptr
  %1 = load float, ptr %0, align 4, !tbaa !6
  ret float %1
}

declare i32 @__hardware_imp(...) local_unnamed_addr #1

; Function Attrs: nounwind uwtable
define dso_local void @kernel() local_unnamed_addr #2 {
entry:
  %0 = load float, ptr @sum_arr, align 4, !tbaa !6
  %1 = load float, ptr @max_arr, align 4, !tbaa !6
  %call = tail call i32 @loop_begin() #3
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %call3 = tail call i32 @loop_end() #3
  store float %add, ptr @sum_arr, align 4, !tbaa !6
  ret void

for.body:                                         ; preds = %entry, %for.body
  %summ.010 = phi float [ %0, %entry ], [ %add, %for.body ]
  %i.09 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds [2048 x float], ptr @input, i32 0, i32 %i.09
  %2 = load float, ptr %arrayidx, align 4, !tbaa !6
  %sub = fsub float %2, %1
  %call1 = tail call float @myExp(float noundef %sub)
  %arrayidx2 = getelementptr inbounds [2048 x float], ptr @output, i32 0, i32 %i.09
  store float %call1, ptr %arrayidx2, align 4, !tbaa !6
  %add = fadd float %summ.010, %call1
  %inc = add nuw nsw i32 %i.09, 1
  %exitcond.not = icmp eq i32 %inc, 2048
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body, !llvm.loop !10
}

declare i32 @loop_begin(...) local_unnamed_addr #1

declare i32 @loop_end(...) local_unnamed_addr #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="i686" "target-features"="+cx8,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="i686" "target-features"="+cx8,+x87" "tune-cpu"="generic" }
attributes #2 = { nounwind uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="i686" "target-features"="+cx8,+x87" "tune-cpu"="generic" }
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
