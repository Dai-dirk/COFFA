; ModuleID = 'compress_1_gvn.ll'
source_filename = "compress_1.c"
target datalayout = "e-m:e-p:32:32-p270:32:32-p271:32:32-p272:64:64-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

@cos1 = common dso_local local_unnamed_addr global [32 x [32 x float]] zeroinitializer, align 4
@cos2 = common dso_local local_unnamed_addr global [32 x [32 x float]] zeroinitializer, align 4

; Function Attrs: nounwind
define dso_local void @kernel() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.end, %entry
  %factor2.094 = phi float [ 0.000000e+00, %entry ], [ %add52, %for.end ]
  %m.093 = phi i32 [ 0, %entry ], [ %inc54, %for.end ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %n.092 = phi i32 [ 1, %for.cond1.preheader ], [ %inc.1, %for.body3 ]
  %cmp4 = icmp eq i32 %n.092, 1
  %mul = shl nuw nsw i32 %n.092, 1
  %sub = add nsw i32 %mul, -1
  %conv = sitofp i32 %sub to float
  %mul5 = fmul float %factor2.094, %conv
  %cond = select i1 %cmp4, float %factor2.094, float %mul5
  %mul6 = fmul float %cond, 0x3FC45F3060000000
  %conv7 = fptosi float %mul6 to i32
  %conv8 = sitofp i32 %conv7 to float
  %mul9 = fmul float %conv8, 0x401921FB60000000
  %sub10 = fsub float %cond, %mul9
  %cmp11 = fcmp ogt float %sub10, 0x400921FB60000000
  %sub14 = fadd float %sub10, 0xC01921FB60000000
  %cmp16 = fcmp olt float %sub10, 0xC00921FB60000000
  %add = fadd float %sub10, 0x401921FB60000000
  %cond21 = select i1 %cmp16, float %add, float %sub10
  %cond23 = select i1 %cmp11, float %sub14, float %cond21
  %cmp24 = fcmp ogt float %cond23, 0x3FF921FB60000000
  %cmp27 = fcmp olt float %cond23, 0xBFF921FB60000000
  %sub31 = fsub float 0x400921FB60000000, %cond23
  %sub35 = fsub float 0xC00921FB60000000, %cond23
  %cond38 = select i1 %cmp27, float %sub35, float %cond23
  %cond40 = select i1 %cmp24, float %sub31, float %cond38
  %0 = or i1 %cmp24, %cmp27
  %mul.i = fmul float %cond40, %cond40
  %mul1.i = fmul float %mul.i, 0x3FA5555560000000
  %add.i = fadd float %mul1.i, -5.000000e-01
  %mul2.i = fmul float %mul.i, %add.i
  %add3.i = fadd float %mul2.i, 1.000000e+00
  %1 = fneg float %add3.i
  %2 = select i1 %0, float %1, float %add3.i
  %mul46 = fmul float %2, 1.250000e-01
  %sub47 = add nsw i32 %n.092, -1
  %arrayidx48 = getelementptr inbounds [32 x [32 x float]], [32 x [32 x float]]* @cos1, i32 0, i32 %m.093, i32 %sub47
  store float %mul46, float* %arrayidx48, align 4, !tbaa !3
  %arrayidx51 = getelementptr inbounds [32 x [32 x float]], [32 x [32 x float]]* @cos2, i32 0, i32 %sub47, i32 %m.093
  store float %mul46, float* %arrayidx51, align 4, !tbaa !3
  %inc = shl nuw i32 %n.092, 1
  %sub.1 = or i32 %inc, 1
  %conv.1 = sitofp i32 %sub.1 to float
  %mul5.1 = fmul float %factor2.094, %conv.1
  %mul6.1 = fmul float %mul5.1, 0x3FC45F3060000000
  %conv7.1 = fptosi float %mul6.1 to i32
  %conv8.1 = sitofp i32 %conv7.1 to float
  %mul9.1 = fmul float %conv8.1, 0x401921FB60000000
  %sub10.1 = fsub float %mul5.1, %mul9.1
  %cmp11.1 = fcmp ogt float %sub10.1, 0x400921FB60000000
  %sub14.1 = fadd float %sub10.1, 0xC01921FB60000000
  %cmp16.1 = fcmp olt float %sub10.1, 0xC00921FB60000000
  %add.1 = fadd float %sub10.1, 0x401921FB60000000
  %cond21.1 = select i1 %cmp16.1, float %add.1, float %sub10.1
  %cond23.1 = select i1 %cmp11.1, float %sub14.1, float %cond21.1
  %cmp24.1 = fcmp ogt float %cond23.1, 0x3FF921FB60000000
  %cmp27.1 = fcmp olt float %cond23.1, 0xBFF921FB60000000
  %sub31.1 = fsub float 0x400921FB60000000, %cond23.1
  %sub35.1 = fsub float 0xC00921FB60000000, %cond23.1
  %cond38.1 = select i1 %cmp27.1, float %sub35.1, float %cond23.1
  %cond40.1 = select i1 %cmp24.1, float %sub31.1, float %cond38.1
  %3 = or i1 %cmp24.1, %cmp27.1
  %mul.i.1 = fmul float %cond40.1, %cond40.1
  %mul1.i.1 = fmul float %mul.i.1, 0x3FA5555560000000
  %add.i.1 = fadd float %mul1.i.1, -5.000000e-01
  %mul2.i.1 = fmul float %mul.i.1, %add.i.1
  %add3.i.1 = fadd float %mul2.i.1, 1.000000e+00
  %4 = fneg float %add3.i.1
  %5 = select i1 %3, float %4, float %add3.i.1
  %mul46.1 = fmul float %5, 1.250000e-01
  %arrayidx48.1 = getelementptr inbounds [32 x [32 x float]], [32 x [32 x float]]* @cos1, i32 0, i32 %m.093, i32 %n.092
  store float %mul46.1, float* %arrayidx48.1, align 4, !tbaa !3
  %arrayidx51.1 = getelementptr inbounds [32 x [32 x float]], [32 x [32 x float]]* @cos2, i32 0, i32 %n.092, i32 %m.093
  store float %mul46.1, float* %arrayidx51.1, align 4, !tbaa !3
  %inc.1 = add nuw nsw i32 %n.092, 2
  %exitcond.1 = icmp eq i32 %inc.1, 33
  br i1 %exitcond.1, label %for.end, label %for.body3, !llvm.loop !7

for.end:                                          ; preds = %for.body3
  %add52 = fadd float %factor2.094, 0x3FC921FB60000000
  %inc54 = add nuw nsw i32 %m.093, 1
  %exitcond = icmp eq i32 %inc54, 32
  br i1 %exitcond, label %for.end55, label %for.cond1.preheader

for.end55:                                        ; preds = %for.end
  ret void
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
