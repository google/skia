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
OpName %inside_while_loop_b "inside_while_loop_b"
OpName %inside_infinite_do_loop_b "inside_infinite_do_loop_b"
OpName %inside_infinite_while_loop_b "inside_infinite_while_loop_b"
OpName %after_do_loop_b "after_do_loop_b"
OpName %after_while_loop_b "after_while_loop_b"
OpName %switch_with_all_returns_b "switch_with_all_returns_b"
OpName %switch_fallthrough_b "switch_fallthrough_b"
OpName %switch_fallthrough_twice_b "switch_fallthrough_twice_b"
OpName %switch_with_break_in_loop_b "switch_with_break_in_loop_b"
OpName %x "x"
OpName %switch_with_continue_in_loop_b "switch_with_continue_in_loop_b"
OpName %x_0 "x"
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
OpDecorate %22 Binding 0
OpDecorate %22 DescriptorSet 0
OpDecorate %46 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%22 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%27 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%31 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%35 = OpTypeFunction %bool
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_123 = OpConstant %float 123
%false = OpConstantFalse %bool
%true = OpConstantTrue %bool
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%int_1 = OpConstant %int 1
%162 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %27
%28 = OpLabel
%32 = OpVariable %_ptr_Function_v2float Function
OpStore %32 %31
%34 = OpFunctionCall %v4float %main %32
OpStore %sk_FragColor %34
OpReturn
OpFunctionEnd
%inside_while_loop_b = OpFunction %bool None %35
%36 = OpLabel
OpBranch %37
%37 = OpLabel
OpLoopMerge %41 %40 None
OpBranch %38
%38 = OpLabel
%42 = OpAccessChain %_ptr_Uniform_float %22 %int_2
%46 = OpLoad %float %42
%48 = OpFOrdEqual %bool %46 %float_123
OpBranchConditional %48 %39 %41
%39 = OpLabel
OpReturnValue %false
%40 = OpLabel
OpBranch %37
%41 = OpLabel
OpReturnValue %true
OpFunctionEnd
%inside_infinite_do_loop_b = OpFunction %bool None %35
%51 = OpLabel
OpBranch %52
%52 = OpLabel
OpLoopMerge %56 %55 None
OpBranch %53
%53 = OpLabel
OpReturnValue %true
%54 = OpLabel
OpBranch %55
%55 = OpLabel
OpBranchConditional %true %52 %56
%56 = OpLabel
OpUnreachable
OpFunctionEnd
%inside_infinite_while_loop_b = OpFunction %bool None %35
%57 = OpLabel
OpBranch %58
%58 = OpLabel
OpLoopMerge %62 %61 None
OpBranch %59
%59 = OpLabel
OpBranchConditional %true %60 %62
%60 = OpLabel
OpReturnValue %true
%61 = OpLabel
OpBranch %58
%62 = OpLabel
OpUnreachable
OpFunctionEnd
%after_do_loop_b = OpFunction %bool None %35
%63 = OpLabel
OpBranch %64
%64 = OpLabel
OpLoopMerge %68 %67 None
OpBranch %65
%65 = OpLabel
OpBranch %68
%66 = OpLabel
OpBranch %67
%67 = OpLabel
OpBranchConditional %true %64 %68
%68 = OpLabel
OpReturnValue %true
OpFunctionEnd
%after_while_loop_b = OpFunction %bool None %35
%69 = OpLabel
OpBranch %70
%70 = OpLabel
OpLoopMerge %74 %73 None
OpBranch %71
%71 = OpLabel
OpBranchConditional %true %72 %74
%72 = OpLabel
OpBranch %74
%73 = OpLabel
OpBranch %70
%74 = OpLabel
OpReturnValue %true
OpFunctionEnd
%switch_with_all_returns_b = OpFunction %bool None %35
%75 = OpLabel
%76 = OpAccessChain %_ptr_Uniform_float %22 %int_2
%77 = OpLoad %float %76
%78 = OpConvertFToS %int %77
OpSelectionMerge %79 None
OpSwitch %78 %82 1 %80 2 %81
%80 = OpLabel
OpReturnValue %true
%81 = OpLabel
OpReturnValue %false
%82 = OpLabel
OpReturnValue %false
%79 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_fallthrough_b = OpFunction %bool None %35
%83 = OpLabel
%84 = OpAccessChain %_ptr_Uniform_float %22 %int_2
%85 = OpLoad %float %84
%86 = OpConvertFToS %int %85
OpSelectionMerge %87 None
OpSwitch %86 %90 1 %88 2 %89
%88 = OpLabel
OpReturnValue %true
%89 = OpLabel
OpBranch %90
%90 = OpLabel
OpReturnValue %false
%87 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_fallthrough_twice_b = OpFunction %bool None %35
%91 = OpLabel
%92 = OpAccessChain %_ptr_Uniform_float %22 %int_2
%93 = OpLoad %float %92
%94 = OpConvertFToS %int %93
OpSelectionMerge %95 None
OpSwitch %94 %98 1 %96 2 %97
%96 = OpLabel
OpBranch %97
%97 = OpLabel
OpBranch %98
%98 = OpLabel
OpReturnValue %true
%95 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_break_in_loop_b = OpFunction %bool None %35
%99 = OpLabel
%x = OpVariable %_ptr_Function_int Function
%100 = OpAccessChain %_ptr_Uniform_float %22 %int_2
%101 = OpLoad %float %100
%102 = OpConvertFToS %int %101
OpSelectionMerge %103 None
OpSwitch %102 %105 1 %104
%104 = OpLabel
OpStore %x %int_0
OpBranch %109
%109 = OpLabel
OpLoopMerge %113 %112 None
OpBranch %110
%110 = OpLabel
%114 = OpLoad %int %x
%116 = OpSLessThanEqual %bool %114 %int_10
OpBranchConditional %116 %111 %113
%111 = OpLabel
OpBranch %113
%112 = OpLabel
%118 = OpLoad %int %x
%119 = OpIAdd %int %118 %int_1
OpStore %x %119
OpBranch %109
%113 = OpLabel
OpBranch %105
%105 = OpLabel
OpReturnValue %true
%103 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_continue_in_loop_b = OpFunction %bool None %35
%120 = OpLabel
%x_0 = OpVariable %_ptr_Function_int Function
%121 = OpAccessChain %_ptr_Uniform_float %22 %int_2
%122 = OpLoad %float %121
%123 = OpConvertFToS %int %122
OpSelectionMerge %124 None
OpSwitch %123 %126 1 %125
%125 = OpLabel
OpStore %x_0 %int_0
OpBranch %128
%128 = OpLabel
OpLoopMerge %132 %131 None
OpBranch %129
%129 = OpLabel
%133 = OpLoad %int %x_0
%134 = OpSLessThanEqual %bool %133 %int_10
OpBranchConditional %134 %130 %132
%130 = OpLabel
OpBranch %131
%131 = OpLabel
%135 = OpLoad %int %x_0
%136 = OpIAdd %int %135 %int_1
OpStore %x_0 %136
OpBranch %128
%132 = OpLabel
OpBranch %126
%126 = OpLabel
OpReturnValue %true
%124 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_if_that_returns_b = OpFunction %bool None %35
%137 = OpLabel
%138 = OpAccessChain %_ptr_Uniform_float %22 %int_2
%139 = OpLoad %float %138
%140 = OpConvertFToS %int %139
OpSelectionMerge %141 None
OpSwitch %140 %143 1 %142
%142 = OpLabel
%144 = OpAccessChain %_ptr_Uniform_float %22 %int_2
%145 = OpLoad %float %144
%146 = OpFOrdEqual %bool %145 %float_123
OpSelectionMerge %149 None
OpBranchConditional %146 %147 %148
%147 = OpLabel
OpReturnValue %false
%148 = OpLabel
OpReturnValue %true
%149 = OpLabel
OpBranch %143
%143 = OpLabel
OpReturnValue %true
%141 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_one_sided_if_then_fallthrough_b = OpFunction %bool None %35
%150 = OpLabel
%151 = OpAccessChain %_ptr_Uniform_float %22 %int_2
%152 = OpLoad %float %151
%153 = OpConvertFToS %int %152
OpSelectionMerge %154 None
OpSwitch %153 %156 1 %155
%155 = OpLabel
%157 = OpAccessChain %_ptr_Uniform_float %22 %int_2
%158 = OpLoad %float %157
%159 = OpFOrdEqual %bool %158 %float_123
OpSelectionMerge %161 None
OpBranchConditional %159 %160 %161
%160 = OpLabel
OpReturnValue %false
%161 = OpLabel
OpBranch %156
%156 = OpLabel
OpReturnValue %true
%154 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %v4float None %162
%163 = OpFunctionParameter %_ptr_Function_v2float
%164 = OpLabel
%210 = OpVariable %_ptr_Function_v4float Function
%165 = OpFunctionCall %bool %inside_while_loop_b
OpSelectionMerge %167 None
OpBranchConditional %165 %166 %167
%166 = OpLabel
%168 = OpFunctionCall %bool %inside_infinite_do_loop_b
OpBranch %167
%167 = OpLabel
%169 = OpPhi %bool %false %164 %168 %166
OpSelectionMerge %171 None
OpBranchConditional %169 %170 %171
%170 = OpLabel
%172 = OpFunctionCall %bool %inside_infinite_while_loop_b
OpBranch %171
%171 = OpLabel
%173 = OpPhi %bool %false %167 %172 %170
OpSelectionMerge %175 None
OpBranchConditional %173 %174 %175
%174 = OpLabel
%176 = OpFunctionCall %bool %after_do_loop_b
OpBranch %175
%175 = OpLabel
%177 = OpPhi %bool %false %171 %176 %174
OpSelectionMerge %179 None
OpBranchConditional %177 %178 %179
%178 = OpLabel
%180 = OpFunctionCall %bool %after_while_loop_b
OpBranch %179
%179 = OpLabel
%181 = OpPhi %bool %false %175 %180 %178
OpSelectionMerge %183 None
OpBranchConditional %181 %182 %183
%182 = OpLabel
%184 = OpFunctionCall %bool %switch_with_all_returns_b
OpBranch %183
%183 = OpLabel
%185 = OpPhi %bool %false %179 %184 %182
OpSelectionMerge %187 None
OpBranchConditional %185 %186 %187
%186 = OpLabel
%188 = OpFunctionCall %bool %switch_fallthrough_b
OpBranch %187
%187 = OpLabel
%189 = OpPhi %bool %false %183 %188 %186
OpSelectionMerge %191 None
OpBranchConditional %189 %190 %191
%190 = OpLabel
%192 = OpFunctionCall %bool %switch_fallthrough_twice_b
OpBranch %191
%191 = OpLabel
%193 = OpPhi %bool %false %187 %192 %190
OpSelectionMerge %195 None
OpBranchConditional %193 %194 %195
%194 = OpLabel
%196 = OpFunctionCall %bool %switch_with_break_in_loop_b
OpBranch %195
%195 = OpLabel
%197 = OpPhi %bool %false %191 %196 %194
OpSelectionMerge %199 None
OpBranchConditional %197 %198 %199
%198 = OpLabel
%200 = OpFunctionCall %bool %switch_with_continue_in_loop_b
OpBranch %199
%199 = OpLabel
%201 = OpPhi %bool %false %195 %200 %198
OpSelectionMerge %203 None
OpBranchConditional %201 %202 %203
%202 = OpLabel
%204 = OpFunctionCall %bool %switch_with_if_that_returns_b
OpBranch %203
%203 = OpLabel
%205 = OpPhi %bool %false %199 %204 %202
OpSelectionMerge %207 None
OpBranchConditional %205 %206 %207
%206 = OpLabel
%208 = OpFunctionCall %bool %switch_with_one_sided_if_then_fallthrough_b
OpBranch %207
%207 = OpLabel
%209 = OpPhi %bool %false %203 %208 %206
OpSelectionMerge %214 None
OpBranchConditional %209 %212 %213
%212 = OpLabel
%215 = OpAccessChain %_ptr_Uniform_v4float %22 %int_0
%217 = OpLoad %v4float %215
OpStore %210 %217
OpBranch %214
%213 = OpLabel
%218 = OpAccessChain %_ptr_Uniform_v4float %22 %int_1
%219 = OpLoad %v4float %218
OpStore %210 %219
OpBranch %214
%214 = OpLabel
%220 = OpLoad %v4float %210
OpReturnValue %220
OpFunctionEnd
