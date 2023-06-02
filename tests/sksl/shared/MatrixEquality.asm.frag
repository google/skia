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
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpMemberName %_UniformBuffer 3 "testMatrix3x3"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_zero "_1_zero"
OpName %_2_one "_2_one"
OpName %_3_two "_3_two"
OpName %_4_nine "_4_nine"
OpName %_5_m "_5_m"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 64
OpMemberDecorate %_UniformBuffer 3 ColMajor
OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %39 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %427 RelaxedPrecision
OpDecorate %428 RelaxedPrecision
OpDecorate %429 RelaxedPrecision
OpDecorate %430 RelaxedPrecision
OpDecorate %431 RelaxedPrecision
OpDecorate %432 RelaxedPrecision
OpDecorate %443 RelaxedPrecision
OpDecorate %444 RelaxedPrecision
OpDecorate %445 RelaxedPrecision
OpDecorate %446 RelaxedPrecision
OpDecorate %447 RelaxedPrecision
OpDecorate %448 RelaxedPrecision
OpDecorate %452 RelaxedPrecision
OpDecorate %453 RelaxedPrecision
OpDecorate %454 RelaxedPrecision
OpDecorate %455 RelaxedPrecision
OpDecorate %456 RelaxedPrecision
OpDecorate %457 RelaxedPrecision
OpDecorate %464 RelaxedPrecision
OpDecorate %465 RelaxedPrecision
OpDecorate %466 RelaxedPrecision
OpDecorate %467 RelaxedPrecision
OpDecorate %468 RelaxedPrecision
OpDecorate %469 RelaxedPrecision
OpDecorate %573 RelaxedPrecision
OpDecorate %575 RelaxedPrecision
OpDecorate %576 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float %mat3v3float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%44 = OpConstantComposite %v2float %float_1 %float_2
%45 = OpConstantComposite %v2float %float_3 %float_4
%46 = OpConstantComposite %mat2v2float %44 %45
%v2bool = OpTypeVector %bool 2
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_3 = OpConstant %int 3
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%67 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%68 = OpConstantComposite %v3float %float_4 %float_5 %float_6
%69 = OpConstantComposite %v3float %float_7 %float_8 %float_9
%70 = OpConstantComposite %mat3v3float %67 %68 %69
%v3bool = OpTypeVector %bool 3
%float_100 = OpConstant %float 100
%89 = OpConstantComposite %v2float %float_100 %float_0
%90 = OpConstantComposite %v2float %float_0 %float_100
%91 = OpConstantComposite %mat2v2float %89 %90
%104 = OpConstantComposite %v3float %float_9 %float_8 %float_7
%105 = OpConstantComposite %v3float %float_6 %float_5 %float_4
%106 = OpConstantComposite %v3float %float_3 %float_2 %float_1
%107 = OpConstantComposite %mat3v3float %104 %105 %106
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%140 = OpConstantComposite %v2float %float_1 %float_0
%141 = OpConstantComposite %v2float %float_0 %float_1
%142 = OpConstantComposite %mat2v2float %140 %141
%178 = OpConstantComposite %mat2v2float %22 %22
%float_n1 = OpConstant %float -1
%192 = OpConstantComposite %v2float %float_n1 %float_0
%193 = OpConstantComposite %v2float %float_0 %float_n1
%194 = OpConstantComposite %mat2v2float %192 %193
%float_n0 = OpConstant %float -0
%207 = OpConstantComposite %v2float %float_n0 %float_0
%208 = OpConstantComposite %v2float %float_0 %float_n0
%209 = OpConstantComposite %mat2v2float %207 %208
%297 = OpConstantComposite %v3float %float_1 %float_0 %float_0
%298 = OpConstantComposite %v3float %float_0 %float_1 %float_0
%299 = OpConstantComposite %v3float %float_0 %float_0 %float_1
%300 = OpConstantComposite %mat3v3float %297 %298 %299
%316 = OpConstantComposite %v2float %float_9 %float_0
%317 = OpConstantComposite %v2float %float_0 %float_9
%318 = OpConstantComposite %mat2v2float %316 %317
%319 = OpConstantComposite %v3float %float_9 %float_0 %float_0
%320 = OpConstantComposite %v3float %float_0 %float_9 %float_0
%321 = OpConstantComposite %mat3v3float %319 %320 %299
%435 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%v4bool = OpTypeVector %bool 4
%472 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_v3float = OpTypePointer Function %v3float
%int_1 = OpConstant %int 1
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %19
%20 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %26
%27 = OpFunctionParameter %_ptr_Function_v2float
%28 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_zero = OpVariable %_ptr_Function_float Function
%_2_one = OpVariable %_ptr_Function_float Function
%_3_two = OpVariable %_ptr_Function_float Function
%_4_nine = OpVariable %_ptr_Function_float Function
%_5_m = OpVariable %_ptr_Function_mat3v3float Function
%567 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
OpSelectionMerge %34 None
OpBranchConditional %true %33 %34
%33 = OpLabel
%35 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%39 = OpLoad %mat2v2float %35
%48 = OpCompositeExtract %v2float %39 0
%49 = OpFOrdEqual %v2bool %48 %44
%50 = OpAll %bool %49
%51 = OpCompositeExtract %v2float %39 1
%52 = OpFOrdEqual %v2bool %51 %45
%53 = OpAll %bool %52
%54 = OpLogicalAnd %bool %50 %53
OpBranch %34
%34 = OpLabel
%55 = OpPhi %bool %false %28 %54 %33
OpStore %_0_ok %55
OpSelectionMerge %57 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
%58 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%61 = OpLoad %mat3v3float %58
%72 = OpCompositeExtract %v3float %61 0
%73 = OpFOrdEqual %v3bool %72 %67
%74 = OpAll %bool %73
%75 = OpCompositeExtract %v3float %61 1
%76 = OpFOrdEqual %v3bool %75 %68
%77 = OpAll %bool %76
%78 = OpLogicalAnd %bool %74 %77
%79 = OpCompositeExtract %v3float %61 2
%80 = OpFOrdEqual %v3bool %79 %69
%81 = OpAll %bool %80
%82 = OpLogicalAnd %bool %78 %81
OpBranch %57
%57 = OpLabel
%83 = OpPhi %bool %false %34 %82 %56
OpStore %_0_ok %83
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%86 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%87 = OpLoad %mat2v2float %86
%92 = OpCompositeExtract %v2float %87 0
%93 = OpFUnordNotEqual %v2bool %92 %89
%94 = OpAny %bool %93
%95 = OpCompositeExtract %v2float %87 1
%96 = OpFUnordNotEqual %v2bool %95 %90
%97 = OpAny %bool %96
%98 = OpLogicalOr %bool %94 %97
OpBranch %85
%85 = OpLabel
%99 = OpPhi %bool %false %57 %98 %84
OpStore %_0_ok %99
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%102 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%103 = OpLoad %mat3v3float %102
%108 = OpCompositeExtract %v3float %103 0
%109 = OpFUnordNotEqual %v3bool %108 %104
%110 = OpAny %bool %109
%111 = OpCompositeExtract %v3float %103 1
%112 = OpFUnordNotEqual %v3bool %111 %105
%113 = OpAny %bool %112
%114 = OpLogicalOr %bool %110 %113
%115 = OpCompositeExtract %v3float %103 2
%116 = OpFUnordNotEqual %v3bool %115 %106
%117 = OpAny %bool %116
%118 = OpLogicalOr %bool %114 %117
OpBranch %101
%101 = OpLabel
%119 = OpPhi %bool %false %85 %118 %100
OpStore %_0_ok %119
%122 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%125 = OpLoad %v4float %122
%126 = OpCompositeExtract %float %125 0
OpStore %_1_zero %126
%128 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%129 = OpLoad %v4float %128
%130 = OpCompositeExtract %float %129 1
OpStore %_2_one %130
%132 = OpFMul %float %float_2 %130
OpStore %_3_two %132
%134 = OpFMul %float %float_9 %130
OpStore %_4_nine %134
OpSelectionMerge %136 None
OpBranchConditional %119 %135 %136
%135 = OpLabel
%137 = OpCompositeConstruct %v2float %130 %126
%138 = OpCompositeConstruct %v2float %126 %130
%139 = OpCompositeConstruct %mat2v2float %137 %138
%143 = OpFOrdEqual %v2bool %137 %140
%144 = OpAll %bool %143
%145 = OpFOrdEqual %v2bool %138 %141
%146 = OpAll %bool %145
%147 = OpLogicalAnd %bool %144 %146
OpBranch %136
%136 = OpLabel
%148 = OpPhi %bool %false %101 %147 %135
OpStore %_0_ok %148
OpSelectionMerge %150 None
OpBranchConditional %148 %149 %150
%149 = OpLabel
%152 = OpCompositeConstruct %v2float %130 %130
%153 = OpCompositeConstruct %v2float %130 %126
%154 = OpCompositeConstruct %mat2v2float %153 %152
%155 = OpFOrdEqual %v2bool %153 %140
%156 = OpAll %bool %155
%157 = OpFOrdEqual %v2bool %152 %141
%158 = OpAll %bool %157
%159 = OpLogicalAnd %bool %156 %158
%151 = OpLogicalNot %bool %159
OpBranch %150
%150 = OpLabel
%160 = OpPhi %bool %false %136 %151 %149
OpStore %_0_ok %160
OpSelectionMerge %162 None
OpBranchConditional %160 %161 %162
%161 = OpLabel
%163 = OpCompositeConstruct %v2float %130 %float_0
%164 = OpCompositeConstruct %v2float %float_0 %130
%165 = OpCompositeConstruct %mat2v2float %163 %164
%166 = OpFOrdEqual %v2bool %163 %140
%167 = OpAll %bool %166
%168 = OpFOrdEqual %v2bool %164 %141
%169 = OpAll %bool %168
%170 = OpLogicalAnd %bool %167 %169
OpBranch %162
%162 = OpLabel
%171 = OpPhi %bool %false %150 %170 %161
OpStore %_0_ok %171
OpSelectionMerge %173 None
OpBranchConditional %171 %172 %173
%172 = OpLabel
%175 = OpCompositeConstruct %v2float %130 %float_0
%176 = OpCompositeConstruct %v2float %float_0 %130
%177 = OpCompositeConstruct %mat2v2float %175 %176
%179 = OpFOrdEqual %v2bool %175 %22
%180 = OpAll %bool %179
%181 = OpFOrdEqual %v2bool %176 %22
%182 = OpAll %bool %181
%183 = OpLogicalAnd %bool %180 %182
%174 = OpLogicalNot %bool %183
OpBranch %173
%173 = OpLabel
%184 = OpPhi %bool %false %162 %174 %172
OpStore %_0_ok %184
OpSelectionMerge %186 None
OpBranchConditional %184 %185 %186
%185 = OpLabel
%187 = OpFNegate %float %130
%188 = OpCompositeConstruct %v2float %187 %float_0
%189 = OpCompositeConstruct %v2float %float_0 %187
%190 = OpCompositeConstruct %mat2v2float %188 %189
%195 = OpFOrdEqual %v2bool %188 %192
%196 = OpAll %bool %195
%197 = OpFOrdEqual %v2bool %189 %193
%198 = OpAll %bool %197
%199 = OpLogicalAnd %bool %196 %198
OpBranch %186
%186 = OpLabel
%200 = OpPhi %bool %false %173 %199 %185
OpStore %_0_ok %200
OpSelectionMerge %202 None
OpBranchConditional %200 %201 %202
%201 = OpLabel
%203 = OpCompositeConstruct %v2float %126 %float_0
%204 = OpCompositeConstruct %v2float %float_0 %126
%205 = OpCompositeConstruct %mat2v2float %203 %204
%210 = OpFOrdEqual %v2bool %203 %207
%211 = OpAll %bool %210
%212 = OpFOrdEqual %v2bool %204 %208
%213 = OpAll %bool %212
%214 = OpLogicalAnd %bool %211 %213
OpBranch %202
%202 = OpLabel
%215 = OpPhi %bool %false %186 %214 %201
OpStore %_0_ok %215
OpSelectionMerge %217 None
OpBranchConditional %215 %216 %217
%216 = OpLabel
%218 = OpFNegate %float %130
%219 = OpCompositeConstruct %v2float %218 %float_0
%220 = OpCompositeConstruct %v2float %float_0 %218
%221 = OpCompositeConstruct %mat2v2float %219 %220
%222 = OpFNegate %v2float %219
%223 = OpFNegate %v2float %220
%224 = OpCompositeConstruct %mat2v2float %222 %223
%225 = OpFOrdEqual %v2bool %222 %140
%226 = OpAll %bool %225
%227 = OpFOrdEqual %v2bool %223 %141
%228 = OpAll %bool %227
%229 = OpLogicalAnd %bool %226 %228
OpBranch %217
%217 = OpLabel
%230 = OpPhi %bool %false %202 %229 %216
OpStore %_0_ok %230
OpSelectionMerge %232 None
OpBranchConditional %230 %231 %232
%231 = OpLabel
%233 = OpCompositeConstruct %v2float %126 %float_0
%234 = OpCompositeConstruct %v2float %float_0 %126
%235 = OpCompositeConstruct %mat2v2float %233 %234
%236 = OpFNegate %v2float %233
%237 = OpFNegate %v2float %234
%238 = OpCompositeConstruct %mat2v2float %236 %237
%239 = OpFOrdEqual %v2bool %236 %207
%240 = OpAll %bool %239
%241 = OpFOrdEqual %v2bool %237 %208
%242 = OpAll %bool %241
%243 = OpLogicalAnd %bool %240 %242
OpBranch %232
%232 = OpLabel
%244 = OpPhi %bool %false %217 %243 %231
OpStore %_0_ok %244
OpSelectionMerge %246 None
OpBranchConditional %244 %245 %246
%245 = OpLabel
%247 = OpCompositeConstruct %v2float %130 %float_0
%248 = OpCompositeConstruct %v2float %float_0 %130
%249 = OpCompositeConstruct %mat2v2float %247 %248
%250 = OpFOrdEqual %v2bool %247 %140
%251 = OpAll %bool %250
%252 = OpFOrdEqual %v2bool %248 %141
%253 = OpAll %bool %252
%254 = OpLogicalAnd %bool %251 %253
OpBranch %246
%246 = OpLabel
%255 = OpPhi %bool %false %232 %254 %245
OpStore %_0_ok %255
OpSelectionMerge %257 None
OpBranchConditional %255 %256 %257
%256 = OpLabel
%259 = OpCompositeConstruct %v2float %132 %float_0
%260 = OpCompositeConstruct %v2float %float_0 %132
%261 = OpCompositeConstruct %mat2v2float %259 %260
%262 = OpFOrdEqual %v2bool %259 %140
%263 = OpAll %bool %262
%264 = OpFOrdEqual %v2bool %260 %141
%265 = OpAll %bool %264
%266 = OpLogicalAnd %bool %263 %265
%258 = OpLogicalNot %bool %266
OpBranch %257
%257 = OpLabel
%267 = OpPhi %bool %false %246 %258 %256
OpStore %_0_ok %267
OpSelectionMerge %269 None
OpBranchConditional %267 %268 %269
%268 = OpLabel
%271 = OpCompositeConstruct %v2float %130 %float_0
%272 = OpCompositeConstruct %v2float %float_0 %130
%273 = OpCompositeConstruct %mat2v2float %271 %272
%274 = OpFUnordNotEqual %v2bool %271 %140
%275 = OpAny %bool %274
%276 = OpFUnordNotEqual %v2bool %272 %141
%277 = OpAny %bool %276
%278 = OpLogicalOr %bool %275 %277
%270 = OpLogicalNot %bool %278
OpBranch %269
%269 = OpLabel
%279 = OpPhi %bool %false %257 %270 %268
OpStore %_0_ok %279
OpSelectionMerge %281 None
OpBranchConditional %279 %280 %281
%280 = OpLabel
%282 = OpCompositeConstruct %v2float %130 %float_0
%283 = OpCompositeConstruct %v2float %float_0 %130
%284 = OpCompositeConstruct %mat2v2float %282 %283
%285 = OpFUnordNotEqual %v2bool %282 %22
%286 = OpAny %bool %285
%287 = OpFUnordNotEqual %v2bool %283 %22
%288 = OpAny %bool %287
%289 = OpLogicalOr %bool %286 %288
OpBranch %281
%281 = OpLabel
%290 = OpPhi %bool %false %269 %289 %280
OpStore %_0_ok %290
OpSelectionMerge %292 None
OpBranchConditional %290 %291 %292
%291 = OpLabel
%293 = OpCompositeConstruct %v3float %130 %126 %126
%294 = OpCompositeConstruct %v3float %126 %130 %126
%295 = OpCompositeConstruct %v3float %126 %126 %130
%296 = OpCompositeConstruct %mat3v3float %293 %294 %295
%301 = OpFOrdEqual %v3bool %293 %297
%302 = OpAll %bool %301
%303 = OpFOrdEqual %v3bool %294 %298
%304 = OpAll %bool %303
%305 = OpLogicalAnd %bool %302 %304
%306 = OpFOrdEqual %v3bool %295 %299
%307 = OpAll %bool %306
%308 = OpLogicalAnd %bool %305 %307
OpBranch %292
%292 = OpLabel
%309 = OpPhi %bool %false %281 %308 %291
OpStore %_0_ok %309
OpSelectionMerge %311 None
OpBranchConditional %309 %310 %311
%310 = OpLabel
%312 = OpCompositeConstruct %v3float %134 %126 %126
%313 = OpCompositeConstruct %v3float %126 %134 %126
%314 = OpCompositeConstruct %v3float %126 %126 %130
%315 = OpCompositeConstruct %mat3v3float %312 %313 %314
%322 = OpFOrdEqual %v3bool %312 %319
%323 = OpAll %bool %322
%324 = OpFOrdEqual %v3bool %313 %320
%325 = OpAll %bool %324
%326 = OpLogicalAnd %bool %323 %325
%327 = OpFOrdEqual %v3bool %314 %299
%328 = OpAll %bool %327
%329 = OpLogicalAnd %bool %326 %328
OpBranch %311
%311 = OpLabel
%330 = OpPhi %bool %false %292 %329 %310
OpStore %_0_ok %330
OpSelectionMerge %332 None
OpBranchConditional %330 %331 %332
%331 = OpLabel
%333 = OpCompositeConstruct %v3float %130 %float_0 %float_0
%334 = OpCompositeConstruct %v3float %float_0 %130 %float_0
%335 = OpCompositeConstruct %v3float %float_0 %float_0 %130
%336 = OpCompositeConstruct %mat3v3float %333 %334 %335
%337 = OpFOrdEqual %v3bool %333 %297
%338 = OpAll %bool %337
%339 = OpFOrdEqual %v3bool %334 %298
%340 = OpAll %bool %339
%341 = OpLogicalAnd %bool %338 %340
%342 = OpFOrdEqual %v3bool %335 %299
%343 = OpAll %bool %342
%344 = OpLogicalAnd %bool %341 %343
OpBranch %332
%332 = OpLabel
%345 = OpPhi %bool %false %311 %344 %331
OpStore %_0_ok %345
OpSelectionMerge %347 None
OpBranchConditional %345 %346 %347
%346 = OpLabel
%348 = OpCompositeConstruct %v3float %134 %float_0 %float_0
%349 = OpCompositeConstruct %v3float %float_0 %134 %float_0
%350 = OpCompositeConstruct %v3float %float_0 %float_0 %130
%351 = OpCompositeConstruct %mat3v3float %348 %349 %350
%352 = OpFOrdEqual %v3bool %348 %319
%353 = OpAll %bool %352
%354 = OpFOrdEqual %v3bool %349 %320
%355 = OpAll %bool %354
%356 = OpLogicalAnd %bool %353 %355
%357 = OpFOrdEqual %v3bool %350 %299
%358 = OpAll %bool %357
%359 = OpLogicalAnd %bool %356 %358
OpBranch %347
%347 = OpLabel
%360 = OpPhi %bool %false %332 %359 %346
OpStore %_0_ok %360
OpSelectionMerge %362 None
OpBranchConditional %360 %361 %362
%361 = OpLabel
%363 = OpCompositeConstruct %v3float %130 %float_0 %float_0
%364 = OpCompositeConstruct %v3float %float_0 %130 %float_0
%365 = OpCompositeConstruct %v3float %float_0 %float_0 %130
%366 = OpCompositeConstruct %mat3v3float %363 %364 %365
%367 = OpVectorShuffle %v2float %363 %363 0 1
%368 = OpVectorShuffle %v2float %364 %364 0 1
%369 = OpCompositeConstruct %mat2v2float %367 %368
%370 = OpFOrdEqual %v2bool %367 %140
%371 = OpAll %bool %370
%372 = OpFOrdEqual %v2bool %368 %141
%373 = OpAll %bool %372
%374 = OpLogicalAnd %bool %371 %373
OpBranch %362
%362 = OpLabel
%375 = OpPhi %bool %false %347 %374 %361
OpStore %_0_ok %375
OpSelectionMerge %377 None
OpBranchConditional %375 %376 %377
%376 = OpLabel
%378 = OpCompositeConstruct %v3float %130 %float_0 %float_0
%379 = OpCompositeConstruct %v3float %float_0 %130 %float_0
%380 = OpCompositeConstruct %v3float %float_0 %float_0 %130
%381 = OpCompositeConstruct %mat3v3float %378 %379 %380
%382 = OpVectorShuffle %v2float %378 %378 0 1
%383 = OpVectorShuffle %v2float %379 %379 0 1
%384 = OpCompositeConstruct %mat2v2float %382 %383
%385 = OpFOrdEqual %v2bool %382 %140
%386 = OpAll %bool %385
%387 = OpFOrdEqual %v2bool %383 %141
%388 = OpAll %bool %387
%389 = OpLogicalAnd %bool %386 %388
OpBranch %377
%377 = OpLabel
%390 = OpPhi %bool %false %362 %389 %376
OpStore %_0_ok %390
OpSelectionMerge %392 None
OpBranchConditional %390 %391 %392
%391 = OpLabel
%393 = OpCompositeConstruct %v2float %130 %126
%394 = OpCompositeConstruct %v2float %126 %130
%395 = OpCompositeConstruct %mat2v2float %393 %394
%396 = OpFOrdEqual %v2bool %393 %140
%397 = OpAll %bool %396
%398 = OpFOrdEqual %v2bool %394 %141
%399 = OpAll %bool %398
%400 = OpLogicalAnd %bool %397 %399
OpBranch %392
%392 = OpLabel
%401 = OpPhi %bool %false %377 %400 %391
OpStore %_0_ok %401
OpSelectionMerge %403 None
OpBranchConditional %401 %402 %403
%402 = OpLabel
%404 = OpCompositeConstruct %v2float %130 %126
%405 = OpCompositeConstruct %v2float %126 %130
%406 = OpCompositeConstruct %mat2v2float %404 %405
%407 = OpFOrdEqual %v2bool %404 %140
%408 = OpAll %bool %407
%409 = OpFOrdEqual %v2bool %405 %141
%410 = OpAll %bool %409
%411 = OpLogicalAnd %bool %408 %410
OpBranch %403
%403 = OpLabel
%412 = OpPhi %bool %false %392 %411 %402
OpStore %_0_ok %412
OpSelectionMerge %414 None
OpBranchConditional %412 %413 %414
%413 = OpLabel
%415 = OpCompositeConstruct %v2float %130 %126
%416 = OpCompositeConstruct %v2float %126 %130
%417 = OpCompositeConstruct %mat2v2float %415 %416
%418 = OpFOrdEqual %v2bool %415 %140
%419 = OpAll %bool %418
%420 = OpFOrdEqual %v2bool %416 %141
%421 = OpAll %bool %420
%422 = OpLogicalAnd %bool %419 %421
OpBranch %414
%414 = OpLabel
%423 = OpPhi %bool %false %403 %422 %413
OpStore %_0_ok %423
OpSelectionMerge %425 None
OpBranchConditional %423 %424 %425
%424 = OpLabel
%426 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%427 = OpLoad %mat2v2float %426
%428 = OpCompositeExtract %float %427 0 0
%429 = OpCompositeExtract %float %427 0 1
%430 = OpCompositeExtract %float %427 1 0
%431 = OpCompositeExtract %float %427 1 1
%432 = OpCompositeConstruct %v4float %428 %429 %430 %431
%433 = OpCompositeConstruct %v4float %130 %130 %130 %130
%434 = OpFMul %v4float %432 %433
%436 = OpFOrdEqual %v4bool %434 %435
%438 = OpAll %bool %436
OpBranch %425
%425 = OpLabel
%439 = OpPhi %bool %false %414 %438 %424
OpStore %_0_ok %439
OpSelectionMerge %441 None
OpBranchConditional %439 %440 %441
%440 = OpLabel
%442 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%443 = OpLoad %mat2v2float %442
%444 = OpCompositeExtract %float %443 0 0
%445 = OpCompositeExtract %float %443 0 1
%446 = OpCompositeExtract %float %443 1 0
%447 = OpCompositeExtract %float %443 1 1
%448 = OpCompositeConstruct %v4float %444 %445 %446 %447
%449 = OpCompositeConstruct %v4float %130 %130 %130 %130
%450 = OpFMul %v4float %448 %449
%451 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%452 = OpLoad %mat2v2float %451
%453 = OpCompositeExtract %float %452 0 0
%454 = OpCompositeExtract %float %452 0 1
%455 = OpCompositeExtract %float %452 1 0
%456 = OpCompositeExtract %float %452 1 1
%457 = OpCompositeConstruct %v4float %453 %454 %455 %456
%458 = OpFOrdEqual %v4bool %450 %457
%459 = OpAll %bool %458
OpBranch %441
%441 = OpLabel
%460 = OpPhi %bool %false %425 %459 %440
OpStore %_0_ok %460
OpSelectionMerge %462 None
OpBranchConditional %460 %461 %462
%461 = OpLabel
%463 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%464 = OpLoad %mat2v2float %463
%465 = OpCompositeExtract %float %464 0 0
%466 = OpCompositeExtract %float %464 0 1
%467 = OpCompositeExtract %float %464 1 0
%468 = OpCompositeExtract %float %464 1 1
%469 = OpCompositeConstruct %v4float %465 %466 %467 %468
%470 = OpCompositeConstruct %v4float %126 %126 %126 %126
%471 = OpFMul %v4float %469 %470
%473 = OpFOrdEqual %v4bool %471 %472
%474 = OpAll %bool %473
OpBranch %462
%462 = OpLabel
%475 = OpPhi %bool %false %441 %474 %461
OpStore %_0_ok %475
%478 = OpCompositeConstruct %v3float %130 %132 %float_3
%479 = OpCompositeConstruct %v3float %float_7 %float_8 %134
%480 = OpCompositeConstruct %mat3v3float %478 %68 %479
OpStore %_5_m %480
OpSelectionMerge %482 None
OpBranchConditional %475 %481 %482
%481 = OpLabel
%483 = OpAccessChain %_ptr_Function_v3float %_5_m %int_0
%485 = OpLoad %v3float %483
%486 = OpFOrdEqual %v3bool %485 %67
%487 = OpAll %bool %486
OpBranch %482
%482 = OpLabel
%488 = OpPhi %bool %false %462 %487 %481
OpStore %_0_ok %488
OpSelectionMerge %490 None
OpBranchConditional %488 %489 %490
%489 = OpLabel
%492 = OpAccessChain %_ptr_Function_v3float %_5_m %int_1
%493 = OpLoad %v3float %492
%494 = OpFOrdEqual %v3bool %493 %68
%495 = OpAll %bool %494
OpBranch %490
%490 = OpLabel
%496 = OpPhi %bool %false %482 %495 %489
OpStore %_0_ok %496
OpSelectionMerge %498 None
OpBranchConditional %496 %497 %498
%497 = OpLabel
%499 = OpAccessChain %_ptr_Function_v3float %_5_m %int_2
%500 = OpLoad %v3float %499
%501 = OpFOrdEqual %v3bool %500 %69
%502 = OpAll %bool %501
OpBranch %498
%498 = OpLabel
%503 = OpPhi %bool %false %490 %502 %497
OpStore %_0_ok %503
OpSelectionMerge %505 None
OpBranchConditional %503 %504 %505
%504 = OpLabel
%506 = OpAccessChain %_ptr_Function_v3float %_5_m %int_0
%507 = OpLoad %v3float %506
%508 = OpCompositeExtract %float %507 0
%509 = OpFOrdEqual %bool %508 %float_1
OpBranch %505
%505 = OpLabel
%510 = OpPhi %bool %false %498 %509 %504
OpStore %_0_ok %510
OpSelectionMerge %512 None
OpBranchConditional %510 %511 %512
%511 = OpLabel
%513 = OpAccessChain %_ptr_Function_v3float %_5_m %int_0
%514 = OpLoad %v3float %513
%515 = OpCompositeExtract %float %514 1
%516 = OpFOrdEqual %bool %515 %float_2
OpBranch %512
%512 = OpLabel
%517 = OpPhi %bool %false %505 %516 %511
OpStore %_0_ok %517
OpSelectionMerge %519 None
OpBranchConditional %517 %518 %519
%518 = OpLabel
%520 = OpAccessChain %_ptr_Function_v3float %_5_m %int_0
%521 = OpLoad %v3float %520
%522 = OpCompositeExtract %float %521 2
%523 = OpFOrdEqual %bool %522 %float_3
OpBranch %519
%519 = OpLabel
%524 = OpPhi %bool %false %512 %523 %518
OpStore %_0_ok %524
OpSelectionMerge %526 None
OpBranchConditional %524 %525 %526
%525 = OpLabel
%527 = OpAccessChain %_ptr_Function_v3float %_5_m %int_1
%528 = OpLoad %v3float %527
%529 = OpCompositeExtract %float %528 0
%530 = OpFOrdEqual %bool %529 %float_4
OpBranch %526
%526 = OpLabel
%531 = OpPhi %bool %false %519 %530 %525
OpStore %_0_ok %531
OpSelectionMerge %533 None
OpBranchConditional %531 %532 %533
%532 = OpLabel
%534 = OpAccessChain %_ptr_Function_v3float %_5_m %int_1
%535 = OpLoad %v3float %534
%536 = OpCompositeExtract %float %535 1
%537 = OpFOrdEqual %bool %536 %float_5
OpBranch %533
%533 = OpLabel
%538 = OpPhi %bool %false %526 %537 %532
OpStore %_0_ok %538
OpSelectionMerge %540 None
OpBranchConditional %538 %539 %540
%539 = OpLabel
%541 = OpAccessChain %_ptr_Function_v3float %_5_m %int_1
%542 = OpLoad %v3float %541
%543 = OpCompositeExtract %float %542 2
%544 = OpFOrdEqual %bool %543 %float_6
OpBranch %540
%540 = OpLabel
%545 = OpPhi %bool %false %533 %544 %539
OpStore %_0_ok %545
OpSelectionMerge %547 None
OpBranchConditional %545 %546 %547
%546 = OpLabel
%548 = OpAccessChain %_ptr_Function_v3float %_5_m %int_2
%549 = OpLoad %v3float %548
%550 = OpCompositeExtract %float %549 0
%551 = OpFOrdEqual %bool %550 %float_7
OpBranch %547
%547 = OpLabel
%552 = OpPhi %bool %false %540 %551 %546
OpStore %_0_ok %552
OpSelectionMerge %554 None
OpBranchConditional %552 %553 %554
%553 = OpLabel
%555 = OpAccessChain %_ptr_Function_v3float %_5_m %int_2
%556 = OpLoad %v3float %555
%557 = OpCompositeExtract %float %556 1
%558 = OpFOrdEqual %bool %557 %float_8
OpBranch %554
%554 = OpLabel
%559 = OpPhi %bool %false %547 %558 %553
OpStore %_0_ok %559
OpSelectionMerge %561 None
OpBranchConditional %559 %560 %561
%560 = OpLabel
%562 = OpAccessChain %_ptr_Function_v3float %_5_m %int_2
%563 = OpLoad %v3float %562
%564 = OpCompositeExtract %float %563 2
%565 = OpFOrdEqual %bool %564 %float_9
OpBranch %561
%561 = OpLabel
%566 = OpPhi %bool %false %554 %565 %560
OpStore %_0_ok %566
OpSelectionMerge %571 None
OpBranchConditional %566 %569 %570
%569 = OpLabel
%572 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%573 = OpLoad %v4float %572
OpStore %567 %573
OpBranch %571
%570 = OpLabel
%574 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%575 = OpLoad %v4float %574
OpStore %567 %575
OpBranch %571
%571 = OpLabel
%576 = OpLoad %v4float %567
OpReturnValue %576
OpFunctionEnd
