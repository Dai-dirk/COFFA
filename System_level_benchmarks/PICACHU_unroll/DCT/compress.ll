; ModuleID = 'compress.c'
source_filename = "compress.c"
target datalayout = "e-m:e-p:32:32-p270:32:32-p271:32:32-p272:64:64-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

@cos1 = dso_local local_unnamed_addr global [8 x [8 x float]] zeroinitializer, align 4
@cos2 = dso_local local_unnamed_addr global [8 x [8 x float]] zeroinitializer, align 4

; Function Attrs: nounwind uwtable
define dso_local void @kernel() local_unnamed_addr #0 {
entry:
  %call = tail call i32 @loop_begin() #3
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.end
  %factor2.075 = phi float [ 0.000000e+00, %entry ], [ %add, %for.end ]
  %m.074 = phi i32 [ 0, %entry ], [ %inc49, %for.end ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %n.073 = phi i32 [ 1, %for.cond1.preheader ], [ %inc, %for.body3 ]
  %cmp4 = icmp eq i32 %n.073, 1
  %mul = shl nuw nsw i32 %n.073, 1
  %sub = add nsw i32 %mul, -1
  %conv = sitofp i32 %sub to float
  %mul5 = select i1 %cmp4, float 1.000000e+00, float %conv
  %cond = fmul float %factor2.075, %mul5
  %mul7 = fmul float %cond, 0x3FC45F3060000000
  %conv8 = fptosi float %mul7 to i32
  %conv9 = sitofp i32 %conv8 to float
  %neg = fneg float %conv9
  %0 = tail call float @llvm.fmuladd.f32(float %neg, float 0x401921FB60000000, float %cond)
  %cmp11 = fcmp ogt float %0, 0x400921FB60000000
  %cmp15 = fcmp olt float %0, 0xC00921FB60000000
  %cond17 = select i1 %cmp15, float 0x401921FB60000000, float 0.000000e+00
  %cond19 = select i1 %cmp11, float 0xC01921FB60000000, float %cond17
  %cmp20 = fcmp ogt float %cond19, 0x3FF921FB60000000
  %sub27 = fsub float 0x400921FB60000000, %cond19
  %cond34 = select i1 %cmp11, float 0x400921FB60000000, float %cond17
  %cond36 = select i1 %cmp20, float %sub27, float %cond34
  %1 = or i1 %cmp11, %cmp20
  %mul.i = fmul float %cond36, %cond36
  %2 = tail call float @llvm.fmuladd.f32(float %mul.i, float 0x3FA5555560000000, float -5.000000e-01)
  %3 = tail call float @llvm.fmuladd.f32(float %mul.i, float %2, float 1.000000e+00)
  %4 = fneg float %3
  %5 = select i1 %1, float %4, float %3
  %mul42 = fmul float %5, 1.250000e-01
  %sub43 = add nsw i32 %conv8, -1
  %arrayidx44 = getelementptr inbounds [8 x [8 x float]], ptr @cos1, i32 0, i32 %m.074, i32 %sub43
  store float %mul42, ptr %arrayidx44, align 4, !tbaa !6
  %arrayidx47 = getelementptr inbounds [8 x [8 x float]], ptr @cos2, i32 0, i32 %sub43, i32 %m.074
  store float %mul42, ptr %arrayidx47, align 4, !tbaa !6
  %inc = add nuw nsw i32 %n.073, 1
  %exitcond.not = icmp eq i32 %inc, 9
  br i1 %exitcond.not, label %for.end, label %for.body3, !llvm.loop !10

for.end:                                          ; preds = %for.body3
  %add = fadd float %factor2.075, 0x3FC921FB60000000
  %inc49 = add nuw nsw i32 %m.074, 1
  %exitcond76.not = icmp eq i32 %inc49, 8
  br i1 %exitcond76.not, label %for.end50, label %for.cond1.preheader, !llvm.loop !13

for.end50:                                        ; preds = %for.end
  %call51 = tail call i32 @loop_end() #3
  ret void
}

declare i32 @loop_begin(...) local_unnamed_addr #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind readnone speculatable willreturn
declare float @llvm.fmuladd.f32(float, float, float) #2

declare i32 @loop_end(...) local_unnamed_addr #1

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
!13 = distinct !{!13, !11, !12}
