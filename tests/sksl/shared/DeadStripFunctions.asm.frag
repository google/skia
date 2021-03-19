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
OpDecorate %28 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
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
%52 = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
%float_n5 = OpConstant %float -5
%55 = OpConstantComposite %v4float %float_n5 %float_n5 %float_n5 %float_n5
%58 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%64 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
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
%a = OpVariable %_ptr_Function_v4float Function
%b = OpVariable %_ptr_Function_v4float Function
%53 = OpVariable %_ptr_Function_v4float Function
%56 = OpVariable %_ptr_Function_v4float Function
%59 = OpVariable %_ptr_Function_v4float Function
%74 = OpVariable %_ptr_Function_v4float Function
OpStore %53 %52
OpStore %56 %55
%57 = OpFunctionCall %v4float %live_fn %53 %56
OpStore %a %57
OpStore %59 %58
%60 = OpFunctionCall %v4float %unpremul %59
OpStore %b %60
%62 = OpLoad %v4float %a
%65 = OpFOrdNotEqual %v4bool %62 %64
%67 = OpAny %bool %65
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%70 = OpLoad %v4float %b
%71 = OpFOrdNotEqual %v4bool %70 %64
%72 = OpAny %bool %71
OpBranch %69
%69 = OpLabel
%73 = OpPhi %bool %false %48 %72 %68
OpSelectionMerge %77 None
OpBranchConditional %73 %75 %76
%75 = OpLabel
%78 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%82 = OpLoad %v4float %78
OpStore %74 %82
OpBranch %77
%76 = OpLabel
%83 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%85 = OpLoad %v4float %83
OpStore %74 %85
OpBranch %77
%77 = OpLabel
%86 = OpLoad %v4float %74
OpReturnValue %86
OpFunctionEnd
