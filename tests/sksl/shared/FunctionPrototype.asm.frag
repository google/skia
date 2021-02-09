OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpName %_entrypoint "_entrypoint"
OpName %this_function_is_defined_before_use "this_function_is_defined_before_use"
OpName %main "main"
OpName %this_function_is_defined_after_use "this_function_is_defined_after_use"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %24 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%20 = OpTypeFunction %v4float %_ptr_Function_v4float
%25 = OpTypeFunction %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_entrypoint = OpFunction %void None %17
%18 = OpLabel
%19 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %19
OpReturn
OpFunctionEnd
%this_function_is_defined_before_use = OpFunction %v4float None %20
%22 = OpFunctionParameter %_ptr_Function_v4float
%23 = OpLabel
%24 = OpLoad %v4float %22
OpReturnValue %24
OpFunctionEnd
%main = OpFunction %v4float None %25
%26 = OpLabel
%32 = OpVariable %_ptr_Function_v4float Function
%34 = OpVariable %_ptr_Function_v4float Function
%27 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%31 = OpLoad %v4float %27
OpStore %32 %31
%33 = OpFunctionCall %v4float %this_function_is_defined_before_use %32
OpStore %34 %33
%35 = OpFunctionCall %v4float %this_function_is_defined_after_use %34
OpReturnValue %35
OpFunctionEnd
%this_function_is_defined_after_use = OpFunction %v4float None %20
%36 = OpFunctionParameter %_ptr_Function_v4float
%37 = OpLabel
%38 = OpLoad %v4float %36
OpReturnValue %38
OpFunctionEnd
