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
OpDecorate %61 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %S 1 Offset 4
OpDecorate %_arr_S_int_3 ArrayStride 16
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
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
%47 = OpConstantComposite %v3int %int_1 %int_2 %int_3
%int_5 = OpConstant %int 5
%int_6 = OpConstant %int 6
%50 = OpConstantComposite %v3int %int_4 %int_5 %int_6
%int_n6 = OpConstant %int -6
%55 = OpConstantComposite %v3int %int_4 %int_5 %int_n6
%mat2v2float = OpTypeMatrix %v2float 2
%_arr_mat2v2float_int_3 = OpTypeArray %mat2v2float %int_3
%_ptr_Function__arr_mat2v2float_int_3 = OpTypePointer Function %_arr_mat2v2float_int_3
%62 = OpConstantComposite %v2float %float_1 %float_0
%63 = OpConstantComposite %v2float %float_0 %float_1
%65 = OpConstantComposite %v2float %float_2 %float_0
%66 = OpConstantComposite %v2float %float_0 %float_2
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%69 = OpConstantComposite %v2float %float_3 %float_4
%70 = OpConstantComposite %v2float %float_5 %float_6
%79 = OpConstantComposite %v2float %float_2 %float_3
%80 = OpConstantComposite %v2float %float_4 %float_5
%83 = OpConstantComposite %v2float %float_6 %float_0
%84 = OpConstantComposite %v2float %float_0 %float_6
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
%315 = OpVariable %_ptr_Function_v4float Function
%35 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %f1 %35
OpStore %f2 %35
%39 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_n4
OpStore %f3 %39
%51 = OpCompositeConstruct %_arr_v3int_int_2 %47 %50
OpStore %v1 %51
OpStore %v2 %51
%56 = OpCompositeConstruct %_arr_v3int_int_2 %47 %55
OpStore %v3 %56
%61 = OpCompositeConstruct %mat2v2float %62 %63
%64 = OpCompositeConstruct %mat2v2float %65 %66
%71 = OpCompositeConstruct %mat2v2float %69 %70
%72 = OpCompositeConstruct %_arr_mat2v2float_int_3 %61 %64 %71
OpStore %m1 %72
%74 = OpCompositeConstruct %mat2v2float %62 %63
%75 = OpCompositeConstruct %mat2v2float %65 %66
%76 = OpCompositeConstruct %_arr_mat2v2float_int_3 %74 %75 %71
OpStore %m2 %76
%78 = OpCompositeConstruct %mat2v2float %62 %63
%81 = OpCompositeConstruct %mat2v2float %79 %80
%82 = OpCompositeConstruct %mat2v2float %83 %84
%85 = OpCompositeConstruct %_arr_mat2v2float_int_3 %78 %81 %82
OpStore %m3 %85
%90 = OpCompositeConstruct %S %int_1 %int_2
%91 = OpCompositeConstruct %S %int_3 %int_4
%92 = OpCompositeConstruct %S %int_5 %int_6
%93 = OpCompositeConstruct %_arr_S_int_3 %90 %91 %92
OpStore %s1 %93
%96 = OpCompositeConstruct %S %int_0 %int_0
%97 = OpCompositeConstruct %_arr_S_int_3 %90 %96 %92
OpStore %s2 %97
OpStore %s3 %93
%100 = OpLoad %_arr_float_int_4 %f1
%101 = OpLoad %_arr_float_int_4 %f2
%102 = OpCompositeExtract %float %100 0
%103 = OpCompositeExtract %float %101 0
%104 = OpFOrdEqual %bool %102 %103
%105 = OpCompositeExtract %float %100 1
%106 = OpCompositeExtract %float %101 1
%107 = OpFOrdEqual %bool %105 %106
%108 = OpLogicalAnd %bool %107 %104
%109 = OpCompositeExtract %float %100 2
%110 = OpCompositeExtract %float %101 2
%111 = OpFOrdEqual %bool %109 %110
%112 = OpLogicalAnd %bool %111 %108
%113 = OpCompositeExtract %float %100 3
%114 = OpCompositeExtract %float %101 3
%115 = OpFOrdEqual %bool %113 %114
%116 = OpLogicalAnd %bool %115 %112
OpSelectionMerge %118 None
OpBranchConditional %116 %117 %118
%117 = OpLabel
%119 = OpLoad %_arr_float_int_4 %f1
%120 = OpLoad %_arr_float_int_4 %f3
%121 = OpCompositeExtract %float %119 0
%122 = OpCompositeExtract %float %120 0
%123 = OpFUnordNotEqual %bool %121 %122
%124 = OpCompositeExtract %float %119 1
%125 = OpCompositeExtract %float %120 1
%126 = OpFUnordNotEqual %bool %124 %125
%127 = OpLogicalOr %bool %126 %123
%128 = OpCompositeExtract %float %119 2
%129 = OpCompositeExtract %float %120 2
%130 = OpFUnordNotEqual %bool %128 %129
%131 = OpLogicalOr %bool %130 %127
%132 = OpCompositeExtract %float %119 3
%133 = OpCompositeExtract %float %120 3
%134 = OpFUnordNotEqual %bool %132 %133
%135 = OpLogicalOr %bool %134 %131
OpBranch %118
%118 = OpLabel
%136 = OpPhi %bool %false %25 %135 %117
OpSelectionMerge %138 None
OpBranchConditional %136 %137 %138
%137 = OpLabel
%139 = OpLoad %_arr_v3int_int_2 %v1
%140 = OpLoad %_arr_v3int_int_2 %v2
%141 = OpCompositeExtract %v3int %139 0
%142 = OpCompositeExtract %v3int %140 0
%143 = OpIEqual %v3bool %141 %142
%145 = OpAll %bool %143
%146 = OpCompositeExtract %v3int %139 1
%147 = OpCompositeExtract %v3int %140 1
%148 = OpIEqual %v3bool %146 %147
%149 = OpAll %bool %148
%150 = OpLogicalAnd %bool %149 %145
OpBranch %138
%138 = OpLabel
%151 = OpPhi %bool %false %118 %150 %137
OpSelectionMerge %153 None
OpBranchConditional %151 %152 %153
%152 = OpLabel
%154 = OpLoad %_arr_v3int_int_2 %v1
%155 = OpLoad %_arr_v3int_int_2 %v3
%156 = OpCompositeExtract %v3int %154 0
%157 = OpCompositeExtract %v3int %155 0
%158 = OpINotEqual %v3bool %156 %157
%159 = OpAny %bool %158
%160 = OpCompositeExtract %v3int %154 1
%161 = OpCompositeExtract %v3int %155 1
%162 = OpINotEqual %v3bool %160 %161
%163 = OpAny %bool %162
%164 = OpLogicalOr %bool %163 %159
OpBranch %153
%153 = OpLabel
%165 = OpPhi %bool %false %138 %164 %152
OpSelectionMerge %167 None
OpBranchConditional %165 %166 %167
%166 = OpLabel
%168 = OpLoad %_arr_mat2v2float_int_3 %m1
%169 = OpLoad %_arr_mat2v2float_int_3 %m2
%170 = OpCompositeExtract %mat2v2float %168 0
%171 = OpCompositeExtract %mat2v2float %169 0
%173 = OpCompositeExtract %v2float %170 0
%174 = OpCompositeExtract %v2float %171 0
%175 = OpFOrdEqual %v2bool %173 %174
%176 = OpAll %bool %175
%177 = OpCompositeExtract %v2float %170 1
%178 = OpCompositeExtract %v2float %171 1
%179 = OpFOrdEqual %v2bool %177 %178
%180 = OpAll %bool %179
%181 = OpLogicalAnd %bool %176 %180
%182 = OpCompositeExtract %mat2v2float %168 1
%183 = OpCompositeExtract %mat2v2float %169 1
%184 = OpCompositeExtract %v2float %182 0
%185 = OpCompositeExtract %v2float %183 0
%186 = OpFOrdEqual %v2bool %184 %185
%187 = OpAll %bool %186
%188 = OpCompositeExtract %v2float %182 1
%189 = OpCompositeExtract %v2float %183 1
%190 = OpFOrdEqual %v2bool %188 %189
%191 = OpAll %bool %190
%192 = OpLogicalAnd %bool %187 %191
%193 = OpLogicalAnd %bool %192 %181
%194 = OpCompositeExtract %mat2v2float %168 2
%195 = OpCompositeExtract %mat2v2float %169 2
%196 = OpCompositeExtract %v2float %194 0
%197 = OpCompositeExtract %v2float %195 0
%198 = OpFOrdEqual %v2bool %196 %197
%199 = OpAll %bool %198
%200 = OpCompositeExtract %v2float %194 1
%201 = OpCompositeExtract %v2float %195 1
%202 = OpFOrdEqual %v2bool %200 %201
%203 = OpAll %bool %202
%204 = OpLogicalAnd %bool %199 %203
%205 = OpLogicalAnd %bool %204 %193
OpBranch %167
%167 = OpLabel
%206 = OpPhi %bool %false %153 %205 %166
OpSelectionMerge %208 None
OpBranchConditional %206 %207 %208
%207 = OpLabel
%209 = OpLoad %_arr_mat2v2float_int_3 %m1
%210 = OpLoad %_arr_mat2v2float_int_3 %m3
%211 = OpCompositeExtract %mat2v2float %209 0
%212 = OpCompositeExtract %mat2v2float %210 0
%213 = OpCompositeExtract %v2float %211 0
%214 = OpCompositeExtract %v2float %212 0
%215 = OpFUnordNotEqual %v2bool %213 %214
%216 = OpAny %bool %215
%217 = OpCompositeExtract %v2float %211 1
%218 = OpCompositeExtract %v2float %212 1
%219 = OpFUnordNotEqual %v2bool %217 %218
%220 = OpAny %bool %219
%221 = OpLogicalOr %bool %216 %220
%222 = OpCompositeExtract %mat2v2float %209 1
%223 = OpCompositeExtract %mat2v2float %210 1
%224 = OpCompositeExtract %v2float %222 0
%225 = OpCompositeExtract %v2float %223 0
%226 = OpFUnordNotEqual %v2bool %224 %225
%227 = OpAny %bool %226
%228 = OpCompositeExtract %v2float %222 1
%229 = OpCompositeExtract %v2float %223 1
%230 = OpFUnordNotEqual %v2bool %228 %229
%231 = OpAny %bool %230
%232 = OpLogicalOr %bool %227 %231
%233 = OpLogicalOr %bool %232 %221
%234 = OpCompositeExtract %mat2v2float %209 2
%235 = OpCompositeExtract %mat2v2float %210 2
%236 = OpCompositeExtract %v2float %234 0
%237 = OpCompositeExtract %v2float %235 0
%238 = OpFUnordNotEqual %v2bool %236 %237
%239 = OpAny %bool %238
%240 = OpCompositeExtract %v2float %234 1
%241 = OpCompositeExtract %v2float %235 1
%242 = OpFUnordNotEqual %v2bool %240 %241
%243 = OpAny %bool %242
%244 = OpLogicalOr %bool %239 %243
%245 = OpLogicalOr %bool %244 %233
OpBranch %208
%208 = OpLabel
%246 = OpPhi %bool %false %167 %245 %207
OpSelectionMerge %248 None
OpBranchConditional %246 %247 %248
%247 = OpLabel
%249 = OpLoad %_arr_S_int_3 %s1
%250 = OpLoad %_arr_S_int_3 %s2
%251 = OpCompositeExtract %S %249 0
%252 = OpCompositeExtract %S %250 0
%253 = OpCompositeExtract %int %251 0
%254 = OpCompositeExtract %int %252 0
%255 = OpINotEqual %bool %253 %254
%256 = OpCompositeExtract %int %251 1
%257 = OpCompositeExtract %int %252 1
%258 = OpINotEqual %bool %256 %257
%259 = OpLogicalOr %bool %258 %255
%260 = OpCompositeExtract %S %249 1
%261 = OpCompositeExtract %S %250 1
%262 = OpCompositeExtract %int %260 0
%263 = OpCompositeExtract %int %261 0
%264 = OpINotEqual %bool %262 %263
%265 = OpCompositeExtract %int %260 1
%266 = OpCompositeExtract %int %261 1
%267 = OpINotEqual %bool %265 %266
%268 = OpLogicalOr %bool %267 %264
%269 = OpLogicalOr %bool %268 %259
%270 = OpCompositeExtract %S %249 2
%271 = OpCompositeExtract %S %250 2
%272 = OpCompositeExtract %int %270 0
%273 = OpCompositeExtract %int %271 0
%274 = OpINotEqual %bool %272 %273
%275 = OpCompositeExtract %int %270 1
%276 = OpCompositeExtract %int %271 1
%277 = OpINotEqual %bool %275 %276
%278 = OpLogicalOr %bool %277 %274
%279 = OpLogicalOr %bool %278 %269
OpBranch %248
%248 = OpLabel
%280 = OpPhi %bool %false %208 %279 %247
OpSelectionMerge %282 None
OpBranchConditional %280 %281 %282
%281 = OpLabel
%283 = OpLoad %_arr_S_int_3 %s3
%284 = OpLoad %_arr_S_int_3 %s1
%285 = OpCompositeExtract %S %283 0
%286 = OpCompositeExtract %S %284 0
%287 = OpCompositeExtract %int %285 0
%288 = OpCompositeExtract %int %286 0
%289 = OpIEqual %bool %287 %288
%290 = OpCompositeExtract %int %285 1
%291 = OpCompositeExtract %int %286 1
%292 = OpIEqual %bool %290 %291
%293 = OpLogicalAnd %bool %292 %289
%294 = OpCompositeExtract %S %283 1
%295 = OpCompositeExtract %S %284 1
%296 = OpCompositeExtract %int %294 0
%297 = OpCompositeExtract %int %295 0
%298 = OpIEqual %bool %296 %297
%299 = OpCompositeExtract %int %294 1
%300 = OpCompositeExtract %int %295 1
%301 = OpIEqual %bool %299 %300
%302 = OpLogicalAnd %bool %301 %298
%303 = OpLogicalAnd %bool %302 %293
%304 = OpCompositeExtract %S %283 2
%305 = OpCompositeExtract %S %284 2
%306 = OpCompositeExtract %int %304 0
%307 = OpCompositeExtract %int %305 0
%308 = OpIEqual %bool %306 %307
%309 = OpCompositeExtract %int %304 1
%310 = OpCompositeExtract %int %305 1
%311 = OpIEqual %bool %309 %310
%312 = OpLogicalAnd %bool %311 %308
%313 = OpLogicalAnd %bool %312 %303
OpBranch %282
%282 = OpLabel
%314 = OpPhi %bool %false %248 %313 %281
OpSelectionMerge %319 None
OpBranchConditional %314 %317 %318
%317 = OpLabel
%320 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%322 = OpLoad %v4float %320
OpStore %315 %322
OpBranch %319
%318 = OpLabel
%323 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%324 = OpLoad %v4float %323
OpStore %315 %324
OpBranch %319
%319 = OpLabel
%325 = OpLoad %v4float %315
OpReturnValue %325
OpFunctionEnd
