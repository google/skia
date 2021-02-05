OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %x "x"
OpName %y "y"
OpName %z "z"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %41 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_3 = OpConstant %int 3
%float_0_5 = OpConstant %float 0.5
%int_8 = OpConstant %int 8
%float_14 = OpConstant %float 14
%float_0_0500000007 = OpConstant %float 0.0500000007
%int_2 = OpConstant %int 2
%int_32 = OpConstant %int 32
%float_6 = OpConstant %float 6
%int_6 = OpConstant %int 6
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%x = OpVariable %_ptr_Function_float Function
%y = OpVariable %_ptr_Function_float Function
%z = OpVariable %_ptr_Function_int Function
OpStore %x %float_1
OpStore %y %float_2
OpStore %z %int_3
OpStore %x %float_2
OpStore %y %float_0_5
OpStore %z %int_8
OpStore %x %float_14
OpStore %x %float_2
OpStore %y %float_0_0500000007
%33 = OpFMul %float %float_2 %float_0_0500000007
OpStore %x %33
OpStore %z %int_8
OpStore %z %int_8
OpStore %z %int_8
OpStore %z %int_2
OpStore %z %int_32
OpStore %z %int_2
OpStore %x %float_6
OpStore %y %float_6
OpStore %z %int_6
%38 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%41 = OpLoad %v4float %38
OpReturnValue %41
OpFunctionEnd
