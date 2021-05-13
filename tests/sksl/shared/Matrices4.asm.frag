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
OpName %m2 "m2"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m2 "_1_m2"
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
OpDecorate %m2 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
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
%33 = OpConstantComposite %v4float %float_5 %float_5 %float_5 %float_5
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%61 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%m2 = OpVariable %_ptr_Function_mat2v2float Function
OpStore %ok %true
%35 = OpCompositeExtract %float %33 0
%36 = OpCompositeExtract %float %33 1
%37 = OpCompositeExtract %float %33 2
%38 = OpCompositeExtract %float %33 3
%39 = OpCompositeConstruct %v2float %35 %36
%40 = OpCompositeConstruct %v2float %37 %38
%34 = OpCompositeConstruct %mat2v2float %39 %40
OpStore %m2 %34
%42 = OpLoad %bool %ok
OpSelectionMerge %44 None
OpBranchConditional %42 %43 %44
%43 = OpLabel
%45 = OpLoad %mat2v2float %m2
%47 = OpCompositeConstruct %v2float %float_5 %float_0
%48 = OpCompositeConstruct %v2float %float_0 %float_5
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
%59 = OpPhi %bool %false %25 %58 %43
OpStore %ok %59
%60 = OpLoad %bool %ok
OpReturnValue %60
OpFunctionEnd
%main = OpFunction %v4float None %61
%62 = OpFunctionParameter %_ptr_Function_v2float
%63 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m2 = OpVariable %_ptr_Function_mat2v2float Function
%95 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%67 = OpCompositeExtract %float %33 0
%68 = OpCompositeExtract %float %33 1
%69 = OpCompositeExtract %float %33 2
%70 = OpCompositeExtract %float %33 3
%71 = OpCompositeConstruct %v2float %67 %68
%72 = OpCompositeConstruct %v2float %69 %70
%66 = OpCompositeConstruct %mat2v2float %71 %72
OpStore %_1_m2 %66
%73 = OpLoad %bool %_0_ok
OpSelectionMerge %75 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
%76 = OpLoad %mat2v2float %_1_m2
%78 = OpCompositeConstruct %v2float %float_5 %float_0
%79 = OpCompositeConstruct %v2float %float_0 %float_5
%77 = OpCompositeConstruct %mat2v2float %78 %79
%80 = OpCompositeExtract %v2float %76 0
%81 = OpCompositeExtract %v2float %77 0
%82 = OpFOrdEqual %v2bool %80 %81
%83 = OpAll %bool %82
%84 = OpCompositeExtract %v2float %76 1
%85 = OpCompositeExtract %v2float %77 1
%86 = OpFOrdEqual %v2bool %84 %85
%87 = OpAll %bool %86
%88 = OpLogicalAnd %bool %83 %87
OpBranch %75
%75 = OpLabel
%89 = OpPhi %bool %false %63 %88 %74
OpStore %_0_ok %89
%90 = OpLoad %bool %_0_ok
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%93 = OpFunctionCall %bool %test_half_b
OpBranch %92
%92 = OpLabel
%94 = OpPhi %bool %false %75 %93 %91
OpSelectionMerge %99 None
OpBranchConditional %94 %97 %98
%97 = OpLabel
%100 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%104 = OpLoad %v4float %100
OpStore %95 %104
OpBranch %99
%98 = OpLabel
%105 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%107 = OpLoad %v4float %105
OpStore %95 %107
OpBranch %99
%99 = OpLabel
%108 = OpLoad %v4float %95
OpReturnValue %108
OpFunctionEnd
