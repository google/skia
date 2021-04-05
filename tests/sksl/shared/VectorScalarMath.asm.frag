OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorRed"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "unknownInput"
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_int_b "test_int_b"
OpName %ok "ok"
OpName %inputRed "inputRed"
OpName %inputGreen "inputGreen"
OpName %x "x"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_inputRed "_1_inputRed"
OpName %_2_inputGreen "_2_inputGreen"
OpName %_3_x "_3_x"
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
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %_1_inputRed RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %_2_inputGreen RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %_3_x RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %338 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %383 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %398 RelaxedPrecision
OpDecorate %399 RelaxedPrecision
OpDecorate %400 RelaxedPrecision
OpDecorate %403 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
OpDecorate %411 RelaxedPrecision
OpDecorate %412 RelaxedPrecision
OpDecorate %413 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %415 RelaxedPrecision
OpDecorate %418 RelaxedPrecision
OpDecorate %419 RelaxedPrecision
OpDecorate %423 RelaxedPrecision
OpDecorate %424 RelaxedPrecision
OpDecorate %425 RelaxedPrecision
OpDecorate %426 RelaxedPrecision
OpDecorate %427 RelaxedPrecision
OpDecorate %430 RelaxedPrecision
OpDecorate %431 RelaxedPrecision
OpDecorate %435 RelaxedPrecision
OpDecorate %436 RelaxedPrecision
OpDecorate %437 RelaxedPrecision
OpDecorate %438 RelaxedPrecision
OpDecorate %439 RelaxedPrecision
OpDecorate %440 RelaxedPrecision
OpDecorate %441 RelaxedPrecision
OpDecorate %442 RelaxedPrecision
OpDecorate %443 RelaxedPrecision
OpDecorate %444 RelaxedPrecision
OpDecorate %445 RelaxedPrecision
OpDecorate %446 RelaxedPrecision
OpDecorate %449 RelaxedPrecision
OpDecorate %450 RelaxedPrecision
OpDecorate %454 RelaxedPrecision
OpDecorate %455 RelaxedPrecision
OpDecorate %456 RelaxedPrecision
OpDecorate %457 RelaxedPrecision
OpDecorate %458 RelaxedPrecision
OpDecorate %459 RelaxedPrecision
OpDecorate %460 RelaxedPrecision
OpDecorate %461 RelaxedPrecision
OpDecorate %462 RelaxedPrecision
OpDecorate %464 RelaxedPrecision
OpDecorate %465 RelaxedPrecision
OpDecorate %468 RelaxedPrecision
OpDecorate %469 RelaxedPrecision
OpDecorate %473 RelaxedPrecision
OpDecorate %483 RelaxedPrecision
OpDecorate %485 RelaxedPrecision
OpDecorate %486 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%19 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%false = OpConstantFalse %bool
%int_3 = OpConstant %int 3
%v4bool = OpTypeVector %bool 4
%int_n1 = OpConstant %int -1
%int_n2 = OpConstant %int -2
%v3int = OpTypeVector %int 3
%int_9 = OpConstant %int 9
%v2int = OpTypeVector %int 2
%int_5 = OpConstant %int 5
%int_10 = OpConstant %int 10
%int_36 = OpConstant %int 36
%int_4 = OpConstant %int 4
%int_18 = OpConstant %int 18
%262 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_n1 = OpConstant %float -1
%float_n2 = OpConstant %float -2
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%float_9 = OpConstant %float 9
%v2float = OpTypeVector %float 2
%float_18 = OpConstant %float 18
%float_4 = OpConstant %float 4
%float_5 = OpConstant %float 5
%float_0 = OpConstant %float 0
%float_10 = OpConstant %float 10
%float_36 = OpConstant %float 36
%float_0_5 = OpConstant %float 0.5
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%18 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %18
OpReturn
OpFunctionEnd
%test_int_b = OpFunction %bool None %19
%20 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%inputRed = OpVariable %_ptr_Function_v4int Function
%inputGreen = OpVariable %_ptr_Function_v4int Function
%x = OpVariable %_ptr_Function_v4int Function
OpStore %ok %true
%28 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%31 = OpLoad %v4float %28
%32 = OpCompositeExtract %float %31 0
%33 = OpConvertFToS %int %32
%34 = OpCompositeExtract %float %31 1
%35 = OpConvertFToS %int %34
%36 = OpCompositeExtract %float %31 2
%37 = OpConvertFToS %int %36
%38 = OpCompositeExtract %float %31 3
%39 = OpConvertFToS %int %38
%40 = OpCompositeConstruct %v4int %33 %35 %37 %39
OpStore %inputRed %40
%42 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%44 = OpLoad %v4float %42
%45 = OpCompositeExtract %float %44 0
%46 = OpConvertFToS %int %45
%47 = OpCompositeExtract %float %44 1
%48 = OpConvertFToS %int %47
%49 = OpCompositeExtract %float %44 2
%50 = OpConvertFToS %int %49
%51 = OpCompositeExtract %float %44 3
%52 = OpConvertFToS %int %51
%53 = OpCompositeConstruct %v4int %46 %48 %50 %52
OpStore %inputGreen %53
%55 = OpLoad %v4int %inputRed
%57 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%58 = OpIAdd %v4int %55 %57
OpStore %x %58
%60 = OpLoad %bool %ok
OpSelectionMerge %62 None
OpBranchConditional %60 %61 %62
%61 = OpLabel
%63 = OpLoad %v4int %x
%65 = OpCompositeConstruct %v4int %int_3 %int_2 %int_2 %int_3
%66 = OpIEqual %v4bool %63 %65
%68 = OpAll %bool %66
OpBranch %62
%62 = OpLabel
%69 = OpPhi %bool %false %20 %68 %61
OpStore %ok %69
%70 = OpLoad %v4int %inputGreen
%71 = OpVectorShuffle %v4int %70 %70 1 3 0 2
%72 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%73 = OpISub %v4int %71 %72
OpStore %x %73
%74 = OpLoad %bool %ok
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%77 = OpLoad %v4int %x
%80 = OpCompositeConstruct %v4int %int_n1 %int_n1 %int_n2 %int_n2
%81 = OpIEqual %v4bool %77 %80
%82 = OpAll %bool %81
OpBranch %76
%76 = OpLabel
%83 = OpPhi %bool %false %62 %82 %75
OpStore %ok %83
%84 = OpLoad %v4int %inputRed
%85 = OpLoad %v4int %inputGreen
%86 = OpCompositeExtract %int %85 1
%87 = OpCompositeConstruct %v4int %86 %86 %86 %86
%88 = OpIAdd %v4int %84 %87
OpStore %x %88
%89 = OpLoad %bool %ok
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%92 = OpLoad %v4int %x
%93 = OpCompositeConstruct %v4int %int_2 %int_1 %int_1 %int_2
%94 = OpIEqual %v4bool %92 %93
%95 = OpAll %bool %94
OpBranch %91
%91 = OpLabel
%96 = OpPhi %bool %false %76 %95 %90
OpStore %ok %96
%97 = OpLoad %v4int %inputGreen
%98 = OpVectorShuffle %v3int %97 %97 3 1 3
%101 = OpCompositeConstruct %v3int %int_9 %int_9 %int_9
%102 = OpIMul %v3int %98 %101
%103 = OpLoad %v4int %x
%104 = OpVectorShuffle %v4int %103 %102 4 5 6 3
OpStore %x %104
%105 = OpLoad %bool %ok
OpSelectionMerge %107 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
%108 = OpLoad %v4int %x
%109 = OpCompositeConstruct %v4int %int_9 %int_9 %int_9 %int_2
%110 = OpIEqual %v4bool %108 %109
%111 = OpAll %bool %110
OpBranch %107
%107 = OpLabel
%112 = OpPhi %bool %false %91 %111 %106
OpStore %ok %112
%113 = OpLoad %v4int %x
%114 = OpVectorShuffle %v2int %113 %113 2 3
%116 = OpCompositeConstruct %v2int %int_3 %int_3
%117 = OpSDiv %v2int %114 %116
%118 = OpLoad %v4int %x
%119 = OpVectorShuffle %v4int %118 %117 4 5 2 3
OpStore %x %119
%120 = OpLoad %bool %ok
OpSelectionMerge %122 None
OpBranchConditional %120 %121 %122
%121 = OpLabel
%123 = OpLoad %v4int %x
%124 = OpCompositeConstruct %v4int %int_3 %int_0 %int_9 %int_2
%125 = OpIEqual %v4bool %123 %124
%126 = OpAll %bool %125
OpBranch %122
%122 = OpLabel
%127 = OpPhi %bool %false %107 %126 %121
OpStore %ok %127
%128 = OpLoad %v4int %inputRed
%130 = OpCompositeConstruct %v4int %int_5 %int_5 %int_5 %int_5
%131 = OpIMul %v4int %128 %130
%132 = OpVectorShuffle %v4int %131 %131 1 0 3 2
OpStore %x %132
%133 = OpLoad %bool %ok
OpSelectionMerge %135 None
OpBranchConditional %133 %134 %135
%134 = OpLabel
%136 = OpLoad %v4int %x
%137 = OpCompositeConstruct %v4int %int_0 %int_5 %int_5 %int_0
%138 = OpIEqual %v4bool %136 %137
%139 = OpAll %bool %138
OpBranch %135
%135 = OpLabel
%140 = OpPhi %bool %false %122 %139 %134
OpStore %ok %140
%141 = OpLoad %v4int %inputRed
%142 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%143 = OpIAdd %v4int %142 %141
OpStore %x %143
%144 = OpLoad %bool %ok
OpSelectionMerge %146 None
OpBranchConditional %144 %145 %146
%145 = OpLabel
%147 = OpLoad %v4int %x
%148 = OpCompositeConstruct %v4int %int_3 %int_2 %int_2 %int_3
%149 = OpIEqual %v4bool %147 %148
%150 = OpAll %bool %149
OpBranch %146
%146 = OpLabel
%151 = OpPhi %bool %false %135 %150 %145
OpStore %ok %151
%153 = OpLoad %v4int %inputGreen
%154 = OpVectorShuffle %v4int %153 %153 1 3 0 2
%155 = OpCompositeConstruct %v4int %int_10 %int_10 %int_10 %int_10
%156 = OpISub %v4int %155 %154
OpStore %x %156
%157 = OpLoad %bool %ok
OpSelectionMerge %159 None
OpBranchConditional %157 %158 %159
%158 = OpLabel
%160 = OpLoad %v4int %x
%161 = OpCompositeConstruct %v4int %int_9 %int_9 %int_10 %int_10
%162 = OpIEqual %v4bool %160 %161
%163 = OpAll %bool %162
OpBranch %159
%159 = OpLabel
%164 = OpPhi %bool %false %146 %163 %158
OpStore %ok %164
%165 = OpLoad %v4int %inputRed
%166 = OpCompositeExtract %int %165 0
%167 = OpLoad %v4int %inputGreen
%168 = OpCompositeConstruct %v4int %166 %166 %166 %166
%169 = OpIAdd %v4int %168 %167
OpStore %x %169
%170 = OpLoad %bool %ok
OpSelectionMerge %172 None
OpBranchConditional %170 %171 %172
%171 = OpLabel
%173 = OpLoad %v4int %x
%174 = OpCompositeConstruct %v4int %int_1 %int_2 %int_1 %int_2
%175 = OpIEqual %v4bool %173 %174
%176 = OpAll %bool %175
OpBranch %172
%172 = OpLabel
%177 = OpPhi %bool %false %159 %176 %171
OpStore %ok %177
%178 = OpLoad %v4int %inputGreen
%179 = OpVectorShuffle %v3int %178 %178 3 1 3
%180 = OpCompositeConstruct %v3int %int_9 %int_9 %int_9
%181 = OpIMul %v3int %180 %179
%182 = OpLoad %v4int %x
%183 = OpVectorShuffle %v4int %182 %181 4 5 6 3
OpStore %x %183
%184 = OpLoad %bool %ok
OpSelectionMerge %186 None
OpBranchConditional %184 %185 %186
%185 = OpLabel
%187 = OpLoad %v4int %x
%188 = OpCompositeConstruct %v4int %int_9 %int_9 %int_9 %int_2
%189 = OpIEqual %v4bool %187 %188
%190 = OpAll %bool %189
OpBranch %186
%186 = OpLabel
%191 = OpPhi %bool %false %172 %190 %185
OpStore %ok %191
%193 = OpLoad %v4int %x
%194 = OpVectorShuffle %v2int %193 %193 2 3
%195 = OpCompositeConstruct %v2int %int_36 %int_36
%196 = OpSDiv %v2int %195 %194
%197 = OpLoad %v4int %x
%198 = OpVectorShuffle %v4int %197 %196 4 5 2 3
OpStore %x %198
%199 = OpLoad %bool %ok
OpSelectionMerge %201 None
OpBranchConditional %199 %200 %201
%200 = OpLabel
%202 = OpLoad %v4int %x
%205 = OpCompositeConstruct %v4int %int_4 %int_18 %int_9 %int_2
%206 = OpIEqual %v4bool %202 %205
%207 = OpAll %bool %206
OpBranch %201
%201 = OpLabel
%208 = OpPhi %bool %false %186 %207 %200
OpStore %ok %208
%209 = OpLoad %v4int %x
%210 = OpCompositeConstruct %v4int %int_36 %int_36 %int_36 %int_36
%211 = OpSDiv %v4int %210 %209
%212 = OpVectorShuffle %v4int %211 %211 1 0 3 2
OpStore %x %212
%213 = OpLoad %bool %ok
OpSelectionMerge %215 None
OpBranchConditional %213 %214 %215
%214 = OpLabel
%216 = OpLoad %v4int %x
%217 = OpCompositeConstruct %v4int %int_2 %int_9 %int_18 %int_4
%218 = OpIEqual %v4bool %216 %217
%219 = OpAll %bool %218
OpBranch %215
%215 = OpLabel
%220 = OpPhi %bool %false %201 %219 %214
OpStore %ok %220
%221 = OpLoad %v4int %x
%222 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%223 = OpIAdd %v4int %221 %222
OpStore %x %223
%224 = OpLoad %v4int %x
%225 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%226 = OpIMul %v4int %224 %225
OpStore %x %226
%227 = OpLoad %v4int %x
%228 = OpCompositeConstruct %v4int %int_4 %int_4 %int_4 %int_4
%229 = OpISub %v4int %227 %228
OpStore %x %229
%230 = OpLoad %v4int %x
%231 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%232 = OpSDiv %v4int %230 %231
OpStore %x %232
%233 = OpLoad %bool %ok
OpSelectionMerge %235 None
OpBranchConditional %233 %234 %235
%234 = OpLabel
%236 = OpLoad %v4int %x
%237 = OpCompositeConstruct %v4int %int_2 %int_9 %int_18 %int_4
%238 = OpIEqual %v4bool %236 %237
%239 = OpAll %bool %238
OpBranch %235
%235 = OpLabel
%240 = OpPhi %bool %false %215 %239 %234
OpStore %ok %240
%241 = OpLoad %v4int %x
%242 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%243 = OpIAdd %v4int %241 %242
OpStore %x %243
%244 = OpLoad %v4int %x
%245 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%246 = OpIMul %v4int %244 %245
OpStore %x %246
%247 = OpLoad %v4int %x
%248 = OpCompositeConstruct %v4int %int_4 %int_4 %int_4 %int_4
%249 = OpISub %v4int %247 %248
OpStore %x %249
%250 = OpLoad %v4int %x
%251 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%252 = OpSDiv %v4int %250 %251
OpStore %x %252
%253 = OpLoad %bool %ok
OpSelectionMerge %255 None
OpBranchConditional %253 %254 %255
%254 = OpLabel
%256 = OpLoad %v4int %x
%257 = OpCompositeConstruct %v4int %int_2 %int_9 %int_18 %int_4
%258 = OpIEqual %v4bool %256 %257
%259 = OpAll %bool %258
OpBranch %255
%255 = OpLabel
%260 = OpPhi %bool %false %235 %259 %254
OpStore %ok %260
%261 = OpLoad %bool %ok
OpReturnValue %261
OpFunctionEnd
%main = OpFunction %v4float None %262
%263 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_inputRed = OpVariable %_ptr_Function_v4float Function
%_2_inputGreen = OpVariable %_ptr_Function_v4float Function
%_3_x = OpVariable %_ptr_Function_v4float Function
%478 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%267 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%268 = OpLoad %v4float %267
OpStore %_1_inputRed %268
%270 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%271 = OpLoad %v4float %270
OpStore %_2_inputGreen %271
%273 = OpLoad %v4float %_1_inputRed
%275 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%276 = OpFAdd %v4float %273 %275
OpStore %_3_x %276
%277 = OpLoad %bool %_0_ok
OpSelectionMerge %279 None
OpBranchConditional %277 %278 %279
%278 = OpLabel
%280 = OpLoad %v4float %_3_x
%282 = OpCompositeConstruct %v4float %float_3 %float_2 %float_2 %float_3
%283 = OpFOrdEqual %v4bool %280 %282
%284 = OpAll %bool %283
OpBranch %279
%279 = OpLabel
%285 = OpPhi %bool %false %263 %284 %278
OpStore %_0_ok %285
%286 = OpLoad %v4float %_2_inputGreen
%287 = OpVectorShuffle %v4float %286 %286 1 3 0 2
%288 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%289 = OpFSub %v4float %287 %288
OpStore %_3_x %289
%290 = OpLoad %bool %_0_ok
OpSelectionMerge %292 None
OpBranchConditional %290 %291 %292
%291 = OpLabel
%293 = OpLoad %v4float %_3_x
%296 = OpCompositeConstruct %v4float %float_n1 %float_n1 %float_n2 %float_n2
%297 = OpFOrdEqual %v4bool %293 %296
%298 = OpAll %bool %297
OpBranch %292
%292 = OpLabel
%299 = OpPhi %bool %false %279 %298 %291
OpStore %_0_ok %299
%300 = OpLoad %v4float %_1_inputRed
%301 = OpLoad %v4float %_2_inputGreen
%302 = OpCompositeExtract %float %301 1
%303 = OpCompositeConstruct %v4float %302 %302 %302 %302
%304 = OpFAdd %v4float %300 %303
OpStore %_3_x %304
%305 = OpLoad %bool %_0_ok
OpSelectionMerge %307 None
OpBranchConditional %305 %306 %307
%306 = OpLabel
%308 = OpLoad %v4float %_3_x
%310 = OpCompositeConstruct %v4float %float_2 %float_1 %float_1 %float_2
%311 = OpFOrdEqual %v4bool %308 %310
%312 = OpAll %bool %311
OpBranch %307
%307 = OpLabel
%313 = OpPhi %bool %false %292 %312 %306
OpStore %_0_ok %313
%314 = OpLoad %v4float %_2_inputGreen
%315 = OpVectorShuffle %v3float %314 %314 3 1 3
%318 = OpVectorTimesScalar %v3float %315 %float_9
%319 = OpLoad %v4float %_3_x
%320 = OpVectorShuffle %v4float %319 %318 4 5 6 3
OpStore %_3_x %320
%321 = OpLoad %bool %_0_ok
OpSelectionMerge %323 None
OpBranchConditional %321 %322 %323
%322 = OpLabel
%324 = OpLoad %v4float %_3_x
%325 = OpCompositeConstruct %v4float %float_9 %float_9 %float_9 %float_2
%326 = OpFOrdEqual %v4bool %324 %325
%327 = OpAll %bool %326
OpBranch %323
%323 = OpLabel
%328 = OpPhi %bool %false %307 %327 %322
OpStore %_0_ok %328
%329 = OpLoad %v4float %_3_x
%330 = OpVectorShuffle %v2float %329 %329 2 3
%332 = OpVectorTimesScalar %v2float %330 %float_2
%333 = OpLoad %v4float %_3_x
%334 = OpVectorShuffle %v4float %333 %332 4 5 2 3
OpStore %_3_x %334
%335 = OpLoad %bool %_0_ok
OpSelectionMerge %337 None
OpBranchConditional %335 %336 %337
%336 = OpLabel
%338 = OpLoad %v4float %_3_x
%341 = OpCompositeConstruct %v4float %float_18 %float_4 %float_9 %float_2
%342 = OpFOrdEqual %v4bool %338 %341
%343 = OpAll %bool %342
OpBranch %337
%337 = OpLabel
%344 = OpPhi %bool %false %323 %343 %336
OpStore %_0_ok %344
%345 = OpLoad %v4float %_1_inputRed
%347 = OpVectorTimesScalar %v4float %345 %float_5
%348 = OpVectorShuffle %v4float %347 %347 1 0 3 2
OpStore %_3_x %348
%349 = OpLoad %bool %_0_ok
OpSelectionMerge %351 None
OpBranchConditional %349 %350 %351
%350 = OpLabel
%352 = OpLoad %v4float %_3_x
%354 = OpCompositeConstruct %v4float %float_0 %float_5 %float_5 %float_0
%355 = OpFOrdEqual %v4bool %352 %354
%356 = OpAll %bool %355
OpBranch %351
%351 = OpLabel
%357 = OpPhi %bool %false %337 %356 %350
OpStore %_0_ok %357
%358 = OpLoad %v4float %_1_inputRed
%359 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%360 = OpFAdd %v4float %359 %358
OpStore %_3_x %360
%361 = OpLoad %bool %_0_ok
OpSelectionMerge %363 None
OpBranchConditional %361 %362 %363
%362 = OpLabel
%364 = OpLoad %v4float %_3_x
%365 = OpCompositeConstruct %v4float %float_3 %float_2 %float_2 %float_3
%366 = OpFOrdEqual %v4bool %364 %365
%367 = OpAll %bool %366
OpBranch %363
%363 = OpLabel
%368 = OpPhi %bool %false %351 %367 %362
OpStore %_0_ok %368
%370 = OpLoad %v4float %_2_inputGreen
%371 = OpVectorShuffle %v4float %370 %370 1 3 0 2
%372 = OpCompositeConstruct %v4float %float_10 %float_10 %float_10 %float_10
%373 = OpFSub %v4float %372 %371
OpStore %_3_x %373
%374 = OpLoad %bool %_0_ok
OpSelectionMerge %376 None
OpBranchConditional %374 %375 %376
%375 = OpLabel
%377 = OpLoad %v4float %_3_x
%378 = OpCompositeConstruct %v4float %float_9 %float_9 %float_10 %float_10
%379 = OpFOrdEqual %v4bool %377 %378
%380 = OpAll %bool %379
OpBranch %376
%376 = OpLabel
%381 = OpPhi %bool %false %363 %380 %375
OpStore %_0_ok %381
%382 = OpLoad %v4float %_1_inputRed
%383 = OpCompositeExtract %float %382 0
%384 = OpLoad %v4float %_2_inputGreen
%385 = OpCompositeConstruct %v4float %383 %383 %383 %383
%386 = OpFAdd %v4float %385 %384
OpStore %_3_x %386
%387 = OpLoad %bool %_0_ok
OpSelectionMerge %389 None
OpBranchConditional %387 %388 %389
%388 = OpLabel
%390 = OpLoad %v4float %_3_x
%391 = OpCompositeConstruct %v4float %float_1 %float_2 %float_1 %float_2
%392 = OpFOrdEqual %v4bool %390 %391
%393 = OpAll %bool %392
OpBranch %389
%389 = OpLabel
%394 = OpPhi %bool %false %376 %393 %388
OpStore %_0_ok %394
%395 = OpLoad %v4float %_2_inputGreen
%396 = OpVectorShuffle %v3float %395 %395 3 1 3
%397 = OpVectorTimesScalar %v3float %396 %float_9
%398 = OpLoad %v4float %_3_x
%399 = OpVectorShuffle %v4float %398 %397 4 5 6 3
OpStore %_3_x %399
%400 = OpLoad %bool %_0_ok
OpSelectionMerge %402 None
OpBranchConditional %400 %401 %402
%401 = OpLabel
%403 = OpLoad %v4float %_3_x
%404 = OpCompositeConstruct %v4float %float_9 %float_9 %float_9 %float_2
%405 = OpFOrdEqual %v4bool %403 %404
%406 = OpAll %bool %405
OpBranch %402
%402 = OpLabel
%407 = OpPhi %bool %false %389 %406 %401
OpStore %_0_ok %407
%409 = OpLoad %v4float %_3_x
%410 = OpVectorShuffle %v2float %409 %409 2 3
%411 = OpCompositeConstruct %v2float %float_36 %float_36
%412 = OpFDiv %v2float %411 %410
%413 = OpLoad %v4float %_3_x
%414 = OpVectorShuffle %v4float %413 %412 4 5 2 3
OpStore %_3_x %414
%415 = OpLoad %bool %_0_ok
OpSelectionMerge %417 None
OpBranchConditional %415 %416 %417
%416 = OpLabel
%418 = OpLoad %v4float %_3_x
%419 = OpCompositeConstruct %v4float %float_4 %float_18 %float_9 %float_2
%420 = OpFOrdEqual %v4bool %418 %419
%421 = OpAll %bool %420
OpBranch %417
%417 = OpLabel
%422 = OpPhi %bool %false %402 %421 %416
OpStore %_0_ok %422
%423 = OpLoad %v4float %_3_x
%424 = OpCompositeConstruct %v4float %float_36 %float_36 %float_36 %float_36
%425 = OpFDiv %v4float %424 %423
%426 = OpVectorShuffle %v4float %425 %425 1 0 3 2
OpStore %_3_x %426
%427 = OpLoad %bool %_0_ok
OpSelectionMerge %429 None
OpBranchConditional %427 %428 %429
%428 = OpLabel
%430 = OpLoad %v4float %_3_x
%431 = OpCompositeConstruct %v4float %float_2 %float_9 %float_18 %float_4
%432 = OpFOrdEqual %v4bool %430 %431
%433 = OpAll %bool %432
OpBranch %429
%429 = OpLabel
%434 = OpPhi %bool %false %417 %433 %428
OpStore %_0_ok %434
%435 = OpLoad %v4float %_3_x
%436 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%437 = OpFAdd %v4float %435 %436
OpStore %_3_x %437
%438 = OpLoad %v4float %_3_x
%439 = OpVectorTimesScalar %v4float %438 %float_2
OpStore %_3_x %439
%440 = OpLoad %v4float %_3_x
%441 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%442 = OpFSub %v4float %440 %441
OpStore %_3_x %442
%443 = OpLoad %v4float %_3_x
%444 = OpFDiv %float %float_1 %float_2
%445 = OpVectorTimesScalar %v4float %443 %444
OpStore %_3_x %445
%446 = OpLoad %bool %_0_ok
OpSelectionMerge %448 None
OpBranchConditional %446 %447 %448
%447 = OpLabel
%449 = OpLoad %v4float %_3_x
%450 = OpCompositeConstruct %v4float %float_2 %float_9 %float_18 %float_4
%451 = OpFOrdEqual %v4bool %449 %450
%452 = OpAll %bool %451
OpBranch %448
%448 = OpLabel
%453 = OpPhi %bool %false %429 %452 %447
OpStore %_0_ok %453
%454 = OpLoad %v4float %_3_x
%455 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%456 = OpFAdd %v4float %454 %455
OpStore %_3_x %456
%457 = OpLoad %v4float %_3_x
%458 = OpVectorTimesScalar %v4float %457 %float_2
OpStore %_3_x %458
%459 = OpLoad %v4float %_3_x
%460 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%461 = OpFSub %v4float %459 %460
OpStore %_3_x %461
%462 = OpLoad %v4float %_3_x
%464 = OpVectorTimesScalar %v4float %462 %float_0_5
OpStore %_3_x %464
%465 = OpLoad %bool %_0_ok
OpSelectionMerge %467 None
OpBranchConditional %465 %466 %467
%466 = OpLabel
%468 = OpLoad %v4float %_3_x
%469 = OpCompositeConstruct %v4float %float_2 %float_9 %float_18 %float_4
%470 = OpFOrdEqual %v4bool %468 %469
%471 = OpAll %bool %470
OpBranch %467
%467 = OpLabel
%472 = OpPhi %bool %false %448 %471 %466
OpStore %_0_ok %472
%473 = OpLoad %bool %_0_ok
OpSelectionMerge %475 None
OpBranchConditional %473 %474 %475
%474 = OpLabel
%476 = OpFunctionCall %bool %test_int_b
OpBranch %475
%475 = OpLabel
%477 = OpPhi %bool %false %467 %476 %474
OpSelectionMerge %481 None
OpBranchConditional %477 %479 %480
%479 = OpLabel
%482 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%483 = OpLoad %v4float %482
OpStore %478 %483
OpBranch %481
%480 = OpLabel
%484 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%485 = OpLoad %v4float %484
OpStore %478 %485
OpBranch %481
%481 = OpLabel
%486 = OpLoad %v4float %478
OpReturnValue %486
OpFunctionEnd
