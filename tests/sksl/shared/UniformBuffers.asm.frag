### Compilation failed:

error: SPIR-V validation error: If OpTypeBool is stored in conjunction with OpVariable, it can only be used with non-externally visible shader Storage Classes: Workgroup, CrossWorkgroup, Private, and Function
  %3 = OpVariable %_ptr_Uniform_testBlock Uniform

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %testBlock "testBlock"
OpMemberName %testBlock 0 "x"
OpMemberName %testBlock 1 "y"
OpMemberName %testBlock 2 "z"
OpMemberName %testBlock 3 "w"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpDecorate %_arr_float_int_2 ArrayStride 16
OpMemberDecorate %testBlock 0 Offset 0
OpMemberDecorate %testBlock 0 RelaxedPrecision
OpMemberDecorate %testBlock 1 Offset 16
OpMemberDecorate %testBlock 1 RelaxedPrecision
OpMemberDecorate %testBlock 2 Binding 12
OpMemberDecorate %testBlock 2 Offset 48
OpMemberDecorate %testBlock 2 ColMajor
OpMemberDecorate %testBlock 2 MatrixStride 16
OpMemberDecorate %testBlock 2 RelaxedPrecision
OpMemberDecorate %testBlock 3 Offset 96
OpMemberDecorate %testBlock 3 RelaxedPrecision
OpDecorate %testBlock Block
OpDecorate %3 Binding 789
OpDecorate %3 DescriptorSet 0
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %24 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
%float = OpTypeFloat 32
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%v2float = OpTypeVector %float 2
%mat3v2float = OpTypeMatrix %v2float 3
%bool = OpTypeBool
%testBlock = OpTypeStruct %float %_arr_float_int_2 %mat3v2float %bool
%_ptr_Uniform_testBlock = OpTypePointer Uniform %testBlock
%3 = OpVariable %_ptr_Uniform_testBlock Uniform
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
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
%26 = OpAccessChain %_ptr_Uniform_float %3 %int_1 %int_0
%27 = OpLoad %float %26
%28 = OpAccessChain %_ptr_Uniform_float %3 %int_1 %int_1
%29 = OpLoad %float %28
%31 = OpCompositeConstruct %v4float %24 %27 %29 %float_0
OpStore %sk_FragColor %31
OpReturn
OpFunctionEnd

1 error
