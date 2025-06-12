               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %test2D "test2D"              ; id %7
               OpName %test2DRect "test2DRect"      ; id %11
               OpName %main "main"                  ; id %2

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %test2D RelaxedPrecision
               OpDecorate %test2D Binding 0
               OpDecorate %test2D DescriptorSet 0
               OpDecorate %test2DRect RelaxedPrecision
               OpDecorate %test2DRect Binding 1
               OpDecorate %test2DRect DescriptorSet 0
               OpDecorate %15 RelaxedPrecision
               OpDecorate %16 RelaxedPrecision
               OpDecorate %20 RelaxedPrecision
               OpDecorate %21 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
          %8 = OpTypeImage %float 2D 0 0 0 1 Unknown
          %9 = OpTypeSampledImage %8
%_ptr_UniformConstant_9 = OpTypePointer UniformConstant %9
     %test2D = OpVariable %_ptr_UniformConstant_9 UniformConstant   ; RelaxedPrecision, Binding 0, DescriptorSet 0
 %test2DRect = OpVariable %_ptr_UniformConstant_9 UniformConstant   ; RelaxedPrecision, Binding 1, DescriptorSet 0
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
  %float_0_5 = OpConstant %float 0.5
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0_5 %float_0_5
    %v3float = OpTypeVector %float 3
         %25 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5


               ; Function main
       %main = OpFunction %void None %13

         %14 = OpLabel
         %16 =   OpLoad %9 %test2D                  ; RelaxedPrecision
         %15 =   OpImageSampleImplicitLod %v4float %16 %19  ; RelaxedPrecision
                 OpStore %sk_FragColor %15
         %21 =   OpLoad %9 %test2DRect              ; RelaxedPrecision
         %20 =   OpImageSampleImplicitLod %v4float %21 %19  ; RelaxedPrecision
                 OpStore %sk_FragColor %20
         %23 =   OpLoad %9 %test2DRect              ; RelaxedPrecision
         %22 =   OpImageSampleProjImplicitLod %v4float %23 %25  ; RelaxedPrecision
                 OpStore %sk_FragColor %22
                 OpReturn
               OpFunctionEnd
