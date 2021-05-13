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
OpName %m1 "m1"
OpName %m2 "m2"
OpName %m3 "m3"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m1 "_1_m1"
OpName %_2_m2 "_2_m2"
OpName %_3_m3 "_3_m3"
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
OpDecorate %m1 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %m2 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %m3 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%float_5 = OpConstant %float 5
%60 = OpConstantComposite %v4float %float_5 %float_5 %float_5 %float_5
%105 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%m1 = OpVariable %_ptr_Function_mat2v2float Function
%m2 = OpVariable %_ptr_Function_mat2v2float Function
%m3 = OpVariable %_ptr_Function_mat2v2float Function
OpStore %ok %true
%37 = OpCompositeConstruct %v2float %float_1 %float_2
%38 = OpCompositeConstruct %v2float %float_3 %float_4
%36 = OpCompositeConstruct %mat2v2float %37 %38
OpStore %m1 %36
%40 = OpLoad %bool %ok
OpSelectionMerge %42 None
OpBranchConditional %40 %41 %42
%41 = OpLabel
%43 = OpLoad %mat2v2float %m1
%45 = OpCompositeConstruct %v2float %float_1 %float_2
%46 = OpCompositeConstruct %v2float %float_3 %float_4
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
%62 = OpCompositeExtract %float %60 0
%63 = OpCompositeExtract %float %60 1
%64 = OpCompositeExtract %float %60 2
%65 = OpCompositeExtract %float %60 3
%66 = OpCompositeConstruct %v2float %62 %63
%67 = OpCompositeConstruct %v2float %64 %65
%61 = OpCompositeConstruct %mat2v2float %66 %67
OpStore %m2 %61
%68 = OpLoad %bool %ok
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%71 = OpLoad %mat2v2float %m2
%73 = OpCompositeConstruct %v2float %float_5 %float_5
%74 = OpCompositeConstruct %v2float %float_5 %float_5
%72 = OpCompositeConstruct %mat2v2float %73 %74
%75 = OpCompositeExtract %v2float %71 0
%76 = OpCompositeExtract %v2float %72 0
%77 = OpFOrdEqual %v2bool %75 %76
%78 = OpAll %bool %77
%79 = OpCompositeExtract %v2float %71 1
%80 = OpCompositeExtract %v2float %72 1
%81 = OpFOrdEqual %v2bool %79 %80
%82 = OpAll %bool %81
%83 = OpLogicalAnd %bool %78 %82
OpBranch %70
%70 = OpLabel
%84 = OpPhi %bool %false %42 %83 %69
OpStore %ok %84
%86 = OpLoad %mat2v2float %m1
OpStore %m3 %86
%87 = OpLoad %bool %ok
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%90 = OpLoad %mat2v2float %m3
%92 = OpCompositeConstruct %v2float %float_1 %float_2
%93 = OpCompositeConstruct %v2float %float_3 %float_4
%91 = OpCompositeConstruct %mat2v2float %92 %93
%94 = OpCompositeExtract %v2float %90 0
%95 = OpCompositeExtract %v2float %91 0
%96 = OpFOrdEqual %v2bool %94 %95
%97 = OpAll %bool %96
%98 = OpCompositeExtract %v2float %90 1
%99 = OpCompositeExtract %v2float %91 1
%100 = OpFOrdEqual %v2bool %98 %99
%101 = OpAll %bool %100
%102 = OpLogicalAnd %bool %97 %101
OpBranch %89
%89 = OpLabel
%103 = OpPhi %bool %false %70 %102 %88
OpStore %ok %103
%104 = OpLoad %bool %ok
OpReturnValue %104
OpFunctionEnd
%main = OpFunction %v4float None %105
%106 = OpFunctionParameter %_ptr_Function_v2float
%107 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m2 = OpVariable %_ptr_Function_mat2v2float Function
%_3_m3 = OpVariable %_ptr_Function_mat2v2float Function
%179 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%111 = OpCompositeConstruct %v2float %float_1 %float_2
%112 = OpCompositeConstruct %v2float %float_3 %float_4
%110 = OpCompositeConstruct %mat2v2float %111 %112
OpStore %_1_m1 %110
%113 = OpLoad %bool %_0_ok
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%116 = OpLoad %mat2v2float %_1_m1
%118 = OpCompositeConstruct %v2float %float_1 %float_2
%119 = OpCompositeConstruct %v2float %float_3 %float_4
%117 = OpCompositeConstruct %mat2v2float %118 %119
%120 = OpCompositeExtract %v2float %116 0
%121 = OpCompositeExtract %v2float %117 0
%122 = OpFOrdEqual %v2bool %120 %121
%123 = OpAll %bool %122
%124 = OpCompositeExtract %v2float %116 1
%125 = OpCompositeExtract %v2float %117 1
%126 = OpFOrdEqual %v2bool %124 %125
%127 = OpAll %bool %126
%128 = OpLogicalAnd %bool %123 %127
OpBranch %115
%115 = OpLabel
%129 = OpPhi %bool %false %107 %128 %114
OpStore %_0_ok %129
%132 = OpCompositeExtract %float %60 0
%133 = OpCompositeExtract %float %60 1
%134 = OpCompositeExtract %float %60 2
%135 = OpCompositeExtract %float %60 3
%136 = OpCompositeConstruct %v2float %132 %133
%137 = OpCompositeConstruct %v2float %134 %135
%131 = OpCompositeConstruct %mat2v2float %136 %137
OpStore %_2_m2 %131
%138 = OpLoad %bool %_0_ok
OpSelectionMerge %140 None
OpBranchConditional %138 %139 %140
%139 = OpLabel
%141 = OpLoad %mat2v2float %_2_m2
%143 = OpCompositeConstruct %v2float %float_5 %float_5
%144 = OpCompositeConstruct %v2float %float_5 %float_5
%142 = OpCompositeConstruct %mat2v2float %143 %144
%145 = OpCompositeExtract %v2float %141 0
%146 = OpCompositeExtract %v2float %142 0
%147 = OpFOrdEqual %v2bool %145 %146
%148 = OpAll %bool %147
%149 = OpCompositeExtract %v2float %141 1
%150 = OpCompositeExtract %v2float %142 1
%151 = OpFOrdEqual %v2bool %149 %150
%152 = OpAll %bool %151
%153 = OpLogicalAnd %bool %148 %152
OpBranch %140
%140 = OpLabel
%154 = OpPhi %bool %false %115 %153 %139
OpStore %_0_ok %154
%156 = OpLoad %mat2v2float %_1_m1
OpStore %_3_m3 %156
%157 = OpLoad %bool %_0_ok
OpSelectionMerge %159 None
OpBranchConditional %157 %158 %159
%158 = OpLabel
%160 = OpLoad %mat2v2float %_3_m3
%162 = OpCompositeConstruct %v2float %float_1 %float_2
%163 = OpCompositeConstruct %v2float %float_3 %float_4
%161 = OpCompositeConstruct %mat2v2float %162 %163
%164 = OpCompositeExtract %v2float %160 0
%165 = OpCompositeExtract %v2float %161 0
%166 = OpFOrdEqual %v2bool %164 %165
%167 = OpAll %bool %166
%168 = OpCompositeExtract %v2float %160 1
%169 = OpCompositeExtract %v2float %161 1
%170 = OpFOrdEqual %v2bool %168 %169
%171 = OpAll %bool %170
%172 = OpLogicalAnd %bool %167 %171
OpBranch %159
%159 = OpLabel
%173 = OpPhi %bool %false %140 %172 %158
OpStore %_0_ok %173
%174 = OpLoad %bool %_0_ok
OpSelectionMerge %176 None
OpBranchConditional %174 %175 %176
%175 = OpLabel
%177 = OpFunctionCall %bool %test_half_b
OpBranch %176
%176 = OpLabel
%178 = OpPhi %bool %false %159 %177 %175
OpSelectionMerge %183 None
OpBranchConditional %178 %181 %182
%181 = OpLabel
%184 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%188 = OpLoad %v4float %184
OpStore %179 %188
OpBranch %183
%182 = OpLabel
%189 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%191 = OpLoad %v4float %189
OpStore %179 %191
OpBranch %183
%183 = OpLabel
%192 = OpLoad %v4float %179
OpReturnValue %192
OpFunctionEnd
