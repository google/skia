OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %sksl_synthetic_uniforms "sksl_synthetic_uniforms"
OpMemberName %sksl_synthetic_uniforms 0 "u_skRTFlip"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %sksl_synthetic_uniforms 0 Offset 32
OpDecorate %sksl_synthetic_uniforms Block
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%sksl_synthetic_uniforms = OpTypeStruct %v2float
%_ptr_Uniform_sksl_synthetic_uniforms = OpTypePointer Uniform %sksl_synthetic_uniforms
%13 = OpVariable %_ptr_Uniform_sksl_synthetic_uniforms Uniform
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%float_0 = OpConstant %float 0
%_ptr_Function_bool = OpTypePointer Function %bool
%int_1 = OpConstant %int 1
%int_n1 = OpConstant %int -1
%main = OpFunction %void None %11
%12 = OpLabel
%25 = OpVariable %_ptr_Function_bool Function
%19 = OpAccessChain %_ptr_Uniform_v2float %13 %int_0
%21 = OpLoad %v2float %19
%22 = OpCompositeExtract %float %21 1
%24 = OpFOrdGreaterThan %bool %22 %float_0
OpSelectionMerge %29 None
OpBranchConditional %24 %27 %28
%27 = OpLabel
%31 = OpLoad %bool %sk_Clockwise
%30 = OpLogicalNot %bool %31
OpStore %25 %30
OpBranch %29
%28 = OpLabel
%32 = OpLoad %bool %sk_Clockwise
OpStore %25 %32
OpBranch %29
%29 = OpLabel
%33 = OpLoad %bool %25
%34 = OpSelect %int %33 %int_1 %int_n1
%37 = OpConvertSToF %float %34
%38 = OpCompositeConstruct %v4float %37 %37 %37 %37
OpStore %sk_FragColor %38
OpReturn
OpFunctionEnd
