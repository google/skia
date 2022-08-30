OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorRed"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_matrix_op_matrix_float_b "test_matrix_op_matrix_float_b"
OpName %ok "ok"
OpName %splat_4 "splat_4"
OpName %m "m"
OpName %splat_4_0 "splat_4"
OpName %m_0 "m"
OpName %m_1 "m"
OpName %m_2 "m"
OpName %m_3 "m"
OpName %m_4 "m"
OpName %test_matrix_op_matrix_half_b "test_matrix_op_matrix_half_b"
OpName %ok_0 "ok"
OpName %splat_4_1 "splat_4"
OpName %m_5 "m"
OpName %splat_4_2 "splat_4"
OpName %m_6 "m"
OpName %m_7 "m"
OpName %m_8 "m"
OpName %m_9 "m"
OpName %m_10 "m"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %splat_4_1 RelaxedPrecision
OpDecorate %m_5 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %337 RelaxedPrecision
OpDecorate %338 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %splat_4_2 RelaxedPrecision
OpDecorate %m_6 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %366 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %383 RelaxedPrecision
OpDecorate %m_7 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %389 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %400 RelaxedPrecision
OpDecorate %403 RelaxedPrecision
OpDecorate %m_8 RelaxedPrecision
OpDecorate %408 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
OpDecorate %411 RelaxedPrecision
OpDecorate %412 RelaxedPrecision
OpDecorate %415 RelaxedPrecision
OpDecorate %417 RelaxedPrecision
OpDecorate %420 RelaxedPrecision
OpDecorate %423 RelaxedPrecision
OpDecorate %m_9 RelaxedPrecision
OpDecorate %428 RelaxedPrecision
OpDecorate %429 RelaxedPrecision
OpDecorate %430 RelaxedPrecision
OpDecorate %433 RelaxedPrecision
OpDecorate %435 RelaxedPrecision
OpDecorate %m_10 RelaxedPrecision
OpDecorate %440 RelaxedPrecision
OpDecorate %443 RelaxedPrecision
OpDecorate %444 RelaxedPrecision
OpDecorate %446 RelaxedPrecision
OpDecorate %447 RelaxedPrecision
OpDecorate %468 RelaxedPrecision
OpDecorate %471 RelaxedPrecision
OpDecorate %472 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%float_4 = OpConstant %float 4
%34 = OpConstantComposite %v2float %float_4 %float_4
%35 = OpConstantComposite %mat3v2float %34 %34 %34
%float_2 = OpConstant %float 2
%38 = OpConstantComposite %v2float %float_2 %float_0
%39 = OpConstantComposite %v2float %float_0 %float_2
%40 = OpConstantComposite %mat3v2float %38 %39 %21
%false = OpConstantFalse %bool
%float_6 = OpConstant %float 6
%49 = OpConstantComposite %v2float %float_6 %float_4
%50 = OpConstantComposite %v2float %float_4 %float_6
%51 = OpConstantComposite %mat3v2float %49 %50 %34
%v2bool = OpTypeVector %bool 2
%float_n2 = OpConstant %float -2
%float_n4 = OpConstant %float -4
%70 = OpConstantComposite %v2float %float_n2 %float_n4
%71 = OpConstantComposite %v2float %float_n4 %float_n2
%72 = OpConstantComposite %v2float %float_n4 %float_n4
%73 = OpConstantComposite %mat3v2float %70 %71 %72
%float_0_5 = OpConstant %float 0.5
%90 = OpConstantComposite %v2float %float_0_5 %float_0
%91 = OpConstantComposite %v2float %float_0 %float_0_5
%92 = OpConstantComposite %mat3v2float %90 %91 %21
%v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
%106 = OpConstantComposite %v3float %float_4 %float_4 %float_4
%107 = OpConstantComposite %mat2v3float %106 %106
%109 = OpConstantComposite %v3float %float_2 %float_0 %float_0
%110 = OpConstantComposite %v3float %float_0 %float_2 %float_0
%111 = OpConstantComposite %mat2v3float %109 %110
%117 = OpConstantComposite %v3float %float_6 %float_4 %float_4
%118 = OpConstantComposite %v3float %float_4 %float_6 %float_4
%119 = OpConstantComposite %mat2v3float %117 %118
%v3bool = OpTypeVector %bool 3
%132 = OpConstantComposite %v3float %float_2 %float_4 %float_4
%133 = OpConstantComposite %v3float %float_4 %float_2 %float_4
%134 = OpConstantComposite %mat2v3float %132 %133
%141 = OpConstantComposite %v3float %float_2 %float_2 %float_2
%142 = OpConstantComposite %mat2v3float %141 %141
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_1 = OpConstant %float 1
%float_3 = OpConstant %float 3
%float_5 = OpConstant %float 5
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%float_10 = OpConstant %float 10
%float_11 = OpConstant %float 11
%float_12 = OpConstant %float 12
%166 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%167 = OpConstantComposite %v3float %float_4 %float_5 %float_6
%168 = OpConstantComposite %v3float %float_7 %float_8 %float_9
%169 = OpConstantComposite %v3float %float_10 %float_11 %float_12
%170 = OpConstantComposite %mat4v3float %166 %167 %168 %169
%float_16 = OpConstant %float 16
%float_15 = OpConstant %float 15
%float_14 = OpConstant %float 14
%float_13 = OpConstant %float 13
%175 = OpConstantComposite %v3float %float_16 %float_15 %float_14
%176 = OpConstantComposite %v3float %float_13 %float_12 %float_11
%177 = OpConstantComposite %v3float %float_10 %float_9 %float_8
%178 = OpConstantComposite %v3float %float_7 %float_6 %float_5
%179 = OpConstantComposite %mat4v3float %175 %176 %177 %178
%float_17 = OpConstant %float 17
%188 = OpConstantComposite %v3float %float_17 %float_17 %float_17
%189 = OpConstantComposite %mat4v3float %188 %188 %188 %188
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_20 = OpConstant %float 20
%float_30 = OpConstant %float 30
%float_40 = OpConstant %float 40
%float_50 = OpConstant %float 50
%float_60 = OpConstant %float 60
%float_70 = OpConstant %float 70
%float_80 = OpConstant %float 80
%212 = OpConstantComposite %v2float %float_10 %float_20
%213 = OpConstantComposite %v2float %float_30 %float_40
%214 = OpConstantComposite %v2float %float_50 %float_60
%215 = OpConstantComposite %v2float %float_70 %float_80
%216 = OpConstantComposite %mat4v2float %212 %213 %214 %215
%217 = OpConstantComposite %v2float %float_1 %float_2
%218 = OpConstantComposite %v2float %float_3 %float_4
%219 = OpConstantComposite %v2float %float_5 %float_6
%220 = OpConstantComposite %v2float %float_7 %float_8
%221 = OpConstantComposite %mat4v2float %217 %218 %219 %220
%float_18 = OpConstant %float 18
%float_27 = OpConstant %float 27
%float_36 = OpConstant %float 36
%float_45 = OpConstant %float 45
%float_54 = OpConstant %float 54
%float_63 = OpConstant %float 63
%float_72 = OpConstant %float 72
%236 = OpConstantComposite %v2float %float_9 %float_18
%237 = OpConstantComposite %v2float %float_27 %float_36
%238 = OpConstantComposite %v2float %float_45 %float_54
%239 = OpConstantComposite %v2float %float_63 %float_72
%240 = OpConstantComposite %mat4v2float %236 %237 %238 %239
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%256 = OpConstantComposite %v4float %float_10 %float_20 %float_30 %float_40
%257 = OpConstantComposite %mat2v4float %256 %256
%258 = OpConstantComposite %v4float %float_10 %float_10 %float_10 %float_10
%259 = OpConstantComposite %v4float %float_5 %float_5 %float_5 %float_5
%260 = OpConstantComposite %mat2v4float %258 %259
%266 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%267 = OpConstantComposite %v4float %float_2 %float_4 %float_6 %float_8
%268 = OpConstantComposite %mat2v4float %266 %267
%v4bool = OpTypeVector %bool 4
%277 = OpConstantComposite %v3float %float_7 %float_9 %float_11
%278 = OpConstantComposite %v3float %float_8 %float_10 %float_12
%279 = OpConstantComposite %mat2v3float %277 %278
%280 = OpConstantComposite %v2float %float_1 %float_4
%281 = OpConstantComposite %v2float %float_2 %float_5
%mat2v2float = OpTypeMatrix %v2float 2
%283 = OpConstantComposite %mat2v2float %280 %281
%float_39 = OpConstant %float 39
%float_49 = OpConstant %float 49
%float_59 = OpConstant %float 59
%float_68 = OpConstant %float 68
%float_82 = OpConstant %float 82
%292 = OpConstantComposite %v3float %float_39 %float_49 %float_59
%293 = OpConstantComposite %v3float %float_54 %float_68 %float_82
%294 = OpConstantComposite %mat2v3float %292 %293
%451 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%test_matrix_op_matrix_float_b = OpFunction %bool None %25
%26 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%splat_4 = OpVariable %_ptr_Function_mat3v2float Function
%m = OpVariable %_ptr_Function_mat3v2float Function
%splat_4_0 = OpVariable %_ptr_Function_mat2v3float Function
%m_0 = OpVariable %_ptr_Function_mat2v3float Function
%m_1 = OpVariable %_ptr_Function_mat4v3float Function
%m_2 = OpVariable %_ptr_Function_mat4v2float Function
%m_3 = OpVariable %_ptr_Function_mat2v4float Function
%m_4 = OpVariable %_ptr_Function_mat2v3float Function
OpStore %ok %true
OpStore %splat_4 %35
OpStore %m %40
%41 = OpFAdd %v2float %38 %34
%42 = OpFAdd %v2float %39 %34
%43 = OpFAdd %v2float %21 %34
%44 = OpCompositeConstruct %mat3v2float %41 %42 %43
OpStore %m %44
OpSelectionMerge %47 None
OpBranchConditional %true %46 %47
%46 = OpLabel
%53 = OpFOrdEqual %v2bool %41 %49
%54 = OpAll %bool %53
%55 = OpFOrdEqual %v2bool %42 %50
%56 = OpAll %bool %55
%57 = OpLogicalAnd %bool %54 %56
%58 = OpFOrdEqual %v2bool %43 %34
%59 = OpAll %bool %58
%60 = OpLogicalAnd %bool %57 %59
OpBranch %47
%47 = OpLabel
%61 = OpPhi %bool %false %26 %60 %46
OpStore %ok %61
OpStore %m %40
%62 = OpFSub %v2float %38 %34
%63 = OpFSub %v2float %39 %34
%64 = OpFSub %v2float %21 %34
%65 = OpCompositeConstruct %mat3v2float %62 %63 %64
OpStore %m %65
OpSelectionMerge %67 None
OpBranchConditional %61 %66 %67
%66 = OpLabel
%74 = OpFOrdEqual %v2bool %62 %70
%75 = OpAll %bool %74
%76 = OpFOrdEqual %v2bool %63 %71
%77 = OpAll %bool %76
%78 = OpLogicalAnd %bool %75 %77
%79 = OpFOrdEqual %v2bool %64 %72
%80 = OpAll %bool %79
%81 = OpLogicalAnd %bool %78 %80
OpBranch %67
%67 = OpLabel
%82 = OpPhi %bool %false %47 %81 %66
OpStore %ok %82
OpStore %m %40
%83 = OpFDiv %v2float %38 %34
%84 = OpFDiv %v2float %39 %34
%85 = OpFDiv %v2float %21 %34
%86 = OpCompositeConstruct %mat3v2float %83 %84 %85
OpStore %m %86
OpSelectionMerge %88 None
OpBranchConditional %82 %87 %88
%87 = OpLabel
%93 = OpFOrdEqual %v2bool %83 %90
%94 = OpAll %bool %93
%95 = OpFOrdEqual %v2bool %84 %91
%96 = OpAll %bool %95
%97 = OpLogicalAnd %bool %94 %96
%98 = OpFOrdEqual %v2bool %85 %21
%99 = OpAll %bool %98
%100 = OpLogicalAnd %bool %97 %99
OpBranch %88
%88 = OpLabel
%101 = OpPhi %bool %false %67 %100 %87
OpStore %ok %101
OpStore %splat_4_0 %107
OpStore %m_0 %107
%112 = OpFAdd %v3float %106 %109
%113 = OpFAdd %v3float %106 %110
%114 = OpCompositeConstruct %mat2v3float %112 %113
OpStore %m_0 %114
OpSelectionMerge %116 None
OpBranchConditional %101 %115 %116
%115 = OpLabel
%121 = OpFOrdEqual %v3bool %112 %117
%122 = OpAll %bool %121
%123 = OpFOrdEqual %v3bool %113 %118
%124 = OpAll %bool %123
%125 = OpLogicalAnd %bool %122 %124
OpBranch %116
%116 = OpLabel
%126 = OpPhi %bool %false %88 %125 %115
OpStore %ok %126
OpStore %m_0 %107
%127 = OpFSub %v3float %106 %109
%128 = OpFSub %v3float %106 %110
%129 = OpCompositeConstruct %mat2v3float %127 %128
OpStore %m_0 %129
OpSelectionMerge %131 None
OpBranchConditional %126 %130 %131
%130 = OpLabel
%135 = OpFOrdEqual %v3bool %127 %132
%136 = OpAll %bool %135
%137 = OpFOrdEqual %v3bool %128 %133
%138 = OpAll %bool %137
%139 = OpLogicalAnd %bool %136 %138
OpBranch %131
%131 = OpLabel
%140 = OpPhi %bool %false %116 %139 %130
OpStore %ok %140
OpStore %m_0 %107
%143 = OpFDiv %v3float %106 %141
%144 = OpFDiv %v3float %106 %141
%145 = OpCompositeConstruct %mat2v3float %143 %144
OpStore %m_0 %145
OpSelectionMerge %147 None
OpBranchConditional %140 %146 %147
%146 = OpLabel
%148 = OpFOrdEqual %v3bool %143 %141
%149 = OpAll %bool %148
%150 = OpFOrdEqual %v3bool %144 %141
%151 = OpAll %bool %150
%152 = OpLogicalAnd %bool %149 %151
OpBranch %147
%147 = OpLabel
%153 = OpPhi %bool %false %131 %152 %146
OpStore %ok %153
OpStore %m_1 %170
%180 = OpFAdd %v3float %166 %175
%181 = OpFAdd %v3float %167 %176
%182 = OpFAdd %v3float %168 %177
%183 = OpFAdd %v3float %169 %178
%184 = OpCompositeConstruct %mat4v3float %180 %181 %182 %183
OpStore %m_1 %184
OpSelectionMerge %186 None
OpBranchConditional %153 %185 %186
%185 = OpLabel
%190 = OpFOrdEqual %v3bool %180 %188
%191 = OpAll %bool %190
%192 = OpFOrdEqual %v3bool %181 %188
%193 = OpAll %bool %192
%194 = OpLogicalAnd %bool %191 %193
%195 = OpFOrdEqual %v3bool %182 %188
%196 = OpAll %bool %195
%197 = OpLogicalAnd %bool %194 %196
%198 = OpFOrdEqual %v3bool %183 %188
%199 = OpAll %bool %198
%200 = OpLogicalAnd %bool %197 %199
OpBranch %186
%186 = OpLabel
%201 = OpPhi %bool %false %147 %200 %185
OpStore %ok %201
OpStore %m_2 %216
%222 = OpFSub %v2float %212 %217
%223 = OpFSub %v2float %213 %218
%224 = OpFSub %v2float %214 %219
%225 = OpFSub %v2float %215 %220
%226 = OpCompositeConstruct %mat4v2float %222 %223 %224 %225
OpStore %m_2 %226
OpSelectionMerge %228 None
OpBranchConditional %201 %227 %228
%227 = OpLabel
%241 = OpFOrdEqual %v2bool %222 %236
%242 = OpAll %bool %241
%243 = OpFOrdEqual %v2bool %223 %237
%244 = OpAll %bool %243
%245 = OpLogicalAnd %bool %242 %244
%246 = OpFOrdEqual %v2bool %224 %238
%247 = OpAll %bool %246
%248 = OpLogicalAnd %bool %245 %247
%249 = OpFOrdEqual %v2bool %225 %239
%250 = OpAll %bool %249
%251 = OpLogicalAnd %bool %248 %250
OpBranch %228
%228 = OpLabel
%252 = OpPhi %bool %false %186 %251 %227
OpStore %ok %252
OpStore %m_3 %257
%261 = OpFDiv %v4float %256 %258
%262 = OpFDiv %v4float %256 %259
%263 = OpCompositeConstruct %mat2v4float %261 %262
OpStore %m_3 %263
OpSelectionMerge %265 None
OpBranchConditional %252 %264 %265
%264 = OpLabel
%270 = OpFOrdEqual %v4bool %261 %266
%271 = OpAll %bool %270
%272 = OpFOrdEqual %v4bool %262 %267
%273 = OpAll %bool %272
%274 = OpLogicalAnd %bool %271 %273
OpBranch %265
%265 = OpLabel
%275 = OpPhi %bool %false %228 %274 %264
OpStore %ok %275
OpStore %m_4 %279
%284 = OpMatrixTimesMatrix %mat2v3float %279 %283
OpStore %m_4 %284
OpSelectionMerge %286 None
OpBranchConditional %275 %285 %286
%285 = OpLabel
%295 = OpCompositeExtract %v3float %284 0
%296 = OpFOrdEqual %v3bool %295 %292
%297 = OpAll %bool %296
%298 = OpCompositeExtract %v3float %284 1
%299 = OpFOrdEqual %v3bool %298 %293
%300 = OpAll %bool %299
%301 = OpLogicalAnd %bool %297 %300
OpBranch %286
%286 = OpLabel
%302 = OpPhi %bool %false %265 %301 %285
OpStore %ok %302
OpReturnValue %302
OpFunctionEnd
%test_matrix_op_matrix_half_b = OpFunction %bool None %25
%303 = OpLabel
%ok_0 = OpVariable %_ptr_Function_bool Function
%splat_4_1 = OpVariable %_ptr_Function_mat3v2float Function
%m_5 = OpVariable %_ptr_Function_mat3v2float Function
%splat_4_2 = OpVariable %_ptr_Function_mat2v3float Function
%m_6 = OpVariable %_ptr_Function_mat2v3float Function
%m_7 = OpVariable %_ptr_Function_mat4v3float Function
%m_8 = OpVariable %_ptr_Function_mat4v2float Function
%m_9 = OpVariable %_ptr_Function_mat2v4float Function
%m_10 = OpVariable %_ptr_Function_mat2v3float Function
OpStore %ok_0 %true
OpStore %splat_4_1 %35
OpStore %m_5 %40
%307 = OpFAdd %v2float %38 %34
%308 = OpFAdd %v2float %39 %34
%309 = OpFAdd %v2float %21 %34
%310 = OpCompositeConstruct %mat3v2float %307 %308 %309
OpStore %m_5 %310
OpSelectionMerge %312 None
OpBranchConditional %true %311 %312
%311 = OpLabel
%313 = OpFOrdEqual %v2bool %307 %49
%314 = OpAll %bool %313
%315 = OpFOrdEqual %v2bool %308 %50
%316 = OpAll %bool %315
%317 = OpLogicalAnd %bool %314 %316
%318 = OpFOrdEqual %v2bool %309 %34
%319 = OpAll %bool %318
%320 = OpLogicalAnd %bool %317 %319
OpBranch %312
%312 = OpLabel
%321 = OpPhi %bool %false %303 %320 %311
OpStore %ok_0 %321
OpStore %m_5 %40
%322 = OpFSub %v2float %38 %34
%323 = OpFSub %v2float %39 %34
%324 = OpFSub %v2float %21 %34
%325 = OpCompositeConstruct %mat3v2float %322 %323 %324
OpStore %m_5 %325
OpSelectionMerge %327 None
OpBranchConditional %321 %326 %327
%326 = OpLabel
%328 = OpFOrdEqual %v2bool %322 %70
%329 = OpAll %bool %328
%330 = OpFOrdEqual %v2bool %323 %71
%331 = OpAll %bool %330
%332 = OpLogicalAnd %bool %329 %331
%333 = OpFOrdEqual %v2bool %324 %72
%334 = OpAll %bool %333
%335 = OpLogicalAnd %bool %332 %334
OpBranch %327
%327 = OpLabel
%336 = OpPhi %bool %false %312 %335 %326
OpStore %ok_0 %336
OpStore %m_5 %40
%337 = OpFDiv %v2float %38 %34
%338 = OpFDiv %v2float %39 %34
%339 = OpFDiv %v2float %21 %34
%340 = OpCompositeConstruct %mat3v2float %337 %338 %339
OpStore %m_5 %340
OpSelectionMerge %342 None
OpBranchConditional %336 %341 %342
%341 = OpLabel
%343 = OpFOrdEqual %v2bool %337 %90
%344 = OpAll %bool %343
%345 = OpFOrdEqual %v2bool %338 %91
%346 = OpAll %bool %345
%347 = OpLogicalAnd %bool %344 %346
%348 = OpFOrdEqual %v2bool %339 %21
%349 = OpAll %bool %348
%350 = OpLogicalAnd %bool %347 %349
OpBranch %342
%342 = OpLabel
%351 = OpPhi %bool %false %327 %350 %341
OpStore %ok_0 %351
OpStore %splat_4_2 %107
OpStore %m_6 %107
%354 = OpFAdd %v3float %106 %109
%355 = OpFAdd %v3float %106 %110
%356 = OpCompositeConstruct %mat2v3float %354 %355
OpStore %m_6 %356
OpSelectionMerge %358 None
OpBranchConditional %351 %357 %358
%357 = OpLabel
%359 = OpFOrdEqual %v3bool %354 %117
%360 = OpAll %bool %359
%361 = OpFOrdEqual %v3bool %355 %118
%362 = OpAll %bool %361
%363 = OpLogicalAnd %bool %360 %362
OpBranch %358
%358 = OpLabel
%364 = OpPhi %bool %false %342 %363 %357
OpStore %ok_0 %364
OpStore %m_6 %107
%365 = OpFSub %v3float %106 %109
%366 = OpFSub %v3float %106 %110
%367 = OpCompositeConstruct %mat2v3float %365 %366
OpStore %m_6 %367
OpSelectionMerge %369 None
OpBranchConditional %364 %368 %369
%368 = OpLabel
%370 = OpFOrdEqual %v3bool %365 %132
%371 = OpAll %bool %370
%372 = OpFOrdEqual %v3bool %366 %133
%373 = OpAll %bool %372
%374 = OpLogicalAnd %bool %371 %373
OpBranch %369
%369 = OpLabel
%375 = OpPhi %bool %false %358 %374 %368
OpStore %ok_0 %375
OpStore %m_6 %107
%376 = OpFDiv %v3float %106 %141
%377 = OpFDiv %v3float %106 %141
%378 = OpCompositeConstruct %mat2v3float %376 %377
OpStore %m_6 %378
OpSelectionMerge %380 None
OpBranchConditional %375 %379 %380
%379 = OpLabel
%381 = OpFOrdEqual %v3bool %376 %141
%382 = OpAll %bool %381
%383 = OpFOrdEqual %v3bool %377 %141
%384 = OpAll %bool %383
%385 = OpLogicalAnd %bool %382 %384
OpBranch %380
%380 = OpLabel
%386 = OpPhi %bool %false %369 %385 %379
OpStore %ok_0 %386
OpStore %m_7 %170
%388 = OpFAdd %v3float %166 %175
%389 = OpFAdd %v3float %167 %176
%390 = OpFAdd %v3float %168 %177
%391 = OpFAdd %v3float %169 %178
%392 = OpCompositeConstruct %mat4v3float %388 %389 %390 %391
OpStore %m_7 %392
OpSelectionMerge %394 None
OpBranchConditional %386 %393 %394
%393 = OpLabel
%395 = OpFOrdEqual %v3bool %388 %188
%396 = OpAll %bool %395
%397 = OpFOrdEqual %v3bool %389 %188
%398 = OpAll %bool %397
%399 = OpLogicalAnd %bool %396 %398
%400 = OpFOrdEqual %v3bool %390 %188
%401 = OpAll %bool %400
%402 = OpLogicalAnd %bool %399 %401
%403 = OpFOrdEqual %v3bool %391 %188
%404 = OpAll %bool %403
%405 = OpLogicalAnd %bool %402 %404
OpBranch %394
%394 = OpLabel
%406 = OpPhi %bool %false %380 %405 %393
OpStore %ok_0 %406
OpStore %m_8 %216
%408 = OpFSub %v2float %212 %217
%409 = OpFSub %v2float %213 %218
%410 = OpFSub %v2float %214 %219
%411 = OpFSub %v2float %215 %220
%412 = OpCompositeConstruct %mat4v2float %408 %409 %410 %411
OpStore %m_8 %412
OpSelectionMerge %414 None
OpBranchConditional %406 %413 %414
%413 = OpLabel
%415 = OpFOrdEqual %v2bool %408 %236
%416 = OpAll %bool %415
%417 = OpFOrdEqual %v2bool %409 %237
%418 = OpAll %bool %417
%419 = OpLogicalAnd %bool %416 %418
%420 = OpFOrdEqual %v2bool %410 %238
%421 = OpAll %bool %420
%422 = OpLogicalAnd %bool %419 %421
%423 = OpFOrdEqual %v2bool %411 %239
%424 = OpAll %bool %423
%425 = OpLogicalAnd %bool %422 %424
OpBranch %414
%414 = OpLabel
%426 = OpPhi %bool %false %394 %425 %413
OpStore %ok_0 %426
OpStore %m_9 %257
%428 = OpFDiv %v4float %256 %258
%429 = OpFDiv %v4float %256 %259
%430 = OpCompositeConstruct %mat2v4float %428 %429
OpStore %m_9 %430
OpSelectionMerge %432 None
OpBranchConditional %426 %431 %432
%431 = OpLabel
%433 = OpFOrdEqual %v4bool %428 %266
%434 = OpAll %bool %433
%435 = OpFOrdEqual %v4bool %429 %267
%436 = OpAll %bool %435
%437 = OpLogicalAnd %bool %434 %436
OpBranch %432
%432 = OpLabel
%438 = OpPhi %bool %false %414 %437 %431
OpStore %ok_0 %438
OpStore %m_10 %279
%440 = OpMatrixTimesMatrix %mat2v3float %279 %283
OpStore %m_10 %440
OpSelectionMerge %442 None
OpBranchConditional %438 %441 %442
%441 = OpLabel
%443 = OpCompositeExtract %v3float %440 0
%444 = OpFOrdEqual %v3bool %443 %292
%445 = OpAll %bool %444
%446 = OpCompositeExtract %v3float %440 1
%447 = OpFOrdEqual %v3bool %446 %293
%448 = OpAll %bool %447
%449 = OpLogicalAnd %bool %445 %448
OpBranch %442
%442 = OpLabel
%450 = OpPhi %bool %false %432 %449 %441
OpStore %ok_0 %450
OpReturnValue %450
OpFunctionEnd
%main = OpFunction %v4float None %451
%452 = OpFunctionParameter %_ptr_Function_v2float
%453 = OpLabel
%459 = OpVariable %_ptr_Function_v4float Function
%454 = OpFunctionCall %bool %test_matrix_op_matrix_float_b
OpSelectionMerge %456 None
OpBranchConditional %454 %455 %456
%455 = OpLabel
%457 = OpFunctionCall %bool %test_matrix_op_matrix_half_b
OpBranch %456
%456 = OpLabel
%458 = OpPhi %bool %false %453 %457 %455
OpSelectionMerge %463 None
OpBranchConditional %458 %461 %462
%461 = OpLabel
%464 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%468 = OpLoad %v4float %464
OpStore %459 %468
OpBranch %463
%462 = OpLabel
%469 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%471 = OpLoad %v4float %469
OpStore %459 %471
OpBranch %463
%463 = OpLabel
%472 = OpLoad %v4float %459
OpReturnValue %472
OpFunctionEnd
