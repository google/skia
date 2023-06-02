OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %verify_const_globals_bii "verify_const_globals_bii"
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
OpDecorate %52 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
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
%void = OpTypeVoid
%16 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%26 = OpTypeFunction %bool %_ptr_Function_int %_ptr_Function_int
%false = OpConstantFalse %bool
%int_7 = OpConstant %int 7
%int_10 = OpConstant %int 10
%40 = OpTypeFunction %v4float %_ptr_Function_v2float
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%verify_const_globals_bii = OpFunction %bool None %26
%27 = OpFunctionParameter %_ptr_Function_int
%28 = OpFunctionParameter %_ptr_Function_int
%29 = OpLabel
%31 = OpLoad %int %27
%33 = OpIEqual %bool %31 %int_7
OpSelectionMerge %35 None
OpBranchConditional %33 %34 %35
%34 = OpLabel
%36 = OpLoad %int %28
%38 = OpIEqual %bool %36 %int_10
OpBranch %35
%35 = OpLabel
%39 = OpPhi %bool %false %29 %38 %34
OpReturnValue %39
OpFunctionEnd
%main = OpFunction %v4float None %40
%41 = OpFunctionParameter %_ptr_Function_v2float
%42 = OpLabel
%43 = OpVariable %_ptr_Function_int Function
%44 = OpVariable %_ptr_Function_int Function
OpStore %43 %int_7
OpStore %44 %int_10
%45 = OpFunctionCall %bool %verify_const_globals_bii %43 %44
%47 = OpCompositeConstruct %v4bool %45 %45 %45 %45
%49 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%52 = OpLoad %v4float %49
%53 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%55 = OpLoad %v4float %53
%48 = OpSelect %v4float %47 %52 %55
OpReturnValue %48
OpFunctionEnd
