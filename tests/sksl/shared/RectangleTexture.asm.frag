               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %test2D "test2D"              ; id %11
               OpName %test2DRect "test2DRect"      ; id %15
               OpName %main "main"                  ; id %6

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
               OpDecorate %19 RelaxedPrecision
               OpDecorate %20 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
         %12 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %13 = OpTypeSampledImage %12
%_ptr_UniformConstant_13 = OpTypePointer UniformConstant %13
     %test2D = OpVariable %_ptr_UniformConstant_13 UniformConstant  ; RelaxedPrecision, Binding 0, DescriptorSet 0
 %test2DRect = OpVariable %_ptr_UniformConstant_13 UniformConstant  ; RelaxedPrecision, Binding 1, DescriptorSet 0
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
  %float_0_5 = OpConstant %float 0.5
    %v2float = OpTypeVector %float 2
         %23 = OpConstantComposite %v2float %float_0_5 %float_0_5
    %v3float = OpTypeVector %float 3
         %29 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5


               ; Function main
       %main = OpFunction %void None %17

         %18 = OpLabel
         %20 =   OpLoad %13 %test2D                 ; RelaxedPrecision
         %19 =   OpImageSampleImplicitLod %v4float %20 %23  ; RelaxedPrecision
                 OpStore %sk_FragColor %19
         %25 =   OpLoad %13 %test2DRect             ; RelaxedPrecision
         %24 =   OpImageSampleImplicitLod %v4float %25 %23  ; RelaxedPrecision
                 OpStore %sk_FragColor %24
         %27 =   OpLoad %13 %test2DRect             ; RelaxedPrecision
         %26 =   OpImageSampleProjImplicitLod %v4float %27 %29  ; RelaxedPrecision
                 OpStore %sk_FragColor %26
                 OpReturn
               OpFunctionEnd
