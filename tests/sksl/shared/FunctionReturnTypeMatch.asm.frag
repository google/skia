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
OpName %returns_float2_f2 "returns_float2_f2"
OpName %returns_float3_f3 "returns_float3_f3"
OpName %returns_float4_f4 "returns_float4_f4"
OpName %returns_float2x2_f22 "returns_float2x2_f22"
OpName %returns_float3x3_f33 "returns_float3x3_f33"
OpName %returns_float4x4_f44 "returns_float4x4_f44"
OpName %returns_half_h "returns_half_h"
OpName %returns_half2_h2 "returns_half2_h2"
OpName %returns_half3_h3 "returns_half3_h3"
OpName %returns_half4_h4 "returns_half4_h4"
OpName %returns_half2x2_h22 "returns_half2x2_h22"
OpName %returns_half3x3_h33 "returns_half3x3_h33"
OpName %returns_half4x4_h44 "returns_half4x4_h44"
OpName %returns_bool_b "returns_bool_b"
OpName %returns_bool2_b2 "returns_bool2_b2"
OpName %returns_bool3_b3 "returns_bool3_b3"
OpName %returns_bool4_b4 "returns_bool4_b4"
OpName %returns_int_i "returns_int_i"
OpName %returns_int2_i2 "returns_int2_i2"
OpName %returns_int3_i3 "returns_int3_i3"
OpName %returns_int4_i4 "returns_int4_i4"
OpName %main "main"
OpName %x1 "x1"
OpName %x2 "x2"
OpName %x3 "x3"
OpName %x4 "x4"
OpName %x5 "x5"
OpName %x6 "x6"
OpName %x7 "x7"
OpName %x8 "x8"
OpName %x9 "x9"
OpName %x10 "x10"
OpName %x11 "x11"
OpName %x12 "x12"
OpName %x13 "x13"
OpName %x14 "x14"
OpName %x15 "x15"
OpName %x16 "x16"
OpName %x17 "x17"
OpName %x18 "x18"
OpName %x19 "x19"
OpName %x20 "x20"
OpName %x21 "x21"
OpName %x22 "x22"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %31 Binding 0
OpDecorate %31 DescriptorSet 0
OpDecorate %x8 RelaxedPrecision
OpDecorate %x9 RelaxedPrecision
OpDecorate %x10 RelaxedPrecision
OpDecorate %x11 RelaxedPrecision
OpDecorate %x12 RelaxedPrecision
OpDecorate %x13 RelaxedPrecision
OpDecorate %x14 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %389 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%31 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%36 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%40 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%44 = OpTypeFunction %v2float
%float_2 = OpConstant %float 2
%47 = OpConstantComposite %v2float %float_2 %float_2
%v3float = OpTypeVector %float 3
%49 = OpTypeFunction %v3float
%float_3 = OpConstant %float 3
%52 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%53 = OpTypeFunction %v4float
%float_4 = OpConstant %float 4
%56 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%mat2v2float = OpTypeMatrix %v2float 2
%58 = OpTypeFunction %mat2v2float
%60 = OpConstantComposite %v2float %float_2 %float_0
%61 = OpConstantComposite %v2float %float_0 %float_2
%62 = OpConstantComposite %mat2v2float %60 %61
%mat3v3float = OpTypeMatrix %v3float 3
%64 = OpTypeFunction %mat3v3float
%66 = OpConstantComposite %v3float %float_3 %float_0 %float_0
%67 = OpConstantComposite %v3float %float_0 %float_3 %float_0
%68 = OpConstantComposite %v3float %float_0 %float_0 %float_3
%69 = OpConstantComposite %mat3v3float %66 %67 %68
%mat4v4float = OpTypeMatrix %v4float 4
%71 = OpTypeFunction %mat4v4float
%73 = OpConstantComposite %v4float %float_4 %float_0 %float_0 %float_0
%74 = OpConstantComposite %v4float %float_0 %float_4 %float_0 %float_0
%75 = OpConstantComposite %v4float %float_0 %float_0 %float_4 %float_0
%76 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_4
%77 = OpConstantComposite %mat4v4float %73 %74 %75 %76
%78 = OpTypeFunction %float
%float_1 = OpConstant %float 1
%87 = OpTypeFunction %bool
%true = OpConstantTrue %bool
%v2bool = OpTypeVector %bool 2
%91 = OpTypeFunction %v2bool
%93 = OpConstantComposite %v2bool %true %true
%v3bool = OpTypeVector %bool 3
%95 = OpTypeFunction %v3bool
%97 = OpConstantComposite %v3bool %true %true %true
%v4bool = OpTypeVector %bool 4
%99 = OpTypeFunction %v4bool
%101 = OpConstantComposite %v4bool %true %true %true %true
%int = OpTypeInt 32 1
%103 = OpTypeFunction %int
%int_1 = OpConstant %int 1
%v2int = OpTypeVector %int 2
%107 = OpTypeFunction %v2int
%int_2 = OpConstant %int 2
%110 = OpConstantComposite %v2int %int_2 %int_2
%v3int = OpTypeVector %int 3
%112 = OpTypeFunction %v3int
%int_3 = OpConstant %int 3
%115 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%v4int = OpTypeVector %int 4
%117 = OpTypeFunction %v4int
%int_4 = OpConstant %int 4
%120 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%121 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_v2int = OpTypePointer Function %v2int
%_ptr_Function_v3int = OpTypePointer Function %v3int
%_ptr_Function_v4int = OpTypePointer Function %v4int
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %36
%37 = OpLabel
%41 = OpVariable %_ptr_Function_v2float Function
OpStore %41 %40
%43 = OpFunctionCall %v4float %main %41
OpStore %sk_FragColor %43
OpReturn
OpFunctionEnd
%returns_float2_f2 = OpFunction %v2float None %44
%45 = OpLabel
OpReturnValue %47
OpFunctionEnd
%returns_float3_f3 = OpFunction %v3float None %49
%50 = OpLabel
OpReturnValue %52
OpFunctionEnd
%returns_float4_f4 = OpFunction %v4float None %53
%54 = OpLabel
OpReturnValue %56
OpFunctionEnd
%returns_float2x2_f22 = OpFunction %mat2v2float None %58
%59 = OpLabel
OpReturnValue %62
OpFunctionEnd
%returns_float3x3_f33 = OpFunction %mat3v3float None %64
%65 = OpLabel
OpReturnValue %69
OpFunctionEnd
%returns_float4x4_f44 = OpFunction %mat4v4float None %71
%72 = OpLabel
OpReturnValue %77
OpFunctionEnd
%returns_half_h = OpFunction %float None %78
%79 = OpLabel
OpReturnValue %float_1
OpFunctionEnd
%returns_half2_h2 = OpFunction %v2float None %44
%81 = OpLabel
OpReturnValue %47
OpFunctionEnd
%returns_half3_h3 = OpFunction %v3float None %49
%82 = OpLabel
OpReturnValue %52
OpFunctionEnd
%returns_half4_h4 = OpFunction %v4float None %53
%83 = OpLabel
OpReturnValue %56
OpFunctionEnd
%returns_half2x2_h22 = OpFunction %mat2v2float None %58
%84 = OpLabel
OpReturnValue %62
OpFunctionEnd
%returns_half3x3_h33 = OpFunction %mat3v3float None %64
%85 = OpLabel
OpReturnValue %69
OpFunctionEnd
%returns_half4x4_h44 = OpFunction %mat4v4float None %71
%86 = OpLabel
OpReturnValue %77
OpFunctionEnd
%returns_bool_b = OpFunction %bool None %87
%88 = OpLabel
OpReturnValue %true
OpFunctionEnd
%returns_bool2_b2 = OpFunction %v2bool None %91
%92 = OpLabel
OpReturnValue %93
OpFunctionEnd
%returns_bool3_b3 = OpFunction %v3bool None %95
%96 = OpLabel
OpReturnValue %97
OpFunctionEnd
%returns_bool4_b4 = OpFunction %v4bool None %99
%100 = OpLabel
OpReturnValue %101
OpFunctionEnd
%returns_int_i = OpFunction %int None %103
%104 = OpLabel
OpReturnValue %int_1
OpFunctionEnd
%returns_int2_i2 = OpFunction %v2int None %107
%108 = OpLabel
OpReturnValue %110
OpFunctionEnd
%returns_int3_i3 = OpFunction %v3int None %112
%113 = OpLabel
OpReturnValue %115
OpFunctionEnd
%returns_int4_i4 = OpFunction %v4int None %117
%118 = OpLabel
OpReturnValue %120
OpFunctionEnd
%main = OpFunction %v4float None %121
%122 = OpFunctionParameter %_ptr_Function_v2float
%123 = OpLabel
%x1 = OpVariable %_ptr_Function_float Function
%x2 = OpVariable %_ptr_Function_v2float Function
%x3 = OpVariable %_ptr_Function_v3float Function
%x4 = OpVariable %_ptr_Function_v4float Function
%x5 = OpVariable %_ptr_Function_mat2v2float Function
%x6 = OpVariable %_ptr_Function_mat3v3float Function
%x7 = OpVariable %_ptr_Function_mat4v4float Function
%x8 = OpVariable %_ptr_Function_float Function
%x9 = OpVariable %_ptr_Function_v2float Function
%x10 = OpVariable %_ptr_Function_v3float Function
%x11 = OpVariable %_ptr_Function_v4float Function
%x12 = OpVariable %_ptr_Function_mat2v2float Function
%x13 = OpVariable %_ptr_Function_mat3v3float Function
%x14 = OpVariable %_ptr_Function_mat4v4float Function
%x15 = OpVariable %_ptr_Function_bool Function
%x16 = OpVariable %_ptr_Function_v2bool Function
%x17 = OpVariable %_ptr_Function_v3bool Function
%x18 = OpVariable %_ptr_Function_v4bool Function
%x19 = OpVariable %_ptr_Function_int Function
%x20 = OpVariable %_ptr_Function_v2int Function
%x21 = OpVariable %_ptr_Function_v3int Function
%x22 = OpVariable %_ptr_Function_v4int Function
%379 = OpVariable %_ptr_Function_v4float Function
OpStore %x1 %float_1
OpStore %x2 %47
OpStore %x3 %52
OpStore %x4 %56
OpStore %x5 %62
OpStore %x6 %69
OpStore %x7 %77
OpStore %x8 %float_1
OpStore %x9 %47
OpStore %x10 %52
OpStore %x11 %56
OpStore %x12 %62
OpStore %x13 %69
OpStore %x14 %77
OpStore %x15 %true
OpStore %x16 %93
OpStore %x17 %97
OpStore %x18 %101
OpStore %x19 %int_1
OpStore %x20 %110
OpStore %x21 %115
OpStore %x22 %120
%161 = OpLoad %float %x1
%162 = OpFOrdEqual %bool %161 %float_1
OpSelectionMerge %164 None
OpBranchConditional %162 %163 %164
%163 = OpLabel
%165 = OpLoad %v2float %x2
%166 = OpFunctionCall %v2float %returns_float2_f2
%167 = OpFOrdEqual %v2bool %165 %166
%168 = OpAll %bool %167
OpBranch %164
%164 = OpLabel
%169 = OpPhi %bool %false %123 %168 %163
OpSelectionMerge %171 None
OpBranchConditional %169 %170 %171
%170 = OpLabel
%172 = OpLoad %v3float %x3
%173 = OpFunctionCall %v3float %returns_float3_f3
%174 = OpFOrdEqual %v3bool %172 %173
%175 = OpAll %bool %174
OpBranch %171
%171 = OpLabel
%176 = OpPhi %bool %false %164 %175 %170
OpSelectionMerge %178 None
OpBranchConditional %176 %177 %178
%177 = OpLabel
%179 = OpLoad %v4float %x4
%180 = OpFunctionCall %v4float %returns_float4_f4
%181 = OpFOrdEqual %v4bool %179 %180
%182 = OpAll %bool %181
OpBranch %178
%178 = OpLabel
%183 = OpPhi %bool %false %171 %182 %177
OpSelectionMerge %185 None
OpBranchConditional %183 %184 %185
%184 = OpLabel
%186 = OpLoad %mat2v2float %x5
%187 = OpFunctionCall %mat2v2float %returns_float2x2_f22
%188 = OpCompositeExtract %v2float %186 0
%189 = OpCompositeExtract %v2float %187 0
%190 = OpFOrdEqual %v2bool %188 %189
%191 = OpAll %bool %190
%192 = OpCompositeExtract %v2float %186 1
%193 = OpCompositeExtract %v2float %187 1
%194 = OpFOrdEqual %v2bool %192 %193
%195 = OpAll %bool %194
%196 = OpLogicalAnd %bool %191 %195
OpBranch %185
%185 = OpLabel
%197 = OpPhi %bool %false %178 %196 %184
OpSelectionMerge %199 None
OpBranchConditional %197 %198 %199
%198 = OpLabel
%200 = OpLoad %mat3v3float %x6
%201 = OpFunctionCall %mat3v3float %returns_float3x3_f33
%202 = OpCompositeExtract %v3float %200 0
%203 = OpCompositeExtract %v3float %201 0
%204 = OpFOrdEqual %v3bool %202 %203
%205 = OpAll %bool %204
%206 = OpCompositeExtract %v3float %200 1
%207 = OpCompositeExtract %v3float %201 1
%208 = OpFOrdEqual %v3bool %206 %207
%209 = OpAll %bool %208
%210 = OpLogicalAnd %bool %205 %209
%211 = OpCompositeExtract %v3float %200 2
%212 = OpCompositeExtract %v3float %201 2
%213 = OpFOrdEqual %v3bool %211 %212
%214 = OpAll %bool %213
%215 = OpLogicalAnd %bool %210 %214
OpBranch %199
%199 = OpLabel
%216 = OpPhi %bool %false %185 %215 %198
OpSelectionMerge %218 None
OpBranchConditional %216 %217 %218
%217 = OpLabel
%219 = OpLoad %mat4v4float %x7
%220 = OpFunctionCall %mat4v4float %returns_float4x4_f44
%221 = OpCompositeExtract %v4float %219 0
%222 = OpCompositeExtract %v4float %220 0
%223 = OpFOrdEqual %v4bool %221 %222
%224 = OpAll %bool %223
%225 = OpCompositeExtract %v4float %219 1
%226 = OpCompositeExtract %v4float %220 1
%227 = OpFOrdEqual %v4bool %225 %226
%228 = OpAll %bool %227
%229 = OpLogicalAnd %bool %224 %228
%230 = OpCompositeExtract %v4float %219 2
%231 = OpCompositeExtract %v4float %220 2
%232 = OpFOrdEqual %v4bool %230 %231
%233 = OpAll %bool %232
%234 = OpLogicalAnd %bool %229 %233
%235 = OpCompositeExtract %v4float %219 3
%236 = OpCompositeExtract %v4float %220 3
%237 = OpFOrdEqual %v4bool %235 %236
%238 = OpAll %bool %237
%239 = OpLogicalAnd %bool %234 %238
OpBranch %218
%218 = OpLabel
%240 = OpPhi %bool %false %199 %239 %217
OpSelectionMerge %242 None
OpBranchConditional %240 %241 %242
%241 = OpLabel
%243 = OpLoad %float %x8
%244 = OpFunctionCall %float %returns_half_h
%245 = OpFOrdEqual %bool %243 %244
OpBranch %242
%242 = OpLabel
%246 = OpPhi %bool %false %218 %245 %241
OpSelectionMerge %248 None
OpBranchConditional %246 %247 %248
%247 = OpLabel
%249 = OpLoad %v2float %x9
%250 = OpFunctionCall %v2float %returns_half2_h2
%251 = OpFOrdEqual %v2bool %249 %250
%252 = OpAll %bool %251
OpBranch %248
%248 = OpLabel
%253 = OpPhi %bool %false %242 %252 %247
OpSelectionMerge %255 None
OpBranchConditional %253 %254 %255
%254 = OpLabel
%256 = OpLoad %v3float %x10
%257 = OpFunctionCall %v3float %returns_half3_h3
%258 = OpFOrdEqual %v3bool %256 %257
%259 = OpAll %bool %258
OpBranch %255
%255 = OpLabel
%260 = OpPhi %bool %false %248 %259 %254
OpSelectionMerge %262 None
OpBranchConditional %260 %261 %262
%261 = OpLabel
%263 = OpLoad %v4float %x11
%264 = OpFunctionCall %v4float %returns_half4_h4
%265 = OpFOrdEqual %v4bool %263 %264
%266 = OpAll %bool %265
OpBranch %262
%262 = OpLabel
%267 = OpPhi %bool %false %255 %266 %261
OpSelectionMerge %269 None
OpBranchConditional %267 %268 %269
%268 = OpLabel
%270 = OpLoad %mat2v2float %x12
%271 = OpFunctionCall %mat2v2float %returns_half2x2_h22
%272 = OpCompositeExtract %v2float %270 0
%273 = OpCompositeExtract %v2float %271 0
%274 = OpFOrdEqual %v2bool %272 %273
%275 = OpAll %bool %274
%276 = OpCompositeExtract %v2float %270 1
%277 = OpCompositeExtract %v2float %271 1
%278 = OpFOrdEqual %v2bool %276 %277
%279 = OpAll %bool %278
%280 = OpLogicalAnd %bool %275 %279
OpBranch %269
%269 = OpLabel
%281 = OpPhi %bool %false %262 %280 %268
OpSelectionMerge %283 None
OpBranchConditional %281 %282 %283
%282 = OpLabel
%284 = OpLoad %mat3v3float %x13
%285 = OpFunctionCall %mat3v3float %returns_half3x3_h33
%286 = OpCompositeExtract %v3float %284 0
%287 = OpCompositeExtract %v3float %285 0
%288 = OpFOrdEqual %v3bool %286 %287
%289 = OpAll %bool %288
%290 = OpCompositeExtract %v3float %284 1
%291 = OpCompositeExtract %v3float %285 1
%292 = OpFOrdEqual %v3bool %290 %291
%293 = OpAll %bool %292
%294 = OpLogicalAnd %bool %289 %293
%295 = OpCompositeExtract %v3float %284 2
%296 = OpCompositeExtract %v3float %285 2
%297 = OpFOrdEqual %v3bool %295 %296
%298 = OpAll %bool %297
%299 = OpLogicalAnd %bool %294 %298
OpBranch %283
%283 = OpLabel
%300 = OpPhi %bool %false %269 %299 %282
OpSelectionMerge %302 None
OpBranchConditional %300 %301 %302
%301 = OpLabel
%303 = OpLoad %mat4v4float %x14
%304 = OpFunctionCall %mat4v4float %returns_half4x4_h44
%305 = OpCompositeExtract %v4float %303 0
%306 = OpCompositeExtract %v4float %304 0
%307 = OpFOrdEqual %v4bool %305 %306
%308 = OpAll %bool %307
%309 = OpCompositeExtract %v4float %303 1
%310 = OpCompositeExtract %v4float %304 1
%311 = OpFOrdEqual %v4bool %309 %310
%312 = OpAll %bool %311
%313 = OpLogicalAnd %bool %308 %312
%314 = OpCompositeExtract %v4float %303 2
%315 = OpCompositeExtract %v4float %304 2
%316 = OpFOrdEqual %v4bool %314 %315
%317 = OpAll %bool %316
%318 = OpLogicalAnd %bool %313 %317
%319 = OpCompositeExtract %v4float %303 3
%320 = OpCompositeExtract %v4float %304 3
%321 = OpFOrdEqual %v4bool %319 %320
%322 = OpAll %bool %321
%323 = OpLogicalAnd %bool %318 %322
OpBranch %302
%302 = OpLabel
%324 = OpPhi %bool %false %283 %323 %301
OpSelectionMerge %326 None
OpBranchConditional %324 %325 %326
%325 = OpLabel
%327 = OpLoad %bool %x15
%328 = OpFunctionCall %bool %returns_bool_b
%329 = OpLogicalEqual %bool %327 %328
OpBranch %326
%326 = OpLabel
%330 = OpPhi %bool %false %302 %329 %325
OpSelectionMerge %332 None
OpBranchConditional %330 %331 %332
%331 = OpLabel
%333 = OpLoad %v2bool %x16
%334 = OpFunctionCall %v2bool %returns_bool2_b2
%335 = OpLogicalEqual %v2bool %333 %334
%336 = OpAll %bool %335
OpBranch %332
%332 = OpLabel
%337 = OpPhi %bool %false %326 %336 %331
OpSelectionMerge %339 None
OpBranchConditional %337 %338 %339
%338 = OpLabel
%340 = OpLoad %v3bool %x17
%341 = OpFunctionCall %v3bool %returns_bool3_b3
%342 = OpLogicalEqual %v3bool %340 %341
%343 = OpAll %bool %342
OpBranch %339
%339 = OpLabel
%344 = OpPhi %bool %false %332 %343 %338
OpSelectionMerge %346 None
OpBranchConditional %344 %345 %346
%345 = OpLabel
%347 = OpLoad %v4bool %x18
%348 = OpFunctionCall %v4bool %returns_bool4_b4
%349 = OpLogicalEqual %v4bool %347 %348
%350 = OpAll %bool %349
OpBranch %346
%346 = OpLabel
%351 = OpPhi %bool %false %339 %350 %345
OpSelectionMerge %353 None
OpBranchConditional %351 %352 %353
%352 = OpLabel
%354 = OpLoad %int %x19
%355 = OpFunctionCall %int %returns_int_i
%356 = OpIEqual %bool %354 %355
OpBranch %353
%353 = OpLabel
%357 = OpPhi %bool %false %346 %356 %352
OpSelectionMerge %359 None
OpBranchConditional %357 %358 %359
%358 = OpLabel
%360 = OpLoad %v2int %x20
%361 = OpFunctionCall %v2int %returns_int2_i2
%362 = OpIEqual %v2bool %360 %361
%363 = OpAll %bool %362
OpBranch %359
%359 = OpLabel
%364 = OpPhi %bool %false %353 %363 %358
OpSelectionMerge %366 None
OpBranchConditional %364 %365 %366
%365 = OpLabel
%367 = OpLoad %v3int %x21
%368 = OpFunctionCall %v3int %returns_int3_i3
%369 = OpIEqual %v3bool %367 %368
%370 = OpAll %bool %369
OpBranch %366
%366 = OpLabel
%371 = OpPhi %bool %false %359 %370 %365
OpSelectionMerge %373 None
OpBranchConditional %371 %372 %373
%372 = OpLabel
%374 = OpLoad %v4int %x22
%375 = OpFunctionCall %v4int %returns_int4_i4
%376 = OpIEqual %v4bool %374 %375
%377 = OpAll %bool %376
OpBranch %373
%373 = OpLabel
%378 = OpPhi %bool %false %366 %377 %372
OpSelectionMerge %382 None
OpBranchConditional %378 %380 %381
%380 = OpLabel
%383 = OpAccessChain %_ptr_Uniform_v4float %31 %int_0
%386 = OpLoad %v4float %383
OpStore %379 %386
OpBranch %382
%381 = OpLabel
%387 = OpAccessChain %_ptr_Uniform_v4float %31 %int_1
%388 = OpLoad %v4float %387
OpStore %379 %388
OpBranch %382
%382 = OpLabel
%389 = OpLoad %v4float %379
OpReturnValue %389
OpFunctionEnd
