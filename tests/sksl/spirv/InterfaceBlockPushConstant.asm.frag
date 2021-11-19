OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %testBlock "testBlock"
OpMemberName %testBlock 0 "m1"
OpMemberName %testBlock 1 "m2"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpMemberDecorate %testBlock 0 Offset 16
OpMemberDecorate %testBlock 0 ColMajor
OpMemberDecorate %testBlock 0 MatrixStride 8
OpMemberDecorate %testBlock 0 RelaxedPrecision
OpMemberDecorate %testBlock 1 Offset 32
OpMemberDecorate %testBlock 1 ColMajor
OpMemberDecorate %testBlock 1 MatrixStride 8
OpMemberDecorate %testBlock 1 RelaxedPrecision
OpDecorate %testBlock Block
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%testBlock = OpTypeStruct %mat2v2float %mat2v2float
%_ptr_PushConstant_testBlock = OpTypePointer PushConstant %testBlock
%3 = OpVariable %_ptr_PushConstant_testBlock PushConstant
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%16 = OpTypeFunction %void
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_PushConstant_v2float = OpTypePointer PushConstant %v2float
%int_1 = OpConstant %int 1
%main = OpFunction %void None %16
%17 = OpLabel
%20 = OpAccessChain %_ptr_PushConstant_v2float %3 %int_0 %int_0
%22 = OpLoad %v2float %20
%23 = OpCompositeExtract %float %22 0
%25 = OpAccessChain %_ptr_PushConstant_v2float %3 %int_0 %int_1
%26 = OpLoad %v2float %25
%27 = OpCompositeExtract %float %26 1
%28 = OpAccessChain %_ptr_PushConstant_v2float %3 %int_1 %int_0
%29 = OpLoad %v2float %28
%30 = OpCompositeExtract %float %29 0
%31 = OpAccessChain %_ptr_PushConstant_v2float %3 %int_1 %int_1
%32 = OpLoad %v2float %31
%33 = OpCompositeExtract %float %32 1
%34 = OpCompositeConstruct %v4float %23 %27 %30 %33
OpStore %sk_FragColor %34
OpReturn
OpFunctionEnd
