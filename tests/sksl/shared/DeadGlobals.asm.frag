OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %Pi "Pi"
OpName %Alias1 "Alias1"
OpName %Alias2 "Alias2"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %17 Binding 0
OpDecorate %17 DescriptorSet 0
OpDecorate %37 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Private_float = OpTypePointer Private %float
%Pi = OpVariable %_ptr_Private_float Private
%float_3_1400001 = OpConstant %float 3.1400001
%Alias1 = OpVariable %_ptr_Private_float Private
%Alias2 = OpVariable %_ptr_Private_float Private
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%17 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%22 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%26 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%30 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %22
%23 = OpLabel
%27 = OpVariable %_ptr_Function_v2float Function
OpStore %27 %26
%29 = OpFunctionCall %v4float %main %27
OpStore %sk_FragColor %29
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %30
%31 = OpFunctionParameter %_ptr_Function_v2float
%32 = OpLabel
OpStore %Pi %float_3_1400001
%14 = OpLoad %float %Pi
OpStore %Alias1 %14
%16 = OpLoad %float %Alias1
OpStore %Alias2 %16
%33 = OpAccessChain %_ptr_Uniform_v4float %17 %int_0
%37 = OpLoad %v4float %33
OpReturnValue %37
OpFunctionEnd
