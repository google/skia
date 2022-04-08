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
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %f1 "f1"
OpName %f2 "f2"
OpName %f3 "f3"
OpName %v1 "v1"
OpName %v2 "v2"
OpName %v3 "v3"
OpName %m1 "m1"
OpName %m2 "m2"
OpName %m3 "m3"
OpName %S "S"
OpMemberName %S 0 "x"
OpMemberName %S 1 "y"
OpName %s1 "s1"
OpName %s2 "s2"
OpName %s3 "s3"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %_arr_float_int_4 ArrayStride 16
OpDecorate %_arr_v3int_int_2 ArrayStride 16
OpDecorate %_arr_mat2v2float_int_3 ArrayStride 32
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %S 1 Offset 4
OpDecorate %_arr_S_int_3 ArrayStride 16
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %337 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
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
%int_4 = OpConstant %int 4
%_arr_float_int_4 = OpTypeArray %float %int_4
%_ptr_Function__arr_float_int_4 = OpTypePointer Function %_arr_float_int_4
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_n4 = OpConstant %float -4
%v3int = OpTypeVector %int 3
%int_2 = OpConstant %int 2
%_arr_v3int_int_2 = OpTypeArray %v3int %int_2
%_ptr_Function__arr_v3int_int_2 = OpTypePointer Function %_arr_v3int_int_2
%int_1 = OpConstant %int 1
%int_3 = OpConstant %int 3
%48 = OpConstantComposite %v3int %int_1 %int_2 %int_3
%int_5 = OpConstant %int 5
%int_6 = OpConstant %int 6
%51 = OpConstantComposite %v3int %int_4 %int_5 %int_6
%int_n6 = OpConstant %int -6
%57 = OpConstantComposite %v3int %int_4 %int_5 %int_n6
%mat2v2float = OpTypeMatrix %v2float 2
%_arr_mat2v2float_int_3 = OpTypeArray %mat2v2float %int_3
%_ptr_Function__arr_mat2v2float_int_3 = OpTypePointer Function %_arr_mat2v2float_int_3
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%71 = OpConstantComposite %v2float %float_3 %float_4
%72 = OpConstantComposite %v2float %float_5 %float_6
%88 = OpConstantComposite %v2float %float_2 %float_3
%89 = OpConstantComposite %v2float %float_4 %float_5
%S = OpTypeStruct %int %int
%_arr_S_int_3 = OpTypeArray %S %int_3
%_ptr_Function__arr_S_int_3 = OpTypePointer Function %_arr_S_int_3
%int_0 = OpConstant %int 0
%false = OpConstantFalse %bool
%v3bool = OpTypeVector %bool 3
%v2bool = OpTypeVector %bool 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
%f1 = OpVariable %_ptr_Function__arr_float_int_4 Function
%f2 = OpVariable %_ptr_Function__arr_float_int_4 Function
%f3 = OpVariable %_ptr_Function__arr_float_int_4 Function
%v1 = OpVariable %_ptr_Function__arr_v3int_int_2 Function
%v2 = OpVariable %_ptr_Function__arr_v3int_int_2 Function
%v3 = OpVariable %_ptr_Function__arr_v3int_int_2 Function
%m1 = OpVariable %_ptr_Function__arr_mat2v2float_int_3 Function
%m2 = OpVariable %_ptr_Function__arr_mat2v2float_int_3 Function
%m3 = OpVariable %_ptr_Function__arr_mat2v2float_int_3 Function
%s1 = OpVariable %_ptr_Function__arr_S_int_3 Function
%s2 = OpVariable %_ptr_Function__arr_S_int_3 Function
%s3 = OpVariable %_ptr_Function__arr_S_int_3 Function
%330 = OpVariable %_ptr_Function_v4float Function
%35 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %f1 %35
%37 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %f2 %37
%40 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_n4
OpStore %f3 %40
%52 = OpCompositeConstruct %_arr_v3int_int_2 %48 %51
OpStore %v1 %52
%54 = OpCompositeConstruct %_arr_v3int_int_2 %48 %51
OpStore %v2 %54
%58 = OpCompositeConstruct %_arr_v3int_int_2 %48 %57
OpStore %v3 %58
%64 = OpCompositeConstruct %v2float %float_1 %float_0
%65 = OpCompositeConstruct %v2float %float_0 %float_1
%63 = OpCompositeConstruct %mat2v2float %64 %65
%67 = OpCompositeConstruct %v2float %float_2 %float_0
%68 = OpCompositeConstruct %v2float %float_0 %float_2
%66 = OpCompositeConstruct %mat2v2float %67 %68
%73 = OpCompositeConstruct %mat2v2float %71 %72
%74 = OpCompositeConstruct %_arr_mat2v2float_int_3 %63 %66 %73
OpStore %m1 %74
%77 = OpCompositeConstruct %v2float %float_1 %float_0
%78 = OpCompositeConstruct %v2float %float_0 %float_1
%76 = OpCompositeConstruct %mat2v2float %77 %78
%80 = OpCompositeConstruct %v2float %float_2 %float_0
%81 = OpCompositeConstruct %v2float %float_0 %float_2
%79 = OpCompositeConstruct %mat2v2float %80 %81
%82 = OpCompositeConstruct %mat2v2float %71 %72
%83 = OpCompositeConstruct %_arr_mat2v2float_int_3 %76 %79 %82
OpStore %m2 %83
%86 = OpCompositeConstruct %v2float %float_1 %float_0
%87 = OpCompositeConstruct %v2float %float_0 %float_1
%85 = OpCompositeConstruct %mat2v2float %86 %87
%90 = OpCompositeConstruct %mat2v2float %88 %89
%92 = OpCompositeConstruct %v2float %float_6 %float_0
%93 = OpCompositeConstruct %v2float %float_0 %float_6
%91 = OpCompositeConstruct %mat2v2float %92 %93
%94 = OpCompositeConstruct %_arr_mat2v2float_int_3 %85 %90 %91
OpStore %m3 %94
%99 = OpCompositeConstruct %S %int_1 %int_2
%100 = OpCompositeConstruct %S %int_3 %int_4
%101 = OpCompositeConstruct %S %int_5 %int_6
%102 = OpCompositeConstruct %_arr_S_int_3 %99 %100 %101
OpStore %s1 %102
%104 = OpCompositeConstruct %S %int_1 %int_2
%106 = OpCompositeConstruct %S %int_0 %int_0
%107 = OpCompositeConstruct %S %int_5 %int_6
%108 = OpCompositeConstruct %_arr_S_int_3 %104 %106 %107
OpStore %s2 %108
%110 = OpCompositeConstruct %S %int_1 %int_2
%111 = OpCompositeConstruct %S %int_3 %int_4
%112 = OpCompositeConstruct %S %int_5 %int_6
%113 = OpCompositeConstruct %_arr_S_int_3 %110 %111 %112
OpStore %s3 %113
%115 = OpLoad %_arr_float_int_4 %f1
%116 = OpLoad %_arr_float_int_4 %f2
%117 = OpCompositeExtract %float %115 0
%118 = OpCompositeExtract %float %116 0
%119 = OpFOrdEqual %bool %117 %118
%120 = OpCompositeExtract %float %115 1
%121 = OpCompositeExtract %float %116 1
%122 = OpFOrdEqual %bool %120 %121
%123 = OpLogicalAnd %bool %122 %119
%124 = OpCompositeExtract %float %115 2
%125 = OpCompositeExtract %float %116 2
%126 = OpFOrdEqual %bool %124 %125
%127 = OpLogicalAnd %bool %126 %123
%128 = OpCompositeExtract %float %115 3
%129 = OpCompositeExtract %float %116 3
%130 = OpFOrdEqual %bool %128 %129
%131 = OpLogicalAnd %bool %130 %127
OpSelectionMerge %133 None
OpBranchConditional %131 %132 %133
%132 = OpLabel
%134 = OpLoad %_arr_float_int_4 %f1
%135 = OpLoad %_arr_float_int_4 %f3
%136 = OpCompositeExtract %float %134 0
%137 = OpCompositeExtract %float %135 0
%138 = OpFUnordNotEqual %bool %136 %137
%139 = OpCompositeExtract %float %134 1
%140 = OpCompositeExtract %float %135 1
%141 = OpFUnordNotEqual %bool %139 %140
%142 = OpLogicalOr %bool %141 %138
%143 = OpCompositeExtract %float %134 2
%144 = OpCompositeExtract %float %135 2
%145 = OpFUnordNotEqual %bool %143 %144
%146 = OpLogicalOr %bool %145 %142
%147 = OpCompositeExtract %float %134 3
%148 = OpCompositeExtract %float %135 3
%149 = OpFUnordNotEqual %bool %147 %148
%150 = OpLogicalOr %bool %149 %146
OpBranch %133
%133 = OpLabel
%151 = OpPhi %bool %false %25 %150 %132
OpSelectionMerge %153 None
OpBranchConditional %151 %152 %153
%152 = OpLabel
%154 = OpLoad %_arr_v3int_int_2 %v1
%155 = OpLoad %_arr_v3int_int_2 %v2
%156 = OpCompositeExtract %v3int %154 0
%157 = OpCompositeExtract %v3int %155 0
%158 = OpIEqual %v3bool %156 %157
%160 = OpAll %bool %158
%161 = OpCompositeExtract %v3int %154 1
%162 = OpCompositeExtract %v3int %155 1
%163 = OpIEqual %v3bool %161 %162
%164 = OpAll %bool %163
%165 = OpLogicalAnd %bool %164 %160
OpBranch %153
%153 = OpLabel
%166 = OpPhi %bool %false %133 %165 %152
OpSelectionMerge %168 None
OpBranchConditional %166 %167 %168
%167 = OpLabel
%169 = OpLoad %_arr_v3int_int_2 %v1
%170 = OpLoad %_arr_v3int_int_2 %v3
%171 = OpCompositeExtract %v3int %169 0
%172 = OpCompositeExtract %v3int %170 0
%173 = OpINotEqual %v3bool %171 %172
%174 = OpAny %bool %173
%175 = OpCompositeExtract %v3int %169 1
%176 = OpCompositeExtract %v3int %170 1
%177 = OpINotEqual %v3bool %175 %176
%178 = OpAny %bool %177
%179 = OpLogicalOr %bool %178 %174
OpBranch %168
%168 = OpLabel
%180 = OpPhi %bool %false %153 %179 %167
OpSelectionMerge %182 None
OpBranchConditional %180 %181 %182
%181 = OpLabel
%183 = OpLoad %_arr_mat2v2float_int_3 %m1
%184 = OpLoad %_arr_mat2v2float_int_3 %m2
%185 = OpCompositeExtract %mat2v2float %183 0
%186 = OpCompositeExtract %mat2v2float %184 0
%188 = OpCompositeExtract %v2float %185 0
%189 = OpCompositeExtract %v2float %186 0
%190 = OpFOrdEqual %v2bool %188 %189
%191 = OpAll %bool %190
%192 = OpCompositeExtract %v2float %185 1
%193 = OpCompositeExtract %v2float %186 1
%194 = OpFOrdEqual %v2bool %192 %193
%195 = OpAll %bool %194
%196 = OpLogicalAnd %bool %191 %195
%197 = OpCompositeExtract %mat2v2float %183 1
%198 = OpCompositeExtract %mat2v2float %184 1
%199 = OpCompositeExtract %v2float %197 0
%200 = OpCompositeExtract %v2float %198 0
%201 = OpFOrdEqual %v2bool %199 %200
%202 = OpAll %bool %201
%203 = OpCompositeExtract %v2float %197 1
%204 = OpCompositeExtract %v2float %198 1
%205 = OpFOrdEqual %v2bool %203 %204
%206 = OpAll %bool %205
%207 = OpLogicalAnd %bool %202 %206
%208 = OpLogicalAnd %bool %207 %196
%209 = OpCompositeExtract %mat2v2float %183 2
%210 = OpCompositeExtract %mat2v2float %184 2
%211 = OpCompositeExtract %v2float %209 0
%212 = OpCompositeExtract %v2float %210 0
%213 = OpFOrdEqual %v2bool %211 %212
%214 = OpAll %bool %213
%215 = OpCompositeExtract %v2float %209 1
%216 = OpCompositeExtract %v2float %210 1
%217 = OpFOrdEqual %v2bool %215 %216
%218 = OpAll %bool %217
%219 = OpLogicalAnd %bool %214 %218
%220 = OpLogicalAnd %bool %219 %208
OpBranch %182
%182 = OpLabel
%221 = OpPhi %bool %false %168 %220 %181
OpSelectionMerge %223 None
OpBranchConditional %221 %222 %223
%222 = OpLabel
%224 = OpLoad %_arr_mat2v2float_int_3 %m1
%225 = OpLoad %_arr_mat2v2float_int_3 %m3
%226 = OpCompositeExtract %mat2v2float %224 0
%227 = OpCompositeExtract %mat2v2float %225 0
%228 = OpCompositeExtract %v2float %226 0
%229 = OpCompositeExtract %v2float %227 0
%230 = OpFUnordNotEqual %v2bool %228 %229
%231 = OpAny %bool %230
%232 = OpCompositeExtract %v2float %226 1
%233 = OpCompositeExtract %v2float %227 1
%234 = OpFUnordNotEqual %v2bool %232 %233
%235 = OpAny %bool %234
%236 = OpLogicalOr %bool %231 %235
%237 = OpCompositeExtract %mat2v2float %224 1
%238 = OpCompositeExtract %mat2v2float %225 1
%239 = OpCompositeExtract %v2float %237 0
%240 = OpCompositeExtract %v2float %238 0
%241 = OpFUnordNotEqual %v2bool %239 %240
%242 = OpAny %bool %241
%243 = OpCompositeExtract %v2float %237 1
%244 = OpCompositeExtract %v2float %238 1
%245 = OpFUnordNotEqual %v2bool %243 %244
%246 = OpAny %bool %245
%247 = OpLogicalOr %bool %242 %246
%248 = OpLogicalOr %bool %247 %236
%249 = OpCompositeExtract %mat2v2float %224 2
%250 = OpCompositeExtract %mat2v2float %225 2
%251 = OpCompositeExtract %v2float %249 0
%252 = OpCompositeExtract %v2float %250 0
%253 = OpFUnordNotEqual %v2bool %251 %252
%254 = OpAny %bool %253
%255 = OpCompositeExtract %v2float %249 1
%256 = OpCompositeExtract %v2float %250 1
%257 = OpFUnordNotEqual %v2bool %255 %256
%258 = OpAny %bool %257
%259 = OpLogicalOr %bool %254 %258
%260 = OpLogicalOr %bool %259 %248
OpBranch %223
%223 = OpLabel
%261 = OpPhi %bool %false %182 %260 %222
OpSelectionMerge %263 None
OpBranchConditional %261 %262 %263
%262 = OpLabel
%264 = OpLoad %_arr_S_int_3 %s1
%265 = OpLoad %_arr_S_int_3 %s2
%266 = OpCompositeExtract %S %264 0
%267 = OpCompositeExtract %S %265 0
%268 = OpCompositeExtract %int %266 0
%269 = OpCompositeExtract %int %267 0
%270 = OpINotEqual %bool %268 %269
%271 = OpCompositeExtract %int %266 1
%272 = OpCompositeExtract %int %267 1
%273 = OpINotEqual %bool %271 %272
%274 = OpLogicalOr %bool %273 %270
%275 = OpCompositeExtract %S %264 1
%276 = OpCompositeExtract %S %265 1
%277 = OpCompositeExtract %int %275 0
%278 = OpCompositeExtract %int %276 0
%279 = OpINotEqual %bool %277 %278
%280 = OpCompositeExtract %int %275 1
%281 = OpCompositeExtract %int %276 1
%282 = OpINotEqual %bool %280 %281
%283 = OpLogicalOr %bool %282 %279
%284 = OpLogicalOr %bool %283 %274
%285 = OpCompositeExtract %S %264 2
%286 = OpCompositeExtract %S %265 2
%287 = OpCompositeExtract %int %285 0
%288 = OpCompositeExtract %int %286 0
%289 = OpINotEqual %bool %287 %288
%290 = OpCompositeExtract %int %285 1
%291 = OpCompositeExtract %int %286 1
%292 = OpINotEqual %bool %290 %291
%293 = OpLogicalOr %bool %292 %289
%294 = OpLogicalOr %bool %293 %284
OpBranch %263
%263 = OpLabel
%295 = OpPhi %bool %false %223 %294 %262
OpSelectionMerge %297 None
OpBranchConditional %295 %296 %297
%296 = OpLabel
%298 = OpLoad %_arr_S_int_3 %s3
%299 = OpLoad %_arr_S_int_3 %s1
%300 = OpCompositeExtract %S %298 0
%301 = OpCompositeExtract %S %299 0
%302 = OpCompositeExtract %int %300 0
%303 = OpCompositeExtract %int %301 0
%304 = OpIEqual %bool %302 %303
%305 = OpCompositeExtract %int %300 1
%306 = OpCompositeExtract %int %301 1
%307 = OpIEqual %bool %305 %306
%308 = OpLogicalAnd %bool %307 %304
%309 = OpCompositeExtract %S %298 1
%310 = OpCompositeExtract %S %299 1
%311 = OpCompositeExtract %int %309 0
%312 = OpCompositeExtract %int %310 0
%313 = OpIEqual %bool %311 %312
%314 = OpCompositeExtract %int %309 1
%315 = OpCompositeExtract %int %310 1
%316 = OpIEqual %bool %314 %315
%317 = OpLogicalAnd %bool %316 %313
%318 = OpLogicalAnd %bool %317 %308
%319 = OpCompositeExtract %S %298 2
%320 = OpCompositeExtract %S %299 2
%321 = OpCompositeExtract %int %319 0
%322 = OpCompositeExtract %int %320 0
%323 = OpIEqual %bool %321 %322
%324 = OpCompositeExtract %int %319 1
%325 = OpCompositeExtract %int %320 1
%326 = OpIEqual %bool %324 %325
%327 = OpLogicalAnd %bool %326 %323
%328 = OpLogicalAnd %bool %327 %318
OpBranch %297
%297 = OpLabel
%329 = OpPhi %bool %false %263 %328 %296
OpSelectionMerge %334 None
OpBranchConditional %329 %332 %333
%332 = OpLabel
%335 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%337 = OpLoad %v4float %335
OpStore %330 %337
OpBranch %334
%333 = OpLabel
%338 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%339 = OpLoad %v4float %338
OpStore %330 %339
OpBranch %334
%334 = OpLabel
%340 = OpLoad %v4float %330
OpReturnValue %340
OpFunctionEnd
