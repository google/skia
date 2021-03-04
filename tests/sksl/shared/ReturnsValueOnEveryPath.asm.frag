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
OpName %for_inside_body "for_inside_body"
OpName %x "x"
OpName %after_for_body "after_for_body"
OpName %x_0 "x"
OpName %for_with_double_sided_conditional_return "for_with_double_sided_conditional_return"
OpName %x_1 "x"
OpName %switch_with_all_returns "switch_with_all_returns"
OpName %switch_only_default "switch_only_default"
OpName %switch_fallthrough "switch_fallthrough"
OpName %switch_fallthrough_twice "switch_fallthrough_twice"
OpName %switch_with_break_in_loop "switch_with_break_in_loop"
OpName %x_2 "x"
OpName %switch_with_continue_in_loop "switch_with_continue_in_loop"
OpName %x_3 "x"
OpName %switch_with_if_that_returns "switch_with_if_that_returns"
OpName %main "main"
OpName %_0_return_on_both_sides "_0_return_on_both_sides"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %20 Binding 0
OpDecorate %20 DescriptorSet 0
OpDecorate %69 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%20 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%25 = OpTypeFunction %void
%28 = OpTypeFunction %bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%true = OpConstantTrue %bool
%int_1 = OpConstant %int 1
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%float_123 = OpConstant %float 123
%155 = OpTypeFunction %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint = OpFunction %void None %25
%26 = OpLabel
%27 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %27
OpReturn
OpFunctionEnd
%for_inside_body = OpFunction %bool None %28
%29 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_0
OpBranch %34
%34 = OpLabel
OpLoopMerge %38 %37 None
OpBranch %35
%35 = OpLabel
%39 = OpLoad %int %x
%41 = OpSLessThanEqual %bool %39 %int_10
OpBranchConditional %41 %36 %38
%36 = OpLabel
OpReturnValue %true
%37 = OpLabel
%44 = OpLoad %int %x
%45 = OpIAdd %int %44 %int_1
OpStore %x %45
OpBranch %34
%38 = OpLabel
OpUnreachable
OpFunctionEnd
%after_for_body = OpFunction %bool None %28
%46 = OpLabel
%x_0 = OpVariable %_ptr_Function_int Function
OpStore %x_0 %int_0
OpBranch %48
%48 = OpLabel
OpLoopMerge %52 %51 None
OpBranch %49
%49 = OpLabel
%53 = OpLoad %int %x_0
%54 = OpSLessThanEqual %bool %53 %int_10
OpBranchConditional %54 %50 %52
%50 = OpLabel
OpBranch %51
%51 = OpLabel
%55 = OpLoad %int %x_0
%56 = OpIAdd %int %55 %int_1
OpStore %x_0 %56
OpBranch %48
%52 = OpLabel
OpReturnValue %true
OpFunctionEnd
%for_with_double_sided_conditional_return = OpFunction %bool None %28
%57 = OpLabel
%x_1 = OpVariable %_ptr_Function_int Function
OpStore %x_1 %int_0
OpBranch %59
%59 = OpLabel
OpLoopMerge %63 %62 None
OpBranch %60
%60 = OpLabel
%64 = OpLoad %int %x_1
%65 = OpSLessThanEqual %bool %64 %int_10
OpBranchConditional %65 %61 %63
%61 = OpLabel
%66 = OpAccessChain %_ptr_Uniform_float %20 %int_2
%69 = OpLoad %float %66
%71 = OpFOrdEqual %bool %69 %float_1
OpSelectionMerge %74 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
OpReturnValue %true
%73 = OpLabel
OpReturnValue %true
%74 = OpLabel
OpBranch %62
%62 = OpLabel
%75 = OpLoad %int %x_1
%76 = OpIAdd %int %75 %int_1
OpStore %x_1 %76
OpBranch %59
%63 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_all_returns = OpFunction %bool None %28
%77 = OpLabel
%78 = OpAccessChain %_ptr_Uniform_float %20 %int_2
%79 = OpLoad %float %78
%80 = OpConvertFToS %int %79
OpSelectionMerge %81 None
OpSwitch %80 %84 1 %82 2 %83
%82 = OpLabel
OpReturnValue %true
%83 = OpLabel
OpReturnValue %true
%84 = OpLabel
OpReturnValue %true
%81 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_only_default = OpFunction %bool None %28
%85 = OpLabel
%86 = OpAccessChain %_ptr_Uniform_float %20 %int_2
%87 = OpLoad %float %86
%88 = OpConvertFToS %int %87
OpSelectionMerge %89 None
OpSwitch %88 %90
%90 = OpLabel
OpReturnValue %true
%89 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_fallthrough = OpFunction %bool None %28
%91 = OpLabel
%92 = OpAccessChain %_ptr_Uniform_float %20 %int_2
%93 = OpLoad %float %92
%94 = OpConvertFToS %int %93
OpSelectionMerge %95 None
OpSwitch %94 %98 1 %96 2 %97
%96 = OpLabel
OpReturnValue %true
%97 = OpLabel
OpBranch %98
%98 = OpLabel
OpReturnValue %true
%95 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_fallthrough_twice = OpFunction %bool None %28
%99 = OpLabel
%100 = OpAccessChain %_ptr_Uniform_float %20 %int_2
%101 = OpLoad %float %100
%102 = OpConvertFToS %int %101
OpSelectionMerge %103 None
OpSwitch %102 %106 1 %104 2 %105
%104 = OpLabel
OpBranch %105
%105 = OpLabel
OpBranch %106
%106 = OpLabel
OpReturnValue %true
%103 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_break_in_loop = OpFunction %bool None %28
%107 = OpLabel
%x_2 = OpVariable %_ptr_Function_int Function
%108 = OpAccessChain %_ptr_Uniform_float %20 %int_2
%109 = OpLoad %float %108
%110 = OpConvertFToS %int %109
OpSelectionMerge %111 None
OpSwitch %110 %113 1 %112
%112 = OpLabel
OpStore %x_2 %int_0
OpBranch %115
%115 = OpLabel
OpLoopMerge %119 %118 None
OpBranch %116
%116 = OpLabel
%120 = OpLoad %int %x_2
%121 = OpSLessThanEqual %bool %120 %int_10
OpBranchConditional %121 %117 %119
%117 = OpLabel
OpBranch %119
%118 = OpLabel
%122 = OpLoad %int %x_2
%123 = OpIAdd %int %122 %int_1
OpStore %x_2 %123
OpBranch %115
%119 = OpLabel
OpBranch %113
%113 = OpLabel
OpReturnValue %true
%111 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_continue_in_loop = OpFunction %bool None %28
%124 = OpLabel
%x_3 = OpVariable %_ptr_Function_int Function
%125 = OpAccessChain %_ptr_Uniform_float %20 %int_2
%126 = OpLoad %float %125
%127 = OpConvertFToS %int %126
OpSelectionMerge %128 None
OpSwitch %127 %130 1 %129
%129 = OpLabel
OpStore %x_3 %int_0
OpBranch %132
%132 = OpLabel
OpLoopMerge %136 %135 None
OpBranch %133
%133 = OpLabel
%137 = OpLoad %int %x_3
%138 = OpSLessThanEqual %bool %137 %int_10
OpBranchConditional %138 %134 %136
%134 = OpLabel
OpBranch %135
%135 = OpLabel
%139 = OpLoad %int %x_3
%140 = OpIAdd %int %139 %int_1
OpStore %x_3 %140
OpBranch %132
%136 = OpLabel
OpBranch %130
%130 = OpLabel
OpReturnValue %true
%128 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_if_that_returns = OpFunction %bool None %28
%141 = OpLabel
%142 = OpAccessChain %_ptr_Uniform_float %20 %int_2
%143 = OpLoad %float %142
%144 = OpConvertFToS %int %143
OpSelectionMerge %145 None
OpSwitch %144 %147 1 %146
%146 = OpLabel
%148 = OpAccessChain %_ptr_Uniform_float %20 %int_2
%149 = OpLoad %float %148
%151 = OpFOrdEqual %bool %149 %float_123
OpSelectionMerge %154 None
OpBranchConditional %151 %152 %153
%152 = OpLabel
OpReturnValue %true
%153 = OpLabel
OpReturnValue %true
%154 = OpLabel
OpBranch %147
%147 = OpLabel
OpReturnValue %true
%145 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %v4float None %155
%156 = OpLabel
%_0_return_on_both_sides = OpVariable %_ptr_Function_bool Function
%207 = OpVariable %_ptr_Function_v4float Function
%159 = OpAccessChain %_ptr_Uniform_float %20 %int_2
%160 = OpLoad %float %159
%161 = OpFOrdEqual %bool %160 %float_1
OpSelectionMerge %164 None
OpBranchConditional %161 %162 %163
%162 = OpLabel
OpStore %_0_return_on_both_sides %true
OpBranch %164
%163 = OpLabel
OpStore %_0_return_on_both_sides %true
OpBranch %164
%164 = OpLabel
%166 = OpLoad %bool %_0_return_on_both_sides
OpSelectionMerge %168 None
OpBranchConditional %166 %167 %168
%167 = OpLabel
%169 = OpFunctionCall %bool %for_inside_body
OpBranch %168
%168 = OpLabel
%170 = OpPhi %bool %false %164 %169 %167
OpSelectionMerge %172 None
OpBranchConditional %170 %171 %172
%171 = OpLabel
%173 = OpFunctionCall %bool %after_for_body
OpBranch %172
%172 = OpLabel
%174 = OpPhi %bool %false %168 %173 %171
OpSelectionMerge %176 None
OpBranchConditional %174 %175 %176
%175 = OpLabel
%177 = OpFunctionCall %bool %for_with_double_sided_conditional_return
OpBranch %176
%176 = OpLabel
%178 = OpPhi %bool %false %172 %177 %175
OpSelectionMerge %180 None
OpBranchConditional %178 %179 %180
%179 = OpLabel
%181 = OpFunctionCall %bool %switch_with_all_returns
OpBranch %180
%180 = OpLabel
%182 = OpPhi %bool %false %176 %181 %179
OpSelectionMerge %184 None
OpBranchConditional %182 %183 %184
%183 = OpLabel
%185 = OpFunctionCall %bool %switch_only_default
OpBranch %184
%184 = OpLabel
%186 = OpPhi %bool %false %180 %185 %183
OpSelectionMerge %188 None
OpBranchConditional %186 %187 %188
%187 = OpLabel
%189 = OpFunctionCall %bool %switch_fallthrough
OpBranch %188
%188 = OpLabel
%190 = OpPhi %bool %false %184 %189 %187
OpSelectionMerge %192 None
OpBranchConditional %190 %191 %192
%191 = OpLabel
%193 = OpFunctionCall %bool %switch_fallthrough_twice
OpBranch %192
%192 = OpLabel
%194 = OpPhi %bool %false %188 %193 %191
OpSelectionMerge %196 None
OpBranchConditional %194 %195 %196
%195 = OpLabel
%197 = OpFunctionCall %bool %switch_with_break_in_loop
OpBranch %196
%196 = OpLabel
%198 = OpPhi %bool %false %192 %197 %195
OpSelectionMerge %200 None
OpBranchConditional %198 %199 %200
%199 = OpLabel
%201 = OpFunctionCall %bool %switch_with_continue_in_loop
OpBranch %200
%200 = OpLabel
%202 = OpPhi %bool %false %196 %201 %199
OpSelectionMerge %204 None
OpBranchConditional %202 %203 %204
%203 = OpLabel
%205 = OpFunctionCall %bool %switch_with_if_that_returns
OpBranch %204
%204 = OpLabel
%206 = OpPhi %bool %false %200 %205 %203
OpSelectionMerge %211 None
OpBranchConditional %206 %209 %210
%209 = OpLabel
%212 = OpAccessChain %_ptr_Uniform_v4float %20 %int_0
%214 = OpLoad %v4float %212
OpStore %207 %214
OpBranch %211
%210 = OpLabel
%215 = OpAccessChain %_ptr_Uniform_v4float %20 %int_1
%216 = OpLoad %v4float %215
OpStore %207 %216
OpBranch %211
%211 = OpLabel
%217 = OpLoad %v4float %207
OpReturnValue %217
OpFunctionEnd
