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
OpName %test_half_b "test_half_b"
OpName %ok "ok"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m4 "_1_m4"
OpName %_2_m9 "_2_m9"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %29 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%30 = OpTypeFunction %v4float %_ptr_Function_v2float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_6 = OpConstant %float 6
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_9 = OpConstant %float 9
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
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
%test_half_b = OpFunction %bool None %24
%25 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
OpStore %ok %true
%29 = OpLoad %bool %ok
OpReturnValue %29
OpFunctionEnd
%main = OpFunction %v4float None %30
%31 = OpFunctionParameter %_ptr_Function_v2float
%32 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m4 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m9 = OpVariable %_ptr_Function_mat3v3float Function
%98 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%39 = OpCompositeConstruct %v2float %float_6 %float_0
%40 = OpCompositeConstruct %v2float %float_0 %float_6
%38 = OpCompositeConstruct %mat2v2float %39 %40
OpStore %_1_m4 %38
%42 = OpLoad %bool %_0_ok
OpSelectionMerge %44 None
OpBranchConditional %42 %43 %44
%43 = OpLabel
%45 = OpLoad %mat2v2float %_1_m4
%47 = OpCompositeConstruct %v2float %float_6 %float_0
%48 = OpCompositeConstruct %v2float %float_0 %float_6
%46 = OpCompositeConstruct %mat2v2float %47 %48
%50 = OpCompositeExtract %v2float %45 0
%51 = OpCompositeExtract %v2float %46 0
%52 = OpFOrdEqual %v2bool %50 %51
%53 = OpAll %bool %52
%54 = OpCompositeExtract %v2float %45 1
%55 = OpCompositeExtract %v2float %46 1
%56 = OpFOrdEqual %v2bool %54 %55
%57 = OpAll %bool %56
%58 = OpLogicalAnd %bool %53 %57
OpBranch %44
%44 = OpLabel
%59 = OpPhi %bool %false %32 %58 %43
OpStore %_0_ok %59
%66 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%67 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%68 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%65 = OpCompositeConstruct %mat3v3float %66 %67 %68
OpStore %_2_m9 %65
%69 = OpLoad %bool %_0_ok
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%72 = OpLoad %mat3v3float %_2_m9
%74 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%75 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%76 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%73 = OpCompositeConstruct %mat3v3float %74 %75 %76
%78 = OpCompositeExtract %v3float %72 0
%79 = OpCompositeExtract %v3float %73 0
%80 = OpFOrdEqual %v3bool %78 %79
%81 = OpAll %bool %80
%82 = OpCompositeExtract %v3float %72 1
%83 = OpCompositeExtract %v3float %73 1
%84 = OpFOrdEqual %v3bool %82 %83
%85 = OpAll %bool %84
%86 = OpLogicalAnd %bool %81 %85
%87 = OpCompositeExtract %v3float %72 2
%88 = OpCompositeExtract %v3float %73 2
%89 = OpFOrdEqual %v3bool %87 %88
%90 = OpAll %bool %89
%91 = OpLogicalAnd %bool %86 %90
OpBranch %71
%71 = OpLabel
%92 = OpPhi %bool %false %44 %91 %70
OpStore %_0_ok %92
%93 = OpLoad %bool %_0_ok
OpSelectionMerge %95 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%96 = OpFunctionCall %bool %test_half_b
OpBranch %95
%95 = OpLabel
%97 = OpPhi %bool %false %71 %96 %94
OpSelectionMerge %102 None
OpBranchConditional %97 %100 %101
%100 = OpLabel
%103 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%107 = OpLoad %v4float %103
OpStore %98 %107
OpBranch %102
%101 = OpLabel
%108 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%110 = OpLoad %v4float %108
OpStore %98 %110
OpBranch %102
%102 = OpLabel
%111 = OpLoad %v4float %98
OpReturnValue %111
OpFunctionEnd
