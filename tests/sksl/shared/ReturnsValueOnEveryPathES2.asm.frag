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
OpDecorate %23 Binding 0
OpDecorate %23 DescriptorSet 0
OpDecorate %42 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%23 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%28 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%32 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
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
%211 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %28
%29 = OpLabel
%33 = OpVariable %_ptr_Function_v2float Function
OpStore %33 %32
%35 = OpFunctionCall %v4float %main %33
OpStore %sk_FragColor %35
OpReturn
OpFunctionEnd
%return_on_both_sides_b = OpFunction %bool None %36
%37 = OpLabel
%38 = OpAccessChain %_ptr_Uniform_float %23 %int_2
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
%for_inside_body_b = OpFunction %bool None %36
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
%after_for_body_b = OpFunction %bool None %36
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
%for_with_double_sided_conditional_return_b = OpFunction %bool None %36
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
%84 = OpAccessChain %_ptr_Uniform_float %23 %int_2
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
%if_else_chain_b = OpFunction %bool None %36
%92 = OpLabel
%93 = OpAccessChain %_ptr_Uniform_float %23 %int_2
%94 = OpLoad %float %93
%95 = OpFOrdEqual %bool %94 %float_1
OpSelectionMerge %98 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
OpReturnValue %true
%97 = OpLabel
%99 = OpAccessChain %_ptr_Uniform_float %23 %int_2
%100 = OpLoad %float %99
%102 = OpFOrdEqual %bool %100 %float_2
OpSelectionMerge %105 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
OpReturnValue %false
%104 = OpLabel
%107 = OpAccessChain %_ptr_Uniform_float %23 %int_2
%108 = OpLoad %float %107
%110 = OpFOrdEqual %bool %108 %float_3
OpSelectionMerge %113 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
OpReturnValue %true
%112 = OpLabel
%114 = OpAccessChain %_ptr_Uniform_float %23 %int_2
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
%switch_with_all_returns_b = OpFunction %bool None %36
%121 = OpLabel
%122 = OpAccessChain %_ptr_Uniform_float %23 %int_2
%123 = OpLoad %float %122
%124 = OpConvertFToS %int %123
OpSelectionMerge %125 None
OpSwitch %124 %128 1 %126 2 %127
%126 = OpLabel
OpReturnValue %true
%127 = OpLabel
OpReturnValue %true
%128 = OpLabel
OpReturnValue %true
%125 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_only_default_b = OpFunction %bool None %36
%129 = OpLabel
%130 = OpAccessChain %_ptr_Uniform_float %23 %int_2
%131 = OpLoad %float %130
%132 = OpConvertFToS %int %131
OpSelectionMerge %133 None
OpSwitch %132 %134
%134 = OpLabel
OpReturnValue %true
%133 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_fallthrough_b = OpFunction %bool None %36
%135 = OpLabel
%136 = OpAccessChain %_ptr_Uniform_float %23 %int_2
%137 = OpLoad %float %136
%138 = OpConvertFToS %int %137
OpSelectionMerge %139 None
OpSwitch %138 %142 1 %140 2 %141
%140 = OpLabel
OpReturnValue %true
%141 = OpLabel
OpBranch %142
%142 = OpLabel
OpReturnValue %true
%139 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_fallthrough_twice_b = OpFunction %bool None %36
%143 = OpLabel
%144 = OpAccessChain %_ptr_Uniform_float %23 %int_2
%145 = OpLoad %float %144
%146 = OpConvertFToS %int %145
OpSelectionMerge %147 None
OpSwitch %146 %150 1 %148 2 %149
%148 = OpLabel
OpBranch %149
%149 = OpLabel
OpBranch %150
%150 = OpLabel
OpReturnValue %true
%147 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_break_in_loop_b = OpFunction %bool None %36
%151 = OpLabel
%x_2 = OpVariable %_ptr_Function_int Function
%152 = OpAccessChain %_ptr_Uniform_float %23 %int_2
%153 = OpLoad %float %152
%154 = OpConvertFToS %int %153
OpSelectionMerge %155 None
OpSwitch %154 %157 1 %156
%156 = OpLabel
OpStore %x_2 %int_0
OpBranch %159
%159 = OpLabel
OpLoopMerge %163 %162 None
OpBranch %160
%160 = OpLabel
%164 = OpLoad %int %x_2
%165 = OpSLessThanEqual %bool %164 %int_10
OpBranchConditional %165 %161 %163
%161 = OpLabel
OpBranch %163
%162 = OpLabel
%166 = OpLoad %int %x_2
%167 = OpIAdd %int %166 %int_1
OpStore %x_2 %167
OpBranch %159
%163 = OpLabel
OpBranch %157
%157 = OpLabel
OpReturnValue %true
%155 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_continue_in_loop_b = OpFunction %bool None %36
%168 = OpLabel
%x_3 = OpVariable %_ptr_Function_int Function
%169 = OpAccessChain %_ptr_Uniform_float %23 %int_2
%170 = OpLoad %float %169
%171 = OpConvertFToS %int %170
OpSelectionMerge %172 None
OpSwitch %171 %174 1 %173
%173 = OpLabel
OpStore %x_3 %int_0
OpBranch %176
%176 = OpLabel
OpLoopMerge %180 %179 None
OpBranch %177
%177 = OpLabel
%181 = OpLoad %int %x_3
%182 = OpSLessThanEqual %bool %181 %int_10
OpBranchConditional %182 %178 %180
%178 = OpLabel
OpBranch %179
%179 = OpLabel
%183 = OpLoad %int %x_3
%184 = OpIAdd %int %183 %int_1
OpStore %x_3 %184
OpBranch %176
%180 = OpLabel
OpBranch %174
%174 = OpLabel
OpReturnValue %true
%172 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_if_that_returns_b = OpFunction %bool None %36
%185 = OpLabel
%186 = OpAccessChain %_ptr_Uniform_float %23 %int_2
%187 = OpLoad %float %186
%188 = OpConvertFToS %int %187
OpSelectionMerge %189 None
OpSwitch %188 %191 1 %190
%190 = OpLabel
%192 = OpAccessChain %_ptr_Uniform_float %23 %int_2
%193 = OpLoad %float %192
%195 = OpFOrdEqual %bool %193 %float_123
OpSelectionMerge %198 None
OpBranchConditional %195 %196 %197
%196 = OpLabel
OpReturnValue %true
%197 = OpLabel
OpReturnValue %true
%198 = OpLabel
OpBranch %191
%191 = OpLabel
OpReturnValue %true
%189 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_one_sided_if_then_fallthrough_b = OpFunction %bool None %36
%199 = OpLabel
%200 = OpAccessChain %_ptr_Uniform_float %23 %int_2
%201 = OpLoad %float %200
%202 = OpConvertFToS %int %201
OpSelectionMerge %203 None
OpSwitch %202 %205 1 %204
%204 = OpLabel
%206 = OpAccessChain %_ptr_Uniform_float %23 %int_2
%207 = OpLoad %float %206
%208 = OpFOrdEqual %bool %207 %float_123
OpSelectionMerge %210 None
OpBranchConditional %208 %209 %210
%209 = OpLabel
OpReturnValue %true
%210 = OpLabel
OpBranch %205
%205 = OpLabel
OpReturnValue %true
%203 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %v4float None %211
%212 = OpFunctionParameter %_ptr_Function_v2float
%213 = OpLabel
%266 = OpVariable %_ptr_Function_v4float Function
OpSelectionMerge %215 None
OpBranchConditional %true %214 %215
%214 = OpLabel
%216 = OpFunctionCall %bool %return_on_both_sides_b
OpBranch %215
%215 = OpLabel
%217 = OpPhi %bool %false %213 %216 %214
OpSelectionMerge %219 None
OpBranchConditional %217 %218 %219
%218 = OpLabel
%220 = OpFunctionCall %bool %for_inside_body_b
OpBranch %219
%219 = OpLabel
%221 = OpPhi %bool %false %215 %220 %218
OpSelectionMerge %223 None
OpBranchConditional %221 %222 %223
%222 = OpLabel
%224 = OpFunctionCall %bool %after_for_body_b
OpBranch %223
%223 = OpLabel
%225 = OpPhi %bool %false %219 %224 %222
OpSelectionMerge %227 None
OpBranchConditional %225 %226 %227
%226 = OpLabel
%228 = OpFunctionCall %bool %for_with_double_sided_conditional_return_b
OpBranch %227
%227 = OpLabel
%229 = OpPhi %bool %false %223 %228 %226
OpSelectionMerge %231 None
OpBranchConditional %229 %230 %231
%230 = OpLabel
%232 = OpFunctionCall %bool %if_else_chain_b
OpBranch %231
%231 = OpLabel
%233 = OpPhi %bool %false %227 %232 %230
OpSelectionMerge %235 None
OpBranchConditional %233 %234 %235
%234 = OpLabel
%236 = OpFunctionCall %bool %switch_with_all_returns_b
OpBranch %235
%235 = OpLabel
%237 = OpPhi %bool %false %231 %236 %234
OpSelectionMerge %239 None
OpBranchConditional %237 %238 %239
%238 = OpLabel
%240 = OpFunctionCall %bool %switch_only_default_b
OpBranch %239
%239 = OpLabel
%241 = OpPhi %bool %false %235 %240 %238
OpSelectionMerge %243 None
OpBranchConditional %241 %242 %243
%242 = OpLabel
%244 = OpFunctionCall %bool %switch_fallthrough_b
OpBranch %243
%243 = OpLabel
%245 = OpPhi %bool %false %239 %244 %242
OpSelectionMerge %247 None
OpBranchConditional %245 %246 %247
%246 = OpLabel
%248 = OpFunctionCall %bool %switch_fallthrough_twice_b
OpBranch %247
%247 = OpLabel
%249 = OpPhi %bool %false %243 %248 %246
OpSelectionMerge %251 None
OpBranchConditional %249 %250 %251
%250 = OpLabel
%252 = OpFunctionCall %bool %switch_with_break_in_loop_b
OpBranch %251
%251 = OpLabel
%253 = OpPhi %bool %false %247 %252 %250
OpSelectionMerge %255 None
OpBranchConditional %253 %254 %255
%254 = OpLabel
%256 = OpFunctionCall %bool %switch_with_continue_in_loop_b
OpBranch %255
%255 = OpLabel
%257 = OpPhi %bool %false %251 %256 %254
OpSelectionMerge %259 None
OpBranchConditional %257 %258 %259
%258 = OpLabel
%260 = OpFunctionCall %bool %switch_with_if_that_returns_b
OpBranch %259
%259 = OpLabel
%261 = OpPhi %bool %false %255 %260 %258
OpSelectionMerge %263 None
OpBranchConditional %261 %262 %263
%262 = OpLabel
%264 = OpFunctionCall %bool %switch_with_one_sided_if_then_fallthrough_b
OpBranch %263
%263 = OpLabel
%265 = OpPhi %bool %false %259 %264 %262
OpSelectionMerge %270 None
OpBranchConditional %265 %268 %269
%268 = OpLabel
%271 = OpAccessChain %_ptr_Uniform_v4float %23 %int_0
%273 = OpLoad %v4float %271
OpStore %266 %273
OpBranch %270
%269 = OpLabel
%274 = OpAccessChain %_ptr_Uniform_v4float %23 %int_1
%275 = OpLoad %v4float %274
OpStore %266 %275
OpBranch %270
%270 = OpLabel
%276 = OpLoad %v4float %266
OpReturnValue %276
OpFunctionEnd
