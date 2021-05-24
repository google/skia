OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "unknownInput"
OpName %main "main"
OpName %value "value"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %value RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%int = OpTypeInt 32 1
%_UniformBuffer = OpTypeStruct %int
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_int = OpTypePointer Uniform %int
%int_0 = OpConstant %int 0
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%main = OpFunction %void None %15
%16 = OpLabel
%value = OpVariable %_ptr_Function_float Function
%19 = OpAccessChain %_ptr_Uniform_int %10 %int_0
%22 = OpLoad %int %19
OpSelectionMerge %23 None
OpSwitch %22 %26 0 %24 1 %25
%24 = OpLabel
OpStore %value %float_0
OpBranch %25
%25 = OpLabel
OpStore %value %float_1
OpBranch %26
%26 = OpLabel
OpStore %value %float_2
OpBranch %23
%23 = OpLabel
%30 = OpLoad %float %value
%31 = OpCompositeConstruct %v4float %30 %30 %30 %30
OpStore %sk_FragColor %31
OpReturn
OpFunctionEnd
