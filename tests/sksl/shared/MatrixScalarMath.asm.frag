OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %minus "minus"
OpName %star "star"
OpName %slash "slash"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_bifffff22 "test_bifffff22"
OpName %one "one"
OpName %m2 "m2"
OpName %main "main"
OpName %f1 "f1"
OpName %f2 "f2"
OpName %f3 "f3"
OpName %f4 "f4"
OpName %_0_expected "_0_expected"
OpName %_1_one "_1_one"
OpName %_2_m2 "_2_m2"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %19 Binding 0
OpDecorate %19 DescriptorSet 0
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%int = OpTypeInt 32 1
%_ptr_Private_int = OpTypePointer Private %int
%minus = OpVariable %_ptr_Private_int Private
%int_2 = OpConstant %int 2
%star = OpVariable %_ptr_Private_int Private
%int_3 = OpConstant %int 3
%slash = OpVariable %_ptr_Private_int Private
%int_4 = OpConstant %int 4
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%19 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%24 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%28 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_float = OpTypePointer Function %float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%36 = OpTypeFunction %bool %_ptr_Function_int %_ptr_Function_float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_mat2v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%69 = OpConstantComposite %v2float %float_1 %float_1
%70 = OpConstantComposite %mat2v2float %69 %69
%float_2 = OpConstant %float 2
%84 = OpConstantComposite %v2float %float_2 %float_2
%85 = OpConstantComposite %mat2v2float %84 %84
%false = OpConstantFalse %bool
%int_0 = OpConstant %int 0
%130 = OpTypeFunction %v4float %_ptr_Function_v2float
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_0_5 = OpConstant %float 0.5
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %24
%25 = OpLabel
%29 = OpVariable %_ptr_Function_v2float Function
OpStore %29 %28
%31 = OpFunctionCall %v4float %main %29
OpStore %sk_FragColor %31
OpReturn
OpFunctionEnd
%test_bifffff22 = OpFunction %bool None %36
%37 = OpFunctionParameter %_ptr_Function_int
%38 = OpFunctionParameter %_ptr_Function_float
%39 = OpFunctionParameter %_ptr_Function_float
%40 = OpFunctionParameter %_ptr_Function_float
%41 = OpFunctionParameter %_ptr_Function_float
%42 = OpFunctionParameter %_ptr_Function_mat2v2float
%43 = OpLabel
%one = OpVariable %_ptr_Function_float Function
%m2 = OpVariable %_ptr_Function_mat2v2float Function
%45 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%48 = OpLoad %v4float %45
%49 = OpCompositeExtract %float %48 0
OpStore %one %49
%51 = OpLoad %float %38
%52 = OpFMul %float %51 %49
%53 = OpLoad %float %39
%54 = OpFMul %float %53 %49
%55 = OpLoad %float %40
%56 = OpFMul %float %55 %49
%57 = OpLoad %float %41
%58 = OpFMul %float %57 %49
%59 = OpCompositeConstruct %v2float %52 %54
%60 = OpCompositeConstruct %v2float %56 %58
%61 = OpCompositeConstruct %mat2v2float %59 %60
OpStore %m2 %61
%62 = OpLoad %int %37
OpSelectionMerge %63 None
OpSwitch %62 %63 1 %64 2 %65 3 %66 4 %67
%64 = OpLabel
%71 = OpFAdd %v2float %59 %69
%72 = OpFAdd %v2float %60 %69
%73 = OpCompositeConstruct %mat2v2float %71 %72
OpStore %m2 %73
OpBranch %63
%65 = OpLabel
%74 = OpLoad %mat2v2float %m2
%75 = OpCompositeExtract %v2float %74 0
%76 = OpFSub %v2float %75 %69
%77 = OpCompositeExtract %v2float %74 1
%78 = OpFSub %v2float %77 %69
%79 = OpCompositeConstruct %mat2v2float %76 %78
OpStore %m2 %79
OpBranch %63
%66 = OpLabel
%80 = OpLoad %mat2v2float %m2
%82 = OpMatrixTimesScalar %mat2v2float %80 %float_2
OpStore %m2 %82
OpBranch %63
%67 = OpLabel
%83 = OpLoad %mat2v2float %m2
%86 = OpCompositeExtract %v2float %83 0
%87 = OpFDiv %v2float %86 %84
%88 = OpCompositeExtract %v2float %83 1
%89 = OpFDiv %v2float %88 %84
%90 = OpCompositeConstruct %mat2v2float %87 %89
OpStore %m2 %90
OpBranch %63
%63 = OpLabel
%93 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
%94 = OpLoad %v2float %93
%95 = OpCompositeExtract %float %94 0
%96 = OpAccessChain %_ptr_Function_v2float %42 %int_0
%97 = OpLoad %v2float %96
%98 = OpCompositeExtract %float %97 0
%99 = OpFOrdEqual %bool %95 %98
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%102 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
%103 = OpLoad %v2float %102
%104 = OpCompositeExtract %float %103 1
%105 = OpAccessChain %_ptr_Function_v2float %42 %int_0
%106 = OpLoad %v2float %105
%107 = OpCompositeExtract %float %106 1
%108 = OpFOrdEqual %bool %104 %107
OpBranch %101
%101 = OpLabel
%109 = OpPhi %bool %false %63 %108 %100
OpSelectionMerge %111 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
%112 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
%113 = OpLoad %v2float %112
%114 = OpCompositeExtract %float %113 0
%115 = OpAccessChain %_ptr_Function_v2float %42 %int_1
%116 = OpLoad %v2float %115
%117 = OpCompositeExtract %float %116 0
%118 = OpFOrdEqual %bool %114 %117
OpBranch %111
%111 = OpLabel
%119 = OpPhi %bool %false %101 %118 %110
OpSelectionMerge %121 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%122 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
%123 = OpLoad %v2float %122
%124 = OpCompositeExtract %float %123 1
%125 = OpAccessChain %_ptr_Function_v2float %42 %int_1
%126 = OpLoad %v2float %125
%127 = OpCompositeExtract %float %126 1
%128 = OpFOrdEqual %bool %124 %127
OpBranch %121
%121 = OpLabel
%129 = OpPhi %bool %false %111 %128 %120
OpReturnValue %129
OpFunctionEnd
%main = OpFunction %v4float None %130
%131 = OpFunctionParameter %_ptr_Function_v2float
%132 = OpLabel
%f1 = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_float Function
%f3 = OpVariable %_ptr_Function_float Function
%f4 = OpVariable %_ptr_Function_float Function
%_0_expected = OpVariable %_ptr_Function_mat2v2float Function
%_1_one = OpVariable %_ptr_Function_float Function
%_2_m2 = OpVariable %_ptr_Function_mat2v2float Function
%217 = OpVariable %_ptr_Function_int Function
%218 = OpVariable %_ptr_Function_float Function
%219 = OpVariable %_ptr_Function_float Function
%220 = OpVariable %_ptr_Function_float Function
%221 = OpVariable %_ptr_Function_float Function
%229 = OpVariable %_ptr_Function_mat2v2float Function
%235 = OpVariable %_ptr_Function_int Function
%236 = OpVariable %_ptr_Function_float Function
%237 = OpVariable %_ptr_Function_float Function
%238 = OpVariable %_ptr_Function_float Function
%239 = OpVariable %_ptr_Function_float Function
%247 = OpVariable %_ptr_Function_mat2v2float Function
%253 = OpVariable %_ptr_Function_int Function
%254 = OpVariable %_ptr_Function_float Function
%255 = OpVariable %_ptr_Function_float Function
%256 = OpVariable %_ptr_Function_float Function
%257 = OpVariable %_ptr_Function_float Function
%266 = OpVariable %_ptr_Function_mat2v2float Function
%269 = OpVariable %_ptr_Function_v4float Function
OpStore %minus %int_2
OpStore %star %int_3
OpStore %slash %int_4
%134 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%135 = OpLoad %v4float %134
%136 = OpCompositeExtract %float %135 1
OpStore %f1 %136
%138 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%139 = OpLoad %v4float %138
%140 = OpCompositeExtract %float %139 1
%141 = OpFMul %float %float_2 %140
OpStore %f2 %141
%144 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%145 = OpLoad %v4float %144
%146 = OpCompositeExtract %float %145 1
%147 = OpFMul %float %float_3 %146
OpStore %f3 %147
%150 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%151 = OpLoad %v4float %150
%152 = OpCompositeExtract %float %151 1
%153 = OpFMul %float %float_4 %152
OpStore %f4 %153
%155 = OpFAdd %float %136 %float_1
%156 = OpFAdd %float %141 %float_1
%157 = OpFAdd %float %147 %float_1
%158 = OpFAdd %float %153 %float_1
%159 = OpCompositeConstruct %v2float %155 %156
%160 = OpCompositeConstruct %v2float %157 %158
%161 = OpCompositeConstruct %mat2v2float %159 %160
OpStore %_0_expected %161
%163 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%164 = OpLoad %v4float %163
%165 = OpCompositeExtract %float %164 0
OpStore %_1_one %165
%167 = OpFMul %float %136 %165
%168 = OpFMul %float %141 %165
%169 = OpFMul %float %147 %165
%170 = OpFMul %float %153 %165
%171 = OpCompositeConstruct %v2float %167 %168
%172 = OpCompositeConstruct %v2float %169 %170
%173 = OpCompositeConstruct %mat2v2float %171 %172
OpStore %_2_m2 %173
%174 = OpFAdd %v2float %171 %69
%175 = OpFAdd %v2float %172 %69
%176 = OpCompositeConstruct %mat2v2float %174 %175
OpStore %_2_m2 %176
%177 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
%178 = OpLoad %v2float %177
%179 = OpCompositeExtract %float %178 0
%180 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
%181 = OpLoad %v2float %180
%182 = OpCompositeExtract %float %181 0
%183 = OpFOrdEqual %bool %179 %182
OpSelectionMerge %185 None
OpBranchConditional %183 %184 %185
%184 = OpLabel
%186 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
%187 = OpLoad %v2float %186
%188 = OpCompositeExtract %float %187 1
%189 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
%190 = OpLoad %v2float %189
%191 = OpCompositeExtract %float %190 1
%192 = OpFOrdEqual %bool %188 %191
OpBranch %185
%185 = OpLabel
%193 = OpPhi %bool %false %132 %192 %184
OpSelectionMerge %195 None
OpBranchConditional %193 %194 %195
%194 = OpLabel
%196 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
%197 = OpLoad %v2float %196
%198 = OpCompositeExtract %float %197 0
%199 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
%200 = OpLoad %v2float %199
%201 = OpCompositeExtract %float %200 0
%202 = OpFOrdEqual %bool %198 %201
OpBranch %195
%195 = OpLabel
%203 = OpPhi %bool %false %185 %202 %194
OpSelectionMerge %205 None
OpBranchConditional %203 %204 %205
%204 = OpLabel
%206 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
%207 = OpLoad %v2float %206
%208 = OpCompositeExtract %float %207 1
%209 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
%210 = OpLoad %v2float %209
%211 = OpCompositeExtract %float %210 1
%212 = OpFOrdEqual %bool %208 %211
OpBranch %205
%205 = OpLabel
%213 = OpPhi %bool %false %195 %212 %204
OpSelectionMerge %215 None
OpBranchConditional %213 %214 %215
%214 = OpLabel
%216 = OpLoad %int %minus
OpStore %217 %216
OpStore %218 %136
OpStore %219 %141
OpStore %220 %147
OpStore %221 %153
%222 = OpFSub %float %136 %float_1
%223 = OpFSub %float %141 %float_1
%224 = OpFSub %float %147 %float_1
%225 = OpFSub %float %153 %float_1
%226 = OpCompositeConstruct %v2float %222 %223
%227 = OpCompositeConstruct %v2float %224 %225
%228 = OpCompositeConstruct %mat2v2float %226 %227
OpStore %229 %228
%230 = OpFunctionCall %bool %test_bifffff22 %217 %218 %219 %220 %221 %229
OpBranch %215
%215 = OpLabel
%231 = OpPhi %bool %false %205 %230 %214
OpSelectionMerge %233 None
OpBranchConditional %231 %232 %233
%232 = OpLabel
%234 = OpLoad %int %star
OpStore %235 %234
OpStore %236 %136
OpStore %237 %141
OpStore %238 %147
OpStore %239 %153
%240 = OpFMul %float %136 %float_2
%241 = OpFMul %float %141 %float_2
%242 = OpFMul %float %147 %float_2
%243 = OpFMul %float %153 %float_2
%244 = OpCompositeConstruct %v2float %240 %241
%245 = OpCompositeConstruct %v2float %242 %243
%246 = OpCompositeConstruct %mat2v2float %244 %245
OpStore %247 %246
%248 = OpFunctionCall %bool %test_bifffff22 %235 %236 %237 %238 %239 %247
OpBranch %233
%233 = OpLabel
%249 = OpPhi %bool %false %215 %248 %232
OpSelectionMerge %251 None
OpBranchConditional %249 %250 %251
%250 = OpLabel
%252 = OpLoad %int %slash
OpStore %253 %252
OpStore %254 %136
OpStore %255 %141
OpStore %256 %147
OpStore %257 %153
%259 = OpFMul %float %136 %float_0_5
%260 = OpFMul %float %141 %float_0_5
%261 = OpFMul %float %147 %float_0_5
%262 = OpFMul %float %153 %float_0_5
%263 = OpCompositeConstruct %v2float %259 %260
%264 = OpCompositeConstruct %v2float %261 %262
%265 = OpCompositeConstruct %mat2v2float %263 %264
OpStore %266 %265
%267 = OpFunctionCall %bool %test_bifffff22 %253 %254 %255 %256 %257 %266
OpBranch %251
%251 = OpLabel
%268 = OpPhi %bool %false %233 %267 %250
OpSelectionMerge %273 None
OpBranchConditional %268 %271 %272
%271 = OpLabel
%274 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%275 = OpLoad %v4float %274
OpStore %269 %275
OpBranch %273
%272 = OpLabel
%276 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%277 = OpLoad %v4float %276
OpStore %269 %277
OpBranch %273
%273 = OpLabel
%278 = OpLoad %v4float %269
OpReturnValue %278
OpFunctionEnd
