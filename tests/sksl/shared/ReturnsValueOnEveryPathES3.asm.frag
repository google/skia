OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "unknownInput"
OpName %_entrypoint "_entrypoint"
OpName %return_on_both_sides "return_on_both_sides"
OpName %for_inside_body "for_inside_body"
OpName %x "x"
OpName %after_for_body "after_for_body"
OpName %x_0 "x"
OpName %for_with_double_sided_conditional_return "for_with_double_sided_conditional_return"
OpName %x_1 "x"
OpName %if_else_chain "if_else_chain"
OpName %conditional_inside_while_loop "conditional_inside_while_loop"
OpName %inside_do_loop "inside_do_loop"
OpName %inside_while_loop "inside_while_loop"
OpName %after_do_loop "after_do_loop"
OpName %after_while_loop "after_while_loop"
OpName %switch_with_all_returns "switch_with_all_returns"
OpName %switch_only_default "switch_only_default"
OpName %switch_fallthrough "switch_fallthrough"
OpName %switch_fallthrough_twice "switch_fallthrough_twice"
OpName %switch_with_break_in_loop "switch_with_break_in_loop"
OpName %x_2 "x"
OpName %switch_with_continue_in_loop "switch_with_continue_in_loop"
OpName %x_3 "x"
OpName %switch_with_if_that_returns "switch_with_if_that_returns"
OpName %switch_with_one_sided_if_then_fallthrough "switch_with_one_sided_if_then_fallthrough"
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
OpDecorate %42 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
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
%36 = OpTypeFunction %bool
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
%244 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint = OpFunction %void None %33
%34 = OpLabel
%35 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %35
OpReturn
OpFunctionEnd
%return_on_both_sides = OpFunction %bool None %36
%37 = OpLabel
%38 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%42 = OpLoad %float %38
%44 = OpFOrdEqual %bool %42 %float_1
OpSelectionMerge %47 None
OpBranchConditional %44 %45 %46
%45 = OpLabel
OpReturnValue %true
%46 = OpLabel
OpReturnValue %true
%47 = OpLabel
OpUnreachable
OpFunctionEnd
%for_inside_body = OpFunction %bool None %36
%49 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_0
OpBranch %53
%53 = OpLabel
OpLoopMerge %57 %56 None
OpBranch %54
%54 = OpLabel
%58 = OpLoad %int %x
%60 = OpSLessThanEqual %bool %58 %int_10
OpBranchConditional %60 %55 %57
%55 = OpLabel
OpReturnValue %true
%56 = OpLabel
%62 = OpLoad %int %x
%63 = OpIAdd %int %62 %int_1
OpStore %x %63
OpBranch %53
%57 = OpLabel
OpUnreachable
OpFunctionEnd
%after_for_body = OpFunction %bool None %36
%64 = OpLabel
%x_0 = OpVariable %_ptr_Function_int Function
OpStore %x_0 %int_0
OpBranch %66
%66 = OpLabel
OpLoopMerge %70 %69 None
OpBranch %67
%67 = OpLabel
%71 = OpLoad %int %x_0
%72 = OpSLessThanEqual %bool %71 %int_10
OpBranchConditional %72 %68 %70
%68 = OpLabel
OpBranch %69
%69 = OpLabel
%73 = OpLoad %int %x_0
%74 = OpIAdd %int %73 %int_1
OpStore %x_0 %74
OpBranch %66
%70 = OpLabel
OpReturnValue %true
OpFunctionEnd
%for_with_double_sided_conditional_return = OpFunction %bool None %36
%75 = OpLabel
%x_1 = OpVariable %_ptr_Function_int Function
OpStore %x_1 %int_0
OpBranch %77
%77 = OpLabel
OpLoopMerge %81 %80 None
OpBranch %78
%78 = OpLabel
%82 = OpLoad %int %x_1
%83 = OpSLessThanEqual %bool %82 %int_10
OpBranchConditional %83 %79 %81
%79 = OpLabel
%84 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%85 = OpLoad %float %84
%86 = OpFOrdEqual %bool %85 %float_1
OpSelectionMerge %89 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
OpReturnValue %true
%88 = OpLabel
OpReturnValue %true
%89 = OpLabel
OpBranch %80
%80 = OpLabel
%90 = OpLoad %int %x_1
%91 = OpIAdd %int %90 %int_1
OpStore %x_1 %91
OpBranch %77
%81 = OpLabel
OpUnreachable
OpFunctionEnd
%if_else_chain = OpFunction %bool None %36
%92 = OpLabel
%93 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%94 = OpLoad %float %93
%95 = OpFOrdEqual %bool %94 %float_1
OpSelectionMerge %98 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
OpReturnValue %true
%97 = OpLabel
%99 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%100 = OpLoad %float %99
%102 = OpFOrdEqual %bool %100 %float_2
OpSelectionMerge %105 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
OpReturnValue %false
%104 = OpLabel
%107 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%108 = OpLoad %float %107
%110 = OpFOrdEqual %bool %108 %float_3
OpSelectionMerge %113 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
OpReturnValue %true
%112 = OpLabel
%114 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%115 = OpLoad %float %114
%117 = OpFOrdEqual %bool %115 %float_4
OpSelectionMerge %120 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
OpReturnValue %false
%119 = OpLabel
OpReturnValue %true
%120 = OpLabel
OpBranch %113
%113 = OpLabel
OpBranch %105
%105 = OpLabel
OpBranch %98
%98 = OpLabel
OpUnreachable
OpFunctionEnd
%conditional_inside_while_loop = OpFunction %bool None %36
%121 = OpLabel
OpBranch %122
%122 = OpLabel
OpLoopMerge %126 %125 None
OpBranch %123
%123 = OpLabel
%127 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%128 = OpLoad %float %127
%130 = OpFOrdEqual %bool %128 %float_123
OpBranchConditional %130 %124 %126
%124 = OpLabel
OpReturnValue %true
%125 = OpLabel
OpBranch %122
%126 = OpLabel
OpUnreachable
OpFunctionEnd
%inside_do_loop = OpFunction %bool None %36
%131 = OpLabel
OpBranch %132
%132 = OpLabel
OpLoopMerge %136 %135 None
OpBranch %133
%133 = OpLabel
OpReturnValue %true
%134 = OpLabel
OpBranchConditional %true %135 %136
%135 = OpLabel
OpBranch %132
%136 = OpLabel
OpUnreachable
OpFunctionEnd
%inside_while_loop = OpFunction %bool None %36
%137 = OpLabel
OpBranch %138
%138 = OpLabel
OpLoopMerge %142 %141 None
OpBranch %139
%139 = OpLabel
OpBranchConditional %true %140 %142
%140 = OpLabel
OpReturnValue %true
%141 = OpLabel
OpBranch %138
%142 = OpLabel
OpUnreachable
OpFunctionEnd
%after_do_loop = OpFunction %bool None %36
%143 = OpLabel
OpBranch %144
%144 = OpLabel
OpLoopMerge %148 %147 None
OpBranch %145
%145 = OpLabel
OpBranch %148
%146 = OpLabel
OpBranchConditional %true %147 %148
%147 = OpLabel
OpBranch %144
%148 = OpLabel
OpReturnValue %true
OpFunctionEnd
%after_while_loop = OpFunction %bool None %36
%149 = OpLabel
OpBranch %150
%150 = OpLabel
OpLoopMerge %154 %153 None
OpBranch %151
%151 = OpLabel
OpBranchConditional %true %152 %154
%152 = OpLabel
OpBranch %154
%153 = OpLabel
OpBranch %150
%154 = OpLabel
OpReturnValue %true
OpFunctionEnd
%switch_with_all_returns = OpFunction %bool None %36
%155 = OpLabel
%156 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%157 = OpLoad %float %156
%158 = OpConvertFToS %int %157
OpSelectionMerge %159 None
OpSwitch %158 %162 1 %160 2 %161
%160 = OpLabel
OpReturnValue %true
%161 = OpLabel
OpReturnValue %true
%162 = OpLabel
OpReturnValue %true
%159 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_only_default = OpFunction %bool None %36
%163 = OpLabel
%164 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%165 = OpLoad %float %164
%166 = OpConvertFToS %int %165
OpSelectionMerge %167 None
OpSwitch %166 %168
%168 = OpLabel
OpReturnValue %true
%167 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_fallthrough = OpFunction %bool None %36
%169 = OpLabel
%170 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%171 = OpLoad %float %170
%172 = OpConvertFToS %int %171
OpSelectionMerge %173 None
OpSwitch %172 %176 1 %174 2 %175
%174 = OpLabel
OpReturnValue %true
%175 = OpLabel
OpBranch %176
%176 = OpLabel
OpReturnValue %true
%173 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_fallthrough_twice = OpFunction %bool None %36
%177 = OpLabel
%178 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%179 = OpLoad %float %178
%180 = OpConvertFToS %int %179
OpSelectionMerge %181 None
OpSwitch %180 %184 1 %182 2 %183
%182 = OpLabel
OpBranch %183
%183 = OpLabel
OpBranch %184
%184 = OpLabel
OpReturnValue %true
%181 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_break_in_loop = OpFunction %bool None %36
%185 = OpLabel
%x_2 = OpVariable %_ptr_Function_int Function
%186 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%187 = OpLoad %float %186
%188 = OpConvertFToS %int %187
OpSelectionMerge %189 None
OpSwitch %188 %191 1 %190
%190 = OpLabel
OpStore %x_2 %int_0
OpBranch %193
%193 = OpLabel
OpLoopMerge %197 %196 None
OpBranch %194
%194 = OpLabel
%198 = OpLoad %int %x_2
%199 = OpSLessThanEqual %bool %198 %int_10
OpBranchConditional %199 %195 %197
%195 = OpLabel
OpBranch %197
%196 = OpLabel
%200 = OpLoad %int %x_2
%201 = OpIAdd %int %200 %int_1
OpStore %x_2 %201
OpBranch %193
%197 = OpLabel
OpBranch %191
%191 = OpLabel
OpReturnValue %true
%189 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_continue_in_loop = OpFunction %bool None %36
%202 = OpLabel
%x_3 = OpVariable %_ptr_Function_int Function
%203 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%204 = OpLoad %float %203
%205 = OpConvertFToS %int %204
OpSelectionMerge %206 None
OpSwitch %205 %208 1 %207
%207 = OpLabel
OpStore %x_3 %int_0
OpBranch %210
%210 = OpLabel
OpLoopMerge %214 %213 None
OpBranch %211
%211 = OpLabel
%215 = OpLoad %int %x_3
%216 = OpSLessThanEqual %bool %215 %int_10
OpBranchConditional %216 %212 %214
%212 = OpLabel
OpBranch %213
%213 = OpLabel
%217 = OpLoad %int %x_3
%218 = OpIAdd %int %217 %int_1
OpStore %x_3 %218
OpBranch %210
%214 = OpLabel
OpBranch %208
%208 = OpLabel
OpReturnValue %true
%206 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_if_that_returns = OpFunction %bool None %36
%219 = OpLabel
%220 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%221 = OpLoad %float %220
%222 = OpConvertFToS %int %221
OpSelectionMerge %223 None
OpSwitch %222 %225 1 %224
%224 = OpLabel
%226 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%227 = OpLoad %float %226
%228 = OpFOrdEqual %bool %227 %float_123
OpSelectionMerge %231 None
OpBranchConditional %228 %229 %230
%229 = OpLabel
OpReturnValue %true
%230 = OpLabel
OpReturnValue %true
%231 = OpLabel
OpBranch %225
%225 = OpLabel
OpReturnValue %true
%223 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_one_sided_if_then_fallthrough = OpFunction %bool None %36
%232 = OpLabel
%233 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%234 = OpLoad %float %233
%235 = OpConvertFToS %int %234
OpSelectionMerge %236 None
OpSwitch %235 %238 1 %237
%237 = OpLabel
%239 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%240 = OpLoad %float %239
%241 = OpFOrdEqual %bool %240 %float_123
OpSelectionMerge %243 None
OpBranchConditional %241 %242 %243
%242 = OpLabel
OpReturnValue %true
%243 = OpLabel
OpBranch %238
%238 = OpLabel
OpReturnValue %true
%236 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %v4float None %244
%245 = OpLabel
%318 = OpVariable %_ptr_Function_v4float Function
OpSelectionMerge %247 None
OpBranchConditional %true %246 %247
%246 = OpLabel
%248 = OpFunctionCall %bool %return_on_both_sides
OpBranch %247
%247 = OpLabel
%249 = OpPhi %bool %false %245 %248 %246
OpSelectionMerge %251 None
OpBranchConditional %249 %250 %251
%250 = OpLabel
%252 = OpFunctionCall %bool %for_inside_body
OpBranch %251
%251 = OpLabel
%253 = OpPhi %bool %false %247 %252 %250
OpSelectionMerge %255 None
OpBranchConditional %253 %254 %255
%254 = OpLabel
%256 = OpFunctionCall %bool %after_for_body
OpBranch %255
%255 = OpLabel
%257 = OpPhi %bool %false %251 %256 %254
OpSelectionMerge %259 None
OpBranchConditional %257 %258 %259
%258 = OpLabel
%260 = OpFunctionCall %bool %for_with_double_sided_conditional_return
OpBranch %259
%259 = OpLabel
%261 = OpPhi %bool %false %255 %260 %258
OpSelectionMerge %263 None
OpBranchConditional %261 %262 %263
%262 = OpLabel
%264 = OpFunctionCall %bool %if_else_chain
OpBranch %263
%263 = OpLabel
%265 = OpPhi %bool %false %259 %264 %262
OpSelectionMerge %267 None
OpBranchConditional %265 %266 %267
%266 = OpLabel
%268 = OpFunctionCall %bool %conditional_inside_while_loop
OpBranch %267
%267 = OpLabel
%269 = OpPhi %bool %false %263 %268 %266
OpSelectionMerge %271 None
OpBranchConditional %269 %270 %271
%270 = OpLabel
%272 = OpFunctionCall %bool %inside_do_loop
OpBranch %271
%271 = OpLabel
%273 = OpPhi %bool %false %267 %272 %270
OpSelectionMerge %275 None
OpBranchConditional %273 %274 %275
%274 = OpLabel
%276 = OpFunctionCall %bool %inside_while_loop
OpBranch %275
%275 = OpLabel
%277 = OpPhi %bool %false %271 %276 %274
OpSelectionMerge %279 None
OpBranchConditional %277 %278 %279
%278 = OpLabel
%280 = OpFunctionCall %bool %after_do_loop
OpBranch %279
%279 = OpLabel
%281 = OpPhi %bool %false %275 %280 %278
OpSelectionMerge %283 None
OpBranchConditional %281 %282 %283
%282 = OpLabel
%284 = OpFunctionCall %bool %after_while_loop
OpBranch %283
%283 = OpLabel
%285 = OpPhi %bool %false %279 %284 %282
OpSelectionMerge %287 None
OpBranchConditional %285 %286 %287
%286 = OpLabel
%288 = OpFunctionCall %bool %switch_with_all_returns
OpBranch %287
%287 = OpLabel
%289 = OpPhi %bool %false %283 %288 %286
OpSelectionMerge %291 None
OpBranchConditional %289 %290 %291
%290 = OpLabel
%292 = OpFunctionCall %bool %switch_only_default
OpBranch %291
%291 = OpLabel
%293 = OpPhi %bool %false %287 %292 %290
OpSelectionMerge %295 None
OpBranchConditional %293 %294 %295
%294 = OpLabel
%296 = OpFunctionCall %bool %switch_fallthrough
OpBranch %295
%295 = OpLabel
%297 = OpPhi %bool %false %291 %296 %294
OpSelectionMerge %299 None
OpBranchConditional %297 %298 %299
%298 = OpLabel
%300 = OpFunctionCall %bool %switch_fallthrough_twice
OpBranch %299
%299 = OpLabel
%301 = OpPhi %bool %false %295 %300 %298
OpSelectionMerge %303 None
OpBranchConditional %301 %302 %303
%302 = OpLabel
%304 = OpFunctionCall %bool %switch_with_break_in_loop
OpBranch %303
%303 = OpLabel
%305 = OpPhi %bool %false %299 %304 %302
OpSelectionMerge %307 None
OpBranchConditional %305 %306 %307
%306 = OpLabel
%308 = OpFunctionCall %bool %switch_with_continue_in_loop
OpBranch %307
%307 = OpLabel
%309 = OpPhi %bool %false %303 %308 %306
OpSelectionMerge %311 None
OpBranchConditional %309 %310 %311
%310 = OpLabel
%312 = OpFunctionCall %bool %switch_with_if_that_returns
OpBranch %311
%311 = OpLabel
%313 = OpPhi %bool %false %307 %312 %310
OpSelectionMerge %315 None
OpBranchConditional %313 %314 %315
%314 = OpLabel
%316 = OpFunctionCall %bool %switch_with_one_sided_if_then_fallthrough
OpBranch %315
%315 = OpLabel
%317 = OpPhi %bool %false %311 %316 %314
OpSelectionMerge %322 None
OpBranchConditional %317 %320 %321
%320 = OpLabel
%323 = OpAccessChain %_ptr_Uniform_v4float %28 %int_0
%325 = OpLoad %v4float %323
OpStore %318 %325
OpBranch %322
%321 = OpLabel
%326 = OpAccessChain %_ptr_Uniform_v4float %28 %int_1
%327 = OpLoad %v4float %326
OpStore %318 %327
OpBranch %322
%322 = OpLabel
%328 = OpLoad %v4float %318
OpReturnValue %328
OpFunctionEnd
