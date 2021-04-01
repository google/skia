OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %unpremul_h4h4 "unpremul_h4h4"
OpName %live_fn_h4h4h4 "live_fn_h4h4h4"
OpName %main "main"
OpName %a "a"
OpName %b "b"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %a RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
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
%float_3 = OpConstant %float 3
%float_n5 = OpConstant %float -5
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%19 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %19
OpReturn
OpFunctionEnd
%unpremul_h4h4 = OpFunction %v4float None %20
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
%live_fn_h4h4h4 = OpFunction %v4float None %40
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
%a = OpVariable %_ptr_Function_v4float Function
%b = OpVariable %_ptr_Function_v4float Function
%53 = OpVariable %_ptr_Function_v4float Function
%56 = OpVariable %_ptr_Function_v4float Function
%59 = OpVariable %_ptr_Function_v4float Function
%75 = OpVariable %_ptr_Function_v4float Function
%52 = OpCompositeConstruct %v4float %float_3 %float_3 %float_3 %float_3
OpStore %53 %52
%55 = OpCompositeConstruct %v4float %float_n5 %float_n5 %float_n5 %float_n5
OpStore %56 %55
%57 = OpFunctionCall %v4float %live_fn_h4h4h4 %53 %56
OpStore %a %57
%58 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
OpStore %59 %58
%60 = OpFunctionCall %v4float %unpremul_h4h4 %59
OpStore %b %60
%62 = OpLoad %v4float %a
%64 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%65 = OpFOrdNotEqual %v4bool %62 %64
%67 = OpAny %bool %65
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%70 = OpLoad %v4float %b
%71 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%72 = OpFOrdNotEqual %v4bool %70 %71
%73 = OpAny %bool %72
OpBranch %69
%69 = OpLabel
%74 = OpPhi %bool %false %48 %73 %68
OpSelectionMerge %78 None
OpBranchConditional %74 %76 %77
%76 = OpLabel
%79 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%83 = OpLoad %v4float %79
OpStore %75 %83
OpBranch %78
%77 = OpLabel
%84 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%86 = OpLoad %v4float %84
OpStore %75 %86
OpBranch %78
%78 = OpLabel
%87 = OpLoad %v4float %75
OpReturnValue %87
OpFunctionEnd
