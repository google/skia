OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_float_b "test_float_b"
OpName %ok "ok"
OpName %m23 "m23"
OpName %m24 "m24"
OpName %m32 "m32"
OpName %m34 "m34"
OpName %m42 "m42"
OpName %m43 "m43"
OpName %m22 "m22"
OpName %m33 "m33"
OpName %test_half_b "test_half_b"
OpName %ok_0 "ok"
OpName %m23_0 "m23"
OpName %m24_0 "m24"
OpName %m32_0 "m32"
OpName %m34_0 "m34"
OpName %m42_0 "m42"
OpName %m43_0 "m43"
OpName %m22_0 "m22"
OpName %m33_0 "m33"
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
OpDecorate %m23_0 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %m24_0 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %m32_0 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %m34_0 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %m42_0 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %m43_0 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %m22_0 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %m33_0 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %337 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %350 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %399 RelaxedPrecision
OpDecorate %402 RelaxedPrecision
OpDecorate %403 RelaxedPrecision
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
%v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
%float_2 = OpConstant %float 2
%35 = OpConstantComposite %v3float %float_2 %float_0 %float_0
%36 = OpConstantComposite %v3float %float_0 %float_2 %float_0
%37 = OpConstantComposite %mat2v3float %35 %36
%false = OpConstantFalse %bool
%v3bool = OpTypeVector %bool 3
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%float_3 = OpConstant %float 3
%52 = OpConstantComposite %v4float %float_3 %float_0 %float_0 %float_0
%53 = OpConstantComposite %v4float %float_0 %float_3 %float_0 %float_0
%54 = OpConstantComposite %mat2v4float %52 %53
%v4bool = OpTypeVector %bool 4
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%float_4 = OpConstant %float 4
%68 = OpConstantComposite %v2float %float_4 %float_0
%69 = OpConstantComposite %v2float %float_0 %float_4
%70 = OpConstantComposite %mat3v2float %68 %69 %21
%v2bool = OpTypeVector %bool 2
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
%float_5 = OpConstant %float 5
%87 = OpConstantComposite %v4float %float_5 %float_0 %float_0 %float_0
%88 = OpConstantComposite %v4float %float_0 %float_5 %float_0 %float_0
%89 = OpConstantComposite %v4float %float_0 %float_0 %float_5 %float_0
%90 = OpConstantComposite %mat3v4float %87 %88 %89
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_6 = OpConstant %float 6
%106 = OpConstantComposite %v2float %float_6 %float_0
%107 = OpConstantComposite %v2float %float_0 %float_6
%108 = OpConstantComposite %mat4v2float %106 %107 %21 %21
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_7 = OpConstant %float 7
%127 = OpConstantComposite %v3float %float_7 %float_0 %float_0
%128 = OpConstantComposite %v3float %float_0 %float_7 %float_0
%129 = OpConstantComposite %v3float %float_0 %float_0 %float_7
%130 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%131 = OpConstantComposite %mat4v3float %127 %128 %129 %130
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_8 = OpConstant %float 8
%153 = OpConstantComposite %v2float %float_8 %float_0
%154 = OpConstantComposite %v2float %float_0 %float_8
%155 = OpConstantComposite %mat2v2float %153 %154
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_35 = OpConstant %float 35
%171 = OpConstantComposite %v3float %float_35 %float_0 %float_0
%172 = OpConstantComposite %v3float %float_0 %float_35 %float_0
%173 = OpConstantComposite %v3float %float_0 %float_0 %float_35
%174 = OpConstantComposite %mat3v3float %171 %172 %173
%float_1 = OpConstant %float 1
%188 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%189 = OpConstantComposite %mat2v3float %188 %188
%195 = OpConstantComposite %v3float %float_3 %float_1 %float_1
%196 = OpConstantComposite %v3float %float_1 %float_3 %float_1
%197 = OpConstantComposite %mat2v3float %195 %196
%204 = OpConstantComposite %v2float %float_2 %float_2
%205 = OpConstantComposite %mat3v2float %204 %204 %204
%float_n2 = OpConstant %float -2
%213 = OpConstantComposite %v2float %float_2 %float_n2
%214 = OpConstantComposite %v2float %float_n2 %float_2
%215 = OpConstantComposite %v2float %float_n2 %float_n2
%216 = OpConstantComposite %mat3v2float %213 %214 %215
%226 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%227 = OpConstantComposite %mat2v4float %226 %226
%float_0_75 = OpConstant %float 0.75
%234 = OpConstantComposite %v4float %float_0_75 %float_0 %float_0 %float_0
%235 = OpConstantComposite %v4float %float_0 %float_0_75 %float_0 %float_0
%236 = OpConstantComposite %mat2v4float %234 %235
%382 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%test_float_b = OpFunction %bool None %25
%26 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%m23 = OpVariable %_ptr_Function_mat2v3float Function
%m24 = OpVariable %_ptr_Function_mat2v4float Function
%m32 = OpVariable %_ptr_Function_mat3v2float Function
%m34 = OpVariable %_ptr_Function_mat3v4float Function
%m42 = OpVariable %_ptr_Function_mat4v2float Function
%m43 = OpVariable %_ptr_Function_mat4v3float Function
%m22 = OpVariable %_ptr_Function_mat2v2float Function
%m33 = OpVariable %_ptr_Function_mat3v3float Function
OpStore %ok %true
OpStore %m23 %37
OpSelectionMerge %40 None
OpBranchConditional %true %39 %40
%39 = OpLabel
%42 = OpFOrdEqual %v3bool %35 %35
%43 = OpAll %bool %42
%44 = OpFOrdEqual %v3bool %36 %36
%45 = OpAll %bool %44
%46 = OpLogicalAnd %bool %43 %45
OpBranch %40
%40 = OpLabel
%47 = OpPhi %bool %false %26 %46 %39
OpStore %ok %47
OpStore %m24 %54
OpSelectionMerge %56 None
OpBranchConditional %47 %55 %56
%55 = OpLabel
%58 = OpFOrdEqual %v4bool %52 %52
%59 = OpAll %bool %58
%60 = OpFOrdEqual %v4bool %53 %53
%61 = OpAll %bool %60
%62 = OpLogicalAnd %bool %59 %61
OpBranch %56
%56 = OpLabel
%63 = OpPhi %bool %false %40 %62 %55
OpStore %ok %63
OpStore %m32 %70
OpSelectionMerge %72 None
OpBranchConditional %63 %71 %72
%71 = OpLabel
%74 = OpFOrdEqual %v2bool %68 %68
%75 = OpAll %bool %74
%76 = OpFOrdEqual %v2bool %69 %69
%77 = OpAll %bool %76
%78 = OpLogicalAnd %bool %75 %77
%79 = OpFOrdEqual %v2bool %21 %21
%80 = OpAll %bool %79
%81 = OpLogicalAnd %bool %78 %80
OpBranch %72
%72 = OpLabel
%82 = OpPhi %bool %false %56 %81 %71
OpStore %ok %82
OpStore %m34 %90
OpSelectionMerge %92 None
OpBranchConditional %82 %91 %92
%91 = OpLabel
%93 = OpFOrdEqual %v4bool %87 %87
%94 = OpAll %bool %93
%95 = OpFOrdEqual %v4bool %88 %88
%96 = OpAll %bool %95
%97 = OpLogicalAnd %bool %94 %96
%98 = OpFOrdEqual %v4bool %89 %89
%99 = OpAll %bool %98
%100 = OpLogicalAnd %bool %97 %99
OpBranch %92
%92 = OpLabel
%101 = OpPhi %bool %false %72 %100 %91
OpStore %ok %101
OpStore %m42 %108
OpSelectionMerge %110 None
OpBranchConditional %101 %109 %110
%109 = OpLabel
%111 = OpFOrdEqual %v2bool %106 %106
%112 = OpAll %bool %111
%113 = OpFOrdEqual %v2bool %107 %107
%114 = OpAll %bool %113
%115 = OpLogicalAnd %bool %112 %114
%116 = OpFOrdEqual %v2bool %21 %21
%117 = OpAll %bool %116
%118 = OpLogicalAnd %bool %115 %117
%119 = OpFOrdEqual %v2bool %21 %21
%120 = OpAll %bool %119
%121 = OpLogicalAnd %bool %118 %120
OpBranch %110
%110 = OpLabel
%122 = OpPhi %bool %false %92 %121 %109
OpStore %ok %122
OpStore %m43 %131
OpSelectionMerge %133 None
OpBranchConditional %122 %132 %133
%132 = OpLabel
%134 = OpFOrdEqual %v3bool %127 %127
%135 = OpAll %bool %134
%136 = OpFOrdEqual %v3bool %128 %128
%137 = OpAll %bool %136
%138 = OpLogicalAnd %bool %135 %137
%139 = OpFOrdEqual %v3bool %129 %129
%140 = OpAll %bool %139
%141 = OpLogicalAnd %bool %138 %140
%142 = OpFOrdEqual %v3bool %130 %130
%143 = OpAll %bool %142
%144 = OpLogicalAnd %bool %141 %143
OpBranch %133
%133 = OpLabel
%145 = OpPhi %bool %false %110 %144 %132
OpStore %ok %145
%149 = OpMatrixTimesMatrix %mat2v2float %70 %37
OpStore %m22 %149
OpSelectionMerge %151 None
OpBranchConditional %145 %150 %151
%150 = OpLabel
%156 = OpCompositeExtract %v2float %149 0
%157 = OpFOrdEqual %v2bool %156 %153
%158 = OpAll %bool %157
%159 = OpCompositeExtract %v2float %149 1
%160 = OpFOrdEqual %v2bool %159 %154
%161 = OpAll %bool %160
%162 = OpLogicalAnd %bool %158 %161
OpBranch %151
%151 = OpLabel
%163 = OpPhi %bool %false %133 %162 %150
OpStore %ok %163
%167 = OpMatrixTimesMatrix %mat3v3float %131 %90
OpStore %m33 %167
OpSelectionMerge %169 None
OpBranchConditional %163 %168 %169
%168 = OpLabel
%175 = OpCompositeExtract %v3float %167 0
%176 = OpFOrdEqual %v3bool %175 %171
%177 = OpAll %bool %176
%178 = OpCompositeExtract %v3float %167 1
%179 = OpFOrdEqual %v3bool %178 %172
%180 = OpAll %bool %179
%181 = OpLogicalAnd %bool %177 %180
%182 = OpCompositeExtract %v3float %167 2
%183 = OpFOrdEqual %v3bool %182 %173
%184 = OpAll %bool %183
%185 = OpLogicalAnd %bool %181 %184
OpBranch %169
%169 = OpLabel
%186 = OpPhi %bool %false %151 %185 %168
OpStore %ok %186
%190 = OpFAdd %v3float %35 %188
%191 = OpFAdd %v3float %36 %188
%192 = OpCompositeConstruct %mat2v3float %190 %191
OpStore %m23 %192
OpSelectionMerge %194 None
OpBranchConditional %186 %193 %194
%193 = OpLabel
%198 = OpFOrdEqual %v3bool %190 %195
%199 = OpAll %bool %198
%200 = OpFOrdEqual %v3bool %191 %196
%201 = OpAll %bool %200
%202 = OpLogicalAnd %bool %199 %201
OpBranch %194
%194 = OpLabel
%203 = OpPhi %bool %false %169 %202 %193
OpStore %ok %203
%206 = OpFSub %v2float %68 %204
%207 = OpFSub %v2float %69 %204
%208 = OpFSub %v2float %21 %204
%209 = OpCompositeConstruct %mat3v2float %206 %207 %208
OpStore %m32 %209
OpSelectionMerge %211 None
OpBranchConditional %203 %210 %211
%210 = OpLabel
%217 = OpFOrdEqual %v2bool %206 %213
%218 = OpAll %bool %217
%219 = OpFOrdEqual %v2bool %207 %214
%220 = OpAll %bool %219
%221 = OpLogicalAnd %bool %218 %220
%222 = OpFOrdEqual %v2bool %208 %215
%223 = OpAll %bool %222
%224 = OpLogicalAnd %bool %221 %223
OpBranch %211
%211 = OpLabel
%225 = OpPhi %bool %false %194 %224 %210
OpStore %ok %225
%228 = OpFDiv %v4float %52 %226
%229 = OpFDiv %v4float %53 %226
%230 = OpCompositeConstruct %mat2v4float %228 %229
OpStore %m24 %230
OpSelectionMerge %232 None
OpBranchConditional %225 %231 %232
%231 = OpLabel
%237 = OpFOrdEqual %v4bool %228 %234
%238 = OpAll %bool %237
%239 = OpFOrdEqual %v4bool %229 %235
%240 = OpAll %bool %239
%241 = OpLogicalAnd %bool %238 %240
OpBranch %232
%232 = OpLabel
%242 = OpPhi %bool %false %211 %241 %231
OpStore %ok %242
OpReturnValue %242
OpFunctionEnd
%test_half_b = OpFunction %bool None %25
%243 = OpLabel
%ok_0 = OpVariable %_ptr_Function_bool Function
%m23_0 = OpVariable %_ptr_Function_mat2v3float Function
%m24_0 = OpVariable %_ptr_Function_mat2v4float Function
%m32_0 = OpVariable %_ptr_Function_mat3v2float Function
%m34_0 = OpVariable %_ptr_Function_mat3v4float Function
%m42_0 = OpVariable %_ptr_Function_mat4v2float Function
%m43_0 = OpVariable %_ptr_Function_mat4v3float Function
%m22_0 = OpVariable %_ptr_Function_mat2v2float Function
%m33_0 = OpVariable %_ptr_Function_mat3v3float Function
OpStore %ok_0 %true
OpStore %m23_0 %37
OpSelectionMerge %247 None
OpBranchConditional %true %246 %247
%246 = OpLabel
%248 = OpFOrdEqual %v3bool %35 %35
%249 = OpAll %bool %248
%250 = OpFOrdEqual %v3bool %36 %36
%251 = OpAll %bool %250
%252 = OpLogicalAnd %bool %249 %251
OpBranch %247
%247 = OpLabel
%253 = OpPhi %bool %false %243 %252 %246
OpStore %ok_0 %253
OpStore %m24_0 %54
OpSelectionMerge %256 None
OpBranchConditional %253 %255 %256
%255 = OpLabel
%257 = OpFOrdEqual %v4bool %52 %52
%258 = OpAll %bool %257
%259 = OpFOrdEqual %v4bool %53 %53
%260 = OpAll %bool %259
%261 = OpLogicalAnd %bool %258 %260
OpBranch %256
%256 = OpLabel
%262 = OpPhi %bool %false %247 %261 %255
OpStore %ok_0 %262
OpStore %m32_0 %70
OpSelectionMerge %265 None
OpBranchConditional %262 %264 %265
%264 = OpLabel
%266 = OpFOrdEqual %v2bool %68 %68
%267 = OpAll %bool %266
%268 = OpFOrdEqual %v2bool %69 %69
%269 = OpAll %bool %268
%270 = OpLogicalAnd %bool %267 %269
%271 = OpFOrdEqual %v2bool %21 %21
%272 = OpAll %bool %271
%273 = OpLogicalAnd %bool %270 %272
OpBranch %265
%265 = OpLabel
%274 = OpPhi %bool %false %256 %273 %264
OpStore %ok_0 %274
OpStore %m34_0 %90
OpSelectionMerge %277 None
OpBranchConditional %274 %276 %277
%276 = OpLabel
%278 = OpFOrdEqual %v4bool %87 %87
%279 = OpAll %bool %278
%280 = OpFOrdEqual %v4bool %88 %88
%281 = OpAll %bool %280
%282 = OpLogicalAnd %bool %279 %281
%283 = OpFOrdEqual %v4bool %89 %89
%284 = OpAll %bool %283
%285 = OpLogicalAnd %bool %282 %284
OpBranch %277
%277 = OpLabel
%286 = OpPhi %bool %false %265 %285 %276
OpStore %ok_0 %286
OpStore %m42_0 %108
OpSelectionMerge %289 None
OpBranchConditional %286 %288 %289
%288 = OpLabel
%290 = OpFOrdEqual %v2bool %106 %106
%291 = OpAll %bool %290
%292 = OpFOrdEqual %v2bool %107 %107
%293 = OpAll %bool %292
%294 = OpLogicalAnd %bool %291 %293
%295 = OpFOrdEqual %v2bool %21 %21
%296 = OpAll %bool %295
%297 = OpLogicalAnd %bool %294 %296
%298 = OpFOrdEqual %v2bool %21 %21
%299 = OpAll %bool %298
%300 = OpLogicalAnd %bool %297 %299
OpBranch %289
%289 = OpLabel
%301 = OpPhi %bool %false %277 %300 %288
OpStore %ok_0 %301
OpStore %m43_0 %131
OpSelectionMerge %304 None
OpBranchConditional %301 %303 %304
%303 = OpLabel
%305 = OpFOrdEqual %v3bool %127 %127
%306 = OpAll %bool %305
%307 = OpFOrdEqual %v3bool %128 %128
%308 = OpAll %bool %307
%309 = OpLogicalAnd %bool %306 %308
%310 = OpFOrdEqual %v3bool %129 %129
%311 = OpAll %bool %310
%312 = OpLogicalAnd %bool %309 %311
%313 = OpFOrdEqual %v3bool %130 %130
%314 = OpAll %bool %313
%315 = OpLogicalAnd %bool %312 %314
OpBranch %304
%304 = OpLabel
%316 = OpPhi %bool %false %289 %315 %303
OpStore %ok_0 %316
%318 = OpMatrixTimesMatrix %mat2v2float %70 %37
OpStore %m22_0 %318
OpSelectionMerge %320 None
OpBranchConditional %316 %319 %320
%319 = OpLabel
%321 = OpCompositeExtract %v2float %318 0
%322 = OpFOrdEqual %v2bool %321 %153
%323 = OpAll %bool %322
%324 = OpCompositeExtract %v2float %318 1
%325 = OpFOrdEqual %v2bool %324 %154
%326 = OpAll %bool %325
%327 = OpLogicalAnd %bool %323 %326
OpBranch %320
%320 = OpLabel
%328 = OpPhi %bool %false %304 %327 %319
OpStore %ok_0 %328
%330 = OpMatrixTimesMatrix %mat3v3float %131 %90
OpStore %m33_0 %330
OpSelectionMerge %332 None
OpBranchConditional %328 %331 %332
%331 = OpLabel
%333 = OpCompositeExtract %v3float %330 0
%334 = OpFOrdEqual %v3bool %333 %171
%335 = OpAll %bool %334
%336 = OpCompositeExtract %v3float %330 1
%337 = OpFOrdEqual %v3bool %336 %172
%338 = OpAll %bool %337
%339 = OpLogicalAnd %bool %335 %338
%340 = OpCompositeExtract %v3float %330 2
%341 = OpFOrdEqual %v3bool %340 %173
%342 = OpAll %bool %341
%343 = OpLogicalAnd %bool %339 %342
OpBranch %332
%332 = OpLabel
%344 = OpPhi %bool %false %320 %343 %331
OpStore %ok_0 %344
%345 = OpFAdd %v3float %35 %188
%346 = OpFAdd %v3float %36 %188
%347 = OpCompositeConstruct %mat2v3float %345 %346
OpStore %m23_0 %347
OpSelectionMerge %349 None
OpBranchConditional %344 %348 %349
%348 = OpLabel
%350 = OpFOrdEqual %v3bool %345 %195
%351 = OpAll %bool %350
%352 = OpFOrdEqual %v3bool %346 %196
%353 = OpAll %bool %352
%354 = OpLogicalAnd %bool %351 %353
OpBranch %349
%349 = OpLabel
%355 = OpPhi %bool %false %332 %354 %348
OpStore %ok_0 %355
%356 = OpFSub %v2float %68 %204
%357 = OpFSub %v2float %69 %204
%358 = OpFSub %v2float %21 %204
%359 = OpCompositeConstruct %mat3v2float %356 %357 %358
OpStore %m32_0 %359
OpSelectionMerge %361 None
OpBranchConditional %355 %360 %361
%360 = OpLabel
%362 = OpFOrdEqual %v2bool %356 %213
%363 = OpAll %bool %362
%364 = OpFOrdEqual %v2bool %357 %214
%365 = OpAll %bool %364
%366 = OpLogicalAnd %bool %363 %365
%367 = OpFOrdEqual %v2bool %358 %215
%368 = OpAll %bool %367
%369 = OpLogicalAnd %bool %366 %368
OpBranch %361
%361 = OpLabel
%370 = OpPhi %bool %false %349 %369 %360
OpStore %ok_0 %370
%371 = OpFDiv %v4float %52 %226
%372 = OpFDiv %v4float %53 %226
%373 = OpCompositeConstruct %mat2v4float %371 %372
OpStore %m24_0 %373
OpSelectionMerge %375 None
OpBranchConditional %370 %374 %375
%374 = OpLabel
%376 = OpFOrdEqual %v4bool %371 %234
%377 = OpAll %bool %376
%378 = OpFOrdEqual %v4bool %372 %235
%379 = OpAll %bool %378
%380 = OpLogicalAnd %bool %377 %379
OpBranch %375
%375 = OpLabel
%381 = OpPhi %bool %false %361 %380 %374
OpStore %ok_0 %381
OpReturnValue %381
OpFunctionEnd
%main = OpFunction %v4float None %382
%383 = OpFunctionParameter %_ptr_Function_v2float
%384 = OpLabel
%390 = OpVariable %_ptr_Function_v4float Function
%385 = OpFunctionCall %bool %test_float_b
OpSelectionMerge %387 None
OpBranchConditional %385 %386 %387
%386 = OpLabel
%388 = OpFunctionCall %bool %test_half_b
OpBranch %387
%387 = OpLabel
%389 = OpPhi %bool %false %384 %388 %386
OpSelectionMerge %394 None
OpBranchConditional %389 %392 %393
%392 = OpLabel
%395 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%399 = OpLoad %v4float %395
OpStore %390 %399
OpBranch %394
%393 = OpLabel
%400 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%402 = OpLoad %v4float %400
OpStore %390 %402
OpBranch %394
%394 = OpLabel
%403 = OpLoad %v4float %390
OpReturnValue %403
OpFunctionEnd
