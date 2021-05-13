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
OpName %m7 "m7"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m7 "_1_m7"
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
OpDecorate %m7 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
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
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%59 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%m7 = OpVariable %_ptr_Function_mat2v2float Function
OpStore %ok %true
%37 = OpCompositeConstruct %v2float %float_5 %float_6
%38 = OpCompositeConstruct %v2float %float_7 %float_8
%36 = OpCompositeConstruct %mat2v2float %37 %38
OpStore %m7 %36
%40 = OpLoad %bool %ok
OpSelectionMerge %42 None
OpBranchConditional %40 %41 %42
%41 = OpLabel
%43 = OpLoad %mat2v2float %m7
%45 = OpCompositeConstruct %v2float %float_5 %float_6
%46 = OpCompositeConstruct %v2float %float_7 %float_8
%44 = OpCompositeConstruct %mat2v2float %45 %46
%48 = OpCompositeExtract %v2float %43 0
%49 = OpCompositeExtract %v2float %44 0
%50 = OpFOrdEqual %v2bool %48 %49
%51 = OpAll %bool %50
%52 = OpCompositeExtract %v2float %43 1
%53 = OpCompositeExtract %v2float %44 1
%54 = OpFOrdEqual %v2bool %52 %53
%55 = OpAll %bool %54
%56 = OpLogicalAnd %bool %51 %55
OpBranch %42
%42 = OpLabel
%57 = OpPhi %bool %false %25 %56 %41
OpStore %ok %57
%58 = OpLoad %bool %ok
OpReturnValue %58
OpFunctionEnd
%main = OpFunction %v4float None %59
%60 = OpFunctionParameter %_ptr_Function_v2float
%61 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m7 = OpVariable %_ptr_Function_mat2v2float Function
%89 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%65 = OpCompositeConstruct %v2float %float_5 %float_6
%66 = OpCompositeConstruct %v2float %float_7 %float_8
%64 = OpCompositeConstruct %mat2v2float %65 %66
OpStore %_1_m7 %64
%67 = OpLoad %bool %_0_ok
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%70 = OpLoad %mat2v2float %_1_m7
%72 = OpCompositeConstruct %v2float %float_5 %float_6
%73 = OpCompositeConstruct %v2float %float_7 %float_8
%71 = OpCompositeConstruct %mat2v2float %72 %73
%74 = OpCompositeExtract %v2float %70 0
%75 = OpCompositeExtract %v2float %71 0
%76 = OpFOrdEqual %v2bool %74 %75
%77 = OpAll %bool %76
%78 = OpCompositeExtract %v2float %70 1
%79 = OpCompositeExtract %v2float %71 1
%80 = OpFOrdEqual %v2bool %78 %79
%81 = OpAll %bool %80
%82 = OpLogicalAnd %bool %77 %81
OpBranch %69
%69 = OpLabel
%83 = OpPhi %bool %false %61 %82 %68
OpStore %_0_ok %83
%84 = OpLoad %bool %_0_ok
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%87 = OpFunctionCall %bool %test_half_b
OpBranch %86
%86 = OpLabel
%88 = OpPhi %bool %false %69 %87 %85
OpSelectionMerge %93 None
OpBranchConditional %88 %91 %92
%91 = OpLabel
%94 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%98 = OpLoad %v4float %94
OpStore %89 %98
OpBranch %93
%92 = OpLabel
%99 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%101 = OpLoad %v4float %99
OpStore %89 %101
OpBranch %93
%93 = OpLabel
%102 = OpLoad %v4float %89
OpReturnValue %102
OpFunctionEnd
