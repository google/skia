               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor %sk_FragCoord
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %sk_FragCoord "sk_FragCoord"
               OpName %_entrypoint_v "_entrypoint_v"
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
               OpDecorate %25 Binding 0
               OpDecorate %25 DescriptorSet 0
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
         %14 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %18 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %22 = OpTypeFunction %v4float %_ptr_Function_v2float
%sksl_synthetic_uniforms = OpTypeStruct %v2float
%_ptr_Uniform_sksl_synthetic_uniforms = OpTypePointer Uniform %sksl_synthetic_uniforms
         %25 = OpVariable %_ptr_Uniform_sksl_synthetic_uniforms Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
    %float_1 = OpConstant %float 1
%_entrypoint_v = OpFunction %void None %14
         %15 = OpLabel
         %19 = OpVariable %_ptr_Function_v2float Function
               OpStore %19 %18
         %21 = OpFunctionCall %v4float %main %19
               OpStore %sk_FragColor %21
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %22
         %23 = OpFunctionParameter %_ptr_Function_v2float
         %24 = OpLabel
         %30 = OpAccessChain %_ptr_Uniform_v2float %25 %int_0
         %32 = OpLoad %v2float %30
         %33 = OpCompositeExtract %float %32 0
         %34 = OpAccessChain %_ptr_Uniform_v2float %25 %int_0
         %35 = OpLoad %v2float %34
         %36 = OpCompositeExtract %float %35 1
         %37 = OpLoad %v4float %sk_FragCoord
         %38 = OpCompositeExtract %float %37 0
         %39 = OpLoad %v4float %sk_FragCoord
         %40 = OpCompositeExtract %float %39 1
         %41 = OpLoad %v4float %sk_FragCoord
         %42 = OpVectorShuffle %v2float %41 %41 2 3
         %43 = OpFMul %float %36 %40
         %44 = OpFAdd %float %33 %43
         %45 = OpCompositeConstruct %v4float %38 %44 %42
         %46 = OpVectorShuffle %v2float %45 %45 1 0
         %47 = OpCompositeExtract %float %46 0
         %48 = OpCompositeExtract %float %46 1
         %50 = OpCompositeConstruct %v4float %47 %48 %float_1 %float_1
               OpReturnValue %50
               OpFunctionEnd
