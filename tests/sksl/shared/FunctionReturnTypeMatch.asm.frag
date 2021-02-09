OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %returns_float "returns_float"
OpName %returns_float2 "returns_float2"
OpName %returns_float3 "returns_float3"
OpName %returns_float4 "returns_float4"
OpName %returns_float2x2 "returns_float2x2"
OpName %returns_float3x3 "returns_float3x3"
OpName %returns_float4x4 "returns_float4x4"
OpName %returns_half "returns_half"
OpName %returns_half2 "returns_half2"
OpName %returns_half3 "returns_half3"
OpName %returns_half4 "returns_half4"
OpName %returns_half2x2 "returns_half2x2"
OpName %returns_half3x3 "returns_half3x3"
OpName %returns_half4x4 "returns_half4x4"
OpName %returns_bool "returns_bool"
OpName %returns_bool2 "returns_bool2"
OpName %returns_bool3 "returns_bool3"
OpName %returns_bool4 "returns_bool4"
OpName %returns_int "returns_int"
OpName %returns_int2 "returns_int2"
OpName %returns_int3 "returns_int3"
OpName %returns_int4 "returns_int4"
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
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %32 Binding 0
OpDecorate %32 DescriptorSet 0
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %366 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %419 RelaxedPrecision
OpDecorate %421 RelaxedPrecision
OpDecorate %422 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%32 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%37 = OpTypeFunction %void
%40 = OpTypeFunction %float
%float_1 = OpConstant %float 1
%v2float = OpTypeVector %float 2
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
%float_0 = OpConstant %float 0
%mat3v3float = OpTypeMatrix %v3float 3
%65 = OpTypeFunction %mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%72 = OpTypeFunction %mat4v4float
%98 = OpTypeFunction %bool
%true = OpConstantTrue %bool
%v2bool = OpTypeVector %bool 2
%102 = OpTypeFunction %v2bool
%104 = OpConstantComposite %v2bool %true %true
%v3bool = OpTypeVector %bool 3
%106 = OpTypeFunction %v3bool
%108 = OpConstantComposite %v3bool %true %true %true
%v4bool = OpTypeVector %bool 4
%110 = OpTypeFunction %v4bool
%112 = OpConstantComposite %v4bool %true %true %true %true
%int = OpTypeInt 32 1
%114 = OpTypeFunction %int
%int_1 = OpConstant %int 1
%v2int = OpTypeVector %int 2
%118 = OpTypeFunction %v2int
%int_2 = OpConstant %int 2
%121 = OpConstantComposite %v2int %int_2 %int_2
%v3int = OpTypeVector %int 3
%123 = OpTypeFunction %v3int
%int_3 = OpConstant %int 3
%126 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%v4int = OpTypeVector %int 4
%128 = OpTypeFunction %v4int
%int_4 = OpConstant %int 4
%131 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_v2float = OpTypePointer Function %v2float
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
%_entrypoint = OpFunction %void None %37
%38 = OpLabel
%39 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %39
OpReturn
OpFunctionEnd
%returns_float = OpFunction %float None %40
%41 = OpLabel
OpReturnValue %float_1
OpFunctionEnd
%returns_float2 = OpFunction %v2float None %44
%45 = OpLabel
OpReturnValue %47
OpFunctionEnd
%returns_float3 = OpFunction %v3float None %49
%50 = OpLabel
OpReturnValue %52
OpFunctionEnd
%returns_float4 = OpFunction %v4float None %53
%54 = OpLabel
OpReturnValue %56
OpFunctionEnd
%returns_float2x2 = OpFunction %mat2v2float None %58
%59 = OpLabel
%62 = OpCompositeConstruct %v2float %float_2 %float_0
%63 = OpCompositeConstruct %v2float %float_0 %float_2
%60 = OpCompositeConstruct %mat2v2float %62 %63
OpReturnValue %60
OpFunctionEnd
%returns_float3x3 = OpFunction %mat3v3float None %65
%66 = OpLabel
%68 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%69 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%70 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%67 = OpCompositeConstruct %mat3v3float %68 %69 %70
OpReturnValue %67
OpFunctionEnd
%returns_float4x4 = OpFunction %mat4v4float None %72
%73 = OpLabel
%75 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%76 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%77 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%78 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%74 = OpCompositeConstruct %mat4v4float %75 %76 %77 %78
OpReturnValue %74
OpFunctionEnd
%returns_half = OpFunction %float None %40
%79 = OpLabel
OpReturnValue %float_1
OpFunctionEnd
%returns_half2 = OpFunction %v2float None %44
%80 = OpLabel
OpReturnValue %47
OpFunctionEnd
%returns_half3 = OpFunction %v3float None %49
%81 = OpLabel
OpReturnValue %52
OpFunctionEnd
%returns_half4 = OpFunction %v4float None %53
%82 = OpLabel
OpReturnValue %56
OpFunctionEnd
%returns_half2x2 = OpFunction %mat2v2float None %58
%83 = OpLabel
%85 = OpCompositeConstruct %v2float %float_2 %float_0
%86 = OpCompositeConstruct %v2float %float_0 %float_2
%84 = OpCompositeConstruct %mat2v2float %85 %86
OpReturnValue %84
OpFunctionEnd
%returns_half3x3 = OpFunction %mat3v3float None %65
%87 = OpLabel
%89 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%90 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%91 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%88 = OpCompositeConstruct %mat3v3float %89 %90 %91
OpReturnValue %88
OpFunctionEnd
%returns_half4x4 = OpFunction %mat4v4float None %72
%92 = OpLabel
%94 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%95 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%96 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%97 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%93 = OpCompositeConstruct %mat4v4float %94 %95 %96 %97
OpReturnValue %93
OpFunctionEnd
%returns_bool = OpFunction %bool None %98
%99 = OpLabel
OpReturnValue %true
OpFunctionEnd
%returns_bool2 = OpFunction %v2bool None %102
%103 = OpLabel
OpReturnValue %104
OpFunctionEnd
%returns_bool3 = OpFunction %v3bool None %106
%107 = OpLabel
OpReturnValue %108
OpFunctionEnd
%returns_bool4 = OpFunction %v4bool None %110
%111 = OpLabel
OpReturnValue %112
OpFunctionEnd
%returns_int = OpFunction %int None %114
%115 = OpLabel
OpReturnValue %int_1
OpFunctionEnd
%returns_int2 = OpFunction %v2int None %118
%119 = OpLabel
OpReturnValue %121
OpFunctionEnd
%returns_int3 = OpFunction %v3int None %123
%124 = OpLabel
OpReturnValue %126
OpFunctionEnd
%returns_int4 = OpFunction %v4int None %128
%129 = OpLabel
OpReturnValue %131
OpFunctionEnd
%main = OpFunction %v4float None %53
%132 = OpLabel
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
%412 = OpVariable %_ptr_Function_v4float Function
%135 = OpFunctionCall %float %returns_float
OpStore %x1 %135
%138 = OpFunctionCall %v2float %returns_float2
OpStore %x2 %138
%141 = OpFunctionCall %v3float %returns_float3
OpStore %x3 %141
%144 = OpFunctionCall %v4float %returns_float4
OpStore %x4 %144
%147 = OpFunctionCall %mat2v2float %returns_float2x2
OpStore %x5 %147
%150 = OpFunctionCall %mat3v3float %returns_float3x3
OpStore %x6 %150
%153 = OpFunctionCall %mat4v4float %returns_float4x4
OpStore %x7 %153
%155 = OpFunctionCall %float %returns_half
OpStore %x8 %155
%157 = OpFunctionCall %v2float %returns_half2
OpStore %x9 %157
%159 = OpFunctionCall %v3float %returns_half3
OpStore %x10 %159
%161 = OpFunctionCall %v4float %returns_half4
OpStore %x11 %161
%163 = OpFunctionCall %mat2v2float %returns_half2x2
OpStore %x12 %163
%165 = OpFunctionCall %mat3v3float %returns_half3x3
OpStore %x13 %165
%167 = OpFunctionCall %mat4v4float %returns_half4x4
OpStore %x14 %167
%170 = OpFunctionCall %bool %returns_bool
OpStore %x15 %170
%173 = OpFunctionCall %v2bool %returns_bool2
OpStore %x16 %173
%176 = OpFunctionCall %v3bool %returns_bool3
OpStore %x17 %176
%179 = OpFunctionCall %v4bool %returns_bool4
OpStore %x18 %179
%182 = OpFunctionCall %int %returns_int
OpStore %x19 %182
%185 = OpFunctionCall %v2int %returns_int2
OpStore %x20 %185
%188 = OpFunctionCall %v3int %returns_int3
OpStore %x21 %188
%191 = OpFunctionCall %v4int %returns_int4
OpStore %x22 %191
%193 = OpLoad %float %x1
%194 = OpFunctionCall %float %returns_float
%195 = OpFOrdEqual %bool %193 %194
OpSelectionMerge %197 None
OpBranchConditional %195 %196 %197
%196 = OpLabel
%198 = OpLoad %v2float %x2
%199 = OpFunctionCall %v2float %returns_float2
%200 = OpFOrdEqual %v2bool %198 %199
%201 = OpAll %bool %200
OpBranch %197
%197 = OpLabel
%202 = OpPhi %bool %false %132 %201 %196
OpSelectionMerge %204 None
OpBranchConditional %202 %203 %204
%203 = OpLabel
%205 = OpLoad %v3float %x3
%206 = OpFunctionCall %v3float %returns_float3
%207 = OpFOrdEqual %v3bool %205 %206
%208 = OpAll %bool %207
OpBranch %204
%204 = OpLabel
%209 = OpPhi %bool %false %197 %208 %203
OpSelectionMerge %211 None
OpBranchConditional %209 %210 %211
%210 = OpLabel
%212 = OpLoad %v4float %x4
%213 = OpFunctionCall %v4float %returns_float4
%214 = OpFOrdEqual %v4bool %212 %213
%215 = OpAll %bool %214
OpBranch %211
%211 = OpLabel
%216 = OpPhi %bool %false %204 %215 %210
OpSelectionMerge %218 None
OpBranchConditional %216 %217 %218
%217 = OpLabel
%219 = OpLoad %mat2v2float %x5
%220 = OpFunctionCall %mat2v2float %returns_float2x2
%221 = OpCompositeExtract %v2float %219 0
%222 = OpCompositeExtract %v2float %220 0
%223 = OpFOrdEqual %v2bool %221 %222
%224 = OpAll %bool %223
%225 = OpCompositeExtract %v2float %219 1
%226 = OpCompositeExtract %v2float %220 1
%227 = OpFOrdEqual %v2bool %225 %226
%228 = OpAll %bool %227
%229 = OpLogicalAnd %bool %224 %228
OpBranch %218
%218 = OpLabel
%230 = OpPhi %bool %false %211 %229 %217
OpSelectionMerge %232 None
OpBranchConditional %230 %231 %232
%231 = OpLabel
%233 = OpLoad %mat3v3float %x6
%234 = OpFunctionCall %mat3v3float %returns_float3x3
%235 = OpCompositeExtract %v3float %233 0
%236 = OpCompositeExtract %v3float %234 0
%237 = OpFOrdEqual %v3bool %235 %236
%238 = OpAll %bool %237
%239 = OpCompositeExtract %v3float %233 1
%240 = OpCompositeExtract %v3float %234 1
%241 = OpFOrdEqual %v3bool %239 %240
%242 = OpAll %bool %241
%243 = OpLogicalAnd %bool %238 %242
%244 = OpCompositeExtract %v3float %233 2
%245 = OpCompositeExtract %v3float %234 2
%246 = OpFOrdEqual %v3bool %244 %245
%247 = OpAll %bool %246
%248 = OpLogicalAnd %bool %243 %247
OpBranch %232
%232 = OpLabel
%249 = OpPhi %bool %false %218 %248 %231
OpSelectionMerge %251 None
OpBranchConditional %249 %250 %251
%250 = OpLabel
%252 = OpLoad %mat4v4float %x7
%253 = OpFunctionCall %mat4v4float %returns_float4x4
%254 = OpCompositeExtract %v4float %252 0
%255 = OpCompositeExtract %v4float %253 0
%256 = OpFOrdEqual %v4bool %254 %255
%257 = OpAll %bool %256
%258 = OpCompositeExtract %v4float %252 1
%259 = OpCompositeExtract %v4float %253 1
%260 = OpFOrdEqual %v4bool %258 %259
%261 = OpAll %bool %260
%262 = OpLogicalAnd %bool %257 %261
%263 = OpCompositeExtract %v4float %252 2
%264 = OpCompositeExtract %v4float %253 2
%265 = OpFOrdEqual %v4bool %263 %264
%266 = OpAll %bool %265
%267 = OpLogicalAnd %bool %262 %266
%268 = OpCompositeExtract %v4float %252 3
%269 = OpCompositeExtract %v4float %253 3
%270 = OpFOrdEqual %v4bool %268 %269
%271 = OpAll %bool %270
%272 = OpLogicalAnd %bool %267 %271
OpBranch %251
%251 = OpLabel
%273 = OpPhi %bool %false %232 %272 %250
OpSelectionMerge %275 None
OpBranchConditional %273 %274 %275
%274 = OpLabel
%276 = OpLoad %float %x8
%277 = OpFunctionCall %float %returns_half
%278 = OpFOrdEqual %bool %276 %277
OpBranch %275
%275 = OpLabel
%279 = OpPhi %bool %false %251 %278 %274
OpSelectionMerge %281 None
OpBranchConditional %279 %280 %281
%280 = OpLabel
%282 = OpLoad %v2float %x9
%283 = OpFunctionCall %v2float %returns_half2
%284 = OpFOrdEqual %v2bool %282 %283
%285 = OpAll %bool %284
OpBranch %281
%281 = OpLabel
%286 = OpPhi %bool %false %275 %285 %280
OpSelectionMerge %288 None
OpBranchConditional %286 %287 %288
%287 = OpLabel
%289 = OpLoad %v3float %x10
%290 = OpFunctionCall %v3float %returns_half3
%291 = OpFOrdEqual %v3bool %289 %290
%292 = OpAll %bool %291
OpBranch %288
%288 = OpLabel
%293 = OpPhi %bool %false %281 %292 %287
OpSelectionMerge %295 None
OpBranchConditional %293 %294 %295
%294 = OpLabel
%296 = OpLoad %v4float %x11
%297 = OpFunctionCall %v4float %returns_half4
%298 = OpFOrdEqual %v4bool %296 %297
%299 = OpAll %bool %298
OpBranch %295
%295 = OpLabel
%300 = OpPhi %bool %false %288 %299 %294
OpSelectionMerge %302 None
OpBranchConditional %300 %301 %302
%301 = OpLabel
%303 = OpLoad %mat2v2float %x12
%304 = OpFunctionCall %mat2v2float %returns_half2x2
%305 = OpCompositeExtract %v2float %303 0
%306 = OpCompositeExtract %v2float %304 0
%307 = OpFOrdEqual %v2bool %305 %306
%308 = OpAll %bool %307
%309 = OpCompositeExtract %v2float %303 1
%310 = OpCompositeExtract %v2float %304 1
%311 = OpFOrdEqual %v2bool %309 %310
%312 = OpAll %bool %311
%313 = OpLogicalAnd %bool %308 %312
OpBranch %302
%302 = OpLabel
%314 = OpPhi %bool %false %295 %313 %301
OpSelectionMerge %316 None
OpBranchConditional %314 %315 %316
%315 = OpLabel
%317 = OpLoad %mat3v3float %x13
%318 = OpFunctionCall %mat3v3float %returns_half3x3
%319 = OpCompositeExtract %v3float %317 0
%320 = OpCompositeExtract %v3float %318 0
%321 = OpFOrdEqual %v3bool %319 %320
%322 = OpAll %bool %321
%323 = OpCompositeExtract %v3float %317 1
%324 = OpCompositeExtract %v3float %318 1
%325 = OpFOrdEqual %v3bool %323 %324
%326 = OpAll %bool %325
%327 = OpLogicalAnd %bool %322 %326
%328 = OpCompositeExtract %v3float %317 2
%329 = OpCompositeExtract %v3float %318 2
%330 = OpFOrdEqual %v3bool %328 %329
%331 = OpAll %bool %330
%332 = OpLogicalAnd %bool %327 %331
OpBranch %316
%316 = OpLabel
%333 = OpPhi %bool %false %302 %332 %315
OpSelectionMerge %335 None
OpBranchConditional %333 %334 %335
%334 = OpLabel
%336 = OpLoad %mat4v4float %x14
%337 = OpFunctionCall %mat4v4float %returns_half4x4
%338 = OpCompositeExtract %v4float %336 0
%339 = OpCompositeExtract %v4float %337 0
%340 = OpFOrdEqual %v4bool %338 %339
%341 = OpAll %bool %340
%342 = OpCompositeExtract %v4float %336 1
%343 = OpCompositeExtract %v4float %337 1
%344 = OpFOrdEqual %v4bool %342 %343
%345 = OpAll %bool %344
%346 = OpLogicalAnd %bool %341 %345
%347 = OpCompositeExtract %v4float %336 2
%348 = OpCompositeExtract %v4float %337 2
%349 = OpFOrdEqual %v4bool %347 %348
%350 = OpAll %bool %349
%351 = OpLogicalAnd %bool %346 %350
%352 = OpCompositeExtract %v4float %336 3
%353 = OpCompositeExtract %v4float %337 3
%354 = OpFOrdEqual %v4bool %352 %353
%355 = OpAll %bool %354
%356 = OpLogicalAnd %bool %351 %355
OpBranch %335
%335 = OpLabel
%357 = OpPhi %bool %false %316 %356 %334
OpSelectionMerge %359 None
OpBranchConditional %357 %358 %359
%358 = OpLabel
%360 = OpLoad %bool %x15
%361 = OpFunctionCall %bool %returns_bool
%362 = OpLogicalEqual %bool %360 %361
OpBranch %359
%359 = OpLabel
%363 = OpPhi %bool %false %335 %362 %358
OpSelectionMerge %365 None
OpBranchConditional %363 %364 %365
%364 = OpLabel
%366 = OpLoad %v2bool %x16
%367 = OpFunctionCall %v2bool %returns_bool2
%368 = OpLogicalEqual %v2bool %366 %367
%369 = OpAll %bool %368
OpBranch %365
%365 = OpLabel
%370 = OpPhi %bool %false %359 %369 %364
OpSelectionMerge %372 None
OpBranchConditional %370 %371 %372
%371 = OpLabel
%373 = OpLoad %v3bool %x17
%374 = OpFunctionCall %v3bool %returns_bool3
%375 = OpLogicalEqual %v3bool %373 %374
%376 = OpAll %bool %375
OpBranch %372
%372 = OpLabel
%377 = OpPhi %bool %false %365 %376 %371
OpSelectionMerge %379 None
OpBranchConditional %377 %378 %379
%378 = OpLabel
%380 = OpLoad %v4bool %x18
%381 = OpFunctionCall %v4bool %returns_bool4
%382 = OpLogicalEqual %v4bool %380 %381
%383 = OpAll %bool %382
OpBranch %379
%379 = OpLabel
%384 = OpPhi %bool %false %372 %383 %378
OpSelectionMerge %386 None
OpBranchConditional %384 %385 %386
%385 = OpLabel
%387 = OpLoad %int %x19
%388 = OpFunctionCall %int %returns_int
%389 = OpIEqual %bool %387 %388
OpBranch %386
%386 = OpLabel
%390 = OpPhi %bool %false %379 %389 %385
OpSelectionMerge %392 None
OpBranchConditional %390 %391 %392
%391 = OpLabel
%393 = OpLoad %v2int %x20
%394 = OpFunctionCall %v2int %returns_int2
%395 = OpIEqual %v2bool %393 %394
%396 = OpAll %bool %395
OpBranch %392
%392 = OpLabel
%397 = OpPhi %bool %false %386 %396 %391
OpSelectionMerge %399 None
OpBranchConditional %397 %398 %399
%398 = OpLabel
%400 = OpLoad %v3int %x21
%401 = OpFunctionCall %v3int %returns_int3
%402 = OpIEqual %v3bool %400 %401
%403 = OpAll %bool %402
OpBranch %399
%399 = OpLabel
%404 = OpPhi %bool %false %392 %403 %398
OpSelectionMerge %406 None
OpBranchConditional %404 %405 %406
%405 = OpLabel
%407 = OpLoad %v4int %x22
%408 = OpFunctionCall %v4int %returns_int4
%409 = OpIEqual %v4bool %407 %408
%410 = OpAll %bool %409
OpBranch %406
%406 = OpLabel
%411 = OpPhi %bool %false %399 %410 %405
OpSelectionMerge %415 None
OpBranchConditional %411 %413 %414
%413 = OpLabel
%416 = OpAccessChain %_ptr_Uniform_v4float %32 %int_0
%419 = OpLoad %v4float %416
OpStore %412 %419
OpBranch %415
%414 = OpLabel
%420 = OpAccessChain %_ptr_Uniform_v4float %32 %int_1
%421 = OpLoad %v4float %420
OpStore %412 %421
OpBranch %415
%415 = OpLabel
%422 = OpLoad %v4float %412
OpReturnValue %422
OpFunctionEnd
