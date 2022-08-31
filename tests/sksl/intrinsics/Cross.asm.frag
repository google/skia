OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "ah"
OpMemberName %_UniformBuffer 1 "bh"
OpMemberName %_UniformBuffer 2 "af"
OpMemberName %_UniformBuffer 3 "bf"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 8
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 16
OpMemberDecorate %_UniformBuffer 3 Offset 24
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %17 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%v2float = OpTypeVector %float 2
%_UniformBuffer = OpTypeStruct %v2float %v2float %v2float %v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Output_float = OpTypePointer Output %float
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%float_12 = OpConstant %float 12
%float_n8 = OpConstant %float -8
%v3float = OpTypeVector %float 3
%43 = OpConstantComposite %v3float %float_n8 %float_n8 %float_12
%float_9 = OpConstant %float 9
%float_n18 = OpConstant %float -18
%float_n9 = OpConstant %float -9
%49 = OpConstantComposite %v3float %float_9 %float_n18 %float_n9
%main = OpFunction %void None %15
%16 = OpLabel
%18 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%22 = OpLoad %v2float %18
%23 = OpAccessChain %_ptr_Uniform_v2float %10 %int_1
%25 = OpLoad %v2float %23
%27 = OpCompositeConstruct %mat2v2float %22 %25
%17 = OpExtInst %float %1 Determinant %27
%28 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %28 %17
%31 = OpAccessChain %_ptr_Uniform_v2float %10 %int_2
%33 = OpLoad %v2float %31
%34 = OpAccessChain %_ptr_Uniform_v2float %10 %int_3
%36 = OpLoad %v2float %34
%37 = OpCompositeConstruct %mat2v2float %33 %36
%30 = OpExtInst %float %1 Determinant %37
%38 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
OpStore %38 %30
%40 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_2
OpStore %40 %float_12
%44 = OpLoad %v4float %sk_FragColor
%45 = OpVectorShuffle %v4float %44 %43 4 5 6 3
OpStore %sk_FragColor %45
%50 = OpLoad %v4float %sk_FragColor
%51 = OpVectorShuffle %v4float %50 %49 0 4 5 6
OpStore %sk_FragColor %51
OpReturn
OpFunctionEnd
