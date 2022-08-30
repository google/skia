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
OpName %returns_float_f "returns_float_f"
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
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %32 Binding 0
OpDecorate %32 DescriptorSet 0
OpDecorate %x8 RelaxedPrecision
OpDecorate %x9 RelaxedPrecision
OpDecorate %x10 RelaxedPrecision
OpDecorate %x11 RelaxedPrecision
OpDecorate %x12 RelaxedPrecision
OpDecorate %x13 RelaxedPrecision
OpDecorate %x14 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %389 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%32 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%37 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%41 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%45 = OpTypeFunction %float
%float_1 = OpConstant %float 1
%48 = OpTypeFunction %v2float
%float_2 = OpConstant %float 2
%51 = OpConstantComposite %v2float %float_2 %float_2
%v3float = OpTypeVector %float 3
%53 = OpTypeFunction %v3float
%float_3 = OpConstant %float 3
%56 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%57 = OpTypeFunction %v4float
%float_4 = OpConstant %float 4
%60 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%mat2v2float = OpTypeMatrix %v2float 2
%62 = OpTypeFunction %mat2v2float
%64 = OpConstantComposite %v2float %float_2 %float_0
%65 = OpConstantComposite %v2float %float_0 %float_2
%66 = OpConstantComposite %mat2v2float %64 %65
%mat3v3float = OpTypeMatrix %v3float 3
%68 = OpTypeFunction %mat3v3float
%70 = OpConstantComposite %v3float %float_3 %float_0 %float_0
%71 = OpConstantComposite %v3float %float_0 %float_3 %float_0
%72 = OpConstantComposite %v3float %float_0 %float_0 %float_3
%73 = OpConstantComposite %mat3v3float %70 %71 %72
%mat4v4float = OpTypeMatrix %v4float 4
%75 = OpTypeFunction %mat4v4float
%77 = OpConstantComposite %v4float %float_4 %float_0 %float_0 %float_0
%78 = OpConstantComposite %v4float %float_0 %float_4 %float_0 %float_0
%79 = OpConstantComposite %v4float %float_0 %float_0 %float_4 %float_0
%80 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_4
%81 = OpConstantComposite %mat4v4float %77 %78 %79 %80
%89 = OpTypeFunction %bool
%true = OpConstantTrue %bool
%v2bool = OpTypeVector %bool 2
%93 = OpTypeFunction %v2bool
%95 = OpConstantComposite %v2bool %true %true
%v3bool = OpTypeVector %bool 3
%97 = OpTypeFunction %v3bool
%99 = OpConstantComposite %v3bool %true %true %true
%v4bool = OpTypeVector %bool 4
%101 = OpTypeFunction %v4bool
%103 = OpConstantComposite %v4bool %true %true %true %true
%int = OpTypeInt 32 1
%105 = OpTypeFunction %int
%int_1 = OpConstant %int 1
%v2int = OpTypeVector %int 2
%109 = OpTypeFunction %v2int
%int_2 = OpConstant %int 2
%112 = OpConstantComposite %v2int %int_2 %int_2
%v3int = OpTypeVector %int 3
%114 = OpTypeFunction %v3int
%int_3 = OpConstant %int 3
%117 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%v4int = OpTypeVector %int 4
%119 = OpTypeFunction %v4int
%int_4 = OpConstant %int 4
%122 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%123 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%_entrypoint_v = OpFunction %void None %37
%38 = OpLabel
%42 = OpVariable %_ptr_Function_v2float Function
OpStore %42 %41
%44 = OpFunctionCall %v4float %main %42
OpStore %sk_FragColor %44
OpReturn
OpFunctionEnd
%returns_float_f = OpFunction %float None %45
%46 = OpLabel
OpReturnValue %float_1
OpFunctionEnd
%returns_float2_f2 = OpFunction %v2float None %48
%49 = OpLabel
OpReturnValue %51
OpFunctionEnd
%returns_float3_f3 = OpFunction %v3float None %53
%54 = OpLabel
OpReturnValue %56
OpFunctionEnd
%returns_float4_f4 = OpFunction %v4float None %57
%58 = OpLabel
OpReturnValue %60
OpFunctionEnd
%returns_float2x2_f22 = OpFunction %mat2v2float None %62
%63 = OpLabel
OpReturnValue %66
OpFunctionEnd
%returns_float3x3_f33 = OpFunction %mat3v3float None %68
%69 = OpLabel
OpReturnValue %73
OpFunctionEnd
%returns_float4x4_f44 = OpFunction %mat4v4float None %75
%76 = OpLabel
OpReturnValue %81
OpFunctionEnd
%returns_half_h = OpFunction %float None %45
%82 = OpLabel
OpReturnValue %float_1
OpFunctionEnd
%returns_half2_h2 = OpFunction %v2float None %48
%83 = OpLabel
OpReturnValue %51
OpFunctionEnd
%returns_half3_h3 = OpFunction %v3float None %53
%84 = OpLabel
OpReturnValue %56
OpFunctionEnd
%returns_half4_h4 = OpFunction %v4float None %57
%85 = OpLabel
OpReturnValue %60
OpFunctionEnd
%returns_half2x2_h22 = OpFunction %mat2v2float None %62
%86 = OpLabel
OpReturnValue %66
OpFunctionEnd
%returns_half3x3_h33 = OpFunction %mat3v3float None %68
%87 = OpLabel
OpReturnValue %73
OpFunctionEnd
%returns_half4x4_h44 = OpFunction %mat4v4float None %75
%88 = OpLabel
OpReturnValue %81
OpFunctionEnd
%returns_bool_b = OpFunction %bool None %89
%90 = OpLabel
OpReturnValue %true
OpFunctionEnd
%returns_bool2_b2 = OpFunction %v2bool None %93
%94 = OpLabel
OpReturnValue %95
OpFunctionEnd
%returns_bool3_b3 = OpFunction %v3bool None %97
%98 = OpLabel
OpReturnValue %99
OpFunctionEnd
%returns_bool4_b4 = OpFunction %v4bool None %101
%102 = OpLabel
OpReturnValue %103
OpFunctionEnd
%returns_int_i = OpFunction %int None %105
%106 = OpLabel
OpReturnValue %int_1
OpFunctionEnd
%returns_int2_i2 = OpFunction %v2int None %109
%110 = OpLabel
OpReturnValue %112
OpFunctionEnd
%returns_int3_i3 = OpFunction %v3int None %114
%115 = OpLabel
OpReturnValue %117
OpFunctionEnd
%returns_int4_i4 = OpFunction %v4int None %119
%120 = OpLabel
OpReturnValue %122
OpFunctionEnd
%main = OpFunction %v4float None %123
%124 = OpFunctionParameter %_ptr_Function_v2float
%125 = OpLabel
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
%382 = OpVariable %_ptr_Function_v4float Function
%128 = OpFunctionCall %float %returns_float_f
OpStore %x1 %128
%130 = OpFunctionCall %v2float %returns_float2_f2
OpStore %x2 %130
%133 = OpFunctionCall %v3float %returns_float3_f3
OpStore %x3 %133
%136 = OpFunctionCall %v4float %returns_float4_f4
OpStore %x4 %136
%139 = OpFunctionCall %mat2v2float %returns_float2x2_f22
OpStore %x5 %139
%142 = OpFunctionCall %mat3v3float %returns_float3x3_f33
OpStore %x6 %142
%145 = OpFunctionCall %mat4v4float %returns_float4x4_f44
OpStore %x7 %145
%147 = OpFunctionCall %float %returns_half_h
OpStore %x8 %147
%149 = OpFunctionCall %v2float %returns_half2_h2
OpStore %x9 %149
%151 = OpFunctionCall %v3float %returns_half3_h3
OpStore %x10 %151
%153 = OpFunctionCall %v4float %returns_half4_h4
OpStore %x11 %153
%155 = OpFunctionCall %mat2v2float %returns_half2x2_h22
OpStore %x12 %155
%157 = OpFunctionCall %mat3v3float %returns_half3x3_h33
OpStore %x13 %157
%159 = OpFunctionCall %mat4v4float %returns_half4x4_h44
OpStore %x14 %159
%162 = OpFunctionCall %bool %returns_bool_b
OpStore %x15 %162
%165 = OpFunctionCall %v2bool %returns_bool2_b2
OpStore %x16 %165
%168 = OpFunctionCall %v3bool %returns_bool3_b3
OpStore %x17 %168
%171 = OpFunctionCall %v4bool %returns_bool4_b4
OpStore %x18 %171
%174 = OpFunctionCall %int %returns_int_i
OpStore %x19 %174
%177 = OpFunctionCall %v2int %returns_int2_i2
OpStore %x20 %177
%180 = OpFunctionCall %v3int %returns_int3_i3
OpStore %x21 %180
%183 = OpFunctionCall %v4int %returns_int4_i4
OpStore %x22 %183
%185 = OpFunctionCall %float %returns_float_f
%186 = OpFOrdEqual %bool %128 %185
OpSelectionMerge %188 None
OpBranchConditional %186 %187 %188
%187 = OpLabel
%189 = OpFunctionCall %v2float %returns_float2_f2
%190 = OpFOrdEqual %v2bool %130 %189
%191 = OpAll %bool %190
OpBranch %188
%188 = OpLabel
%192 = OpPhi %bool %false %125 %191 %187
OpSelectionMerge %194 None
OpBranchConditional %192 %193 %194
%193 = OpLabel
%195 = OpFunctionCall %v3float %returns_float3_f3
%196 = OpFOrdEqual %v3bool %133 %195
%197 = OpAll %bool %196
OpBranch %194
%194 = OpLabel
%198 = OpPhi %bool %false %188 %197 %193
OpSelectionMerge %200 None
OpBranchConditional %198 %199 %200
%199 = OpLabel
%201 = OpFunctionCall %v4float %returns_float4_f4
%202 = OpFOrdEqual %v4bool %136 %201
%203 = OpAll %bool %202
OpBranch %200
%200 = OpLabel
%204 = OpPhi %bool %false %194 %203 %199
OpSelectionMerge %206 None
OpBranchConditional %204 %205 %206
%205 = OpLabel
%207 = OpFunctionCall %mat2v2float %returns_float2x2_f22
%208 = OpCompositeExtract %v2float %139 0
%209 = OpCompositeExtract %v2float %207 0
%210 = OpFOrdEqual %v2bool %208 %209
%211 = OpAll %bool %210
%212 = OpCompositeExtract %v2float %139 1
%213 = OpCompositeExtract %v2float %207 1
%214 = OpFOrdEqual %v2bool %212 %213
%215 = OpAll %bool %214
%216 = OpLogicalAnd %bool %211 %215
OpBranch %206
%206 = OpLabel
%217 = OpPhi %bool %false %200 %216 %205
OpSelectionMerge %219 None
OpBranchConditional %217 %218 %219
%218 = OpLabel
%220 = OpFunctionCall %mat3v3float %returns_float3x3_f33
%221 = OpCompositeExtract %v3float %142 0
%222 = OpCompositeExtract %v3float %220 0
%223 = OpFOrdEqual %v3bool %221 %222
%224 = OpAll %bool %223
%225 = OpCompositeExtract %v3float %142 1
%226 = OpCompositeExtract %v3float %220 1
%227 = OpFOrdEqual %v3bool %225 %226
%228 = OpAll %bool %227
%229 = OpLogicalAnd %bool %224 %228
%230 = OpCompositeExtract %v3float %142 2
%231 = OpCompositeExtract %v3float %220 2
%232 = OpFOrdEqual %v3bool %230 %231
%233 = OpAll %bool %232
%234 = OpLogicalAnd %bool %229 %233
OpBranch %219
%219 = OpLabel
%235 = OpPhi %bool %false %206 %234 %218
OpSelectionMerge %237 None
OpBranchConditional %235 %236 %237
%236 = OpLabel
%238 = OpFunctionCall %mat4v4float %returns_float4x4_f44
%239 = OpCompositeExtract %v4float %145 0
%240 = OpCompositeExtract %v4float %238 0
%241 = OpFOrdEqual %v4bool %239 %240
%242 = OpAll %bool %241
%243 = OpCompositeExtract %v4float %145 1
%244 = OpCompositeExtract %v4float %238 1
%245 = OpFOrdEqual %v4bool %243 %244
%246 = OpAll %bool %245
%247 = OpLogicalAnd %bool %242 %246
%248 = OpCompositeExtract %v4float %145 2
%249 = OpCompositeExtract %v4float %238 2
%250 = OpFOrdEqual %v4bool %248 %249
%251 = OpAll %bool %250
%252 = OpLogicalAnd %bool %247 %251
%253 = OpCompositeExtract %v4float %145 3
%254 = OpCompositeExtract %v4float %238 3
%255 = OpFOrdEqual %v4bool %253 %254
%256 = OpAll %bool %255
%257 = OpLogicalAnd %bool %252 %256
OpBranch %237
%237 = OpLabel
%258 = OpPhi %bool %false %219 %257 %236
OpSelectionMerge %260 None
OpBranchConditional %258 %259 %260
%259 = OpLabel
%261 = OpFunctionCall %float %returns_half_h
%262 = OpFOrdEqual %bool %147 %261
OpBranch %260
%260 = OpLabel
%263 = OpPhi %bool %false %237 %262 %259
OpSelectionMerge %265 None
OpBranchConditional %263 %264 %265
%264 = OpLabel
%266 = OpFunctionCall %v2float %returns_half2_h2
%267 = OpFOrdEqual %v2bool %149 %266
%268 = OpAll %bool %267
OpBranch %265
%265 = OpLabel
%269 = OpPhi %bool %false %260 %268 %264
OpSelectionMerge %271 None
OpBranchConditional %269 %270 %271
%270 = OpLabel
%272 = OpFunctionCall %v3float %returns_half3_h3
%273 = OpFOrdEqual %v3bool %151 %272
%274 = OpAll %bool %273
OpBranch %271
%271 = OpLabel
%275 = OpPhi %bool %false %265 %274 %270
OpSelectionMerge %277 None
OpBranchConditional %275 %276 %277
%276 = OpLabel
%278 = OpFunctionCall %v4float %returns_half4_h4
%279 = OpFOrdEqual %v4bool %153 %278
%280 = OpAll %bool %279
OpBranch %277
%277 = OpLabel
%281 = OpPhi %bool %false %271 %280 %276
OpSelectionMerge %283 None
OpBranchConditional %281 %282 %283
%282 = OpLabel
%284 = OpFunctionCall %mat2v2float %returns_half2x2_h22
%285 = OpCompositeExtract %v2float %155 0
%286 = OpCompositeExtract %v2float %284 0
%287 = OpFOrdEqual %v2bool %285 %286
%288 = OpAll %bool %287
%289 = OpCompositeExtract %v2float %155 1
%290 = OpCompositeExtract %v2float %284 1
%291 = OpFOrdEqual %v2bool %289 %290
%292 = OpAll %bool %291
%293 = OpLogicalAnd %bool %288 %292
OpBranch %283
%283 = OpLabel
%294 = OpPhi %bool %false %277 %293 %282
OpSelectionMerge %296 None
OpBranchConditional %294 %295 %296
%295 = OpLabel
%297 = OpFunctionCall %mat3v3float %returns_half3x3_h33
%298 = OpCompositeExtract %v3float %157 0
%299 = OpCompositeExtract %v3float %297 0
%300 = OpFOrdEqual %v3bool %298 %299
%301 = OpAll %bool %300
%302 = OpCompositeExtract %v3float %157 1
%303 = OpCompositeExtract %v3float %297 1
%304 = OpFOrdEqual %v3bool %302 %303
%305 = OpAll %bool %304
%306 = OpLogicalAnd %bool %301 %305
%307 = OpCompositeExtract %v3float %157 2
%308 = OpCompositeExtract %v3float %297 2
%309 = OpFOrdEqual %v3bool %307 %308
%310 = OpAll %bool %309
%311 = OpLogicalAnd %bool %306 %310
OpBranch %296
%296 = OpLabel
%312 = OpPhi %bool %false %283 %311 %295
OpSelectionMerge %314 None
OpBranchConditional %312 %313 %314
%313 = OpLabel
%315 = OpFunctionCall %mat4v4float %returns_half4x4_h44
%316 = OpCompositeExtract %v4float %159 0
%317 = OpCompositeExtract %v4float %315 0
%318 = OpFOrdEqual %v4bool %316 %317
%319 = OpAll %bool %318
%320 = OpCompositeExtract %v4float %159 1
%321 = OpCompositeExtract %v4float %315 1
%322 = OpFOrdEqual %v4bool %320 %321
%323 = OpAll %bool %322
%324 = OpLogicalAnd %bool %319 %323
%325 = OpCompositeExtract %v4float %159 2
%326 = OpCompositeExtract %v4float %315 2
%327 = OpFOrdEqual %v4bool %325 %326
%328 = OpAll %bool %327
%329 = OpLogicalAnd %bool %324 %328
%330 = OpCompositeExtract %v4float %159 3
%331 = OpCompositeExtract %v4float %315 3
%332 = OpFOrdEqual %v4bool %330 %331
%333 = OpAll %bool %332
%334 = OpLogicalAnd %bool %329 %333
OpBranch %314
%314 = OpLabel
%335 = OpPhi %bool %false %296 %334 %313
OpSelectionMerge %337 None
OpBranchConditional %335 %336 %337
%336 = OpLabel
%338 = OpFunctionCall %bool %returns_bool_b
%339 = OpLogicalEqual %bool %162 %338
OpBranch %337
%337 = OpLabel
%340 = OpPhi %bool %false %314 %339 %336
OpSelectionMerge %342 None
OpBranchConditional %340 %341 %342
%341 = OpLabel
%343 = OpFunctionCall %v2bool %returns_bool2_b2
%344 = OpLogicalEqual %v2bool %165 %343
%345 = OpAll %bool %344
OpBranch %342
%342 = OpLabel
%346 = OpPhi %bool %false %337 %345 %341
OpSelectionMerge %348 None
OpBranchConditional %346 %347 %348
%347 = OpLabel
%349 = OpFunctionCall %v3bool %returns_bool3_b3
%350 = OpLogicalEqual %v3bool %168 %349
%351 = OpAll %bool %350
OpBranch %348
%348 = OpLabel
%352 = OpPhi %bool %false %342 %351 %347
OpSelectionMerge %354 None
OpBranchConditional %352 %353 %354
%353 = OpLabel
%355 = OpFunctionCall %v4bool %returns_bool4_b4
%356 = OpLogicalEqual %v4bool %171 %355
%357 = OpAll %bool %356
OpBranch %354
%354 = OpLabel
%358 = OpPhi %bool %false %348 %357 %353
OpSelectionMerge %360 None
OpBranchConditional %358 %359 %360
%359 = OpLabel
%361 = OpFunctionCall %int %returns_int_i
%362 = OpIEqual %bool %174 %361
OpBranch %360
%360 = OpLabel
%363 = OpPhi %bool %false %354 %362 %359
OpSelectionMerge %365 None
OpBranchConditional %363 %364 %365
%364 = OpLabel
%366 = OpFunctionCall %v2int %returns_int2_i2
%367 = OpIEqual %v2bool %177 %366
%368 = OpAll %bool %367
OpBranch %365
%365 = OpLabel
%369 = OpPhi %bool %false %360 %368 %364
OpSelectionMerge %371 None
OpBranchConditional %369 %370 %371
%370 = OpLabel
%372 = OpFunctionCall %v3int %returns_int3_i3
%373 = OpIEqual %v3bool %180 %372
%374 = OpAll %bool %373
OpBranch %371
%371 = OpLabel
%375 = OpPhi %bool %false %365 %374 %370
OpSelectionMerge %377 None
OpBranchConditional %375 %376 %377
%376 = OpLabel
%378 = OpFunctionCall %v4int %returns_int4_i4
%379 = OpIEqual %v4bool %183 %378
%380 = OpAll %bool %379
OpBranch %377
%377 = OpLabel
%381 = OpPhi %bool %false %371 %380 %376
OpSelectionMerge %385 None
OpBranchConditional %381 %383 %384
%383 = OpLabel
%386 = OpAccessChain %_ptr_Uniform_v4float %32 %int_0
%389 = OpLoad %v4float %386
OpStore %382 %389
OpBranch %385
%384 = OpLabel
%390 = OpAccessChain %_ptr_Uniform_v4float %32 %int_1
%391 = OpLoad %v4float %390
OpStore %382 %391
OpBranch %385
%385 = OpLabel
%392 = OpLoad %v4float %382
OpReturnValue %392
OpFunctionEnd
