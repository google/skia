               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %s "s"
               OpName %main "main"
               OpName %a "a"
               OpName %b "b"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %s RelaxedPrecision
               OpDecorate %s Binding 0
               OpDecorate %s DescriptorSet 0
               OpDecorate %17 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
          %8 = OpTypeImage %float 2D 0 0 0 1 Unknown
          %9 = OpTypeSampledImage %8
%_ptr_UniformConstant_9 = OpTypePointer UniformConstant %9
          %s = OpVariable %_ptr_UniformConstant_9 UniformConstant
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%float_n0_474999994 = OpConstant %float -0.474999994
    %v3float = OpTypeVector %float 3
         %26 = OpConstantComposite %v3float %float_0 %float_0 %float_0
       %main = OpFunction %void None %12
         %13 = OpLabel
          %a = OpVariable %_ptr_Function_v4float Function
          %b = OpVariable %_ptr_Function_v4float Function
         %17 = OpLoad %9 %s
         %16 = OpImageSampleImplicitLod %v4float %17 %20 Bias %float_n0_474999994
               OpStore %a %16
         %24 = OpLoad %9 %s
         %23 = OpImageSampleProjImplicitLod %v4float %24 %26 Bias %float_n0_474999994
               OpStore %b %23
         %27 = OpVectorShuffle %v2float %16 %16 0 1
         %28 = OpCompositeExtract %float %27 0
         %29 = OpCompositeExtract %float %27 1
         %30 = OpVectorShuffle %v2float %23 %23 0 1
         %31 = OpCompositeExtract %float %30 0
         %32 = OpCompositeExtract %float %30 1
         %33 = OpCompositeConstruct %v4float %28 %29 %31 %32
               OpStore %sk_FragColor %33
               OpReturn
               OpFunctionEnd
