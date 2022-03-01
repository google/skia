OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorWhite"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpMemberName %_UniformBuffer 3 "testMatrix2x2"
OpMemberName %_UniformBuffer 4 "testMatrix3x3"
OpMemberName %_UniformBuffer 5 "testMatrix4x4"
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_iscalar_b "test_iscalar_b"
OpName %x "x"
OpName %test_fvec_b "test_fvec_b"
OpName %x_0 "x"
OpName %test_ivec_b "test_ivec_b"
OpName %x_1 "x"
OpName %test_mat2_b "test_mat2_b"
OpName %negated "negated"
OpName %x_2 "x"
OpName %test_mat3_b "test_mat3_b"
OpName %negated_0 "negated"
OpName %x_3 "x"
OpName %test_mat4_b "test_mat4_b"
OpName %negated_1 "negated"
OpName %x_4 "x"
OpName %main "main"
OpName %_0_x "_0_x"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
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
OpDecorate %16 Binding 0
OpDecorate %16 DescriptorSet 0
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %x_0 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%mat4v4float = OpTypeMatrix %v4float 4
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %mat2v2float %mat3v3float %mat4v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%16 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%26 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%29 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%33 = OpTypeFunction %bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_n1 = OpConstant %int -1
%float_n1 = OpConstant %float -1
%58 = OpConstantComposite %v2float %float_n1 %float_n1
%v2bool = OpTypeVector %bool 2
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%74 = OpConstantComposite %v2int %int_n1 %int_n1
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_n2 = OpConstant %float -2
%float_n3 = OpConstant %float -3
%float_n4 = OpConstant %float -4
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int_3 = OpConstant %int 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_n5 = OpConstant %float -5
%float_n6 = OpConstant %float -6
%float_n7 = OpConstant %float -7
%float_n8 = OpConstant %float -8
%float_n9 = OpConstant %float -9
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
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%int_5 = OpConstant %int 5
%v4bool = OpTypeVector %bool 4
%202 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %26
%27 = OpLabel
%30 = OpVariable %_ptr_Function_v2float Function
OpStore %30 %29
%32 = OpFunctionCall %v4float %main %30
OpStore %sk_FragColor %32
OpReturn
OpFunctionEnd
%test_iscalar_b = OpFunction %bool None %33
%34 = OpLabel
%x = OpVariable %_ptr_Function_int Function
%38 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%41 = OpLoad %v4float %38
%42 = OpCompositeExtract %float %41 0
%43 = OpConvertFToS %int %42
OpStore %x %43
%44 = OpLoad %int %x
%45 = OpSNegate %int %44
OpStore %x %45
%46 = OpLoad %int %x
%48 = OpIEqual %bool %46 %int_n1
OpReturnValue %48
OpFunctionEnd
%test_fvec_b = OpFunction %bool None %33
%49 = OpLabel
%x_0 = OpVariable %_ptr_Function_v2float Function
%51 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%52 = OpLoad %v4float %51
%53 = OpVectorShuffle %v2float %52 %52 0 1
OpStore %x_0 %53
%54 = OpLoad %v2float %x_0
%55 = OpFNegate %v2float %54
OpStore %x_0 %55
%56 = OpLoad %v2float %x_0
%59 = OpFOrdEqual %v2bool %56 %58
%61 = OpAll %bool %59
OpReturnValue %61
OpFunctionEnd
%test_ivec_b = OpFunction %bool None %33
%62 = OpLabel
%x_1 = OpVariable %_ptr_Function_v2int Function
%66 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%67 = OpLoad %v4float %66
%68 = OpCompositeExtract %float %67 0
%69 = OpConvertFToS %int %68
%70 = OpCompositeConstruct %v2int %69 %69
OpStore %x_1 %70
%71 = OpLoad %v2int %x_1
%72 = OpSNegate %v2int %71
OpStore %x_1 %72
%73 = OpLoad %v2int %x_1
%75 = OpIEqual %v2bool %73 %74
%76 = OpAll %bool %75
OpReturnValue %76
OpFunctionEnd
%test_mat2_b = OpFunction %bool None %33
%77 = OpLabel
%negated = OpVariable %_ptr_Function_mat2v2float Function
%x_2 = OpVariable %_ptr_Function_mat2v2float Function
%83 = OpCompositeConstruct %v2float %float_n1 %float_n2
%84 = OpCompositeConstruct %v2float %float_n3 %float_n4
%85 = OpCompositeConstruct %mat2v2float %83 %84
OpStore %negated %85
%87 = OpAccessChain %_ptr_Uniform_mat2v2float %16 %int_3
%90 = OpLoad %mat2v2float %87
OpStore %x_2 %90
%91 = OpLoad %mat2v2float %x_2
%92 = OpCompositeExtract %v2float %91 0
%93 = OpFNegate %v2float %92
%94 = OpCompositeExtract %v2float %91 1
%95 = OpFNegate %v2float %94
%96 = OpCompositeConstruct %mat2v2float %93 %95
OpStore %x_2 %96
%97 = OpLoad %mat2v2float %x_2
%98 = OpLoad %mat2v2float %negated
%99 = OpCompositeExtract %v2float %97 0
%100 = OpCompositeExtract %v2float %98 0
%101 = OpFOrdEqual %v2bool %99 %100
%102 = OpAll %bool %101
%103 = OpCompositeExtract %v2float %97 1
%104 = OpCompositeExtract %v2float %98 1
%105 = OpFOrdEqual %v2bool %103 %104
%106 = OpAll %bool %105
%107 = OpLogicalAnd %bool %102 %106
OpReturnValue %107
OpFunctionEnd
%test_mat3_b = OpFunction %bool None %33
%108 = OpLabel
%negated_0 = OpVariable %_ptr_Function_mat3v3float Function
%x_3 = OpVariable %_ptr_Function_mat3v3float Function
%116 = OpCompositeConstruct %v3float %float_n1 %float_n2 %float_n3
%117 = OpCompositeConstruct %v3float %float_n4 %float_n5 %float_n6
%118 = OpCompositeConstruct %v3float %float_n7 %float_n8 %float_n9
%119 = OpCompositeConstruct %mat3v3float %116 %117 %118
OpStore %negated_0 %119
%121 = OpAccessChain %_ptr_Uniform_mat3v3float %16 %int_4
%124 = OpLoad %mat3v3float %121
OpStore %x_3 %124
%125 = OpLoad %mat3v3float %x_3
%126 = OpCompositeExtract %v3float %125 0
%127 = OpFNegate %v3float %126
%128 = OpCompositeExtract %v3float %125 1
%129 = OpFNegate %v3float %128
%130 = OpCompositeExtract %v3float %125 2
%131 = OpFNegate %v3float %130
%132 = OpCompositeConstruct %mat3v3float %127 %129 %131
OpStore %x_3 %132
%133 = OpLoad %mat3v3float %x_3
%134 = OpLoad %mat3v3float %negated_0
%136 = OpCompositeExtract %v3float %133 0
%137 = OpCompositeExtract %v3float %134 0
%138 = OpFOrdEqual %v3bool %136 %137
%139 = OpAll %bool %138
%140 = OpCompositeExtract %v3float %133 1
%141 = OpCompositeExtract %v3float %134 1
%142 = OpFOrdEqual %v3bool %140 %141
%143 = OpAll %bool %142
%144 = OpLogicalAnd %bool %139 %143
%145 = OpCompositeExtract %v3float %133 2
%146 = OpCompositeExtract %v3float %134 2
%147 = OpFOrdEqual %v3bool %145 %146
%148 = OpAll %bool %147
%149 = OpLogicalAnd %bool %144 %148
OpReturnValue %149
OpFunctionEnd
%test_mat4_b = OpFunction %bool None %33
%150 = OpLabel
%negated_1 = OpVariable %_ptr_Function_mat4v4float Function
%x_4 = OpVariable %_ptr_Function_mat4v4float Function
%160 = OpCompositeConstruct %v4float %float_n1 %float_n2 %float_n3 %float_n4
%161 = OpCompositeConstruct %v4float %float_n5 %float_n6 %float_n7 %float_n8
%162 = OpCompositeConstruct %v4float %float_n9 %float_n10 %float_n11 %float_n12
%163 = OpCompositeConstruct %v4float %float_n13 %float_n14 %float_n15 %float_n16
%164 = OpCompositeConstruct %mat4v4float %160 %161 %162 %163
OpStore %negated_1 %164
%166 = OpAccessChain %_ptr_Uniform_mat4v4float %16 %int_5
%169 = OpLoad %mat4v4float %166
OpStore %x_4 %169
%170 = OpLoad %mat4v4float %x_4
%171 = OpCompositeExtract %v4float %170 0
%172 = OpFNegate %v4float %171
%173 = OpCompositeExtract %v4float %170 1
%174 = OpFNegate %v4float %173
%175 = OpCompositeExtract %v4float %170 2
%176 = OpFNegate %v4float %175
%177 = OpCompositeExtract %v4float %170 3
%178 = OpFNegate %v4float %177
%179 = OpCompositeConstruct %mat4v4float %172 %174 %176 %178
OpStore %x_4 %179
%180 = OpLoad %mat4v4float %x_4
%181 = OpLoad %mat4v4float %negated_1
%183 = OpCompositeExtract %v4float %180 0
%184 = OpCompositeExtract %v4float %181 0
%185 = OpFOrdEqual %v4bool %183 %184
%186 = OpAll %bool %185
%187 = OpCompositeExtract %v4float %180 1
%188 = OpCompositeExtract %v4float %181 1
%189 = OpFOrdEqual %v4bool %187 %188
%190 = OpAll %bool %189
%191 = OpLogicalAnd %bool %186 %190
%192 = OpCompositeExtract %v4float %180 2
%193 = OpCompositeExtract %v4float %181 2
%194 = OpFOrdEqual %v4bool %192 %193
%195 = OpAll %bool %194
%196 = OpLogicalAnd %bool %191 %195
%197 = OpCompositeExtract %v4float %180 3
%198 = OpCompositeExtract %v4float %181 3
%199 = OpFOrdEqual %v4bool %197 %198
%200 = OpAll %bool %199
%201 = OpLogicalAnd %bool %196 %200
OpReturnValue %201
OpFunctionEnd
%main = OpFunction %v4float None %202
%203 = OpFunctionParameter %_ptr_Function_v2float
%204 = OpLabel
%_0_x = OpVariable %_ptr_Function_float Function
%239 = OpVariable %_ptr_Function_v4float Function
%207 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%208 = OpLoad %v4float %207
%209 = OpCompositeExtract %float %208 0
OpStore %_0_x %209
%210 = OpLoad %float %_0_x
%211 = OpFNegate %float %210
OpStore %_0_x %211
%213 = OpLoad %float %_0_x
%214 = OpFOrdEqual %bool %213 %float_n1
OpSelectionMerge %216 None
OpBranchConditional %214 %215 %216
%215 = OpLabel
%217 = OpFunctionCall %bool %test_iscalar_b
OpBranch %216
%216 = OpLabel
%218 = OpPhi %bool %false %204 %217 %215
OpSelectionMerge %220 None
OpBranchConditional %218 %219 %220
%219 = OpLabel
%221 = OpFunctionCall %bool %test_fvec_b
OpBranch %220
%220 = OpLabel
%222 = OpPhi %bool %false %216 %221 %219
OpSelectionMerge %224 None
OpBranchConditional %222 %223 %224
%223 = OpLabel
%225 = OpFunctionCall %bool %test_ivec_b
OpBranch %224
%224 = OpLabel
%226 = OpPhi %bool %false %220 %225 %223
OpSelectionMerge %228 None
OpBranchConditional %226 %227 %228
%227 = OpLabel
%229 = OpFunctionCall %bool %test_mat2_b
OpBranch %228
%228 = OpLabel
%230 = OpPhi %bool %false %224 %229 %227
OpSelectionMerge %232 None
OpBranchConditional %230 %231 %232
%231 = OpLabel
%233 = OpFunctionCall %bool %test_mat3_b
OpBranch %232
%232 = OpLabel
%234 = OpPhi %bool %false %228 %233 %231
OpSelectionMerge %236 None
OpBranchConditional %234 %235 %236
%235 = OpLabel
%237 = OpFunctionCall %bool %test_mat4_b
OpBranch %236
%236 = OpLabel
%238 = OpPhi %bool %false %232 %237 %235
OpSelectionMerge %243 None
OpBranchConditional %238 %241 %242
%241 = OpLabel
%244 = OpAccessChain %_ptr_Uniform_v4float %16 %int_1
%246 = OpLoad %v4float %244
OpStore %239 %246
OpBranch %243
%242 = OpLabel
%247 = OpAccessChain %_ptr_Uniform_v4float %16 %int_2
%249 = OpLoad %v4float %247
OpStore %239 %249
OpBranch %243
%243 = OpLabel
%250 = OpLoad %v4float %239
OpReturnValue %250
OpFunctionEnd
