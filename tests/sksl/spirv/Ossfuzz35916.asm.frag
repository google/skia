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
               OpDecorate %19 RelaxedPrecision
               OpDecorate %21 RelaxedPrecision
               OpMemberDecorate %sksl_synthetic_uniforms 0 Offset 16384
               OpDecorate %sksl_synthetic_uniforms Block
               OpDecorate %22 Binding 0
               OpDecorate %22 DescriptorSet 0
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
%_ptr_Output_float = OpTypePointer Output %float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
    %v2float = OpTypeVector %float 2
%sksl_synthetic_uniforms = OpTypeStruct %v2float
%_ptr_Uniform_sksl_synthetic_uniforms = OpTypePointer Uniform %sksl_synthetic_uniforms
         %22 = OpVariable %_ptr_Uniform_sksl_synthetic_uniforms Uniform
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
       %main = OpFunction %void None %13
         %14 = OpLabel
         %15 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
         %19 = OpLoad %float %15
         %21 = OpFSub %float %19 %float_1
               OpStore %15 %21
         %26 = OpAccessChain %_ptr_Uniform_v2float %22 %int_0
         %28 = OpLoad %v2float %26
         %29 = OpCompositeExtract %float %28 0
         %30 = OpAccessChain %_ptr_Uniform_v2float %22 %int_0
         %31 = OpLoad %v2float %30
         %32 = OpCompositeExtract %float %31 1
         %33 = OpLoad %v4float %sk_FragCoord
         %34 = OpCompositeExtract %float %33 0
         %35 = OpLoad %v4float %sk_FragCoord
         %36 = OpCompositeExtract %float %35 1
         %37 = OpLoad %v4float %sk_FragCoord
         %38 = OpVectorShuffle %v2float %37 %37 2 3
         %39 = OpFMul %float %32 %36
         %40 = OpFAdd %float %29 %39
         %41 = OpCompositeConstruct %v4float %34 %40 %38
         %42 = OpCompositeConstruct %v4float %19 %19 %19 %19
         %43 = OpFAdd %v4float %42 %41
               OpReturn
               OpFunctionEnd
