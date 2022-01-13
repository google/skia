OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %minus "minus"
OpName %star "star"
OpName %slash "slash"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_bifffff22 "test_bifffff22"
OpName %one "one"
OpName %m2 "m2"
OpName %main "main"
OpName %f1 "f1"
OpName %f2 "f2"
OpName %f3 "f3"
OpName %f4 "f4"
OpName %_0_expected "_0_expected"
OpName %_1_one "_1_one"
OpName %_2_m2 "_2_m2"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %19 Binding 0
OpDecorate %19 DescriptorSet 0
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%int = OpTypeInt 32 1
%_ptr_Private_int = OpTypePointer Private %int
%minus = OpVariable %_ptr_Private_int Private
%int_2 = OpConstant %int 2
%star = OpVariable %_ptr_Private_int Private
%int_3 = OpConstant %int 3
%slash = OpVariable %_ptr_Private_int Private
%int_4 = OpConstant %int 4
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%19 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%24 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%28 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%33 = OpTypeFunction %bool %_ptr_Function_int %_ptr_Function_float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_mat2v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%false = OpConstantFalse %bool
%int_0 = OpConstant %int 0
%145 = OpTypeFunction %v4float %_ptr_Function_v2float
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_0_5 = OpConstant %float 0.5
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %24
%25 = OpLabel
%29 = OpVariable %_ptr_Function_v2float Function
OpStore %29 %28
%31 = OpFunctionCall %v4float %main %29
OpStore %sk_FragColor %31
OpReturn
OpFunctionEnd
%test_bifffff22 = OpFunction %bool None %33
%37 = OpFunctionParameter %_ptr_Function_int
%38 = OpFunctionParameter %_ptr_Function_float
%39 = OpFunctionParameter %_ptr_Function_float
%40 = OpFunctionParameter %_ptr_Function_float
%41 = OpFunctionParameter %_ptr_Function_float
%42 = OpFunctionParameter %_ptr_Function_mat2v2float
%43 = OpLabel
%one = OpVariable %_ptr_Function_float Function
%m2 = OpVariable %_ptr_Function_mat2v2float Function
%45 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%48 = OpLoad %v4float %45
%49 = OpCompositeExtract %float %48 0
OpStore %one %49
%51 = OpLoad %float %38
%52 = OpLoad %float %one
%53 = OpFMul %float %51 %52
%54 = OpLoad %float %39
%55 = OpLoad %float %one
%56 = OpFMul %float %54 %55
%57 = OpLoad %float %40
%58 = OpLoad %float %one
%59 = OpFMul %float %57 %58
%60 = OpLoad %float %41
%61 = OpLoad %float %one
%62 = OpFMul %float %60 %61
%63 = OpCompositeConstruct %v2float %53 %56
%64 = OpCompositeConstruct %v2float %59 %62
%65 = OpCompositeConstruct %mat2v2float %63 %64
OpStore %m2 %65
%66 = OpLoad %int %37
OpSelectionMerge %67 None
OpSwitch %66 %67 1 %68 2 %69 3 %70 4 %71
%68 = OpLabel
%72 = OpLoad %mat2v2float %m2
%74 = OpCompositeConstruct %v2float %float_1 %float_1
%75 = OpCompositeConstruct %mat2v2float %74 %74
%76 = OpCompositeExtract %v2float %72 0
%77 = OpCompositeExtract %v2float %75 0
%78 = OpFAdd %v2float %76 %77
%79 = OpCompositeExtract %v2float %72 1
%80 = OpCompositeExtract %v2float %75 1
%81 = OpFAdd %v2float %79 %80
%82 = OpCompositeConstruct %mat2v2float %78 %81
OpStore %m2 %82
OpBranch %67
%69 = OpLabel
%83 = OpLoad %mat2v2float %m2
%84 = OpCompositeConstruct %v2float %float_1 %float_1
%85 = OpCompositeConstruct %mat2v2float %84 %84
%86 = OpCompositeExtract %v2float %83 0
%87 = OpCompositeExtract %v2float %85 0
%88 = OpFSub %v2float %86 %87
%89 = OpCompositeExtract %v2float %83 1
%90 = OpCompositeExtract %v2float %85 1
%91 = OpFSub %v2float %89 %90
%92 = OpCompositeConstruct %mat2v2float %88 %91
OpStore %m2 %92
OpBranch %67
%70 = OpLabel
%93 = OpLoad %mat2v2float %m2
%95 = OpMatrixTimesScalar %mat2v2float %93 %float_2
OpStore %m2 %95
OpBranch %67
%71 = OpLabel
%96 = OpLoad %mat2v2float %m2
%97 = OpCompositeConstruct %v2float %float_2 %float_2
%98 = OpCompositeConstruct %mat2v2float %97 %97
%99 = OpCompositeExtract %v2float %96 0
%100 = OpCompositeExtract %v2float %98 0
%101 = OpFDiv %v2float %99 %100
%102 = OpCompositeExtract %v2float %96 1
%103 = OpCompositeExtract %v2float %98 1
%104 = OpFDiv %v2float %102 %103
%105 = OpCompositeConstruct %mat2v2float %101 %104
OpStore %m2 %105
OpBranch %67
%67 = OpLabel
%108 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
%109 = OpLoad %v2float %108
%110 = OpCompositeExtract %float %109 0
%111 = OpAccessChain %_ptr_Function_v2float %42 %int_0
%112 = OpLoad %v2float %111
%113 = OpCompositeExtract %float %112 0
%114 = OpFOrdEqual %bool %110 %113
OpSelectionMerge %116 None
OpBranchConditional %114 %115 %116
%115 = OpLabel
%117 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
%118 = OpLoad %v2float %117
%119 = OpCompositeExtract %float %118 1
%120 = OpAccessChain %_ptr_Function_v2float %42 %int_0
%121 = OpLoad %v2float %120
%122 = OpCompositeExtract %float %121 1
%123 = OpFOrdEqual %bool %119 %122
OpBranch %116
%116 = OpLabel
%124 = OpPhi %bool %false %67 %123 %115
OpSelectionMerge %126 None
OpBranchConditional %124 %125 %126
%125 = OpLabel
%127 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
%128 = OpLoad %v2float %127
%129 = OpCompositeExtract %float %128 0
%130 = OpAccessChain %_ptr_Function_v2float %42 %int_1
%131 = OpLoad %v2float %130
%132 = OpCompositeExtract %float %131 0
%133 = OpFOrdEqual %bool %129 %132
OpBranch %126
%126 = OpLabel
%134 = OpPhi %bool %false %116 %133 %125
OpSelectionMerge %136 None
OpBranchConditional %134 %135 %136
%135 = OpLabel
%137 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
%138 = OpLoad %v2float %137
%139 = OpCompositeExtract %float %138 1
%140 = OpAccessChain %_ptr_Function_v2float %42 %int_1
%141 = OpLoad %v2float %140
%142 = OpCompositeExtract %float %141 1
%143 = OpFOrdEqual %bool %139 %142
OpBranch %136
%136 = OpLabel
%144 = OpPhi %bool %false %126 %143 %135
OpReturnValue %144
OpFunctionEnd
%main = OpFunction %v4float None %145
%146 = OpFunctionParameter %_ptr_Function_v2float
%147 = OpLabel
%f1 = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_float Function
%f3 = OpVariable %_ptr_Function_float Function
%f4 = OpVariable %_ptr_Function_float Function
%_0_expected = OpVariable %_ptr_Function_mat2v2float Function
%_1_one = OpVariable %_ptr_Function_float Function
%_2_m2 = OpVariable %_ptr_Function_mat2v2float Function
%251 = OpVariable %_ptr_Function_int Function
%253 = OpVariable %_ptr_Function_float Function
%255 = OpVariable %_ptr_Function_float Function
%257 = OpVariable %_ptr_Function_float Function
%259 = OpVariable %_ptr_Function_float Function
%271 = OpVariable %_ptr_Function_mat2v2float Function
%277 = OpVariable %_ptr_Function_int Function
%279 = OpVariable %_ptr_Function_float Function
%281 = OpVariable %_ptr_Function_float Function
%283 = OpVariable %_ptr_Function_float Function
%285 = OpVariable %_ptr_Function_float Function
%297 = OpVariable %_ptr_Function_mat2v2float Function
%303 = OpVariable %_ptr_Function_int Function
%305 = OpVariable %_ptr_Function_float Function
%307 = OpVariable %_ptr_Function_float Function
%309 = OpVariable %_ptr_Function_float Function
%311 = OpVariable %_ptr_Function_float Function
%324 = OpVariable %_ptr_Function_mat2v2float Function
%327 = OpVariable %_ptr_Function_v4float Function
OpStore %minus %int_2
OpStore %star %int_3
OpStore %slash %int_4
%149 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%150 = OpLoad %v4float %149
%151 = OpCompositeExtract %float %150 1
OpStore %f1 %151
%153 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%154 = OpLoad %v4float %153
%155 = OpCompositeExtract %float %154 1
%156 = OpFMul %float %float_2 %155
OpStore %f2 %156
%159 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%160 = OpLoad %v4float %159
%161 = OpCompositeExtract %float %160 1
%162 = OpFMul %float %float_3 %161
OpStore %f3 %162
%165 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%166 = OpLoad %v4float %165
%167 = OpCompositeExtract %float %166 1
%168 = OpFMul %float %float_4 %167
OpStore %f4 %168
%170 = OpLoad %float %f1
%171 = OpFAdd %float %170 %float_1
%172 = OpLoad %float %f2
%173 = OpFAdd %float %172 %float_1
%174 = OpLoad %float %f3
%175 = OpFAdd %float %174 %float_1
%176 = OpLoad %float %f4
%177 = OpFAdd %float %176 %float_1
%178 = OpCompositeConstruct %v2float %171 %173
%179 = OpCompositeConstruct %v2float %175 %177
%180 = OpCompositeConstruct %mat2v2float %178 %179
OpStore %_0_expected %180
%182 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%183 = OpLoad %v4float %182
%184 = OpCompositeExtract %float %183 0
OpStore %_1_one %184
%186 = OpLoad %float %f1
%187 = OpLoad %float %_1_one
%188 = OpFMul %float %186 %187
%189 = OpLoad %float %f2
%190 = OpLoad %float %_1_one
%191 = OpFMul %float %189 %190
%192 = OpLoad %float %f3
%193 = OpLoad %float %_1_one
%194 = OpFMul %float %192 %193
%195 = OpLoad %float %f4
%196 = OpLoad %float %_1_one
%197 = OpFMul %float %195 %196
%198 = OpCompositeConstruct %v2float %188 %191
%199 = OpCompositeConstruct %v2float %194 %197
%200 = OpCompositeConstruct %mat2v2float %198 %199
OpStore %_2_m2 %200
%201 = OpLoad %mat2v2float %_2_m2
%202 = OpCompositeConstruct %v2float %float_1 %float_1
%203 = OpCompositeConstruct %mat2v2float %202 %202
%204 = OpCompositeExtract %v2float %201 0
%205 = OpCompositeExtract %v2float %203 0
%206 = OpFAdd %v2float %204 %205
%207 = OpCompositeExtract %v2float %201 1
%208 = OpCompositeExtract %v2float %203 1
%209 = OpFAdd %v2float %207 %208
%210 = OpCompositeConstruct %mat2v2float %206 %209
OpStore %_2_m2 %210
%211 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
%212 = OpLoad %v2float %211
%213 = OpCompositeExtract %float %212 0
%214 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
%215 = OpLoad %v2float %214
%216 = OpCompositeExtract %float %215 0
%217 = OpFOrdEqual %bool %213 %216
OpSelectionMerge %219 None
OpBranchConditional %217 %218 %219
%218 = OpLabel
%220 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
%221 = OpLoad %v2float %220
%222 = OpCompositeExtract %float %221 1
%223 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
%224 = OpLoad %v2float %223
%225 = OpCompositeExtract %float %224 1
%226 = OpFOrdEqual %bool %222 %225
OpBranch %219
%219 = OpLabel
%227 = OpPhi %bool %false %147 %226 %218
OpSelectionMerge %229 None
OpBranchConditional %227 %228 %229
%228 = OpLabel
%230 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
%231 = OpLoad %v2float %230
%232 = OpCompositeExtract %float %231 0
%233 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
%234 = OpLoad %v2float %233
%235 = OpCompositeExtract %float %234 0
%236 = OpFOrdEqual %bool %232 %235
OpBranch %229
%229 = OpLabel
%237 = OpPhi %bool %false %219 %236 %228
OpSelectionMerge %239 None
OpBranchConditional %237 %238 %239
%238 = OpLabel
%240 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
%241 = OpLoad %v2float %240
%242 = OpCompositeExtract %float %241 1
%243 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
%244 = OpLoad %v2float %243
%245 = OpCompositeExtract %float %244 1
%246 = OpFOrdEqual %bool %242 %245
OpBranch %239
%239 = OpLabel
%247 = OpPhi %bool %false %229 %246 %238
OpSelectionMerge %249 None
OpBranchConditional %247 %248 %249
%248 = OpLabel
%250 = OpLoad %int %minus
OpStore %251 %250
%252 = OpLoad %float %f1
OpStore %253 %252
%254 = OpLoad %float %f2
OpStore %255 %254
%256 = OpLoad %float %f3
OpStore %257 %256
%258 = OpLoad %float %f4
OpStore %259 %258
%260 = OpLoad %float %f1
%261 = OpFSub %float %260 %float_1
%262 = OpLoad %float %f2
%263 = OpFSub %float %262 %float_1
%264 = OpLoad %float %f3
%265 = OpFSub %float %264 %float_1
%266 = OpLoad %float %f4
%267 = OpFSub %float %266 %float_1
%268 = OpCompositeConstruct %v2float %261 %263
%269 = OpCompositeConstruct %v2float %265 %267
%270 = OpCompositeConstruct %mat2v2float %268 %269
OpStore %271 %270
%272 = OpFunctionCall %bool %test_bifffff22 %251 %253 %255 %257 %259 %271
OpBranch %249
%249 = OpLabel
%273 = OpPhi %bool %false %239 %272 %248
OpSelectionMerge %275 None
OpBranchConditional %273 %274 %275
%274 = OpLabel
%276 = OpLoad %int %star
OpStore %277 %276
%278 = OpLoad %float %f1
OpStore %279 %278
%280 = OpLoad %float %f2
OpStore %281 %280
%282 = OpLoad %float %f3
OpStore %283 %282
%284 = OpLoad %float %f4
OpStore %285 %284
%286 = OpLoad %float %f1
%287 = OpFMul %float %286 %float_2
%288 = OpLoad %float %f2
%289 = OpFMul %float %288 %float_2
%290 = OpLoad %float %f3
%291 = OpFMul %float %290 %float_2
%292 = OpLoad %float %f4
%293 = OpFMul %float %292 %float_2
%294 = OpCompositeConstruct %v2float %287 %289
%295 = OpCompositeConstruct %v2float %291 %293
%296 = OpCompositeConstruct %mat2v2float %294 %295
OpStore %297 %296
%298 = OpFunctionCall %bool %test_bifffff22 %277 %279 %281 %283 %285 %297
OpBranch %275
%275 = OpLabel
%299 = OpPhi %bool %false %249 %298 %274
OpSelectionMerge %301 None
OpBranchConditional %299 %300 %301
%300 = OpLabel
%302 = OpLoad %int %slash
OpStore %303 %302
%304 = OpLoad %float %f1
OpStore %305 %304
%306 = OpLoad %float %f2
OpStore %307 %306
%308 = OpLoad %float %f3
OpStore %309 %308
%310 = OpLoad %float %f4
OpStore %311 %310
%312 = OpLoad %float %f1
%314 = OpFMul %float %312 %float_0_5
%315 = OpLoad %float %f2
%316 = OpFMul %float %315 %float_0_5
%317 = OpLoad %float %f3
%318 = OpFMul %float %317 %float_0_5
%319 = OpLoad %float %f4
%320 = OpFMul %float %319 %float_0_5
%321 = OpCompositeConstruct %v2float %314 %316
%322 = OpCompositeConstruct %v2float %318 %320
%323 = OpCompositeConstruct %mat2v2float %321 %322
OpStore %324 %323
%325 = OpFunctionCall %bool %test_bifffff22 %303 %305 %307 %309 %311 %324
OpBranch %301
%301 = OpLabel
%326 = OpPhi %bool %false %275 %325 %300
OpSelectionMerge %331 None
OpBranchConditional %326 %329 %330
%329 = OpLabel
%332 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%333 = OpLoad %v4float %332
OpStore %327 %333
OpBranch %331
%330 = OpLabel
%334 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%335 = OpLoad %v4float %334
OpStore %327 %335
OpBranch %331
%331 = OpLabel
%336 = OpLoad %v4float %327
OpReturnValue %336
OpFunctionEnd
