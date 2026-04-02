; ModuleID = 'mish_3order_gvn.ll'
source_filename = "mish_3order.c"
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
@beta2 = dso_local local_unnamed_addr constant float 1.000000e+00, align 4
@const1 = dso_local local_unnamed_addr constant float 1.000000e+00, align 4
@const2 = dso_local local_unnamed_addr constant float 1.000000e+00, align 4
@input = common dso_local global [2048 x float] zeroinitializer, align 4
@output = common dso_local local_unnamed_addr global [2048 x float] zeroinitializer, align 4

; Function Attrs: nounwind
define dso_local void @kernel() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %i.035 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds [2048 x float], [2048 x float]* @input, i32 0, i32 %i.035
  %0 = load volatile float, float* %arrayidx, align 4, !tbaa !3
  %mul.i = fmul float %0, 0x3FF7154760000000
  %conv.i = fptosi float %mul.i to i32
  %conv1.i = sitofp i32 %conv.i to float
  %mul2.i = fmul float %conv1.i, 0x3FE62E4300000000
  %sub.i = fsub float %0, %mul2.i
  %add.i = shl i32 %conv.i, 23
  %shl.i = add i32 %add.i, 1065353216
  %1 = bitcast i32 %shl.i to float
  %mul6.i = fmul float %sub.i, 5.000000e-01
  %add7.i = fadd float %mul6.i, 1.000000e+00
  %mul8.i = fmul float %sub.i, %add7.i
  %add9.i = fadd float %mul8.i, 1.000000e+00
  %mul10.i = fmul float %add9.i, %1
  %add = fadd float %mul10.i, 1.000000e+00
  %2 = bitcast float %add to i32
  %3 = lshr i32 %2, 23
  %and.i = and i32 %3, 255
  %sub.i32 = add nsw i32 %and.i, -127
  %and1.i = and i32 %2, 8388607
  %or.i = or i32 %and1.i, 1065353216
  %4 = bitcast i32 %or.i to float
  %sub2.i = fadd float %4, -1.000000e+00
  %mul.i33 = fmul float %sub2.i, 5.000000e-01
  %5 = fsub float 1.000000e+00, %mul.i33
  %mul3.i = fmul float %sub2.i, %5
  %conv.i34 = sitofp i32 %sub.i32 to float
  %mul4.i = fmul float %mul3.i, 0x3FF7154760000000
  %add5.i = fadd float %mul4.i, %conv.i34
  %mul.i20 = fmul float %add5.i, 0x3FF7154760000000
  %conv.i21 = fptosi float %mul.i20 to i32
  %conv1.i22 = sitofp i32 %conv.i21 to float
  %mul2.i23 = fmul float %conv1.i22, 0x3FE62E4300000000
  %sub.i24 = fsub float %add5.i, %mul2.i23
  %add.i25 = shl i32 %conv.i21, 23
  %shl.i26 = add i32 %add.i25, 1065353216
  %6 = bitcast i32 %shl.i26 to float
  %mul6.i27 = fmul float %sub.i24, 5.000000e-01
  %add7.i28 = fadd float %mul6.i27, 1.000000e+00
  %mul8.i29 = fmul float %sub.i24, %add7.i28
  %add9.i30 = fadd float %mul8.i29, 1.000000e+00
  %mul10.i31 = fmul float %add9.i30, %6
  %7 = load volatile float, float* %arrayidx, align 4, !tbaa !3
  %sub = fsub float 1.000000e+00, %mul10.i31
  %mul6 = fmul float %7, %sub
  %add7 = fadd float %mul10.i31, 1.000000e+00
  %div = fdiv float %mul6, %add7
  %arrayidx8 = getelementptr inbounds [2048 x float], [2048 x float]* @output, i32 0, i32 %i.035
  store float %div, float* %arrayidx8, align 4, !tbaa !3
  %inc = add nuw nsw i32 %i.035, 1
  %exitcond = icmp eq i32 %inc, 2048
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
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
