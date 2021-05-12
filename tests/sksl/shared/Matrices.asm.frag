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
OpName %_6_m4 "_6_m4"
OpName %_7_m5 "_7_m5"
OpName %_8_m7 "_8_m7"
OpName %_9_m9 "_9_m9"
OpName %_10_m10 "_10_m10"
OpName %_11_m11 "_11_m11"
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
OpDecorate %148 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %398 RelaxedPrecision
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
%float_12 = OpConstant %float 12
%float_18 = OpConstant %float 18
%float_24 = OpConstant %float 24
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%float_8 = OpConstant %float 8
%float_7 = OpConstant %float 7
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_11 = OpConstant %float 11
%v4bool = OpTypeVector %bool 4
%float_20 = OpConstant %float 20
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
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
%_6_m4 = OpVariable %_ptr_Function_mat2v2float Function
%_7_m5 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m7 = OpVariable %_ptr_Function_mat2v2float Function
%_9_m9 = OpVariable %_ptr_Function_mat3v3float Function
%_10_m10 = OpVariable %_ptr_Function_mat4v4float Function
%_11_m11 = OpVariable %_ptr_Function_mat4v4float Function
%387 = OpVariable %_ptr_Function_v4float Function
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
%146 = OpCompositeConstruct %v2float %float_6 %float_0
%147 = OpCompositeConstruct %v2float %float_0 %float_6
%145 = OpCompositeConstruct %mat2v2float %146 %147
OpStore %_6_m4 %145
%148 = OpLoad %bool %_0_ok
OpSelectionMerge %150 None
OpBranchConditional %148 %149 %150
%149 = OpLabel
%151 = OpLoad %mat2v2float %_6_m4
%153 = OpCompositeConstruct %v2float %float_6 %float_0
%154 = OpCompositeConstruct %v2float %float_0 %float_6
%152 = OpCompositeConstruct %mat2v2float %153 %154
%155 = OpCompositeExtract %v2float %151 0
%156 = OpCompositeExtract %v2float %152 0
%157 = OpFOrdEqual %v2bool %155 %156
%158 = OpAll %bool %157
%159 = OpCompositeExtract %v2float %151 1
%160 = OpCompositeExtract %v2float %152 1
%161 = OpFOrdEqual %v2bool %159 %160
%162 = OpAll %bool %161
%163 = OpLogicalAnd %bool %158 %162
OpBranch %150
%150 = OpLabel
%164 = OpPhi %bool %false %129 %163 %149
OpStore %_0_ok %164
%165 = OpLoad %mat2v2float %_5_m3
%166 = OpLoad %mat2v2float %_6_m4
%167 = OpMatrixTimesMatrix %mat2v2float %165 %166
OpStore %_5_m3 %167
%168 = OpLoad %bool %_0_ok
OpSelectionMerge %170 None
OpBranchConditional %168 %169 %170
%169 = OpLabel
%171 = OpLoad %mat2v2float %_5_m3
%176 = OpCompositeConstruct %v2float %float_6 %float_12
%177 = OpCompositeConstruct %v2float %float_18 %float_24
%175 = OpCompositeConstruct %mat2v2float %176 %177
%178 = OpCompositeExtract %v2float %171 0
%179 = OpCompositeExtract %v2float %175 0
%180 = OpFOrdEqual %v2bool %178 %179
%181 = OpAll %bool %180
%182 = OpCompositeExtract %v2float %171 1
%183 = OpCompositeExtract %v2float %175 1
%184 = OpFOrdEqual %v2bool %182 %183
%185 = OpAll %bool %184
%186 = OpLogicalAnd %bool %181 %185
OpBranch %170
%170 = OpLabel
%187 = OpPhi %bool %false %150 %186 %169
OpStore %_0_ok %187
%191 = OpAccessChain %_ptr_Function_v2float %_3_m1 %int_1
%192 = OpLoad %v2float %191
%193 = OpCompositeExtract %float %192 1
%195 = OpCompositeConstruct %v2float %193 %float_0
%196 = OpCompositeConstruct %v2float %float_0 %193
%194 = OpCompositeConstruct %mat2v2float %195 %196
OpStore %_7_m5 %194
%197 = OpLoad %bool %_0_ok
OpSelectionMerge %199 None
OpBranchConditional %197 %198 %199
%198 = OpLabel
%200 = OpLoad %mat2v2float %_7_m5
%202 = OpCompositeConstruct %v2float %float_4 %float_0
%203 = OpCompositeConstruct %v2float %float_0 %float_4
%201 = OpCompositeConstruct %mat2v2float %202 %203
%204 = OpCompositeExtract %v2float %200 0
%205 = OpCompositeExtract %v2float %201 0
%206 = OpFOrdEqual %v2bool %204 %205
%207 = OpAll %bool %206
%208 = OpCompositeExtract %v2float %200 1
%209 = OpCompositeExtract %v2float %201 1
%210 = OpFOrdEqual %v2bool %208 %209
%211 = OpAll %bool %210
%212 = OpLogicalAnd %bool %207 %211
OpBranch %199
%199 = OpLabel
%213 = OpPhi %bool %false %170 %212 %198
OpStore %_0_ok %213
%214 = OpLoad %mat2v2float %_3_m1
%215 = OpLoad %mat2v2float %_7_m5
%216 = OpCompositeExtract %v2float %214 0
%217 = OpCompositeExtract %v2float %215 0
%218 = OpFAdd %v2float %216 %217
%219 = OpCompositeExtract %v2float %214 1
%220 = OpCompositeExtract %v2float %215 1
%221 = OpFAdd %v2float %219 %220
%222 = OpCompositeConstruct %mat2v2float %218 %221
OpStore %_3_m1 %222
%223 = OpLoad %bool %_0_ok
OpSelectionMerge %225 None
OpBranchConditional %223 %224 %225
%224 = OpLabel
%226 = OpLoad %mat2v2float %_3_m1
%229 = OpCompositeConstruct %v2float %float_5 %float_2
%230 = OpCompositeConstruct %v2float %float_3 %float_8
%228 = OpCompositeConstruct %mat2v2float %229 %230
%231 = OpCompositeExtract %v2float %226 0
%232 = OpCompositeExtract %v2float %228 0
%233 = OpFOrdEqual %v2bool %231 %232
%234 = OpAll %bool %233
%235 = OpCompositeExtract %v2float %226 1
%236 = OpCompositeExtract %v2float %228 1
%237 = OpFOrdEqual %v2bool %235 %236
%238 = OpAll %bool %237
%239 = OpLogicalAnd %bool %234 %238
OpBranch %225
%225 = OpLabel
%240 = OpPhi %bool %false %199 %239 %224
OpStore %_0_ok %240
%244 = OpCompositeConstruct %v2float %float_5 %float_6
%245 = OpCompositeConstruct %v2float %float_7 %float_8
%243 = OpCompositeConstruct %mat2v2float %244 %245
OpStore %_8_m7 %243
%246 = OpLoad %bool %_0_ok
OpSelectionMerge %248 None
OpBranchConditional %246 %247 %248
%247 = OpLabel
%249 = OpLoad %mat2v2float %_8_m7
%251 = OpCompositeConstruct %v2float %float_5 %float_6
%252 = OpCompositeConstruct %v2float %float_7 %float_8
%250 = OpCompositeConstruct %mat2v2float %251 %252
%253 = OpCompositeExtract %v2float %249 0
%254 = OpCompositeExtract %v2float %250 0
%255 = OpFOrdEqual %v2bool %253 %254
%256 = OpAll %bool %255
%257 = OpCompositeExtract %v2float %249 1
%258 = OpCompositeExtract %v2float %250 1
%259 = OpFOrdEqual %v2bool %257 %258
%260 = OpAll %bool %259
%261 = OpLogicalAnd %bool %256 %260
OpBranch %248
%248 = OpLabel
%262 = OpPhi %bool %false %225 %261 %247
OpStore %_0_ok %262
%266 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%267 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%268 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%265 = OpCompositeConstruct %mat3v3float %266 %267 %268
OpStore %_9_m9 %265
%269 = OpLoad %bool %_0_ok
OpSelectionMerge %271 None
OpBranchConditional %269 %270 %271
%270 = OpLabel
%272 = OpLoad %mat3v3float %_9_m9
%274 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%275 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%276 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%273 = OpCompositeConstruct %mat3v3float %274 %275 %276
%277 = OpCompositeExtract %v3float %272 0
%278 = OpCompositeExtract %v3float %273 0
%279 = OpFOrdEqual %v3bool %277 %278
%280 = OpAll %bool %279
%281 = OpCompositeExtract %v3float %272 1
%282 = OpCompositeExtract %v3float %273 1
%283 = OpFOrdEqual %v3bool %281 %282
%284 = OpAll %bool %283
%285 = OpLogicalAnd %bool %280 %284
%286 = OpCompositeExtract %v3float %272 2
%287 = OpCompositeExtract %v3float %273 2
%288 = OpFOrdEqual %v3bool %286 %287
%289 = OpAll %bool %288
%290 = OpLogicalAnd %bool %285 %289
OpBranch %271
%271 = OpLabel
%291 = OpPhi %bool %false %248 %290 %270
OpStore %_0_ok %291
%297 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%298 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%299 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%300 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%296 = OpCompositeConstruct %mat4v4float %297 %298 %299 %300
OpStore %_10_m10 %296
%301 = OpLoad %bool %_0_ok
OpSelectionMerge %303 None
OpBranchConditional %301 %302 %303
%302 = OpLabel
%304 = OpLoad %mat4v4float %_10_m10
%306 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%307 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%308 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%309 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%305 = OpCompositeConstruct %mat4v4float %306 %307 %308 %309
%311 = OpCompositeExtract %v4float %304 0
%312 = OpCompositeExtract %v4float %305 0
%313 = OpFOrdEqual %v4bool %311 %312
%314 = OpAll %bool %313
%315 = OpCompositeExtract %v4float %304 1
%316 = OpCompositeExtract %v4float %305 1
%317 = OpFOrdEqual %v4bool %315 %316
%318 = OpAll %bool %317
%319 = OpLogicalAnd %bool %314 %318
%320 = OpCompositeExtract %v4float %304 2
%321 = OpCompositeExtract %v4float %305 2
%322 = OpFOrdEqual %v4bool %320 %321
%323 = OpAll %bool %322
%324 = OpLogicalAnd %bool %319 %323
%325 = OpCompositeExtract %v4float %304 3
%326 = OpCompositeExtract %v4float %305 3
%327 = OpFOrdEqual %v4bool %325 %326
%328 = OpAll %bool %327
%329 = OpLogicalAnd %bool %324 %328
OpBranch %303
%303 = OpLabel
%330 = OpPhi %bool %false %271 %329 %302
OpStore %_0_ok %330
%334 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%335 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%336 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%337 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%333 = OpCompositeConstruct %mat4v4float %334 %335 %336 %337
OpStore %_11_m11 %333
%338 = OpLoad %mat4v4float %_11_m11
%339 = OpLoad %mat4v4float %_10_m10
%340 = OpCompositeExtract %v4float %338 0
%341 = OpCompositeExtract %v4float %339 0
%342 = OpFSub %v4float %340 %341
%343 = OpCompositeExtract %v4float %338 1
%344 = OpCompositeExtract %v4float %339 1
%345 = OpFSub %v4float %343 %344
%346 = OpCompositeExtract %v4float %338 2
%347 = OpCompositeExtract %v4float %339 2
%348 = OpFSub %v4float %346 %347
%349 = OpCompositeExtract %v4float %338 3
%350 = OpCompositeExtract %v4float %339 3
%351 = OpFSub %v4float %349 %350
%352 = OpCompositeConstruct %mat4v4float %342 %345 %348 %351
OpStore %_11_m11 %352
%353 = OpLoad %bool %_0_ok
OpSelectionMerge %355 None
OpBranchConditional %353 %354 %355
%354 = OpLabel
%356 = OpLoad %mat4v4float %_11_m11
%358 = OpCompositeConstruct %v4float %float_9 %float_20 %float_20 %float_20
%359 = OpCompositeConstruct %v4float %float_20 %float_9 %float_20 %float_20
%360 = OpCompositeConstruct %v4float %float_20 %float_20 %float_9 %float_20
%361 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_9
%357 = OpCompositeConstruct %mat4v4float %358 %359 %360 %361
%362 = OpCompositeExtract %v4float %356 0
%363 = OpCompositeExtract %v4float %357 0
%364 = OpFOrdEqual %v4bool %362 %363
%365 = OpAll %bool %364
%366 = OpCompositeExtract %v4float %356 1
%367 = OpCompositeExtract %v4float %357 1
%368 = OpFOrdEqual %v4bool %366 %367
%369 = OpAll %bool %368
%370 = OpLogicalAnd %bool %365 %369
%371 = OpCompositeExtract %v4float %356 2
%372 = OpCompositeExtract %v4float %357 2
%373 = OpFOrdEqual %v4bool %371 %372
%374 = OpAll %bool %373
%375 = OpLogicalAnd %bool %370 %374
%376 = OpCompositeExtract %v4float %356 3
%377 = OpCompositeExtract %v4float %357 3
%378 = OpFOrdEqual %v4bool %376 %377
%379 = OpAll %bool %378
%380 = OpLogicalAnd %bool %375 %379
OpBranch %355
%355 = OpLabel
%381 = OpPhi %bool %false %303 %380 %354
OpStore %_0_ok %381
%382 = OpLoad %bool %_0_ok
OpSelectionMerge %384 None
OpBranchConditional %382 %383 %384
%383 = OpLabel
%385 = OpFunctionCall %bool %test_half_b
OpBranch %384
%384 = OpLabel
%386 = OpPhi %bool %false %355 %385 %383
OpSelectionMerge %391 None
OpBranchConditional %386 %389 %390
%389 = OpLabel
%392 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%395 = OpLoad %v4float %392
OpStore %387 %395
OpBranch %391
%390 = OpLabel
%396 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%397 = OpLoad %v4float %396
OpStore %387 %397
OpBranch %391
%391 = OpLabel
%398 = OpLoad %v4float %387
OpReturnValue %398
OpFunctionEnd
