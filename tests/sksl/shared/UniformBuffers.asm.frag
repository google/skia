OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %testBlock "testBlock"
OpMemberName %testBlock 0 "x"
OpMemberName %testBlock 1 "w"
OpMemberName %testBlock 2 "y"
OpMemberName %testBlock 3 "z"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpDecorate %_arr_float_int_2 ArrayStride 16
OpMemberDecorate %testBlock 0 Offset 0
OpMemberDecorate %testBlock 0 RelaxedPrecision
OpMemberDecorate %testBlock 1 Offset 4
OpMemberDecorate %testBlock 2 Offset 16
OpMemberDecorate %testBlock 2 RelaxedPrecision
OpMemberDecorate %testBlock 3 Offset 48
OpMemberDecorate %testBlock 3 ColMajor
OpMemberDecorate %testBlock 3 MatrixStride 16
OpMemberDecorate %testBlock 3 RelaxedPrecision
OpDecorate %testBlock Block
OpDecorate %3 Binding 0
OpDecorate %3 DescriptorSet 0
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %24 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
%float = OpTypeFloat 32
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%testBlock = OpTypeStruct %float %int %_arr_float_int_2 %mat3v3float
%_ptr_Uniform_testBlock = OpTypePointer Uniform %testBlock
%3 = OpVariable %_ptr_Uniform_testBlock Uniform
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%19 = OpTypeFunction %void
%int_0 = OpConstant %int 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int_1 = OpConstant %int 1
%float_0 = OpConstant %float 0
%main = OpFunction %void None %19
%20 = OpLabel
%22 = OpAccessChain %_ptr_Uniform_float %3 %int_0
%24 = OpLoad %float %22
%25 = OpAccessChain %_ptr_Uniform_float %3 %int_2 %int_0
%26 = OpLoad %float %25
%28 = OpAccessChain %_ptr_Uniform_float %3 %int_2 %int_1
%29 = OpLoad %float %28
%31 = OpCompositeConstruct %v4float %24 %26 %29 %float_0
OpStore %sk_FragColor %31
OpReturn
OpFunctionEnd
