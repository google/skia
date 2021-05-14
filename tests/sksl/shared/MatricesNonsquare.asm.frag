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
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_half_b "test_half_b"
OpName %m23 "m23"
OpName %m24 "m24"
OpName %m32 "m32"
OpName %m34 "m34"
OpName %m42 "m42"
OpName %m43 "m43"
OpName %m22 "m22"
OpName %m33 "m33"
OpName %m44 "m44"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m23 "_1_m23"
OpName %_2_m24 "_2_m24"
OpName %_3_m32 "_3_m32"
OpName %_4_m34 "_4_m34"
OpName %_5_m42 "_5_m42"
OpName %_6_m43 "_6_m43"
OpName %_7_m22 "_7_m22"
OpName %_8_m33 "_8_m33"
OpName %_9_m44 "_9_m44"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %m23 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %m24 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %m32 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %m34 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %m42 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %m43 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %m22 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %m33 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %m44 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %bool
%v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
%float_23 = OpConstant %float 23
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%float_24 = OpConstant %float 24
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%float_32 = OpConstant %float 32
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
%float_34 = OpConstant %float 34
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_42 = OpConstant %float 42
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_43 = OpConstant %float 43
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%true = OpConstantTrue %bool
%103 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%false = OpConstantFalse %bool
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%v2bool = OpTypeVector %bool 2
%float_736 = OpConstant %float 736
%float_1462 = OpConstant %float 1462
%float_1008 = OpConstant %float 1008
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%test_half_b = OpFunction %bool None %24
%25 = OpLabel
%m23 = OpVariable %_ptr_Function_mat2v3float Function
%m24 = OpVariable %_ptr_Function_mat2v4float Function
%m32 = OpVariable %_ptr_Function_mat3v2float Function
%m34 = OpVariable %_ptr_Function_mat3v4float Function
%m42 = OpVariable %_ptr_Function_mat4v2float Function
%m43 = OpVariable %_ptr_Function_mat4v3float Function
%m22 = OpVariable %_ptr_Function_mat2v2float Function
%m33 = OpVariable %_ptr_Function_mat3v3float Function
%m44 = OpVariable %_ptr_Function_mat4v4float Function
%32 = OpCompositeConstruct %v3float %float_23 %float_0 %float_0
%33 = OpCompositeConstruct %v3float %float_0 %float_23 %float_0
%31 = OpCompositeConstruct %mat2v3float %32 %33
OpStore %m23 %31
%39 = OpCompositeConstruct %v4float %float_24 %float_0 %float_0 %float_0
%40 = OpCompositeConstruct %v4float %float_0 %float_24 %float_0 %float_0
%38 = OpCompositeConstruct %mat2v4float %39 %40
OpStore %m24 %38
%46 = OpCompositeConstruct %v2float %float_32 %float_0
%47 = OpCompositeConstruct %v2float %float_0 %float_32
%48 = OpCompositeConstruct %v2float %float_0 %float_0
%45 = OpCompositeConstruct %mat3v2float %46 %47 %48
OpStore %m32 %45
%54 = OpCompositeConstruct %v4float %float_34 %float_0 %float_0 %float_0
%55 = OpCompositeConstruct %v4float %float_0 %float_34 %float_0 %float_0
%56 = OpCompositeConstruct %v4float %float_0 %float_0 %float_34 %float_0
%53 = OpCompositeConstruct %mat3v4float %54 %55 %56
OpStore %m34 %53
%62 = OpCompositeConstruct %v2float %float_42 %float_0
%63 = OpCompositeConstruct %v2float %float_0 %float_42
%64 = OpCompositeConstruct %v2float %float_0 %float_0
%65 = OpCompositeConstruct %v2float %float_0 %float_0
%61 = OpCompositeConstruct %mat4v2float %62 %63 %64 %65
OpStore %m42 %61
%71 = OpCompositeConstruct %v3float %float_43 %float_0 %float_0
%72 = OpCompositeConstruct %v3float %float_0 %float_43 %float_0
%73 = OpCompositeConstruct %v3float %float_0 %float_0 %float_43
%74 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%70 = OpCompositeConstruct %mat4v3float %71 %72 %73 %74
OpStore %m43 %70
%78 = OpLoad %mat3v2float %m32
%79 = OpLoad %mat2v3float %m23
%80 = OpMatrixTimesMatrix %mat2v2float %78 %79
OpStore %m22 %80
%81 = OpLoad %mat2v2float %m22
%82 = OpLoad %mat2v2float %m22
%83 = OpMatrixTimesMatrix %mat2v2float %81 %82
OpStore %m22 %83
%87 = OpLoad %mat4v3float %m43
%88 = OpLoad %mat3v4float %m34
%89 = OpMatrixTimesMatrix %mat3v3float %87 %88
OpStore %m33 %89
%90 = OpLoad %mat3v3float %m33
%91 = OpLoad %mat3v3float %m33
%92 = OpMatrixTimesMatrix %mat3v3float %90 %91
OpStore %m33 %92
%96 = OpLoad %mat2v4float %m24
%97 = OpLoad %mat4v2float %m42
%98 = OpMatrixTimesMatrix %mat4v4float %96 %97
OpStore %m44 %98
%99 = OpLoad %mat4v4float %m44
%100 = OpLoad %mat4v4float %m44
%101 = OpMatrixTimesMatrix %mat4v4float %99 %100
OpStore %m44 %101
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %103
%104 = OpFunctionParameter %_ptr_Function_v2float
%105 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_5_m42 = OpVariable %_ptr_Function_mat4v2float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%_7_m22 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m33 = OpVariable %_ptr_Function_mat3v3float Function
%_9_m44 = OpVariable %_ptr_Function_mat4v4float Function
%369 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%110 = OpCompositeConstruct %v3float %float_23 %float_0 %float_0
%111 = OpCompositeConstruct %v3float %float_0 %float_23 %float_0
%109 = OpCompositeConstruct %mat2v3float %110 %111
OpStore %_1_m23 %109
%113 = OpLoad %bool %_0_ok
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%116 = OpLoad %mat2v3float %_1_m23
%118 = OpCompositeConstruct %v3float %float_23 %float_0 %float_0
%119 = OpCompositeConstruct %v3float %float_0 %float_23 %float_0
%117 = OpCompositeConstruct %mat2v3float %118 %119
%121 = OpCompositeExtract %v3float %116 0
%122 = OpCompositeExtract %v3float %117 0
%123 = OpFOrdEqual %v3bool %121 %122
%124 = OpAll %bool %123
%125 = OpCompositeExtract %v3float %116 1
%126 = OpCompositeExtract %v3float %117 1
%127 = OpFOrdEqual %v3bool %125 %126
%128 = OpAll %bool %127
%129 = OpLogicalAnd %bool %124 %128
OpBranch %115
%115 = OpLabel
%130 = OpPhi %bool %false %105 %129 %114
OpStore %_0_ok %130
%133 = OpCompositeConstruct %v4float %float_24 %float_0 %float_0 %float_0
%134 = OpCompositeConstruct %v4float %float_0 %float_24 %float_0 %float_0
%132 = OpCompositeConstruct %mat2v4float %133 %134
OpStore %_2_m24 %132
%135 = OpLoad %bool %_0_ok
OpSelectionMerge %137 None
OpBranchConditional %135 %136 %137
%136 = OpLabel
%138 = OpLoad %mat2v4float %_2_m24
%140 = OpCompositeConstruct %v4float %float_24 %float_0 %float_0 %float_0
%141 = OpCompositeConstruct %v4float %float_0 %float_24 %float_0 %float_0
%139 = OpCompositeConstruct %mat2v4float %140 %141
%143 = OpCompositeExtract %v4float %138 0
%144 = OpCompositeExtract %v4float %139 0
%145 = OpFOrdEqual %v4bool %143 %144
%146 = OpAll %bool %145
%147 = OpCompositeExtract %v4float %138 1
%148 = OpCompositeExtract %v4float %139 1
%149 = OpFOrdEqual %v4bool %147 %148
%150 = OpAll %bool %149
%151 = OpLogicalAnd %bool %146 %150
OpBranch %137
%137 = OpLabel
%152 = OpPhi %bool %false %115 %151 %136
OpStore %_0_ok %152
%155 = OpCompositeConstruct %v2float %float_32 %float_0
%156 = OpCompositeConstruct %v2float %float_0 %float_32
%157 = OpCompositeConstruct %v2float %float_0 %float_0
%154 = OpCompositeConstruct %mat3v2float %155 %156 %157
OpStore %_3_m32 %154
%158 = OpLoad %bool %_0_ok
OpSelectionMerge %160 None
OpBranchConditional %158 %159 %160
%159 = OpLabel
%161 = OpLoad %mat3v2float %_3_m32
%163 = OpCompositeConstruct %v2float %float_32 %float_0
%164 = OpCompositeConstruct %v2float %float_0 %float_32
%165 = OpCompositeConstruct %v2float %float_0 %float_0
%162 = OpCompositeConstruct %mat3v2float %163 %164 %165
%167 = OpCompositeExtract %v2float %161 0
%168 = OpCompositeExtract %v2float %162 0
%169 = OpFOrdEqual %v2bool %167 %168
%170 = OpAll %bool %169
%171 = OpCompositeExtract %v2float %161 1
%172 = OpCompositeExtract %v2float %162 1
%173 = OpFOrdEqual %v2bool %171 %172
%174 = OpAll %bool %173
%175 = OpLogicalAnd %bool %170 %174
%176 = OpCompositeExtract %v2float %161 2
%177 = OpCompositeExtract %v2float %162 2
%178 = OpFOrdEqual %v2bool %176 %177
%179 = OpAll %bool %178
%180 = OpLogicalAnd %bool %175 %179
OpBranch %160
%160 = OpLabel
%181 = OpPhi %bool %false %137 %180 %159
OpStore %_0_ok %181
%184 = OpCompositeConstruct %v4float %float_34 %float_0 %float_0 %float_0
%185 = OpCompositeConstruct %v4float %float_0 %float_34 %float_0 %float_0
%186 = OpCompositeConstruct %v4float %float_0 %float_0 %float_34 %float_0
%183 = OpCompositeConstruct %mat3v4float %184 %185 %186
OpStore %_4_m34 %183
%187 = OpLoad %bool %_0_ok
OpSelectionMerge %189 None
OpBranchConditional %187 %188 %189
%188 = OpLabel
%190 = OpLoad %mat3v4float %_4_m34
%192 = OpCompositeConstruct %v4float %float_34 %float_0 %float_0 %float_0
%193 = OpCompositeConstruct %v4float %float_0 %float_34 %float_0 %float_0
%194 = OpCompositeConstruct %v4float %float_0 %float_0 %float_34 %float_0
%191 = OpCompositeConstruct %mat3v4float %192 %193 %194
%195 = OpCompositeExtract %v4float %190 0
%196 = OpCompositeExtract %v4float %191 0
%197 = OpFOrdEqual %v4bool %195 %196
%198 = OpAll %bool %197
%199 = OpCompositeExtract %v4float %190 1
%200 = OpCompositeExtract %v4float %191 1
%201 = OpFOrdEqual %v4bool %199 %200
%202 = OpAll %bool %201
%203 = OpLogicalAnd %bool %198 %202
%204 = OpCompositeExtract %v4float %190 2
%205 = OpCompositeExtract %v4float %191 2
%206 = OpFOrdEqual %v4bool %204 %205
%207 = OpAll %bool %206
%208 = OpLogicalAnd %bool %203 %207
OpBranch %189
%189 = OpLabel
%209 = OpPhi %bool %false %160 %208 %188
OpStore %_0_ok %209
%212 = OpCompositeConstruct %v2float %float_42 %float_0
%213 = OpCompositeConstruct %v2float %float_0 %float_42
%214 = OpCompositeConstruct %v2float %float_0 %float_0
%215 = OpCompositeConstruct %v2float %float_0 %float_0
%211 = OpCompositeConstruct %mat4v2float %212 %213 %214 %215
OpStore %_5_m42 %211
%216 = OpLoad %bool %_0_ok
OpSelectionMerge %218 None
OpBranchConditional %216 %217 %218
%217 = OpLabel
%219 = OpLoad %mat4v2float %_5_m42
%221 = OpCompositeConstruct %v2float %float_42 %float_0
%222 = OpCompositeConstruct %v2float %float_0 %float_42
%223 = OpCompositeConstruct %v2float %float_0 %float_0
%224 = OpCompositeConstruct %v2float %float_0 %float_0
%220 = OpCompositeConstruct %mat4v2float %221 %222 %223 %224
%225 = OpCompositeExtract %v2float %219 0
%226 = OpCompositeExtract %v2float %220 0
%227 = OpFOrdEqual %v2bool %225 %226
%228 = OpAll %bool %227
%229 = OpCompositeExtract %v2float %219 1
%230 = OpCompositeExtract %v2float %220 1
%231 = OpFOrdEqual %v2bool %229 %230
%232 = OpAll %bool %231
%233 = OpLogicalAnd %bool %228 %232
%234 = OpCompositeExtract %v2float %219 2
%235 = OpCompositeExtract %v2float %220 2
%236 = OpFOrdEqual %v2bool %234 %235
%237 = OpAll %bool %236
%238 = OpLogicalAnd %bool %233 %237
%239 = OpCompositeExtract %v2float %219 3
%240 = OpCompositeExtract %v2float %220 3
%241 = OpFOrdEqual %v2bool %239 %240
%242 = OpAll %bool %241
%243 = OpLogicalAnd %bool %238 %242
OpBranch %218
%218 = OpLabel
%244 = OpPhi %bool %false %189 %243 %217
OpStore %_0_ok %244
%247 = OpCompositeConstruct %v3float %float_43 %float_0 %float_0
%248 = OpCompositeConstruct %v3float %float_0 %float_43 %float_0
%249 = OpCompositeConstruct %v3float %float_0 %float_0 %float_43
%250 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%246 = OpCompositeConstruct %mat4v3float %247 %248 %249 %250
OpStore %_6_m43 %246
%251 = OpLoad %bool %_0_ok
OpSelectionMerge %253 None
OpBranchConditional %251 %252 %253
%252 = OpLabel
%254 = OpLoad %mat4v3float %_6_m43
%256 = OpCompositeConstruct %v3float %float_43 %float_0 %float_0
%257 = OpCompositeConstruct %v3float %float_0 %float_43 %float_0
%258 = OpCompositeConstruct %v3float %float_0 %float_0 %float_43
%259 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%255 = OpCompositeConstruct %mat4v3float %256 %257 %258 %259
%260 = OpCompositeExtract %v3float %254 0
%261 = OpCompositeExtract %v3float %255 0
%262 = OpFOrdEqual %v3bool %260 %261
%263 = OpAll %bool %262
%264 = OpCompositeExtract %v3float %254 1
%265 = OpCompositeExtract %v3float %255 1
%266 = OpFOrdEqual %v3bool %264 %265
%267 = OpAll %bool %266
%268 = OpLogicalAnd %bool %263 %267
%269 = OpCompositeExtract %v3float %254 2
%270 = OpCompositeExtract %v3float %255 2
%271 = OpFOrdEqual %v3bool %269 %270
%272 = OpAll %bool %271
%273 = OpLogicalAnd %bool %268 %272
%274 = OpCompositeExtract %v3float %254 3
%275 = OpCompositeExtract %v3float %255 3
%276 = OpFOrdEqual %v3bool %274 %275
%277 = OpAll %bool %276
%278 = OpLogicalAnd %bool %273 %277
OpBranch %253
%253 = OpLabel
%279 = OpPhi %bool %false %218 %278 %252
OpStore %_0_ok %279
%281 = OpLoad %mat3v2float %_3_m32
%282 = OpLoad %mat2v3float %_1_m23
%283 = OpMatrixTimesMatrix %mat2v2float %281 %282
OpStore %_7_m22 %283
%284 = OpLoad %bool %_0_ok
OpSelectionMerge %286 None
OpBranchConditional %284 %285 %286
%285 = OpLabel
%287 = OpLoad %mat2v2float %_7_m22
%290 = OpCompositeConstruct %v2float %float_736 %float_0
%291 = OpCompositeConstruct %v2float %float_0 %float_736
%289 = OpCompositeConstruct %mat2v2float %290 %291
%292 = OpCompositeExtract %v2float %287 0
%293 = OpCompositeExtract %v2float %289 0
%294 = OpFOrdEqual %v2bool %292 %293
%295 = OpAll %bool %294
%296 = OpCompositeExtract %v2float %287 1
%297 = OpCompositeExtract %v2float %289 1
%298 = OpFOrdEqual %v2bool %296 %297
%299 = OpAll %bool %298
%300 = OpLogicalAnd %bool %295 %299
OpBranch %286
%286 = OpLabel
%301 = OpPhi %bool %false %253 %300 %285
OpStore %_0_ok %301
%303 = OpLoad %mat4v3float %_6_m43
%304 = OpLoad %mat3v4float %_4_m34
%305 = OpMatrixTimesMatrix %mat3v3float %303 %304
OpStore %_8_m33 %305
%306 = OpLoad %bool %_0_ok
OpSelectionMerge %308 None
OpBranchConditional %306 %307 %308
%307 = OpLabel
%309 = OpLoad %mat3v3float %_8_m33
%312 = OpCompositeConstruct %v3float %float_1462 %float_0 %float_0
%313 = OpCompositeConstruct %v3float %float_0 %float_1462 %float_0
%314 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1462
%311 = OpCompositeConstruct %mat3v3float %312 %313 %314
%315 = OpCompositeExtract %v3float %309 0
%316 = OpCompositeExtract %v3float %311 0
%317 = OpFOrdEqual %v3bool %315 %316
%318 = OpAll %bool %317
%319 = OpCompositeExtract %v3float %309 1
%320 = OpCompositeExtract %v3float %311 1
%321 = OpFOrdEqual %v3bool %319 %320
%322 = OpAll %bool %321
%323 = OpLogicalAnd %bool %318 %322
%324 = OpCompositeExtract %v3float %309 2
%325 = OpCompositeExtract %v3float %311 2
%326 = OpFOrdEqual %v3bool %324 %325
%327 = OpAll %bool %326
%328 = OpLogicalAnd %bool %323 %327
OpBranch %308
%308 = OpLabel
%329 = OpPhi %bool %false %286 %328 %307
OpStore %_0_ok %329
%331 = OpLoad %mat2v4float %_2_m24
%332 = OpLoad %mat4v2float %_5_m42
%333 = OpMatrixTimesMatrix %mat4v4float %331 %332
OpStore %_9_m44 %333
%334 = OpLoad %bool %_0_ok
OpSelectionMerge %336 None
OpBranchConditional %334 %335 %336
%335 = OpLabel
%337 = OpLoad %mat4v4float %_9_m44
%340 = OpCompositeConstruct %v4float %float_1008 %float_0 %float_0 %float_0
%341 = OpCompositeConstruct %v4float %float_0 %float_1008 %float_0 %float_0
%342 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%343 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%339 = OpCompositeConstruct %mat4v4float %340 %341 %342 %343
%344 = OpCompositeExtract %v4float %337 0
%345 = OpCompositeExtract %v4float %339 0
%346 = OpFOrdEqual %v4bool %344 %345
%347 = OpAll %bool %346
%348 = OpCompositeExtract %v4float %337 1
%349 = OpCompositeExtract %v4float %339 1
%350 = OpFOrdEqual %v4bool %348 %349
%351 = OpAll %bool %350
%352 = OpLogicalAnd %bool %347 %351
%353 = OpCompositeExtract %v4float %337 2
%354 = OpCompositeExtract %v4float %339 2
%355 = OpFOrdEqual %v4bool %353 %354
%356 = OpAll %bool %355
%357 = OpLogicalAnd %bool %352 %356
%358 = OpCompositeExtract %v4float %337 3
%359 = OpCompositeExtract %v4float %339 3
%360 = OpFOrdEqual %v4bool %358 %359
%361 = OpAll %bool %360
%362 = OpLogicalAnd %bool %357 %361
OpBranch %336
%336 = OpLabel
%363 = OpPhi %bool %false %308 %362 %335
OpStore %_0_ok %363
%364 = OpLoad %bool %_0_ok
OpSelectionMerge %366 None
OpBranchConditional %364 %365 %366
%365 = OpLabel
%367 = OpFunctionCall %bool %test_half_b
OpBranch %366
%366 = OpLabel
%368 = OpPhi %bool %false %336 %367 %365
OpSelectionMerge %373 None
OpBranchConditional %368 %371 %372
%371 = OpLabel
%374 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%378 = OpLoad %v4float %374
OpStore %369 %378
OpBranch %373
%372 = OpLabel
%379 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%381 = OpLoad %v4float %379
OpStore %369 %381
OpBranch %373
%373 = OpLabel
%382 = OpLoad %v4float %369
OpReturnValue %382
OpFunctionEnd
