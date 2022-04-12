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
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
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
%74 = OpConstantComposite %v2float %float_1 %float_1
%float_2 = OpConstant %float 2
%96 = OpConstantComposite %v2float %float_2 %float_2
%false = OpConstantFalse %bool
%int_0 = OpConstant %int 0
%144 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%84 = OpCompositeConstruct %mat2v2float %74 %74
%85 = OpCompositeExtract %v2float %83 0
%86 = OpCompositeExtract %v2float %84 0
%87 = OpFSub %v2float %85 %86
%88 = OpCompositeExtract %v2float %83 1
%89 = OpCompositeExtract %v2float %84 1
%90 = OpFSub %v2float %88 %89
%91 = OpCompositeConstruct %mat2v2float %87 %90
OpStore %m2 %91
OpBranch %67
%70 = OpLabel
%92 = OpLoad %mat2v2float %m2
%94 = OpMatrixTimesScalar %mat2v2float %92 %float_2
OpStore %m2 %94
OpBranch %67
%71 = OpLabel
%95 = OpLoad %mat2v2float %m2
%97 = OpCompositeConstruct %mat2v2float %96 %96
%98 = OpCompositeExtract %v2float %95 0
%99 = OpCompositeExtract %v2float %97 0
%100 = OpFDiv %v2float %98 %99
%101 = OpCompositeExtract %v2float %95 1
%102 = OpCompositeExtract %v2float %97 1
%103 = OpFDiv %v2float %101 %102
%104 = OpCompositeConstruct %mat2v2float %100 %103
OpStore %m2 %104
OpBranch %67
%67 = OpLabel
%107 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
%108 = OpLoad %v2float %107
%109 = OpCompositeExtract %float %108 0
%110 = OpAccessChain %_ptr_Function_v2float %42 %int_0
%111 = OpLoad %v2float %110
%112 = OpCompositeExtract %float %111 0
%113 = OpFOrdEqual %bool %109 %112
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%116 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
%117 = OpLoad %v2float %116
%118 = OpCompositeExtract %float %117 1
%119 = OpAccessChain %_ptr_Function_v2float %42 %int_0
%120 = OpLoad %v2float %119
%121 = OpCompositeExtract %float %120 1
%122 = OpFOrdEqual %bool %118 %121
OpBranch %115
%115 = OpLabel
%123 = OpPhi %bool %false %67 %122 %114
OpSelectionMerge %125 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
%126 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
%127 = OpLoad %v2float %126
%128 = OpCompositeExtract %float %127 0
%129 = OpAccessChain %_ptr_Function_v2float %42 %int_1
%130 = OpLoad %v2float %129
%131 = OpCompositeExtract %float %130 0
%132 = OpFOrdEqual %bool %128 %131
OpBranch %125
%125 = OpLabel
%133 = OpPhi %bool %false %115 %132 %124
OpSelectionMerge %135 None
OpBranchConditional %133 %134 %135
%134 = OpLabel
%136 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
%137 = OpLoad %v2float %136
%138 = OpCompositeExtract %float %137 1
%139 = OpAccessChain %_ptr_Function_v2float %42 %int_1
%140 = OpLoad %v2float %139
%141 = OpCompositeExtract %float %140 1
%142 = OpFOrdEqual %bool %138 %141
OpBranch %135
%135 = OpLabel
%143 = OpPhi %bool %false %125 %142 %134
OpReturnValue %143
OpFunctionEnd
%main = OpFunction %v4float None %144
%145 = OpFunctionParameter %_ptr_Function_v2float
%146 = OpLabel
%f1 = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_float Function
%f3 = OpVariable %_ptr_Function_float Function
%f4 = OpVariable %_ptr_Function_float Function
%_0_expected = OpVariable %_ptr_Function_mat2v2float Function
%_1_one = OpVariable %_ptr_Function_float Function
%_2_m2 = OpVariable %_ptr_Function_mat2v2float Function
%249 = OpVariable %_ptr_Function_int Function
%251 = OpVariable %_ptr_Function_float Function
%253 = OpVariable %_ptr_Function_float Function
%255 = OpVariable %_ptr_Function_float Function
%257 = OpVariable %_ptr_Function_float Function
%269 = OpVariable %_ptr_Function_mat2v2float Function
%275 = OpVariable %_ptr_Function_int Function
%277 = OpVariable %_ptr_Function_float Function
%279 = OpVariable %_ptr_Function_float Function
%281 = OpVariable %_ptr_Function_float Function
%283 = OpVariable %_ptr_Function_float Function
%295 = OpVariable %_ptr_Function_mat2v2float Function
%301 = OpVariable %_ptr_Function_int Function
%303 = OpVariable %_ptr_Function_float Function
%305 = OpVariable %_ptr_Function_float Function
%307 = OpVariable %_ptr_Function_float Function
%309 = OpVariable %_ptr_Function_float Function
%322 = OpVariable %_ptr_Function_mat2v2float Function
%325 = OpVariable %_ptr_Function_v4float Function
OpStore %minus %int_2
OpStore %star %int_3
OpStore %slash %int_4
%148 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%149 = OpLoad %v4float %148
%150 = OpCompositeExtract %float %149 1
OpStore %f1 %150
%152 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%153 = OpLoad %v4float %152
%154 = OpCompositeExtract %float %153 1
%155 = OpFMul %float %float_2 %154
OpStore %f2 %155
%158 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%159 = OpLoad %v4float %158
%160 = OpCompositeExtract %float %159 1
%161 = OpFMul %float %float_3 %160
OpStore %f3 %161
%164 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%165 = OpLoad %v4float %164
%166 = OpCompositeExtract %float %165 1
%167 = OpFMul %float %float_4 %166
OpStore %f4 %167
%169 = OpLoad %float %f1
%170 = OpFAdd %float %169 %float_1
%171 = OpLoad %float %f2
%172 = OpFAdd %float %171 %float_1
%173 = OpLoad %float %f3
%174 = OpFAdd %float %173 %float_1
%175 = OpLoad %float %f4
%176 = OpFAdd %float %175 %float_1
%177 = OpCompositeConstruct %v2float %170 %172
%178 = OpCompositeConstruct %v2float %174 %176
%179 = OpCompositeConstruct %mat2v2float %177 %178
OpStore %_0_expected %179
%181 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%182 = OpLoad %v4float %181
%183 = OpCompositeExtract %float %182 0
OpStore %_1_one %183
%185 = OpLoad %float %f1
%186 = OpLoad %float %_1_one
%187 = OpFMul %float %185 %186
%188 = OpLoad %float %f2
%189 = OpLoad %float %_1_one
%190 = OpFMul %float %188 %189
%191 = OpLoad %float %f3
%192 = OpLoad %float %_1_one
%193 = OpFMul %float %191 %192
%194 = OpLoad %float %f4
%195 = OpLoad %float %_1_one
%196 = OpFMul %float %194 %195
%197 = OpCompositeConstruct %v2float %187 %190
%198 = OpCompositeConstruct %v2float %193 %196
%199 = OpCompositeConstruct %mat2v2float %197 %198
OpStore %_2_m2 %199
%200 = OpLoad %mat2v2float %_2_m2
%201 = OpCompositeConstruct %mat2v2float %74 %74
%202 = OpCompositeExtract %v2float %200 0
%203 = OpCompositeExtract %v2float %201 0
%204 = OpFAdd %v2float %202 %203
%205 = OpCompositeExtract %v2float %200 1
%206 = OpCompositeExtract %v2float %201 1
%207 = OpFAdd %v2float %205 %206
%208 = OpCompositeConstruct %mat2v2float %204 %207
OpStore %_2_m2 %208
%209 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
%210 = OpLoad %v2float %209
%211 = OpCompositeExtract %float %210 0
%212 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
%213 = OpLoad %v2float %212
%214 = OpCompositeExtract %float %213 0
%215 = OpFOrdEqual %bool %211 %214
OpSelectionMerge %217 None
OpBranchConditional %215 %216 %217
%216 = OpLabel
%218 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
%219 = OpLoad %v2float %218
%220 = OpCompositeExtract %float %219 1
%221 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
%222 = OpLoad %v2float %221
%223 = OpCompositeExtract %float %222 1
%224 = OpFOrdEqual %bool %220 %223
OpBranch %217
%217 = OpLabel
%225 = OpPhi %bool %false %146 %224 %216
OpSelectionMerge %227 None
OpBranchConditional %225 %226 %227
%226 = OpLabel
%228 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
%229 = OpLoad %v2float %228
%230 = OpCompositeExtract %float %229 0
%231 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
%232 = OpLoad %v2float %231
%233 = OpCompositeExtract %float %232 0
%234 = OpFOrdEqual %bool %230 %233
OpBranch %227
%227 = OpLabel
%235 = OpPhi %bool %false %217 %234 %226
OpSelectionMerge %237 None
OpBranchConditional %235 %236 %237
%236 = OpLabel
%238 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
%239 = OpLoad %v2float %238
%240 = OpCompositeExtract %float %239 1
%241 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
%242 = OpLoad %v2float %241
%243 = OpCompositeExtract %float %242 1
%244 = OpFOrdEqual %bool %240 %243
OpBranch %237
%237 = OpLabel
%245 = OpPhi %bool %false %227 %244 %236
OpSelectionMerge %247 None
OpBranchConditional %245 %246 %247
%246 = OpLabel
%248 = OpLoad %int %minus
OpStore %249 %248
%250 = OpLoad %float %f1
OpStore %251 %250
%252 = OpLoad %float %f2
OpStore %253 %252
%254 = OpLoad %float %f3
OpStore %255 %254
%256 = OpLoad %float %f4
OpStore %257 %256
%258 = OpLoad %float %f1
%259 = OpFSub %float %258 %float_1
%260 = OpLoad %float %f2
%261 = OpFSub %float %260 %float_1
%262 = OpLoad %float %f3
%263 = OpFSub %float %262 %float_1
%264 = OpLoad %float %f4
%265 = OpFSub %float %264 %float_1
%266 = OpCompositeConstruct %v2float %259 %261
%267 = OpCompositeConstruct %v2float %263 %265
%268 = OpCompositeConstruct %mat2v2float %266 %267
OpStore %269 %268
%270 = OpFunctionCall %bool %test_bifffff22 %249 %251 %253 %255 %257 %269
OpBranch %247
%247 = OpLabel
%271 = OpPhi %bool %false %237 %270 %246
OpSelectionMerge %273 None
OpBranchConditional %271 %272 %273
%272 = OpLabel
%274 = OpLoad %int %star
OpStore %275 %274
%276 = OpLoad %float %f1
OpStore %277 %276
%278 = OpLoad %float %f2
OpStore %279 %278
%280 = OpLoad %float %f3
OpStore %281 %280
%282 = OpLoad %float %f4
OpStore %283 %282
%284 = OpLoad %float %f1
%285 = OpFMul %float %284 %float_2
%286 = OpLoad %float %f2
%287 = OpFMul %float %286 %float_2
%288 = OpLoad %float %f3
%289 = OpFMul %float %288 %float_2
%290 = OpLoad %float %f4
%291 = OpFMul %float %290 %float_2
%292 = OpCompositeConstruct %v2float %285 %287
%293 = OpCompositeConstruct %v2float %289 %291
%294 = OpCompositeConstruct %mat2v2float %292 %293
OpStore %295 %294
%296 = OpFunctionCall %bool %test_bifffff22 %275 %277 %279 %281 %283 %295
OpBranch %273
%273 = OpLabel
%297 = OpPhi %bool %false %247 %296 %272
OpSelectionMerge %299 None
OpBranchConditional %297 %298 %299
%298 = OpLabel
%300 = OpLoad %int %slash
OpStore %301 %300
%302 = OpLoad %float %f1
OpStore %303 %302
%304 = OpLoad %float %f2
OpStore %305 %304
%306 = OpLoad %float %f3
OpStore %307 %306
%308 = OpLoad %float %f4
OpStore %309 %308
%310 = OpLoad %float %f1
%312 = OpFMul %float %310 %float_0_5
%313 = OpLoad %float %f2
%314 = OpFMul %float %313 %float_0_5
%315 = OpLoad %float %f3
%316 = OpFMul %float %315 %float_0_5
%317 = OpLoad %float %f4
%318 = OpFMul %float %317 %float_0_5
%319 = OpCompositeConstruct %v2float %312 %314
%320 = OpCompositeConstruct %v2float %316 %318
%321 = OpCompositeConstruct %mat2v2float %319 %320
OpStore %322 %321
%323 = OpFunctionCall %bool %test_bifffff22 %301 %303 %305 %307 %309 %322
OpBranch %299
%299 = OpLabel
%324 = OpPhi %bool %false %273 %323 %298
OpSelectionMerge %329 None
OpBranchConditional %324 %327 %328
%327 = OpLabel
%330 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%331 = OpLoad %v4float %330
OpStore %325 %331
OpBranch %329
%328 = OpLabel
%332 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%333 = OpLoad %v4float %332
OpStore %325 %333
OpBranch %329
%329 = OpLabel
%334 = OpLoad %v4float %325
OpReturnValue %334
OpFunctionEnd
