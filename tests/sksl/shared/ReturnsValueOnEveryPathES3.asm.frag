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
OpDecorate %28 Binding 0
OpDecorate %28 DescriptorSet 0
OpDecorate %47 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
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
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%28 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%33 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%37 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%41 = OpTypeFunction %bool
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
%249 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %33
%34 = OpLabel
%38 = OpVariable %_ptr_Function_v2float Function
OpStore %38 %37
%40 = OpFunctionCall %v4float %main %38
OpStore %sk_FragColor %40
OpReturn
OpFunctionEnd
%return_on_both_sides_b = OpFunction %bool None %41
%42 = OpLabel
%43 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%47 = OpLoad %float %43
%49 = OpFOrdEqual %bool %47 %float_1
OpSelectionMerge %52 None
OpBranchConditional %49 %50 %51
%50 = OpLabel
OpReturnValue %true
%51 = OpLabel
OpReturnValue %true
%52 = OpLabel
OpUnreachable
OpFunctionEnd
%for_inside_body_b = OpFunction %bool None %41
%54 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_0
OpBranch %58
%58 = OpLabel
OpLoopMerge %62 %61 None
OpBranch %59
%59 = OpLabel
%63 = OpLoad %int %x
%65 = OpSLessThanEqual %bool %63 %int_10
OpBranchConditional %65 %60 %62
%60 = OpLabel
OpReturnValue %true
%61 = OpLabel
%67 = OpLoad %int %x
%68 = OpIAdd %int %67 %int_1
OpStore %x %68
OpBranch %58
%62 = OpLabel
OpUnreachable
OpFunctionEnd
%after_for_body_b = OpFunction %bool None %41
%69 = OpLabel
%x_0 = OpVariable %_ptr_Function_int Function
OpStore %x_0 %int_0
OpBranch %71
%71 = OpLabel
OpLoopMerge %75 %74 None
OpBranch %72
%72 = OpLabel
%76 = OpLoad %int %x_0
%77 = OpSLessThanEqual %bool %76 %int_10
OpBranchConditional %77 %73 %75
%73 = OpLabel
OpBranch %74
%74 = OpLabel
%78 = OpLoad %int %x_0
%79 = OpIAdd %int %78 %int_1
OpStore %x_0 %79
OpBranch %71
%75 = OpLabel
OpReturnValue %true
OpFunctionEnd
%for_with_double_sided_conditional_return_b = OpFunction %bool None %41
%80 = OpLabel
%x_1 = OpVariable %_ptr_Function_int Function
OpStore %x_1 %int_0
OpBranch %82
%82 = OpLabel
OpLoopMerge %86 %85 None
OpBranch %83
%83 = OpLabel
%87 = OpLoad %int %x_1
%88 = OpSLessThanEqual %bool %87 %int_10
OpBranchConditional %88 %84 %86
%84 = OpLabel
%89 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%90 = OpLoad %float %89
%91 = OpFOrdEqual %bool %90 %float_1
OpSelectionMerge %94 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
OpReturnValue %true
%93 = OpLabel
OpReturnValue %true
%94 = OpLabel
OpBranch %85
%85 = OpLabel
%95 = OpLoad %int %x_1
%96 = OpIAdd %int %95 %int_1
OpStore %x_1 %96
OpBranch %82
%86 = OpLabel
OpUnreachable
OpFunctionEnd
%if_else_chain_b = OpFunction %bool None %41
%97 = OpLabel
%98 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%99 = OpLoad %float %98
%100 = OpFOrdEqual %bool %99 %float_1
OpSelectionMerge %103 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
OpReturnValue %true
%102 = OpLabel
%104 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%105 = OpLoad %float %104
%107 = OpFOrdEqual %bool %105 %float_2
OpSelectionMerge %110 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
OpReturnValue %false
%109 = OpLabel
%112 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%113 = OpLoad %float %112
%115 = OpFOrdEqual %bool %113 %float_3
OpSelectionMerge %118 None
OpBranchConditional %115 %116 %117
%116 = OpLabel
OpReturnValue %true
%117 = OpLabel
%119 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%120 = OpLoad %float %119
%122 = OpFOrdEqual %bool %120 %float_4
OpSelectionMerge %125 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
OpReturnValue %false
%124 = OpLabel
OpReturnValue %true
%125 = OpLabel
OpBranch %118
%118 = OpLabel
OpBranch %110
%110 = OpLabel
OpBranch %103
%103 = OpLabel
OpUnreachable
OpFunctionEnd
%conditional_inside_while_loop_b = OpFunction %bool None %41
%126 = OpLabel
OpBranch %127
%127 = OpLabel
OpLoopMerge %131 %130 None
OpBranch %128
%128 = OpLabel
%132 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%133 = OpLoad %float %132
%135 = OpFOrdEqual %bool %133 %float_123
OpBranchConditional %135 %129 %131
%129 = OpLabel
OpReturnValue %true
%130 = OpLabel
OpBranch %127
%131 = OpLabel
OpUnreachable
OpFunctionEnd
%inside_do_loop_b = OpFunction %bool None %41
%136 = OpLabel
OpBranch %137
%137 = OpLabel
OpLoopMerge %141 %140 None
OpBranch %138
%138 = OpLabel
OpReturnValue %true
%139 = OpLabel
OpBranch %140
%140 = OpLabel
OpBranchConditional %true %137 %141
%141 = OpLabel
OpUnreachable
OpFunctionEnd
%inside_while_loop_b = OpFunction %bool None %41
%142 = OpLabel
OpBranch %143
%143 = OpLabel
OpLoopMerge %147 %146 None
OpBranch %144
%144 = OpLabel
OpBranchConditional %true %145 %147
%145 = OpLabel
OpReturnValue %true
%146 = OpLabel
OpBranch %143
%147 = OpLabel
OpUnreachable
OpFunctionEnd
%after_do_loop_b = OpFunction %bool None %41
%148 = OpLabel
OpBranch %149
%149 = OpLabel
OpLoopMerge %153 %152 None
OpBranch %150
%150 = OpLabel
OpBranch %153
%151 = OpLabel
OpBranch %152
%152 = OpLabel
OpBranchConditional %true %149 %153
%153 = OpLabel
OpReturnValue %true
OpFunctionEnd
%after_while_loop_b = OpFunction %bool None %41
%154 = OpLabel
OpBranch %155
%155 = OpLabel
OpLoopMerge %159 %158 None
OpBranch %156
%156 = OpLabel
OpBranchConditional %true %157 %159
%157 = OpLabel
OpBranch %159
%158 = OpLabel
OpBranch %155
%159 = OpLabel
OpReturnValue %true
OpFunctionEnd
%switch_with_all_returns_b = OpFunction %bool None %41
%160 = OpLabel
%161 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%162 = OpLoad %float %161
%163 = OpConvertFToS %int %162
OpSelectionMerge %164 None
OpSwitch %163 %167 1 %165 2 %166
%165 = OpLabel
OpReturnValue %true
%166 = OpLabel
OpReturnValue %true
%167 = OpLabel
OpReturnValue %true
%164 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_only_default_b = OpFunction %bool None %41
%168 = OpLabel
%169 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%170 = OpLoad %float %169
%171 = OpConvertFToS %int %170
OpSelectionMerge %172 None
OpSwitch %171 %173
%173 = OpLabel
OpReturnValue %true
%172 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_fallthrough_b = OpFunction %bool None %41
%174 = OpLabel
%175 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%176 = OpLoad %float %175
%177 = OpConvertFToS %int %176
OpSelectionMerge %178 None
OpSwitch %177 %181 1 %179 2 %180
%179 = OpLabel
OpReturnValue %true
%180 = OpLabel
OpBranch %181
%181 = OpLabel
OpReturnValue %true
%178 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_fallthrough_twice_b = OpFunction %bool None %41
%182 = OpLabel
%183 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%184 = OpLoad %float %183
%185 = OpConvertFToS %int %184
OpSelectionMerge %186 None
OpSwitch %185 %189 1 %187 2 %188
%187 = OpLabel
OpBranch %188
%188 = OpLabel
OpBranch %189
%189 = OpLabel
OpReturnValue %true
%186 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_break_in_loop_b = OpFunction %bool None %41
%190 = OpLabel
%x_2 = OpVariable %_ptr_Function_int Function
%191 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%192 = OpLoad %float %191
%193 = OpConvertFToS %int %192
OpSelectionMerge %194 None
OpSwitch %193 %196 1 %195
%195 = OpLabel
OpStore %x_2 %int_0
OpBranch %198
%198 = OpLabel
OpLoopMerge %202 %201 None
OpBranch %199
%199 = OpLabel
%203 = OpLoad %int %x_2
%204 = OpSLessThanEqual %bool %203 %int_10
OpBranchConditional %204 %200 %202
%200 = OpLabel
OpBranch %202
%201 = OpLabel
%205 = OpLoad %int %x_2
%206 = OpIAdd %int %205 %int_1
OpStore %x_2 %206
OpBranch %198
%202 = OpLabel
OpBranch %196
%196 = OpLabel
OpReturnValue %true
%194 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_continue_in_loop_b = OpFunction %bool None %41
%207 = OpLabel
%x_3 = OpVariable %_ptr_Function_int Function
%208 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%209 = OpLoad %float %208
%210 = OpConvertFToS %int %209
OpSelectionMerge %211 None
OpSwitch %210 %213 1 %212
%212 = OpLabel
OpStore %x_3 %int_0
OpBranch %215
%215 = OpLabel
OpLoopMerge %219 %218 None
OpBranch %216
%216 = OpLabel
%220 = OpLoad %int %x_3
%221 = OpSLessThanEqual %bool %220 %int_10
OpBranchConditional %221 %217 %219
%217 = OpLabel
OpBranch %218
%218 = OpLabel
%222 = OpLoad %int %x_3
%223 = OpIAdd %int %222 %int_1
OpStore %x_3 %223
OpBranch %215
%219 = OpLabel
OpBranch %213
%213 = OpLabel
OpReturnValue %true
%211 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_if_that_returns_b = OpFunction %bool None %41
%224 = OpLabel
%225 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%226 = OpLoad %float %225
%227 = OpConvertFToS %int %226
OpSelectionMerge %228 None
OpSwitch %227 %230 1 %229
%229 = OpLabel
%231 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%232 = OpLoad %float %231
%233 = OpFOrdEqual %bool %232 %float_123
OpSelectionMerge %236 None
OpBranchConditional %233 %234 %235
%234 = OpLabel
OpReturnValue %true
%235 = OpLabel
OpReturnValue %true
%236 = OpLabel
OpBranch %230
%230 = OpLabel
OpReturnValue %true
%228 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_one_sided_if_then_fallthrough_b = OpFunction %bool None %41
%237 = OpLabel
%238 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%239 = OpLoad %float %238
%240 = OpConvertFToS %int %239
OpSelectionMerge %241 None
OpSwitch %240 %243 1 %242
%242 = OpLabel
%244 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%245 = OpLoad %float %244
%246 = OpFOrdEqual %bool %245 %float_123
OpSelectionMerge %248 None
OpBranchConditional %246 %247 %248
%247 = OpLabel
OpReturnValue %true
%248 = OpLabel
OpBranch %243
%243 = OpLabel
OpReturnValue %true
%241 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %v4float None %249
%250 = OpFunctionParameter %_ptr_Function_v2float
%251 = OpLabel
%324 = OpVariable %_ptr_Function_v4float Function
OpSelectionMerge %253 None
OpBranchConditional %true %252 %253
%252 = OpLabel
%254 = OpFunctionCall %bool %return_on_both_sides_b
OpBranch %253
%253 = OpLabel
%255 = OpPhi %bool %false %251 %254 %252
OpSelectionMerge %257 None
OpBranchConditional %255 %256 %257
%256 = OpLabel
%258 = OpFunctionCall %bool %for_inside_body_b
OpBranch %257
%257 = OpLabel
%259 = OpPhi %bool %false %253 %258 %256
OpSelectionMerge %261 None
OpBranchConditional %259 %260 %261
%260 = OpLabel
%262 = OpFunctionCall %bool %after_for_body_b
OpBranch %261
%261 = OpLabel
%263 = OpPhi %bool %false %257 %262 %260
OpSelectionMerge %265 None
OpBranchConditional %263 %264 %265
%264 = OpLabel
%266 = OpFunctionCall %bool %for_with_double_sided_conditional_return_b
OpBranch %265
%265 = OpLabel
%267 = OpPhi %bool %false %261 %266 %264
OpSelectionMerge %269 None
OpBranchConditional %267 %268 %269
%268 = OpLabel
%270 = OpFunctionCall %bool %if_else_chain_b
OpBranch %269
%269 = OpLabel
%271 = OpPhi %bool %false %265 %270 %268
OpSelectionMerge %273 None
OpBranchConditional %271 %272 %273
%272 = OpLabel
%274 = OpFunctionCall %bool %conditional_inside_while_loop_b
OpBranch %273
%273 = OpLabel
%275 = OpPhi %bool %false %269 %274 %272
OpSelectionMerge %277 None
OpBranchConditional %275 %276 %277
%276 = OpLabel
%278 = OpFunctionCall %bool %inside_do_loop_b
OpBranch %277
%277 = OpLabel
%279 = OpPhi %bool %false %273 %278 %276
OpSelectionMerge %281 None
OpBranchConditional %279 %280 %281
%280 = OpLabel
%282 = OpFunctionCall %bool %inside_while_loop_b
OpBranch %281
%281 = OpLabel
%283 = OpPhi %bool %false %277 %282 %280
OpSelectionMerge %285 None
OpBranchConditional %283 %284 %285
%284 = OpLabel
%286 = OpFunctionCall %bool %after_do_loop_b
OpBranch %285
%285 = OpLabel
%287 = OpPhi %bool %false %281 %286 %284
OpSelectionMerge %289 None
OpBranchConditional %287 %288 %289
%288 = OpLabel
%290 = OpFunctionCall %bool %after_while_loop_b
OpBranch %289
%289 = OpLabel
%291 = OpPhi %bool %false %285 %290 %288
OpSelectionMerge %293 None
OpBranchConditional %291 %292 %293
%292 = OpLabel
%294 = OpFunctionCall %bool %switch_with_all_returns_b
OpBranch %293
%293 = OpLabel
%295 = OpPhi %bool %false %289 %294 %292
OpSelectionMerge %297 None
OpBranchConditional %295 %296 %297
%296 = OpLabel
%298 = OpFunctionCall %bool %switch_only_default_b
OpBranch %297
%297 = OpLabel
%299 = OpPhi %bool %false %293 %298 %296
OpSelectionMerge %301 None
OpBranchConditional %299 %300 %301
%300 = OpLabel
%302 = OpFunctionCall %bool %switch_fallthrough_b
OpBranch %301
%301 = OpLabel
%303 = OpPhi %bool %false %297 %302 %300
OpSelectionMerge %305 None
OpBranchConditional %303 %304 %305
%304 = OpLabel
%306 = OpFunctionCall %bool %switch_fallthrough_twice_b
OpBranch %305
%305 = OpLabel
%307 = OpPhi %bool %false %301 %306 %304
OpSelectionMerge %309 None
OpBranchConditional %307 %308 %309
%308 = OpLabel
%310 = OpFunctionCall %bool %switch_with_break_in_loop_b
OpBranch %309
%309 = OpLabel
%311 = OpPhi %bool %false %305 %310 %308
OpSelectionMerge %313 None
OpBranchConditional %311 %312 %313
%312 = OpLabel
%314 = OpFunctionCall %bool %switch_with_continue_in_loop_b
OpBranch %313
%313 = OpLabel
%315 = OpPhi %bool %false %309 %314 %312
OpSelectionMerge %317 None
OpBranchConditional %315 %316 %317
%316 = OpLabel
%318 = OpFunctionCall %bool %switch_with_if_that_returns_b
OpBranch %317
%317 = OpLabel
%319 = OpPhi %bool %false %313 %318 %316
OpSelectionMerge %321 None
OpBranchConditional %319 %320 %321
%320 = OpLabel
%322 = OpFunctionCall %bool %switch_with_one_sided_if_then_fallthrough_b
OpBranch %321
%321 = OpLabel
%323 = OpPhi %bool %false %317 %322 %320
OpSelectionMerge %328 None
OpBranchConditional %323 %326 %327
%326 = OpLabel
%329 = OpAccessChain %_ptr_Uniform_v4float %28 %int_0
%331 = OpLoad %v4float %329
OpStore %324 %331
OpBranch %328
%327 = OpLabel
%332 = OpAccessChain %_ptr_Uniform_v4float %28 %int_1
%333 = OpLoad %v4float %332
OpStore %324 %333
OpBranch %328
%328 = OpLabel
%334 = OpLoad %v4float %324
OpReturnValue %334
OpFunctionEnd
