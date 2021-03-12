OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %unpremul "unpremul"
OpName %live_fn "live_fn"
OpName %main "main"
OpName %TRUE "TRUE"
OpName %FALSE "FALSE"
OpName %a "a"
OpName %b "b"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %24 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%20 = OpTypeFunction %v4float %_ptr_Function_v4float
%v3float = OpTypeVector %float 3
%float_9_99999975en05 = OpConstant %float 9.99999975e-05
%float_1 = OpConstant %float 1
%40 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%47 = OpTypeFunction %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%float_3 = OpConstant %float 3
%57 = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
%float_n5 = OpConstant %float -5
%60 = OpConstantComposite %v4float %float_n5 %float_n5 %float_n5 %float_n5
%63 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%float_0 = OpConstant %float 0
%68 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint = OpFunction %void None %17
%18 = OpLabel
%19 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %19
OpReturn
OpFunctionEnd
%unpremul = OpFunction %v4float None %20
%22 = OpFunctionParameter %_ptr_Function_v4float
%23 = OpLabel
%24 = OpLoad %v4float %22
%25 = OpVectorShuffle %v3float %24 %24 0 1 2
%28 = OpLoad %v4float %22
%29 = OpCompositeExtract %float %28 3
%27 = OpExtInst %float %1 FMax %29 %float_9_99999975en05
%32 = OpFDiv %float %float_1 %27
%33 = OpVectorTimesScalar %v3float %25 %32
%34 = OpCompositeExtract %float %33 0
%35 = OpCompositeExtract %float %33 1
%36 = OpCompositeExtract %float %33 2
%37 = OpLoad %v4float %22
%38 = OpCompositeExtract %float %37 3
%39 = OpCompositeConstruct %v4float %34 %35 %36 %38
OpReturnValue %39
OpFunctionEnd
%live_fn = OpFunction %v4float None %40
%41 = OpFunctionParameter %_ptr_Function_v4float
%42 = OpFunctionParameter %_ptr_Function_v4float
%43 = OpLabel
%44 = OpLoad %v4float %41
%45 = OpLoad %v4float %42
%46 = OpFAdd %v4float %44 %45
OpReturnValue %46
OpFunctionEnd
%main = OpFunction %v4float None %47
%48 = OpLabel
%TRUE = OpVariable %_ptr_Function_bool Function
%FALSE = OpVariable %_ptr_Function_bool Function
%a = OpVariable %_ptr_Function_v4float Function
%b = OpVariable %_ptr_Function_v4float Function
%58 = OpVariable %_ptr_Function_v4float Function
%61 = OpVariable %_ptr_Function_v4float Function
%64 = OpVariable %_ptr_Function_v4float Function
%78 = OpVariable %_ptr_Function_v4float Function
OpStore %TRUE %true
OpStore %FALSE %false
OpStore %58 %57
OpStore %61 %60
%62 = OpFunctionCall %v4float %live_fn %58 %61
OpStore %a %62
OpStore %64 %63
%65 = OpFunctionCall %v4float %unpremul %64
OpStore %b %65
%66 = OpLoad %v4float %a
%69 = OpFOrdNotEqual %v4bool %66 %68
%71 = OpAny %bool %69
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%74 = OpLoad %v4float %b
%75 = OpFOrdNotEqual %v4bool %74 %68
%76 = OpAny %bool %75
OpBranch %73
%73 = OpLabel
%77 = OpPhi %bool %false %48 %76 %72
OpSelectionMerge %81 None
OpBranchConditional %77 %79 %80
%79 = OpLabel
%82 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%86 = OpLoad %v4float %82
OpStore %78 %86
OpBranch %81
%80 = OpLabel
%87 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%89 = OpLoad %v4float %87
OpStore %78 %89
OpBranch %81
%81 = OpLabel
%90 = OpLoad %v4float %78
OpReturnValue %90
OpFunctionEnd
