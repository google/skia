OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
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
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_float = OpTypePointer Function %float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%29 = OpTypeFunction %bool %_ptr_Function_int %_ptr_Function_float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_mat2v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%62 = OpConstantComposite %v2float %float_1 %float_1
%63 = OpConstantComposite %mat2v2float %62 %62
%float_2 = OpConstant %float 2
%77 = OpConstantComposite %v2float %float_2 %float_2
%78 = OpConstantComposite %mat2v2float %77 %77
%false = OpConstantFalse %bool
%int_0 = OpConstant %int 0
%123 = OpTypeFunction %v4float %_ptr_Function_v2float
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%float_0_5 = OpConstant %float 0.5
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%test_bifffff22 = OpFunction %bool None %29
%30 = OpFunctionParameter %_ptr_Function_int
%31 = OpFunctionParameter %_ptr_Function_float
%32 = OpFunctionParameter %_ptr_Function_float
%33 = OpFunctionParameter %_ptr_Function_float
%34 = OpFunctionParameter %_ptr_Function_float
%35 = OpFunctionParameter %_ptr_Function_mat2v2float
%36 = OpLabel
%one = OpVariable %_ptr_Function_float Function
%m2 = OpVariable %_ptr_Function_mat2v2float Function
%38 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%41 = OpLoad %v4float %38
%42 = OpCompositeExtract %float %41 0
OpStore %one %42
%44 = OpLoad %float %31
%45 = OpFMul %float %44 %42
%46 = OpLoad %float %32
%47 = OpFMul %float %46 %42
%48 = OpLoad %float %33
%49 = OpFMul %float %48 %42
%50 = OpLoad %float %34
%51 = OpFMul %float %50 %42
%52 = OpCompositeConstruct %v2float %45 %47
%53 = OpCompositeConstruct %v2float %49 %51
%54 = OpCompositeConstruct %mat2v2float %52 %53
OpStore %m2 %54
%55 = OpLoad %int %30
OpSelectionMerge %56 None
OpSwitch %55 %56 1 %57 2 %58 3 %59 4 %60
%57 = OpLabel
%64 = OpFAdd %v2float %52 %62
%65 = OpFAdd %v2float %53 %62
%66 = OpCompositeConstruct %mat2v2float %64 %65
OpStore %m2 %66
OpBranch %56
%58 = OpLabel
%67 = OpLoad %mat2v2float %m2
%68 = OpCompositeExtract %v2float %67 0
%69 = OpFSub %v2float %68 %62
%70 = OpCompositeExtract %v2float %67 1
%71 = OpFSub %v2float %70 %62
%72 = OpCompositeConstruct %mat2v2float %69 %71
OpStore %m2 %72
OpBranch %56
%59 = OpLabel
%73 = OpLoad %mat2v2float %m2
%75 = OpMatrixTimesScalar %mat2v2float %73 %float_2
OpStore %m2 %75
OpBranch %56
%60 = OpLabel
%76 = OpLoad %mat2v2float %m2
%79 = OpCompositeExtract %v2float %76 0
%80 = OpFDiv %v2float %79 %77
%81 = OpCompositeExtract %v2float %76 1
%82 = OpFDiv %v2float %81 %77
%83 = OpCompositeConstruct %mat2v2float %80 %82
OpStore %m2 %83
OpBranch %56
%56 = OpLabel
%86 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
%87 = OpLoad %v2float %86
%88 = OpCompositeExtract %float %87 0
%89 = OpAccessChain %_ptr_Function_v2float %35 %int_0
%90 = OpLoad %v2float %89
%91 = OpCompositeExtract %float %90 0
%92 = OpFOrdEqual %bool %88 %91
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%95 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
%96 = OpLoad %v2float %95
%97 = OpCompositeExtract %float %96 1
%98 = OpAccessChain %_ptr_Function_v2float %35 %int_0
%99 = OpLoad %v2float %98
%100 = OpCompositeExtract %float %99 1
%101 = OpFOrdEqual %bool %97 %100
OpBranch %94
%94 = OpLabel
%102 = OpPhi %bool %false %56 %101 %93
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%105 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
%106 = OpLoad %v2float %105
%107 = OpCompositeExtract %float %106 0
%108 = OpAccessChain %_ptr_Function_v2float %35 %int_1
%109 = OpLoad %v2float %108
%110 = OpCompositeExtract %float %109 0
%111 = OpFOrdEqual %bool %107 %110
OpBranch %104
%104 = OpLabel
%112 = OpPhi %bool %false %94 %111 %103
OpSelectionMerge %114 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
%115 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
%116 = OpLoad %v2float %115
%117 = OpCompositeExtract %float %116 1
%118 = OpAccessChain %_ptr_Function_v2float %35 %int_1
%119 = OpLoad %v2float %118
%120 = OpCompositeExtract %float %119 1
%121 = OpFOrdEqual %bool %117 %120
OpBranch %114
%114 = OpLabel
%122 = OpPhi %bool %false %104 %121 %113
OpReturnValue %122
OpFunctionEnd
%main = OpFunction %v4float None %123
%124 = OpFunctionParameter %_ptr_Function_v2float
%125 = OpLabel
%f1 = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_float Function
%f3 = OpVariable %_ptr_Function_float Function
%f4 = OpVariable %_ptr_Function_float Function
%_0_expected = OpVariable %_ptr_Function_mat2v2float Function
%_1_one = OpVariable %_ptr_Function_float Function
%_2_m2 = OpVariable %_ptr_Function_mat2v2float Function
%210 = OpVariable %_ptr_Function_int Function
%211 = OpVariable %_ptr_Function_float Function
%212 = OpVariable %_ptr_Function_float Function
%213 = OpVariable %_ptr_Function_float Function
%214 = OpVariable %_ptr_Function_float Function
%222 = OpVariable %_ptr_Function_mat2v2float Function
%228 = OpVariable %_ptr_Function_int Function
%229 = OpVariable %_ptr_Function_float Function
%230 = OpVariable %_ptr_Function_float Function
%231 = OpVariable %_ptr_Function_float Function
%232 = OpVariable %_ptr_Function_float Function
%240 = OpVariable %_ptr_Function_mat2v2float Function
%246 = OpVariable %_ptr_Function_int Function
%247 = OpVariable %_ptr_Function_float Function
%248 = OpVariable %_ptr_Function_float Function
%249 = OpVariable %_ptr_Function_float Function
%250 = OpVariable %_ptr_Function_float Function
%259 = OpVariable %_ptr_Function_mat2v2float Function
%262 = OpVariable %_ptr_Function_v4float Function
%127 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%128 = OpLoad %v4float %127
%129 = OpCompositeExtract %float %128 1
OpStore %f1 %129
%131 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%132 = OpLoad %v4float %131
%133 = OpCompositeExtract %float %132 1
%134 = OpFMul %float %float_2 %133
OpStore %f2 %134
%137 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%138 = OpLoad %v4float %137
%139 = OpCompositeExtract %float %138 1
%140 = OpFMul %float %float_3 %139
OpStore %f3 %140
%143 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%144 = OpLoad %v4float %143
%145 = OpCompositeExtract %float %144 1
%146 = OpFMul %float %float_4 %145
OpStore %f4 %146
%148 = OpFAdd %float %129 %float_1
%149 = OpFAdd %float %134 %float_1
%150 = OpFAdd %float %140 %float_1
%151 = OpFAdd %float %146 %float_1
%152 = OpCompositeConstruct %v2float %148 %149
%153 = OpCompositeConstruct %v2float %150 %151
%154 = OpCompositeConstruct %mat2v2float %152 %153
OpStore %_0_expected %154
%156 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%157 = OpLoad %v4float %156
%158 = OpCompositeExtract %float %157 0
OpStore %_1_one %158
%160 = OpFMul %float %129 %158
%161 = OpFMul %float %134 %158
%162 = OpFMul %float %140 %158
%163 = OpFMul %float %146 %158
%164 = OpCompositeConstruct %v2float %160 %161
%165 = OpCompositeConstruct %v2float %162 %163
%166 = OpCompositeConstruct %mat2v2float %164 %165
OpStore %_2_m2 %166
%167 = OpFAdd %v2float %164 %62
%168 = OpFAdd %v2float %165 %62
%169 = OpCompositeConstruct %mat2v2float %167 %168
OpStore %_2_m2 %169
%170 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
%171 = OpLoad %v2float %170
%172 = OpCompositeExtract %float %171 0
%173 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
%174 = OpLoad %v2float %173
%175 = OpCompositeExtract %float %174 0
%176 = OpFOrdEqual %bool %172 %175
OpSelectionMerge %178 None
OpBranchConditional %176 %177 %178
%177 = OpLabel
%179 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
%180 = OpLoad %v2float %179
%181 = OpCompositeExtract %float %180 1
%182 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
%183 = OpLoad %v2float %182
%184 = OpCompositeExtract %float %183 1
%185 = OpFOrdEqual %bool %181 %184
OpBranch %178
%178 = OpLabel
%186 = OpPhi %bool %false %125 %185 %177
OpSelectionMerge %188 None
OpBranchConditional %186 %187 %188
%187 = OpLabel
%189 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
%190 = OpLoad %v2float %189
%191 = OpCompositeExtract %float %190 0
%192 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
%193 = OpLoad %v2float %192
%194 = OpCompositeExtract %float %193 0
%195 = OpFOrdEqual %bool %191 %194
OpBranch %188
%188 = OpLabel
%196 = OpPhi %bool %false %178 %195 %187
OpSelectionMerge %198 None
OpBranchConditional %196 %197 %198
%197 = OpLabel
%199 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
%200 = OpLoad %v2float %199
%201 = OpCompositeExtract %float %200 1
%202 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
%203 = OpLoad %v2float %202
%204 = OpCompositeExtract %float %203 1
%205 = OpFOrdEqual %bool %201 %204
OpBranch %198
%198 = OpLabel
%206 = OpPhi %bool %false %188 %205 %197
OpSelectionMerge %208 None
OpBranchConditional %206 %207 %208
%207 = OpLabel
OpStore %210 %int_2
OpStore %211 %129
OpStore %212 %134
OpStore %213 %140
OpStore %214 %146
%215 = OpFSub %float %129 %float_1
%216 = OpFSub %float %134 %float_1
%217 = OpFSub %float %140 %float_1
%218 = OpFSub %float %146 %float_1
%219 = OpCompositeConstruct %v2float %215 %216
%220 = OpCompositeConstruct %v2float %217 %218
%221 = OpCompositeConstruct %mat2v2float %219 %220
OpStore %222 %221
%223 = OpFunctionCall %bool %test_bifffff22 %210 %211 %212 %213 %214 %222
OpBranch %208
%208 = OpLabel
%224 = OpPhi %bool %false %198 %223 %207
OpSelectionMerge %226 None
OpBranchConditional %224 %225 %226
%225 = OpLabel
OpStore %228 %int_3
OpStore %229 %129
OpStore %230 %134
OpStore %231 %140
OpStore %232 %146
%233 = OpFMul %float %129 %float_2
%234 = OpFMul %float %134 %float_2
%235 = OpFMul %float %140 %float_2
%236 = OpFMul %float %146 %float_2
%237 = OpCompositeConstruct %v2float %233 %234
%238 = OpCompositeConstruct %v2float %235 %236
%239 = OpCompositeConstruct %mat2v2float %237 %238
OpStore %240 %239
%241 = OpFunctionCall %bool %test_bifffff22 %228 %229 %230 %231 %232 %240
OpBranch %226
%226 = OpLabel
%242 = OpPhi %bool %false %208 %241 %225
OpSelectionMerge %244 None
OpBranchConditional %242 %243 %244
%243 = OpLabel
OpStore %246 %int_4
OpStore %247 %129
OpStore %248 %134
OpStore %249 %140
OpStore %250 %146
%252 = OpFMul %float %129 %float_0_5
%253 = OpFMul %float %134 %float_0_5
%254 = OpFMul %float %140 %float_0_5
%255 = OpFMul %float %146 %float_0_5
%256 = OpCompositeConstruct %v2float %252 %253
%257 = OpCompositeConstruct %v2float %254 %255
%258 = OpCompositeConstruct %mat2v2float %256 %257
OpStore %259 %258
%260 = OpFunctionCall %bool %test_bifffff22 %246 %247 %248 %249 %250 %259
OpBranch %244
%244 = OpLabel
%261 = OpPhi %bool %false %226 %260 %243
OpSelectionMerge %266 None
OpBranchConditional %261 %264 %265
%264 = OpLabel
%267 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%268 = OpLoad %v4float %267
OpStore %262 %268
OpBranch %266
%265 = OpLabel
%269 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%270 = OpLoad %v4float %269
OpStore %262 %270
OpBranch %266
%266 = OpLabel
%271 = OpLoad %v4float %262
OpReturnValue %271
OpFunctionEnd
