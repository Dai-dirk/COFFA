; ModuleID = 'softmax_2_gvn.ll'
source_filename = "softmax_2.c"
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
@maxx = common dso_local local_unnamed_addr global [1 x float] zeroinitializer, align 4
@summ = common dso_local local_unnamed_addr global [1 x float] zeroinitializer, align 4
@input = common dso_local local_unnamed_addr global [2048 x float] zeroinitializer, align 4
@output = common dso_local local_unnamed_addr global [2048 x float] zeroinitializer, align 4
@input_buf = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 4
@output_buf = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 4

; Function Attrs: nounwind
define dso_local void @kernel(float* nocapture readonly %input, float* nocapture %output) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %add.1.lcssa = phi float [ %add.1, %for.body ]
  %0 = getelementptr inbounds [1 x float], [1 x float]* @summ, i32 0, i32 0
  store float %add.1.lcssa, float* %0, align 4, !tbaa !3
  ret void

for.body:                                         ; preds = %for.body, %entry
  %temp.014 = phi float [ 0.000000e+00, %entry ], [ %add.1, %for.body ]
  %i.013 = phi i32 [ 0, %entry ], [ %inc.1, %for.body ]
  %arrayidx = getelementptr inbounds float, float* %input, i32 %i.013
  %1 = load float, float* %arrayidx, align 4, !tbaa !3
  %2 = getelementptr inbounds [1 x float], [1 x float]* @maxx, i32 0, i32 0
  %3 = load float, float* %2, align 4, !tbaa !3
  %sub = fsub float %1, %3
  %mul.i = fmul float %sub, 0x3FF7154760000000
  %conv.i = fptosi float %mul.i to i32
  %conv1.i = sitofp i32 %conv.i to float
  %mul2.i = fmul float %conv1.i, 0x3FE62E4300000000
  %sub.i = fsub float %sub, %mul2.i
  %add.i = shl i32 %conv.i, 23
  %shl.i = add i32 %add.i, 1065353216
  %4 = bitcast i32 %shl.i to float
  %mul6.i = fmul float %sub.i, 0x3FA5555560000000
  %add7.i = fadd float %mul6.i, 0x3FC5555560000000
  %mul8.i = fmul float %sub.i, %add7.i
  %add9.i = fadd float %mul8.i, 5.000000e-01
  %mul10.i = fmul float %sub.i, %add9.i
  %add11.i = fadd float %mul10.i, 1.000000e+00
  %mul12.i = fmul float %sub.i, %add11.i
  %add13.i = fadd float %mul12.i, 1.000000e+00
  %mul14.i = fmul float %add13.i, %4
  %arrayidx2 = getelementptr inbounds float, float* %output, i32 %i.013
  store float %mul14.i, float* %arrayidx2, align 4, !tbaa !3
  %add = fadd float %temp.014, %mul14.i
  %inc = or i32 %i.013, 1
  %arrayidx.1 = getelementptr inbounds float, float* %input, i32 %inc
  %5 = load float, float* %arrayidx.1, align 4, !tbaa !3
  %6 = getelementptr inbounds [1 x float], [1 x float]* @maxx, i32 0, i32 0
  %7 = load float, float* %6, align 4, !tbaa !3
  %sub.1 = fsub float %5, %7
  %mul.i.1 = fmul float %sub.1, 0x3FF7154760000000
  %conv.i.1 = fptosi float %mul.i.1 to i32
  %conv1.i.1 = sitofp i32 %conv.i.1 to float
  %mul2.i.1 = fmul float %conv1.i.1, 0x3FE62E4300000000
  %sub.i.1 = fsub float %sub.1, %mul2.i.1
  %add.i.1 = shl i32 %conv.i.1, 23
  %shl.i.1 = add i32 %add.i.1, 1065353216
  %8 = bitcast i32 %shl.i.1 to float
  %mul6.i.1 = fmul float %sub.i.1, 0x3FA5555560000000
  %add7.i.1 = fadd float %mul6.i.1, 0x3FC5555560000000
  %mul8.i.1 = fmul float %sub.i.1, %add7.i.1
  %add9.i.1 = fadd float %mul8.i.1, 5.000000e-01
  %mul10.i.1 = fmul float %sub.i.1, %add9.i.1
  %add11.i.1 = fadd float %mul10.i.1, 1.000000e+00
  %mul12.i.1 = fmul float %sub.i.1, %add11.i.1
  %add13.i.1 = fadd float %mul12.i.1, 1.000000e+00
  %mul14.i.1 = fmul float %add13.i.1, %8
  %arrayidx2.1 = getelementptr inbounds float, float* %output, i32 %inc
  store float %mul14.i.1, float* %arrayidx2.1, align 4, !tbaa !3
  %add.1 = fadd float %add, %mul14.i.1
  %inc.1 = add nuw nsw i32 %i.013, 2
  %exitcond.1 = icmp eq i32 %inc.1, 2048
  br i1 %exitcond.1, label %for.cond.cleanup, label %for.body, !llvm.loop !7
}

declare dso_local i32 @loop_begin(...) local_unnamed_addr #1

declare dso_local i32 @loop_end(...) local_unnamed_addr #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

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
