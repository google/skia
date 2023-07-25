               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %tex "tex"
               OpName %main "main"
               OpName %a "a"
               OpName %b "b"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %tex RelaxedPrecision
               OpDecorate %tex Binding 0
               OpDecorate %tex DescriptorSet 0
               OpDecorate %20 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
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
        %tex = OpVariable %_ptr_UniformConstant_12 UniformConstant
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %23 = OpConstantComposite %v2float %float_0 %float_0
    %v3float = OpTypeVector %float 3
         %28 = OpConstantComposite %v3float %float_0 %float_0 %float_0
       %main = OpFunction %void None %15
         %16 = OpLabel
          %a = OpVariable %_ptr_Function_v4float Function
          %b = OpVariable %_ptr_Function_v4float Function
         %20 = OpLoad %12 %tex
         %19 = OpImageSampleImplicitLod %v4float %20 %23
               OpStore %a %19
         %26 = OpLoad %12 %tex
         %25 = OpImageSampleProjImplicitLod %v4float %26 %28
               OpStore %b %25
         %29 = OpVectorShuffle %v2float %19 %19 0 1
         %30 = OpCompositeExtract %float %29 0
         %31 = OpCompositeExtract %float %29 1
         %32 = OpVectorShuffle %v2float %25 %25 2 3
         %33 = OpCompositeExtract %float %32 0
         %34 = OpCompositeExtract %float %32 1
         %35 = OpCompositeConstruct %v4float %30 %31 %33 %34
               OpStore %sk_FragColor %35
               OpReturn
               OpFunctionEnd
