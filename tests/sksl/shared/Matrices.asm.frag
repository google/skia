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
OpName %_1_m1 "_1_m1"
OpName %_2_m2 "_2_m2"
OpName %_3_m3 "_3_m3"
OpName %_4_m4 "_4_m4"
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
OpDecorate %45 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%float_5 = OpConstant %float 5
%65 = OpConstantComposite %v4float %float_5 %float_5 %float_5 %float_5
%float_6 = OpConstant %float 6
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
%_1_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m2 = OpVariable %_ptr_Function_mat2v2float Function
%_3_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m4 = OpVariable %_ptr_Function_mat2v2float Function
%136 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%42 = OpCompositeConstruct %v2float %float_1 %float_2
%43 = OpCompositeConstruct %v2float %float_3 %float_4
%41 = OpCompositeConstruct %mat2v2float %42 %43
OpStore %_1_m1 %41
%45 = OpLoad %bool %_0_ok
OpSelectionMerge %47 None
OpBranchConditional %45 %46 %47
%46 = OpLabel
%48 = OpLoad %mat2v2float %_1_m1
%50 = OpCompositeConstruct %v2float %float_1 %float_2
%51 = OpCompositeConstruct %v2float %float_3 %float_4
%49 = OpCompositeConstruct %mat2v2float %50 %51
%53 = OpCompositeExtract %v2float %48 0
%54 = OpCompositeExtract %v2float %49 0
%55 = OpFOrdEqual %v2bool %53 %54
%56 = OpAll %bool %55
%57 = OpCompositeExtract %v2float %48 1
%58 = OpCompositeExtract %v2float %49 1
%59 = OpFOrdEqual %v2bool %57 %58
%60 = OpAll %bool %59
%61 = OpLogicalAnd %bool %56 %60
OpBranch %47
%47 = OpLabel
%62 = OpPhi %bool %false %32 %61 %46
OpStore %_0_ok %62
%67 = OpCompositeExtract %float %65 0
%68 = OpCompositeExtract %float %65 1
%69 = OpCompositeExtract %float %65 2
%70 = OpCompositeExtract %float %65 3
%71 = OpCompositeConstruct %v2float %67 %68
%72 = OpCompositeConstruct %v2float %69 %70
%66 = OpCompositeConstruct %mat2v2float %71 %72
OpStore %_2_m2 %66
%73 = OpLoad %bool %_0_ok
OpSelectionMerge %75 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
%76 = OpLoad %mat2v2float %_2_m2
%78 = OpCompositeConstruct %v2float %float_5 %float_5
%79 = OpCompositeConstruct %v2float %float_5 %float_5
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
%89 = OpPhi %bool %false %47 %88 %74
OpStore %_0_ok %89
%91 = OpLoad %mat2v2float %_1_m1
OpStore %_3_m3 %91
%92 = OpLoad %bool %_0_ok
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%95 = OpLoad %mat2v2float %_3_m3
%97 = OpCompositeConstruct %v2float %float_1 %float_2
%98 = OpCompositeConstruct %v2float %float_3 %float_4
%96 = OpCompositeConstruct %mat2v2float %97 %98
%99 = OpCompositeExtract %v2float %95 0
%100 = OpCompositeExtract %v2float %96 0
%101 = OpFOrdEqual %v2bool %99 %100
%102 = OpAll %bool %101
%103 = OpCompositeExtract %v2float %95 1
%104 = OpCompositeExtract %v2float %96 1
%105 = OpFOrdEqual %v2bool %103 %104
%106 = OpAll %bool %105
%107 = OpLogicalAnd %bool %102 %106
OpBranch %94
%94 = OpLabel
%108 = OpPhi %bool %false %75 %107 %93
OpStore %_0_ok %108
%112 = OpCompositeConstruct %v2float %float_6 %float_0
%113 = OpCompositeConstruct %v2float %float_0 %float_6
%111 = OpCompositeConstruct %mat2v2float %112 %113
OpStore %_4_m4 %111
%114 = OpLoad %bool %_0_ok
OpSelectionMerge %116 None
OpBranchConditional %114 %115 %116
%115 = OpLabel
%117 = OpLoad %mat2v2float %_4_m4
%119 = OpCompositeConstruct %v2float %float_6 %float_0
%120 = OpCompositeConstruct %v2float %float_0 %float_6
%118 = OpCompositeConstruct %mat2v2float %119 %120
%121 = OpCompositeExtract %v2float %117 0
%122 = OpCompositeExtract %v2float %118 0
%123 = OpFOrdEqual %v2bool %121 %122
%124 = OpAll %bool %123
%125 = OpCompositeExtract %v2float %117 1
%126 = OpCompositeExtract %v2float %118 1
%127 = OpFOrdEqual %v2bool %125 %126
%128 = OpAll %bool %127
%129 = OpLogicalAnd %bool %124 %128
OpBranch %116
%116 = OpLabel
%130 = OpPhi %bool %false %94 %129 %115
OpStore %_0_ok %130
%131 = OpLoad %bool %_0_ok
OpSelectionMerge %133 None
OpBranchConditional %131 %132 %133
%132 = OpLabel
%134 = OpFunctionCall %bool %test_half_b
OpBranch %133
%133 = OpLabel
%135 = OpPhi %bool %false %116 %134 %132
OpSelectionMerge %140 None
OpBranchConditional %135 %138 %139
%138 = OpLabel
%141 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%145 = OpLoad %v4float %141
OpStore %136 %145
OpBranch %140
%139 = OpLabel
%146 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%148 = OpLoad %v4float %146
OpStore %136 %148
OpBranch %140
%140 = OpLabel
%149 = OpLoad %v4float %136
OpReturnValue %149
OpFunctionEnd
