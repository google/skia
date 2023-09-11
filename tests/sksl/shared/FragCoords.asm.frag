               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor %sk_FragCoord
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %sk_FragCoord "sk_FragCoord"
               OpName %main "main"
               OpName %sksl_synthetic_uniforms "sksl_synthetic_uniforms"
               OpMemberName %sksl_synthetic_uniforms 0 "u_skRTFlip"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %sk_FragCoord BuiltIn FragCoord
               OpMemberDecorate %sksl_synthetic_uniforms 0 Offset 16384
               OpDecorate %sksl_synthetic_uniforms Block
               OpDecorate %15 Binding 0
               OpDecorate %15 DescriptorSet 0
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
    %v2float = OpTypeVector %float 2
%sksl_synthetic_uniforms = OpTypeStruct %v2float
%_ptr_Uniform_sksl_synthetic_uniforms = OpTypePointer Uniform %sksl_synthetic_uniforms
         %15 = OpVariable %_ptr_Uniform_sksl_synthetic_uniforms Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
       %main = OpFunction %void None %13
         %14 = OpLabel
         %21 = OpAccessChain %_ptr_Uniform_v2float %15 %int_0
         %23 = OpLoad %v2float %21
         %24 = OpCompositeExtract %float %23 0
         %25 = OpAccessChain %_ptr_Uniform_v2float %15 %int_0
         %26 = OpLoad %v2float %25
         %27 = OpCompositeExtract %float %26 1
         %28 = OpLoad %v4float %sk_FragCoord
         %29 = OpCompositeExtract %float %28 0
         %30 = OpLoad %v4float %sk_FragCoord
         %31 = OpCompositeExtract %float %30 1
         %32 = OpLoad %v4float %sk_FragCoord
         %33 = OpVectorShuffle %v2float %32 %32 2 3
         %34 = OpFMul %float %27 %31
         %35 = OpFAdd %float %24 %34
         %36 = OpCompositeConstruct %v4float %29 %35 %33
         %37 = OpVectorShuffle %v2float %36 %36 1 0
         %38 = OpLoad %v4float %sk_FragColor
         %39 = OpVectorShuffle %v4float %38 %37 4 5 2 3
               OpStore %sk_FragColor %39
               OpReturn
               OpFunctionEnd
