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
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpMemberName %_UniformBuffer 3 "testMatrix3x3"
OpName %_entrypoint "_entrypoint"
OpName %test_float "test_float"
OpName %v1 "v1"
OpName %v2 "v2"
OpName %m1 "m1"
OpName %m2 "m2"
OpName %m3 "m3"
OpName %m4 "m4"
OpName %m5 "m5"
OpName %m6 "m6"
OpName %m7 "m7"
OpName %m9 "m9"
OpName %m10 "m10"
OpName %m11 "m11"
OpName %test_half "test_half"
OpName %v1_0 "v1"
OpName %v2_0 "v2"
OpName %m1_0 "m1"
OpName %m2_0 "m2"
OpName %m3_0 "m3"
OpName %m4_0 "m4"
OpName %m5_0 "m5"
OpName %m6_0 "m6"
OpName %m7_0 "m7"
OpName %m9_0 "m9"
OpName %m10_0 "m10"
OpName %m11_0 "m11"
OpName %test_equality "test_equality"
OpName %ok "ok"
OpName %main "main"
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
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 64
OpMemberDecorate %_UniformBuffer 3 ColMajor
OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %338 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float %mat3v3float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%22 = OpTypeFunction %void
%25 = OpTypeFunction %bool
%_ptr_Function_v3float = OpTypePointer Function %v3float
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%float_2 = OpConstant %float 2
%36 = OpConstantComposite %v3float %float_2 %float_2 %float_2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%48 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%57 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%102 = OpConstantComposite %v3float %float_6 %float_7 %float_8
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%true = OpConstantTrue %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int_2 = OpConstant %int 2
%v2bool = OpTypeVector %bool 2
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_3 = OpConstant %int 3
%float_9 = OpConstant %float 9
%v3bool = OpTypeVector %bool 3
%float_100 = OpConstant %float 100
%339 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%_entrypoint = OpFunction %void None %22
%23 = OpLabel
%24 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%test_float = OpFunction %bool None %25
%26 = OpLabel
%v1 = OpVariable %_ptr_Function_v3float Function
%v2 = OpVariable %_ptr_Function_v3float Function
%m1 = OpVariable %_ptr_Function_mat2v2float Function
%m2 = OpVariable %_ptr_Function_mat2v2float Function
%m3 = OpVariable %_ptr_Function_mat2v2float Function
%m4 = OpVariable %_ptr_Function_mat2v2float Function
%m5 = OpVariable %_ptr_Function_mat2v2float Function
%m6 = OpVariable %_ptr_Function_mat2v2float Function
%m7 = OpVariable %_ptr_Function_mat2v2float Function
%m9 = OpVariable %_ptr_Function_mat3v3float Function
%m10 = OpVariable %_ptr_Function_mat4v4float Function
%m11 = OpVariable %_ptr_Function_mat4v4float Function
%32 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%33 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%34 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%30 = OpCompositeConstruct %mat3v3float %32 %33 %34
%37 = OpMatrixTimesVector %v3float %30 %36
OpStore %v1 %37
%40 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%41 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%42 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%39 = OpCompositeConstruct %mat3v3float %40 %41 %42
%43 = OpVectorTimesMatrix %v3float %36 %39
OpStore %v2 %43
%50 = OpCompositeExtract %float %48 0
%51 = OpCompositeExtract %float %48 1
%52 = OpCompositeExtract %float %48 2
%53 = OpCompositeExtract %float %48 3
%54 = OpCompositeConstruct %v2float %50 %51
%55 = OpCompositeConstruct %v2float %52 %53
%49 = OpCompositeConstruct %mat2v2float %54 %55
OpStore %m1 %49
%59 = OpCompositeExtract %float %57 0
%60 = OpCompositeExtract %float %57 1
%61 = OpCompositeExtract %float %57 2
%62 = OpCompositeExtract %float %57 3
%63 = OpCompositeConstruct %v2float %59 %60
%64 = OpCompositeConstruct %v2float %61 %62
%58 = OpCompositeConstruct %mat2v2float %63 %64
OpStore %m2 %58
%66 = OpLoad %mat2v2float %m1
OpStore %m3 %66
%69 = OpCompositeConstruct %v2float %float_1 %float_0
%70 = OpCompositeConstruct %v2float %float_0 %float_1
%68 = OpCompositeConstruct %mat2v2float %69 %70
OpStore %m4 %68
%71 = OpLoad %mat2v2float %m3
%72 = OpLoad %mat2v2float %m4
%73 = OpMatrixTimesMatrix %mat2v2float %71 %72
OpStore %m3 %73
%77 = OpAccessChain %_ptr_Function_v2float %m1 %int_0
%79 = OpLoad %v2float %77
%80 = OpCompositeExtract %float %79 0
%82 = OpCompositeConstruct %v2float %80 %float_0
%83 = OpCompositeConstruct %v2float %float_0 %80
%81 = OpCompositeConstruct %mat2v2float %82 %83
OpStore %m5 %81
%86 = OpCompositeConstruct %v2float %float_1 %float_2
%87 = OpCompositeConstruct %v2float %float_3 %float_4
%85 = OpCompositeConstruct %mat2v2float %86 %87
OpStore %m6 %85
%88 = OpLoad %mat2v2float %m6
%89 = OpLoad %mat2v2float %m5
%90 = OpCompositeExtract %v2float %88 0
%91 = OpCompositeExtract %v2float %89 0
%92 = OpFAdd %v2float %90 %91
%93 = OpCompositeExtract %v2float %88 1
%94 = OpCompositeExtract %v2float %89 1
%95 = OpFAdd %v2float %93 %94
%96 = OpCompositeConstruct %mat2v2float %92 %95
OpStore %m6 %96
%104 = OpCompositeExtract %float %102 0
%105 = OpCompositeConstruct %v2float %float_5 %104
%106 = OpCompositeExtract %float %102 1
%107 = OpCompositeExtract %float %102 2
%108 = OpCompositeConstruct %v2float %106 %107
%103 = OpCompositeConstruct %mat2v2float %105 %108
OpStore %m7 %103
%112 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%113 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%114 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%111 = OpCompositeConstruct %mat3v3float %112 %113 %114
OpStore %m9 %111
%119 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%120 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%121 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%122 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%118 = OpCompositeConstruct %mat4v4float %119 %120 %121 %122
OpStore %m10 %118
%125 = OpCompositeConstruct %v4float %float_2 %float_0 %float_0 %float_0
%126 = OpCompositeConstruct %v4float %float_0 %float_2 %float_0 %float_0
%127 = OpCompositeConstruct %v4float %float_0 %float_0 %float_2 %float_0
%128 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_2
%124 = OpCompositeConstruct %mat4v4float %125 %126 %127 %128
OpStore %m11 %124
%129 = OpLoad %mat4v4float %m11
%130 = OpLoad %mat4v4float %m10
%131 = OpCompositeExtract %v4float %129 0
%132 = OpCompositeExtract %v4float %130 0
%133 = OpFSub %v4float %131 %132
%134 = OpCompositeExtract %v4float %129 1
%135 = OpCompositeExtract %v4float %130 1
%136 = OpFSub %v4float %134 %135
%137 = OpCompositeExtract %v4float %129 2
%138 = OpCompositeExtract %v4float %130 2
%139 = OpFSub %v4float %137 %138
%140 = OpCompositeExtract %v4float %129 3
%141 = OpCompositeExtract %v4float %130 3
%142 = OpFSub %v4float %140 %141
%143 = OpCompositeConstruct %mat4v4float %133 %136 %139 %142
OpStore %m11 %143
OpReturnValue %true
OpFunctionEnd
%test_half = OpFunction %bool None %25
%145 = OpLabel
%v1_0 = OpVariable %_ptr_Function_v3float Function
%v2_0 = OpVariable %_ptr_Function_v3float Function
%m1_0 = OpVariable %_ptr_Function_mat2v2float Function
%m2_0 = OpVariable %_ptr_Function_mat2v2float Function
%m3_0 = OpVariable %_ptr_Function_mat2v2float Function
%m4_0 = OpVariable %_ptr_Function_mat2v2float Function
%m5_0 = OpVariable %_ptr_Function_mat2v2float Function
%m6_0 = OpVariable %_ptr_Function_mat2v2float Function
%m7_0 = OpVariable %_ptr_Function_mat2v2float Function
%m9_0 = OpVariable %_ptr_Function_mat3v3float Function
%m10_0 = OpVariable %_ptr_Function_mat4v4float Function
%m11_0 = OpVariable %_ptr_Function_mat4v4float Function
%148 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%149 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%150 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%147 = OpCompositeConstruct %mat3v3float %148 %149 %150
%151 = OpMatrixTimesVector %v3float %147 %36
OpStore %v1_0 %151
%154 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%155 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%156 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%153 = OpCompositeConstruct %mat3v3float %154 %155 %156
%157 = OpVectorTimesMatrix %v3float %36 %153
OpStore %v2_0 %157
%160 = OpCompositeExtract %float %48 0
%161 = OpCompositeExtract %float %48 1
%162 = OpCompositeExtract %float %48 2
%163 = OpCompositeExtract %float %48 3
%164 = OpCompositeConstruct %v2float %160 %161
%165 = OpCompositeConstruct %v2float %162 %163
%159 = OpCompositeConstruct %mat2v2float %164 %165
OpStore %m1_0 %159
%168 = OpCompositeExtract %float %57 0
%169 = OpCompositeExtract %float %57 1
%170 = OpCompositeExtract %float %57 2
%171 = OpCompositeExtract %float %57 3
%172 = OpCompositeConstruct %v2float %168 %169
%173 = OpCompositeConstruct %v2float %170 %171
%167 = OpCompositeConstruct %mat2v2float %172 %173
OpStore %m2_0 %167
%175 = OpLoad %mat2v2float %m1_0
OpStore %m3_0 %175
%178 = OpCompositeConstruct %v2float %float_1 %float_0
%179 = OpCompositeConstruct %v2float %float_0 %float_1
%177 = OpCompositeConstruct %mat2v2float %178 %179
OpStore %m4_0 %177
%180 = OpLoad %mat2v2float %m3_0
%181 = OpLoad %mat2v2float %m4_0
%182 = OpMatrixTimesMatrix %mat2v2float %180 %181
OpStore %m3_0 %182
%184 = OpAccessChain %_ptr_Function_v2float %m1_0 %int_0
%185 = OpLoad %v2float %184
%186 = OpCompositeExtract %float %185 0
%188 = OpCompositeConstruct %v2float %186 %float_0
%189 = OpCompositeConstruct %v2float %float_0 %186
%187 = OpCompositeConstruct %mat2v2float %188 %189
OpStore %m5_0 %187
%192 = OpCompositeConstruct %v2float %float_1 %float_2
%193 = OpCompositeConstruct %v2float %float_3 %float_4
%191 = OpCompositeConstruct %mat2v2float %192 %193
OpStore %m6_0 %191
%194 = OpLoad %mat2v2float %m6_0
%195 = OpLoad %mat2v2float %m5_0
%196 = OpCompositeExtract %v2float %194 0
%197 = OpCompositeExtract %v2float %195 0
%198 = OpFAdd %v2float %196 %197
%199 = OpCompositeExtract %v2float %194 1
%200 = OpCompositeExtract %v2float %195 1
%201 = OpFAdd %v2float %199 %200
%202 = OpCompositeConstruct %mat2v2float %198 %201
OpStore %m6_0 %202
%205 = OpCompositeExtract %float %102 0
%206 = OpCompositeConstruct %v2float %float_5 %205
%207 = OpCompositeExtract %float %102 1
%208 = OpCompositeExtract %float %102 2
%209 = OpCompositeConstruct %v2float %207 %208
%204 = OpCompositeConstruct %mat2v2float %206 %209
OpStore %m7_0 %204
%212 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%213 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%214 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%211 = OpCompositeConstruct %mat3v3float %212 %213 %214
OpStore %m9_0 %211
%217 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%218 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%219 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%220 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%216 = OpCompositeConstruct %mat4v4float %217 %218 %219 %220
OpStore %m10_0 %216
%223 = OpCompositeConstruct %v4float %float_2 %float_0 %float_0 %float_0
%224 = OpCompositeConstruct %v4float %float_0 %float_2 %float_0 %float_0
%225 = OpCompositeConstruct %v4float %float_0 %float_0 %float_2 %float_0
%226 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_2
%222 = OpCompositeConstruct %mat4v4float %223 %224 %225 %226
OpStore %m11_0 %222
%227 = OpLoad %mat4v4float %m11_0
%228 = OpLoad %mat4v4float %m10_0
%229 = OpCompositeExtract %v4float %227 0
%230 = OpCompositeExtract %v4float %228 0
%231 = OpFSub %v4float %229 %230
%232 = OpCompositeExtract %v4float %227 1
%233 = OpCompositeExtract %v4float %228 1
%234 = OpFSub %v4float %232 %233
%235 = OpCompositeExtract %v4float %227 2
%236 = OpCompositeExtract %v4float %228 2
%237 = OpFSub %v4float %235 %236
%238 = OpCompositeExtract %v4float %227 3
%239 = OpCompositeExtract %v4float %228 3
%240 = OpFSub %v4float %238 %239
%241 = OpCompositeConstruct %mat4v4float %231 %234 %237 %240
OpStore %m11_0 %241
OpReturnValue %true
OpFunctionEnd
%test_equality = OpFunction %bool None %25
%242 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
OpStore %ok %true
%246 = OpLoad %bool %ok
OpSelectionMerge %248 None
OpBranchConditional %246 %247 %248
%247 = OpLabel
%249 = OpAccessChain %_ptr_Uniform_mat2v2float %13 %int_2
%252 = OpLoad %mat2v2float %249
%254 = OpCompositeConstruct %v2float %float_1 %float_2
%255 = OpCompositeConstruct %v2float %float_3 %float_4
%253 = OpCompositeConstruct %mat2v2float %254 %255
%257 = OpCompositeExtract %v2float %252 0
%258 = OpCompositeExtract %v2float %253 0
%259 = OpFOrdEqual %v2bool %257 %258
%260 = OpAll %bool %259
%261 = OpCompositeExtract %v2float %252 1
%262 = OpCompositeExtract %v2float %253 1
%263 = OpFOrdEqual %v2bool %261 %262
%264 = OpAll %bool %263
%265 = OpLogicalAnd %bool %260 %264
OpBranch %248
%248 = OpLabel
%266 = OpPhi %bool %false %242 %265 %247
OpStore %ok %266
%267 = OpLoad %bool %ok
OpSelectionMerge %269 None
OpBranchConditional %267 %268 %269
%268 = OpLabel
%270 = OpAccessChain %_ptr_Uniform_mat3v3float %13 %int_3
%273 = OpLoad %mat3v3float %270
%276 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%277 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%278 = OpCompositeConstruct %v3float %float_7 %float_8 %float_9
%275 = OpCompositeConstruct %mat3v3float %276 %277 %278
%280 = OpCompositeExtract %v3float %273 0
%281 = OpCompositeExtract %v3float %275 0
%282 = OpFOrdEqual %v3bool %280 %281
%283 = OpAll %bool %282
%284 = OpCompositeExtract %v3float %273 1
%285 = OpCompositeExtract %v3float %275 1
%286 = OpFOrdEqual %v3bool %284 %285
%287 = OpAll %bool %286
%288 = OpLogicalAnd %bool %283 %287
%289 = OpCompositeExtract %v3float %273 2
%290 = OpCompositeExtract %v3float %275 2
%291 = OpFOrdEqual %v3bool %289 %290
%292 = OpAll %bool %291
%293 = OpLogicalAnd %bool %288 %292
OpBranch %269
%269 = OpLabel
%294 = OpPhi %bool %false %248 %293 %268
OpStore %ok %294
%295 = OpLoad %bool %ok
OpSelectionMerge %297 None
OpBranchConditional %295 %296 %297
%296 = OpLabel
%298 = OpAccessChain %_ptr_Uniform_mat2v2float %13 %int_2
%299 = OpLoad %mat2v2float %298
%302 = OpCompositeConstruct %v2float %float_100 %float_0
%303 = OpCompositeConstruct %v2float %float_0 %float_100
%301 = OpCompositeConstruct %mat2v2float %302 %303
%304 = OpCompositeExtract %v2float %299 0
%305 = OpCompositeExtract %v2float %301 0
%306 = OpFOrdNotEqual %v2bool %304 %305
%307 = OpAny %bool %306
%308 = OpCompositeExtract %v2float %299 1
%309 = OpCompositeExtract %v2float %301 1
%310 = OpFOrdNotEqual %v2bool %308 %309
%311 = OpAny %bool %310
%312 = OpLogicalOr %bool %307 %311
OpBranch %297
%297 = OpLabel
%313 = OpPhi %bool %false %269 %312 %296
OpStore %ok %313
%314 = OpLoad %bool %ok
OpSelectionMerge %316 None
OpBranchConditional %314 %315 %316
%315 = OpLabel
%317 = OpAccessChain %_ptr_Uniform_mat3v3float %13 %int_3
%318 = OpLoad %mat3v3float %317
%320 = OpCompositeConstruct %v3float %float_9 %float_8 %float_7
%321 = OpCompositeConstruct %v3float %float_6 %float_5 %float_4
%322 = OpCompositeConstruct %v3float %float_3 %float_2 %float_1
%319 = OpCompositeConstruct %mat3v3float %320 %321 %322
%323 = OpCompositeExtract %v3float %318 0
%324 = OpCompositeExtract %v3float %319 0
%325 = OpFOrdNotEqual %v3bool %323 %324
%326 = OpAny %bool %325
%327 = OpCompositeExtract %v3float %318 1
%328 = OpCompositeExtract %v3float %319 1
%329 = OpFOrdNotEqual %v3bool %327 %328
%330 = OpAny %bool %329
%331 = OpLogicalOr %bool %326 %330
%332 = OpCompositeExtract %v3float %318 2
%333 = OpCompositeExtract %v3float %319 2
%334 = OpFOrdNotEqual %v3bool %332 %333
%335 = OpAny %bool %334
%336 = OpLogicalOr %bool %331 %335
OpBranch %316
%316 = OpLabel
%337 = OpPhi %bool %false %297 %336 %315
OpStore %ok %337
%338 = OpLoad %bool %ok
OpReturnValue %338
OpFunctionEnd
%main = OpFunction %v4float None %339
%340 = OpLabel
%350 = OpVariable %_ptr_Function_v4float Function
%341 = OpFunctionCall %bool %test_float
OpSelectionMerge %343 None
OpBranchConditional %341 %342 %343
%342 = OpLabel
%344 = OpFunctionCall %bool %test_half
OpBranch %343
%343 = OpLabel
%345 = OpPhi %bool %false %340 %344 %342
OpSelectionMerge %347 None
OpBranchConditional %345 %346 %347
%346 = OpLabel
%348 = OpFunctionCall %bool %test_equality
OpBranch %347
%347 = OpLabel
%349 = OpPhi %bool %false %343 %348 %346
OpSelectionMerge %354 None
OpBranchConditional %349 %352 %353
%352 = OpLabel
%355 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%357 = OpLoad %v4float %355
OpStore %350 %357
OpBranch %354
%353 = OpLabel
%358 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%360 = OpLoad %v4float %358
OpStore %350 %360
OpBranch %354
%354 = OpLabel
%361 = OpLoad %v4float %350
OpReturnValue %361
OpFunctionEnd
