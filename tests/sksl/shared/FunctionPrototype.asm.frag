OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpName %_entrypoint_v "_entrypoint_v"
OpName %this_function_is_defined_before_use_h4h4 "this_function_is_defined_before_use_h4h4"
OpName %main "main"
OpName %this_function_is_defined_after_use_h4h4 "this_function_is_defined_after_use_h4h4"
OpName %this_function_is_defined_near_the_end_h4h4 "this_function_is_defined_near_the_end_h4h4"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpDecorate %30 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%18 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%27 = OpTypeFunction %v4float %_ptr_Function_v4float
%34 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %18
%19 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%this_function_is_defined_before_use_h4h4 = OpFunction %v4float None %27
%28 = OpFunctionParameter %_ptr_Function_v4float
%29 = OpLabel
%31 = OpVariable %_ptr_Function_v4float Function
%30 = OpLoad %v4float %28
OpStore %31 %30
%32 = OpFunctionCall %v4float %this_function_is_defined_near_the_end_h4h4 %31
%33 = OpFNegate %v4float %32
OpReturnValue %33
OpFunctionEnd
%main = OpFunction %v4float None %34
%35 = OpFunctionParameter %_ptr_Function_v2float
%36 = OpLabel
%42 = OpVariable %_ptr_Function_v4float Function
%37 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%41 = OpLoad %v4float %37
OpStore %42 %41
%43 = OpFunctionCall %v4float %this_function_is_defined_after_use_h4h4 %42
OpReturnValue %43
OpFunctionEnd
%this_function_is_defined_after_use_h4h4 = OpFunction %v4float None %27
%44 = OpFunctionParameter %_ptr_Function_v4float
%45 = OpLabel
%48 = OpVariable %_ptr_Function_v4float Function
%46 = OpLoad %v4float %44
%47 = OpFNegate %v4float %46
OpStore %48 %47
%49 = OpFunctionCall %v4float %this_function_is_defined_before_use_h4h4 %48
OpReturnValue %49
OpFunctionEnd
%this_function_is_defined_near_the_end_h4h4 = OpFunction %v4float None %27
%50 = OpFunctionParameter %_ptr_Function_v4float
%51 = OpLabel
%52 = OpLoad %v4float %50
OpReturnValue %52
OpFunctionEnd
