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
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
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
%float_0_5 = OpConstant %float 0.5
%false = OpConstantFalse %bool
%int_0 = OpConstant %int 0
%118 = OpTypeFunction %v4float %_ptr_Function_v2float
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
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
%78 = OpMatrixTimesScalar %mat2v2float %76 %float_0_5
OpStore %m2 %78
OpBranch %56
%56 = OpLabel
%81 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
%82 = OpLoad %v2float %81
%83 = OpCompositeExtract %float %82 0
%84 = OpAccessChain %_ptr_Function_v2float %35 %int_0
%85 = OpLoad %v2float %84
%86 = OpCompositeExtract %float %85 0
%87 = OpFOrdEqual %bool %83 %86
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%90 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
%91 = OpLoad %v2float %90
%92 = OpCompositeExtract %float %91 1
%93 = OpAccessChain %_ptr_Function_v2float %35 %int_0
%94 = OpLoad %v2float %93
%95 = OpCompositeExtract %float %94 1
%96 = OpFOrdEqual %bool %92 %95
OpBranch %89
%89 = OpLabel
%97 = OpPhi %bool %false %56 %96 %88
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%100 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
%101 = OpLoad %v2float %100
%102 = OpCompositeExtract %float %101 0
%103 = OpAccessChain %_ptr_Function_v2float %35 %int_1
%104 = OpLoad %v2float %103
%105 = OpCompositeExtract %float %104 0
%106 = OpFOrdEqual %bool %102 %105
OpBranch %99
%99 = OpLabel
%107 = OpPhi %bool %false %89 %106 %98
OpSelectionMerge %109 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
%110 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
%111 = OpLoad %v2float %110
%112 = OpCompositeExtract %float %111 1
%113 = OpAccessChain %_ptr_Function_v2float %35 %int_1
%114 = OpLoad %v2float %113
%115 = OpCompositeExtract %float %114 1
%116 = OpFOrdEqual %bool %112 %115
OpBranch %109
%109 = OpLabel
%117 = OpPhi %bool %false %99 %116 %108
OpReturnValue %117
OpFunctionEnd
%main = OpFunction %v4float None %118
%119 = OpFunctionParameter %_ptr_Function_v2float
%120 = OpLabel
%f1 = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_float Function
%f3 = OpVariable %_ptr_Function_float Function
%f4 = OpVariable %_ptr_Function_float Function
%_0_expected = OpVariable %_ptr_Function_mat2v2float Function
%_1_one = OpVariable %_ptr_Function_float Function
%_2_m2 = OpVariable %_ptr_Function_mat2v2float Function
%205 = OpVariable %_ptr_Function_int Function
%206 = OpVariable %_ptr_Function_float Function
%207 = OpVariable %_ptr_Function_float Function
%208 = OpVariable %_ptr_Function_float Function
%209 = OpVariable %_ptr_Function_float Function
%217 = OpVariable %_ptr_Function_mat2v2float Function
%223 = OpVariable %_ptr_Function_int Function
%224 = OpVariable %_ptr_Function_float Function
%225 = OpVariable %_ptr_Function_float Function
%226 = OpVariable %_ptr_Function_float Function
%227 = OpVariable %_ptr_Function_float Function
%235 = OpVariable %_ptr_Function_mat2v2float Function
%241 = OpVariable %_ptr_Function_int Function
%242 = OpVariable %_ptr_Function_float Function
%243 = OpVariable %_ptr_Function_float Function
%244 = OpVariable %_ptr_Function_float Function
%245 = OpVariable %_ptr_Function_float Function
%253 = OpVariable %_ptr_Function_mat2v2float Function
%256 = OpVariable %_ptr_Function_v4float Function
%122 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%123 = OpLoad %v4float %122
%124 = OpCompositeExtract %float %123 1
OpStore %f1 %124
%126 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%127 = OpLoad %v4float %126
%128 = OpCompositeExtract %float %127 1
%129 = OpFMul %float %float_2 %128
OpStore %f2 %129
%132 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%133 = OpLoad %v4float %132
%134 = OpCompositeExtract %float %133 1
%135 = OpFMul %float %float_3 %134
OpStore %f3 %135
%138 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%139 = OpLoad %v4float %138
%140 = OpCompositeExtract %float %139 1
%141 = OpFMul %float %float_4 %140
OpStore %f4 %141
%143 = OpFAdd %float %124 %float_1
%144 = OpFAdd %float %129 %float_1
%145 = OpFAdd %float %135 %float_1
%146 = OpFAdd %float %141 %float_1
%147 = OpCompositeConstruct %v2float %143 %144
%148 = OpCompositeConstruct %v2float %145 %146
%149 = OpCompositeConstruct %mat2v2float %147 %148
OpStore %_0_expected %149
%151 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%152 = OpLoad %v4float %151
%153 = OpCompositeExtract %float %152 0
OpStore %_1_one %153
%155 = OpFMul %float %124 %153
%156 = OpFMul %float %129 %153
%157 = OpFMul %float %135 %153
%158 = OpFMul %float %141 %153
%159 = OpCompositeConstruct %v2float %155 %156
%160 = OpCompositeConstruct %v2float %157 %158
%161 = OpCompositeConstruct %mat2v2float %159 %160
OpStore %_2_m2 %161
%162 = OpFAdd %v2float %159 %62
%163 = OpFAdd %v2float %160 %62
%164 = OpCompositeConstruct %mat2v2float %162 %163
OpStore %_2_m2 %164
%165 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
%166 = OpLoad %v2float %165
%167 = OpCompositeExtract %float %166 0
%168 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
%169 = OpLoad %v2float %168
%170 = OpCompositeExtract %float %169 0
%171 = OpFOrdEqual %bool %167 %170
OpSelectionMerge %173 None
OpBranchConditional %171 %172 %173
%172 = OpLabel
%174 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
%175 = OpLoad %v2float %174
%176 = OpCompositeExtract %float %175 1
%177 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
%178 = OpLoad %v2float %177
%179 = OpCompositeExtract %float %178 1
%180 = OpFOrdEqual %bool %176 %179
OpBranch %173
%173 = OpLabel
%181 = OpPhi %bool %false %120 %180 %172
OpSelectionMerge %183 None
OpBranchConditional %181 %182 %183
%182 = OpLabel
%184 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
%185 = OpLoad %v2float %184
%186 = OpCompositeExtract %float %185 0
%187 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
%188 = OpLoad %v2float %187
%189 = OpCompositeExtract %float %188 0
%190 = OpFOrdEqual %bool %186 %189
OpBranch %183
%183 = OpLabel
%191 = OpPhi %bool %false %173 %190 %182
OpSelectionMerge %193 None
OpBranchConditional %191 %192 %193
%192 = OpLabel
%194 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
%195 = OpLoad %v2float %194
%196 = OpCompositeExtract %float %195 1
%197 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
%198 = OpLoad %v2float %197
%199 = OpCompositeExtract %float %198 1
%200 = OpFOrdEqual %bool %196 %199
OpBranch %193
%193 = OpLabel
%201 = OpPhi %bool %false %183 %200 %192
OpSelectionMerge %203 None
OpBranchConditional %201 %202 %203
%202 = OpLabel
OpStore %205 %int_2
OpStore %206 %124
OpStore %207 %129
OpStore %208 %135
OpStore %209 %141
%210 = OpFSub %float %124 %float_1
%211 = OpFSub %float %129 %float_1
%212 = OpFSub %float %135 %float_1
%213 = OpFSub %float %141 %float_1
%214 = OpCompositeConstruct %v2float %210 %211
%215 = OpCompositeConstruct %v2float %212 %213
%216 = OpCompositeConstruct %mat2v2float %214 %215
OpStore %217 %216
%218 = OpFunctionCall %bool %test_bifffff22 %205 %206 %207 %208 %209 %217
OpBranch %203
%203 = OpLabel
%219 = OpPhi %bool %false %193 %218 %202
OpSelectionMerge %221 None
OpBranchConditional %219 %220 %221
%220 = OpLabel
OpStore %223 %int_3
OpStore %224 %124
OpStore %225 %129
OpStore %226 %135
OpStore %227 %141
%228 = OpFMul %float %124 %float_2
%229 = OpFMul %float %129 %float_2
%230 = OpFMul %float %135 %float_2
%231 = OpFMul %float %141 %float_2
%232 = OpCompositeConstruct %v2float %228 %229
%233 = OpCompositeConstruct %v2float %230 %231
%234 = OpCompositeConstruct %mat2v2float %232 %233
OpStore %235 %234
%236 = OpFunctionCall %bool %test_bifffff22 %223 %224 %225 %226 %227 %235
OpBranch %221
%221 = OpLabel
%237 = OpPhi %bool %false %203 %236 %220
OpSelectionMerge %239 None
OpBranchConditional %237 %238 %239
%238 = OpLabel
OpStore %241 %int_4
OpStore %242 %124
OpStore %243 %129
OpStore %244 %135
OpStore %245 %141
%246 = OpFMul %float %124 %float_0_5
%247 = OpFMul %float %129 %float_0_5
%248 = OpFMul %float %135 %float_0_5
%249 = OpFMul %float %141 %float_0_5
%250 = OpCompositeConstruct %v2float %246 %247
%251 = OpCompositeConstruct %v2float %248 %249
%252 = OpCompositeConstruct %mat2v2float %250 %251
OpStore %253 %252
%254 = OpFunctionCall %bool %test_bifffff22 %241 %242 %243 %244 %245 %253
OpBranch %239
%239 = OpLabel
%255 = OpPhi %bool %false %221 %254 %238
OpSelectionMerge %260 None
OpBranchConditional %255 %258 %259
%258 = OpLabel
%261 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%262 = OpLoad %v4float %261
OpStore %256 %262
OpBranch %260
%259 = OpLabel
%263 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%264 = OpLoad %v4float %263
OpStore %256 %264
OpBranch %260
%260 = OpLabel
%265 = OpLoad %v4float %256
OpReturnValue %265
OpFunctionEnd
