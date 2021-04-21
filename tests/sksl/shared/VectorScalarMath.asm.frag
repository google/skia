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
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %_1_inputRed RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %_2_inputGreen RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %_3_x RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %350 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %383 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %389 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %394 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %398 RelaxedPrecision
OpDecorate %399 RelaxedPrecision
OpDecorate %402 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %408 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
OpDecorate %411 RelaxedPrecision
OpDecorate %412 RelaxedPrecision
OpDecorate %413 RelaxedPrecision
OpDecorate %416 RelaxedPrecision
OpDecorate %417 RelaxedPrecision
OpDecorate %421 RelaxedPrecision
OpDecorate %422 RelaxedPrecision
OpDecorate %423 RelaxedPrecision
OpDecorate %424 RelaxedPrecision
OpDecorate %425 RelaxedPrecision
OpDecorate %428 RelaxedPrecision
OpDecorate %429 RelaxedPrecision
OpDecorate %433 RelaxedPrecision
OpDecorate %434 RelaxedPrecision
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
OpDecorate %447 RelaxedPrecision
OpDecorate %451 RelaxedPrecision
OpDecorate %452 RelaxedPrecision
OpDecorate %453 RelaxedPrecision
OpDecorate %454 RelaxedPrecision
OpDecorate %455 RelaxedPrecision
OpDecorate %456 RelaxedPrecision
OpDecorate %457 RelaxedPrecision
OpDecorate %458 RelaxedPrecision
OpDecorate %459 RelaxedPrecision
OpDecorate %461 RelaxedPrecision
OpDecorate %462 RelaxedPrecision
OpDecorate %465 RelaxedPrecision
OpDecorate %469 RelaxedPrecision
OpDecorate %479 RelaxedPrecision
OpDecorate %481 RelaxedPrecision
OpDecorate %482 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %bool
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
%70 = OpConstantComposite %v4int %int_3 %int_2 %int_2 %int_3
%v4bool = OpTypeVector %bool 4
%int_n1 = OpConstant %int -1
%int_n2 = OpConstant %int -2
%85 = OpConstantComposite %v4int %int_n1 %int_n1 %int_n2 %int_n2
%98 = OpConstantComposite %v4int %int_2 %int_1 %int_1 %int_2
%v3int = OpTypeVector %int 3
%int_9 = OpConstant %int 9
%114 = OpConstantComposite %v4int %int_9 %int_9 %int_9 %int_2
%v2int = OpTypeVector %int 2
%129 = OpConstantComposite %v4int %int_3 %int_0 %int_9 %int_2
%int_5 = OpConstant %int 5
%142 = OpConstantComposite %v4int %int_0 %int_5 %int_5 %int_0
%int_10 = OpConstant %int 10
%165 = OpConstantComposite %v4int %int_9 %int_9 %int_10 %int_10
%178 = OpConstantComposite %v4int %int_1 %int_2 %int_1 %int_2
%int_36 = OpConstant %int 36
%int_4 = OpConstant %int 4
%int_18 = OpConstant %int 18
%208 = OpConstantComposite %v4int %int_4 %int_18 %int_9 %int_2
%220 = OpConstantComposite %v4int %int_2 %int_9 %int_18 %int_4
%263 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%284 = OpConstantComposite %v4float %float_3 %float_2 %float_2 %float_3
%float_n1 = OpConstant %float -1
%float_n2 = OpConstant %float -2
%298 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n2 %float_n2
%float_1 = OpConstant %float 1
%312 = OpConstantComposite %v4float %float_2 %float_1 %float_1 %float_2
%v3float = OpTypeVector %float 3
%float_9 = OpConstant %float 9
%327 = OpConstantComposite %v4float %float_9 %float_9 %float_9 %float_2
%float_18 = OpConstant %float 18
%float_4 = OpConstant %float 4
%342 = OpConstantComposite %v4float %float_18 %float_4 %float_9 %float_2
%float_5 = OpConstant %float 5
%354 = OpConstantComposite %v4float %float_0 %float_5 %float_5 %float_0
%float_10 = OpConstant %float 10
%377 = OpConstantComposite %v4float %float_9 %float_9 %float_10 %float_10
%390 = OpConstantComposite %v4float %float_1 %float_2 %float_1 %float_2
%float_36 = OpConstant %float 36
%417 = OpConstantComposite %v4float %float_4 %float_18 %float_9 %float_2
%429 = OpConstantComposite %v4float %float_2 %float_9 %float_18 %float_4
%float_0_5 = OpConstant %float 0.5
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%test_int_b = OpFunction %bool None %24
%25 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%inputRed = OpVariable %_ptr_Function_v4int Function
%inputGreen = OpVariable %_ptr_Function_v4int Function
%x = OpVariable %_ptr_Function_v4int Function
OpStore %ok %true
%33 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%36 = OpLoad %v4float %33
%37 = OpCompositeExtract %float %36 0
%38 = OpConvertFToS %int %37
%39 = OpCompositeExtract %float %36 1
%40 = OpConvertFToS %int %39
%41 = OpCompositeExtract %float %36 2
%42 = OpConvertFToS %int %41
%43 = OpCompositeExtract %float %36 3
%44 = OpConvertFToS %int %43
%45 = OpCompositeConstruct %v4int %38 %40 %42 %44
OpStore %inputRed %45
%47 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%49 = OpLoad %v4float %47
%50 = OpCompositeExtract %float %49 0
%51 = OpConvertFToS %int %50
%52 = OpCompositeExtract %float %49 1
%53 = OpConvertFToS %int %52
%54 = OpCompositeExtract %float %49 2
%55 = OpConvertFToS %int %54
%56 = OpCompositeExtract %float %49 3
%57 = OpConvertFToS %int %56
%58 = OpCompositeConstruct %v4int %51 %53 %55 %57
OpStore %inputGreen %58
%60 = OpLoad %v4int %inputRed
%62 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%63 = OpIAdd %v4int %60 %62
OpStore %x %63
%65 = OpLoad %bool %ok
OpSelectionMerge %67 None
OpBranchConditional %65 %66 %67
%66 = OpLabel
%68 = OpLoad %v4int %x
%71 = OpIEqual %v4bool %68 %70
%73 = OpAll %bool %71
OpBranch %67
%67 = OpLabel
%74 = OpPhi %bool %false %25 %73 %66
OpStore %ok %74
%75 = OpLoad %v4int %inputGreen
%76 = OpVectorShuffle %v4int %75 %75 1 3 0 2
%77 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%78 = OpISub %v4int %76 %77
OpStore %x %78
%79 = OpLoad %bool %ok
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%82 = OpLoad %v4int %x
%86 = OpIEqual %v4bool %82 %85
%87 = OpAll %bool %86
OpBranch %81
%81 = OpLabel
%88 = OpPhi %bool %false %67 %87 %80
OpStore %ok %88
%89 = OpLoad %v4int %inputRed
%90 = OpLoad %v4int %inputGreen
%91 = OpCompositeExtract %int %90 1
%92 = OpCompositeConstruct %v4int %91 %91 %91 %91
%93 = OpIAdd %v4int %89 %92
OpStore %x %93
%94 = OpLoad %bool %ok
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%97 = OpLoad %v4int %x
%99 = OpIEqual %v4bool %97 %98
%100 = OpAll %bool %99
OpBranch %96
%96 = OpLabel
%101 = OpPhi %bool %false %81 %100 %95
OpStore %ok %101
%102 = OpLoad %v4int %inputGreen
%103 = OpVectorShuffle %v3int %102 %102 3 1 3
%106 = OpCompositeConstruct %v3int %int_9 %int_9 %int_9
%107 = OpIMul %v3int %103 %106
%108 = OpLoad %v4int %x
%109 = OpVectorShuffle %v4int %108 %107 4 5 6 3
OpStore %x %109
%110 = OpLoad %bool %ok
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%113 = OpLoad %v4int %x
%115 = OpIEqual %v4bool %113 %114
%116 = OpAll %bool %115
OpBranch %112
%112 = OpLabel
%117 = OpPhi %bool %false %96 %116 %111
OpStore %ok %117
%118 = OpLoad %v4int %x
%119 = OpVectorShuffle %v2int %118 %118 2 3
%121 = OpCompositeConstruct %v2int %int_3 %int_3
%122 = OpSDiv %v2int %119 %121
%123 = OpLoad %v4int %x
%124 = OpVectorShuffle %v4int %123 %122 4 5 2 3
OpStore %x %124
%125 = OpLoad %bool %ok
OpSelectionMerge %127 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%128 = OpLoad %v4int %x
%130 = OpIEqual %v4bool %128 %129
%131 = OpAll %bool %130
OpBranch %127
%127 = OpLabel
%132 = OpPhi %bool %false %112 %131 %126
OpStore %ok %132
%133 = OpLoad %v4int %inputRed
%135 = OpCompositeConstruct %v4int %int_5 %int_5 %int_5 %int_5
%136 = OpIMul %v4int %133 %135
%137 = OpVectorShuffle %v4int %136 %136 1 0 3 2
OpStore %x %137
%138 = OpLoad %bool %ok
OpSelectionMerge %140 None
OpBranchConditional %138 %139 %140
%139 = OpLabel
%141 = OpLoad %v4int %x
%143 = OpIEqual %v4bool %141 %142
%144 = OpAll %bool %143
OpBranch %140
%140 = OpLabel
%145 = OpPhi %bool %false %127 %144 %139
OpStore %ok %145
%146 = OpLoad %v4int %inputRed
%147 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%148 = OpIAdd %v4int %147 %146
OpStore %x %148
%149 = OpLoad %bool %ok
OpSelectionMerge %151 None
OpBranchConditional %149 %150 %151
%150 = OpLabel
%152 = OpLoad %v4int %x
%153 = OpIEqual %v4bool %152 %70
%154 = OpAll %bool %153
OpBranch %151
%151 = OpLabel
%155 = OpPhi %bool %false %140 %154 %150
OpStore %ok %155
%157 = OpLoad %v4int %inputGreen
%158 = OpVectorShuffle %v4int %157 %157 1 3 0 2
%159 = OpCompositeConstruct %v4int %int_10 %int_10 %int_10 %int_10
%160 = OpISub %v4int %159 %158
OpStore %x %160
%161 = OpLoad %bool %ok
OpSelectionMerge %163 None
OpBranchConditional %161 %162 %163
%162 = OpLabel
%164 = OpLoad %v4int %x
%166 = OpIEqual %v4bool %164 %165
%167 = OpAll %bool %166
OpBranch %163
%163 = OpLabel
%168 = OpPhi %bool %false %151 %167 %162
OpStore %ok %168
%169 = OpLoad %v4int %inputRed
%170 = OpCompositeExtract %int %169 0
%171 = OpLoad %v4int %inputGreen
%172 = OpCompositeConstruct %v4int %170 %170 %170 %170
%173 = OpIAdd %v4int %172 %171
OpStore %x %173
%174 = OpLoad %bool %ok
OpSelectionMerge %176 None
OpBranchConditional %174 %175 %176
%175 = OpLabel
%177 = OpLoad %v4int %x
%179 = OpIEqual %v4bool %177 %178
%180 = OpAll %bool %179
OpBranch %176
%176 = OpLabel
%181 = OpPhi %bool %false %163 %180 %175
OpStore %ok %181
%182 = OpLoad %v4int %inputGreen
%183 = OpVectorShuffle %v3int %182 %182 3 1 3
%184 = OpCompositeConstruct %v3int %int_9 %int_9 %int_9
%185 = OpIMul %v3int %184 %183
%186 = OpLoad %v4int %x
%187 = OpVectorShuffle %v4int %186 %185 4 5 6 3
OpStore %x %187
%188 = OpLoad %bool %ok
OpSelectionMerge %190 None
OpBranchConditional %188 %189 %190
%189 = OpLabel
%191 = OpLoad %v4int %x
%192 = OpIEqual %v4bool %191 %114
%193 = OpAll %bool %192
OpBranch %190
%190 = OpLabel
%194 = OpPhi %bool %false %176 %193 %189
OpStore %ok %194
%196 = OpLoad %v4int %x
%197 = OpVectorShuffle %v2int %196 %196 2 3
%198 = OpCompositeConstruct %v2int %int_36 %int_36
%199 = OpSDiv %v2int %198 %197
%200 = OpLoad %v4int %x
%201 = OpVectorShuffle %v4int %200 %199 4 5 2 3
OpStore %x %201
%202 = OpLoad %bool %ok
OpSelectionMerge %204 None
OpBranchConditional %202 %203 %204
%203 = OpLabel
%205 = OpLoad %v4int %x
%209 = OpIEqual %v4bool %205 %208
%210 = OpAll %bool %209
OpBranch %204
%204 = OpLabel
%211 = OpPhi %bool %false %190 %210 %203
OpStore %ok %211
%212 = OpLoad %v4int %x
%213 = OpCompositeConstruct %v4int %int_36 %int_36 %int_36 %int_36
%214 = OpSDiv %v4int %213 %212
%215 = OpVectorShuffle %v4int %214 %214 1 0 3 2
OpStore %x %215
%216 = OpLoad %bool %ok
OpSelectionMerge %218 None
OpBranchConditional %216 %217 %218
%217 = OpLabel
%219 = OpLoad %v4int %x
%221 = OpIEqual %v4bool %219 %220
%222 = OpAll %bool %221
OpBranch %218
%218 = OpLabel
%223 = OpPhi %bool %false %204 %222 %217
OpStore %ok %223
%224 = OpLoad %v4int %x
%225 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%226 = OpIAdd %v4int %224 %225
OpStore %x %226
%227 = OpLoad %v4int %x
%228 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%229 = OpIMul %v4int %227 %228
OpStore %x %229
%230 = OpLoad %v4int %x
%231 = OpCompositeConstruct %v4int %int_4 %int_4 %int_4 %int_4
%232 = OpISub %v4int %230 %231
OpStore %x %232
%233 = OpLoad %v4int %x
%234 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%235 = OpSDiv %v4int %233 %234
OpStore %x %235
%236 = OpLoad %bool %ok
OpSelectionMerge %238 None
OpBranchConditional %236 %237 %238
%237 = OpLabel
%239 = OpLoad %v4int %x
%240 = OpIEqual %v4bool %239 %220
%241 = OpAll %bool %240
OpBranch %238
%238 = OpLabel
%242 = OpPhi %bool %false %218 %241 %237
OpStore %ok %242
%243 = OpLoad %v4int %x
%244 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%245 = OpIAdd %v4int %243 %244
OpStore %x %245
%246 = OpLoad %v4int %x
%247 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%248 = OpIMul %v4int %246 %247
OpStore %x %248
%249 = OpLoad %v4int %x
%250 = OpCompositeConstruct %v4int %int_4 %int_4 %int_4 %int_4
%251 = OpISub %v4int %249 %250
OpStore %x %251
%252 = OpLoad %v4int %x
%253 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%254 = OpSDiv %v4int %252 %253
OpStore %x %254
%255 = OpLoad %bool %ok
OpSelectionMerge %257 None
OpBranchConditional %255 %256 %257
%256 = OpLabel
%258 = OpLoad %v4int %x
%259 = OpIEqual %v4bool %258 %220
%260 = OpAll %bool %259
OpBranch %257
%257 = OpLabel
%261 = OpPhi %bool %false %238 %260 %256
OpStore %ok %261
%262 = OpLoad %bool %ok
OpReturnValue %262
OpFunctionEnd
%main = OpFunction %v4float None %263
%264 = OpFunctionParameter %_ptr_Function_v2float
%265 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_inputRed = OpVariable %_ptr_Function_v4float Function
%_2_inputGreen = OpVariable %_ptr_Function_v4float Function
%_3_x = OpVariable %_ptr_Function_v4float Function
%474 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%269 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%270 = OpLoad %v4float %269
OpStore %_1_inputRed %270
%272 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%273 = OpLoad %v4float %272
OpStore %_2_inputGreen %273
%275 = OpLoad %v4float %_1_inputRed
%277 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%278 = OpFAdd %v4float %275 %277
OpStore %_3_x %278
%279 = OpLoad %bool %_0_ok
OpSelectionMerge %281 None
OpBranchConditional %279 %280 %281
%280 = OpLabel
%282 = OpLoad %v4float %_3_x
%285 = OpFOrdEqual %v4bool %282 %284
%286 = OpAll %bool %285
OpBranch %281
%281 = OpLabel
%287 = OpPhi %bool %false %265 %286 %280
OpStore %_0_ok %287
%288 = OpLoad %v4float %_2_inputGreen
%289 = OpVectorShuffle %v4float %288 %288 1 3 0 2
%290 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%291 = OpFSub %v4float %289 %290
OpStore %_3_x %291
%292 = OpLoad %bool %_0_ok
OpSelectionMerge %294 None
OpBranchConditional %292 %293 %294
%293 = OpLabel
%295 = OpLoad %v4float %_3_x
%299 = OpFOrdEqual %v4bool %295 %298
%300 = OpAll %bool %299
OpBranch %294
%294 = OpLabel
%301 = OpPhi %bool %false %281 %300 %293
OpStore %_0_ok %301
%302 = OpLoad %v4float %_1_inputRed
%303 = OpLoad %v4float %_2_inputGreen
%304 = OpCompositeExtract %float %303 1
%305 = OpCompositeConstruct %v4float %304 %304 %304 %304
%306 = OpFAdd %v4float %302 %305
OpStore %_3_x %306
%307 = OpLoad %bool %_0_ok
OpSelectionMerge %309 None
OpBranchConditional %307 %308 %309
%308 = OpLabel
%310 = OpLoad %v4float %_3_x
%313 = OpFOrdEqual %v4bool %310 %312
%314 = OpAll %bool %313
OpBranch %309
%309 = OpLabel
%315 = OpPhi %bool %false %294 %314 %308
OpStore %_0_ok %315
%316 = OpLoad %v4float %_2_inputGreen
%317 = OpVectorShuffle %v3float %316 %316 3 1 3
%320 = OpVectorTimesScalar %v3float %317 %float_9
%321 = OpLoad %v4float %_3_x
%322 = OpVectorShuffle %v4float %321 %320 4 5 6 3
OpStore %_3_x %322
%323 = OpLoad %bool %_0_ok
OpSelectionMerge %325 None
OpBranchConditional %323 %324 %325
%324 = OpLabel
%326 = OpLoad %v4float %_3_x
%328 = OpFOrdEqual %v4bool %326 %327
%329 = OpAll %bool %328
OpBranch %325
%325 = OpLabel
%330 = OpPhi %bool %false %309 %329 %324
OpStore %_0_ok %330
%331 = OpLoad %v4float %_3_x
%332 = OpVectorShuffle %v2float %331 %331 2 3
%333 = OpVectorTimesScalar %v2float %332 %float_2
%334 = OpLoad %v4float %_3_x
%335 = OpVectorShuffle %v4float %334 %333 4 5 2 3
OpStore %_3_x %335
%336 = OpLoad %bool %_0_ok
OpSelectionMerge %338 None
OpBranchConditional %336 %337 %338
%337 = OpLabel
%339 = OpLoad %v4float %_3_x
%343 = OpFOrdEqual %v4bool %339 %342
%344 = OpAll %bool %343
OpBranch %338
%338 = OpLabel
%345 = OpPhi %bool %false %325 %344 %337
OpStore %_0_ok %345
%346 = OpLoad %v4float %_1_inputRed
%348 = OpVectorTimesScalar %v4float %346 %float_5
%349 = OpVectorShuffle %v4float %348 %348 1 0 3 2
OpStore %_3_x %349
%350 = OpLoad %bool %_0_ok
OpSelectionMerge %352 None
OpBranchConditional %350 %351 %352
%351 = OpLabel
%353 = OpLoad %v4float %_3_x
%355 = OpFOrdEqual %v4bool %353 %354
%356 = OpAll %bool %355
OpBranch %352
%352 = OpLabel
%357 = OpPhi %bool %false %338 %356 %351
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
%365 = OpFOrdEqual %v4bool %364 %284
%366 = OpAll %bool %365
OpBranch %363
%363 = OpLabel
%367 = OpPhi %bool %false %352 %366 %362
OpStore %_0_ok %367
%369 = OpLoad %v4float %_2_inputGreen
%370 = OpVectorShuffle %v4float %369 %369 1 3 0 2
%371 = OpCompositeConstruct %v4float %float_10 %float_10 %float_10 %float_10
%372 = OpFSub %v4float %371 %370
OpStore %_3_x %372
%373 = OpLoad %bool %_0_ok
OpSelectionMerge %375 None
OpBranchConditional %373 %374 %375
%374 = OpLabel
%376 = OpLoad %v4float %_3_x
%378 = OpFOrdEqual %v4bool %376 %377
%379 = OpAll %bool %378
OpBranch %375
%375 = OpLabel
%380 = OpPhi %bool %false %363 %379 %374
OpStore %_0_ok %380
%381 = OpLoad %v4float %_1_inputRed
%382 = OpCompositeExtract %float %381 0
%383 = OpLoad %v4float %_2_inputGreen
%384 = OpCompositeConstruct %v4float %382 %382 %382 %382
%385 = OpFAdd %v4float %384 %383
OpStore %_3_x %385
%386 = OpLoad %bool %_0_ok
OpSelectionMerge %388 None
OpBranchConditional %386 %387 %388
%387 = OpLabel
%389 = OpLoad %v4float %_3_x
%391 = OpFOrdEqual %v4bool %389 %390
%392 = OpAll %bool %391
OpBranch %388
%388 = OpLabel
%393 = OpPhi %bool %false %375 %392 %387
OpStore %_0_ok %393
%394 = OpLoad %v4float %_2_inputGreen
%395 = OpVectorShuffle %v3float %394 %394 3 1 3
%396 = OpVectorTimesScalar %v3float %395 %float_9
%397 = OpLoad %v4float %_3_x
%398 = OpVectorShuffle %v4float %397 %396 4 5 6 3
OpStore %_3_x %398
%399 = OpLoad %bool %_0_ok
OpSelectionMerge %401 None
OpBranchConditional %399 %400 %401
%400 = OpLabel
%402 = OpLoad %v4float %_3_x
%403 = OpFOrdEqual %v4bool %402 %327
%404 = OpAll %bool %403
OpBranch %401
%401 = OpLabel
%405 = OpPhi %bool %false %388 %404 %400
OpStore %_0_ok %405
%407 = OpLoad %v4float %_3_x
%408 = OpVectorShuffle %v2float %407 %407 2 3
%409 = OpCompositeConstruct %v2float %float_36 %float_36
%410 = OpFDiv %v2float %409 %408
%411 = OpLoad %v4float %_3_x
%412 = OpVectorShuffle %v4float %411 %410 4 5 2 3
OpStore %_3_x %412
%413 = OpLoad %bool %_0_ok
OpSelectionMerge %415 None
OpBranchConditional %413 %414 %415
%414 = OpLabel
%416 = OpLoad %v4float %_3_x
%418 = OpFOrdEqual %v4bool %416 %417
%419 = OpAll %bool %418
OpBranch %415
%415 = OpLabel
%420 = OpPhi %bool %false %401 %419 %414
OpStore %_0_ok %420
%421 = OpLoad %v4float %_3_x
%422 = OpCompositeConstruct %v4float %float_36 %float_36 %float_36 %float_36
%423 = OpFDiv %v4float %422 %421
%424 = OpVectorShuffle %v4float %423 %423 1 0 3 2
OpStore %_3_x %424
%425 = OpLoad %bool %_0_ok
OpSelectionMerge %427 None
OpBranchConditional %425 %426 %427
%426 = OpLabel
%428 = OpLoad %v4float %_3_x
%430 = OpFOrdEqual %v4bool %428 %429
%431 = OpAll %bool %430
OpBranch %427
%427 = OpLabel
%432 = OpPhi %bool %false %415 %431 %426
OpStore %_0_ok %432
%433 = OpLoad %v4float %_3_x
%434 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%435 = OpFAdd %v4float %433 %434
OpStore %_3_x %435
%436 = OpLoad %v4float %_3_x
%437 = OpVectorTimesScalar %v4float %436 %float_2
OpStore %_3_x %437
%438 = OpLoad %v4float %_3_x
%439 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%440 = OpFSub %v4float %438 %439
OpStore %_3_x %440
%441 = OpLoad %v4float %_3_x
%442 = OpFDiv %float %float_1 %float_2
%443 = OpVectorTimesScalar %v4float %441 %442
OpStore %_3_x %443
%444 = OpLoad %bool %_0_ok
OpSelectionMerge %446 None
OpBranchConditional %444 %445 %446
%445 = OpLabel
%447 = OpLoad %v4float %_3_x
%448 = OpFOrdEqual %v4bool %447 %429
%449 = OpAll %bool %448
OpBranch %446
%446 = OpLabel
%450 = OpPhi %bool %false %427 %449 %445
OpStore %_0_ok %450
%451 = OpLoad %v4float %_3_x
%452 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%453 = OpFAdd %v4float %451 %452
OpStore %_3_x %453
%454 = OpLoad %v4float %_3_x
%455 = OpVectorTimesScalar %v4float %454 %float_2
OpStore %_3_x %455
%456 = OpLoad %v4float %_3_x
%457 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%458 = OpFSub %v4float %456 %457
OpStore %_3_x %458
%459 = OpLoad %v4float %_3_x
%461 = OpVectorTimesScalar %v4float %459 %float_0_5
OpStore %_3_x %461
%462 = OpLoad %bool %_0_ok
OpSelectionMerge %464 None
OpBranchConditional %462 %463 %464
%463 = OpLabel
%465 = OpLoad %v4float %_3_x
%466 = OpFOrdEqual %v4bool %465 %429
%467 = OpAll %bool %466
OpBranch %464
%464 = OpLabel
%468 = OpPhi %bool %false %446 %467 %463
OpStore %_0_ok %468
%469 = OpLoad %bool %_0_ok
OpSelectionMerge %471 None
OpBranchConditional %469 %470 %471
%470 = OpLabel
%472 = OpFunctionCall %bool %test_int_b
OpBranch %471
%471 = OpLabel
%473 = OpPhi %bool %false %464 %472 %470
OpSelectionMerge %477 None
OpBranchConditional %473 %475 %476
%475 = OpLabel
%478 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%479 = OpLoad %v4float %478
OpStore %474 %479
OpBranch %477
%476 = OpLabel
%480 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%481 = OpLoad %v4float %480
OpStore %474 %481
OpBranch %477
%477 = OpLabel
%482 = OpLoad %v4float %474
OpReturnValue %482
OpFunctionEnd
