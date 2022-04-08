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
OpDecorate %85 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %x8 RelaxedPrecision
OpDecorate %x9 RelaxedPrecision
OpDecorate %x10 RelaxedPrecision
OpDecorate %x11 RelaxedPrecision
OpDecorate %x12 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %x13 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %x14 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
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
%31 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%36 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
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
%61 = OpConstantComposite %v2float %float_2 %float_0
%62 = OpConstantComposite %v2float %float_0 %float_2
%mat3v3float = OpTypeMatrix %v3float 3
%64 = OpTypeFunction %mat3v3float
%67 = OpConstantComposite %v3float %float_3 %float_0 %float_0
%68 = OpConstantComposite %v3float %float_0 %float_3 %float_0
%69 = OpConstantComposite %v3float %float_0 %float_0 %float_3
%mat4v4float = OpTypeMatrix %v4float 4
%71 = OpTypeFunction %mat4v4float
%74 = OpConstantComposite %v4float %float_4 %float_0 %float_0 %float_0
%75 = OpConstantComposite %v4float %float_0 %float_4 %float_0 %float_0
%76 = OpConstantComposite %v4float %float_0 %float_0 %float_4 %float_0
%77 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_4
%78 = OpTypeFunction %float
%float_1 = OpConstant %float 1
%90 = OpTypeFunction %bool
%true = OpConstantTrue %bool
%v2bool = OpTypeVector %bool 2
%94 = OpTypeFunction %v2bool
%96 = OpConstantComposite %v2bool %true %true
%v3bool = OpTypeVector %bool 3
%98 = OpTypeFunction %v3bool
%100 = OpConstantComposite %v3bool %true %true %true
%v4bool = OpTypeVector %bool 4
%102 = OpTypeFunction %v4bool
%104 = OpConstantComposite %v4bool %true %true %true %true
%int = OpTypeInt 32 1
%106 = OpTypeFunction %int
%int_1 = OpConstant %int 1
%v2int = OpTypeVector %int 2
%110 = OpTypeFunction %v2int
%int_2 = OpConstant %int 2
%113 = OpConstantComposite %v2int %int_2 %int_2
%v3int = OpTypeVector %int 3
%115 = OpTypeFunction %v3int
%int_3 = OpConstant %int 3
%118 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%v4int = OpTypeVector %int 4
%120 = OpTypeFunction %v4int
%int_4 = OpConstant %int 4
%123 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%124 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%60 = OpCompositeConstruct %mat2v2float %61 %62
OpReturnValue %60
OpFunctionEnd
%returns_float3x3_f33 = OpFunction %mat3v3float None %64
%65 = OpLabel
%66 = OpCompositeConstruct %mat3v3float %67 %68 %69
OpReturnValue %66
OpFunctionEnd
%returns_float4x4_f44 = OpFunction %mat4v4float None %71
%72 = OpLabel
%73 = OpCompositeConstruct %mat4v4float %74 %75 %76 %77
OpReturnValue %73
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
%85 = OpCompositeConstruct %mat2v2float %61 %62
OpReturnValue %85
OpFunctionEnd
%returns_half3x3_h33 = OpFunction %mat3v3float None %64
%86 = OpLabel
%87 = OpCompositeConstruct %mat3v3float %67 %68 %69
OpReturnValue %87
OpFunctionEnd
%returns_half4x4_h44 = OpFunction %mat4v4float None %71
%88 = OpLabel
%89 = OpCompositeConstruct %mat4v4float %74 %75 %76 %77
OpReturnValue %89
OpFunctionEnd
%returns_bool_b = OpFunction %bool None %90
%91 = OpLabel
OpReturnValue %true
OpFunctionEnd
%returns_bool2_b2 = OpFunction %v2bool None %94
%95 = OpLabel
OpReturnValue %96
OpFunctionEnd
%returns_bool3_b3 = OpFunction %v3bool None %98
%99 = OpLabel
OpReturnValue %100
OpFunctionEnd
%returns_bool4_b4 = OpFunction %v4bool None %102
%103 = OpLabel
OpReturnValue %104
OpFunctionEnd
%returns_int_i = OpFunction %int None %106
%107 = OpLabel
OpReturnValue %int_1
OpFunctionEnd
%returns_int2_i2 = OpFunction %v2int None %110
%111 = OpLabel
OpReturnValue %113
OpFunctionEnd
%returns_int3_i3 = OpFunction %v3int None %115
%116 = OpLabel
OpReturnValue %118
OpFunctionEnd
%returns_int4_i4 = OpFunction %v4int None %120
%121 = OpLabel
OpReturnValue %123
OpFunctionEnd
%main = OpFunction %v4float None %124
%125 = OpFunctionParameter %_ptr_Function_v2float
%126 = OpLabel
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
%388 = OpVariable %_ptr_Function_v4float Function
OpStore %x1 %float_1
OpStore %x2 %47
OpStore %x3 %52
OpStore %x4 %56
%136 = OpCompositeConstruct %mat2v2float %61 %62
OpStore %x5 %136
%139 = OpCompositeConstruct %mat3v3float %67 %68 %69
OpStore %x6 %139
%142 = OpCompositeConstruct %mat4v4float %74 %75 %76 %77
OpStore %x7 %142
OpStore %x8 %float_1
OpStore %x9 %47
OpStore %x10 %52
OpStore %x11 %56
%148 = OpCompositeConstruct %mat2v2float %61 %62
OpStore %x12 %148
%150 = OpCompositeConstruct %mat3v3float %67 %68 %69
OpStore %x13 %150
%152 = OpCompositeConstruct %mat4v4float %74 %75 %76 %77
OpStore %x14 %152
OpStore %x15 %true
OpStore %x16 %96
OpStore %x17 %100
OpStore %x18 %104
OpStore %x19 %int_1
OpStore %x20 %113
OpStore %x21 %118
OpStore %x22 %123
%170 = OpLoad %float %x1
%171 = OpFOrdEqual %bool %170 %float_1
OpSelectionMerge %173 None
OpBranchConditional %171 %172 %173
%172 = OpLabel
%174 = OpLoad %v2float %x2
%175 = OpFunctionCall %v2float %returns_float2_f2
%176 = OpFOrdEqual %v2bool %174 %175
%177 = OpAll %bool %176
OpBranch %173
%173 = OpLabel
%178 = OpPhi %bool %false %126 %177 %172
OpSelectionMerge %180 None
OpBranchConditional %178 %179 %180
%179 = OpLabel
%181 = OpLoad %v3float %x3
%182 = OpFunctionCall %v3float %returns_float3_f3
%183 = OpFOrdEqual %v3bool %181 %182
%184 = OpAll %bool %183
OpBranch %180
%180 = OpLabel
%185 = OpPhi %bool %false %173 %184 %179
OpSelectionMerge %187 None
OpBranchConditional %185 %186 %187
%186 = OpLabel
%188 = OpLoad %v4float %x4
%189 = OpFunctionCall %v4float %returns_float4_f4
%190 = OpFOrdEqual %v4bool %188 %189
%191 = OpAll %bool %190
OpBranch %187
%187 = OpLabel
%192 = OpPhi %bool %false %180 %191 %186
OpSelectionMerge %194 None
OpBranchConditional %192 %193 %194
%193 = OpLabel
%195 = OpLoad %mat2v2float %x5
%196 = OpFunctionCall %mat2v2float %returns_float2x2_f22
%197 = OpCompositeExtract %v2float %195 0
%198 = OpCompositeExtract %v2float %196 0
%199 = OpFOrdEqual %v2bool %197 %198
%200 = OpAll %bool %199
%201 = OpCompositeExtract %v2float %195 1
%202 = OpCompositeExtract %v2float %196 1
%203 = OpFOrdEqual %v2bool %201 %202
%204 = OpAll %bool %203
%205 = OpLogicalAnd %bool %200 %204
OpBranch %194
%194 = OpLabel
%206 = OpPhi %bool %false %187 %205 %193
OpSelectionMerge %208 None
OpBranchConditional %206 %207 %208
%207 = OpLabel
%209 = OpLoad %mat3v3float %x6
%210 = OpFunctionCall %mat3v3float %returns_float3x3_f33
%211 = OpCompositeExtract %v3float %209 0
%212 = OpCompositeExtract %v3float %210 0
%213 = OpFOrdEqual %v3bool %211 %212
%214 = OpAll %bool %213
%215 = OpCompositeExtract %v3float %209 1
%216 = OpCompositeExtract %v3float %210 1
%217 = OpFOrdEqual %v3bool %215 %216
%218 = OpAll %bool %217
%219 = OpLogicalAnd %bool %214 %218
%220 = OpCompositeExtract %v3float %209 2
%221 = OpCompositeExtract %v3float %210 2
%222 = OpFOrdEqual %v3bool %220 %221
%223 = OpAll %bool %222
%224 = OpLogicalAnd %bool %219 %223
OpBranch %208
%208 = OpLabel
%225 = OpPhi %bool %false %194 %224 %207
OpSelectionMerge %227 None
OpBranchConditional %225 %226 %227
%226 = OpLabel
%228 = OpLoad %mat4v4float %x7
%229 = OpFunctionCall %mat4v4float %returns_float4x4_f44
%230 = OpCompositeExtract %v4float %228 0
%231 = OpCompositeExtract %v4float %229 0
%232 = OpFOrdEqual %v4bool %230 %231
%233 = OpAll %bool %232
%234 = OpCompositeExtract %v4float %228 1
%235 = OpCompositeExtract %v4float %229 1
%236 = OpFOrdEqual %v4bool %234 %235
%237 = OpAll %bool %236
%238 = OpLogicalAnd %bool %233 %237
%239 = OpCompositeExtract %v4float %228 2
%240 = OpCompositeExtract %v4float %229 2
%241 = OpFOrdEqual %v4bool %239 %240
%242 = OpAll %bool %241
%243 = OpLogicalAnd %bool %238 %242
%244 = OpCompositeExtract %v4float %228 3
%245 = OpCompositeExtract %v4float %229 3
%246 = OpFOrdEqual %v4bool %244 %245
%247 = OpAll %bool %246
%248 = OpLogicalAnd %bool %243 %247
OpBranch %227
%227 = OpLabel
%249 = OpPhi %bool %false %208 %248 %226
OpSelectionMerge %251 None
OpBranchConditional %249 %250 %251
%250 = OpLabel
%252 = OpLoad %float %x8
%253 = OpFunctionCall %float %returns_half_h
%254 = OpFOrdEqual %bool %252 %253
OpBranch %251
%251 = OpLabel
%255 = OpPhi %bool %false %227 %254 %250
OpSelectionMerge %257 None
OpBranchConditional %255 %256 %257
%256 = OpLabel
%258 = OpLoad %v2float %x9
%259 = OpFunctionCall %v2float %returns_half2_h2
%260 = OpFOrdEqual %v2bool %258 %259
%261 = OpAll %bool %260
OpBranch %257
%257 = OpLabel
%262 = OpPhi %bool %false %251 %261 %256
OpSelectionMerge %264 None
OpBranchConditional %262 %263 %264
%263 = OpLabel
%265 = OpLoad %v3float %x10
%266 = OpFunctionCall %v3float %returns_half3_h3
%267 = OpFOrdEqual %v3bool %265 %266
%268 = OpAll %bool %267
OpBranch %264
%264 = OpLabel
%269 = OpPhi %bool %false %257 %268 %263
OpSelectionMerge %271 None
OpBranchConditional %269 %270 %271
%270 = OpLabel
%272 = OpLoad %v4float %x11
%273 = OpFunctionCall %v4float %returns_half4_h4
%274 = OpFOrdEqual %v4bool %272 %273
%275 = OpAll %bool %274
OpBranch %271
%271 = OpLabel
%276 = OpPhi %bool %false %264 %275 %270
OpSelectionMerge %278 None
OpBranchConditional %276 %277 %278
%277 = OpLabel
%279 = OpLoad %mat2v2float %x12
%280 = OpFunctionCall %mat2v2float %returns_half2x2_h22
%281 = OpCompositeExtract %v2float %279 0
%282 = OpCompositeExtract %v2float %280 0
%283 = OpFOrdEqual %v2bool %281 %282
%284 = OpAll %bool %283
%285 = OpCompositeExtract %v2float %279 1
%286 = OpCompositeExtract %v2float %280 1
%287 = OpFOrdEqual %v2bool %285 %286
%288 = OpAll %bool %287
%289 = OpLogicalAnd %bool %284 %288
OpBranch %278
%278 = OpLabel
%290 = OpPhi %bool %false %271 %289 %277
OpSelectionMerge %292 None
OpBranchConditional %290 %291 %292
%291 = OpLabel
%293 = OpLoad %mat3v3float %x13
%294 = OpFunctionCall %mat3v3float %returns_half3x3_h33
%295 = OpCompositeExtract %v3float %293 0
%296 = OpCompositeExtract %v3float %294 0
%297 = OpFOrdEqual %v3bool %295 %296
%298 = OpAll %bool %297
%299 = OpCompositeExtract %v3float %293 1
%300 = OpCompositeExtract %v3float %294 1
%301 = OpFOrdEqual %v3bool %299 %300
%302 = OpAll %bool %301
%303 = OpLogicalAnd %bool %298 %302
%304 = OpCompositeExtract %v3float %293 2
%305 = OpCompositeExtract %v3float %294 2
%306 = OpFOrdEqual %v3bool %304 %305
%307 = OpAll %bool %306
%308 = OpLogicalAnd %bool %303 %307
OpBranch %292
%292 = OpLabel
%309 = OpPhi %bool %false %278 %308 %291
OpSelectionMerge %311 None
OpBranchConditional %309 %310 %311
%310 = OpLabel
%312 = OpLoad %mat4v4float %x14
%313 = OpFunctionCall %mat4v4float %returns_half4x4_h44
%314 = OpCompositeExtract %v4float %312 0
%315 = OpCompositeExtract %v4float %313 0
%316 = OpFOrdEqual %v4bool %314 %315
%317 = OpAll %bool %316
%318 = OpCompositeExtract %v4float %312 1
%319 = OpCompositeExtract %v4float %313 1
%320 = OpFOrdEqual %v4bool %318 %319
%321 = OpAll %bool %320
%322 = OpLogicalAnd %bool %317 %321
%323 = OpCompositeExtract %v4float %312 2
%324 = OpCompositeExtract %v4float %313 2
%325 = OpFOrdEqual %v4bool %323 %324
%326 = OpAll %bool %325
%327 = OpLogicalAnd %bool %322 %326
%328 = OpCompositeExtract %v4float %312 3
%329 = OpCompositeExtract %v4float %313 3
%330 = OpFOrdEqual %v4bool %328 %329
%331 = OpAll %bool %330
%332 = OpLogicalAnd %bool %327 %331
OpBranch %311
%311 = OpLabel
%333 = OpPhi %bool %false %292 %332 %310
OpSelectionMerge %335 None
OpBranchConditional %333 %334 %335
%334 = OpLabel
%336 = OpLoad %bool %x15
%337 = OpFunctionCall %bool %returns_bool_b
%338 = OpLogicalEqual %bool %336 %337
OpBranch %335
%335 = OpLabel
%339 = OpPhi %bool %false %311 %338 %334
OpSelectionMerge %341 None
OpBranchConditional %339 %340 %341
%340 = OpLabel
%342 = OpLoad %v2bool %x16
%343 = OpFunctionCall %v2bool %returns_bool2_b2
%344 = OpLogicalEqual %v2bool %342 %343
%345 = OpAll %bool %344
OpBranch %341
%341 = OpLabel
%346 = OpPhi %bool %false %335 %345 %340
OpSelectionMerge %348 None
OpBranchConditional %346 %347 %348
%347 = OpLabel
%349 = OpLoad %v3bool %x17
%350 = OpFunctionCall %v3bool %returns_bool3_b3
%351 = OpLogicalEqual %v3bool %349 %350
%352 = OpAll %bool %351
OpBranch %348
%348 = OpLabel
%353 = OpPhi %bool %false %341 %352 %347
OpSelectionMerge %355 None
OpBranchConditional %353 %354 %355
%354 = OpLabel
%356 = OpLoad %v4bool %x18
%357 = OpFunctionCall %v4bool %returns_bool4_b4
%358 = OpLogicalEqual %v4bool %356 %357
%359 = OpAll %bool %358
OpBranch %355
%355 = OpLabel
%360 = OpPhi %bool %false %348 %359 %354
OpSelectionMerge %362 None
OpBranchConditional %360 %361 %362
%361 = OpLabel
%363 = OpLoad %int %x19
%364 = OpFunctionCall %int %returns_int_i
%365 = OpIEqual %bool %363 %364
OpBranch %362
%362 = OpLabel
%366 = OpPhi %bool %false %355 %365 %361
OpSelectionMerge %368 None
OpBranchConditional %366 %367 %368
%367 = OpLabel
%369 = OpLoad %v2int %x20
%370 = OpFunctionCall %v2int %returns_int2_i2
%371 = OpIEqual %v2bool %369 %370
%372 = OpAll %bool %371
OpBranch %368
%368 = OpLabel
%373 = OpPhi %bool %false %362 %372 %367
OpSelectionMerge %375 None
OpBranchConditional %373 %374 %375
%374 = OpLabel
%376 = OpLoad %v3int %x21
%377 = OpFunctionCall %v3int %returns_int3_i3
%378 = OpIEqual %v3bool %376 %377
%379 = OpAll %bool %378
OpBranch %375
%375 = OpLabel
%380 = OpPhi %bool %false %368 %379 %374
OpSelectionMerge %382 None
OpBranchConditional %380 %381 %382
%381 = OpLabel
%383 = OpLoad %v4int %x22
%384 = OpFunctionCall %v4int %returns_int4_i4
%385 = OpIEqual %v4bool %383 %384
%386 = OpAll %bool %385
OpBranch %382
%382 = OpLabel
%387 = OpPhi %bool %false %375 %386 %381
OpSelectionMerge %391 None
OpBranchConditional %387 %389 %390
%389 = OpLabel
%392 = OpAccessChain %_ptr_Uniform_v4float %31 %int_0
%395 = OpLoad %v4float %392
OpStore %388 %395
OpBranch %391
%390 = OpLabel
%396 = OpAccessChain %_ptr_Uniform_v4float %31 %int_1
%397 = OpLoad %v4float %396
OpStore %388 %397
OpBranch %391
%391 = OpLabel
%398 = OpLoad %v4float %388
OpReturnValue %398
OpFunctionEnd
