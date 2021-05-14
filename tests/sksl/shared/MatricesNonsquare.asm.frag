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
OpName %ok "ok"
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
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %m24 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %m32 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %m34 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %m42 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %m43 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %m22 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %m33 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %m44 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %401 RelaxedPrecision
OpDecorate %429 RelaxedPrecision
OpDecorate %464 RelaxedPrecision
OpDecorate %497 RelaxedPrecision
OpDecorate %518 RelaxedPrecision
OpDecorate %545 RelaxedPrecision
OpDecorate %574 RelaxedPrecision
OpDecorate %585 RelaxedPrecision
OpDecorate %587 RelaxedPrecision
OpDecorate %588 RelaxedPrecision
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
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
%float_2 = OpConstant %float 2
%false = OpConstantFalse %bool
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_v3float = OpTypePointer Function %v3float
%46 = OpConstantComposite %v3float %float_2 %float_0 %float_0
%v3bool = OpTypeVector %bool 3
%int_1 = OpConstant %int 1
%55 = OpConstantComposite %v3float %float_0 %float_2 %float_0
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%float_3 = OpConstant %float 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%73 = OpConstantComposite %v4float %float_3 %float_0 %float_0 %float_0
%v4bool = OpTypeVector %bool 4
%81 = OpConstantComposite %v4float %float_0 %float_3 %float_0 %float_0
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%float_4 = OpConstant %float 4
%99 = OpConstantComposite %v2float %float_4 %float_0
%v2bool = OpTypeVector %bool 2
%107 = OpConstantComposite %v2float %float_0 %float_4
%int_2 = OpConstant %int 2
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
%float_5 = OpConstant %float 5
%133 = OpConstantComposite %v4float %float_5 %float_0 %float_0 %float_0
%140 = OpConstantComposite %v4float %float_0 %float_5 %float_0 %float_0
%148 = OpConstantComposite %v4float %float_0 %float_0 %float_5 %float_0
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_6 = OpConstant %float 6
%167 = OpConstantComposite %v2float %float_6 %float_0
%174 = OpConstantComposite %v2float %float_0 %float_6
%int_3 = OpConstant %int 3
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_7 = OpConstant %float 7
%208 = OpConstantComposite %v3float %float_7 %float_0 %float_0
%215 = OpConstantComposite %v3float %float_0 %float_7 %float_0
%223 = OpConstantComposite %v3float %float_0 %float_0 %float_7
%231 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_8 = OpConstant %float 8
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_35 = OpConstant %float 35
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_18 = OpConstant %float 18
%327 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
%ok = OpVariable %_ptr_Function_bool Function
%m23 = OpVariable %_ptr_Function_mat2v3float Function
%m24 = OpVariable %_ptr_Function_mat2v4float Function
%m32 = OpVariable %_ptr_Function_mat3v2float Function
%m34 = OpVariable %_ptr_Function_mat3v4float Function
%m42 = OpVariable %_ptr_Function_mat4v2float Function
%m43 = OpVariable %_ptr_Function_mat4v3float Function
%m22 = OpVariable %_ptr_Function_mat2v2float Function
%m33 = OpVariable %_ptr_Function_mat3v3float Function
%m44 = OpVariable %_ptr_Function_mat4v4float Function
OpStore %ok %true
%35 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%36 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%34 = OpCompositeConstruct %mat2v3float %35 %36
OpStore %m23 %34
%38 = OpLoad %bool %ok
OpSelectionMerge %40 None
OpBranchConditional %38 %39 %40
%39 = OpLabel
%43 = OpAccessChain %_ptr_Function_v3float %m23 %int_0
%45 = OpLoad %v3float %43
%47 = OpFOrdEqual %v3bool %45 %46
%49 = OpAll %bool %47
OpSelectionMerge %51 None
OpBranchConditional %49 %50 %51
%50 = OpLabel
%53 = OpAccessChain %_ptr_Function_v3float %m23 %int_1
%54 = OpLoad %v3float %53
%56 = OpFOrdEqual %v3bool %54 %55
%57 = OpAll %bool %56
OpBranch %51
%51 = OpLabel
%58 = OpPhi %bool %false %39 %57 %50
OpBranch %40
%40 = OpLabel
%59 = OpPhi %bool %false %25 %58 %51
OpStore %ok %59
%65 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%66 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%64 = OpCompositeConstruct %mat2v4float %65 %66
OpStore %m24 %64
%67 = OpLoad %bool %ok
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%70 = OpAccessChain %_ptr_Function_v4float %m24 %int_0
%72 = OpLoad %v4float %70
%74 = OpFOrdEqual %v4bool %72 %73
%76 = OpAll %bool %74
OpSelectionMerge %78 None
OpBranchConditional %76 %77 %78
%77 = OpLabel
%79 = OpAccessChain %_ptr_Function_v4float %m24 %int_1
%80 = OpLoad %v4float %79
%82 = OpFOrdEqual %v4bool %80 %81
%83 = OpAll %bool %82
OpBranch %78
%78 = OpLabel
%84 = OpPhi %bool %false %68 %83 %77
OpBranch %69
%69 = OpLabel
%85 = OpPhi %bool %false %40 %84 %78
OpStore %ok %85
%91 = OpCompositeConstruct %v2float %float_4 %float_0
%92 = OpCompositeConstruct %v2float %float_0 %float_4
%93 = OpCompositeConstruct %v2float %float_0 %float_0
%90 = OpCompositeConstruct %mat3v2float %91 %92 %93
OpStore %m32 %90
%94 = OpLoad %bool %ok
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%97 = OpAccessChain %_ptr_Function_v2float %m32 %int_0
%98 = OpLoad %v2float %97
%100 = OpFOrdEqual %v2bool %98 %99
%102 = OpAll %bool %100
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%105 = OpAccessChain %_ptr_Function_v2float %m32 %int_1
%106 = OpLoad %v2float %105
%108 = OpFOrdEqual %v2bool %106 %107
%109 = OpAll %bool %108
OpBranch %104
%104 = OpLabel
%110 = OpPhi %bool %false %95 %109 %103
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%114 = OpAccessChain %_ptr_Function_v2float %m32 %int_2
%115 = OpLoad %v2float %114
%116 = OpFOrdEqual %v2bool %115 %20
%117 = OpAll %bool %116
OpBranch %112
%112 = OpLabel
%118 = OpPhi %bool %false %104 %117 %111
OpBranch %96
%96 = OpLabel
%119 = OpPhi %bool %false %69 %118 %112
OpStore %ok %119
%125 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%126 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%127 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%124 = OpCompositeConstruct %mat3v4float %125 %126 %127
OpStore %m34 %124
%128 = OpLoad %bool %ok
OpSelectionMerge %130 None
OpBranchConditional %128 %129 %130
%129 = OpLabel
%131 = OpAccessChain %_ptr_Function_v4float %m34 %int_0
%132 = OpLoad %v4float %131
%134 = OpFOrdEqual %v4bool %132 %133
%135 = OpAll %bool %134
OpSelectionMerge %137 None
OpBranchConditional %135 %136 %137
%136 = OpLabel
%138 = OpAccessChain %_ptr_Function_v4float %m34 %int_1
%139 = OpLoad %v4float %138
%141 = OpFOrdEqual %v4bool %139 %140
%142 = OpAll %bool %141
OpBranch %137
%137 = OpLabel
%143 = OpPhi %bool %false %129 %142 %136
OpSelectionMerge %145 None
OpBranchConditional %143 %144 %145
%144 = OpLabel
%146 = OpAccessChain %_ptr_Function_v4float %m34 %int_2
%147 = OpLoad %v4float %146
%149 = OpFOrdEqual %v4bool %147 %148
%150 = OpAll %bool %149
OpBranch %145
%145 = OpLabel
%151 = OpPhi %bool %false %137 %150 %144
OpBranch %130
%130 = OpLabel
%152 = OpPhi %bool %false %96 %151 %145
OpStore %ok %152
%158 = OpCompositeConstruct %v2float %float_6 %float_0
%159 = OpCompositeConstruct %v2float %float_0 %float_6
%160 = OpCompositeConstruct %v2float %float_0 %float_0
%161 = OpCompositeConstruct %v2float %float_0 %float_0
%157 = OpCompositeConstruct %mat4v2float %158 %159 %160 %161
OpStore %m42 %157
%162 = OpLoad %bool %ok
OpSelectionMerge %164 None
OpBranchConditional %162 %163 %164
%163 = OpLabel
%165 = OpAccessChain %_ptr_Function_v2float %m42 %int_0
%166 = OpLoad %v2float %165
%168 = OpFOrdEqual %v2bool %166 %167
%169 = OpAll %bool %168
OpSelectionMerge %171 None
OpBranchConditional %169 %170 %171
%170 = OpLabel
%172 = OpAccessChain %_ptr_Function_v2float %m42 %int_1
%173 = OpLoad %v2float %172
%175 = OpFOrdEqual %v2bool %173 %174
%176 = OpAll %bool %175
OpBranch %171
%171 = OpLabel
%177 = OpPhi %bool %false %163 %176 %170
OpSelectionMerge %179 None
OpBranchConditional %177 %178 %179
%178 = OpLabel
%180 = OpAccessChain %_ptr_Function_v2float %m42 %int_2
%181 = OpLoad %v2float %180
%182 = OpFOrdEqual %v2bool %181 %20
%183 = OpAll %bool %182
OpBranch %179
%179 = OpLabel
%184 = OpPhi %bool %false %171 %183 %178
OpSelectionMerge %186 None
OpBranchConditional %184 %185 %186
%185 = OpLabel
%188 = OpAccessChain %_ptr_Function_v2float %m42 %int_3
%189 = OpLoad %v2float %188
%190 = OpFOrdEqual %v2bool %189 %20
%191 = OpAll %bool %190
OpBranch %186
%186 = OpLabel
%192 = OpPhi %bool %false %179 %191 %185
OpBranch %164
%164 = OpLabel
%193 = OpPhi %bool %false %130 %192 %186
OpStore %ok %193
%199 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%200 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%201 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%202 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%198 = OpCompositeConstruct %mat4v3float %199 %200 %201 %202
OpStore %m43 %198
%203 = OpLoad %bool %ok
OpSelectionMerge %205 None
OpBranchConditional %203 %204 %205
%204 = OpLabel
%206 = OpAccessChain %_ptr_Function_v3float %m43 %int_0
%207 = OpLoad %v3float %206
%209 = OpFOrdEqual %v3bool %207 %208
%210 = OpAll %bool %209
OpSelectionMerge %212 None
OpBranchConditional %210 %211 %212
%211 = OpLabel
%213 = OpAccessChain %_ptr_Function_v3float %m43 %int_1
%214 = OpLoad %v3float %213
%216 = OpFOrdEqual %v3bool %214 %215
%217 = OpAll %bool %216
OpBranch %212
%212 = OpLabel
%218 = OpPhi %bool %false %204 %217 %211
OpSelectionMerge %220 None
OpBranchConditional %218 %219 %220
%219 = OpLabel
%221 = OpAccessChain %_ptr_Function_v3float %m43 %int_2
%222 = OpLoad %v3float %221
%224 = OpFOrdEqual %v3bool %222 %223
%225 = OpAll %bool %224
OpBranch %220
%220 = OpLabel
%226 = OpPhi %bool %false %212 %225 %219
OpSelectionMerge %228 None
OpBranchConditional %226 %227 %228
%227 = OpLabel
%229 = OpAccessChain %_ptr_Function_v3float %m43 %int_3
%230 = OpLoad %v3float %229
%232 = OpFOrdEqual %v3bool %230 %231
%233 = OpAll %bool %232
OpBranch %228
%228 = OpLabel
%234 = OpPhi %bool %false %220 %233 %227
OpBranch %205
%205 = OpLabel
%235 = OpPhi %bool %false %164 %234 %228
OpStore %ok %235
%239 = OpLoad %mat3v2float %m32
%240 = OpLoad %mat2v3float %m23
%241 = OpMatrixTimesMatrix %mat2v2float %239 %240
OpStore %m22 %241
%242 = OpLoad %bool %ok
OpSelectionMerge %244 None
OpBranchConditional %242 %243 %244
%243 = OpLabel
%245 = OpLoad %mat2v2float %m22
%248 = OpCompositeConstruct %v2float %float_8 %float_0
%249 = OpCompositeConstruct %v2float %float_0 %float_8
%247 = OpCompositeConstruct %mat2v2float %248 %249
%250 = OpCompositeExtract %v2float %245 0
%251 = OpCompositeExtract %v2float %247 0
%252 = OpFOrdEqual %v2bool %250 %251
%253 = OpAll %bool %252
%254 = OpCompositeExtract %v2float %245 1
%255 = OpCompositeExtract %v2float %247 1
%256 = OpFOrdEqual %v2bool %254 %255
%257 = OpAll %bool %256
%258 = OpLogicalAnd %bool %253 %257
OpBranch %244
%244 = OpLabel
%259 = OpPhi %bool %false %205 %258 %243
OpStore %ok %259
%263 = OpLoad %mat4v3float %m43
%264 = OpLoad %mat3v4float %m34
%265 = OpMatrixTimesMatrix %mat3v3float %263 %264
OpStore %m33 %265
%266 = OpLoad %bool %ok
OpSelectionMerge %268 None
OpBranchConditional %266 %267 %268
%267 = OpLabel
%269 = OpLoad %mat3v3float %m33
%272 = OpCompositeConstruct %v3float %float_35 %float_0 %float_0
%273 = OpCompositeConstruct %v3float %float_0 %float_35 %float_0
%274 = OpCompositeConstruct %v3float %float_0 %float_0 %float_35
%271 = OpCompositeConstruct %mat3v3float %272 %273 %274
%275 = OpCompositeExtract %v3float %269 0
%276 = OpCompositeExtract %v3float %271 0
%277 = OpFOrdEqual %v3bool %275 %276
%278 = OpAll %bool %277
%279 = OpCompositeExtract %v3float %269 1
%280 = OpCompositeExtract %v3float %271 1
%281 = OpFOrdEqual %v3bool %279 %280
%282 = OpAll %bool %281
%283 = OpLogicalAnd %bool %278 %282
%284 = OpCompositeExtract %v3float %269 2
%285 = OpCompositeExtract %v3float %271 2
%286 = OpFOrdEqual %v3bool %284 %285
%287 = OpAll %bool %286
%288 = OpLogicalAnd %bool %283 %287
OpBranch %268
%268 = OpLabel
%289 = OpPhi %bool %false %244 %288 %267
OpStore %ok %289
%293 = OpLoad %mat2v4float %m24
%294 = OpLoad %mat4v2float %m42
%295 = OpMatrixTimesMatrix %mat4v4float %293 %294
OpStore %m44 %295
%296 = OpLoad %bool %ok
OpSelectionMerge %298 None
OpBranchConditional %296 %297 %298
%297 = OpLabel
%299 = OpLoad %mat4v4float %m44
%302 = OpCompositeConstruct %v4float %float_18 %float_0 %float_0 %float_0
%303 = OpCompositeConstruct %v4float %float_0 %float_18 %float_0 %float_0
%304 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%305 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%301 = OpCompositeConstruct %mat4v4float %302 %303 %304 %305
%306 = OpCompositeExtract %v4float %299 0
%307 = OpCompositeExtract %v4float %301 0
%308 = OpFOrdEqual %v4bool %306 %307
%309 = OpAll %bool %308
%310 = OpCompositeExtract %v4float %299 1
%311 = OpCompositeExtract %v4float %301 1
%312 = OpFOrdEqual %v4bool %310 %311
%313 = OpAll %bool %312
%314 = OpLogicalAnd %bool %309 %313
%315 = OpCompositeExtract %v4float %299 2
%316 = OpCompositeExtract %v4float %301 2
%317 = OpFOrdEqual %v4bool %315 %316
%318 = OpAll %bool %317
%319 = OpLogicalAnd %bool %314 %318
%320 = OpCompositeExtract %v4float %299 3
%321 = OpCompositeExtract %v4float %301 3
%322 = OpFOrdEqual %v4bool %320 %321
%323 = OpAll %bool %322
%324 = OpLogicalAnd %bool %319 %323
OpBranch %298
%298 = OpLabel
%325 = OpPhi %bool %false %268 %324 %297
OpStore %ok %325
%326 = OpLoad %bool %ok
OpReturnValue %326
OpFunctionEnd
%main = OpFunction %v4float None %327
%328 = OpFunctionParameter %_ptr_Function_v2float
%329 = OpLabel
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
%579 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%333 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%334 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%332 = OpCompositeConstruct %mat2v3float %333 %334
OpStore %_1_m23 %332
%335 = OpLoad %bool %_0_ok
OpSelectionMerge %337 None
OpBranchConditional %335 %336 %337
%336 = OpLabel
%338 = OpAccessChain %_ptr_Function_v3float %_1_m23 %int_0
%339 = OpLoad %v3float %338
%340 = OpFOrdEqual %v3bool %339 %46
%341 = OpAll %bool %340
OpSelectionMerge %343 None
OpBranchConditional %341 %342 %343
%342 = OpLabel
%344 = OpAccessChain %_ptr_Function_v3float %_1_m23 %int_1
%345 = OpLoad %v3float %344
%346 = OpFOrdEqual %v3bool %345 %55
%347 = OpAll %bool %346
OpBranch %343
%343 = OpLabel
%348 = OpPhi %bool %false %336 %347 %342
OpBranch %337
%337 = OpLabel
%349 = OpPhi %bool %false %329 %348 %343
OpStore %_0_ok %349
%352 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%353 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%351 = OpCompositeConstruct %mat2v4float %352 %353
OpStore %_2_m24 %351
%354 = OpLoad %bool %_0_ok
OpSelectionMerge %356 None
OpBranchConditional %354 %355 %356
%355 = OpLabel
%357 = OpAccessChain %_ptr_Function_v4float %_2_m24 %int_0
%358 = OpLoad %v4float %357
%359 = OpFOrdEqual %v4bool %358 %73
%360 = OpAll %bool %359
OpSelectionMerge %362 None
OpBranchConditional %360 %361 %362
%361 = OpLabel
%363 = OpAccessChain %_ptr_Function_v4float %_2_m24 %int_1
%364 = OpLoad %v4float %363
%365 = OpFOrdEqual %v4bool %364 %81
%366 = OpAll %bool %365
OpBranch %362
%362 = OpLabel
%367 = OpPhi %bool %false %355 %366 %361
OpBranch %356
%356 = OpLabel
%368 = OpPhi %bool %false %337 %367 %362
OpStore %_0_ok %368
%371 = OpCompositeConstruct %v2float %float_4 %float_0
%372 = OpCompositeConstruct %v2float %float_0 %float_4
%373 = OpCompositeConstruct %v2float %float_0 %float_0
%370 = OpCompositeConstruct %mat3v2float %371 %372 %373
OpStore %_3_m32 %370
%374 = OpLoad %bool %_0_ok
OpSelectionMerge %376 None
OpBranchConditional %374 %375 %376
%375 = OpLabel
%377 = OpAccessChain %_ptr_Function_v2float %_3_m32 %int_0
%378 = OpLoad %v2float %377
%379 = OpFOrdEqual %v2bool %378 %99
%380 = OpAll %bool %379
OpSelectionMerge %382 None
OpBranchConditional %380 %381 %382
%381 = OpLabel
%383 = OpAccessChain %_ptr_Function_v2float %_3_m32 %int_1
%384 = OpLoad %v2float %383
%385 = OpFOrdEqual %v2bool %384 %107
%386 = OpAll %bool %385
OpBranch %382
%382 = OpLabel
%387 = OpPhi %bool %false %375 %386 %381
OpSelectionMerge %389 None
OpBranchConditional %387 %388 %389
%388 = OpLabel
%390 = OpAccessChain %_ptr_Function_v2float %_3_m32 %int_2
%391 = OpLoad %v2float %390
%392 = OpFOrdEqual %v2bool %391 %20
%393 = OpAll %bool %392
OpBranch %389
%389 = OpLabel
%394 = OpPhi %bool %false %382 %393 %388
OpBranch %376
%376 = OpLabel
%395 = OpPhi %bool %false %356 %394 %389
OpStore %_0_ok %395
%398 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%399 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%400 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%397 = OpCompositeConstruct %mat3v4float %398 %399 %400
OpStore %_4_m34 %397
%401 = OpLoad %bool %_0_ok
OpSelectionMerge %403 None
OpBranchConditional %401 %402 %403
%402 = OpLabel
%404 = OpAccessChain %_ptr_Function_v4float %_4_m34 %int_0
%405 = OpLoad %v4float %404
%406 = OpFOrdEqual %v4bool %405 %133
%407 = OpAll %bool %406
OpSelectionMerge %409 None
OpBranchConditional %407 %408 %409
%408 = OpLabel
%410 = OpAccessChain %_ptr_Function_v4float %_4_m34 %int_1
%411 = OpLoad %v4float %410
%412 = OpFOrdEqual %v4bool %411 %140
%413 = OpAll %bool %412
OpBranch %409
%409 = OpLabel
%414 = OpPhi %bool %false %402 %413 %408
OpSelectionMerge %416 None
OpBranchConditional %414 %415 %416
%415 = OpLabel
%417 = OpAccessChain %_ptr_Function_v4float %_4_m34 %int_2
%418 = OpLoad %v4float %417
%419 = OpFOrdEqual %v4bool %418 %148
%420 = OpAll %bool %419
OpBranch %416
%416 = OpLabel
%421 = OpPhi %bool %false %409 %420 %415
OpBranch %403
%403 = OpLabel
%422 = OpPhi %bool %false %376 %421 %416
OpStore %_0_ok %422
%425 = OpCompositeConstruct %v2float %float_6 %float_0
%426 = OpCompositeConstruct %v2float %float_0 %float_6
%427 = OpCompositeConstruct %v2float %float_0 %float_0
%428 = OpCompositeConstruct %v2float %float_0 %float_0
%424 = OpCompositeConstruct %mat4v2float %425 %426 %427 %428
OpStore %_5_m42 %424
%429 = OpLoad %bool %_0_ok
OpSelectionMerge %431 None
OpBranchConditional %429 %430 %431
%430 = OpLabel
%432 = OpAccessChain %_ptr_Function_v2float %_5_m42 %int_0
%433 = OpLoad %v2float %432
%434 = OpFOrdEqual %v2bool %433 %167
%435 = OpAll %bool %434
OpSelectionMerge %437 None
OpBranchConditional %435 %436 %437
%436 = OpLabel
%438 = OpAccessChain %_ptr_Function_v2float %_5_m42 %int_1
%439 = OpLoad %v2float %438
%440 = OpFOrdEqual %v2bool %439 %174
%441 = OpAll %bool %440
OpBranch %437
%437 = OpLabel
%442 = OpPhi %bool %false %430 %441 %436
OpSelectionMerge %444 None
OpBranchConditional %442 %443 %444
%443 = OpLabel
%445 = OpAccessChain %_ptr_Function_v2float %_5_m42 %int_2
%446 = OpLoad %v2float %445
%447 = OpFOrdEqual %v2bool %446 %20
%448 = OpAll %bool %447
OpBranch %444
%444 = OpLabel
%449 = OpPhi %bool %false %437 %448 %443
OpSelectionMerge %451 None
OpBranchConditional %449 %450 %451
%450 = OpLabel
%452 = OpAccessChain %_ptr_Function_v2float %_5_m42 %int_3
%453 = OpLoad %v2float %452
%454 = OpFOrdEqual %v2bool %453 %20
%455 = OpAll %bool %454
OpBranch %451
%451 = OpLabel
%456 = OpPhi %bool %false %444 %455 %450
OpBranch %431
%431 = OpLabel
%457 = OpPhi %bool %false %403 %456 %451
OpStore %_0_ok %457
%460 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%461 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%462 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%463 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%459 = OpCompositeConstruct %mat4v3float %460 %461 %462 %463
OpStore %_6_m43 %459
%464 = OpLoad %bool %_0_ok
OpSelectionMerge %466 None
OpBranchConditional %464 %465 %466
%465 = OpLabel
%467 = OpAccessChain %_ptr_Function_v3float %_6_m43 %int_0
%468 = OpLoad %v3float %467
%469 = OpFOrdEqual %v3bool %468 %208
%470 = OpAll %bool %469
OpSelectionMerge %472 None
OpBranchConditional %470 %471 %472
%471 = OpLabel
%473 = OpAccessChain %_ptr_Function_v3float %_6_m43 %int_1
%474 = OpLoad %v3float %473
%475 = OpFOrdEqual %v3bool %474 %215
%476 = OpAll %bool %475
OpBranch %472
%472 = OpLabel
%477 = OpPhi %bool %false %465 %476 %471
OpSelectionMerge %479 None
OpBranchConditional %477 %478 %479
%478 = OpLabel
%480 = OpAccessChain %_ptr_Function_v3float %_6_m43 %int_2
%481 = OpLoad %v3float %480
%482 = OpFOrdEqual %v3bool %481 %223
%483 = OpAll %bool %482
OpBranch %479
%479 = OpLabel
%484 = OpPhi %bool %false %472 %483 %478
OpSelectionMerge %486 None
OpBranchConditional %484 %485 %486
%485 = OpLabel
%487 = OpAccessChain %_ptr_Function_v3float %_6_m43 %int_3
%488 = OpLoad %v3float %487
%489 = OpFOrdEqual %v3bool %488 %231
%490 = OpAll %bool %489
OpBranch %486
%486 = OpLabel
%491 = OpPhi %bool %false %479 %490 %485
OpBranch %466
%466 = OpLabel
%492 = OpPhi %bool %false %431 %491 %486
OpStore %_0_ok %492
%494 = OpLoad %mat3v2float %_3_m32
%495 = OpLoad %mat2v3float %_1_m23
%496 = OpMatrixTimesMatrix %mat2v2float %494 %495
OpStore %_7_m22 %496
%497 = OpLoad %bool %_0_ok
OpSelectionMerge %499 None
OpBranchConditional %497 %498 %499
%498 = OpLabel
%500 = OpLoad %mat2v2float %_7_m22
%502 = OpCompositeConstruct %v2float %float_8 %float_0
%503 = OpCompositeConstruct %v2float %float_0 %float_8
%501 = OpCompositeConstruct %mat2v2float %502 %503
%504 = OpCompositeExtract %v2float %500 0
%505 = OpCompositeExtract %v2float %501 0
%506 = OpFOrdEqual %v2bool %504 %505
%507 = OpAll %bool %506
%508 = OpCompositeExtract %v2float %500 1
%509 = OpCompositeExtract %v2float %501 1
%510 = OpFOrdEqual %v2bool %508 %509
%511 = OpAll %bool %510
%512 = OpLogicalAnd %bool %507 %511
OpBranch %499
%499 = OpLabel
%513 = OpPhi %bool %false %466 %512 %498
OpStore %_0_ok %513
%515 = OpLoad %mat4v3float %_6_m43
%516 = OpLoad %mat3v4float %_4_m34
%517 = OpMatrixTimesMatrix %mat3v3float %515 %516
OpStore %_8_m33 %517
%518 = OpLoad %bool %_0_ok
OpSelectionMerge %520 None
OpBranchConditional %518 %519 %520
%519 = OpLabel
%521 = OpLoad %mat3v3float %_8_m33
%523 = OpCompositeConstruct %v3float %float_35 %float_0 %float_0
%524 = OpCompositeConstruct %v3float %float_0 %float_35 %float_0
%525 = OpCompositeConstruct %v3float %float_0 %float_0 %float_35
%522 = OpCompositeConstruct %mat3v3float %523 %524 %525
%526 = OpCompositeExtract %v3float %521 0
%527 = OpCompositeExtract %v3float %522 0
%528 = OpFOrdEqual %v3bool %526 %527
%529 = OpAll %bool %528
%530 = OpCompositeExtract %v3float %521 1
%531 = OpCompositeExtract %v3float %522 1
%532 = OpFOrdEqual %v3bool %530 %531
%533 = OpAll %bool %532
%534 = OpLogicalAnd %bool %529 %533
%535 = OpCompositeExtract %v3float %521 2
%536 = OpCompositeExtract %v3float %522 2
%537 = OpFOrdEqual %v3bool %535 %536
%538 = OpAll %bool %537
%539 = OpLogicalAnd %bool %534 %538
OpBranch %520
%520 = OpLabel
%540 = OpPhi %bool %false %499 %539 %519
OpStore %_0_ok %540
%542 = OpLoad %mat2v4float %_2_m24
%543 = OpLoad %mat4v2float %_5_m42
%544 = OpMatrixTimesMatrix %mat4v4float %542 %543
OpStore %_9_m44 %544
%545 = OpLoad %bool %_0_ok
OpSelectionMerge %547 None
OpBranchConditional %545 %546 %547
%546 = OpLabel
%548 = OpLoad %mat4v4float %_9_m44
%550 = OpCompositeConstruct %v4float %float_18 %float_0 %float_0 %float_0
%551 = OpCompositeConstruct %v4float %float_0 %float_18 %float_0 %float_0
%552 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%553 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%549 = OpCompositeConstruct %mat4v4float %550 %551 %552 %553
%554 = OpCompositeExtract %v4float %548 0
%555 = OpCompositeExtract %v4float %549 0
%556 = OpFOrdEqual %v4bool %554 %555
%557 = OpAll %bool %556
%558 = OpCompositeExtract %v4float %548 1
%559 = OpCompositeExtract %v4float %549 1
%560 = OpFOrdEqual %v4bool %558 %559
%561 = OpAll %bool %560
%562 = OpLogicalAnd %bool %557 %561
%563 = OpCompositeExtract %v4float %548 2
%564 = OpCompositeExtract %v4float %549 2
%565 = OpFOrdEqual %v4bool %563 %564
%566 = OpAll %bool %565
%567 = OpLogicalAnd %bool %562 %566
%568 = OpCompositeExtract %v4float %548 3
%569 = OpCompositeExtract %v4float %549 3
%570 = OpFOrdEqual %v4bool %568 %569
%571 = OpAll %bool %570
%572 = OpLogicalAnd %bool %567 %571
OpBranch %547
%547 = OpLabel
%573 = OpPhi %bool %false %520 %572 %546
OpStore %_0_ok %573
%574 = OpLoad %bool %_0_ok
OpSelectionMerge %576 None
OpBranchConditional %574 %575 %576
%575 = OpLabel
%577 = OpFunctionCall %bool %test_half_b
OpBranch %576
%576 = OpLabel
%578 = OpPhi %bool %false %547 %577 %575
OpSelectionMerge %582 None
OpBranchConditional %578 %580 %581
%580 = OpLabel
%583 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%585 = OpLoad %v4float %583
OpStore %579 %585
OpBranch %582
%581 = OpLabel
%586 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%587 = OpLoad %v4float %586
OpStore %579 %587
OpBranch %582
%582 = OpLabel
%588 = OpLoad %v4float %579
OpReturnValue %588
OpFunctionEnd
