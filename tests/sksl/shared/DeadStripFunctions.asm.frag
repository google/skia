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
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %a RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%25 = OpTypeFunction %v4float %_ptr_Function_v4float
%v3float = OpTypeVector %float 3
%float_9_99999975en05 = OpConstant %float 9.99999975e-05
%float_1 = OpConstant %float 1
%45 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%52 = OpTypeFunction %v4float %_ptr_Function_v2float
%float_3 = OpConstant %float 3
%58 = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
%float_n5 = OpConstant %float -5
%61 = OpConstantComposite %v4float %float_n5 %float_n5 %float_n5 %float_n5
%64 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%false = OpConstantFalse %bool
%69 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%unpremul_h4h4 = OpFunction %v4float None %25
%27 = OpFunctionParameter %_ptr_Function_v4float
%28 = OpLabel
%29 = OpLoad %v4float %27
%30 = OpVectorShuffle %v3float %29 %29 0 1 2
%33 = OpLoad %v4float %27
%34 = OpCompositeExtract %float %33 3
%32 = OpExtInst %float %1 FMax %34 %float_9_99999975en05
%37 = OpFDiv %float %float_1 %32
%38 = OpVectorTimesScalar %v3float %30 %37
%39 = OpCompositeExtract %float %38 0
%40 = OpCompositeExtract %float %38 1
%41 = OpCompositeExtract %float %38 2
%42 = OpLoad %v4float %27
%43 = OpCompositeExtract %float %42 3
%44 = OpCompositeConstruct %v4float %39 %40 %41 %43
OpReturnValue %44
OpFunctionEnd
%live_fn_h4h4h4 = OpFunction %v4float None %45
%46 = OpFunctionParameter %_ptr_Function_v4float
%47 = OpFunctionParameter %_ptr_Function_v4float
%48 = OpLabel
%49 = OpLoad %v4float %46
%50 = OpLoad %v4float %47
%51 = OpFAdd %v4float %49 %50
OpReturnValue %51
OpFunctionEnd
%main = OpFunction %v4float None %52
%53 = OpFunctionParameter %_ptr_Function_v2float
%54 = OpLabel
%a = OpVariable %_ptr_Function_v4float Function
%b = OpVariable %_ptr_Function_v4float Function
%59 = OpVariable %_ptr_Function_v4float Function
%62 = OpVariable %_ptr_Function_v4float Function
%65 = OpVariable %_ptr_Function_v4float Function
%79 = OpVariable %_ptr_Function_v4float Function
OpStore %59 %58
OpStore %62 %61
%63 = OpFunctionCall %v4float %live_fn_h4h4h4 %59 %62
OpStore %a %63
OpStore %65 %64
%66 = OpFunctionCall %v4float %unpremul_h4h4 %65
OpStore %b %66
%68 = OpLoad %v4float %a
%70 = OpFOrdNotEqual %v4bool %68 %69
%72 = OpAny %bool %70
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%75 = OpLoad %v4float %b
%76 = OpFOrdNotEqual %v4bool %75 %69
%77 = OpAny %bool %76
OpBranch %74
%74 = OpLabel
%78 = OpPhi %bool %false %54 %77 %73
OpSelectionMerge %82 None
OpBranchConditional %78 %80 %81
%80 = OpLabel
%83 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%87 = OpLoad %v4float %83
OpStore %79 %87
OpBranch %82
%81 = OpLabel
%88 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%90 = OpLoad %v4float %88
OpStore %79 %90
OpBranch %82
%82 = OpLabel
%91 = OpLoad %v4float %79
OpReturnValue %91
OpFunctionEnd
