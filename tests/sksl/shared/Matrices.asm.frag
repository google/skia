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
OpName %_1_v1 "_1_v1"
OpName %_2_v2 "_2_v2"
OpName %_3_m1 "_3_m1"
OpName %_4_m2 "_4_m2"
OpName %_5_m3 "_5_m3"
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
OpDecorate %47 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
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
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%float_2 = OpConstant %float 2
%mat3v3float = OpTypeMatrix %v3float 3
%float_3 = OpConstant %float 3
%44 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%false = OpConstantFalse %bool
%float_6 = OpConstant %float 6
%52 = OpConstantComposite %v3float %float_6 %float_6 %float_6
%v3bool = OpTypeVector %bool 3
%float_9 = OpConstant %float 9
%68 = OpConstantComposite %v3float %float_9 %float_9 %float_9
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1 = OpConstant %float 1
%float_4 = OpConstant %float 4
%v2bool = OpTypeVector %bool 2
%float_5 = OpConstant %float 5
%100 = OpConstantComposite %v4float %float_5 %float_5 %float_5 %float_5
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
%_1_v1 = OpVariable %_ptr_Function_v3float Function
%_2_v2 = OpVariable %_ptr_Function_v3float Function
%_3_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m2 = OpVariable %_ptr_Function_mat2v2float Function
%_5_m3 = OpVariable %_ptr_Function_mat2v2float Function
%149 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%39 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%40 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%41 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%38 = OpCompositeConstruct %mat3v3float %39 %40 %41
%45 = OpMatrixTimesVector %v3float %38 %44
OpStore %_1_v1 %45
%47 = OpLoad %bool %_0_ok
OpSelectionMerge %49 None
OpBranchConditional %47 %48 %49
%48 = OpLabel
%50 = OpLoad %v3float %_1_v1
%53 = OpFOrdEqual %v3bool %50 %52
%55 = OpAll %bool %53
OpBranch %49
%49 = OpLabel
%56 = OpPhi %bool %false %32 %55 %48
OpStore %_0_ok %56
%59 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%60 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%61 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%58 = OpCompositeConstruct %mat3v3float %59 %60 %61
%62 = OpVectorTimesMatrix %v3float %44 %58
OpStore %_2_v2 %62
%63 = OpLoad %bool %_0_ok
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%66 = OpLoad %v3float %_2_v2
%69 = OpFOrdEqual %v3bool %66 %68
%70 = OpAll %bool %69
OpBranch %65
%65 = OpLabel
%71 = OpPhi %bool %false %49 %70 %64
OpStore %_0_ok %71
%78 = OpCompositeConstruct %v2float %float_1 %float_2
%79 = OpCompositeConstruct %v2float %float_3 %float_4
%77 = OpCompositeConstruct %mat2v2float %78 %79
OpStore %_3_m1 %77
%80 = OpLoad %bool %_0_ok
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%83 = OpLoad %mat2v2float %_3_m1
%85 = OpCompositeConstruct %v2float %float_1 %float_2
%86 = OpCompositeConstruct %v2float %float_3 %float_4
%84 = OpCompositeConstruct %mat2v2float %85 %86
%88 = OpCompositeExtract %v2float %83 0
%89 = OpCompositeExtract %v2float %84 0
%90 = OpFOrdEqual %v2bool %88 %89
%91 = OpAll %bool %90
%92 = OpCompositeExtract %v2float %83 1
%93 = OpCompositeExtract %v2float %84 1
%94 = OpFOrdEqual %v2bool %92 %93
%95 = OpAll %bool %94
%96 = OpLogicalAnd %bool %91 %95
OpBranch %82
%82 = OpLabel
%97 = OpPhi %bool %false %65 %96 %81
OpStore %_0_ok %97
%102 = OpCompositeExtract %float %100 0
%103 = OpCompositeExtract %float %100 1
%104 = OpCompositeExtract %float %100 2
%105 = OpCompositeExtract %float %100 3
%106 = OpCompositeConstruct %v2float %102 %103
%107 = OpCompositeConstruct %v2float %104 %105
%101 = OpCompositeConstruct %mat2v2float %106 %107
OpStore %_4_m2 %101
%108 = OpLoad %bool %_0_ok
OpSelectionMerge %110 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%111 = OpLoad %mat2v2float %_4_m2
%113 = OpCompositeConstruct %v2float %float_5 %float_5
%114 = OpCompositeConstruct %v2float %float_5 %float_5
%112 = OpCompositeConstruct %mat2v2float %113 %114
%115 = OpCompositeExtract %v2float %111 0
%116 = OpCompositeExtract %v2float %112 0
%117 = OpFOrdEqual %v2bool %115 %116
%118 = OpAll %bool %117
%119 = OpCompositeExtract %v2float %111 1
%120 = OpCompositeExtract %v2float %112 1
%121 = OpFOrdEqual %v2bool %119 %120
%122 = OpAll %bool %121
%123 = OpLogicalAnd %bool %118 %122
OpBranch %110
%110 = OpLabel
%124 = OpPhi %bool %false %82 %123 %109
OpStore %_0_ok %124
%126 = OpLoad %mat2v2float %_3_m1
OpStore %_5_m3 %126
%127 = OpLoad %bool %_0_ok
OpSelectionMerge %129 None
OpBranchConditional %127 %128 %129
%128 = OpLabel
%130 = OpLoad %mat2v2float %_5_m3
%132 = OpCompositeConstruct %v2float %float_1 %float_2
%133 = OpCompositeConstruct %v2float %float_3 %float_4
%131 = OpCompositeConstruct %mat2v2float %132 %133
%134 = OpCompositeExtract %v2float %130 0
%135 = OpCompositeExtract %v2float %131 0
%136 = OpFOrdEqual %v2bool %134 %135
%137 = OpAll %bool %136
%138 = OpCompositeExtract %v2float %130 1
%139 = OpCompositeExtract %v2float %131 1
%140 = OpFOrdEqual %v2bool %138 %139
%141 = OpAll %bool %140
%142 = OpLogicalAnd %bool %137 %141
OpBranch %129
%129 = OpLabel
%143 = OpPhi %bool %false %110 %142 %128
OpStore %_0_ok %143
%144 = OpLoad %bool %_0_ok
OpSelectionMerge %146 None
OpBranchConditional %144 %145 %146
%145 = OpLabel
%147 = OpFunctionCall %bool %test_half_b
OpBranch %146
%146 = OpLabel
%148 = OpPhi %bool %false %129 %147 %145
OpSelectionMerge %153 None
OpBranchConditional %148 %151 %152
%151 = OpLabel
%154 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%158 = OpLoad %v4float %154
OpStore %149 %158
OpBranch %153
%152 = OpLabel
%159 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%161 = OpLoad %v4float %159
OpStore %149 %161
OpBranch %153
%153 = OpLabel
%162 = OpLoad %v4float %149
OpReturnValue %162
OpFunctionEnd
