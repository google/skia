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
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
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
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_float = OpTypePointer Function %float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%36 = OpTypeFunction %bool %_ptr_Function_int %_ptr_Function_float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_mat2v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%74 = OpConstantComposite %v2float %float_1 %float_1
%75 = OpConstantComposite %mat2v2float %74 %74
%float_2 = OpConstant %float 2
%91 = OpConstantComposite %v2float %float_2 %float_2
%92 = OpConstantComposite %mat2v2float %91 %91
%false = OpConstantFalse %bool
%int_0 = OpConstant %int 0
%137 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%test_bifffff22 = OpFunction %bool None %36
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
%76 = OpCompositeExtract %v2float %72 0
%77 = OpFAdd %v2float %76 %74
%78 = OpCompositeExtract %v2float %72 1
%79 = OpFAdd %v2float %78 %74
%80 = OpCompositeConstruct %mat2v2float %77 %79
OpStore %m2 %80
OpBranch %67
%69 = OpLabel
%81 = OpLoad %mat2v2float %m2
%82 = OpCompositeExtract %v2float %81 0
%83 = OpFSub %v2float %82 %74
%84 = OpCompositeExtract %v2float %81 1
%85 = OpFSub %v2float %84 %74
%86 = OpCompositeConstruct %mat2v2float %83 %85
OpStore %m2 %86
OpBranch %67
%70 = OpLabel
%87 = OpLoad %mat2v2float %m2
%89 = OpMatrixTimesScalar %mat2v2float %87 %float_2
OpStore %m2 %89
OpBranch %67
%71 = OpLabel
%90 = OpLoad %mat2v2float %m2
%93 = OpCompositeExtract %v2float %90 0
%94 = OpFDiv %v2float %93 %91
%95 = OpCompositeExtract %v2float %90 1
%96 = OpFDiv %v2float %95 %91
%97 = OpCompositeConstruct %mat2v2float %94 %96
OpStore %m2 %97
OpBranch %67
%67 = OpLabel
%100 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
%101 = OpLoad %v2float %100
%102 = OpCompositeExtract %float %101 0
%103 = OpAccessChain %_ptr_Function_v2float %42 %int_0
%104 = OpLoad %v2float %103
%105 = OpCompositeExtract %float %104 0
%106 = OpFOrdEqual %bool %102 %105
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%109 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
%110 = OpLoad %v2float %109
%111 = OpCompositeExtract %float %110 1
%112 = OpAccessChain %_ptr_Function_v2float %42 %int_0
%113 = OpLoad %v2float %112
%114 = OpCompositeExtract %float %113 1
%115 = OpFOrdEqual %bool %111 %114
OpBranch %108
%108 = OpLabel
%116 = OpPhi %bool %false %67 %115 %107
OpSelectionMerge %118 None
OpBranchConditional %116 %117 %118
%117 = OpLabel
%119 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
%120 = OpLoad %v2float %119
%121 = OpCompositeExtract %float %120 0
%122 = OpAccessChain %_ptr_Function_v2float %42 %int_1
%123 = OpLoad %v2float %122
%124 = OpCompositeExtract %float %123 0
%125 = OpFOrdEqual %bool %121 %124
OpBranch %118
%118 = OpLabel
%126 = OpPhi %bool %false %108 %125 %117
OpSelectionMerge %128 None
OpBranchConditional %126 %127 %128
%127 = OpLabel
%129 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
%130 = OpLoad %v2float %129
%131 = OpCompositeExtract %float %130 1
%132 = OpAccessChain %_ptr_Function_v2float %42 %int_1
%133 = OpLoad %v2float %132
%134 = OpCompositeExtract %float %133 1
%135 = OpFOrdEqual %bool %131 %134
OpBranch %128
%128 = OpLabel
%136 = OpPhi %bool %false %118 %135 %127
OpReturnValue %136
OpFunctionEnd
%main = OpFunction %v4float None %137
%138 = OpFunctionParameter %_ptr_Function_v2float
%139 = OpLabel
%f1 = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_float Function
%f3 = OpVariable %_ptr_Function_float Function
%f4 = OpVariable %_ptr_Function_float Function
%_0_expected = OpVariable %_ptr_Function_mat2v2float Function
%_1_one = OpVariable %_ptr_Function_float Function
%_2_m2 = OpVariable %_ptr_Function_mat2v2float Function
%239 = OpVariable %_ptr_Function_int Function
%241 = OpVariable %_ptr_Function_float Function
%243 = OpVariable %_ptr_Function_float Function
%245 = OpVariable %_ptr_Function_float Function
%247 = OpVariable %_ptr_Function_float Function
%259 = OpVariable %_ptr_Function_mat2v2float Function
%265 = OpVariable %_ptr_Function_int Function
%267 = OpVariable %_ptr_Function_float Function
%269 = OpVariable %_ptr_Function_float Function
%271 = OpVariable %_ptr_Function_float Function
%273 = OpVariable %_ptr_Function_float Function
%285 = OpVariable %_ptr_Function_mat2v2float Function
%291 = OpVariable %_ptr_Function_int Function
%293 = OpVariable %_ptr_Function_float Function
%295 = OpVariable %_ptr_Function_float Function
%297 = OpVariable %_ptr_Function_float Function
%299 = OpVariable %_ptr_Function_float Function
%312 = OpVariable %_ptr_Function_mat2v2float Function
%315 = OpVariable %_ptr_Function_v4float Function
OpStore %minus %int_2
OpStore %star %int_3
OpStore %slash %int_4
%141 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%142 = OpLoad %v4float %141
%143 = OpCompositeExtract %float %142 1
OpStore %f1 %143
%145 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%146 = OpLoad %v4float %145
%147 = OpCompositeExtract %float %146 1
%148 = OpFMul %float %float_2 %147
OpStore %f2 %148
%151 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%152 = OpLoad %v4float %151
%153 = OpCompositeExtract %float %152 1
%154 = OpFMul %float %float_3 %153
OpStore %f3 %154
%157 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%158 = OpLoad %v4float %157
%159 = OpCompositeExtract %float %158 1
%160 = OpFMul %float %float_4 %159
OpStore %f4 %160
%162 = OpLoad %float %f1
%163 = OpFAdd %float %162 %float_1
%164 = OpLoad %float %f2
%165 = OpFAdd %float %164 %float_1
%166 = OpLoad %float %f3
%167 = OpFAdd %float %166 %float_1
%168 = OpLoad %float %f4
%169 = OpFAdd %float %168 %float_1
%170 = OpCompositeConstruct %v2float %163 %165
%171 = OpCompositeConstruct %v2float %167 %169
%172 = OpCompositeConstruct %mat2v2float %170 %171
OpStore %_0_expected %172
%174 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%175 = OpLoad %v4float %174
%176 = OpCompositeExtract %float %175 0
OpStore %_1_one %176
%178 = OpLoad %float %f1
%179 = OpLoad %float %_1_one
%180 = OpFMul %float %178 %179
%181 = OpLoad %float %f2
%182 = OpLoad %float %_1_one
%183 = OpFMul %float %181 %182
%184 = OpLoad %float %f3
%185 = OpLoad %float %_1_one
%186 = OpFMul %float %184 %185
%187 = OpLoad %float %f4
%188 = OpLoad %float %_1_one
%189 = OpFMul %float %187 %188
%190 = OpCompositeConstruct %v2float %180 %183
%191 = OpCompositeConstruct %v2float %186 %189
%192 = OpCompositeConstruct %mat2v2float %190 %191
OpStore %_2_m2 %192
%193 = OpLoad %mat2v2float %_2_m2
%194 = OpCompositeExtract %v2float %193 0
%195 = OpFAdd %v2float %194 %74
%196 = OpCompositeExtract %v2float %193 1
%197 = OpFAdd %v2float %196 %74
%198 = OpCompositeConstruct %mat2v2float %195 %197
OpStore %_2_m2 %198
%199 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
%200 = OpLoad %v2float %199
%201 = OpCompositeExtract %float %200 0
%202 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
%203 = OpLoad %v2float %202
%204 = OpCompositeExtract %float %203 0
%205 = OpFOrdEqual %bool %201 %204
OpSelectionMerge %207 None
OpBranchConditional %205 %206 %207
%206 = OpLabel
%208 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
%209 = OpLoad %v2float %208
%210 = OpCompositeExtract %float %209 1
%211 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
%212 = OpLoad %v2float %211
%213 = OpCompositeExtract %float %212 1
%214 = OpFOrdEqual %bool %210 %213
OpBranch %207
%207 = OpLabel
%215 = OpPhi %bool %false %139 %214 %206
OpSelectionMerge %217 None
OpBranchConditional %215 %216 %217
%216 = OpLabel
%218 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
%219 = OpLoad %v2float %218
%220 = OpCompositeExtract %float %219 0
%221 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
%222 = OpLoad %v2float %221
%223 = OpCompositeExtract %float %222 0
%224 = OpFOrdEqual %bool %220 %223
OpBranch %217
%217 = OpLabel
%225 = OpPhi %bool %false %207 %224 %216
OpSelectionMerge %227 None
OpBranchConditional %225 %226 %227
%226 = OpLabel
%228 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
%229 = OpLoad %v2float %228
%230 = OpCompositeExtract %float %229 1
%231 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
%232 = OpLoad %v2float %231
%233 = OpCompositeExtract %float %232 1
%234 = OpFOrdEqual %bool %230 %233
OpBranch %227
%227 = OpLabel
%235 = OpPhi %bool %false %217 %234 %226
OpSelectionMerge %237 None
OpBranchConditional %235 %236 %237
%236 = OpLabel
%238 = OpLoad %int %minus
OpStore %239 %238
%240 = OpLoad %float %f1
OpStore %241 %240
%242 = OpLoad %float %f2
OpStore %243 %242
%244 = OpLoad %float %f3
OpStore %245 %244
%246 = OpLoad %float %f4
OpStore %247 %246
%248 = OpLoad %float %f1
%249 = OpFSub %float %248 %float_1
%250 = OpLoad %float %f2
%251 = OpFSub %float %250 %float_1
%252 = OpLoad %float %f3
%253 = OpFSub %float %252 %float_1
%254 = OpLoad %float %f4
%255 = OpFSub %float %254 %float_1
%256 = OpCompositeConstruct %v2float %249 %251
%257 = OpCompositeConstruct %v2float %253 %255
%258 = OpCompositeConstruct %mat2v2float %256 %257
OpStore %259 %258
%260 = OpFunctionCall %bool %test_bifffff22 %239 %241 %243 %245 %247 %259
OpBranch %237
%237 = OpLabel
%261 = OpPhi %bool %false %227 %260 %236
OpSelectionMerge %263 None
OpBranchConditional %261 %262 %263
%262 = OpLabel
%264 = OpLoad %int %star
OpStore %265 %264
%266 = OpLoad %float %f1
OpStore %267 %266
%268 = OpLoad %float %f2
OpStore %269 %268
%270 = OpLoad %float %f3
OpStore %271 %270
%272 = OpLoad %float %f4
OpStore %273 %272
%274 = OpLoad %float %f1
%275 = OpFMul %float %274 %float_2
%276 = OpLoad %float %f2
%277 = OpFMul %float %276 %float_2
%278 = OpLoad %float %f3
%279 = OpFMul %float %278 %float_2
%280 = OpLoad %float %f4
%281 = OpFMul %float %280 %float_2
%282 = OpCompositeConstruct %v2float %275 %277
%283 = OpCompositeConstruct %v2float %279 %281
%284 = OpCompositeConstruct %mat2v2float %282 %283
OpStore %285 %284
%286 = OpFunctionCall %bool %test_bifffff22 %265 %267 %269 %271 %273 %285
OpBranch %263
%263 = OpLabel
%287 = OpPhi %bool %false %237 %286 %262
OpSelectionMerge %289 None
OpBranchConditional %287 %288 %289
%288 = OpLabel
%290 = OpLoad %int %slash
OpStore %291 %290
%292 = OpLoad %float %f1
OpStore %293 %292
%294 = OpLoad %float %f2
OpStore %295 %294
%296 = OpLoad %float %f3
OpStore %297 %296
%298 = OpLoad %float %f4
OpStore %299 %298
%300 = OpLoad %float %f1
%302 = OpFMul %float %300 %float_0_5
%303 = OpLoad %float %f2
%304 = OpFMul %float %303 %float_0_5
%305 = OpLoad %float %f3
%306 = OpFMul %float %305 %float_0_5
%307 = OpLoad %float %f4
%308 = OpFMul %float %307 %float_0_5
%309 = OpCompositeConstruct %v2float %302 %304
%310 = OpCompositeConstruct %v2float %306 %308
%311 = OpCompositeConstruct %mat2v2float %309 %310
OpStore %312 %311
%313 = OpFunctionCall %bool %test_bifffff22 %291 %293 %295 %297 %299 %312
OpBranch %289
%289 = OpLabel
%314 = OpPhi %bool %false %263 %313 %288
OpSelectionMerge %319 None
OpBranchConditional %314 %317 %318
%317 = OpLabel
%320 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%321 = OpLoad %v4float %320
OpStore %315 %321
OpBranch %319
%318 = OpLabel
%322 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%323 = OpLoad %v4float %322
OpStore %315 %323
OpBranch %319
%319 = OpLabel
%324 = OpLoad %v4float %315
OpReturnValue %324
OpFunctionEnd
