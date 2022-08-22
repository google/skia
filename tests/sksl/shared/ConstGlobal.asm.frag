OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %SEVEN "SEVEN"
OpName %TEN "TEN"
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
OpDecorate %17 Binding 0
OpDecorate %17 DescriptorSet 0
OpDecorate %59 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%int = OpTypeInt 32 1
%_ptr_Private_int = OpTypePointer Private %int
%SEVEN = OpVariable %_ptr_Private_int Private
%int_7 = OpConstant %int 7
%TEN = OpVariable %_ptr_Private_int Private
%int_10 = OpConstant %int 10
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%17 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%22 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%26 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_int = OpTypePointer Function %int
%31 = OpTypeFunction %bool %_ptr_Function_int %_ptr_Function_int
%false = OpConstantFalse %bool
%43 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %22
%23 = OpLabel
%27 = OpVariable %_ptr_Function_v2float Function
OpStore %27 %26
%29 = OpFunctionCall %v4float %main %27
OpStore %sk_FragColor %29
OpReturn
OpFunctionEnd
%verify_const_globals_bii = OpFunction %bool None %31
%32 = OpFunctionParameter %_ptr_Function_int
%33 = OpFunctionParameter %_ptr_Function_int
%34 = OpLabel
%36 = OpLoad %int %32
%37 = OpIEqual %bool %36 %int_7
OpSelectionMerge %39 None
OpBranchConditional %37 %38 %39
%38 = OpLabel
%40 = OpLoad %int %33
%41 = OpIEqual %bool %40 %int_10
OpBranch %39
%39 = OpLabel
%42 = OpPhi %bool %false %34 %41 %38
OpReturnValue %42
OpFunctionEnd
%main = OpFunction %v4float None %43
%44 = OpFunctionParameter %_ptr_Function_v2float
%45 = OpLabel
%47 = OpVariable %_ptr_Function_int Function
%49 = OpVariable %_ptr_Function_int Function
%51 = OpVariable %_ptr_Function_v4float Function
OpStore %SEVEN %int_7
OpStore %TEN %int_10
%46 = OpLoad %int %SEVEN
OpStore %47 %46
%48 = OpLoad %int %TEN
OpStore %49 %48
%50 = OpFunctionCall %bool %verify_const_globals_bii %47 %49
OpSelectionMerge %55 None
OpBranchConditional %50 %53 %54
%53 = OpLabel
%56 = OpAccessChain %_ptr_Uniform_v4float %17 %int_0
%59 = OpLoad %v4float %56
OpStore %51 %59
OpBranch %55
%54 = OpLabel
%60 = OpAccessChain %_ptr_Uniform_v4float %17 %int_1
%62 = OpLoad %v4float %60
OpStore %51 %62
OpBranch %55
%55 = OpLabel
%63 = OpLoad %v4float %51
OpReturnValue %63
OpFunctionEnd
