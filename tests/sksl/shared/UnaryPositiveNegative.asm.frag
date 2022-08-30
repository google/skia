OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorWhite"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpMemberName %_UniformBuffer 3 "testMatrix2x2"
OpMemberName %_UniformBuffer 4 "testMatrix3x3"
OpMemberName %_UniformBuffer 5 "testMatrix4x4"
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_fscalar_b "test_fscalar_b"
OpName %x "x"
OpName %test_iscalar_b "test_iscalar_b"
OpName %x_0 "x"
OpName %test_fvec_b "test_fvec_b"
OpName %x_1 "x"
OpName %test_ivec_b "test_ivec_b"
OpName %x_2 "x"
OpName %test_mat2_b "test_mat2_b"
OpName %negated "negated"
OpName %x_3 "x"
OpName %test_mat3_b "test_mat3_b"
OpName %negated_0 "negated"
OpName %x_4 "x"
OpName %test_mat4_b "test_mat4_b"
OpName %negated_1 "negated"
OpName %x_5 "x"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 ColMajor
OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
OpMemberDecorate %_UniformBuffer 4 Offset 80
OpMemberDecorate %_UniformBuffer 4 ColMajor
OpMemberDecorate %_UniformBuffer 4 MatrixStride 16
OpMemberDecorate %_UniformBuffer 5 Offset 128
OpMemberDecorate %_UniformBuffer 5 ColMajor
OpMemberDecorate %_UniformBuffer 5 MatrixStride 16
OpDecorate %_UniformBuffer Block
OpDecorate %17 Binding 0
OpDecorate %17 DescriptorSet 0
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %x_1 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%mat4v4float = OpTypeMatrix %v4float 4
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %mat2v2float %mat3v3float %mat4v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%17 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%27 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%30 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%34 = OpTypeFunction %bool
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_n1 = OpConstant %float -1
%_ptr_Function_int = OpTypePointer Function %int
%int_n1 = OpConstant %int -1
%63 = OpConstantComposite %v2float %float_n1 %float_n1
%v2bool = OpTypeVector %bool 2
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%77 = OpConstantComposite %v2int %int_n1 %int_n1
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_n2 = OpConstant %float -2
%float_n3 = OpConstant %float -3
%float_n4 = OpConstant %float -4
%86 = OpConstantComposite %v2float %float_n1 %float_n2
%87 = OpConstantComposite %v2float %float_n3 %float_n4
%88 = OpConstantComposite %mat2v2float %86 %87
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int_3 = OpConstant %int 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_n5 = OpConstant %float -5
%float_n6 = OpConstant %float -6
%float_n7 = OpConstant %float -7
%float_n8 = OpConstant %float -8
%float_n9 = OpConstant %float -9
%112 = OpConstantComposite %v3float %float_n1 %float_n2 %float_n3
%113 = OpConstantComposite %v3float %float_n4 %float_n5 %float_n6
%114 = OpConstantComposite %v3float %float_n7 %float_n8 %float_n9
%115 = OpConstantComposite %mat3v3float %112 %113 %114
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_4 = OpConstant %int 4
%v3bool = OpTypeVector %bool 3
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_n10 = OpConstant %float -10
%float_n11 = OpConstant %float -11
%float_n12 = OpConstant %float -12
%float_n13 = OpConstant %float -13
%float_n14 = OpConstant %float -14
%float_n15 = OpConstant %float -15
%float_n16 = OpConstant %float -16
%147 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n3 %float_n4
%148 = OpConstantComposite %v4float %float_n5 %float_n6 %float_n7 %float_n8
%149 = OpConstantComposite %v4float %float_n9 %float_n10 %float_n11 %float_n12
%150 = OpConstantComposite %v4float %float_n13 %float_n14 %float_n15 %float_n16
%151 = OpConstantComposite %mat4v4float %147 %148 %149 %150
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%int_5 = OpConstant %int 5
%v4bool = OpTypeVector %bool 4
%178 = OpTypeFunction %v4float %_ptr_Function_v2float
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %27
%28 = OpLabel
%31 = OpVariable %_ptr_Function_v2float Function
OpStore %31 %30
%33 = OpFunctionCall %v4float %main %31
OpStore %sk_FragColor %33
OpReturn
OpFunctionEnd
%test_fscalar_b = OpFunction %bool None %34
%35 = OpLabel
%x = OpVariable %_ptr_Function_float Function
%38 = OpAccessChain %_ptr_Uniform_v4float %17 %int_0
%42 = OpLoad %v4float %38
%43 = OpCompositeExtract %float %42 0
OpStore %x %43
%44 = OpFNegate %float %43
OpStore %x %44
%46 = OpFOrdEqual %bool %44 %float_n1
OpReturnValue %46
OpFunctionEnd
%test_iscalar_b = OpFunction %bool None %34
%47 = OpLabel
%x_0 = OpVariable %_ptr_Function_int Function
%50 = OpAccessChain %_ptr_Uniform_v4float %17 %int_0
%51 = OpLoad %v4float %50
%52 = OpCompositeExtract %float %51 0
%53 = OpConvertFToS %int %52
OpStore %x_0 %53
%54 = OpSNegate %int %53
OpStore %x_0 %54
%56 = OpIEqual %bool %54 %int_n1
OpReturnValue %56
OpFunctionEnd
%test_fvec_b = OpFunction %bool None %34
%57 = OpLabel
%x_1 = OpVariable %_ptr_Function_v2float Function
%59 = OpAccessChain %_ptr_Uniform_v4float %17 %int_0
%60 = OpLoad %v4float %59
%61 = OpVectorShuffle %v2float %60 %60 0 1
OpStore %x_1 %61
%62 = OpFNegate %v2float %61
OpStore %x_1 %62
%64 = OpFOrdEqual %v2bool %62 %63
%66 = OpAll %bool %64
OpReturnValue %66
OpFunctionEnd
%test_ivec_b = OpFunction %bool None %34
%67 = OpLabel
%x_2 = OpVariable %_ptr_Function_v2int Function
%71 = OpAccessChain %_ptr_Uniform_v4float %17 %int_0
%72 = OpLoad %v4float %71
%73 = OpCompositeExtract %float %72 0
%74 = OpConvertFToS %int %73
%75 = OpCompositeConstruct %v2int %74 %74
OpStore %x_2 %75
%76 = OpSNegate %v2int %75
OpStore %x_2 %76
%78 = OpIEqual %v2bool %76 %77
%79 = OpAll %bool %78
OpReturnValue %79
OpFunctionEnd
%test_mat2_b = OpFunction %bool None %34
%80 = OpLabel
%negated = OpVariable %_ptr_Function_mat2v2float Function
%x_3 = OpVariable %_ptr_Function_mat2v2float Function
OpStore %negated %88
%90 = OpAccessChain %_ptr_Uniform_mat2v2float %17 %int_3
%93 = OpLoad %mat2v2float %90
OpStore %x_3 %93
%94 = OpCompositeExtract %v2float %93 0
%95 = OpFNegate %v2float %94
%96 = OpCompositeExtract %v2float %93 1
%97 = OpFNegate %v2float %96
%98 = OpCompositeConstruct %mat2v2float %95 %97
OpStore %x_3 %98
%99 = OpFOrdEqual %v2bool %95 %86
%100 = OpAll %bool %99
%101 = OpFOrdEqual %v2bool %97 %87
%102 = OpAll %bool %101
%103 = OpLogicalAnd %bool %100 %102
OpReturnValue %103
OpFunctionEnd
%test_mat3_b = OpFunction %bool None %34
%104 = OpLabel
%negated_0 = OpVariable %_ptr_Function_mat3v3float Function
%x_4 = OpVariable %_ptr_Function_mat3v3float Function
OpStore %negated_0 %115
%117 = OpAccessChain %_ptr_Uniform_mat3v3float %17 %int_4
%120 = OpLoad %mat3v3float %117
OpStore %x_4 %120
%121 = OpCompositeExtract %v3float %120 0
%122 = OpFNegate %v3float %121
%123 = OpCompositeExtract %v3float %120 1
%124 = OpFNegate %v3float %123
%125 = OpCompositeExtract %v3float %120 2
%126 = OpFNegate %v3float %125
%127 = OpCompositeConstruct %mat3v3float %122 %124 %126
OpStore %x_4 %127
%129 = OpFOrdEqual %v3bool %122 %112
%130 = OpAll %bool %129
%131 = OpFOrdEqual %v3bool %124 %113
%132 = OpAll %bool %131
%133 = OpLogicalAnd %bool %130 %132
%134 = OpFOrdEqual %v3bool %126 %114
%135 = OpAll %bool %134
%136 = OpLogicalAnd %bool %133 %135
OpReturnValue %136
OpFunctionEnd
%test_mat4_b = OpFunction %bool None %34
%137 = OpLabel
%negated_1 = OpVariable %_ptr_Function_mat4v4float Function
%x_5 = OpVariable %_ptr_Function_mat4v4float Function
OpStore %negated_1 %151
%153 = OpAccessChain %_ptr_Uniform_mat4v4float %17 %int_5
%156 = OpLoad %mat4v4float %153
OpStore %x_5 %156
%157 = OpCompositeExtract %v4float %156 0
%158 = OpFNegate %v4float %157
%159 = OpCompositeExtract %v4float %156 1
%160 = OpFNegate %v4float %159
%161 = OpCompositeExtract %v4float %156 2
%162 = OpFNegate %v4float %161
%163 = OpCompositeExtract %v4float %156 3
%164 = OpFNegate %v4float %163
%165 = OpCompositeConstruct %mat4v4float %158 %160 %162 %164
OpStore %x_5 %165
%167 = OpFOrdEqual %v4bool %158 %147
%168 = OpAll %bool %167
%169 = OpFOrdEqual %v4bool %160 %148
%170 = OpAll %bool %169
%171 = OpLogicalAnd %bool %168 %170
%172 = OpFOrdEqual %v4bool %162 %149
%173 = OpAll %bool %172
%174 = OpLogicalAnd %bool %171 %173
%175 = OpFOrdEqual %v4bool %164 %150
%176 = OpAll %bool %175
%177 = OpLogicalAnd %bool %174 %176
OpReturnValue %177
OpFunctionEnd
%main = OpFunction %v4float None %178
%179 = OpFunctionParameter %_ptr_Function_v2float
%180 = OpLabel
%207 = OpVariable %_ptr_Function_v4float Function
%182 = OpFunctionCall %bool %test_fscalar_b
OpSelectionMerge %184 None
OpBranchConditional %182 %183 %184
%183 = OpLabel
%185 = OpFunctionCall %bool %test_iscalar_b
OpBranch %184
%184 = OpLabel
%186 = OpPhi %bool %false %180 %185 %183
OpSelectionMerge %188 None
OpBranchConditional %186 %187 %188
%187 = OpLabel
%189 = OpFunctionCall %bool %test_fvec_b
OpBranch %188
%188 = OpLabel
%190 = OpPhi %bool %false %184 %189 %187
OpSelectionMerge %192 None
OpBranchConditional %190 %191 %192
%191 = OpLabel
%193 = OpFunctionCall %bool %test_ivec_b
OpBranch %192
%192 = OpLabel
%194 = OpPhi %bool %false %188 %193 %191
OpSelectionMerge %196 None
OpBranchConditional %194 %195 %196
%195 = OpLabel
%197 = OpFunctionCall %bool %test_mat2_b
OpBranch %196
%196 = OpLabel
%198 = OpPhi %bool %false %192 %197 %195
OpSelectionMerge %200 None
OpBranchConditional %198 %199 %200
%199 = OpLabel
%201 = OpFunctionCall %bool %test_mat3_b
OpBranch %200
%200 = OpLabel
%202 = OpPhi %bool %false %196 %201 %199
OpSelectionMerge %204 None
OpBranchConditional %202 %203 %204
%203 = OpLabel
%205 = OpFunctionCall %bool %test_mat4_b
OpBranch %204
%204 = OpLabel
%206 = OpPhi %bool %false %200 %205 %203
OpSelectionMerge %211 None
OpBranchConditional %206 %209 %210
%209 = OpLabel
%212 = OpAccessChain %_ptr_Uniform_v4float %17 %int_1
%214 = OpLoad %v4float %212
OpStore %207 %214
OpBranch %211
%210 = OpLabel
%215 = OpAccessChain %_ptr_Uniform_v4float %17 %int_2
%217 = OpLoad %v4float %215
OpStore %207 %217
OpBranch %211
%211 = OpLabel
%218 = OpLoad %v4float %207
OpReturnValue %218
OpFunctionEnd
