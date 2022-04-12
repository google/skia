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
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %S 1 Offset 4
OpDecorate %_arr_S_int_3 ArrayStride 16
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
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
%61 = OpConstantComposite %v2float %float_1 %float_0
%62 = OpConstantComposite %v2float %float_0 %float_1
%63 = OpConstantComposite %mat2v2float %61 %62
%64 = OpConstantComposite %v2float %float_2 %float_0
%65 = OpConstantComposite %v2float %float_0 %float_2
%66 = OpConstantComposite %mat2v2float %64 %65
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%69 = OpConstantComposite %v2float %float_3 %float_4
%70 = OpConstantComposite %v2float %float_5 %float_6
%71 = OpConstantComposite %mat2v2float %69 %70
%75 = OpConstantComposite %v2float %float_2 %float_3
%76 = OpConstantComposite %v2float %float_4 %float_5
%77 = OpConstantComposite %mat2v2float %75 %76
%78 = OpConstantComposite %v2float %float_6 %float_0
%79 = OpConstantComposite %v2float %float_0 %float_6
%80 = OpConstantComposite %mat2v2float %78 %79
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
%311 = OpVariable %_ptr_Function_v4float Function
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
%72 = OpCompositeConstruct %_arr_mat2v2float_int_3 %63 %66 %71
OpStore %m1 %72
OpStore %m2 %72
%81 = OpCompositeConstruct %_arr_mat2v2float_int_3 %63 %77 %80
OpStore %m3 %81
%86 = OpCompositeConstruct %S %int_1 %int_2
%87 = OpCompositeConstruct %S %int_3 %int_4
%88 = OpCompositeConstruct %S %int_5 %int_6
%89 = OpCompositeConstruct %_arr_S_int_3 %86 %87 %88
OpStore %s1 %89
%92 = OpCompositeConstruct %S %int_0 %int_0
%93 = OpCompositeConstruct %_arr_S_int_3 %86 %92 %88
OpStore %s2 %93
OpStore %s3 %89
%96 = OpLoad %_arr_float_int_4 %f1
%97 = OpLoad %_arr_float_int_4 %f2
%98 = OpCompositeExtract %float %96 0
%99 = OpCompositeExtract %float %97 0
%100 = OpFOrdEqual %bool %98 %99
%101 = OpCompositeExtract %float %96 1
%102 = OpCompositeExtract %float %97 1
%103 = OpFOrdEqual %bool %101 %102
%104 = OpLogicalAnd %bool %103 %100
%105 = OpCompositeExtract %float %96 2
%106 = OpCompositeExtract %float %97 2
%107 = OpFOrdEqual %bool %105 %106
%108 = OpLogicalAnd %bool %107 %104
%109 = OpCompositeExtract %float %96 3
%110 = OpCompositeExtract %float %97 3
%111 = OpFOrdEqual %bool %109 %110
%112 = OpLogicalAnd %bool %111 %108
OpSelectionMerge %114 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
%115 = OpLoad %_arr_float_int_4 %f1
%116 = OpLoad %_arr_float_int_4 %f3
%117 = OpCompositeExtract %float %115 0
%118 = OpCompositeExtract %float %116 0
%119 = OpFUnordNotEqual %bool %117 %118
%120 = OpCompositeExtract %float %115 1
%121 = OpCompositeExtract %float %116 1
%122 = OpFUnordNotEqual %bool %120 %121
%123 = OpLogicalOr %bool %122 %119
%124 = OpCompositeExtract %float %115 2
%125 = OpCompositeExtract %float %116 2
%126 = OpFUnordNotEqual %bool %124 %125
%127 = OpLogicalOr %bool %126 %123
%128 = OpCompositeExtract %float %115 3
%129 = OpCompositeExtract %float %116 3
%130 = OpFUnordNotEqual %bool %128 %129
%131 = OpLogicalOr %bool %130 %127
OpBranch %114
%114 = OpLabel
%132 = OpPhi %bool %false %25 %131 %113
OpSelectionMerge %134 None
OpBranchConditional %132 %133 %134
%133 = OpLabel
%135 = OpLoad %_arr_v3int_int_2 %v1
%136 = OpLoad %_arr_v3int_int_2 %v2
%137 = OpCompositeExtract %v3int %135 0
%138 = OpCompositeExtract %v3int %136 0
%139 = OpIEqual %v3bool %137 %138
%141 = OpAll %bool %139
%142 = OpCompositeExtract %v3int %135 1
%143 = OpCompositeExtract %v3int %136 1
%144 = OpIEqual %v3bool %142 %143
%145 = OpAll %bool %144
%146 = OpLogicalAnd %bool %145 %141
OpBranch %134
%134 = OpLabel
%147 = OpPhi %bool %false %114 %146 %133
OpSelectionMerge %149 None
OpBranchConditional %147 %148 %149
%148 = OpLabel
%150 = OpLoad %_arr_v3int_int_2 %v1
%151 = OpLoad %_arr_v3int_int_2 %v3
%152 = OpCompositeExtract %v3int %150 0
%153 = OpCompositeExtract %v3int %151 0
%154 = OpINotEqual %v3bool %152 %153
%155 = OpAny %bool %154
%156 = OpCompositeExtract %v3int %150 1
%157 = OpCompositeExtract %v3int %151 1
%158 = OpINotEqual %v3bool %156 %157
%159 = OpAny %bool %158
%160 = OpLogicalOr %bool %159 %155
OpBranch %149
%149 = OpLabel
%161 = OpPhi %bool %false %134 %160 %148
OpSelectionMerge %163 None
OpBranchConditional %161 %162 %163
%162 = OpLabel
%164 = OpLoad %_arr_mat2v2float_int_3 %m1
%165 = OpLoad %_arr_mat2v2float_int_3 %m2
%166 = OpCompositeExtract %mat2v2float %164 0
%167 = OpCompositeExtract %mat2v2float %165 0
%169 = OpCompositeExtract %v2float %166 0
%170 = OpCompositeExtract %v2float %167 0
%171 = OpFOrdEqual %v2bool %169 %170
%172 = OpAll %bool %171
%173 = OpCompositeExtract %v2float %166 1
%174 = OpCompositeExtract %v2float %167 1
%175 = OpFOrdEqual %v2bool %173 %174
%176 = OpAll %bool %175
%177 = OpLogicalAnd %bool %172 %176
%178 = OpCompositeExtract %mat2v2float %164 1
%179 = OpCompositeExtract %mat2v2float %165 1
%180 = OpCompositeExtract %v2float %178 0
%181 = OpCompositeExtract %v2float %179 0
%182 = OpFOrdEqual %v2bool %180 %181
%183 = OpAll %bool %182
%184 = OpCompositeExtract %v2float %178 1
%185 = OpCompositeExtract %v2float %179 1
%186 = OpFOrdEqual %v2bool %184 %185
%187 = OpAll %bool %186
%188 = OpLogicalAnd %bool %183 %187
%189 = OpLogicalAnd %bool %188 %177
%190 = OpCompositeExtract %mat2v2float %164 2
%191 = OpCompositeExtract %mat2v2float %165 2
%192 = OpCompositeExtract %v2float %190 0
%193 = OpCompositeExtract %v2float %191 0
%194 = OpFOrdEqual %v2bool %192 %193
%195 = OpAll %bool %194
%196 = OpCompositeExtract %v2float %190 1
%197 = OpCompositeExtract %v2float %191 1
%198 = OpFOrdEqual %v2bool %196 %197
%199 = OpAll %bool %198
%200 = OpLogicalAnd %bool %195 %199
%201 = OpLogicalAnd %bool %200 %189
OpBranch %163
%163 = OpLabel
%202 = OpPhi %bool %false %149 %201 %162
OpSelectionMerge %204 None
OpBranchConditional %202 %203 %204
%203 = OpLabel
%205 = OpLoad %_arr_mat2v2float_int_3 %m1
%206 = OpLoad %_arr_mat2v2float_int_3 %m3
%207 = OpCompositeExtract %mat2v2float %205 0
%208 = OpCompositeExtract %mat2v2float %206 0
%209 = OpCompositeExtract %v2float %207 0
%210 = OpCompositeExtract %v2float %208 0
%211 = OpFUnordNotEqual %v2bool %209 %210
%212 = OpAny %bool %211
%213 = OpCompositeExtract %v2float %207 1
%214 = OpCompositeExtract %v2float %208 1
%215 = OpFUnordNotEqual %v2bool %213 %214
%216 = OpAny %bool %215
%217 = OpLogicalOr %bool %212 %216
%218 = OpCompositeExtract %mat2v2float %205 1
%219 = OpCompositeExtract %mat2v2float %206 1
%220 = OpCompositeExtract %v2float %218 0
%221 = OpCompositeExtract %v2float %219 0
%222 = OpFUnordNotEqual %v2bool %220 %221
%223 = OpAny %bool %222
%224 = OpCompositeExtract %v2float %218 1
%225 = OpCompositeExtract %v2float %219 1
%226 = OpFUnordNotEqual %v2bool %224 %225
%227 = OpAny %bool %226
%228 = OpLogicalOr %bool %223 %227
%229 = OpLogicalOr %bool %228 %217
%230 = OpCompositeExtract %mat2v2float %205 2
%231 = OpCompositeExtract %mat2v2float %206 2
%232 = OpCompositeExtract %v2float %230 0
%233 = OpCompositeExtract %v2float %231 0
%234 = OpFUnordNotEqual %v2bool %232 %233
%235 = OpAny %bool %234
%236 = OpCompositeExtract %v2float %230 1
%237 = OpCompositeExtract %v2float %231 1
%238 = OpFUnordNotEqual %v2bool %236 %237
%239 = OpAny %bool %238
%240 = OpLogicalOr %bool %235 %239
%241 = OpLogicalOr %bool %240 %229
OpBranch %204
%204 = OpLabel
%242 = OpPhi %bool %false %163 %241 %203
OpSelectionMerge %244 None
OpBranchConditional %242 %243 %244
%243 = OpLabel
%245 = OpLoad %_arr_S_int_3 %s1
%246 = OpLoad %_arr_S_int_3 %s2
%247 = OpCompositeExtract %S %245 0
%248 = OpCompositeExtract %S %246 0
%249 = OpCompositeExtract %int %247 0
%250 = OpCompositeExtract %int %248 0
%251 = OpINotEqual %bool %249 %250
%252 = OpCompositeExtract %int %247 1
%253 = OpCompositeExtract %int %248 1
%254 = OpINotEqual %bool %252 %253
%255 = OpLogicalOr %bool %254 %251
%256 = OpCompositeExtract %S %245 1
%257 = OpCompositeExtract %S %246 1
%258 = OpCompositeExtract %int %256 0
%259 = OpCompositeExtract %int %257 0
%260 = OpINotEqual %bool %258 %259
%261 = OpCompositeExtract %int %256 1
%262 = OpCompositeExtract %int %257 1
%263 = OpINotEqual %bool %261 %262
%264 = OpLogicalOr %bool %263 %260
%265 = OpLogicalOr %bool %264 %255
%266 = OpCompositeExtract %S %245 2
%267 = OpCompositeExtract %S %246 2
%268 = OpCompositeExtract %int %266 0
%269 = OpCompositeExtract %int %267 0
%270 = OpINotEqual %bool %268 %269
%271 = OpCompositeExtract %int %266 1
%272 = OpCompositeExtract %int %267 1
%273 = OpINotEqual %bool %271 %272
%274 = OpLogicalOr %bool %273 %270
%275 = OpLogicalOr %bool %274 %265
OpBranch %244
%244 = OpLabel
%276 = OpPhi %bool %false %204 %275 %243
OpSelectionMerge %278 None
OpBranchConditional %276 %277 %278
%277 = OpLabel
%279 = OpLoad %_arr_S_int_3 %s3
%280 = OpLoad %_arr_S_int_3 %s1
%281 = OpCompositeExtract %S %279 0
%282 = OpCompositeExtract %S %280 0
%283 = OpCompositeExtract %int %281 0
%284 = OpCompositeExtract %int %282 0
%285 = OpIEqual %bool %283 %284
%286 = OpCompositeExtract %int %281 1
%287 = OpCompositeExtract %int %282 1
%288 = OpIEqual %bool %286 %287
%289 = OpLogicalAnd %bool %288 %285
%290 = OpCompositeExtract %S %279 1
%291 = OpCompositeExtract %S %280 1
%292 = OpCompositeExtract %int %290 0
%293 = OpCompositeExtract %int %291 0
%294 = OpIEqual %bool %292 %293
%295 = OpCompositeExtract %int %290 1
%296 = OpCompositeExtract %int %291 1
%297 = OpIEqual %bool %295 %296
%298 = OpLogicalAnd %bool %297 %294
%299 = OpLogicalAnd %bool %298 %289
%300 = OpCompositeExtract %S %279 2
%301 = OpCompositeExtract %S %280 2
%302 = OpCompositeExtract %int %300 0
%303 = OpCompositeExtract %int %301 0
%304 = OpIEqual %bool %302 %303
%305 = OpCompositeExtract %int %300 1
%306 = OpCompositeExtract %int %301 1
%307 = OpIEqual %bool %305 %306
%308 = OpLogicalAnd %bool %307 %304
%309 = OpLogicalAnd %bool %308 %299
OpBranch %278
%278 = OpLabel
%310 = OpPhi %bool %false %244 %309 %277
OpSelectionMerge %315 None
OpBranchConditional %310 %313 %314
%313 = OpLabel
%316 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%318 = OpLoad %v4float %316
OpStore %311 %318
OpBranch %315
%314 = OpLabel
%319 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%320 = OpLoad %v4float %319
OpStore %311 %320
OpBranch %315
%315 = OpLabel
%321 = OpLoad %v4float %311
OpReturnValue %321
OpFunctionEnd
