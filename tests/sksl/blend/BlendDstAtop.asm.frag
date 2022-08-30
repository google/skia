OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %blend_src_atop_h4h4h4 "blend_src_atop_h4h4h4"
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
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %19 RelaxedPrecision
OpDecorate %20 RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Function_v4float = OpTypePointer Function %v4float
%15 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%float_1 = OpConstant %float 1
%void = OpTypeVoid
%31 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%blend_src_atop_h4h4h4 = OpFunction %v4float None %15
%16 = OpFunctionParameter %_ptr_Function_v4float
%17 = OpFunctionParameter %_ptr_Function_v4float
%18 = OpLabel
%19 = OpLoad %v4float %17
%20 = OpCompositeExtract %float %19 3
%21 = OpLoad %v4float %16
%22 = OpVectorTimesScalar %v4float %21 %20
%24 = OpLoad %v4float %16
%25 = OpCompositeExtract %float %24 3
%26 = OpFSub %float %float_1 %25
%27 = OpLoad %v4float %17
%28 = OpVectorTimesScalar %v4float %27 %26
%29 = OpFAdd %v4float %22 %28
OpReturnValue %29
OpFunctionEnd
%main = OpFunction %void None %31
%32 = OpLabel
%38 = OpVariable %_ptr_Function_v4float Function
%42 = OpVariable %_ptr_Function_v4float Function
%33 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%37 = OpLoad %v4float %33
OpStore %38 %37
%39 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%41 = OpLoad %v4float %39
OpStore %42 %41
%43 = OpFunctionCall %v4float %blend_src_atop_h4h4h4 %38 %42
OpStore %sk_FragColor %43
OpReturn
OpFunctionEnd
