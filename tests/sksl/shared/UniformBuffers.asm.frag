OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %testBlock "testBlock"
OpMemberName %testBlock 0 "myHalf"
OpMemberName %testBlock 1 "myHalf4"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpMemberDecorate %testBlock 0 Offset 0
OpMemberDecorate %testBlock 0 RelaxedPrecision
OpMemberDecorate %testBlock 1 Offset 16
OpMemberDecorate %testBlock 1 RelaxedPrecision
OpDecorate %testBlock Block
OpDecorate %3 Binding 0
OpDecorate %3 DescriptorSet 0
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %24 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%testBlock = OpTypeStruct %float %v4float
%_ptr_Uniform_testBlock = OpTypePointer Uniform %testBlock
%3 = OpVariable %_ptr_Uniform_testBlock Uniform
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%22 = OpAccessChain %_ptr_Uniform_v4float %3 %int_1
%24 = OpLoad %v4float %22
%26 = OpAccessChain %_ptr_Uniform_float %3 %int_0
%28 = OpLoad %float %26
%29 = OpVectorTimesScalar %v4float %24 %28
OpReturnValue %29
OpFunctionEnd
