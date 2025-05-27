               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %tex "tex"
               OpName %main "main"
               OpName %a "a"
               OpName %b "b"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %tex RelaxedPrecision
               OpDecorate %tex Binding 0
               OpDecorate %tex DescriptorSet 0
               OpDecorate %17 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
          %8 = OpTypeImage %float 2D 0 0 0 1 Unknown
          %9 = OpTypeSampledImage %8
%_ptr_UniformConstant_9 = OpTypePointer UniformConstant %9
        %tex = OpVariable %_ptr_UniformConstant_9 UniformConstant
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
    %v3float = OpTypeVector %float 3
         %25 = OpConstantComposite %v3float %float_0 %float_0 %float_0
       %main = OpFunction %void None %12
         %13 = OpLabel
          %a = OpVariable %_ptr_Function_v4float Function
          %b = OpVariable %_ptr_Function_v4float Function
         %17 = OpLoad %9 %tex
         %16 = OpImageSampleImplicitLod %v4float %17 %20
               OpStore %a %16
         %23 = OpLoad %9 %tex
         %22 = OpImageSampleProjImplicitLod %v4float %23 %25
               OpStore %b %22
         %26 = OpVectorShuffle %v2float %16 %16 0 1
         %27 = OpCompositeExtract %float %26 0
         %28 = OpCompositeExtract %float %26 1
         %29 = OpVectorShuffle %v2float %22 %22 2 3
         %30 = OpCompositeExtract %float %29 0
         %31 = OpCompositeExtract %float %29 1
         %32 = OpCompositeConstruct %v4float %27 %28 %30 %31
               OpStore %sk_FragColor %32
               OpReturn
               OpFunctionEnd
