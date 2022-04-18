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
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
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
%339 = OpVariable %_ptr_Function_v4float Function
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
%161 = OpFOrdEqual %bool %float_1 %float_1
OpSelectionMerge %163 None
OpBranchConditional %161 %162 %163
%162 = OpLabel
%164 = OpFunctionCall %v2float %returns_float2_f2
%165 = OpFOrdEqual %v2bool %47 %164
%166 = OpAll %bool %165
OpBranch %163
%163 = OpLabel
%167 = OpPhi %bool %false %123 %166 %162
OpSelectionMerge %169 None
OpBranchConditional %167 %168 %169
%168 = OpLabel
%170 = OpFunctionCall %v3float %returns_float3_f3
%171 = OpFOrdEqual %v3bool %52 %170
%172 = OpAll %bool %171
OpBranch %169
%169 = OpLabel
%173 = OpPhi %bool %false %163 %172 %168
OpSelectionMerge %175 None
OpBranchConditional %173 %174 %175
%174 = OpLabel
%176 = OpFunctionCall %v4float %returns_float4_f4
%177 = OpFOrdEqual %v4bool %56 %176
%178 = OpAll %bool %177
OpBranch %175
%175 = OpLabel
%179 = OpPhi %bool %false %169 %178 %174
OpSelectionMerge %181 None
OpBranchConditional %179 %180 %181
%180 = OpLabel
%182 = OpFunctionCall %mat2v2float %returns_float2x2_f22
%183 = OpCompositeExtract %v2float %182 0
%184 = OpFOrdEqual %v2bool %60 %183
%185 = OpAll %bool %184
%186 = OpCompositeExtract %v2float %182 1
%187 = OpFOrdEqual %v2bool %61 %186
%188 = OpAll %bool %187
%189 = OpLogicalAnd %bool %185 %188
OpBranch %181
%181 = OpLabel
%190 = OpPhi %bool %false %175 %189 %180
OpSelectionMerge %192 None
OpBranchConditional %190 %191 %192
%191 = OpLabel
%193 = OpFunctionCall %mat3v3float %returns_float3x3_f33
%194 = OpCompositeExtract %v3float %193 0
%195 = OpFOrdEqual %v3bool %66 %194
%196 = OpAll %bool %195
%197 = OpCompositeExtract %v3float %193 1
%198 = OpFOrdEqual %v3bool %67 %197
%199 = OpAll %bool %198
%200 = OpLogicalAnd %bool %196 %199
%201 = OpCompositeExtract %v3float %193 2
%202 = OpFOrdEqual %v3bool %68 %201
%203 = OpAll %bool %202
%204 = OpLogicalAnd %bool %200 %203
OpBranch %192
%192 = OpLabel
%205 = OpPhi %bool %false %181 %204 %191
OpSelectionMerge %207 None
OpBranchConditional %205 %206 %207
%206 = OpLabel
%208 = OpFunctionCall %mat4v4float %returns_float4x4_f44
%209 = OpCompositeExtract %v4float %208 0
%210 = OpFOrdEqual %v4bool %73 %209
%211 = OpAll %bool %210
%212 = OpCompositeExtract %v4float %208 1
%213 = OpFOrdEqual %v4bool %74 %212
%214 = OpAll %bool %213
%215 = OpLogicalAnd %bool %211 %214
%216 = OpCompositeExtract %v4float %208 2
%217 = OpFOrdEqual %v4bool %75 %216
%218 = OpAll %bool %217
%219 = OpLogicalAnd %bool %215 %218
%220 = OpCompositeExtract %v4float %208 3
%221 = OpFOrdEqual %v4bool %76 %220
%222 = OpAll %bool %221
%223 = OpLogicalAnd %bool %219 %222
OpBranch %207
%207 = OpLabel
%224 = OpPhi %bool %false %192 %223 %206
OpSelectionMerge %226 None
OpBranchConditional %224 %225 %226
%225 = OpLabel
%227 = OpFunctionCall %float %returns_half_h
%228 = OpFOrdEqual %bool %float_1 %227
OpBranch %226
%226 = OpLabel
%229 = OpPhi %bool %false %207 %228 %225
OpSelectionMerge %231 None
OpBranchConditional %229 %230 %231
%230 = OpLabel
%232 = OpFunctionCall %v2float %returns_half2_h2
%233 = OpFOrdEqual %v2bool %47 %232
%234 = OpAll %bool %233
OpBranch %231
%231 = OpLabel
%235 = OpPhi %bool %false %226 %234 %230
OpSelectionMerge %237 None
OpBranchConditional %235 %236 %237
%236 = OpLabel
%238 = OpFunctionCall %v3float %returns_half3_h3
%239 = OpFOrdEqual %v3bool %52 %238
%240 = OpAll %bool %239
OpBranch %237
%237 = OpLabel
%241 = OpPhi %bool %false %231 %240 %236
OpSelectionMerge %243 None
OpBranchConditional %241 %242 %243
%242 = OpLabel
%244 = OpFunctionCall %v4float %returns_half4_h4
%245 = OpFOrdEqual %v4bool %56 %244
%246 = OpAll %bool %245
OpBranch %243
%243 = OpLabel
%247 = OpPhi %bool %false %237 %246 %242
OpSelectionMerge %249 None
OpBranchConditional %247 %248 %249
%248 = OpLabel
%250 = OpFunctionCall %mat2v2float %returns_half2x2_h22
%251 = OpCompositeExtract %v2float %250 0
%252 = OpFOrdEqual %v2bool %60 %251
%253 = OpAll %bool %252
%254 = OpCompositeExtract %v2float %250 1
%255 = OpFOrdEqual %v2bool %61 %254
%256 = OpAll %bool %255
%257 = OpLogicalAnd %bool %253 %256
OpBranch %249
%249 = OpLabel
%258 = OpPhi %bool %false %243 %257 %248
OpSelectionMerge %260 None
OpBranchConditional %258 %259 %260
%259 = OpLabel
%261 = OpFunctionCall %mat3v3float %returns_half3x3_h33
%262 = OpCompositeExtract %v3float %261 0
%263 = OpFOrdEqual %v3bool %66 %262
%264 = OpAll %bool %263
%265 = OpCompositeExtract %v3float %261 1
%266 = OpFOrdEqual %v3bool %67 %265
%267 = OpAll %bool %266
%268 = OpLogicalAnd %bool %264 %267
%269 = OpCompositeExtract %v3float %261 2
%270 = OpFOrdEqual %v3bool %68 %269
%271 = OpAll %bool %270
%272 = OpLogicalAnd %bool %268 %271
OpBranch %260
%260 = OpLabel
%273 = OpPhi %bool %false %249 %272 %259
OpSelectionMerge %275 None
OpBranchConditional %273 %274 %275
%274 = OpLabel
%276 = OpFunctionCall %mat4v4float %returns_half4x4_h44
%277 = OpCompositeExtract %v4float %276 0
%278 = OpFOrdEqual %v4bool %73 %277
%279 = OpAll %bool %278
%280 = OpCompositeExtract %v4float %276 1
%281 = OpFOrdEqual %v4bool %74 %280
%282 = OpAll %bool %281
%283 = OpLogicalAnd %bool %279 %282
%284 = OpCompositeExtract %v4float %276 2
%285 = OpFOrdEqual %v4bool %75 %284
%286 = OpAll %bool %285
%287 = OpLogicalAnd %bool %283 %286
%288 = OpCompositeExtract %v4float %276 3
%289 = OpFOrdEqual %v4bool %76 %288
%290 = OpAll %bool %289
%291 = OpLogicalAnd %bool %287 %290
OpBranch %275
%275 = OpLabel
%292 = OpPhi %bool %false %260 %291 %274
OpSelectionMerge %294 None
OpBranchConditional %292 %293 %294
%293 = OpLabel
%295 = OpFunctionCall %bool %returns_bool_b
%296 = OpLogicalEqual %bool %true %295
OpBranch %294
%294 = OpLabel
%297 = OpPhi %bool %false %275 %296 %293
OpSelectionMerge %299 None
OpBranchConditional %297 %298 %299
%298 = OpLabel
%300 = OpFunctionCall %v2bool %returns_bool2_b2
%301 = OpLogicalEqual %v2bool %93 %300
%302 = OpAll %bool %301
OpBranch %299
%299 = OpLabel
%303 = OpPhi %bool %false %294 %302 %298
OpSelectionMerge %305 None
OpBranchConditional %303 %304 %305
%304 = OpLabel
%306 = OpFunctionCall %v3bool %returns_bool3_b3
%307 = OpLogicalEqual %v3bool %97 %306
%308 = OpAll %bool %307
OpBranch %305
%305 = OpLabel
%309 = OpPhi %bool %false %299 %308 %304
OpSelectionMerge %311 None
OpBranchConditional %309 %310 %311
%310 = OpLabel
%312 = OpFunctionCall %v4bool %returns_bool4_b4
%313 = OpLogicalEqual %v4bool %101 %312
%314 = OpAll %bool %313
OpBranch %311
%311 = OpLabel
%315 = OpPhi %bool %false %305 %314 %310
OpSelectionMerge %317 None
OpBranchConditional %315 %316 %317
%316 = OpLabel
%318 = OpFunctionCall %int %returns_int_i
%319 = OpIEqual %bool %int_1 %318
OpBranch %317
%317 = OpLabel
%320 = OpPhi %bool %false %311 %319 %316
OpSelectionMerge %322 None
OpBranchConditional %320 %321 %322
%321 = OpLabel
%323 = OpFunctionCall %v2int %returns_int2_i2
%324 = OpIEqual %v2bool %110 %323
%325 = OpAll %bool %324
OpBranch %322
%322 = OpLabel
%326 = OpPhi %bool %false %317 %325 %321
OpSelectionMerge %328 None
OpBranchConditional %326 %327 %328
%327 = OpLabel
%329 = OpFunctionCall %v3int %returns_int3_i3
%330 = OpIEqual %v3bool %115 %329
%331 = OpAll %bool %330
OpBranch %328
%328 = OpLabel
%332 = OpPhi %bool %false %322 %331 %327
OpSelectionMerge %334 None
OpBranchConditional %332 %333 %334
%333 = OpLabel
%335 = OpFunctionCall %v4int %returns_int4_i4
%336 = OpIEqual %v4bool %120 %335
%337 = OpAll %bool %336
OpBranch %334
%334 = OpLabel
%338 = OpPhi %bool %false %328 %337 %333
OpSelectionMerge %342 None
OpBranchConditional %338 %340 %341
%340 = OpLabel
%343 = OpAccessChain %_ptr_Uniform_v4float %31 %int_0
%346 = OpLoad %v4float %343
OpStore %339 %346
OpBranch %342
%341 = OpLabel
%347 = OpAccessChain %_ptr_Uniform_v4float %31 %int_1
%348 = OpLoad %v4float %347
OpStore %339 %348
OpBranch %342
%342 = OpLabel
%349 = OpLoad %v4float %339
OpReturnValue %349
OpFunctionEnd
