               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %s "s"
               OpName %main "main"
               OpName %a "a"
               OpName %b "b"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %s RelaxedPrecision
               OpDecorate %s Binding 0
               OpDecorate %s DescriptorSet 0
               OpDecorate %20 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
         %11 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %12 = OpTypeSampledImage %11
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12
          %s = OpVariable %_ptr_UniformConstant_12 UniformConstant
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %23 = OpConstantComposite %v2float %float_0 %float_0
%float_n0_474999994 = OpConstant %float -0.474999994
    %v3float = OpTypeVector %float 3
         %29 = OpConstantComposite %v3float %float_0 %float_0 %float_0
       %main = OpFunction %void None %15
         %16 = OpLabel
          %a = OpVariable %_ptr_Function_v4float Function
          %b = OpVariable %_ptr_Function_v4float Function
         %20 = OpLoad %12 %s
         %19 = OpImageSampleImplicitLod %v4float %20 %23 Bias %float_n0_474999994
               OpStore %a %19
         %27 = OpLoad %12 %s
         %26 = OpImageSampleProjImplicitLod %v4float %27 %29 Bias %float_n0_474999994
               OpStore %b %26
         %30 = OpVectorShuffle %v2float %19 %19 0 1
         %31 = OpCompositeExtract %float %30 0
         %32 = OpCompositeExtract %float %30 1
         %33 = OpVectorShuffle %v2float %26 %26 0 1
         %34 = OpCompositeExtract %float %33 0
         %35 = OpCompositeExtract %float %33 1
         %36 = OpCompositeConstruct %v4float %31 %32 %34 %35
               OpStore %sk_FragColor %36
               OpReturn
               OpFunctionEnd
