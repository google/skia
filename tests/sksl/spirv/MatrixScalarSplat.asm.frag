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
OpName %four "four"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_four "_1_four"
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
OpDecorate %33 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %408 RelaxedPrecision
OpDecorate %436 RelaxedPrecision
OpDecorate %475 RelaxedPrecision
OpDecorate %514 RelaxedPrecision
OpDecorate %553 RelaxedPrecision
OpDecorate %581 RelaxedPrecision
OpDecorate %610 RelaxedPrecision
OpDecorate %624 RelaxedPrecision
OpDecorate %627 RelaxedPrecision
OpDecorate %628 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
%float_4 = OpConstant %float 4
%false = OpConstantFalse %bool
%float_2 = OpConstant %float 2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%float_6 = OpConstant %float 6
%v3bool = OpTypeVector %bool 3
%float_n2 = OpConstant %float -2
%float_n4 = OpConstant %float -4
%float_8 = OpConstant %float 8
%float_0_5 = OpConstant %float 0.5
%mat2v2float = OpTypeMatrix %v2float 2
%v2bool = OpTypeVector %bool 2
%325 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%four = OpVariable %_ptr_Function_float Function
OpStore %ok %true
OpStore %four %float_4
%33 = OpLoad %bool %ok
OpSelectionMerge %35 None
OpBranchConditional %33 %34 %35
%34 = OpLabel
%39 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%40 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%41 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%37 = OpCompositeConstruct %mat3v3float %39 %40 %41
%43 = OpLoad %float %four
%44 = OpCompositeConstruct %v3float %43 %43 %43
%45 = OpCompositeConstruct %mat3v3float %44 %44 %44
%46 = OpCompositeExtract %v3float %37 0
%47 = OpCompositeExtract %v3float %45 0
%48 = OpFAdd %v3float %46 %47
%49 = OpCompositeExtract %v3float %37 1
%50 = OpCompositeExtract %v3float %45 1
%51 = OpFAdd %v3float %49 %50
%52 = OpCompositeExtract %v3float %37 2
%53 = OpCompositeExtract %v3float %45 2
%54 = OpFAdd %v3float %52 %53
%55 = OpCompositeConstruct %mat3v3float %48 %51 %54
%58 = OpCompositeConstruct %v3float %float_6 %float_4 %float_4
%59 = OpCompositeConstruct %v3float %float_4 %float_6 %float_4
%60 = OpCompositeConstruct %v3float %float_4 %float_4 %float_6
%57 = OpCompositeConstruct %mat3v3float %58 %59 %60
%62 = OpCompositeExtract %v3float %55 0
%63 = OpCompositeExtract %v3float %57 0
%64 = OpFOrdEqual %v3bool %62 %63
%65 = OpAll %bool %64
%66 = OpCompositeExtract %v3float %55 1
%67 = OpCompositeExtract %v3float %57 1
%68 = OpFOrdEqual %v3bool %66 %67
%69 = OpAll %bool %68
%70 = OpLogicalAnd %bool %65 %69
%71 = OpCompositeExtract %v3float %55 2
%72 = OpCompositeExtract %v3float %57 2
%73 = OpFOrdEqual %v3bool %71 %72
%74 = OpAll %bool %73
%75 = OpLogicalAnd %bool %70 %74
OpBranch %35
%35 = OpLabel
%76 = OpPhi %bool %false %25 %75 %34
OpStore %ok %76
%77 = OpLoad %bool %ok
OpSelectionMerge %79 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
%81 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%82 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%83 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%80 = OpCompositeConstruct %mat3v3float %81 %82 %83
%84 = OpLoad %float %four
%85 = OpCompositeConstruct %v3float %84 %84 %84
%86 = OpCompositeConstruct %mat3v3float %85 %85 %85
%87 = OpCompositeExtract %v3float %80 0
%88 = OpCompositeExtract %v3float %86 0
%89 = OpFSub %v3float %87 %88
%90 = OpCompositeExtract %v3float %80 1
%91 = OpCompositeExtract %v3float %86 1
%92 = OpFSub %v3float %90 %91
%93 = OpCompositeExtract %v3float %80 2
%94 = OpCompositeExtract %v3float %86 2
%95 = OpFSub %v3float %93 %94
%96 = OpCompositeConstruct %mat3v3float %89 %92 %95
%100 = OpCompositeConstruct %v3float %float_n2 %float_n4 %float_n4
%101 = OpCompositeConstruct %v3float %float_n4 %float_n2 %float_n4
%102 = OpCompositeConstruct %v3float %float_n4 %float_n4 %float_n2
%99 = OpCompositeConstruct %mat3v3float %100 %101 %102
%103 = OpCompositeExtract %v3float %96 0
%104 = OpCompositeExtract %v3float %99 0
%105 = OpFOrdEqual %v3bool %103 %104
%106 = OpAll %bool %105
%107 = OpCompositeExtract %v3float %96 1
%108 = OpCompositeExtract %v3float %99 1
%109 = OpFOrdEqual %v3bool %107 %108
%110 = OpAll %bool %109
%111 = OpLogicalAnd %bool %106 %110
%112 = OpCompositeExtract %v3float %96 2
%113 = OpCompositeExtract %v3float %99 2
%114 = OpFOrdEqual %v3bool %112 %113
%115 = OpAll %bool %114
%116 = OpLogicalAnd %bool %111 %115
OpBranch %79
%79 = OpLabel
%117 = OpPhi %bool %false %35 %116 %78
OpStore %ok %117
%118 = OpLoad %bool %ok
OpSelectionMerge %120 None
OpBranchConditional %118 %119 %120
%119 = OpLabel
%122 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%123 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%124 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%121 = OpCompositeConstruct %mat3v3float %122 %123 %124
%125 = OpLoad %float %four
%126 = OpMatrixTimesScalar %mat3v3float %121 %125
%129 = OpCompositeConstruct %v3float %float_8 %float_0 %float_0
%130 = OpCompositeConstruct %v3float %float_0 %float_8 %float_0
%131 = OpCompositeConstruct %v3float %float_0 %float_0 %float_8
%128 = OpCompositeConstruct %mat3v3float %129 %130 %131
%132 = OpCompositeExtract %v3float %126 0
%133 = OpCompositeExtract %v3float %128 0
%134 = OpFOrdEqual %v3bool %132 %133
%135 = OpAll %bool %134
%136 = OpCompositeExtract %v3float %126 1
%137 = OpCompositeExtract %v3float %128 1
%138 = OpFOrdEqual %v3bool %136 %137
%139 = OpAll %bool %138
%140 = OpLogicalAnd %bool %135 %139
%141 = OpCompositeExtract %v3float %126 2
%142 = OpCompositeExtract %v3float %128 2
%143 = OpFOrdEqual %v3bool %141 %142
%144 = OpAll %bool %143
%145 = OpLogicalAnd %bool %140 %144
OpBranch %120
%120 = OpLabel
%146 = OpPhi %bool %false %79 %145 %119
OpStore %ok %146
%147 = OpLoad %bool %ok
OpSelectionMerge %149 None
OpBranchConditional %147 %148 %149
%148 = OpLabel
%151 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%152 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%153 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%150 = OpCompositeConstruct %mat3v3float %151 %152 %153
%154 = OpLoad %float %four
%155 = OpCompositeConstruct %v3float %154 %154 %154
%156 = OpCompositeConstruct %mat3v3float %155 %155 %155
%157 = OpCompositeExtract %v3float %150 0
%158 = OpCompositeExtract %v3float %156 0
%159 = OpFDiv %v3float %157 %158
%160 = OpCompositeExtract %v3float %150 1
%161 = OpCompositeExtract %v3float %156 1
%162 = OpFDiv %v3float %160 %161
%163 = OpCompositeExtract %v3float %150 2
%164 = OpCompositeExtract %v3float %156 2
%165 = OpFDiv %v3float %163 %164
%166 = OpCompositeConstruct %mat3v3float %159 %162 %165
%169 = OpCompositeConstruct %v3float %float_0_5 %float_0 %float_0
%170 = OpCompositeConstruct %v3float %float_0 %float_0_5 %float_0
%171 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0_5
%168 = OpCompositeConstruct %mat3v3float %169 %170 %171
%172 = OpCompositeExtract %v3float %166 0
%173 = OpCompositeExtract %v3float %168 0
%174 = OpFOrdEqual %v3bool %172 %173
%175 = OpAll %bool %174
%176 = OpCompositeExtract %v3float %166 1
%177 = OpCompositeExtract %v3float %168 1
%178 = OpFOrdEqual %v3bool %176 %177
%179 = OpAll %bool %178
%180 = OpLogicalAnd %bool %175 %179
%181 = OpCompositeExtract %v3float %166 2
%182 = OpCompositeExtract %v3float %168 2
%183 = OpFOrdEqual %v3bool %181 %182
%184 = OpAll %bool %183
%185 = OpLogicalAnd %bool %180 %184
OpBranch %149
%149 = OpLabel
%186 = OpPhi %bool %false %120 %185 %148
OpStore %ok %186
%187 = OpLoad %bool %ok
OpSelectionMerge %189 None
OpBranchConditional %187 %188 %189
%188 = OpLabel
%190 = OpLoad %float %four
%192 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%193 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%194 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%191 = OpCompositeConstruct %mat3v3float %192 %193 %194
%195 = OpCompositeConstruct %v3float %190 %190 %190
%196 = OpCompositeConstruct %mat3v3float %195 %195 %195
%197 = OpCompositeExtract %v3float %196 0
%198 = OpCompositeExtract %v3float %191 0
%199 = OpFAdd %v3float %197 %198
%200 = OpCompositeExtract %v3float %196 1
%201 = OpCompositeExtract %v3float %191 1
%202 = OpFAdd %v3float %200 %201
%203 = OpCompositeExtract %v3float %196 2
%204 = OpCompositeExtract %v3float %191 2
%205 = OpFAdd %v3float %203 %204
%206 = OpCompositeConstruct %mat3v3float %199 %202 %205
%208 = OpCompositeConstruct %v3float %float_6 %float_4 %float_4
%209 = OpCompositeConstruct %v3float %float_4 %float_6 %float_4
%210 = OpCompositeConstruct %v3float %float_4 %float_4 %float_6
%207 = OpCompositeConstruct %mat3v3float %208 %209 %210
%211 = OpCompositeExtract %v3float %206 0
%212 = OpCompositeExtract %v3float %207 0
%213 = OpFOrdEqual %v3bool %211 %212
%214 = OpAll %bool %213
%215 = OpCompositeExtract %v3float %206 1
%216 = OpCompositeExtract %v3float %207 1
%217 = OpFOrdEqual %v3bool %215 %216
%218 = OpAll %bool %217
%219 = OpLogicalAnd %bool %214 %218
%220 = OpCompositeExtract %v3float %206 2
%221 = OpCompositeExtract %v3float %207 2
%222 = OpFOrdEqual %v3bool %220 %221
%223 = OpAll %bool %222
%224 = OpLogicalAnd %bool %219 %223
OpBranch %189
%189 = OpLabel
%225 = OpPhi %bool %false %149 %224 %188
OpStore %ok %225
%226 = OpLoad %bool %ok
OpSelectionMerge %228 None
OpBranchConditional %226 %227 %228
%227 = OpLabel
%229 = OpLoad %float %four
%231 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%232 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%233 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%230 = OpCompositeConstruct %mat3v3float %231 %232 %233
%234 = OpCompositeConstruct %v3float %229 %229 %229
%235 = OpCompositeConstruct %mat3v3float %234 %234 %234
%236 = OpCompositeExtract %v3float %235 0
%237 = OpCompositeExtract %v3float %230 0
%238 = OpFSub %v3float %236 %237
%239 = OpCompositeExtract %v3float %235 1
%240 = OpCompositeExtract %v3float %230 1
%241 = OpFSub %v3float %239 %240
%242 = OpCompositeExtract %v3float %235 2
%243 = OpCompositeExtract %v3float %230 2
%244 = OpFSub %v3float %242 %243
%245 = OpCompositeConstruct %mat3v3float %238 %241 %244
%247 = OpCompositeConstruct %v3float %float_2 %float_4 %float_4
%248 = OpCompositeConstruct %v3float %float_4 %float_2 %float_4
%249 = OpCompositeConstruct %v3float %float_4 %float_4 %float_2
%246 = OpCompositeConstruct %mat3v3float %247 %248 %249
%250 = OpCompositeExtract %v3float %245 0
%251 = OpCompositeExtract %v3float %246 0
%252 = OpFOrdEqual %v3bool %250 %251
%253 = OpAll %bool %252
%254 = OpCompositeExtract %v3float %245 1
%255 = OpCompositeExtract %v3float %246 1
%256 = OpFOrdEqual %v3bool %254 %255
%257 = OpAll %bool %256
%258 = OpLogicalAnd %bool %253 %257
%259 = OpCompositeExtract %v3float %245 2
%260 = OpCompositeExtract %v3float %246 2
%261 = OpFOrdEqual %v3bool %259 %260
%262 = OpAll %bool %261
%263 = OpLogicalAnd %bool %258 %262
OpBranch %228
%228 = OpLabel
%264 = OpPhi %bool %false %189 %263 %227
OpStore %ok %264
%265 = OpLoad %bool %ok
OpSelectionMerge %267 None
OpBranchConditional %265 %266 %267
%266 = OpLabel
%268 = OpLoad %float %four
%270 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%271 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%272 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%269 = OpCompositeConstruct %mat3v3float %270 %271 %272
%273 = OpMatrixTimesScalar %mat3v3float %269 %268
%275 = OpCompositeConstruct %v3float %float_8 %float_0 %float_0
%276 = OpCompositeConstruct %v3float %float_0 %float_8 %float_0
%277 = OpCompositeConstruct %v3float %float_0 %float_0 %float_8
%274 = OpCompositeConstruct %mat3v3float %275 %276 %277
%278 = OpCompositeExtract %v3float %273 0
%279 = OpCompositeExtract %v3float %274 0
%280 = OpFOrdEqual %v3bool %278 %279
%281 = OpAll %bool %280
%282 = OpCompositeExtract %v3float %273 1
%283 = OpCompositeExtract %v3float %274 1
%284 = OpFOrdEqual %v3bool %282 %283
%285 = OpAll %bool %284
%286 = OpLogicalAnd %bool %281 %285
%287 = OpCompositeExtract %v3float %273 2
%288 = OpCompositeExtract %v3float %274 2
%289 = OpFOrdEqual %v3bool %287 %288
%290 = OpAll %bool %289
%291 = OpLogicalAnd %bool %286 %290
OpBranch %267
%267 = OpLabel
%292 = OpPhi %bool %false %228 %291 %266
OpStore %ok %292
%293 = OpLoad %bool %ok
OpSelectionMerge %295 None
OpBranchConditional %293 %294 %295
%294 = OpLabel
%296 = OpLoad %float %four
%298 = OpCompositeConstruct %v2float %float_2 %float_2
%299 = OpCompositeConstruct %v2float %float_2 %float_2
%297 = OpCompositeConstruct %mat2v2float %298 %299
%301 = OpCompositeConstruct %v2float %296 %296
%302 = OpCompositeConstruct %mat2v2float %301 %301
%303 = OpCompositeExtract %v2float %302 0
%304 = OpCompositeExtract %v2float %297 0
%305 = OpFDiv %v2float %303 %304
%306 = OpCompositeExtract %v2float %302 1
%307 = OpCompositeExtract %v2float %297 1
%308 = OpFDiv %v2float %306 %307
%309 = OpCompositeConstruct %mat2v2float %305 %308
%311 = OpCompositeConstruct %v2float %float_2 %float_2
%312 = OpCompositeConstruct %v2float %float_2 %float_2
%310 = OpCompositeConstruct %mat2v2float %311 %312
%314 = OpCompositeExtract %v2float %309 0
%315 = OpCompositeExtract %v2float %310 0
%316 = OpFOrdEqual %v2bool %314 %315
%317 = OpAll %bool %316
%318 = OpCompositeExtract %v2float %309 1
%319 = OpCompositeExtract %v2float %310 1
%320 = OpFOrdEqual %v2bool %318 %319
%321 = OpAll %bool %320
%322 = OpLogicalAnd %bool %317 %321
OpBranch %295
%295 = OpLabel
%323 = OpPhi %bool %false %267 %322 %294
OpStore %ok %323
%324 = OpLoad %bool %ok
OpReturnValue %324
OpFunctionEnd
%main = OpFunction %v4float None %325
%326 = OpFunctionParameter %_ptr_Function_v2float
%327 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_four = OpVariable %_ptr_Function_float Function
%615 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
OpStore %_1_four %float_4
%330 = OpLoad %bool %_0_ok
OpSelectionMerge %332 None
OpBranchConditional %330 %331 %332
%331 = OpLabel
%334 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%335 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%336 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%333 = OpCompositeConstruct %mat3v3float %334 %335 %336
%337 = OpLoad %float %_1_four
%338 = OpCompositeConstruct %v3float %337 %337 %337
%339 = OpCompositeConstruct %mat3v3float %338 %338 %338
%340 = OpCompositeExtract %v3float %333 0
%341 = OpCompositeExtract %v3float %339 0
%342 = OpFAdd %v3float %340 %341
%343 = OpCompositeExtract %v3float %333 1
%344 = OpCompositeExtract %v3float %339 1
%345 = OpFAdd %v3float %343 %344
%346 = OpCompositeExtract %v3float %333 2
%347 = OpCompositeExtract %v3float %339 2
%348 = OpFAdd %v3float %346 %347
%349 = OpCompositeConstruct %mat3v3float %342 %345 %348
%351 = OpCompositeConstruct %v3float %float_6 %float_4 %float_4
%352 = OpCompositeConstruct %v3float %float_4 %float_6 %float_4
%353 = OpCompositeConstruct %v3float %float_4 %float_4 %float_6
%350 = OpCompositeConstruct %mat3v3float %351 %352 %353
%354 = OpCompositeExtract %v3float %349 0
%355 = OpCompositeExtract %v3float %350 0
%356 = OpFOrdEqual %v3bool %354 %355
%357 = OpAll %bool %356
%358 = OpCompositeExtract %v3float %349 1
%359 = OpCompositeExtract %v3float %350 1
%360 = OpFOrdEqual %v3bool %358 %359
%361 = OpAll %bool %360
%362 = OpLogicalAnd %bool %357 %361
%363 = OpCompositeExtract %v3float %349 2
%364 = OpCompositeExtract %v3float %350 2
%365 = OpFOrdEqual %v3bool %363 %364
%366 = OpAll %bool %365
%367 = OpLogicalAnd %bool %362 %366
OpBranch %332
%332 = OpLabel
%368 = OpPhi %bool %false %327 %367 %331
OpStore %_0_ok %368
%369 = OpLoad %bool %_0_ok
OpSelectionMerge %371 None
OpBranchConditional %369 %370 %371
%370 = OpLabel
%373 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%374 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%375 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%372 = OpCompositeConstruct %mat3v3float %373 %374 %375
%376 = OpLoad %float %_1_four
%377 = OpCompositeConstruct %v3float %376 %376 %376
%378 = OpCompositeConstruct %mat3v3float %377 %377 %377
%379 = OpCompositeExtract %v3float %372 0
%380 = OpCompositeExtract %v3float %378 0
%381 = OpFSub %v3float %379 %380
%382 = OpCompositeExtract %v3float %372 1
%383 = OpCompositeExtract %v3float %378 1
%384 = OpFSub %v3float %382 %383
%385 = OpCompositeExtract %v3float %372 2
%386 = OpCompositeExtract %v3float %378 2
%387 = OpFSub %v3float %385 %386
%388 = OpCompositeConstruct %mat3v3float %381 %384 %387
%390 = OpCompositeConstruct %v3float %float_n2 %float_n4 %float_n4
%391 = OpCompositeConstruct %v3float %float_n4 %float_n2 %float_n4
%392 = OpCompositeConstruct %v3float %float_n4 %float_n4 %float_n2
%389 = OpCompositeConstruct %mat3v3float %390 %391 %392
%393 = OpCompositeExtract %v3float %388 0
%394 = OpCompositeExtract %v3float %389 0
%395 = OpFOrdEqual %v3bool %393 %394
%396 = OpAll %bool %395
%397 = OpCompositeExtract %v3float %388 1
%398 = OpCompositeExtract %v3float %389 1
%399 = OpFOrdEqual %v3bool %397 %398
%400 = OpAll %bool %399
%401 = OpLogicalAnd %bool %396 %400
%402 = OpCompositeExtract %v3float %388 2
%403 = OpCompositeExtract %v3float %389 2
%404 = OpFOrdEqual %v3bool %402 %403
%405 = OpAll %bool %404
%406 = OpLogicalAnd %bool %401 %405
OpBranch %371
%371 = OpLabel
%407 = OpPhi %bool %false %332 %406 %370
OpStore %_0_ok %407
%408 = OpLoad %bool %_0_ok
OpSelectionMerge %410 None
OpBranchConditional %408 %409 %410
%409 = OpLabel
%412 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%413 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%414 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%411 = OpCompositeConstruct %mat3v3float %412 %413 %414
%415 = OpLoad %float %_1_four
%416 = OpMatrixTimesScalar %mat3v3float %411 %415
%418 = OpCompositeConstruct %v3float %float_8 %float_0 %float_0
%419 = OpCompositeConstruct %v3float %float_0 %float_8 %float_0
%420 = OpCompositeConstruct %v3float %float_0 %float_0 %float_8
%417 = OpCompositeConstruct %mat3v3float %418 %419 %420
%421 = OpCompositeExtract %v3float %416 0
%422 = OpCompositeExtract %v3float %417 0
%423 = OpFOrdEqual %v3bool %421 %422
%424 = OpAll %bool %423
%425 = OpCompositeExtract %v3float %416 1
%426 = OpCompositeExtract %v3float %417 1
%427 = OpFOrdEqual %v3bool %425 %426
%428 = OpAll %bool %427
%429 = OpLogicalAnd %bool %424 %428
%430 = OpCompositeExtract %v3float %416 2
%431 = OpCompositeExtract %v3float %417 2
%432 = OpFOrdEqual %v3bool %430 %431
%433 = OpAll %bool %432
%434 = OpLogicalAnd %bool %429 %433
OpBranch %410
%410 = OpLabel
%435 = OpPhi %bool %false %371 %434 %409
OpStore %_0_ok %435
%436 = OpLoad %bool %_0_ok
OpSelectionMerge %438 None
OpBranchConditional %436 %437 %438
%437 = OpLabel
%440 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%441 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%442 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%439 = OpCompositeConstruct %mat3v3float %440 %441 %442
%443 = OpLoad %float %_1_four
%444 = OpCompositeConstruct %v3float %443 %443 %443
%445 = OpCompositeConstruct %mat3v3float %444 %444 %444
%446 = OpCompositeExtract %v3float %439 0
%447 = OpCompositeExtract %v3float %445 0
%448 = OpFDiv %v3float %446 %447
%449 = OpCompositeExtract %v3float %439 1
%450 = OpCompositeExtract %v3float %445 1
%451 = OpFDiv %v3float %449 %450
%452 = OpCompositeExtract %v3float %439 2
%453 = OpCompositeExtract %v3float %445 2
%454 = OpFDiv %v3float %452 %453
%455 = OpCompositeConstruct %mat3v3float %448 %451 %454
%457 = OpCompositeConstruct %v3float %float_0_5 %float_0 %float_0
%458 = OpCompositeConstruct %v3float %float_0 %float_0_5 %float_0
%459 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0_5
%456 = OpCompositeConstruct %mat3v3float %457 %458 %459
%460 = OpCompositeExtract %v3float %455 0
%461 = OpCompositeExtract %v3float %456 0
%462 = OpFOrdEqual %v3bool %460 %461
%463 = OpAll %bool %462
%464 = OpCompositeExtract %v3float %455 1
%465 = OpCompositeExtract %v3float %456 1
%466 = OpFOrdEqual %v3bool %464 %465
%467 = OpAll %bool %466
%468 = OpLogicalAnd %bool %463 %467
%469 = OpCompositeExtract %v3float %455 2
%470 = OpCompositeExtract %v3float %456 2
%471 = OpFOrdEqual %v3bool %469 %470
%472 = OpAll %bool %471
%473 = OpLogicalAnd %bool %468 %472
OpBranch %438
%438 = OpLabel
%474 = OpPhi %bool %false %410 %473 %437
OpStore %_0_ok %474
%475 = OpLoad %bool %_0_ok
OpSelectionMerge %477 None
OpBranchConditional %475 %476 %477
%476 = OpLabel
%478 = OpLoad %float %_1_four
%480 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%481 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%482 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%479 = OpCompositeConstruct %mat3v3float %480 %481 %482
%483 = OpCompositeConstruct %v3float %478 %478 %478
%484 = OpCompositeConstruct %mat3v3float %483 %483 %483
%485 = OpCompositeExtract %v3float %484 0
%486 = OpCompositeExtract %v3float %479 0
%487 = OpFAdd %v3float %485 %486
%488 = OpCompositeExtract %v3float %484 1
%489 = OpCompositeExtract %v3float %479 1
%490 = OpFAdd %v3float %488 %489
%491 = OpCompositeExtract %v3float %484 2
%492 = OpCompositeExtract %v3float %479 2
%493 = OpFAdd %v3float %491 %492
%494 = OpCompositeConstruct %mat3v3float %487 %490 %493
%496 = OpCompositeConstruct %v3float %float_6 %float_4 %float_4
%497 = OpCompositeConstruct %v3float %float_4 %float_6 %float_4
%498 = OpCompositeConstruct %v3float %float_4 %float_4 %float_6
%495 = OpCompositeConstruct %mat3v3float %496 %497 %498
%499 = OpCompositeExtract %v3float %494 0
%500 = OpCompositeExtract %v3float %495 0
%501 = OpFOrdEqual %v3bool %499 %500
%502 = OpAll %bool %501
%503 = OpCompositeExtract %v3float %494 1
%504 = OpCompositeExtract %v3float %495 1
%505 = OpFOrdEqual %v3bool %503 %504
%506 = OpAll %bool %505
%507 = OpLogicalAnd %bool %502 %506
%508 = OpCompositeExtract %v3float %494 2
%509 = OpCompositeExtract %v3float %495 2
%510 = OpFOrdEqual %v3bool %508 %509
%511 = OpAll %bool %510
%512 = OpLogicalAnd %bool %507 %511
OpBranch %477
%477 = OpLabel
%513 = OpPhi %bool %false %438 %512 %476
OpStore %_0_ok %513
%514 = OpLoad %bool %_0_ok
OpSelectionMerge %516 None
OpBranchConditional %514 %515 %516
%515 = OpLabel
%517 = OpLoad %float %_1_four
%519 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%520 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%521 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%518 = OpCompositeConstruct %mat3v3float %519 %520 %521
%522 = OpCompositeConstruct %v3float %517 %517 %517
%523 = OpCompositeConstruct %mat3v3float %522 %522 %522
%524 = OpCompositeExtract %v3float %523 0
%525 = OpCompositeExtract %v3float %518 0
%526 = OpFSub %v3float %524 %525
%527 = OpCompositeExtract %v3float %523 1
%528 = OpCompositeExtract %v3float %518 1
%529 = OpFSub %v3float %527 %528
%530 = OpCompositeExtract %v3float %523 2
%531 = OpCompositeExtract %v3float %518 2
%532 = OpFSub %v3float %530 %531
%533 = OpCompositeConstruct %mat3v3float %526 %529 %532
%535 = OpCompositeConstruct %v3float %float_2 %float_4 %float_4
%536 = OpCompositeConstruct %v3float %float_4 %float_2 %float_4
%537 = OpCompositeConstruct %v3float %float_4 %float_4 %float_2
%534 = OpCompositeConstruct %mat3v3float %535 %536 %537
%538 = OpCompositeExtract %v3float %533 0
%539 = OpCompositeExtract %v3float %534 0
%540 = OpFOrdEqual %v3bool %538 %539
%541 = OpAll %bool %540
%542 = OpCompositeExtract %v3float %533 1
%543 = OpCompositeExtract %v3float %534 1
%544 = OpFOrdEqual %v3bool %542 %543
%545 = OpAll %bool %544
%546 = OpLogicalAnd %bool %541 %545
%547 = OpCompositeExtract %v3float %533 2
%548 = OpCompositeExtract %v3float %534 2
%549 = OpFOrdEqual %v3bool %547 %548
%550 = OpAll %bool %549
%551 = OpLogicalAnd %bool %546 %550
OpBranch %516
%516 = OpLabel
%552 = OpPhi %bool %false %477 %551 %515
OpStore %_0_ok %552
%553 = OpLoad %bool %_0_ok
OpSelectionMerge %555 None
OpBranchConditional %553 %554 %555
%554 = OpLabel
%556 = OpLoad %float %_1_four
%558 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%559 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%560 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%557 = OpCompositeConstruct %mat3v3float %558 %559 %560
%561 = OpMatrixTimesScalar %mat3v3float %557 %556
%563 = OpCompositeConstruct %v3float %float_8 %float_0 %float_0
%564 = OpCompositeConstruct %v3float %float_0 %float_8 %float_0
%565 = OpCompositeConstruct %v3float %float_0 %float_0 %float_8
%562 = OpCompositeConstruct %mat3v3float %563 %564 %565
%566 = OpCompositeExtract %v3float %561 0
%567 = OpCompositeExtract %v3float %562 0
%568 = OpFOrdEqual %v3bool %566 %567
%569 = OpAll %bool %568
%570 = OpCompositeExtract %v3float %561 1
%571 = OpCompositeExtract %v3float %562 1
%572 = OpFOrdEqual %v3bool %570 %571
%573 = OpAll %bool %572
%574 = OpLogicalAnd %bool %569 %573
%575 = OpCompositeExtract %v3float %561 2
%576 = OpCompositeExtract %v3float %562 2
%577 = OpFOrdEqual %v3bool %575 %576
%578 = OpAll %bool %577
%579 = OpLogicalAnd %bool %574 %578
OpBranch %555
%555 = OpLabel
%580 = OpPhi %bool %false %516 %579 %554
OpStore %_0_ok %580
%581 = OpLoad %bool %_0_ok
OpSelectionMerge %583 None
OpBranchConditional %581 %582 %583
%582 = OpLabel
%584 = OpLoad %float %_1_four
%586 = OpCompositeConstruct %v2float %float_2 %float_2
%587 = OpCompositeConstruct %v2float %float_2 %float_2
%585 = OpCompositeConstruct %mat2v2float %586 %587
%588 = OpCompositeConstruct %v2float %584 %584
%589 = OpCompositeConstruct %mat2v2float %588 %588
%590 = OpCompositeExtract %v2float %589 0
%591 = OpCompositeExtract %v2float %585 0
%592 = OpFDiv %v2float %590 %591
%593 = OpCompositeExtract %v2float %589 1
%594 = OpCompositeExtract %v2float %585 1
%595 = OpFDiv %v2float %593 %594
%596 = OpCompositeConstruct %mat2v2float %592 %595
%598 = OpCompositeConstruct %v2float %float_2 %float_2
%599 = OpCompositeConstruct %v2float %float_2 %float_2
%597 = OpCompositeConstruct %mat2v2float %598 %599
%600 = OpCompositeExtract %v2float %596 0
%601 = OpCompositeExtract %v2float %597 0
%602 = OpFOrdEqual %v2bool %600 %601
%603 = OpAll %bool %602
%604 = OpCompositeExtract %v2float %596 1
%605 = OpCompositeExtract %v2float %597 1
%606 = OpFOrdEqual %v2bool %604 %605
%607 = OpAll %bool %606
%608 = OpLogicalAnd %bool %603 %607
OpBranch %583
%583 = OpLabel
%609 = OpPhi %bool %false %555 %608 %582
OpStore %_0_ok %609
%610 = OpLoad %bool %_0_ok
OpSelectionMerge %612 None
OpBranchConditional %610 %611 %612
%611 = OpLabel
%613 = OpFunctionCall %bool %test_half_b
OpBranch %612
%612 = OpLabel
%614 = OpPhi %bool %false %583 %613 %611
OpSelectionMerge %619 None
OpBranchConditional %614 %617 %618
%617 = OpLabel
%620 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%624 = OpLoad %v4float %620
OpStore %615 %624
OpBranch %619
%618 = OpLabel
%625 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%627 = OpLoad %v4float %625
OpStore %615 %627
OpBranch %619
%619 = OpLabel
%628 = OpLoad %v4float %615
OpReturnValue %628
OpFunctionEnd
