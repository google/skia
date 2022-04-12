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
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
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
%75 = OpConstantComposite %mat2v2float %74 %74
%float_2 = OpConstant %float 2
%95 = OpConstantComposite %v2float %float_2 %float_2
%96 = OpConstantComposite %mat2v2float %95 %95
%false = OpConstantFalse %bool
%int_0 = OpConstant %int 0
%143 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%84 = OpCompositeExtract %v2float %83 0
%85 = OpCompositeExtract %v2float %75 0
%86 = OpFSub %v2float %84 %85
%87 = OpCompositeExtract %v2float %83 1
%88 = OpCompositeExtract %v2float %75 1
%89 = OpFSub %v2float %87 %88
%90 = OpCompositeConstruct %mat2v2float %86 %89
OpStore %m2 %90
OpBranch %67
%70 = OpLabel
%91 = OpLoad %mat2v2float %m2
%93 = OpMatrixTimesScalar %mat2v2float %91 %float_2
OpStore %m2 %93
OpBranch %67
%71 = OpLabel
%94 = OpLoad %mat2v2float %m2
%97 = OpCompositeExtract %v2float %94 0
%98 = OpCompositeExtract %v2float %96 0
%99 = OpFDiv %v2float %97 %98
%100 = OpCompositeExtract %v2float %94 1
%101 = OpCompositeExtract %v2float %96 1
%102 = OpFDiv %v2float %100 %101
%103 = OpCompositeConstruct %mat2v2float %99 %102
OpStore %m2 %103
OpBranch %67
%67 = OpLabel
%106 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
%107 = OpLoad %v2float %106
%108 = OpCompositeExtract %float %107 0
%109 = OpAccessChain %_ptr_Function_v2float %42 %int_0
%110 = OpLoad %v2float %109
%111 = OpCompositeExtract %float %110 0
%112 = OpFOrdEqual %bool %108 %111
OpSelectionMerge %114 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
%115 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
%116 = OpLoad %v2float %115
%117 = OpCompositeExtract %float %116 1
%118 = OpAccessChain %_ptr_Function_v2float %42 %int_0
%119 = OpLoad %v2float %118
%120 = OpCompositeExtract %float %119 1
%121 = OpFOrdEqual %bool %117 %120
OpBranch %114
%114 = OpLabel
%122 = OpPhi %bool %false %67 %121 %113
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%125 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
%126 = OpLoad %v2float %125
%127 = OpCompositeExtract %float %126 0
%128 = OpAccessChain %_ptr_Function_v2float %42 %int_1
%129 = OpLoad %v2float %128
%130 = OpCompositeExtract %float %129 0
%131 = OpFOrdEqual %bool %127 %130
OpBranch %124
%124 = OpLabel
%132 = OpPhi %bool %false %114 %131 %123
OpSelectionMerge %134 None
OpBranchConditional %132 %133 %134
%133 = OpLabel
%135 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
%136 = OpLoad %v2float %135
%137 = OpCompositeExtract %float %136 1
%138 = OpAccessChain %_ptr_Function_v2float %42 %int_1
%139 = OpLoad %v2float %138
%140 = OpCompositeExtract %float %139 1
%141 = OpFOrdEqual %bool %137 %140
OpBranch %134
%134 = OpLabel
%142 = OpPhi %bool %false %124 %141 %133
OpReturnValue %142
OpFunctionEnd
%main = OpFunction %v4float None %143
%144 = OpFunctionParameter %_ptr_Function_v2float
%145 = OpLabel
%f1 = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_float Function
%f3 = OpVariable %_ptr_Function_float Function
%f4 = OpVariable %_ptr_Function_float Function
%_0_expected = OpVariable %_ptr_Function_mat2v2float Function
%_1_one = OpVariable %_ptr_Function_float Function
%_2_m2 = OpVariable %_ptr_Function_mat2v2float Function
%247 = OpVariable %_ptr_Function_int Function
%249 = OpVariable %_ptr_Function_float Function
%251 = OpVariable %_ptr_Function_float Function
%253 = OpVariable %_ptr_Function_float Function
%255 = OpVariable %_ptr_Function_float Function
%267 = OpVariable %_ptr_Function_mat2v2float Function
%273 = OpVariable %_ptr_Function_int Function
%275 = OpVariable %_ptr_Function_float Function
%277 = OpVariable %_ptr_Function_float Function
%279 = OpVariable %_ptr_Function_float Function
%281 = OpVariable %_ptr_Function_float Function
%293 = OpVariable %_ptr_Function_mat2v2float Function
%299 = OpVariable %_ptr_Function_int Function
%301 = OpVariable %_ptr_Function_float Function
%303 = OpVariable %_ptr_Function_float Function
%305 = OpVariable %_ptr_Function_float Function
%307 = OpVariable %_ptr_Function_float Function
%320 = OpVariable %_ptr_Function_mat2v2float Function
%323 = OpVariable %_ptr_Function_v4float Function
OpStore %minus %int_2
OpStore %star %int_3
OpStore %slash %int_4
%147 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%148 = OpLoad %v4float %147
%149 = OpCompositeExtract %float %148 1
OpStore %f1 %149
%151 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%152 = OpLoad %v4float %151
%153 = OpCompositeExtract %float %152 1
%154 = OpFMul %float %float_2 %153
OpStore %f2 %154
%157 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%158 = OpLoad %v4float %157
%159 = OpCompositeExtract %float %158 1
%160 = OpFMul %float %float_3 %159
OpStore %f3 %160
%163 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%164 = OpLoad %v4float %163
%165 = OpCompositeExtract %float %164 1
%166 = OpFMul %float %float_4 %165
OpStore %f4 %166
%168 = OpLoad %float %f1
%169 = OpFAdd %float %168 %float_1
%170 = OpLoad %float %f2
%171 = OpFAdd %float %170 %float_1
%172 = OpLoad %float %f3
%173 = OpFAdd %float %172 %float_1
%174 = OpLoad %float %f4
%175 = OpFAdd %float %174 %float_1
%176 = OpCompositeConstruct %v2float %169 %171
%177 = OpCompositeConstruct %v2float %173 %175
%178 = OpCompositeConstruct %mat2v2float %176 %177
OpStore %_0_expected %178
%180 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%181 = OpLoad %v4float %180
%182 = OpCompositeExtract %float %181 0
OpStore %_1_one %182
%184 = OpLoad %float %f1
%185 = OpLoad %float %_1_one
%186 = OpFMul %float %184 %185
%187 = OpLoad %float %f2
%188 = OpLoad %float %_1_one
%189 = OpFMul %float %187 %188
%190 = OpLoad %float %f3
%191 = OpLoad %float %_1_one
%192 = OpFMul %float %190 %191
%193 = OpLoad %float %f4
%194 = OpLoad %float %_1_one
%195 = OpFMul %float %193 %194
%196 = OpCompositeConstruct %v2float %186 %189
%197 = OpCompositeConstruct %v2float %192 %195
%198 = OpCompositeConstruct %mat2v2float %196 %197
OpStore %_2_m2 %198
%199 = OpLoad %mat2v2float %_2_m2
%200 = OpCompositeExtract %v2float %199 0
%201 = OpCompositeExtract %v2float %75 0
%202 = OpFAdd %v2float %200 %201
%203 = OpCompositeExtract %v2float %199 1
%204 = OpCompositeExtract %v2float %75 1
%205 = OpFAdd %v2float %203 %204
%206 = OpCompositeConstruct %mat2v2float %202 %205
OpStore %_2_m2 %206
%207 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
%208 = OpLoad %v2float %207
%209 = OpCompositeExtract %float %208 0
%210 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
%211 = OpLoad %v2float %210
%212 = OpCompositeExtract %float %211 0
%213 = OpFOrdEqual %bool %209 %212
OpSelectionMerge %215 None
OpBranchConditional %213 %214 %215
%214 = OpLabel
%216 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
%217 = OpLoad %v2float %216
%218 = OpCompositeExtract %float %217 1
%219 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
%220 = OpLoad %v2float %219
%221 = OpCompositeExtract %float %220 1
%222 = OpFOrdEqual %bool %218 %221
OpBranch %215
%215 = OpLabel
%223 = OpPhi %bool %false %145 %222 %214
OpSelectionMerge %225 None
OpBranchConditional %223 %224 %225
%224 = OpLabel
%226 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
%227 = OpLoad %v2float %226
%228 = OpCompositeExtract %float %227 0
%229 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
%230 = OpLoad %v2float %229
%231 = OpCompositeExtract %float %230 0
%232 = OpFOrdEqual %bool %228 %231
OpBranch %225
%225 = OpLabel
%233 = OpPhi %bool %false %215 %232 %224
OpSelectionMerge %235 None
OpBranchConditional %233 %234 %235
%234 = OpLabel
%236 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
%237 = OpLoad %v2float %236
%238 = OpCompositeExtract %float %237 1
%239 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
%240 = OpLoad %v2float %239
%241 = OpCompositeExtract %float %240 1
%242 = OpFOrdEqual %bool %238 %241
OpBranch %235
%235 = OpLabel
%243 = OpPhi %bool %false %225 %242 %234
OpSelectionMerge %245 None
OpBranchConditional %243 %244 %245
%244 = OpLabel
%246 = OpLoad %int %minus
OpStore %247 %246
%248 = OpLoad %float %f1
OpStore %249 %248
%250 = OpLoad %float %f2
OpStore %251 %250
%252 = OpLoad %float %f3
OpStore %253 %252
%254 = OpLoad %float %f4
OpStore %255 %254
%256 = OpLoad %float %f1
%257 = OpFSub %float %256 %float_1
%258 = OpLoad %float %f2
%259 = OpFSub %float %258 %float_1
%260 = OpLoad %float %f3
%261 = OpFSub %float %260 %float_1
%262 = OpLoad %float %f4
%263 = OpFSub %float %262 %float_1
%264 = OpCompositeConstruct %v2float %257 %259
%265 = OpCompositeConstruct %v2float %261 %263
%266 = OpCompositeConstruct %mat2v2float %264 %265
OpStore %267 %266
%268 = OpFunctionCall %bool %test_bifffff22 %247 %249 %251 %253 %255 %267
OpBranch %245
%245 = OpLabel
%269 = OpPhi %bool %false %235 %268 %244
OpSelectionMerge %271 None
OpBranchConditional %269 %270 %271
%270 = OpLabel
%272 = OpLoad %int %star
OpStore %273 %272
%274 = OpLoad %float %f1
OpStore %275 %274
%276 = OpLoad %float %f2
OpStore %277 %276
%278 = OpLoad %float %f3
OpStore %279 %278
%280 = OpLoad %float %f4
OpStore %281 %280
%282 = OpLoad %float %f1
%283 = OpFMul %float %282 %float_2
%284 = OpLoad %float %f2
%285 = OpFMul %float %284 %float_2
%286 = OpLoad %float %f3
%287 = OpFMul %float %286 %float_2
%288 = OpLoad %float %f4
%289 = OpFMul %float %288 %float_2
%290 = OpCompositeConstruct %v2float %283 %285
%291 = OpCompositeConstruct %v2float %287 %289
%292 = OpCompositeConstruct %mat2v2float %290 %291
OpStore %293 %292
%294 = OpFunctionCall %bool %test_bifffff22 %273 %275 %277 %279 %281 %293
OpBranch %271
%271 = OpLabel
%295 = OpPhi %bool %false %245 %294 %270
OpSelectionMerge %297 None
OpBranchConditional %295 %296 %297
%296 = OpLabel
%298 = OpLoad %int %slash
OpStore %299 %298
%300 = OpLoad %float %f1
OpStore %301 %300
%302 = OpLoad %float %f2
OpStore %303 %302
%304 = OpLoad %float %f3
OpStore %305 %304
%306 = OpLoad %float %f4
OpStore %307 %306
%308 = OpLoad %float %f1
%310 = OpFMul %float %308 %float_0_5
%311 = OpLoad %float %f2
%312 = OpFMul %float %311 %float_0_5
%313 = OpLoad %float %f3
%314 = OpFMul %float %313 %float_0_5
%315 = OpLoad %float %f4
%316 = OpFMul %float %315 %float_0_5
%317 = OpCompositeConstruct %v2float %310 %312
%318 = OpCompositeConstruct %v2float %314 %316
%319 = OpCompositeConstruct %mat2v2float %317 %318
OpStore %320 %319
%321 = OpFunctionCall %bool %test_bifffff22 %299 %301 %303 %305 %307 %320
OpBranch %297
%297 = OpLabel
%322 = OpPhi %bool %false %271 %321 %296
OpSelectionMerge %327 None
OpBranchConditional %322 %325 %326
%325 = OpLabel
%328 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%329 = OpLoad %v4float %328
OpStore %323 %329
OpBranch %327
%326 = OpLabel
%330 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%331 = OpLoad %v4float %330
OpStore %323 %331
OpBranch %327
%327 = OpLabel
%332 = OpLoad %v4float %323
OpReturnValue %332
OpFunctionEnd
