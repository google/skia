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
OpName %test_matrix_op_matrix_half_b "test_matrix_op_matrix_half_b"
OpName %ok "ok"
OpName %m "m"
OpName %m_0 "m"
OpName %m_1 "m"
OpName %m_2 "m"
OpName %m_3 "m"
OpName %m_4 "m"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_2_m "_2_m"
OpName %_4_m "_4_m"
OpName %_5_m "_5_m"
OpName %_6_m "_6_m"
OpName %_7_m "_7_m"
OpName %_8_m "_8_m"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %m RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %m_0 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %m_1 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %m_2 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %m_3 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %m_4 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %461 RelaxedPrecision
OpDecorate %464 RelaxedPrecision
OpDecorate %465 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%float_2 = OpConstant %float 2
%33 = OpConstantComposite %v2float %float_2 %float_0
%34 = OpConstantComposite %v2float %float_0 %float_2
%35 = OpConstantComposite %mat3v2float %33 %34 %20
%float_4 = OpConstant %float 4
%37 = OpConstantComposite %v2float %float_4 %float_4
%38 = OpConstantComposite %mat3v2float %37 %37 %37
%false = OpConstantFalse %bool
%float_6 = OpConstant %float 6
%47 = OpConstantComposite %v2float %float_6 %float_4
%48 = OpConstantComposite %v2float %float_4 %float_6
%49 = OpConstantComposite %mat3v2float %47 %48 %37
%v2bool = OpTypeVector %bool 2
%float_n2 = OpConstant %float -2
%float_n4 = OpConstant %float -4
%68 = OpConstantComposite %v2float %float_n2 %float_n4
%69 = OpConstantComposite %v2float %float_n4 %float_n2
%70 = OpConstantComposite %v2float %float_n4 %float_n4
%71 = OpConstantComposite %mat3v2float %68 %69 %70
%float_0_5 = OpConstant %float 0.5
%88 = OpConstantComposite %v2float %float_0_5 %float_0
%89 = OpConstantComposite %v2float %float_0 %float_0_5
%90 = OpConstantComposite %mat3v2float %88 %89 %20
%v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
%104 = OpConstantComposite %v3float %float_4 %float_4 %float_4
%105 = OpConstantComposite %mat2v3float %104 %104
%106 = OpConstantComposite %v3float %float_2 %float_0 %float_0
%107 = OpConstantComposite %v3float %float_0 %float_2 %float_0
%108 = OpConstantComposite %mat2v3float %106 %107
%114 = OpConstantComposite %v3float %float_6 %float_4 %float_4
%115 = OpConstantComposite %v3float %float_4 %float_6 %float_4
%116 = OpConstantComposite %mat2v3float %114 %115
%v3bool = OpTypeVector %bool 3
%129 = OpConstantComposite %v3float %float_2 %float_4 %float_4
%130 = OpConstantComposite %v3float %float_4 %float_2 %float_4
%131 = OpConstantComposite %mat2v3float %129 %130
%138 = OpConstantComposite %v3float %float_2 %float_2 %float_2
%139 = OpConstantComposite %mat2v3float %138 %138
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
%163 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%164 = OpConstantComposite %v3float %float_4 %float_5 %float_6
%165 = OpConstantComposite %v3float %float_7 %float_8 %float_9
%166 = OpConstantComposite %v3float %float_10 %float_11 %float_12
%167 = OpConstantComposite %mat4v3float %163 %164 %165 %166
%float_16 = OpConstant %float 16
%float_15 = OpConstant %float 15
%float_14 = OpConstant %float 14
%float_13 = OpConstant %float 13
%172 = OpConstantComposite %v3float %float_16 %float_15 %float_14
%173 = OpConstantComposite %v3float %float_13 %float_12 %float_11
%174 = OpConstantComposite %v3float %float_10 %float_9 %float_8
%175 = OpConstantComposite %v3float %float_7 %float_6 %float_5
%176 = OpConstantComposite %mat4v3float %172 %173 %174 %175
%float_17 = OpConstant %float 17
%185 = OpConstantComposite %v3float %float_17 %float_17 %float_17
%186 = OpConstantComposite %mat4v3float %185 %185 %185 %185
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_20 = OpConstant %float 20
%float_30 = OpConstant %float 30
%float_40 = OpConstant %float 40
%float_50 = OpConstant %float 50
%float_60 = OpConstant %float 60
%float_70 = OpConstant %float 70
%float_80 = OpConstant %float 80
%209 = OpConstantComposite %v2float %float_10 %float_20
%210 = OpConstantComposite %v2float %float_30 %float_40
%211 = OpConstantComposite %v2float %float_50 %float_60
%212 = OpConstantComposite %v2float %float_70 %float_80
%213 = OpConstantComposite %mat4v2float %209 %210 %211 %212
%214 = OpConstantComposite %v2float %float_1 %float_2
%215 = OpConstantComposite %v2float %float_3 %float_4
%216 = OpConstantComposite %v2float %float_5 %float_6
%217 = OpConstantComposite %v2float %float_7 %float_8
%218 = OpConstantComposite %mat4v2float %214 %215 %216 %217
%float_18 = OpConstant %float 18
%float_27 = OpConstant %float 27
%float_36 = OpConstant %float 36
%float_45 = OpConstant %float 45
%float_54 = OpConstant %float 54
%float_63 = OpConstant %float 63
%float_72 = OpConstant %float 72
%233 = OpConstantComposite %v2float %float_9 %float_18
%234 = OpConstantComposite %v2float %float_27 %float_36
%235 = OpConstantComposite %v2float %float_45 %float_54
%236 = OpConstantComposite %v2float %float_63 %float_72
%237 = OpConstantComposite %mat4v2float %233 %234 %235 %236
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%253 = OpConstantComposite %v4float %float_10 %float_20 %float_30 %float_40
%254 = OpConstantComposite %mat2v4float %253 %253
%255 = OpConstantComposite %v4float %float_10 %float_10 %float_10 %float_10
%256 = OpConstantComposite %v4float %float_5 %float_5 %float_5 %float_5
%257 = OpConstantComposite %mat2v4float %255 %256
%263 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%264 = OpConstantComposite %v4float %float_2 %float_4 %float_6 %float_8
%265 = OpConstantComposite %mat2v4float %263 %264
%v4bool = OpTypeVector %bool 4
%274 = OpConstantComposite %v3float %float_7 %float_9 %float_11
%275 = OpConstantComposite %v3float %float_8 %float_10 %float_12
%276 = OpConstantComposite %mat2v3float %274 %275
%277 = OpConstantComposite %v2float %float_1 %float_4
%278 = OpConstantComposite %v2float %float_2 %float_5
%mat2v2float = OpTypeMatrix %v2float 2
%280 = OpConstantComposite %mat2v2float %277 %278
%float_39 = OpConstant %float 39
%float_49 = OpConstant %float 49
%float_59 = OpConstant %float 59
%float_68 = OpConstant %float 68
%float_82 = OpConstant %float 82
%289 = OpConstantComposite %v3float %float_39 %float_49 %float_59
%290 = OpConstantComposite %v3float %float_54 %float_68 %float_82
%291 = OpConstantComposite %mat2v3float %289 %290
%300 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%test_matrix_op_matrix_half_b = OpFunction %bool None %24
%25 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%m = OpVariable %_ptr_Function_mat3v2float Function
%m_0 = OpVariable %_ptr_Function_mat2v3float Function
%m_1 = OpVariable %_ptr_Function_mat4v3float Function
%m_2 = OpVariable %_ptr_Function_mat4v2float Function
%m_3 = OpVariable %_ptr_Function_mat2v4float Function
%m_4 = OpVariable %_ptr_Function_mat2v3float Function
OpStore %ok %true
OpStore %m %35
%39 = OpFAdd %v2float %33 %37
%40 = OpFAdd %v2float %34 %37
%41 = OpFAdd %v2float %20 %37
%42 = OpCompositeConstruct %mat3v2float %39 %40 %41
OpStore %m %42
OpSelectionMerge %45 None
OpBranchConditional %true %44 %45
%44 = OpLabel
%51 = OpFOrdEqual %v2bool %39 %47
%52 = OpAll %bool %51
%53 = OpFOrdEqual %v2bool %40 %48
%54 = OpAll %bool %53
%55 = OpLogicalAnd %bool %52 %54
%56 = OpFOrdEqual %v2bool %41 %37
%57 = OpAll %bool %56
%58 = OpLogicalAnd %bool %55 %57
OpBranch %45
%45 = OpLabel
%59 = OpPhi %bool %false %25 %58 %44
OpStore %ok %59
OpStore %m %35
%60 = OpFSub %v2float %33 %37
%61 = OpFSub %v2float %34 %37
%62 = OpFSub %v2float %20 %37
%63 = OpCompositeConstruct %mat3v2float %60 %61 %62
OpStore %m %63
OpSelectionMerge %65 None
OpBranchConditional %59 %64 %65
%64 = OpLabel
%72 = OpFOrdEqual %v2bool %60 %68
%73 = OpAll %bool %72
%74 = OpFOrdEqual %v2bool %61 %69
%75 = OpAll %bool %74
%76 = OpLogicalAnd %bool %73 %75
%77 = OpFOrdEqual %v2bool %62 %70
%78 = OpAll %bool %77
%79 = OpLogicalAnd %bool %76 %78
OpBranch %65
%65 = OpLabel
%80 = OpPhi %bool %false %45 %79 %64
OpStore %ok %80
OpStore %m %35
%81 = OpFDiv %v2float %33 %37
%82 = OpFDiv %v2float %34 %37
%83 = OpFDiv %v2float %20 %37
%84 = OpCompositeConstruct %mat3v2float %81 %82 %83
OpStore %m %84
OpSelectionMerge %86 None
OpBranchConditional %80 %85 %86
%85 = OpLabel
%91 = OpFOrdEqual %v2bool %81 %88
%92 = OpAll %bool %91
%93 = OpFOrdEqual %v2bool %82 %89
%94 = OpAll %bool %93
%95 = OpLogicalAnd %bool %92 %94
%96 = OpFOrdEqual %v2bool %83 %20
%97 = OpAll %bool %96
%98 = OpLogicalAnd %bool %95 %97
OpBranch %86
%86 = OpLabel
%99 = OpPhi %bool %false %65 %98 %85
OpStore %ok %99
OpStore %m_0 %105
%109 = OpFAdd %v3float %104 %106
%110 = OpFAdd %v3float %104 %107
%111 = OpCompositeConstruct %mat2v3float %109 %110
OpStore %m_0 %111
OpSelectionMerge %113 None
OpBranchConditional %99 %112 %113
%112 = OpLabel
%118 = OpFOrdEqual %v3bool %109 %114
%119 = OpAll %bool %118
%120 = OpFOrdEqual %v3bool %110 %115
%121 = OpAll %bool %120
%122 = OpLogicalAnd %bool %119 %121
OpBranch %113
%113 = OpLabel
%123 = OpPhi %bool %false %86 %122 %112
OpStore %ok %123
OpStore %m_0 %105
%124 = OpFSub %v3float %104 %106
%125 = OpFSub %v3float %104 %107
%126 = OpCompositeConstruct %mat2v3float %124 %125
OpStore %m_0 %126
OpSelectionMerge %128 None
OpBranchConditional %123 %127 %128
%127 = OpLabel
%132 = OpFOrdEqual %v3bool %124 %129
%133 = OpAll %bool %132
%134 = OpFOrdEqual %v3bool %125 %130
%135 = OpAll %bool %134
%136 = OpLogicalAnd %bool %133 %135
OpBranch %128
%128 = OpLabel
%137 = OpPhi %bool %false %113 %136 %127
OpStore %ok %137
OpStore %m_0 %105
%140 = OpFDiv %v3float %104 %138
%141 = OpFDiv %v3float %104 %138
%142 = OpCompositeConstruct %mat2v3float %140 %141
OpStore %m_0 %142
OpSelectionMerge %144 None
OpBranchConditional %137 %143 %144
%143 = OpLabel
%145 = OpFOrdEqual %v3bool %140 %138
%146 = OpAll %bool %145
%147 = OpFOrdEqual %v3bool %141 %138
%148 = OpAll %bool %147
%149 = OpLogicalAnd %bool %146 %148
OpBranch %144
%144 = OpLabel
%150 = OpPhi %bool %false %128 %149 %143
OpStore %ok %150
OpStore %m_1 %167
%177 = OpFAdd %v3float %163 %172
%178 = OpFAdd %v3float %164 %173
%179 = OpFAdd %v3float %165 %174
%180 = OpFAdd %v3float %166 %175
%181 = OpCompositeConstruct %mat4v3float %177 %178 %179 %180
OpStore %m_1 %181
OpSelectionMerge %183 None
OpBranchConditional %150 %182 %183
%182 = OpLabel
%187 = OpFOrdEqual %v3bool %177 %185
%188 = OpAll %bool %187
%189 = OpFOrdEqual %v3bool %178 %185
%190 = OpAll %bool %189
%191 = OpLogicalAnd %bool %188 %190
%192 = OpFOrdEqual %v3bool %179 %185
%193 = OpAll %bool %192
%194 = OpLogicalAnd %bool %191 %193
%195 = OpFOrdEqual %v3bool %180 %185
%196 = OpAll %bool %195
%197 = OpLogicalAnd %bool %194 %196
OpBranch %183
%183 = OpLabel
%198 = OpPhi %bool %false %144 %197 %182
OpStore %ok %198
OpStore %m_2 %213
%219 = OpFSub %v2float %209 %214
%220 = OpFSub %v2float %210 %215
%221 = OpFSub %v2float %211 %216
%222 = OpFSub %v2float %212 %217
%223 = OpCompositeConstruct %mat4v2float %219 %220 %221 %222
OpStore %m_2 %223
OpSelectionMerge %225 None
OpBranchConditional %198 %224 %225
%224 = OpLabel
%238 = OpFOrdEqual %v2bool %219 %233
%239 = OpAll %bool %238
%240 = OpFOrdEqual %v2bool %220 %234
%241 = OpAll %bool %240
%242 = OpLogicalAnd %bool %239 %241
%243 = OpFOrdEqual %v2bool %221 %235
%244 = OpAll %bool %243
%245 = OpLogicalAnd %bool %242 %244
%246 = OpFOrdEqual %v2bool %222 %236
%247 = OpAll %bool %246
%248 = OpLogicalAnd %bool %245 %247
OpBranch %225
%225 = OpLabel
%249 = OpPhi %bool %false %183 %248 %224
OpStore %ok %249
OpStore %m_3 %254
%258 = OpFDiv %v4float %253 %255
%259 = OpFDiv %v4float %253 %256
%260 = OpCompositeConstruct %mat2v4float %258 %259
OpStore %m_3 %260
OpSelectionMerge %262 None
OpBranchConditional %249 %261 %262
%261 = OpLabel
%267 = OpFOrdEqual %v4bool %258 %263
%268 = OpAll %bool %267
%269 = OpFOrdEqual %v4bool %259 %264
%270 = OpAll %bool %269
%271 = OpLogicalAnd %bool %268 %270
OpBranch %262
%262 = OpLabel
%272 = OpPhi %bool %false %225 %271 %261
OpStore %ok %272
OpStore %m_4 %276
%281 = OpMatrixTimesMatrix %mat2v3float %276 %280
OpStore %m_4 %281
OpSelectionMerge %283 None
OpBranchConditional %272 %282 %283
%282 = OpLabel
%292 = OpCompositeExtract %v3float %281 0
%293 = OpFOrdEqual %v3bool %292 %289
%294 = OpAll %bool %293
%295 = OpCompositeExtract %v3float %281 1
%296 = OpFOrdEqual %v3bool %295 %290
%297 = OpAll %bool %296
%298 = OpLogicalAnd %bool %294 %297
OpBranch %283
%283 = OpLabel
%299 = OpPhi %bool %false %262 %298 %282
OpStore %ok %299
OpReturnValue %299
OpFunctionEnd
%main = OpFunction %v4float None %300
%301 = OpFunctionParameter %_ptr_Function_v2float
%302 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_2_m = OpVariable %_ptr_Function_mat3v2float Function
%_4_m = OpVariable %_ptr_Function_mat2v3float Function
%_5_m = OpVariable %_ptr_Function_mat4v3float Function
%_6_m = OpVariable %_ptr_Function_mat4v2float Function
%_7_m = OpVariable %_ptr_Function_mat2v4float Function
%_8_m = OpVariable %_ptr_Function_mat2v3float Function
%452 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
OpStore %_2_m %35
%305 = OpFAdd %v2float %33 %37
%306 = OpFAdd %v2float %34 %37
%307 = OpFAdd %v2float %20 %37
%308 = OpCompositeConstruct %mat3v2float %305 %306 %307
OpStore %_2_m %308
OpSelectionMerge %310 None
OpBranchConditional %true %309 %310
%309 = OpLabel
%311 = OpFOrdEqual %v2bool %305 %47
%312 = OpAll %bool %311
%313 = OpFOrdEqual %v2bool %306 %48
%314 = OpAll %bool %313
%315 = OpLogicalAnd %bool %312 %314
%316 = OpFOrdEqual %v2bool %307 %37
%317 = OpAll %bool %316
%318 = OpLogicalAnd %bool %315 %317
OpBranch %310
%310 = OpLabel
%319 = OpPhi %bool %false %302 %318 %309
OpStore %_0_ok %319
OpStore %_2_m %35
%320 = OpFSub %v2float %33 %37
%321 = OpFSub %v2float %34 %37
%322 = OpFSub %v2float %20 %37
%323 = OpCompositeConstruct %mat3v2float %320 %321 %322
OpStore %_2_m %323
OpSelectionMerge %325 None
OpBranchConditional %319 %324 %325
%324 = OpLabel
%326 = OpFOrdEqual %v2bool %320 %68
%327 = OpAll %bool %326
%328 = OpFOrdEqual %v2bool %321 %69
%329 = OpAll %bool %328
%330 = OpLogicalAnd %bool %327 %329
%331 = OpFOrdEqual %v2bool %322 %70
%332 = OpAll %bool %331
%333 = OpLogicalAnd %bool %330 %332
OpBranch %325
%325 = OpLabel
%334 = OpPhi %bool %false %310 %333 %324
OpStore %_0_ok %334
OpStore %_2_m %35
%335 = OpFDiv %v2float %33 %37
%336 = OpFDiv %v2float %34 %37
%337 = OpFDiv %v2float %20 %37
%338 = OpCompositeConstruct %mat3v2float %335 %336 %337
OpStore %_2_m %338
OpSelectionMerge %340 None
OpBranchConditional %334 %339 %340
%339 = OpLabel
%341 = OpFOrdEqual %v2bool %335 %88
%342 = OpAll %bool %341
%343 = OpFOrdEqual %v2bool %336 %89
%344 = OpAll %bool %343
%345 = OpLogicalAnd %bool %342 %344
%346 = OpFOrdEqual %v2bool %337 %20
%347 = OpAll %bool %346
%348 = OpLogicalAnd %bool %345 %347
OpBranch %340
%340 = OpLabel
%349 = OpPhi %bool %false %325 %348 %339
OpStore %_0_ok %349
OpStore %_4_m %105
%351 = OpFAdd %v3float %104 %106
%352 = OpFAdd %v3float %104 %107
%353 = OpCompositeConstruct %mat2v3float %351 %352
OpStore %_4_m %353
OpSelectionMerge %355 None
OpBranchConditional %349 %354 %355
%354 = OpLabel
%356 = OpFOrdEqual %v3bool %351 %114
%357 = OpAll %bool %356
%358 = OpFOrdEqual %v3bool %352 %115
%359 = OpAll %bool %358
%360 = OpLogicalAnd %bool %357 %359
OpBranch %355
%355 = OpLabel
%361 = OpPhi %bool %false %340 %360 %354
OpStore %_0_ok %361
OpStore %_4_m %105
%362 = OpFSub %v3float %104 %106
%363 = OpFSub %v3float %104 %107
%364 = OpCompositeConstruct %mat2v3float %362 %363
OpStore %_4_m %364
OpSelectionMerge %366 None
OpBranchConditional %361 %365 %366
%365 = OpLabel
%367 = OpFOrdEqual %v3bool %362 %129
%368 = OpAll %bool %367
%369 = OpFOrdEqual %v3bool %363 %130
%370 = OpAll %bool %369
%371 = OpLogicalAnd %bool %368 %370
OpBranch %366
%366 = OpLabel
%372 = OpPhi %bool %false %355 %371 %365
OpStore %_0_ok %372
OpStore %_4_m %105
%373 = OpFDiv %v3float %104 %138
%374 = OpFDiv %v3float %104 %138
%375 = OpCompositeConstruct %mat2v3float %373 %374
OpStore %_4_m %375
OpSelectionMerge %377 None
OpBranchConditional %372 %376 %377
%376 = OpLabel
%378 = OpFOrdEqual %v3bool %373 %138
%379 = OpAll %bool %378
%380 = OpFOrdEqual %v3bool %374 %138
%381 = OpAll %bool %380
%382 = OpLogicalAnd %bool %379 %381
OpBranch %377
%377 = OpLabel
%383 = OpPhi %bool %false %366 %382 %376
OpStore %_0_ok %383
OpStore %_5_m %167
%385 = OpFAdd %v3float %163 %172
%386 = OpFAdd %v3float %164 %173
%387 = OpFAdd %v3float %165 %174
%388 = OpFAdd %v3float %166 %175
%389 = OpCompositeConstruct %mat4v3float %385 %386 %387 %388
OpStore %_5_m %389
OpSelectionMerge %391 None
OpBranchConditional %383 %390 %391
%390 = OpLabel
%392 = OpFOrdEqual %v3bool %385 %185
%393 = OpAll %bool %392
%394 = OpFOrdEqual %v3bool %386 %185
%395 = OpAll %bool %394
%396 = OpLogicalAnd %bool %393 %395
%397 = OpFOrdEqual %v3bool %387 %185
%398 = OpAll %bool %397
%399 = OpLogicalAnd %bool %396 %398
%400 = OpFOrdEqual %v3bool %388 %185
%401 = OpAll %bool %400
%402 = OpLogicalAnd %bool %399 %401
OpBranch %391
%391 = OpLabel
%403 = OpPhi %bool %false %377 %402 %390
OpStore %_0_ok %403
OpStore %_6_m %213
%405 = OpFSub %v2float %209 %214
%406 = OpFSub %v2float %210 %215
%407 = OpFSub %v2float %211 %216
%408 = OpFSub %v2float %212 %217
%409 = OpCompositeConstruct %mat4v2float %405 %406 %407 %408
OpStore %_6_m %409
OpSelectionMerge %411 None
OpBranchConditional %403 %410 %411
%410 = OpLabel
%412 = OpFOrdEqual %v2bool %405 %233
%413 = OpAll %bool %412
%414 = OpFOrdEqual %v2bool %406 %234
%415 = OpAll %bool %414
%416 = OpLogicalAnd %bool %413 %415
%417 = OpFOrdEqual %v2bool %407 %235
%418 = OpAll %bool %417
%419 = OpLogicalAnd %bool %416 %418
%420 = OpFOrdEqual %v2bool %408 %236
%421 = OpAll %bool %420
%422 = OpLogicalAnd %bool %419 %421
OpBranch %411
%411 = OpLabel
%423 = OpPhi %bool %false %391 %422 %410
OpStore %_0_ok %423
OpStore %_7_m %254
%425 = OpFDiv %v4float %253 %255
%426 = OpFDiv %v4float %253 %256
%427 = OpCompositeConstruct %mat2v4float %425 %426
OpStore %_7_m %427
OpSelectionMerge %429 None
OpBranchConditional %423 %428 %429
%428 = OpLabel
%430 = OpFOrdEqual %v4bool %425 %263
%431 = OpAll %bool %430
%432 = OpFOrdEqual %v4bool %426 %264
%433 = OpAll %bool %432
%434 = OpLogicalAnd %bool %431 %433
OpBranch %429
%429 = OpLabel
%435 = OpPhi %bool %false %411 %434 %428
OpStore %_0_ok %435
OpStore %_8_m %276
%437 = OpMatrixTimesMatrix %mat2v3float %276 %280
OpStore %_8_m %437
OpSelectionMerge %439 None
OpBranchConditional %435 %438 %439
%438 = OpLabel
%440 = OpCompositeExtract %v3float %437 0
%441 = OpFOrdEqual %v3bool %440 %289
%442 = OpAll %bool %441
%443 = OpCompositeExtract %v3float %437 1
%444 = OpFOrdEqual %v3bool %443 %290
%445 = OpAll %bool %444
%446 = OpLogicalAnd %bool %442 %445
OpBranch %439
%439 = OpLabel
%447 = OpPhi %bool %false %429 %446 %438
OpStore %_0_ok %447
OpSelectionMerge %449 None
OpBranchConditional %447 %448 %449
%448 = OpLabel
%450 = OpFunctionCall %bool %test_matrix_op_matrix_half_b
OpBranch %449
%449 = OpLabel
%451 = OpPhi %bool %false %439 %450 %448
OpSelectionMerge %456 None
OpBranchConditional %451 %454 %455
%454 = OpLabel
%457 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%461 = OpLoad %v4float %457
OpStore %452 %461
OpBranch %456
%455 = OpLabel
%462 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%464 = OpLoad %v4float %462
OpStore %452 %464
OpBranch %456
%456 = OpLabel
%465 = OpLoad %v4float %452
OpReturnValue %465
OpFunctionEnd
