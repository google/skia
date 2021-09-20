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
OpMemberName %_UniformBuffer 2 "unknownInput"
OpName %_entrypoint_v "_entrypoint_v"
OpName %return_on_both_sides_b "return_on_both_sides_b"
OpName %for_inside_body_b "for_inside_body_b"
OpName %x "x"
OpName %after_for_body_b "after_for_body_b"
OpName %x_0 "x"
OpName %for_with_double_sided_conditional_return_b "for_with_double_sided_conditional_return_b"
OpName %x_1 "x"
OpName %if_else_chain_b "if_else_chain_b"
OpName %conditional_inside_while_loop_b "conditional_inside_while_loop_b"
OpName %inside_do_loop_b "inside_do_loop_b"
OpName %inside_while_loop_b "inside_while_loop_b"
OpName %after_do_loop_b "after_do_loop_b"
OpName %after_while_loop_b "after_while_loop_b"
OpName %switch_with_all_returns_b "switch_with_all_returns_b"
OpName %switch_only_default_b "switch_only_default_b"
OpName %switch_fallthrough_b "switch_fallthrough_b"
OpName %switch_fallthrough_twice_b "switch_fallthrough_twice_b"
OpName %switch_with_break_in_loop_b "switch_with_break_in_loop_b"
OpName %x_2 "x"
OpName %switch_with_continue_in_loop_b "switch_with_continue_in_loop_b"
OpName %x_3 "x"
OpName %loop_with_continue_in_switch_b "loop_with_continue_in_switch_b"
OpName %x_4 "x"
OpName %switch_with_if_that_returns_b "switch_with_if_that_returns_b"
OpName %switch_with_one_sided_if_then_fallthrough_b "switch_with_one_sided_if_then_fallthrough_b"
OpName %main "main"
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
OpDecorate %29 Binding 0
OpDecorate %29 DescriptorSet 0
OpDecorate %48 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%29 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%34 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%38 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%42 = OpTypeFunction %bool
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%true = OpConstantTrue %bool
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%int_1 = OpConstant %int 1
%float_2 = OpConstant %float 2
%false = OpConstantFalse %bool
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_123 = OpConstant %float 123
%267 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %34
%35 = OpLabel
%39 = OpVariable %_ptr_Function_v2float Function
OpStore %39 %38
%41 = OpFunctionCall %v4float %main %39
OpStore %sk_FragColor %41
OpReturn
OpFunctionEnd
%return_on_both_sides_b = OpFunction %bool None %42
%43 = OpLabel
%44 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%48 = OpLoad %float %44
%50 = OpFOrdEqual %bool %48 %float_1
OpSelectionMerge %53 None
OpBranchConditional %50 %51 %52
%51 = OpLabel
OpReturnValue %true
%52 = OpLabel
OpReturnValue %true
%53 = OpLabel
OpUnreachable
OpFunctionEnd
%for_inside_body_b = OpFunction %bool None %42
%55 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_0
OpBranch %59
%59 = OpLabel
OpLoopMerge %63 %62 None
OpBranch %60
%60 = OpLabel
%64 = OpLoad %int %x
%66 = OpSLessThanEqual %bool %64 %int_10
OpBranchConditional %66 %61 %63
%61 = OpLabel
OpReturnValue %true
%62 = OpLabel
%68 = OpLoad %int %x
%69 = OpIAdd %int %68 %int_1
OpStore %x %69
OpBranch %59
%63 = OpLabel
OpUnreachable
OpFunctionEnd
%after_for_body_b = OpFunction %bool None %42
%70 = OpLabel
%x_0 = OpVariable %_ptr_Function_int Function
OpStore %x_0 %int_0
OpBranch %72
%72 = OpLabel
OpLoopMerge %76 %75 None
OpBranch %73
%73 = OpLabel
%77 = OpLoad %int %x_0
%78 = OpSLessThanEqual %bool %77 %int_10
OpBranchConditional %78 %74 %76
%74 = OpLabel
OpBranch %75
%75 = OpLabel
%79 = OpLoad %int %x_0
%80 = OpIAdd %int %79 %int_1
OpStore %x_0 %80
OpBranch %72
%76 = OpLabel
OpReturnValue %true
OpFunctionEnd
%for_with_double_sided_conditional_return_b = OpFunction %bool None %42
%81 = OpLabel
%x_1 = OpVariable %_ptr_Function_int Function
OpStore %x_1 %int_0
OpBranch %83
%83 = OpLabel
OpLoopMerge %87 %86 None
OpBranch %84
%84 = OpLabel
%88 = OpLoad %int %x_1
%89 = OpSLessThanEqual %bool %88 %int_10
OpBranchConditional %89 %85 %87
%85 = OpLabel
%90 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%91 = OpLoad %float %90
%92 = OpFOrdEqual %bool %91 %float_1
OpSelectionMerge %95 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
OpReturnValue %true
%94 = OpLabel
OpReturnValue %true
%95 = OpLabel
OpBranch %86
%86 = OpLabel
%96 = OpLoad %int %x_1
%97 = OpIAdd %int %96 %int_1
OpStore %x_1 %97
OpBranch %83
%87 = OpLabel
OpUnreachable
OpFunctionEnd
%if_else_chain_b = OpFunction %bool None %42
%98 = OpLabel
%99 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%100 = OpLoad %float %99
%101 = OpFOrdEqual %bool %100 %float_1
OpSelectionMerge %104 None
OpBranchConditional %101 %102 %103
%102 = OpLabel
OpReturnValue %true
%103 = OpLabel
%105 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%106 = OpLoad %float %105
%108 = OpFOrdEqual %bool %106 %float_2
OpSelectionMerge %111 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
OpReturnValue %false
%110 = OpLabel
%113 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%114 = OpLoad %float %113
%116 = OpFOrdEqual %bool %114 %float_3
OpSelectionMerge %119 None
OpBranchConditional %116 %117 %118
%117 = OpLabel
OpReturnValue %true
%118 = OpLabel
%120 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%121 = OpLoad %float %120
%123 = OpFOrdEqual %bool %121 %float_4
OpSelectionMerge %126 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
OpReturnValue %false
%125 = OpLabel
OpReturnValue %true
%126 = OpLabel
OpBranch %119
%119 = OpLabel
OpBranch %111
%111 = OpLabel
OpBranch %104
%104 = OpLabel
OpUnreachable
OpFunctionEnd
%conditional_inside_while_loop_b = OpFunction %bool None %42
%127 = OpLabel
OpBranch %128
%128 = OpLabel
OpLoopMerge %132 %131 None
OpBranch %129
%129 = OpLabel
%133 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%134 = OpLoad %float %133
%136 = OpFOrdEqual %bool %134 %float_123
OpBranchConditional %136 %130 %132
%130 = OpLabel
OpReturnValue %true
%131 = OpLabel
OpBranch %128
%132 = OpLabel
OpUnreachable
OpFunctionEnd
%inside_do_loop_b = OpFunction %bool None %42
%137 = OpLabel
OpBranch %138
%138 = OpLabel
OpLoopMerge %142 %141 None
OpBranch %139
%139 = OpLabel
OpReturnValue %true
%140 = OpLabel
OpBranch %141
%141 = OpLabel
OpBranchConditional %true %138 %142
%142 = OpLabel
OpUnreachable
OpFunctionEnd
%inside_while_loop_b = OpFunction %bool None %42
%143 = OpLabel
OpBranch %144
%144 = OpLabel
OpLoopMerge %148 %147 None
OpBranch %145
%145 = OpLabel
OpBranchConditional %true %146 %148
%146 = OpLabel
OpReturnValue %true
%147 = OpLabel
OpBranch %144
%148 = OpLabel
OpUnreachable
OpFunctionEnd
%after_do_loop_b = OpFunction %bool None %42
%149 = OpLabel
OpBranch %150
%150 = OpLabel
OpLoopMerge %154 %153 None
OpBranch %151
%151 = OpLabel
OpBranch %154
%152 = OpLabel
OpBranch %153
%153 = OpLabel
OpBranchConditional %true %150 %154
%154 = OpLabel
OpReturnValue %true
OpFunctionEnd
%after_while_loop_b = OpFunction %bool None %42
%155 = OpLabel
OpBranch %156
%156 = OpLabel
OpLoopMerge %160 %159 None
OpBranch %157
%157 = OpLabel
OpBranchConditional %true %158 %160
%158 = OpLabel
OpBranch %160
%159 = OpLabel
OpBranch %156
%160 = OpLabel
OpReturnValue %true
OpFunctionEnd
%switch_with_all_returns_b = OpFunction %bool None %42
%161 = OpLabel
%162 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%163 = OpLoad %float %162
%164 = OpConvertFToS %int %163
OpSelectionMerge %165 None
OpSwitch %164 %168 1 %166 2 %167
%166 = OpLabel
OpReturnValue %true
%167 = OpLabel
OpReturnValue %true
%168 = OpLabel
OpReturnValue %true
%165 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_only_default_b = OpFunction %bool None %42
%169 = OpLabel
%170 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%171 = OpLoad %float %170
%172 = OpConvertFToS %int %171
OpSelectionMerge %173 None
OpSwitch %172 %174
%174 = OpLabel
OpReturnValue %true
%173 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_fallthrough_b = OpFunction %bool None %42
%175 = OpLabel
%176 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%177 = OpLoad %float %176
%178 = OpConvertFToS %int %177
OpSelectionMerge %179 None
OpSwitch %178 %182 1 %180 2 %181
%180 = OpLabel
OpReturnValue %true
%181 = OpLabel
OpBranch %182
%182 = OpLabel
OpReturnValue %true
%179 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_fallthrough_twice_b = OpFunction %bool None %42
%183 = OpLabel
%184 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%185 = OpLoad %float %184
%186 = OpConvertFToS %int %185
OpSelectionMerge %187 None
OpSwitch %186 %190 1 %188 2 %189
%188 = OpLabel
OpBranch %189
%189 = OpLabel
OpBranch %190
%190 = OpLabel
OpReturnValue %true
%187 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_break_in_loop_b = OpFunction %bool None %42
%191 = OpLabel
%x_2 = OpVariable %_ptr_Function_int Function
%192 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%193 = OpLoad %float %192
%194 = OpConvertFToS %int %193
OpSelectionMerge %195 None
OpSwitch %194 %197 1 %196
%196 = OpLabel
OpStore %x_2 %int_0
OpBranch %199
%199 = OpLabel
OpLoopMerge %203 %202 None
OpBranch %200
%200 = OpLabel
%204 = OpLoad %int %x_2
%205 = OpSLessThanEqual %bool %204 %int_10
OpBranchConditional %205 %201 %203
%201 = OpLabel
OpBranch %203
%202 = OpLabel
%206 = OpLoad %int %x_2
%207 = OpIAdd %int %206 %int_1
OpStore %x_2 %207
OpBranch %199
%203 = OpLabel
OpBranch %197
%197 = OpLabel
OpReturnValue %true
%195 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_continue_in_loop_b = OpFunction %bool None %42
%208 = OpLabel
%x_3 = OpVariable %_ptr_Function_int Function
%209 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%210 = OpLoad %float %209
%211 = OpConvertFToS %int %210
OpSelectionMerge %212 None
OpSwitch %211 %214 1 %213
%213 = OpLabel
OpStore %x_3 %int_0
OpBranch %216
%216 = OpLabel
OpLoopMerge %220 %219 None
OpBranch %217
%217 = OpLabel
%221 = OpLoad %int %x_3
%222 = OpSLessThanEqual %bool %221 %int_10
OpBranchConditional %222 %218 %220
%218 = OpLabel
OpBranch %219
%219 = OpLabel
%223 = OpLoad %int %x_3
%224 = OpIAdd %int %223 %int_1
OpStore %x_3 %224
OpBranch %216
%220 = OpLabel
OpBranch %214
%214 = OpLabel
OpReturnValue %true
%212 = OpLabel
OpUnreachable
OpFunctionEnd
%loop_with_continue_in_switch_b = OpFunction %bool None %42
%225 = OpLabel
%x_4 = OpVariable %_ptr_Function_int Function
OpStore %x_4 %int_0
OpBranch %227
%227 = OpLabel
OpLoopMerge %231 %230 None
OpBranch %228
%228 = OpLabel
%232 = OpLoad %int %x_4
%233 = OpSLessThanEqual %bool %232 %int_10
OpBranchConditional %233 %229 %231
%229 = OpLabel
%234 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%235 = OpLoad %float %234
%236 = OpConvertFToS %int %235
OpSelectionMerge %237 None
OpSwitch %236 %239 1 %238
%238 = OpLabel
OpBranch %230
%239 = OpLabel
OpReturnValue %true
%237 = OpLabel
OpBranch %230
%230 = OpLabel
%240 = OpLoad %int %x_4
%241 = OpIAdd %int %240 %int_1
OpStore %x_4 %241
OpBranch %227
%231 = OpLabel
OpReturnValue %true
OpFunctionEnd
%switch_with_if_that_returns_b = OpFunction %bool None %42
%242 = OpLabel
%243 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%244 = OpLoad %float %243
%245 = OpConvertFToS %int %244
OpSelectionMerge %246 None
OpSwitch %245 %248 1 %247
%247 = OpLabel
%249 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%250 = OpLoad %float %249
%251 = OpFOrdEqual %bool %250 %float_123
OpSelectionMerge %254 None
OpBranchConditional %251 %252 %253
%252 = OpLabel
OpReturnValue %true
%253 = OpLabel
OpReturnValue %true
%254 = OpLabel
OpBranch %248
%248 = OpLabel
OpReturnValue %true
%246 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_one_sided_if_then_fallthrough_b = OpFunction %bool None %42
%255 = OpLabel
%256 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%257 = OpLoad %float %256
%258 = OpConvertFToS %int %257
OpSelectionMerge %259 None
OpSwitch %258 %261 1 %260
%260 = OpLabel
%262 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%263 = OpLoad %float %262
%264 = OpFOrdEqual %bool %263 %float_123
OpSelectionMerge %266 None
OpBranchConditional %264 %265 %266
%265 = OpLabel
OpReturnValue %true
%266 = OpLabel
OpBranch %261
%261 = OpLabel
OpReturnValue %true
%259 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %v4float None %267
%268 = OpFunctionParameter %_ptr_Function_v2float
%269 = OpLabel
%346 = OpVariable %_ptr_Function_v4float Function
OpSelectionMerge %271 None
OpBranchConditional %true %270 %271
%270 = OpLabel
%272 = OpFunctionCall %bool %return_on_both_sides_b
OpBranch %271
%271 = OpLabel
%273 = OpPhi %bool %false %269 %272 %270
OpSelectionMerge %275 None
OpBranchConditional %273 %274 %275
%274 = OpLabel
%276 = OpFunctionCall %bool %for_inside_body_b
OpBranch %275
%275 = OpLabel
%277 = OpPhi %bool %false %271 %276 %274
OpSelectionMerge %279 None
OpBranchConditional %277 %278 %279
%278 = OpLabel
%280 = OpFunctionCall %bool %after_for_body_b
OpBranch %279
%279 = OpLabel
%281 = OpPhi %bool %false %275 %280 %278
OpSelectionMerge %283 None
OpBranchConditional %281 %282 %283
%282 = OpLabel
%284 = OpFunctionCall %bool %for_with_double_sided_conditional_return_b
OpBranch %283
%283 = OpLabel
%285 = OpPhi %bool %false %279 %284 %282
OpSelectionMerge %287 None
OpBranchConditional %285 %286 %287
%286 = OpLabel
%288 = OpFunctionCall %bool %if_else_chain_b
OpBranch %287
%287 = OpLabel
%289 = OpPhi %bool %false %283 %288 %286
OpSelectionMerge %291 None
OpBranchConditional %289 %290 %291
%290 = OpLabel
%292 = OpFunctionCall %bool %conditional_inside_while_loop_b
OpBranch %291
%291 = OpLabel
%293 = OpPhi %bool %false %287 %292 %290
OpSelectionMerge %295 None
OpBranchConditional %293 %294 %295
%294 = OpLabel
%296 = OpFunctionCall %bool %inside_do_loop_b
OpBranch %295
%295 = OpLabel
%297 = OpPhi %bool %false %291 %296 %294
OpSelectionMerge %299 None
OpBranchConditional %297 %298 %299
%298 = OpLabel
%300 = OpFunctionCall %bool %inside_while_loop_b
OpBranch %299
%299 = OpLabel
%301 = OpPhi %bool %false %295 %300 %298
OpSelectionMerge %303 None
OpBranchConditional %301 %302 %303
%302 = OpLabel
%304 = OpFunctionCall %bool %after_do_loop_b
OpBranch %303
%303 = OpLabel
%305 = OpPhi %bool %false %299 %304 %302
OpSelectionMerge %307 None
OpBranchConditional %305 %306 %307
%306 = OpLabel
%308 = OpFunctionCall %bool %after_while_loop_b
OpBranch %307
%307 = OpLabel
%309 = OpPhi %bool %false %303 %308 %306
OpSelectionMerge %311 None
OpBranchConditional %309 %310 %311
%310 = OpLabel
%312 = OpFunctionCall %bool %switch_with_all_returns_b
OpBranch %311
%311 = OpLabel
%313 = OpPhi %bool %false %307 %312 %310
OpSelectionMerge %315 None
OpBranchConditional %313 %314 %315
%314 = OpLabel
%316 = OpFunctionCall %bool %switch_only_default_b
OpBranch %315
%315 = OpLabel
%317 = OpPhi %bool %false %311 %316 %314
OpSelectionMerge %319 None
OpBranchConditional %317 %318 %319
%318 = OpLabel
%320 = OpFunctionCall %bool %switch_fallthrough_b
OpBranch %319
%319 = OpLabel
%321 = OpPhi %bool %false %315 %320 %318
OpSelectionMerge %323 None
OpBranchConditional %321 %322 %323
%322 = OpLabel
%324 = OpFunctionCall %bool %switch_fallthrough_twice_b
OpBranch %323
%323 = OpLabel
%325 = OpPhi %bool %false %319 %324 %322
OpSelectionMerge %327 None
OpBranchConditional %325 %326 %327
%326 = OpLabel
%328 = OpFunctionCall %bool %switch_with_break_in_loop_b
OpBranch %327
%327 = OpLabel
%329 = OpPhi %bool %false %323 %328 %326
OpSelectionMerge %331 None
OpBranchConditional %329 %330 %331
%330 = OpLabel
%332 = OpFunctionCall %bool %loop_with_continue_in_switch_b
OpBranch %331
%331 = OpLabel
%333 = OpPhi %bool %false %327 %332 %330
OpSelectionMerge %335 None
OpBranchConditional %333 %334 %335
%334 = OpLabel
%336 = OpFunctionCall %bool %switch_with_continue_in_loop_b
OpBranch %335
%335 = OpLabel
%337 = OpPhi %bool %false %331 %336 %334
OpSelectionMerge %339 None
OpBranchConditional %337 %338 %339
%338 = OpLabel
%340 = OpFunctionCall %bool %switch_with_if_that_returns_b
OpBranch %339
%339 = OpLabel
%341 = OpPhi %bool %false %335 %340 %338
OpSelectionMerge %343 None
OpBranchConditional %341 %342 %343
%342 = OpLabel
%344 = OpFunctionCall %bool %switch_with_one_sided_if_then_fallthrough_b
OpBranch %343
%343 = OpLabel
%345 = OpPhi %bool %false %339 %344 %342
OpSelectionMerge %350 None
OpBranchConditional %345 %348 %349
%348 = OpLabel
%351 = OpAccessChain %_ptr_Uniform_v4float %29 %int_0
%353 = OpLoad %v4float %351
OpStore %346 %353
OpBranch %350
%349 = OpLabel
%354 = OpAccessChain %_ptr_Uniform_v4float %29 %int_1
%355 = OpLoad %v4float %354
OpStore %346 %355
OpBranch %350
%350 = OpLabel
%356 = OpLoad %v4float %346
OpReturnValue %356
OpFunctionEnd
