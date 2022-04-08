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
OpDecorate %78 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %_1_inputRed RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %_2_inputGreen RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %_3_x RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %337 RelaxedPrecision
OpDecorate %338 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
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
OpDecorate %395 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %398 RelaxedPrecision
OpDecorate %399 RelaxedPrecision
OpDecorate %400 RelaxedPrecision
OpDecorate %401 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
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
OpDecorate %426 RelaxedPrecision
OpDecorate %427 RelaxedPrecision
OpDecorate %428 RelaxedPrecision
OpDecorate %429 RelaxedPrecision
OpDecorate %430 RelaxedPrecision
OpDecorate %431 RelaxedPrecision
OpDecorate %434 RelaxedPrecision
OpDecorate %438 RelaxedPrecision
OpDecorate %439 RelaxedPrecision
OpDecorate %440 RelaxedPrecision
OpDecorate %441 RelaxedPrecision
OpDecorate %442 RelaxedPrecision
OpDecorate %443 RelaxedPrecision
OpDecorate %444 RelaxedPrecision
OpDecorate %446 RelaxedPrecision
OpDecorate %447 RelaxedPrecision
OpDecorate %450 RelaxedPrecision
OpDecorate %454 RelaxedPrecision
OpDecorate %464 RelaxedPrecision
OpDecorate %466 RelaxedPrecision
OpDecorate %467 RelaxedPrecision
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
%62 = OpConstantComposite %v4int %int_2 %int_2 %int_2 %int_2
%false = OpConstantFalse %bool
%int_3 = OpConstant %int 3
%70 = OpConstantComposite %v4int %int_3 %int_2 %int_2 %int_3
%v4bool = OpTypeVector %bool 4
%int_n1 = OpConstant %int -1
%int_n2 = OpConstant %int -2
%84 = OpConstantComposite %v4int %int_n1 %int_n1 %int_n2 %int_n2
%97 = OpConstantComposite %v4int %int_2 %int_1 %int_1 %int_2
%v3int = OpTypeVector %int 3
%int_9 = OpConstant %int 9
%105 = OpConstantComposite %v3int %int_9 %int_9 %int_9
%113 = OpConstantComposite %v4int %int_9 %int_9 %int_9 %int_2
%v2int = OpTypeVector %int 2
%120 = OpConstantComposite %v2int %int_3 %int_3
%128 = OpConstantComposite %v4int %int_3 %int_0 %int_9 %int_2
%int_5 = OpConstant %int 5
%134 = OpConstantComposite %v4int %int_5 %int_5 %int_5 %int_5
%141 = OpConstantComposite %v4int %int_0 %int_5 %int_5 %int_0
%int_10 = OpConstant %int 10
%157 = OpConstantComposite %v4int %int_10 %int_10 %int_10 %int_10
%163 = OpConstantComposite %v4int %int_9 %int_9 %int_10 %int_10
%176 = OpConstantComposite %v4int %int_1 %int_2 %int_1 %int_2
%int_36 = OpConstant %int 36
%195 = OpConstantComposite %v2int %int_36 %int_36
%int_4 = OpConstant %int 4
%int_18 = OpConstant %int 18
%205 = OpConstantComposite %v4int %int_4 %int_18 %int_9 %int_2
%210 = OpConstantComposite %v4int %int_36 %int_36 %int_36 %int_36
%217 = OpConstantComposite %v4int %int_2 %int_9 %int_18 %int_4
%226 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%253 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_2 = OpConstant %float 2
%267 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%float_3 = OpConstant %float 3
%274 = OpConstantComposite %v4float %float_3 %float_2 %float_2 %float_3
%float_n1 = OpConstant %float -1
%float_n2 = OpConstant %float -2
%287 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n2 %float_n2
%float_1 = OpConstant %float 1
%301 = OpConstantComposite %v4float %float_2 %float_1 %float_1 %float_2
%v3float = OpTypeVector %float 3
%float_9 = OpConstant %float 9
%316 = OpConstantComposite %v4float %float_9 %float_9 %float_9 %float_2
%float_18 = OpConstant %float 18
%float_4 = OpConstant %float 4
%331 = OpConstantComposite %v4float %float_18 %float_4 %float_9 %float_2
%float_5 = OpConstant %float 5
%343 = OpConstantComposite %v4float %float_0 %float_5 %float_5 %float_0
%float_10 = OpConstant %float 10
%359 = OpConstantComposite %v4float %float_10 %float_10 %float_10 %float_10
%365 = OpConstantComposite %v4float %float_9 %float_9 %float_10 %float_10
%378 = OpConstantComposite %v4float %float_1 %float_2 %float_1 %float_2
%float_36 = OpConstant %float 36
%397 = OpConstantComposite %v2float %float_36 %float_36
%405 = OpConstantComposite %v4float %float_4 %float_18 %float_9 %float_2
%410 = OpConstantComposite %v4float %float_36 %float_36 %float_36 %float_36
%417 = OpConstantComposite %v4float %float_2 %float_9 %float_18 %float_4
%426 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
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
%77 = OpISub %v4int %76 %62
OpStore %x %77
%78 = OpLoad %bool %ok
OpSelectionMerge %80 None
OpBranchConditional %78 %79 %80
%79 = OpLabel
%81 = OpLoad %v4int %x
%85 = OpIEqual %v4bool %81 %84
%86 = OpAll %bool %85
OpBranch %80
%80 = OpLabel
%87 = OpPhi %bool %false %67 %86 %79
OpStore %ok %87
%88 = OpLoad %v4int %inputRed
%89 = OpLoad %v4int %inputGreen
%90 = OpCompositeExtract %int %89 1
%91 = OpCompositeConstruct %v4int %90 %90 %90 %90
%92 = OpIAdd %v4int %88 %91
OpStore %x %92
%93 = OpLoad %bool %ok
OpSelectionMerge %95 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%96 = OpLoad %v4int %x
%98 = OpIEqual %v4bool %96 %97
%99 = OpAll %bool %98
OpBranch %95
%95 = OpLabel
%100 = OpPhi %bool %false %80 %99 %94
OpStore %ok %100
%101 = OpLoad %v4int %inputGreen
%102 = OpVectorShuffle %v3int %101 %101 3 1 3
%106 = OpIMul %v3int %102 %105
%107 = OpLoad %v4int %x
%108 = OpVectorShuffle %v4int %107 %106 4 5 6 3
OpStore %x %108
%109 = OpLoad %bool %ok
OpSelectionMerge %111 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
%112 = OpLoad %v4int %x
%114 = OpIEqual %v4bool %112 %113
%115 = OpAll %bool %114
OpBranch %111
%111 = OpLabel
%116 = OpPhi %bool %false %95 %115 %110
OpStore %ok %116
%117 = OpLoad %v4int %x
%118 = OpVectorShuffle %v2int %117 %117 2 3
%121 = OpSDiv %v2int %118 %120
%122 = OpLoad %v4int %x
%123 = OpVectorShuffle %v4int %122 %121 4 5 2 3
OpStore %x %123
%124 = OpLoad %bool %ok
OpSelectionMerge %126 None
OpBranchConditional %124 %125 %126
%125 = OpLabel
%127 = OpLoad %v4int %x
%129 = OpIEqual %v4bool %127 %128
%130 = OpAll %bool %129
OpBranch %126
%126 = OpLabel
%131 = OpPhi %bool %false %111 %130 %125
OpStore %ok %131
%132 = OpLoad %v4int %inputRed
%135 = OpIMul %v4int %132 %134
%136 = OpVectorShuffle %v4int %135 %135 1 0 3 2
OpStore %x %136
%137 = OpLoad %bool %ok
OpSelectionMerge %139 None
OpBranchConditional %137 %138 %139
%138 = OpLabel
%140 = OpLoad %v4int %x
%142 = OpIEqual %v4bool %140 %141
%143 = OpAll %bool %142
OpBranch %139
%139 = OpLabel
%144 = OpPhi %bool %false %126 %143 %138
OpStore %ok %144
%145 = OpLoad %v4int %inputRed
%146 = OpIAdd %v4int %62 %145
OpStore %x %146
%147 = OpLoad %bool %ok
OpSelectionMerge %149 None
OpBranchConditional %147 %148 %149
%148 = OpLabel
%150 = OpLoad %v4int %x
%151 = OpIEqual %v4bool %150 %70
%152 = OpAll %bool %151
OpBranch %149
%149 = OpLabel
%153 = OpPhi %bool %false %139 %152 %148
OpStore %ok %153
%155 = OpLoad %v4int %inputGreen
%156 = OpVectorShuffle %v4int %155 %155 1 3 0 2
%158 = OpISub %v4int %157 %156
OpStore %x %158
%159 = OpLoad %bool %ok
OpSelectionMerge %161 None
OpBranchConditional %159 %160 %161
%160 = OpLabel
%162 = OpLoad %v4int %x
%164 = OpIEqual %v4bool %162 %163
%165 = OpAll %bool %164
OpBranch %161
%161 = OpLabel
%166 = OpPhi %bool %false %149 %165 %160
OpStore %ok %166
%167 = OpLoad %v4int %inputRed
%168 = OpCompositeExtract %int %167 0
%169 = OpLoad %v4int %inputGreen
%170 = OpCompositeConstruct %v4int %168 %168 %168 %168
%171 = OpIAdd %v4int %170 %169
OpStore %x %171
%172 = OpLoad %bool %ok
OpSelectionMerge %174 None
OpBranchConditional %172 %173 %174
%173 = OpLabel
%175 = OpLoad %v4int %x
%177 = OpIEqual %v4bool %175 %176
%178 = OpAll %bool %177
OpBranch %174
%174 = OpLabel
%179 = OpPhi %bool %false %161 %178 %173
OpStore %ok %179
%180 = OpLoad %v4int %inputGreen
%181 = OpVectorShuffle %v3int %180 %180 3 1 3
%182 = OpIMul %v3int %105 %181
%183 = OpLoad %v4int %x
%184 = OpVectorShuffle %v4int %183 %182 4 5 6 3
OpStore %x %184
%185 = OpLoad %bool %ok
OpSelectionMerge %187 None
OpBranchConditional %185 %186 %187
%186 = OpLabel
%188 = OpLoad %v4int %x
%189 = OpIEqual %v4bool %188 %113
%190 = OpAll %bool %189
OpBranch %187
%187 = OpLabel
%191 = OpPhi %bool %false %174 %190 %186
OpStore %ok %191
%193 = OpLoad %v4int %x
%194 = OpVectorShuffle %v2int %193 %193 2 3
%196 = OpSDiv %v2int %195 %194
%197 = OpLoad %v4int %x
%198 = OpVectorShuffle %v4int %197 %196 4 5 2 3
OpStore %x %198
%199 = OpLoad %bool %ok
OpSelectionMerge %201 None
OpBranchConditional %199 %200 %201
%200 = OpLabel
%202 = OpLoad %v4int %x
%206 = OpIEqual %v4bool %202 %205
%207 = OpAll %bool %206
OpBranch %201
%201 = OpLabel
%208 = OpPhi %bool %false %187 %207 %200
OpStore %ok %208
%209 = OpLoad %v4int %x
%211 = OpSDiv %v4int %210 %209
%212 = OpVectorShuffle %v4int %211 %211 1 0 3 2
OpStore %x %212
%213 = OpLoad %bool %ok
OpSelectionMerge %215 None
OpBranchConditional %213 %214 %215
%214 = OpLabel
%216 = OpLoad %v4int %x
%218 = OpIEqual %v4bool %216 %217
%219 = OpAll %bool %218
OpBranch %215
%215 = OpLabel
%220 = OpPhi %bool %false %201 %219 %214
OpStore %ok %220
%221 = OpLoad %v4int %x
%222 = OpIAdd %v4int %221 %62
OpStore %x %222
%223 = OpLoad %v4int %x
%224 = OpIMul %v4int %223 %62
OpStore %x %224
%225 = OpLoad %v4int %x
%227 = OpISub %v4int %225 %226
OpStore %x %227
%228 = OpLoad %v4int %x
%229 = OpSDiv %v4int %228 %62
OpStore %x %229
%230 = OpLoad %bool %ok
OpSelectionMerge %232 None
OpBranchConditional %230 %231 %232
%231 = OpLabel
%233 = OpLoad %v4int %x
%234 = OpIEqual %v4bool %233 %217
%235 = OpAll %bool %234
OpBranch %232
%232 = OpLabel
%236 = OpPhi %bool %false %215 %235 %231
OpStore %ok %236
%237 = OpLoad %v4int %x
%238 = OpIAdd %v4int %237 %62
OpStore %x %238
%239 = OpLoad %v4int %x
%240 = OpIMul %v4int %239 %62
OpStore %x %240
%241 = OpLoad %v4int %x
%242 = OpISub %v4int %241 %226
OpStore %x %242
%243 = OpLoad %v4int %x
%244 = OpSDiv %v4int %243 %62
OpStore %x %244
%245 = OpLoad %bool %ok
OpSelectionMerge %247 None
OpBranchConditional %245 %246 %247
%246 = OpLabel
%248 = OpLoad %v4int %x
%249 = OpIEqual %v4bool %248 %217
%250 = OpAll %bool %249
OpBranch %247
%247 = OpLabel
%251 = OpPhi %bool %false %232 %250 %246
OpStore %ok %251
%252 = OpLoad %bool %ok
OpReturnValue %252
OpFunctionEnd
%main = OpFunction %v4float None %253
%254 = OpFunctionParameter %_ptr_Function_v2float
%255 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_inputRed = OpVariable %_ptr_Function_v4float Function
%_2_inputGreen = OpVariable %_ptr_Function_v4float Function
%_3_x = OpVariable %_ptr_Function_v4float Function
%459 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%259 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%260 = OpLoad %v4float %259
OpStore %_1_inputRed %260
%262 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%263 = OpLoad %v4float %262
OpStore %_2_inputGreen %263
%265 = OpLoad %v4float %_1_inputRed
%268 = OpFAdd %v4float %265 %267
OpStore %_3_x %268
%269 = OpLoad %bool %_0_ok
OpSelectionMerge %271 None
OpBranchConditional %269 %270 %271
%270 = OpLabel
%272 = OpLoad %v4float %_3_x
%275 = OpFOrdEqual %v4bool %272 %274
%276 = OpAll %bool %275
OpBranch %271
%271 = OpLabel
%277 = OpPhi %bool %false %255 %276 %270
OpStore %_0_ok %277
%278 = OpLoad %v4float %_2_inputGreen
%279 = OpVectorShuffle %v4float %278 %278 1 3 0 2
%280 = OpFSub %v4float %279 %267
OpStore %_3_x %280
%281 = OpLoad %bool %_0_ok
OpSelectionMerge %283 None
OpBranchConditional %281 %282 %283
%282 = OpLabel
%284 = OpLoad %v4float %_3_x
%288 = OpFOrdEqual %v4bool %284 %287
%289 = OpAll %bool %288
OpBranch %283
%283 = OpLabel
%290 = OpPhi %bool %false %271 %289 %282
OpStore %_0_ok %290
%291 = OpLoad %v4float %_1_inputRed
%292 = OpLoad %v4float %_2_inputGreen
%293 = OpCompositeExtract %float %292 1
%294 = OpCompositeConstruct %v4float %293 %293 %293 %293
%295 = OpFAdd %v4float %291 %294
OpStore %_3_x %295
%296 = OpLoad %bool %_0_ok
OpSelectionMerge %298 None
OpBranchConditional %296 %297 %298
%297 = OpLabel
%299 = OpLoad %v4float %_3_x
%302 = OpFOrdEqual %v4bool %299 %301
%303 = OpAll %bool %302
OpBranch %298
%298 = OpLabel
%304 = OpPhi %bool %false %283 %303 %297
OpStore %_0_ok %304
%305 = OpLoad %v4float %_2_inputGreen
%306 = OpVectorShuffle %v3float %305 %305 3 1 3
%309 = OpVectorTimesScalar %v3float %306 %float_9
%310 = OpLoad %v4float %_3_x
%311 = OpVectorShuffle %v4float %310 %309 4 5 6 3
OpStore %_3_x %311
%312 = OpLoad %bool %_0_ok
OpSelectionMerge %314 None
OpBranchConditional %312 %313 %314
%313 = OpLabel
%315 = OpLoad %v4float %_3_x
%317 = OpFOrdEqual %v4bool %315 %316
%318 = OpAll %bool %317
OpBranch %314
%314 = OpLabel
%319 = OpPhi %bool %false %298 %318 %313
OpStore %_0_ok %319
%320 = OpLoad %v4float %_3_x
%321 = OpVectorShuffle %v2float %320 %320 2 3
%322 = OpVectorTimesScalar %v2float %321 %float_2
%323 = OpLoad %v4float %_3_x
%324 = OpVectorShuffle %v4float %323 %322 4 5 2 3
OpStore %_3_x %324
%325 = OpLoad %bool %_0_ok
OpSelectionMerge %327 None
OpBranchConditional %325 %326 %327
%326 = OpLabel
%328 = OpLoad %v4float %_3_x
%332 = OpFOrdEqual %v4bool %328 %331
%333 = OpAll %bool %332
OpBranch %327
%327 = OpLabel
%334 = OpPhi %bool %false %314 %333 %326
OpStore %_0_ok %334
%335 = OpLoad %v4float %_1_inputRed
%337 = OpVectorTimesScalar %v4float %335 %float_5
%338 = OpVectorShuffle %v4float %337 %337 1 0 3 2
OpStore %_3_x %338
%339 = OpLoad %bool %_0_ok
OpSelectionMerge %341 None
OpBranchConditional %339 %340 %341
%340 = OpLabel
%342 = OpLoad %v4float %_3_x
%344 = OpFOrdEqual %v4bool %342 %343
%345 = OpAll %bool %344
OpBranch %341
%341 = OpLabel
%346 = OpPhi %bool %false %327 %345 %340
OpStore %_0_ok %346
%347 = OpLoad %v4float %_1_inputRed
%348 = OpFAdd %v4float %267 %347
OpStore %_3_x %348
%349 = OpLoad %bool %_0_ok
OpSelectionMerge %351 None
OpBranchConditional %349 %350 %351
%350 = OpLabel
%352 = OpLoad %v4float %_3_x
%353 = OpFOrdEqual %v4bool %352 %274
%354 = OpAll %bool %353
OpBranch %351
%351 = OpLabel
%355 = OpPhi %bool %false %341 %354 %350
OpStore %_0_ok %355
%357 = OpLoad %v4float %_2_inputGreen
%358 = OpVectorShuffle %v4float %357 %357 1 3 0 2
%360 = OpFSub %v4float %359 %358
OpStore %_3_x %360
%361 = OpLoad %bool %_0_ok
OpSelectionMerge %363 None
OpBranchConditional %361 %362 %363
%362 = OpLabel
%364 = OpLoad %v4float %_3_x
%366 = OpFOrdEqual %v4bool %364 %365
%367 = OpAll %bool %366
OpBranch %363
%363 = OpLabel
%368 = OpPhi %bool %false %351 %367 %362
OpStore %_0_ok %368
%369 = OpLoad %v4float %_1_inputRed
%370 = OpCompositeExtract %float %369 0
%371 = OpLoad %v4float %_2_inputGreen
%372 = OpCompositeConstruct %v4float %370 %370 %370 %370
%373 = OpFAdd %v4float %372 %371
OpStore %_3_x %373
%374 = OpLoad %bool %_0_ok
OpSelectionMerge %376 None
OpBranchConditional %374 %375 %376
%375 = OpLabel
%377 = OpLoad %v4float %_3_x
%379 = OpFOrdEqual %v4bool %377 %378
%380 = OpAll %bool %379
OpBranch %376
%376 = OpLabel
%381 = OpPhi %bool %false %363 %380 %375
OpStore %_0_ok %381
%382 = OpLoad %v4float %_2_inputGreen
%383 = OpVectorShuffle %v3float %382 %382 3 1 3
%384 = OpVectorTimesScalar %v3float %383 %float_9
%385 = OpLoad %v4float %_3_x
%386 = OpVectorShuffle %v4float %385 %384 4 5 6 3
OpStore %_3_x %386
%387 = OpLoad %bool %_0_ok
OpSelectionMerge %389 None
OpBranchConditional %387 %388 %389
%388 = OpLabel
%390 = OpLoad %v4float %_3_x
%391 = OpFOrdEqual %v4bool %390 %316
%392 = OpAll %bool %391
OpBranch %389
%389 = OpLabel
%393 = OpPhi %bool %false %376 %392 %388
OpStore %_0_ok %393
%395 = OpLoad %v4float %_3_x
%396 = OpVectorShuffle %v2float %395 %395 2 3
%398 = OpFDiv %v2float %397 %396
%399 = OpLoad %v4float %_3_x
%400 = OpVectorShuffle %v4float %399 %398 4 5 2 3
OpStore %_3_x %400
%401 = OpLoad %bool %_0_ok
OpSelectionMerge %403 None
OpBranchConditional %401 %402 %403
%402 = OpLabel
%404 = OpLoad %v4float %_3_x
%406 = OpFOrdEqual %v4bool %404 %405
%407 = OpAll %bool %406
OpBranch %403
%403 = OpLabel
%408 = OpPhi %bool %false %389 %407 %402
OpStore %_0_ok %408
%409 = OpLoad %v4float %_3_x
%411 = OpFDiv %v4float %410 %409
%412 = OpVectorShuffle %v4float %411 %411 1 0 3 2
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
%420 = OpPhi %bool %false %403 %419 %414
OpStore %_0_ok %420
%421 = OpLoad %v4float %_3_x
%422 = OpFAdd %v4float %421 %267
OpStore %_3_x %422
%423 = OpLoad %v4float %_3_x
%424 = OpVectorTimesScalar %v4float %423 %float_2
OpStore %_3_x %424
%425 = OpLoad %v4float %_3_x
%427 = OpFSub %v4float %425 %426
OpStore %_3_x %427
%428 = OpLoad %v4float %_3_x
%429 = OpFDiv %float %float_1 %float_2
%430 = OpVectorTimesScalar %v4float %428 %429
OpStore %_3_x %430
%431 = OpLoad %bool %_0_ok
OpSelectionMerge %433 None
OpBranchConditional %431 %432 %433
%432 = OpLabel
%434 = OpLoad %v4float %_3_x
%435 = OpFOrdEqual %v4bool %434 %417
%436 = OpAll %bool %435
OpBranch %433
%433 = OpLabel
%437 = OpPhi %bool %false %415 %436 %432
OpStore %_0_ok %437
%438 = OpLoad %v4float %_3_x
%439 = OpFAdd %v4float %438 %267
OpStore %_3_x %439
%440 = OpLoad %v4float %_3_x
%441 = OpVectorTimesScalar %v4float %440 %float_2
OpStore %_3_x %441
%442 = OpLoad %v4float %_3_x
%443 = OpFSub %v4float %442 %426
OpStore %_3_x %443
%444 = OpLoad %v4float %_3_x
%446 = OpVectorTimesScalar %v4float %444 %float_0_5
OpStore %_3_x %446
%447 = OpLoad %bool %_0_ok
OpSelectionMerge %449 None
OpBranchConditional %447 %448 %449
%448 = OpLabel
%450 = OpLoad %v4float %_3_x
%451 = OpFOrdEqual %v4bool %450 %417
%452 = OpAll %bool %451
OpBranch %449
%449 = OpLabel
%453 = OpPhi %bool %false %433 %452 %448
OpStore %_0_ok %453
%454 = OpLoad %bool %_0_ok
OpSelectionMerge %456 None
OpBranchConditional %454 %455 %456
%455 = OpLabel
%457 = OpFunctionCall %bool %test_int_b
OpBranch %456
%456 = OpLabel
%458 = OpPhi %bool %false %449 %457 %455
OpSelectionMerge %462 None
OpBranchConditional %458 %460 %461
%460 = OpLabel
%463 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%464 = OpLoad %v4float %463
OpStore %459 %464
OpBranch %462
%461 = OpLabel
%465 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%466 = OpLoad %v4float %465
OpStore %459 %466
OpBranch %462
%462 = OpLabel
%467 = OpLoad %v4float %459
OpReturnValue %467
OpFunctionEnd
