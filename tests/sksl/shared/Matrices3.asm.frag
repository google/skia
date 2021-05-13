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
OpName %m3 "m3"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m2 "_1_m2"
OpName %_2_m3 "_2_m3"
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
OpDecorate %37 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %m3 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
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
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%75 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%m3 = OpVariable %_ptr_Function_mat2v2float Function
OpStore %ok %true
%34 = OpCompositeConstruct %v2float %float_5 %float_5
%35 = OpCompositeConstruct %v2float %float_5 %float_5
%33 = OpCompositeConstruct %mat2v2float %34 %35
OpStore %m2 %33
%37 = OpLoad %bool %ok
OpSelectionMerge %39 None
OpBranchConditional %37 %38 %39
%38 = OpLabel
%40 = OpLoad %mat2v2float %m2
%42 = OpCompositeConstruct %v2float %float_5 %float_5
%43 = OpCompositeConstruct %v2float %float_5 %float_5
%41 = OpCompositeConstruct %mat2v2float %42 %43
%45 = OpCompositeExtract %v2float %40 0
%46 = OpCompositeExtract %v2float %41 0
%47 = OpFOrdEqual %v2bool %45 %46
%48 = OpAll %bool %47
%49 = OpCompositeExtract %v2float %40 1
%50 = OpCompositeExtract %v2float %41 1
%51 = OpFOrdEqual %v2bool %49 %50
%52 = OpAll %bool %51
%53 = OpLogicalAnd %bool %48 %52
OpBranch %39
%39 = OpLabel
%54 = OpPhi %bool %false %25 %53 %38
OpStore %ok %54
%56 = OpLoad %mat2v2float %m2
OpStore %m3 %56
%57 = OpLoad %bool %ok
OpSelectionMerge %59 None
OpBranchConditional %57 %58 %59
%58 = OpLabel
%60 = OpLoad %mat2v2float %m3
%62 = OpCompositeConstruct %v2float %float_5 %float_5
%63 = OpCompositeConstruct %v2float %float_5 %float_5
%61 = OpCompositeConstruct %mat2v2float %62 %63
%64 = OpCompositeExtract %v2float %60 0
%65 = OpCompositeExtract %v2float %61 0
%66 = OpFOrdEqual %v2bool %64 %65
%67 = OpAll %bool %66
%68 = OpCompositeExtract %v2float %60 1
%69 = OpCompositeExtract %v2float %61 1
%70 = OpFOrdEqual %v2bool %68 %69
%71 = OpAll %bool %70
%72 = OpLogicalAnd %bool %67 %71
OpBranch %59
%59 = OpLabel
%73 = OpPhi %bool %false %39 %72 %58
OpStore %ok %73
%74 = OpLoad %bool %ok
OpReturnValue %74
OpFunctionEnd
%main = OpFunction %v4float None %75
%76 = OpFunctionParameter %_ptr_Function_v2float
%77 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m2 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m3 = OpVariable %_ptr_Function_mat2v2float Function
%124 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%81 = OpCompositeConstruct %v2float %float_5 %float_5
%82 = OpCompositeConstruct %v2float %float_5 %float_5
%80 = OpCompositeConstruct %mat2v2float %81 %82
OpStore %_1_m2 %80
%83 = OpLoad %bool %_0_ok
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%86 = OpLoad %mat2v2float %_1_m2
%88 = OpCompositeConstruct %v2float %float_5 %float_5
%89 = OpCompositeConstruct %v2float %float_5 %float_5
%87 = OpCompositeConstruct %mat2v2float %88 %89
%90 = OpCompositeExtract %v2float %86 0
%91 = OpCompositeExtract %v2float %87 0
%92 = OpFOrdEqual %v2bool %90 %91
%93 = OpAll %bool %92
%94 = OpCompositeExtract %v2float %86 1
%95 = OpCompositeExtract %v2float %87 1
%96 = OpFOrdEqual %v2bool %94 %95
%97 = OpAll %bool %96
%98 = OpLogicalAnd %bool %93 %97
OpBranch %85
%85 = OpLabel
%99 = OpPhi %bool %false %77 %98 %84
OpStore %_0_ok %99
%101 = OpLoad %mat2v2float %_1_m2
OpStore %_2_m3 %101
%102 = OpLoad %bool %_0_ok
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%105 = OpLoad %mat2v2float %_2_m3
%107 = OpCompositeConstruct %v2float %float_5 %float_5
%108 = OpCompositeConstruct %v2float %float_5 %float_5
%106 = OpCompositeConstruct %mat2v2float %107 %108
%109 = OpCompositeExtract %v2float %105 0
%110 = OpCompositeExtract %v2float %106 0
%111 = OpFOrdEqual %v2bool %109 %110
%112 = OpAll %bool %111
%113 = OpCompositeExtract %v2float %105 1
%114 = OpCompositeExtract %v2float %106 1
%115 = OpFOrdEqual %v2bool %113 %114
%116 = OpAll %bool %115
%117 = OpLogicalAnd %bool %112 %116
OpBranch %104
%104 = OpLabel
%118 = OpPhi %bool %false %85 %117 %103
OpStore %_0_ok %118
%119 = OpLoad %bool %_0_ok
OpSelectionMerge %121 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%122 = OpFunctionCall %bool %test_half_b
OpBranch %121
%121 = OpLabel
%123 = OpPhi %bool %false %104 %122 %120
OpSelectionMerge %128 None
OpBranchConditional %123 %126 %127
%126 = OpLabel
%129 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%133 = OpLoad %v4float %129
OpStore %124 %133
OpBranch %128
%127 = OpLabel
%134 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%136 = OpLoad %v4float %134
OpStore %124 %136
OpBranch %128
%128 = OpLabel
%137 = OpLoad %v4float %124
OpReturnValue %137
OpFunctionEnd
