### Compilation failed:

error: SPIR-V validation error: Expected floating scalar or vector type as Result Type: FDiv
  %157 = OpFDiv %mat3v3float %150 %156

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
OpDecorate %178 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %393 RelaxedPrecision
OpDecorate %421 RelaxedPrecision
OpDecorate %451 RelaxedPrecision
OpDecorate %490 RelaxedPrecision
OpDecorate %529 RelaxedPrecision
OpDecorate %557 RelaxedPrecision
OpDecorate %580 RelaxedPrecision
OpDecorate %594 RelaxedPrecision
OpDecorate %597 RelaxedPrecision
OpDecorate %598 RelaxedPrecision
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
%310 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%157 = OpFDiv %mat3v3float %150 %156
%160 = OpCompositeConstruct %v3float %float_0_5 %float_0 %float_0
%161 = OpCompositeConstruct %v3float %float_0 %float_0_5 %float_0
%162 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0_5
%159 = OpCompositeConstruct %mat3v3float %160 %161 %162
%163 = OpCompositeExtract %v3float %157 0
%164 = OpCompositeExtract %v3float %159 0
%165 = OpFOrdEqual %v3bool %163 %164
%166 = OpAll %bool %165
%167 = OpCompositeExtract %v3float %157 1
%168 = OpCompositeExtract %v3float %159 1
%169 = OpFOrdEqual %v3bool %167 %168
%170 = OpAll %bool %169
%171 = OpLogicalAnd %bool %166 %170
%172 = OpCompositeExtract %v3float %157 2
%173 = OpCompositeExtract %v3float %159 2
%174 = OpFOrdEqual %v3bool %172 %173
%175 = OpAll %bool %174
%176 = OpLogicalAnd %bool %171 %175
OpBranch %149
%149 = OpLabel
%177 = OpPhi %bool %false %120 %176 %148
OpStore %ok %177
%178 = OpLoad %bool %ok
OpSelectionMerge %180 None
OpBranchConditional %178 %179 %180
%179 = OpLabel
%181 = OpLoad %float %four
%183 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%184 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%185 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%182 = OpCompositeConstruct %mat3v3float %183 %184 %185
%186 = OpCompositeConstruct %v3float %181 %181 %181
%187 = OpCompositeConstruct %mat3v3float %186 %186 %186
%188 = OpCompositeExtract %v3float %187 0
%189 = OpCompositeExtract %v3float %182 0
%190 = OpFAdd %v3float %188 %189
%191 = OpCompositeExtract %v3float %187 1
%192 = OpCompositeExtract %v3float %182 1
%193 = OpFAdd %v3float %191 %192
%194 = OpCompositeExtract %v3float %187 2
%195 = OpCompositeExtract %v3float %182 2
%196 = OpFAdd %v3float %194 %195
%197 = OpCompositeConstruct %mat3v3float %190 %193 %196
%199 = OpCompositeConstruct %v3float %float_6 %float_4 %float_4
%200 = OpCompositeConstruct %v3float %float_4 %float_6 %float_4
%201 = OpCompositeConstruct %v3float %float_4 %float_4 %float_6
%198 = OpCompositeConstruct %mat3v3float %199 %200 %201
%202 = OpCompositeExtract %v3float %197 0
%203 = OpCompositeExtract %v3float %198 0
%204 = OpFOrdEqual %v3bool %202 %203
%205 = OpAll %bool %204
%206 = OpCompositeExtract %v3float %197 1
%207 = OpCompositeExtract %v3float %198 1
%208 = OpFOrdEqual %v3bool %206 %207
%209 = OpAll %bool %208
%210 = OpLogicalAnd %bool %205 %209
%211 = OpCompositeExtract %v3float %197 2
%212 = OpCompositeExtract %v3float %198 2
%213 = OpFOrdEqual %v3bool %211 %212
%214 = OpAll %bool %213
%215 = OpLogicalAnd %bool %210 %214
OpBranch %180
%180 = OpLabel
%216 = OpPhi %bool %false %149 %215 %179
OpStore %ok %216
%217 = OpLoad %bool %ok
OpSelectionMerge %219 None
OpBranchConditional %217 %218 %219
%218 = OpLabel
%220 = OpLoad %float %four
%222 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%223 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%224 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%221 = OpCompositeConstruct %mat3v3float %222 %223 %224
%225 = OpCompositeConstruct %v3float %220 %220 %220
%226 = OpCompositeConstruct %mat3v3float %225 %225 %225
%227 = OpCompositeExtract %v3float %226 0
%228 = OpCompositeExtract %v3float %221 0
%229 = OpFSub %v3float %227 %228
%230 = OpCompositeExtract %v3float %226 1
%231 = OpCompositeExtract %v3float %221 1
%232 = OpFSub %v3float %230 %231
%233 = OpCompositeExtract %v3float %226 2
%234 = OpCompositeExtract %v3float %221 2
%235 = OpFSub %v3float %233 %234
%236 = OpCompositeConstruct %mat3v3float %229 %232 %235
%238 = OpCompositeConstruct %v3float %float_2 %float_4 %float_4
%239 = OpCompositeConstruct %v3float %float_4 %float_2 %float_4
%240 = OpCompositeConstruct %v3float %float_4 %float_4 %float_2
%237 = OpCompositeConstruct %mat3v3float %238 %239 %240
%241 = OpCompositeExtract %v3float %236 0
%242 = OpCompositeExtract %v3float %237 0
%243 = OpFOrdEqual %v3bool %241 %242
%244 = OpAll %bool %243
%245 = OpCompositeExtract %v3float %236 1
%246 = OpCompositeExtract %v3float %237 1
%247 = OpFOrdEqual %v3bool %245 %246
%248 = OpAll %bool %247
%249 = OpLogicalAnd %bool %244 %248
%250 = OpCompositeExtract %v3float %236 2
%251 = OpCompositeExtract %v3float %237 2
%252 = OpFOrdEqual %v3bool %250 %251
%253 = OpAll %bool %252
%254 = OpLogicalAnd %bool %249 %253
OpBranch %219
%219 = OpLabel
%255 = OpPhi %bool %false %180 %254 %218
OpStore %ok %255
%256 = OpLoad %bool %ok
OpSelectionMerge %258 None
OpBranchConditional %256 %257 %258
%257 = OpLabel
%259 = OpLoad %float %four
%261 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%262 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%263 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%260 = OpCompositeConstruct %mat3v3float %261 %262 %263
%264 = OpMatrixTimesScalar %mat3v3float %260 %259
%266 = OpCompositeConstruct %v3float %float_8 %float_0 %float_0
%267 = OpCompositeConstruct %v3float %float_0 %float_8 %float_0
%268 = OpCompositeConstruct %v3float %float_0 %float_0 %float_8
%265 = OpCompositeConstruct %mat3v3float %266 %267 %268
%269 = OpCompositeExtract %v3float %264 0
%270 = OpCompositeExtract %v3float %265 0
%271 = OpFOrdEqual %v3bool %269 %270
%272 = OpAll %bool %271
%273 = OpCompositeExtract %v3float %264 1
%274 = OpCompositeExtract %v3float %265 1
%275 = OpFOrdEqual %v3bool %273 %274
%276 = OpAll %bool %275
%277 = OpLogicalAnd %bool %272 %276
%278 = OpCompositeExtract %v3float %264 2
%279 = OpCompositeExtract %v3float %265 2
%280 = OpFOrdEqual %v3bool %278 %279
%281 = OpAll %bool %280
%282 = OpLogicalAnd %bool %277 %281
OpBranch %258
%258 = OpLabel
%283 = OpPhi %bool %false %219 %282 %257
OpStore %ok %283
%284 = OpLoad %bool %ok
OpSelectionMerge %286 None
OpBranchConditional %284 %285 %286
%285 = OpLabel
%287 = OpLoad %float %four
%289 = OpCompositeConstruct %v2float %float_2 %float_2
%290 = OpCompositeConstruct %v2float %float_2 %float_2
%288 = OpCompositeConstruct %mat2v2float %289 %290
%292 = OpCompositeConstruct %v2float %287 %287
%293 = OpCompositeConstruct %mat2v2float %292 %292
%294 = OpFDiv %mat2v2float %293 %288
%296 = OpCompositeConstruct %v2float %float_2 %float_2
%297 = OpCompositeConstruct %v2float %float_2 %float_2
%295 = OpCompositeConstruct %mat2v2float %296 %297
%299 = OpCompositeExtract %v2float %294 0
%300 = OpCompositeExtract %v2float %295 0
%301 = OpFOrdEqual %v2bool %299 %300
%302 = OpAll %bool %301
%303 = OpCompositeExtract %v2float %294 1
%304 = OpCompositeExtract %v2float %295 1
%305 = OpFOrdEqual %v2bool %303 %304
%306 = OpAll %bool %305
%307 = OpLogicalAnd %bool %302 %306
OpBranch %286
%286 = OpLabel
%308 = OpPhi %bool %false %258 %307 %285
OpStore %ok %308
%309 = OpLoad %bool %ok
OpReturnValue %309
OpFunctionEnd
%main = OpFunction %v4float None %310
%311 = OpFunctionParameter %_ptr_Function_v2float
%312 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_four = OpVariable %_ptr_Function_float Function
%585 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
OpStore %_1_four %float_4
%315 = OpLoad %bool %_0_ok
OpSelectionMerge %317 None
OpBranchConditional %315 %316 %317
%316 = OpLabel
%319 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%320 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%321 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%318 = OpCompositeConstruct %mat3v3float %319 %320 %321
%322 = OpLoad %float %_1_four
%323 = OpCompositeConstruct %v3float %322 %322 %322
%324 = OpCompositeConstruct %mat3v3float %323 %323 %323
%325 = OpCompositeExtract %v3float %318 0
%326 = OpCompositeExtract %v3float %324 0
%327 = OpFAdd %v3float %325 %326
%328 = OpCompositeExtract %v3float %318 1
%329 = OpCompositeExtract %v3float %324 1
%330 = OpFAdd %v3float %328 %329
%331 = OpCompositeExtract %v3float %318 2
%332 = OpCompositeExtract %v3float %324 2
%333 = OpFAdd %v3float %331 %332
%334 = OpCompositeConstruct %mat3v3float %327 %330 %333
%336 = OpCompositeConstruct %v3float %float_6 %float_4 %float_4
%337 = OpCompositeConstruct %v3float %float_4 %float_6 %float_4
%338 = OpCompositeConstruct %v3float %float_4 %float_4 %float_6
%335 = OpCompositeConstruct %mat3v3float %336 %337 %338
%339 = OpCompositeExtract %v3float %334 0
%340 = OpCompositeExtract %v3float %335 0
%341 = OpFOrdEqual %v3bool %339 %340
%342 = OpAll %bool %341
%343 = OpCompositeExtract %v3float %334 1
%344 = OpCompositeExtract %v3float %335 1
%345 = OpFOrdEqual %v3bool %343 %344
%346 = OpAll %bool %345
%347 = OpLogicalAnd %bool %342 %346
%348 = OpCompositeExtract %v3float %334 2
%349 = OpCompositeExtract %v3float %335 2
%350 = OpFOrdEqual %v3bool %348 %349
%351 = OpAll %bool %350
%352 = OpLogicalAnd %bool %347 %351
OpBranch %317
%317 = OpLabel
%353 = OpPhi %bool %false %312 %352 %316
OpStore %_0_ok %353
%354 = OpLoad %bool %_0_ok
OpSelectionMerge %356 None
OpBranchConditional %354 %355 %356
%355 = OpLabel
%358 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%359 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%360 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%357 = OpCompositeConstruct %mat3v3float %358 %359 %360
%361 = OpLoad %float %_1_four
%362 = OpCompositeConstruct %v3float %361 %361 %361
%363 = OpCompositeConstruct %mat3v3float %362 %362 %362
%364 = OpCompositeExtract %v3float %357 0
%365 = OpCompositeExtract %v3float %363 0
%366 = OpFSub %v3float %364 %365
%367 = OpCompositeExtract %v3float %357 1
%368 = OpCompositeExtract %v3float %363 1
%369 = OpFSub %v3float %367 %368
%370 = OpCompositeExtract %v3float %357 2
%371 = OpCompositeExtract %v3float %363 2
%372 = OpFSub %v3float %370 %371
%373 = OpCompositeConstruct %mat3v3float %366 %369 %372
%375 = OpCompositeConstruct %v3float %float_n2 %float_n4 %float_n4
%376 = OpCompositeConstruct %v3float %float_n4 %float_n2 %float_n4
%377 = OpCompositeConstruct %v3float %float_n4 %float_n4 %float_n2
%374 = OpCompositeConstruct %mat3v3float %375 %376 %377
%378 = OpCompositeExtract %v3float %373 0
%379 = OpCompositeExtract %v3float %374 0
%380 = OpFOrdEqual %v3bool %378 %379
%381 = OpAll %bool %380
%382 = OpCompositeExtract %v3float %373 1
%383 = OpCompositeExtract %v3float %374 1
%384 = OpFOrdEqual %v3bool %382 %383
%385 = OpAll %bool %384
%386 = OpLogicalAnd %bool %381 %385
%387 = OpCompositeExtract %v3float %373 2
%388 = OpCompositeExtract %v3float %374 2
%389 = OpFOrdEqual %v3bool %387 %388
%390 = OpAll %bool %389
%391 = OpLogicalAnd %bool %386 %390
OpBranch %356
%356 = OpLabel
%392 = OpPhi %bool %false %317 %391 %355
OpStore %_0_ok %392
%393 = OpLoad %bool %_0_ok
OpSelectionMerge %395 None
OpBranchConditional %393 %394 %395
%394 = OpLabel
%397 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%398 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%399 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%396 = OpCompositeConstruct %mat3v3float %397 %398 %399
%400 = OpLoad %float %_1_four
%401 = OpMatrixTimesScalar %mat3v3float %396 %400
%403 = OpCompositeConstruct %v3float %float_8 %float_0 %float_0
%404 = OpCompositeConstruct %v3float %float_0 %float_8 %float_0
%405 = OpCompositeConstruct %v3float %float_0 %float_0 %float_8
%402 = OpCompositeConstruct %mat3v3float %403 %404 %405
%406 = OpCompositeExtract %v3float %401 0
%407 = OpCompositeExtract %v3float %402 0
%408 = OpFOrdEqual %v3bool %406 %407
%409 = OpAll %bool %408
%410 = OpCompositeExtract %v3float %401 1
%411 = OpCompositeExtract %v3float %402 1
%412 = OpFOrdEqual %v3bool %410 %411
%413 = OpAll %bool %412
%414 = OpLogicalAnd %bool %409 %413
%415 = OpCompositeExtract %v3float %401 2
%416 = OpCompositeExtract %v3float %402 2
%417 = OpFOrdEqual %v3bool %415 %416
%418 = OpAll %bool %417
%419 = OpLogicalAnd %bool %414 %418
OpBranch %395
%395 = OpLabel
%420 = OpPhi %bool %false %356 %419 %394
OpStore %_0_ok %420
%421 = OpLoad %bool %_0_ok
OpSelectionMerge %423 None
OpBranchConditional %421 %422 %423
%422 = OpLabel
%425 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%426 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%427 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%424 = OpCompositeConstruct %mat3v3float %425 %426 %427
%428 = OpLoad %float %_1_four
%429 = OpCompositeConstruct %v3float %428 %428 %428
%430 = OpCompositeConstruct %mat3v3float %429 %429 %429
%431 = OpFDiv %mat3v3float %424 %430
%433 = OpCompositeConstruct %v3float %float_0_5 %float_0 %float_0
%434 = OpCompositeConstruct %v3float %float_0 %float_0_5 %float_0
%435 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0_5
%432 = OpCompositeConstruct %mat3v3float %433 %434 %435
%436 = OpCompositeExtract %v3float %431 0
%437 = OpCompositeExtract %v3float %432 0
%438 = OpFOrdEqual %v3bool %436 %437
%439 = OpAll %bool %438
%440 = OpCompositeExtract %v3float %431 1
%441 = OpCompositeExtract %v3float %432 1
%442 = OpFOrdEqual %v3bool %440 %441
%443 = OpAll %bool %442
%444 = OpLogicalAnd %bool %439 %443
%445 = OpCompositeExtract %v3float %431 2
%446 = OpCompositeExtract %v3float %432 2
%447 = OpFOrdEqual %v3bool %445 %446
%448 = OpAll %bool %447
%449 = OpLogicalAnd %bool %444 %448
OpBranch %423
%423 = OpLabel
%450 = OpPhi %bool %false %395 %449 %422
OpStore %_0_ok %450
%451 = OpLoad %bool %_0_ok
OpSelectionMerge %453 None
OpBranchConditional %451 %452 %453
%452 = OpLabel
%454 = OpLoad %float %_1_four
%456 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%457 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%458 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%455 = OpCompositeConstruct %mat3v3float %456 %457 %458
%459 = OpCompositeConstruct %v3float %454 %454 %454
%460 = OpCompositeConstruct %mat3v3float %459 %459 %459
%461 = OpCompositeExtract %v3float %460 0
%462 = OpCompositeExtract %v3float %455 0
%463 = OpFAdd %v3float %461 %462
%464 = OpCompositeExtract %v3float %460 1
%465 = OpCompositeExtract %v3float %455 1
%466 = OpFAdd %v3float %464 %465
%467 = OpCompositeExtract %v3float %460 2
%468 = OpCompositeExtract %v3float %455 2
%469 = OpFAdd %v3float %467 %468
%470 = OpCompositeConstruct %mat3v3float %463 %466 %469
%472 = OpCompositeConstruct %v3float %float_6 %float_4 %float_4
%473 = OpCompositeConstruct %v3float %float_4 %float_6 %float_4
%474 = OpCompositeConstruct %v3float %float_4 %float_4 %float_6
%471 = OpCompositeConstruct %mat3v3float %472 %473 %474
%475 = OpCompositeExtract %v3float %470 0
%476 = OpCompositeExtract %v3float %471 0
%477 = OpFOrdEqual %v3bool %475 %476
%478 = OpAll %bool %477
%479 = OpCompositeExtract %v3float %470 1
%480 = OpCompositeExtract %v3float %471 1
%481 = OpFOrdEqual %v3bool %479 %480
%482 = OpAll %bool %481
%483 = OpLogicalAnd %bool %478 %482
%484 = OpCompositeExtract %v3float %470 2
%485 = OpCompositeExtract %v3float %471 2
%486 = OpFOrdEqual %v3bool %484 %485
%487 = OpAll %bool %486
%488 = OpLogicalAnd %bool %483 %487
OpBranch %453
%453 = OpLabel
%489 = OpPhi %bool %false %423 %488 %452
OpStore %_0_ok %489
%490 = OpLoad %bool %_0_ok
OpSelectionMerge %492 None
OpBranchConditional %490 %491 %492
%491 = OpLabel
%493 = OpLoad %float %_1_four
%495 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%496 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%497 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%494 = OpCompositeConstruct %mat3v3float %495 %496 %497
%498 = OpCompositeConstruct %v3float %493 %493 %493
%499 = OpCompositeConstruct %mat3v3float %498 %498 %498
%500 = OpCompositeExtract %v3float %499 0
%501 = OpCompositeExtract %v3float %494 0
%502 = OpFSub %v3float %500 %501
%503 = OpCompositeExtract %v3float %499 1
%504 = OpCompositeExtract %v3float %494 1
%505 = OpFSub %v3float %503 %504
%506 = OpCompositeExtract %v3float %499 2
%507 = OpCompositeExtract %v3float %494 2
%508 = OpFSub %v3float %506 %507
%509 = OpCompositeConstruct %mat3v3float %502 %505 %508
%511 = OpCompositeConstruct %v3float %float_2 %float_4 %float_4
%512 = OpCompositeConstruct %v3float %float_4 %float_2 %float_4
%513 = OpCompositeConstruct %v3float %float_4 %float_4 %float_2
%510 = OpCompositeConstruct %mat3v3float %511 %512 %513
%514 = OpCompositeExtract %v3float %509 0
%515 = OpCompositeExtract %v3float %510 0
%516 = OpFOrdEqual %v3bool %514 %515
%517 = OpAll %bool %516
%518 = OpCompositeExtract %v3float %509 1
%519 = OpCompositeExtract %v3float %510 1
%520 = OpFOrdEqual %v3bool %518 %519
%521 = OpAll %bool %520
%522 = OpLogicalAnd %bool %517 %521
%523 = OpCompositeExtract %v3float %509 2
%524 = OpCompositeExtract %v3float %510 2
%525 = OpFOrdEqual %v3bool %523 %524
%526 = OpAll %bool %525
%527 = OpLogicalAnd %bool %522 %526
OpBranch %492
%492 = OpLabel
%528 = OpPhi %bool %false %453 %527 %491
OpStore %_0_ok %528
%529 = OpLoad %bool %_0_ok
OpSelectionMerge %531 None
OpBranchConditional %529 %530 %531
%530 = OpLabel
%532 = OpLoad %float %_1_four
%534 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%535 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%536 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%533 = OpCompositeConstruct %mat3v3float %534 %535 %536
%537 = OpMatrixTimesScalar %mat3v3float %533 %532
%539 = OpCompositeConstruct %v3float %float_8 %float_0 %float_0
%540 = OpCompositeConstruct %v3float %float_0 %float_8 %float_0
%541 = OpCompositeConstruct %v3float %float_0 %float_0 %float_8
%538 = OpCompositeConstruct %mat3v3float %539 %540 %541
%542 = OpCompositeExtract %v3float %537 0
%543 = OpCompositeExtract %v3float %538 0
%544 = OpFOrdEqual %v3bool %542 %543
%545 = OpAll %bool %544
%546 = OpCompositeExtract %v3float %537 1
%547 = OpCompositeExtract %v3float %538 1
%548 = OpFOrdEqual %v3bool %546 %547
%549 = OpAll %bool %548
%550 = OpLogicalAnd %bool %545 %549
%551 = OpCompositeExtract %v3float %537 2
%552 = OpCompositeExtract %v3float %538 2
%553 = OpFOrdEqual %v3bool %551 %552
%554 = OpAll %bool %553
%555 = OpLogicalAnd %bool %550 %554
OpBranch %531
%531 = OpLabel
%556 = OpPhi %bool %false %492 %555 %530
OpStore %_0_ok %556
%557 = OpLoad %bool %_0_ok
OpSelectionMerge %559 None
OpBranchConditional %557 %558 %559
%558 = OpLabel
%560 = OpLoad %float %_1_four
%562 = OpCompositeConstruct %v2float %float_2 %float_2
%563 = OpCompositeConstruct %v2float %float_2 %float_2
%561 = OpCompositeConstruct %mat2v2float %562 %563
%564 = OpCompositeConstruct %v2float %560 %560
%565 = OpCompositeConstruct %mat2v2float %564 %564
%566 = OpFDiv %mat2v2float %565 %561
%568 = OpCompositeConstruct %v2float %float_2 %float_2
%569 = OpCompositeConstruct %v2float %float_2 %float_2
%567 = OpCompositeConstruct %mat2v2float %568 %569
%570 = OpCompositeExtract %v2float %566 0
%571 = OpCompositeExtract %v2float %567 0
%572 = OpFOrdEqual %v2bool %570 %571
%573 = OpAll %bool %572
%574 = OpCompositeExtract %v2float %566 1
%575 = OpCompositeExtract %v2float %567 1
%576 = OpFOrdEqual %v2bool %574 %575
%577 = OpAll %bool %576
%578 = OpLogicalAnd %bool %573 %577
OpBranch %559
%559 = OpLabel
%579 = OpPhi %bool %false %531 %578 %558
OpStore %_0_ok %579
%580 = OpLoad %bool %_0_ok
OpSelectionMerge %582 None
OpBranchConditional %580 %581 %582
%581 = OpLabel
%583 = OpFunctionCall %bool %test_half_b
OpBranch %582
%582 = OpLabel
%584 = OpPhi %bool %false %559 %583 %581
OpSelectionMerge %589 None
OpBranchConditional %584 %587 %588
%587 = OpLabel
%590 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%594 = OpLoad %v4float %590
OpStore %585 %594
OpBranch %589
%588 = OpLabel
%595 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%597 = OpLoad %v4float %595
OpStore %585 %597
OpBranch %589
%589 = OpLabel
%598 = OpLoad %v4float %585
OpReturnValue %598
OpFunctionEnd

1 error
