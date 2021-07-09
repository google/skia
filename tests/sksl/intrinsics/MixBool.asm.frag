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
OpMemberName %_UniformBuffer 2 "colorBlack"
OpMemberName %_UniformBuffer 3 "colorWhite"
OpMemberName %_UniformBuffer 4 "testInputs"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %intGreen "intGreen"
OpName %intRed "intRed"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 4 Offset 64
OpMemberDecorate %_UniformBuffer 4 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %337 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %350 RelaxedPrecision
OpDecorate %351 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %368 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %389 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %393 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %411 RelaxedPrecision
OpDecorate %413 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %420 RelaxedPrecision
OpDecorate %422 RelaxedPrecision
OpDecorate %423 RelaxedPrecision
OpDecorate %429 RelaxedPrecision
OpDecorate %431 RelaxedPrecision
OpDecorate %438 RelaxedPrecision
OpDecorate %439 RelaxedPrecision
OpDecorate %444 RelaxedPrecision
OpDecorate %446 RelaxedPrecision
OpDecorate %447 RelaxedPrecision
OpDecorate %453 RelaxedPrecision
OpDecorate %455 RelaxedPrecision
OpDecorate %456 RelaxedPrecision
OpDecorate %462 RelaxedPrecision
OpDecorate %464 RelaxedPrecision
OpDecorate %474 RelaxedPrecision
OpDecorate %476 RelaxedPrecision
OpDecorate %477 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%float_100 = OpConstant %float 100
%int_1 = OpConstant %int 1
%false = OpConstantFalse %bool
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%81 = OpConstantComposite %v2bool %false %false
%v3int = OpTypeVector %int 3
%v3bool = OpTypeVector %bool 3
%100 = OpConstantComposite %v3bool %false %false %false
%v4bool = OpTypeVector %bool 4
%116 = OpConstantComposite %v4bool %false %false %false %false
%true = OpConstantTrue %bool
%146 = OpConstantComposite %v2bool %true %true
%163 = OpConstantComposite %v3bool %true %true %true
%178 = OpConstantComposite %v4bool %true %true %true %true
%int_100 = OpConstant %int 100
%194 = OpConstantComposite %v2int %int_0 %int_100
%202 = OpConstantComposite %v3int %int_0 %int_100 %int_0
%210 = OpConstantComposite %v4int %int_0 %int_100 %int_0 %int_100
%223 = OpConstantComposite %v2int %int_100 %int_0
%231 = OpConstantComposite %v3int %int_100 %int_0 %int_0
%239 = OpConstantComposite %v4int %int_100 %int_0 %int_0 %int_100
%v3float = OpTypeVector %float 3
%float_1 = OpConstant %float 1
%411 = OpConstantComposite %v2float %float_0 %float_1
%420 = OpConstantComposite %v3float %float_0 %float_1 %float_0
%429 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
%444 = OpConstantComposite %v2float %float_1 %float_0
%453 = OpConstantComposite %v3float %float_1 %float_0 %float_0
%462 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%20 = OpVariable %_ptr_Function_v2float Function
OpStore %20 %19
%22 = OpFunctionCall %v4float %main %20
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %23
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpLabel
%intGreen = OpVariable %_ptr_Function_v4int Function
%intRed = OpVariable %_ptr_Function_v4int Function
%468 = OpVariable %_ptr_Function_v4float Function
%30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%33 = OpLoad %v4float %30
%35 = OpVectorTimesScalar %v4float %33 %float_100
%36 = OpCompositeExtract %float %35 0
%37 = OpConvertFToS %int %36
%38 = OpCompositeExtract %float %35 1
%39 = OpConvertFToS %int %38
%40 = OpCompositeExtract %float %35 2
%41 = OpConvertFToS %int %40
%42 = OpCompositeExtract %float %35 3
%43 = OpConvertFToS %int %42
%44 = OpCompositeConstruct %v4int %37 %39 %41 %43
OpStore %intGreen %44
%46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%48 = OpLoad %v4float %46
%49 = OpVectorTimesScalar %v4float %48 %float_100
%50 = OpCompositeExtract %float %49 0
%51 = OpConvertFToS %int %50
%52 = OpCompositeExtract %float %49 1
%53 = OpConvertFToS %int %52
%54 = OpCompositeExtract %float %49 2
%55 = OpConvertFToS %int %54
%56 = OpCompositeExtract %float %49 3
%57 = OpConvertFToS %int %56
%58 = OpCompositeConstruct %v4int %51 %53 %55 %57
OpStore %intRed %58
%61 = OpLoad %v4int %intGreen
%62 = OpCompositeExtract %int %61 0
%63 = OpLoad %v4int %intRed
%64 = OpCompositeExtract %int %63 0
%65 = OpLoad %v4int %intGreen
%66 = OpCompositeExtract %int %65 0
%67 = OpLoad %v4int %intRed
%68 = OpCompositeExtract %int %67 0
%60 = OpSelect %int %false %68 %66
%69 = OpLoad %v4int %intGreen
%70 = OpCompositeExtract %int %69 0
%71 = OpIEqual %bool %60 %70
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%75 = OpLoad %v4int %intGreen
%76 = OpVectorShuffle %v2int %75 %75 0 1
%78 = OpLoad %v4int %intRed
%79 = OpVectorShuffle %v2int %78 %78 0 1
%82 = OpLoad %v4int %intGreen
%83 = OpVectorShuffle %v2int %82 %82 0 1
%84 = OpLoad %v4int %intRed
%85 = OpVectorShuffle %v2int %84 %84 0 1
%74 = OpSelect %v2int %81 %85 %83
%86 = OpLoad %v4int %intGreen
%87 = OpVectorShuffle %v2int %86 %86 0 1
%88 = OpIEqual %v2bool %74 %87
%89 = OpAll %bool %88
OpBranch %73
%73 = OpLabel
%90 = OpPhi %bool %false %25 %89 %72
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%94 = OpLoad %v4int %intGreen
%95 = OpVectorShuffle %v3int %94 %94 0 1 2
%97 = OpLoad %v4int %intRed
%98 = OpVectorShuffle %v3int %97 %97 0 1 2
%101 = OpLoad %v4int %intGreen
%102 = OpVectorShuffle %v3int %101 %101 0 1 2
%103 = OpLoad %v4int %intRed
%104 = OpVectorShuffle %v3int %103 %103 0 1 2
%93 = OpSelect %v3int %100 %104 %102
%105 = OpLoad %v4int %intGreen
%106 = OpVectorShuffle %v3int %105 %105 0 1 2
%107 = OpIEqual %v3bool %93 %106
%108 = OpAll %bool %107
OpBranch %92
%92 = OpLabel
%109 = OpPhi %bool %false %73 %108 %91
OpSelectionMerge %111 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
%113 = OpLoad %v4int %intGreen
%114 = OpLoad %v4int %intRed
%117 = OpLoad %v4int %intGreen
%118 = OpLoad %v4int %intRed
%112 = OpSelect %v4int %116 %118 %117
%119 = OpLoad %v4int %intGreen
%120 = OpIEqual %v4bool %112 %119
%121 = OpAll %bool %120
OpBranch %111
%111 = OpLabel
%122 = OpPhi %bool %false %92 %121 %110
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%126 = OpLoad %v4int %intGreen
%127 = OpCompositeExtract %int %126 0
%128 = OpLoad %v4int %intRed
%129 = OpCompositeExtract %int %128 0
%131 = OpLoad %v4int %intGreen
%132 = OpCompositeExtract %int %131 0
%133 = OpLoad %v4int %intRed
%134 = OpCompositeExtract %int %133 0
%125 = OpSelect %int %true %134 %132
%135 = OpLoad %v4int %intRed
%136 = OpCompositeExtract %int %135 0
%137 = OpIEqual %bool %125 %136
OpBranch %124
%124 = OpLabel
%138 = OpPhi %bool %false %111 %137 %123
OpSelectionMerge %140 None
OpBranchConditional %138 %139 %140
%139 = OpLabel
%142 = OpLoad %v4int %intGreen
%143 = OpVectorShuffle %v2int %142 %142 0 1
%144 = OpLoad %v4int %intRed
%145 = OpVectorShuffle %v2int %144 %144 0 1
%147 = OpLoad %v4int %intGreen
%148 = OpVectorShuffle %v2int %147 %147 0 1
%149 = OpLoad %v4int %intRed
%150 = OpVectorShuffle %v2int %149 %149 0 1
%141 = OpSelect %v2int %146 %150 %148
%151 = OpLoad %v4int %intRed
%152 = OpVectorShuffle %v2int %151 %151 0 1
%153 = OpIEqual %v2bool %141 %152
%154 = OpAll %bool %153
OpBranch %140
%140 = OpLabel
%155 = OpPhi %bool %false %124 %154 %139
OpSelectionMerge %157 None
OpBranchConditional %155 %156 %157
%156 = OpLabel
%159 = OpLoad %v4int %intGreen
%160 = OpVectorShuffle %v3int %159 %159 0 1 2
%161 = OpLoad %v4int %intRed
%162 = OpVectorShuffle %v3int %161 %161 0 1 2
%164 = OpLoad %v4int %intGreen
%165 = OpVectorShuffle %v3int %164 %164 0 1 2
%166 = OpLoad %v4int %intRed
%167 = OpVectorShuffle %v3int %166 %166 0 1 2
%158 = OpSelect %v3int %163 %167 %165
%168 = OpLoad %v4int %intRed
%169 = OpVectorShuffle %v3int %168 %168 0 1 2
%170 = OpIEqual %v3bool %158 %169
%171 = OpAll %bool %170
OpBranch %157
%157 = OpLabel
%172 = OpPhi %bool %false %140 %171 %156
OpSelectionMerge %174 None
OpBranchConditional %172 %173 %174
%173 = OpLabel
%176 = OpLoad %v4int %intGreen
%177 = OpLoad %v4int %intRed
%179 = OpLoad %v4int %intGreen
%180 = OpLoad %v4int %intRed
%175 = OpSelect %v4int %178 %180 %179
%181 = OpLoad %v4int %intRed
%182 = OpIEqual %v4bool %175 %181
%183 = OpAll %bool %182
OpBranch %174
%174 = OpLabel
%184 = OpPhi %bool %false %157 %183 %173
OpSelectionMerge %186 None
OpBranchConditional %184 %185 %186
%185 = OpLabel
%187 = OpLoad %v4int %intGreen
%188 = OpCompositeExtract %int %187 0
%189 = OpIEqual %bool %int_0 %188
OpBranch %186
%186 = OpLabel
%190 = OpPhi %bool %false %174 %189 %185
OpSelectionMerge %192 None
OpBranchConditional %190 %191 %192
%191 = OpLabel
%195 = OpLoad %v4int %intGreen
%196 = OpVectorShuffle %v2int %195 %195 0 1
%197 = OpIEqual %v2bool %194 %196
%198 = OpAll %bool %197
OpBranch %192
%192 = OpLabel
%199 = OpPhi %bool %false %186 %198 %191
OpSelectionMerge %201 None
OpBranchConditional %199 %200 %201
%200 = OpLabel
%203 = OpLoad %v4int %intGreen
%204 = OpVectorShuffle %v3int %203 %203 0 1 2
%205 = OpIEqual %v3bool %202 %204
%206 = OpAll %bool %205
OpBranch %201
%201 = OpLabel
%207 = OpPhi %bool %false %192 %206 %200
OpSelectionMerge %209 None
OpBranchConditional %207 %208 %209
%208 = OpLabel
%211 = OpLoad %v4int %intGreen
%212 = OpIEqual %v4bool %210 %211
%213 = OpAll %bool %212
OpBranch %209
%209 = OpLabel
%214 = OpPhi %bool %false %201 %213 %208
OpSelectionMerge %216 None
OpBranchConditional %214 %215 %216
%215 = OpLabel
%217 = OpLoad %v4int %intRed
%218 = OpCompositeExtract %int %217 0
%219 = OpIEqual %bool %int_100 %218
OpBranch %216
%216 = OpLabel
%220 = OpPhi %bool %false %209 %219 %215
OpSelectionMerge %222 None
OpBranchConditional %220 %221 %222
%221 = OpLabel
%224 = OpLoad %v4int %intRed
%225 = OpVectorShuffle %v2int %224 %224 0 1
%226 = OpIEqual %v2bool %223 %225
%227 = OpAll %bool %226
OpBranch %222
%222 = OpLabel
%228 = OpPhi %bool %false %216 %227 %221
OpSelectionMerge %230 None
OpBranchConditional %228 %229 %230
%229 = OpLabel
%232 = OpLoad %v4int %intRed
%233 = OpVectorShuffle %v3int %232 %232 0 1 2
%234 = OpIEqual %v3bool %231 %233
%235 = OpAll %bool %234
OpBranch %230
%230 = OpLabel
%236 = OpPhi %bool %false %222 %235 %229
OpSelectionMerge %238 None
OpBranchConditional %236 %237 %238
%237 = OpLabel
%240 = OpLoad %v4int %intRed
%241 = OpIEqual %v4bool %239 %240
%242 = OpAll %bool %241
OpBranch %238
%238 = OpLabel
%243 = OpPhi %bool %false %230 %242 %237
OpSelectionMerge %245 None
OpBranchConditional %243 %244 %245
%244 = OpLabel
%247 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%248 = OpLoad %v4float %247
%249 = OpCompositeExtract %float %248 0
%250 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%251 = OpLoad %v4float %250
%252 = OpCompositeExtract %float %251 0
%253 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%254 = OpLoad %v4float %253
%255 = OpCompositeExtract %float %254 0
%256 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%257 = OpLoad %v4float %256
%258 = OpCompositeExtract %float %257 0
%246 = OpSelect %float %false %258 %255
%259 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%260 = OpLoad %v4float %259
%261 = OpCompositeExtract %float %260 0
%262 = OpFOrdEqual %bool %246 %261
OpBranch %245
%245 = OpLabel
%263 = OpPhi %bool %false %238 %262 %244
OpSelectionMerge %265 None
OpBranchConditional %263 %264 %265
%264 = OpLabel
%267 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%268 = OpLoad %v4float %267
%269 = OpVectorShuffle %v2float %268 %268 0 1
%270 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%271 = OpLoad %v4float %270
%272 = OpVectorShuffle %v2float %271 %271 0 1
%273 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%274 = OpLoad %v4float %273
%275 = OpVectorShuffle %v2float %274 %274 0 1
%276 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%277 = OpLoad %v4float %276
%278 = OpVectorShuffle %v2float %277 %277 0 1
%266 = OpSelect %v2float %81 %278 %275
%279 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%280 = OpLoad %v4float %279
%281 = OpVectorShuffle %v2float %280 %280 0 1
%282 = OpFOrdEqual %v2bool %266 %281
%283 = OpAll %bool %282
OpBranch %265
%265 = OpLabel
%284 = OpPhi %bool %false %245 %283 %264
OpSelectionMerge %286 None
OpBranchConditional %284 %285 %286
%285 = OpLabel
%288 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%289 = OpLoad %v4float %288
%290 = OpVectorShuffle %v3float %289 %289 0 1 2
%292 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%293 = OpLoad %v4float %292
%294 = OpVectorShuffle %v3float %293 %293 0 1 2
%295 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%296 = OpLoad %v4float %295
%297 = OpVectorShuffle %v3float %296 %296 0 1 2
%298 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%299 = OpLoad %v4float %298
%300 = OpVectorShuffle %v3float %299 %299 0 1 2
%287 = OpSelect %v3float %100 %300 %297
%301 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%302 = OpLoad %v4float %301
%303 = OpVectorShuffle %v3float %302 %302 0 1 2
%304 = OpFOrdEqual %v3bool %287 %303
%305 = OpAll %bool %304
OpBranch %286
%286 = OpLabel
%306 = OpPhi %bool %false %265 %305 %285
OpSelectionMerge %308 None
OpBranchConditional %306 %307 %308
%307 = OpLabel
%310 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%311 = OpLoad %v4float %310
%312 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%313 = OpLoad %v4float %312
%314 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%315 = OpLoad %v4float %314
%316 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%317 = OpLoad %v4float %316
%309 = OpSelect %v4float %116 %317 %315
%318 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%319 = OpLoad %v4float %318
%320 = OpFOrdEqual %v4bool %309 %319
%321 = OpAll %bool %320
OpBranch %308
%308 = OpLabel
%322 = OpPhi %bool %false %286 %321 %307
OpSelectionMerge %324 None
OpBranchConditional %322 %323 %324
%323 = OpLabel
%326 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%327 = OpLoad %v4float %326
%328 = OpCompositeExtract %float %327 0
%329 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%330 = OpLoad %v4float %329
%331 = OpCompositeExtract %float %330 0
%332 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%333 = OpLoad %v4float %332
%334 = OpCompositeExtract %float %333 0
%335 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%336 = OpLoad %v4float %335
%337 = OpCompositeExtract %float %336 0
%325 = OpSelect %float %true %337 %334
%338 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%339 = OpLoad %v4float %338
%340 = OpCompositeExtract %float %339 0
%341 = OpFOrdEqual %bool %325 %340
OpBranch %324
%324 = OpLabel
%342 = OpPhi %bool %false %308 %341 %323
OpSelectionMerge %344 None
OpBranchConditional %342 %343 %344
%343 = OpLabel
%346 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%347 = OpLoad %v4float %346
%348 = OpVectorShuffle %v2float %347 %347 0 1
%349 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%350 = OpLoad %v4float %349
%351 = OpVectorShuffle %v2float %350 %350 0 1
%352 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%353 = OpLoad %v4float %352
%354 = OpVectorShuffle %v2float %353 %353 0 1
%355 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%356 = OpLoad %v4float %355
%357 = OpVectorShuffle %v2float %356 %356 0 1
%345 = OpSelect %v2float %146 %357 %354
%358 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%359 = OpLoad %v4float %358
%360 = OpVectorShuffle %v2float %359 %359 0 1
%361 = OpFOrdEqual %v2bool %345 %360
%362 = OpAll %bool %361
OpBranch %344
%344 = OpLabel
%363 = OpPhi %bool %false %324 %362 %343
OpSelectionMerge %365 None
OpBranchConditional %363 %364 %365
%364 = OpLabel
%367 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%368 = OpLoad %v4float %367
%369 = OpVectorShuffle %v3float %368 %368 0 1 2
%370 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%371 = OpLoad %v4float %370
%372 = OpVectorShuffle %v3float %371 %371 0 1 2
%373 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%374 = OpLoad %v4float %373
%375 = OpVectorShuffle %v3float %374 %374 0 1 2
%376 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%377 = OpLoad %v4float %376
%378 = OpVectorShuffle %v3float %377 %377 0 1 2
%366 = OpSelect %v3float %163 %378 %375
%379 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%380 = OpLoad %v4float %379
%381 = OpVectorShuffle %v3float %380 %380 0 1 2
%382 = OpFOrdEqual %v3bool %366 %381
%383 = OpAll %bool %382
OpBranch %365
%365 = OpLabel
%384 = OpPhi %bool %false %344 %383 %364
OpSelectionMerge %386 None
OpBranchConditional %384 %385 %386
%385 = OpLabel
%388 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%389 = OpLoad %v4float %388
%390 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%391 = OpLoad %v4float %390
%392 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%393 = OpLoad %v4float %392
%394 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%395 = OpLoad %v4float %394
%387 = OpSelect %v4float %178 %395 %393
%396 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%397 = OpLoad %v4float %396
%398 = OpFOrdEqual %v4bool %387 %397
%399 = OpAll %bool %398
OpBranch %386
%386 = OpLabel
%400 = OpPhi %bool %false %365 %399 %385
OpSelectionMerge %402 None
OpBranchConditional %400 %401 %402
%401 = OpLabel
%403 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%404 = OpLoad %v4float %403
%405 = OpCompositeExtract %float %404 0
%406 = OpFOrdEqual %bool %float_0 %405
OpBranch %402
%402 = OpLabel
%407 = OpPhi %bool %false %386 %406 %401
OpSelectionMerge %409 None
OpBranchConditional %407 %408 %409
%408 = OpLabel
%412 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%413 = OpLoad %v4float %412
%414 = OpVectorShuffle %v2float %413 %413 0 1
%415 = OpFOrdEqual %v2bool %411 %414
%416 = OpAll %bool %415
OpBranch %409
%409 = OpLabel
%417 = OpPhi %bool %false %402 %416 %408
OpSelectionMerge %419 None
OpBranchConditional %417 %418 %419
%418 = OpLabel
%421 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%422 = OpLoad %v4float %421
%423 = OpVectorShuffle %v3float %422 %422 0 1 2
%424 = OpFOrdEqual %v3bool %420 %423
%425 = OpAll %bool %424
OpBranch %419
%419 = OpLabel
%426 = OpPhi %bool %false %409 %425 %418
OpSelectionMerge %428 None
OpBranchConditional %426 %427 %428
%427 = OpLabel
%430 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%431 = OpLoad %v4float %430
%432 = OpFOrdEqual %v4bool %429 %431
%433 = OpAll %bool %432
OpBranch %428
%428 = OpLabel
%434 = OpPhi %bool %false %419 %433 %427
OpSelectionMerge %436 None
OpBranchConditional %434 %435 %436
%435 = OpLabel
%437 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%438 = OpLoad %v4float %437
%439 = OpCompositeExtract %float %438 0
%440 = OpFOrdEqual %bool %float_1 %439
OpBranch %436
%436 = OpLabel
%441 = OpPhi %bool %false %428 %440 %435
OpSelectionMerge %443 None
OpBranchConditional %441 %442 %443
%442 = OpLabel
%445 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%446 = OpLoad %v4float %445
%447 = OpVectorShuffle %v2float %446 %446 0 1
%448 = OpFOrdEqual %v2bool %444 %447
%449 = OpAll %bool %448
OpBranch %443
%443 = OpLabel
%450 = OpPhi %bool %false %436 %449 %442
OpSelectionMerge %452 None
OpBranchConditional %450 %451 %452
%451 = OpLabel
%454 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%455 = OpLoad %v4float %454
%456 = OpVectorShuffle %v3float %455 %455 0 1 2
%457 = OpFOrdEqual %v3bool %453 %456
%458 = OpAll %bool %457
OpBranch %452
%452 = OpLabel
%459 = OpPhi %bool %false %443 %458 %451
OpSelectionMerge %461 None
OpBranchConditional %459 %460 %461
%460 = OpLabel
%463 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%464 = OpLoad %v4float %463
%465 = OpFOrdEqual %v4bool %462 %464
%466 = OpAll %bool %465
OpBranch %461
%461 = OpLabel
%467 = OpPhi %bool %false %452 %466 %460
OpSelectionMerge %472 None
OpBranchConditional %467 %470 %471
%470 = OpLabel
%473 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%474 = OpLoad %v4float %473
OpStore %468 %474
OpBranch %472
%471 = OpLabel
%475 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%476 = OpLoad %v4float %475
OpStore %468 %476
OpBranch %472
%472 = OpLabel
%477 = OpLoad %v4float %468
OpReturnValue %477
OpFunctionEnd
