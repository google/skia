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
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%14 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%main = OpFunction %void None %14
%15 = OpLabel
%value = OpVariable %_ptr_Function_float Function
OpStore %value %float_0
%19 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%23 = OpLoad %float %19
%24 = OpConvertFToS %int %23
OpSelectionMerge %25 None
OpSwitch %24 %25 0 %26 1 %27
%26 = OpLabel
OpStore %value %float_0
OpBranch %27
%27 = OpLabel
OpStore %value %float_1
OpBranch %25
%25 = OpLabel
%29 = OpLoad %float %value
%30 = OpCompositeConstruct %v4float %29 %29 %29 %29
OpStore %sk_FragColor %30
OpReturn
OpFunctionEnd
