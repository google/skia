               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor %sk_FragCoord
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %sk_FragCoord "sk_FragCoord"
               OpName %main "main"
               OpName %sksl_synthetic_uniforms "sksl_synthetic_uniforms"
               OpMemberName %sksl_synthetic_uniforms 0 "u_skRTFlip"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %sk_FragCoord BuiltIn FragCoord
               OpDecorate %16 RelaxedPrecision
               OpDecorate %18 RelaxedPrecision
               OpMemberDecorate %sksl_synthetic_uniforms 0 Offset 16384
               OpDecorate %sksl_synthetic_uniforms Block
               OpDecorate %19 Binding 0
               OpDecorate %19 DescriptorSet 0
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
       %void = OpTypeVoid
         %10 = OpTypeFunction %void
%_ptr_Output_float = OpTypePointer Output %float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
    %v2float = OpTypeVector %float 2
%sksl_synthetic_uniforms = OpTypeStruct %v2float
%_ptr_Uniform_sksl_synthetic_uniforms = OpTypePointer Uniform %sksl_synthetic_uniforms
         %19 = OpVariable %_ptr_Uniform_sksl_synthetic_uniforms Uniform
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
       %main = OpFunction %void None %10
         %11 = OpLabel
         %12 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
         %16 = OpLoad %float %12
         %18 = OpFSub %float %16 %float_1
               OpStore %12 %18
         %23 = OpAccessChain %_ptr_Uniform_v2float %19 %int_0
         %25 = OpLoad %v2float %23
         %26 = OpCompositeExtract %float %25 0
         %27 = OpAccessChain %_ptr_Uniform_v2float %19 %int_0
         %28 = OpLoad %v2float %27
         %29 = OpCompositeExtract %float %28 1
         %30 = OpLoad %v4float %sk_FragCoord
         %31 = OpCompositeExtract %float %30 0
         %32 = OpLoad %v4float %sk_FragCoord
         %33 = OpCompositeExtract %float %32 1
         %34 = OpLoad %v4float %sk_FragCoord
         %35 = OpVectorShuffle %v2float %34 %34 2 3
         %36 = OpFMul %float %29 %33
         %37 = OpFAdd %float %26 %36
         %38 = OpCompositeConstruct %v4float %31 %37 %35
         %39 = OpCompositeConstruct %v4float %16 %16 %16 %16
         %40 = OpFAdd %v4float %39 %38
               OpReturn
               OpFunctionEnd
