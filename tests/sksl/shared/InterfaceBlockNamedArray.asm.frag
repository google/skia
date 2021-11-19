### Compilation failed:

error: SPIR-V validation error: Block decoration on target <id> '4[%_arr_testBlock_int_2]' must be a structure type
  OpDecorate %_arr_testBlock_int_2 Block

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %testBlock "testBlock"
OpMemberName %testBlock 0 "x"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpMemberDecorate %testBlock 0 Offset 0
OpDecorate %_arr_testBlock_int_2 ArrayStride 16
OpDecorate %_arr_testBlock_int_2 Block
OpDecorate %3 Binding 123
OpDecorate %3 DescriptorSet 0
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %24 RelaxedPrecision
%float = OpTypeFloat 32
%testBlock = OpTypeStruct %float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_testBlock_int_2 = OpTypeArray %testBlock %int_2
%_ptr_Uniform__arr_testBlock_int_2 = OpTypePointer Uniform %_arr_testBlock_int_2
%3 = OpVariable %_ptr_Uniform__arr_testBlock_int_2 Uniform
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%17 = OpTypeFunction %void
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
%main = OpFunction %void None %17
%18 = OpLabel
%21 = OpAccessChain %_ptr_Uniform_float %3 %int_1 %int_0
%23 = OpLoad %float %21
%24 = OpCompositeConstruct %v4float %23 %23 %23 %23
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd

1 error
