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
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %S 1 Offset 4
OpDecorate %_arr_S_int_3 ArrayStride 16
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
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
%64 = OpConstantComposite %v2float %float_1 %float_0
%65 = OpConstantComposite %v2float %float_0 %float_1
%67 = OpConstantComposite %v2float %float_2 %float_0
%68 = OpConstantComposite %v2float %float_0 %float_2
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%71 = OpConstantComposite %v2float %float_3 %float_4
%72 = OpConstantComposite %v2float %float_5 %float_6
%82 = OpConstantComposite %v2float %float_2 %float_3
%83 = OpConstantComposite %v2float %float_4 %float_5
%86 = OpConstantComposite %v2float %float_6 %float_0
%87 = OpConstantComposite %v2float %float_0 %float_6
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
%324 = OpVariable %_ptr_Function_v4float Function
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
%63 = OpCompositeConstruct %mat2v2float %64 %65
%66 = OpCompositeConstruct %mat2v2float %67 %68
%73 = OpCompositeConstruct %mat2v2float %71 %72
%74 = OpCompositeConstruct %_arr_mat2v2float_int_3 %63 %66 %73
OpStore %m1 %74
%76 = OpCompositeConstruct %mat2v2float %64 %65
%77 = OpCompositeConstruct %mat2v2float %67 %68
%78 = OpCompositeConstruct %mat2v2float %71 %72
%79 = OpCompositeConstruct %_arr_mat2v2float_int_3 %76 %77 %78
OpStore %m2 %79
%81 = OpCompositeConstruct %mat2v2float %64 %65
%84 = OpCompositeConstruct %mat2v2float %82 %83
%85 = OpCompositeConstruct %mat2v2float %86 %87
%88 = OpCompositeConstruct %_arr_mat2v2float_int_3 %81 %84 %85
OpStore %m3 %88
%93 = OpCompositeConstruct %S %int_1 %int_2
%94 = OpCompositeConstruct %S %int_3 %int_4
%95 = OpCompositeConstruct %S %int_5 %int_6
%96 = OpCompositeConstruct %_arr_S_int_3 %93 %94 %95
OpStore %s1 %96
%98 = OpCompositeConstruct %S %int_1 %int_2
%100 = OpCompositeConstruct %S %int_0 %int_0
%101 = OpCompositeConstruct %S %int_5 %int_6
%102 = OpCompositeConstruct %_arr_S_int_3 %98 %100 %101
OpStore %s2 %102
%104 = OpCompositeConstruct %S %int_1 %int_2
%105 = OpCompositeConstruct %S %int_3 %int_4
%106 = OpCompositeConstruct %S %int_5 %int_6
%107 = OpCompositeConstruct %_arr_S_int_3 %104 %105 %106
OpStore %s3 %107
%109 = OpLoad %_arr_float_int_4 %f1
%110 = OpLoad %_arr_float_int_4 %f2
%111 = OpCompositeExtract %float %109 0
%112 = OpCompositeExtract %float %110 0
%113 = OpFOrdEqual %bool %111 %112
%114 = OpCompositeExtract %float %109 1
%115 = OpCompositeExtract %float %110 1
%116 = OpFOrdEqual %bool %114 %115
%117 = OpLogicalAnd %bool %116 %113
%118 = OpCompositeExtract %float %109 2
%119 = OpCompositeExtract %float %110 2
%120 = OpFOrdEqual %bool %118 %119
%121 = OpLogicalAnd %bool %120 %117
%122 = OpCompositeExtract %float %109 3
%123 = OpCompositeExtract %float %110 3
%124 = OpFOrdEqual %bool %122 %123
%125 = OpLogicalAnd %bool %124 %121
OpSelectionMerge %127 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%128 = OpLoad %_arr_float_int_4 %f1
%129 = OpLoad %_arr_float_int_4 %f3
%130 = OpCompositeExtract %float %128 0
%131 = OpCompositeExtract %float %129 0
%132 = OpFUnordNotEqual %bool %130 %131
%133 = OpCompositeExtract %float %128 1
%134 = OpCompositeExtract %float %129 1
%135 = OpFUnordNotEqual %bool %133 %134
%136 = OpLogicalOr %bool %135 %132
%137 = OpCompositeExtract %float %128 2
%138 = OpCompositeExtract %float %129 2
%139 = OpFUnordNotEqual %bool %137 %138
%140 = OpLogicalOr %bool %139 %136
%141 = OpCompositeExtract %float %128 3
%142 = OpCompositeExtract %float %129 3
%143 = OpFUnordNotEqual %bool %141 %142
%144 = OpLogicalOr %bool %143 %140
OpBranch %127
%127 = OpLabel
%145 = OpPhi %bool %false %25 %144 %126
OpSelectionMerge %147 None
OpBranchConditional %145 %146 %147
%146 = OpLabel
%148 = OpLoad %_arr_v3int_int_2 %v1
%149 = OpLoad %_arr_v3int_int_2 %v2
%150 = OpCompositeExtract %v3int %148 0
%151 = OpCompositeExtract %v3int %149 0
%152 = OpIEqual %v3bool %150 %151
%154 = OpAll %bool %152
%155 = OpCompositeExtract %v3int %148 1
%156 = OpCompositeExtract %v3int %149 1
%157 = OpIEqual %v3bool %155 %156
%158 = OpAll %bool %157
%159 = OpLogicalAnd %bool %158 %154
OpBranch %147
%147 = OpLabel
%160 = OpPhi %bool %false %127 %159 %146
OpSelectionMerge %162 None
OpBranchConditional %160 %161 %162
%161 = OpLabel
%163 = OpLoad %_arr_v3int_int_2 %v1
%164 = OpLoad %_arr_v3int_int_2 %v3
%165 = OpCompositeExtract %v3int %163 0
%166 = OpCompositeExtract %v3int %164 0
%167 = OpINotEqual %v3bool %165 %166
%168 = OpAny %bool %167
%169 = OpCompositeExtract %v3int %163 1
%170 = OpCompositeExtract %v3int %164 1
%171 = OpINotEqual %v3bool %169 %170
%172 = OpAny %bool %171
%173 = OpLogicalOr %bool %172 %168
OpBranch %162
%162 = OpLabel
%174 = OpPhi %bool %false %147 %173 %161
OpSelectionMerge %176 None
OpBranchConditional %174 %175 %176
%175 = OpLabel
%177 = OpLoad %_arr_mat2v2float_int_3 %m1
%178 = OpLoad %_arr_mat2v2float_int_3 %m2
%179 = OpCompositeExtract %mat2v2float %177 0
%180 = OpCompositeExtract %mat2v2float %178 0
%182 = OpCompositeExtract %v2float %179 0
%183 = OpCompositeExtract %v2float %180 0
%184 = OpFOrdEqual %v2bool %182 %183
%185 = OpAll %bool %184
%186 = OpCompositeExtract %v2float %179 1
%187 = OpCompositeExtract %v2float %180 1
%188 = OpFOrdEqual %v2bool %186 %187
%189 = OpAll %bool %188
%190 = OpLogicalAnd %bool %185 %189
%191 = OpCompositeExtract %mat2v2float %177 1
%192 = OpCompositeExtract %mat2v2float %178 1
%193 = OpCompositeExtract %v2float %191 0
%194 = OpCompositeExtract %v2float %192 0
%195 = OpFOrdEqual %v2bool %193 %194
%196 = OpAll %bool %195
%197 = OpCompositeExtract %v2float %191 1
%198 = OpCompositeExtract %v2float %192 1
%199 = OpFOrdEqual %v2bool %197 %198
%200 = OpAll %bool %199
%201 = OpLogicalAnd %bool %196 %200
%202 = OpLogicalAnd %bool %201 %190
%203 = OpCompositeExtract %mat2v2float %177 2
%204 = OpCompositeExtract %mat2v2float %178 2
%205 = OpCompositeExtract %v2float %203 0
%206 = OpCompositeExtract %v2float %204 0
%207 = OpFOrdEqual %v2bool %205 %206
%208 = OpAll %bool %207
%209 = OpCompositeExtract %v2float %203 1
%210 = OpCompositeExtract %v2float %204 1
%211 = OpFOrdEqual %v2bool %209 %210
%212 = OpAll %bool %211
%213 = OpLogicalAnd %bool %208 %212
%214 = OpLogicalAnd %bool %213 %202
OpBranch %176
%176 = OpLabel
%215 = OpPhi %bool %false %162 %214 %175
OpSelectionMerge %217 None
OpBranchConditional %215 %216 %217
%216 = OpLabel
%218 = OpLoad %_arr_mat2v2float_int_3 %m1
%219 = OpLoad %_arr_mat2v2float_int_3 %m3
%220 = OpCompositeExtract %mat2v2float %218 0
%221 = OpCompositeExtract %mat2v2float %219 0
%222 = OpCompositeExtract %v2float %220 0
%223 = OpCompositeExtract %v2float %221 0
%224 = OpFUnordNotEqual %v2bool %222 %223
%225 = OpAny %bool %224
%226 = OpCompositeExtract %v2float %220 1
%227 = OpCompositeExtract %v2float %221 1
%228 = OpFUnordNotEqual %v2bool %226 %227
%229 = OpAny %bool %228
%230 = OpLogicalOr %bool %225 %229
%231 = OpCompositeExtract %mat2v2float %218 1
%232 = OpCompositeExtract %mat2v2float %219 1
%233 = OpCompositeExtract %v2float %231 0
%234 = OpCompositeExtract %v2float %232 0
%235 = OpFUnordNotEqual %v2bool %233 %234
%236 = OpAny %bool %235
%237 = OpCompositeExtract %v2float %231 1
%238 = OpCompositeExtract %v2float %232 1
%239 = OpFUnordNotEqual %v2bool %237 %238
%240 = OpAny %bool %239
%241 = OpLogicalOr %bool %236 %240
%242 = OpLogicalOr %bool %241 %230
%243 = OpCompositeExtract %mat2v2float %218 2
%244 = OpCompositeExtract %mat2v2float %219 2
%245 = OpCompositeExtract %v2float %243 0
%246 = OpCompositeExtract %v2float %244 0
%247 = OpFUnordNotEqual %v2bool %245 %246
%248 = OpAny %bool %247
%249 = OpCompositeExtract %v2float %243 1
%250 = OpCompositeExtract %v2float %244 1
%251 = OpFUnordNotEqual %v2bool %249 %250
%252 = OpAny %bool %251
%253 = OpLogicalOr %bool %248 %252
%254 = OpLogicalOr %bool %253 %242
OpBranch %217
%217 = OpLabel
%255 = OpPhi %bool %false %176 %254 %216
OpSelectionMerge %257 None
OpBranchConditional %255 %256 %257
%256 = OpLabel
%258 = OpLoad %_arr_S_int_3 %s1
%259 = OpLoad %_arr_S_int_3 %s2
%260 = OpCompositeExtract %S %258 0
%261 = OpCompositeExtract %S %259 0
%262 = OpCompositeExtract %int %260 0
%263 = OpCompositeExtract %int %261 0
%264 = OpINotEqual %bool %262 %263
%265 = OpCompositeExtract %int %260 1
%266 = OpCompositeExtract %int %261 1
%267 = OpINotEqual %bool %265 %266
%268 = OpLogicalOr %bool %267 %264
%269 = OpCompositeExtract %S %258 1
%270 = OpCompositeExtract %S %259 1
%271 = OpCompositeExtract %int %269 0
%272 = OpCompositeExtract %int %270 0
%273 = OpINotEqual %bool %271 %272
%274 = OpCompositeExtract %int %269 1
%275 = OpCompositeExtract %int %270 1
%276 = OpINotEqual %bool %274 %275
%277 = OpLogicalOr %bool %276 %273
%278 = OpLogicalOr %bool %277 %268
%279 = OpCompositeExtract %S %258 2
%280 = OpCompositeExtract %S %259 2
%281 = OpCompositeExtract %int %279 0
%282 = OpCompositeExtract %int %280 0
%283 = OpINotEqual %bool %281 %282
%284 = OpCompositeExtract %int %279 1
%285 = OpCompositeExtract %int %280 1
%286 = OpINotEqual %bool %284 %285
%287 = OpLogicalOr %bool %286 %283
%288 = OpLogicalOr %bool %287 %278
OpBranch %257
%257 = OpLabel
%289 = OpPhi %bool %false %217 %288 %256
OpSelectionMerge %291 None
OpBranchConditional %289 %290 %291
%290 = OpLabel
%292 = OpLoad %_arr_S_int_3 %s3
%293 = OpLoad %_arr_S_int_3 %s1
%294 = OpCompositeExtract %S %292 0
%295 = OpCompositeExtract %S %293 0
%296 = OpCompositeExtract %int %294 0
%297 = OpCompositeExtract %int %295 0
%298 = OpIEqual %bool %296 %297
%299 = OpCompositeExtract %int %294 1
%300 = OpCompositeExtract %int %295 1
%301 = OpIEqual %bool %299 %300
%302 = OpLogicalAnd %bool %301 %298
%303 = OpCompositeExtract %S %292 1
%304 = OpCompositeExtract %S %293 1
%305 = OpCompositeExtract %int %303 0
%306 = OpCompositeExtract %int %304 0
%307 = OpIEqual %bool %305 %306
%308 = OpCompositeExtract %int %303 1
%309 = OpCompositeExtract %int %304 1
%310 = OpIEqual %bool %308 %309
%311 = OpLogicalAnd %bool %310 %307
%312 = OpLogicalAnd %bool %311 %302
%313 = OpCompositeExtract %S %292 2
%314 = OpCompositeExtract %S %293 2
%315 = OpCompositeExtract %int %313 0
%316 = OpCompositeExtract %int %314 0
%317 = OpIEqual %bool %315 %316
%318 = OpCompositeExtract %int %313 1
%319 = OpCompositeExtract %int %314 1
%320 = OpIEqual %bool %318 %319
%321 = OpLogicalAnd %bool %320 %317
%322 = OpLogicalAnd %bool %321 %312
OpBranch %291
%291 = OpLabel
%323 = OpPhi %bool %false %257 %322 %290
OpSelectionMerge %328 None
OpBranchConditional %323 %326 %327
%326 = OpLabel
%329 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%331 = OpLoad %v4float %329
OpStore %324 %331
OpBranch %328
%327 = OpLabel
%332 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%333 = OpLoad %v4float %332
OpStore %324 %333
OpBranch %328
%328 = OpLabel
%334 = OpLoad %v4float %324
OpReturnValue %334
OpFunctionEnd
