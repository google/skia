OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "unknownInput"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %color "color"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %color RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
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
%15 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%28 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%_ptr_Function_float = OpTypePointer Function %float
%int_1 = OpConstant %int 1
%float_2 = OpConstant %float 2
%int_3 = OpConstant %int 3
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%20 = OpVariable %_ptr_Function_v2float Function
OpStore %20 %19
%22 = OpFunctionCall %v4float %main %20
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %23
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpLabel
%color = OpVariable %_ptr_Function_v4float Function
OpStore %color %28
%29 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%33 = OpLoad %float %29
%35 = OpFOrdEqual %bool %33 %float_1
OpSelectionMerge %37 None
OpBranchConditional %35 %36 %37
%36 = OpLabel
%38 = OpAccessChain %_ptr_Function_float %color %int_1
OpStore %38 %float_1
OpBranch %37
%37 = OpLabel
%41 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%42 = OpLoad %float %41
%44 = OpFOrdEqual %bool %42 %float_2
OpSelectionMerge %47 None
OpBranchConditional %44 %45 %46
%45 = OpLabel
OpBranch %47
%46 = OpLabel
%48 = OpAccessChain %_ptr_Function_float %color %int_3
OpStore %48 %float_1
OpBranch %47
%47 = OpLabel
%50 = OpLoad %v4float %color
OpReturnValue %50
OpFunctionEnd
