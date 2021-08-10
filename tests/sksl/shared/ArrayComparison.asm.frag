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
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %S 1 Offset 4
OpDecorate %_arr_S_int_3 ArrayStride 16
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
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
%332 = OpVariable %_ptr_Function_v4float Function
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
%71 = OpCompositeConstruct %v2float %float_3 %float_4
%72 = OpCompositeConstruct %v2float %float_5 %float_6
%73 = OpCompositeConstruct %mat2v2float %71 %72
%74 = OpCompositeConstruct %_arr_mat2v2float_int_3 %63 %66 %73
OpStore %m1 %74
%77 = OpCompositeConstruct %v2float %float_1 %float_0
%78 = OpCompositeConstruct %v2float %float_0 %float_1
%76 = OpCompositeConstruct %mat2v2float %77 %78
%80 = OpCompositeConstruct %v2float %float_2 %float_0
%81 = OpCompositeConstruct %v2float %float_0 %float_2
%79 = OpCompositeConstruct %mat2v2float %80 %81
%82 = OpCompositeConstruct %v2float %float_3 %float_4
%83 = OpCompositeConstruct %v2float %float_5 %float_6
%84 = OpCompositeConstruct %mat2v2float %82 %83
%85 = OpCompositeConstruct %_arr_mat2v2float_int_3 %76 %79 %84
OpStore %m2 %85
%88 = OpCompositeConstruct %v2float %float_1 %float_0
%89 = OpCompositeConstruct %v2float %float_0 %float_1
%87 = OpCompositeConstruct %mat2v2float %88 %89
%90 = OpCompositeConstruct %v2float %float_2 %float_3
%91 = OpCompositeConstruct %v2float %float_4 %float_5
%92 = OpCompositeConstruct %mat2v2float %90 %91
%94 = OpCompositeConstruct %v2float %float_6 %float_0
%95 = OpCompositeConstruct %v2float %float_0 %float_6
%93 = OpCompositeConstruct %mat2v2float %94 %95
%96 = OpCompositeConstruct %_arr_mat2v2float_int_3 %87 %92 %93
OpStore %m3 %96
%101 = OpCompositeConstruct %S %int_1 %int_2
%102 = OpCompositeConstruct %S %int_3 %int_4
%103 = OpCompositeConstruct %S %int_5 %int_6
%104 = OpCompositeConstruct %_arr_S_int_3 %101 %102 %103
OpStore %s1 %104
%106 = OpCompositeConstruct %S %int_1 %int_2
%108 = OpCompositeConstruct %S %int_0 %int_0
%109 = OpCompositeConstruct %S %int_5 %int_6
%110 = OpCompositeConstruct %_arr_S_int_3 %106 %108 %109
OpStore %s2 %110
%112 = OpCompositeConstruct %S %int_1 %int_2
%113 = OpCompositeConstruct %S %int_3 %int_4
%114 = OpCompositeConstruct %S %int_5 %int_6
%115 = OpCompositeConstruct %_arr_S_int_3 %112 %113 %114
OpStore %s3 %115
%117 = OpLoad %_arr_float_int_4 %f1
%118 = OpLoad %_arr_float_int_4 %f2
%119 = OpCompositeExtract %float %117 0
%120 = OpCompositeExtract %float %118 0
%121 = OpFOrdEqual %bool %119 %120
%122 = OpCompositeExtract %float %117 1
%123 = OpCompositeExtract %float %118 1
%124 = OpFOrdEqual %bool %122 %123
%125 = OpLogicalAnd %bool %124 %121
%126 = OpCompositeExtract %float %117 2
%127 = OpCompositeExtract %float %118 2
%128 = OpFOrdEqual %bool %126 %127
%129 = OpLogicalAnd %bool %128 %125
%130 = OpCompositeExtract %float %117 3
%131 = OpCompositeExtract %float %118 3
%132 = OpFOrdEqual %bool %130 %131
%133 = OpLogicalAnd %bool %132 %129
OpSelectionMerge %135 None
OpBranchConditional %133 %134 %135
%134 = OpLabel
%136 = OpLoad %_arr_float_int_4 %f1
%137 = OpLoad %_arr_float_int_4 %f3
%138 = OpCompositeExtract %float %136 0
%139 = OpCompositeExtract %float %137 0
%140 = OpFOrdNotEqual %bool %138 %139
%141 = OpCompositeExtract %float %136 1
%142 = OpCompositeExtract %float %137 1
%143 = OpFOrdNotEqual %bool %141 %142
%144 = OpLogicalOr %bool %143 %140
%145 = OpCompositeExtract %float %136 2
%146 = OpCompositeExtract %float %137 2
%147 = OpFOrdNotEqual %bool %145 %146
%148 = OpLogicalOr %bool %147 %144
%149 = OpCompositeExtract %float %136 3
%150 = OpCompositeExtract %float %137 3
%151 = OpFOrdNotEqual %bool %149 %150
%152 = OpLogicalOr %bool %151 %148
OpBranch %135
%135 = OpLabel
%153 = OpPhi %bool %false %25 %152 %134
OpSelectionMerge %155 None
OpBranchConditional %153 %154 %155
%154 = OpLabel
%156 = OpLoad %_arr_v3int_int_2 %v1
%157 = OpLoad %_arr_v3int_int_2 %v2
%158 = OpCompositeExtract %v3int %156 0
%159 = OpCompositeExtract %v3int %157 0
%160 = OpIEqual %v3bool %158 %159
%162 = OpAll %bool %160
%163 = OpCompositeExtract %v3int %156 1
%164 = OpCompositeExtract %v3int %157 1
%165 = OpIEqual %v3bool %163 %164
%166 = OpAll %bool %165
%167 = OpLogicalAnd %bool %166 %162
OpBranch %155
%155 = OpLabel
%168 = OpPhi %bool %false %135 %167 %154
OpSelectionMerge %170 None
OpBranchConditional %168 %169 %170
%169 = OpLabel
%171 = OpLoad %_arr_v3int_int_2 %v1
%172 = OpLoad %_arr_v3int_int_2 %v3
%173 = OpCompositeExtract %v3int %171 0
%174 = OpCompositeExtract %v3int %172 0
%175 = OpINotEqual %v3bool %173 %174
%176 = OpAny %bool %175
%177 = OpCompositeExtract %v3int %171 1
%178 = OpCompositeExtract %v3int %172 1
%179 = OpINotEqual %v3bool %177 %178
%180 = OpAny %bool %179
%181 = OpLogicalOr %bool %180 %176
OpBranch %170
%170 = OpLabel
%182 = OpPhi %bool %false %155 %181 %169
OpSelectionMerge %184 None
OpBranchConditional %182 %183 %184
%183 = OpLabel
%185 = OpLoad %_arr_mat2v2float_int_3 %m1
%186 = OpLoad %_arr_mat2v2float_int_3 %m2
%187 = OpCompositeExtract %mat2v2float %185 0
%188 = OpCompositeExtract %mat2v2float %186 0
%190 = OpCompositeExtract %v2float %187 0
%191 = OpCompositeExtract %v2float %188 0
%192 = OpFOrdEqual %v2bool %190 %191
%193 = OpAll %bool %192
%194 = OpCompositeExtract %v2float %187 1
%195 = OpCompositeExtract %v2float %188 1
%196 = OpFOrdEqual %v2bool %194 %195
%197 = OpAll %bool %196
%198 = OpLogicalAnd %bool %193 %197
%199 = OpCompositeExtract %mat2v2float %185 1
%200 = OpCompositeExtract %mat2v2float %186 1
%201 = OpCompositeExtract %v2float %199 0
%202 = OpCompositeExtract %v2float %200 0
%203 = OpFOrdEqual %v2bool %201 %202
%204 = OpAll %bool %203
%205 = OpCompositeExtract %v2float %199 1
%206 = OpCompositeExtract %v2float %200 1
%207 = OpFOrdEqual %v2bool %205 %206
%208 = OpAll %bool %207
%209 = OpLogicalAnd %bool %204 %208
%210 = OpLogicalAnd %bool %209 %198
%211 = OpCompositeExtract %mat2v2float %185 2
%212 = OpCompositeExtract %mat2v2float %186 2
%213 = OpCompositeExtract %v2float %211 0
%214 = OpCompositeExtract %v2float %212 0
%215 = OpFOrdEqual %v2bool %213 %214
%216 = OpAll %bool %215
%217 = OpCompositeExtract %v2float %211 1
%218 = OpCompositeExtract %v2float %212 1
%219 = OpFOrdEqual %v2bool %217 %218
%220 = OpAll %bool %219
%221 = OpLogicalAnd %bool %216 %220
%222 = OpLogicalAnd %bool %221 %210
OpBranch %184
%184 = OpLabel
%223 = OpPhi %bool %false %170 %222 %183
OpSelectionMerge %225 None
OpBranchConditional %223 %224 %225
%224 = OpLabel
%226 = OpLoad %_arr_mat2v2float_int_3 %m1
%227 = OpLoad %_arr_mat2v2float_int_3 %m3
%228 = OpCompositeExtract %mat2v2float %226 0
%229 = OpCompositeExtract %mat2v2float %227 0
%230 = OpCompositeExtract %v2float %228 0
%231 = OpCompositeExtract %v2float %229 0
%232 = OpFOrdNotEqual %v2bool %230 %231
%233 = OpAny %bool %232
%234 = OpCompositeExtract %v2float %228 1
%235 = OpCompositeExtract %v2float %229 1
%236 = OpFOrdNotEqual %v2bool %234 %235
%237 = OpAny %bool %236
%238 = OpLogicalOr %bool %233 %237
%239 = OpCompositeExtract %mat2v2float %226 1
%240 = OpCompositeExtract %mat2v2float %227 1
%241 = OpCompositeExtract %v2float %239 0
%242 = OpCompositeExtract %v2float %240 0
%243 = OpFOrdNotEqual %v2bool %241 %242
%244 = OpAny %bool %243
%245 = OpCompositeExtract %v2float %239 1
%246 = OpCompositeExtract %v2float %240 1
%247 = OpFOrdNotEqual %v2bool %245 %246
%248 = OpAny %bool %247
%249 = OpLogicalOr %bool %244 %248
%250 = OpLogicalOr %bool %249 %238
%251 = OpCompositeExtract %mat2v2float %226 2
%252 = OpCompositeExtract %mat2v2float %227 2
%253 = OpCompositeExtract %v2float %251 0
%254 = OpCompositeExtract %v2float %252 0
%255 = OpFOrdNotEqual %v2bool %253 %254
%256 = OpAny %bool %255
%257 = OpCompositeExtract %v2float %251 1
%258 = OpCompositeExtract %v2float %252 1
%259 = OpFOrdNotEqual %v2bool %257 %258
%260 = OpAny %bool %259
%261 = OpLogicalOr %bool %256 %260
%262 = OpLogicalOr %bool %261 %250
OpBranch %225
%225 = OpLabel
%263 = OpPhi %bool %false %184 %262 %224
OpSelectionMerge %265 None
OpBranchConditional %263 %264 %265
%264 = OpLabel
%266 = OpLoad %_arr_S_int_3 %s1
%267 = OpLoad %_arr_S_int_3 %s2
%268 = OpCompositeExtract %S %266 0
%269 = OpCompositeExtract %S %267 0
%270 = OpCompositeExtract %int %268 0
%271 = OpCompositeExtract %int %269 0
%272 = OpINotEqual %bool %270 %271
%273 = OpCompositeExtract %int %268 1
%274 = OpCompositeExtract %int %269 1
%275 = OpINotEqual %bool %273 %274
%276 = OpLogicalOr %bool %275 %272
%277 = OpCompositeExtract %S %266 1
%278 = OpCompositeExtract %S %267 1
%279 = OpCompositeExtract %int %277 0
%280 = OpCompositeExtract %int %278 0
%281 = OpINotEqual %bool %279 %280
%282 = OpCompositeExtract %int %277 1
%283 = OpCompositeExtract %int %278 1
%284 = OpINotEqual %bool %282 %283
%285 = OpLogicalOr %bool %284 %281
%286 = OpLogicalOr %bool %285 %276
%287 = OpCompositeExtract %S %266 2
%288 = OpCompositeExtract %S %267 2
%289 = OpCompositeExtract %int %287 0
%290 = OpCompositeExtract %int %288 0
%291 = OpINotEqual %bool %289 %290
%292 = OpCompositeExtract %int %287 1
%293 = OpCompositeExtract %int %288 1
%294 = OpINotEqual %bool %292 %293
%295 = OpLogicalOr %bool %294 %291
%296 = OpLogicalOr %bool %295 %286
OpBranch %265
%265 = OpLabel
%297 = OpPhi %bool %false %225 %296 %264
OpSelectionMerge %299 None
OpBranchConditional %297 %298 %299
%298 = OpLabel
%300 = OpLoad %_arr_S_int_3 %s3
%301 = OpLoad %_arr_S_int_3 %s1
%302 = OpCompositeExtract %S %300 0
%303 = OpCompositeExtract %S %301 0
%304 = OpCompositeExtract %int %302 0
%305 = OpCompositeExtract %int %303 0
%306 = OpIEqual %bool %304 %305
%307 = OpCompositeExtract %int %302 1
%308 = OpCompositeExtract %int %303 1
%309 = OpIEqual %bool %307 %308
%310 = OpLogicalAnd %bool %309 %306
%311 = OpCompositeExtract %S %300 1
%312 = OpCompositeExtract %S %301 1
%313 = OpCompositeExtract %int %311 0
%314 = OpCompositeExtract %int %312 0
%315 = OpIEqual %bool %313 %314
%316 = OpCompositeExtract %int %311 1
%317 = OpCompositeExtract %int %312 1
%318 = OpIEqual %bool %316 %317
%319 = OpLogicalAnd %bool %318 %315
%320 = OpLogicalAnd %bool %319 %310
%321 = OpCompositeExtract %S %300 2
%322 = OpCompositeExtract %S %301 2
%323 = OpCompositeExtract %int %321 0
%324 = OpCompositeExtract %int %322 0
%325 = OpIEqual %bool %323 %324
%326 = OpCompositeExtract %int %321 1
%327 = OpCompositeExtract %int %322 1
%328 = OpIEqual %bool %326 %327
%329 = OpLogicalAnd %bool %328 %325
%330 = OpLogicalAnd %bool %329 %320
OpBranch %299
%299 = OpLabel
%331 = OpPhi %bool %false %265 %330 %298
OpSelectionMerge %336 None
OpBranchConditional %331 %334 %335
%334 = OpLabel
%337 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%339 = OpLoad %v4float %337
OpStore %332 %339
OpBranch %336
%335 = OpLabel
%340 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%341 = OpLoad %v4float %340
OpStore %332 %341
OpBranch %336
%336 = OpLabel
%342 = OpLoad %v4float %332
OpReturnValue %342
OpFunctionEnd
