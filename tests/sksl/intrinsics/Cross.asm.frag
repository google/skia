OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "a"
OpMemberName %_UniformBuffer 1 "b"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 8
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %21 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%_UniformBuffer = OpTypeStruct %v2float %v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%main = OpFunction %void None %15
%16 = OpLabel
%17 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%21 = OpLoad %v2float %17
%22 = OpCompositeExtract %float %21 0
%23 = OpAccessChain %_ptr_Uniform_v2float %10 %int_1
%25 = OpLoad %v2float %23
%26 = OpCompositeExtract %float %25 1
%27 = OpFMul %float %22 %26
%28 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%29 = OpLoad %v2float %28
%30 = OpCompositeExtract %float %29 1
%31 = OpAccessChain %_ptr_Uniform_v2float %10 %int_1
%32 = OpLoad %v2float %31
%33 = OpCompositeExtract %float %32 0
%34 = OpFMul %float %30 %33
%35 = OpFSub %float %27 %34
%36 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %36 %35
OpReturn
OpFunctionEnd
