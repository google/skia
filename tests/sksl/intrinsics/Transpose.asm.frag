OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testMatrix2x2"
OpMemberName %_UniformBuffer 1 "testMatrix3x3"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %testMatrix2x3 "testMatrix2x3"
OpName %testMatrix2x4 "testMatrix2x4"
OpName %testMatrix3x2 "testMatrix3x2"
OpName %testMatrix3x4 "testMatrix3x4"
OpName %testMatrix4x2 "testMatrix4x2"
OpName %testMatrix4x3 "testMatrix4x3"
OpName %testMatrix4x4 "testMatrix4x4"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 ColMajor
OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
OpMemberDecorate %_UniformBuffer 1 Offset 32
OpMemberDecorate %_UniformBuffer 1 ColMajor
OpMemberDecorate %_UniformBuffer 1 MatrixStride 16
OpMemberDecorate %_UniformBuffer 2 Offset 80
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 96
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %318 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
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
%_UniformBuffer = OpTypeStruct %mat2v2float %mat3v3float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %v4float %_ptr_Function_v2float
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
%float_9 = OpConstant %float 9
%float_10 = OpConstant %float 10
%float_11 = OpConstant %float 11
%float_12 = OpConstant %float 12
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_13 = OpConstant %float 13
%float_14 = OpConstant %float 14
%float_15 = OpConstant %float 15
%float_16 = OpConstant %float 16
%false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%v3bool = OpTypeVector %bool 3
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_1 = OpConstant %int 1
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%_entrypoint_v = OpFunction %void None %19
%20 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %26
%27 = OpFunctionParameter %_ptr_Function_v2float
%28 = OpLabel
%testMatrix2x3 = OpVariable %_ptr_Function_mat2v3float Function
%testMatrix2x4 = OpVariable %_ptr_Function_mat2v4float Function
%testMatrix3x2 = OpVariable %_ptr_Function_mat3v2float Function
%testMatrix3x4 = OpVariable %_ptr_Function_mat3v4float Function
%testMatrix4x2 = OpVariable %_ptr_Function_mat4v2float Function
%testMatrix4x3 = OpVariable %_ptr_Function_mat4v3float Function
%testMatrix4x4 = OpVariable %_ptr_Function_mat4v4float Function
%310 = OpVariable %_ptr_Function_v4float Function
%38 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%39 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%40 = OpCompositeConstruct %mat2v3float %38 %39
OpStore %testMatrix2x3 %40
%46 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%47 = OpCompositeConstruct %v4float %float_5 %float_6 %float_7 %float_8
%48 = OpCompositeConstruct %mat2v4float %46 %47
OpStore %testMatrix2x4 %48
%52 = OpCompositeConstruct %v2float %float_1 %float_2
%53 = OpCompositeConstruct %v2float %float_3 %float_4
%54 = OpCompositeConstruct %v2float %float_5 %float_6
%55 = OpCompositeConstruct %mat3v2float %52 %53 %54
OpStore %testMatrix3x2 %55
%63 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%64 = OpCompositeConstruct %v4float %float_5 %float_6 %float_7 %float_8
%65 = OpCompositeConstruct %v4float %float_9 %float_10 %float_11 %float_12
%66 = OpCompositeConstruct %mat3v4float %63 %64 %65
OpStore %testMatrix3x4 %66
%70 = OpCompositeConstruct %v2float %float_1 %float_2
%71 = OpCompositeConstruct %v2float %float_3 %float_4
%72 = OpCompositeConstruct %v2float %float_5 %float_6
%73 = OpCompositeConstruct %v2float %float_7 %float_8
%74 = OpCompositeConstruct %mat4v2float %70 %71 %72 %73
OpStore %testMatrix4x2 %74
%78 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%79 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%80 = OpCompositeConstruct %v3float %float_7 %float_8 %float_9
%81 = OpCompositeConstruct %v3float %float_10 %float_11 %float_12
%82 = OpCompositeConstruct %mat4v3float %78 %79 %80 %81
OpStore %testMatrix4x3 %82
%90 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%91 = OpCompositeConstruct %v4float %float_5 %float_6 %float_7 %float_8
%92 = OpCompositeConstruct %v4float %float_9 %float_10 %float_11 %float_12
%93 = OpCompositeConstruct %v4float %float_13 %float_14 %float_15 %float_16
%94 = OpCompositeConstruct %mat4v4float %90 %91 %92 %93
OpStore %testMatrix4x4 %94
%97 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_0
%101 = OpLoad %mat2v2float %97
%96 = OpTranspose %mat2v2float %101
%102 = OpCompositeConstruct %v2float %float_1 %float_3
%103 = OpCompositeConstruct %v2float %float_2 %float_4
%104 = OpCompositeConstruct %mat2v2float %102 %103
%106 = OpCompositeExtract %v2float %96 0
%107 = OpCompositeExtract %v2float %104 0
%108 = OpFOrdEqual %v2bool %106 %107
%109 = OpAll %bool %108
%110 = OpCompositeExtract %v2float %96 1
%111 = OpCompositeExtract %v2float %104 1
%112 = OpFOrdEqual %v2bool %110 %111
%113 = OpAll %bool %112
%114 = OpLogicalAnd %bool %109 %113
OpSelectionMerge %116 None
OpBranchConditional %114 %115 %116
%115 = OpLabel
%118 = OpLoad %mat2v3float %testMatrix2x3
%117 = OpTranspose %mat3v2float %118
%119 = OpCompositeConstruct %v2float %float_1 %float_4
%120 = OpCompositeConstruct %v2float %float_2 %float_5
%121 = OpCompositeConstruct %v2float %float_3 %float_6
%122 = OpCompositeConstruct %mat3v2float %119 %120 %121
%123 = OpCompositeExtract %v2float %117 0
%124 = OpCompositeExtract %v2float %122 0
%125 = OpFOrdEqual %v2bool %123 %124
%126 = OpAll %bool %125
%127 = OpCompositeExtract %v2float %117 1
%128 = OpCompositeExtract %v2float %122 1
%129 = OpFOrdEqual %v2bool %127 %128
%130 = OpAll %bool %129
%131 = OpLogicalAnd %bool %126 %130
%132 = OpCompositeExtract %v2float %117 2
%133 = OpCompositeExtract %v2float %122 2
%134 = OpFOrdEqual %v2bool %132 %133
%135 = OpAll %bool %134
%136 = OpLogicalAnd %bool %131 %135
OpBranch %116
%116 = OpLabel
%137 = OpPhi %bool %false %28 %136 %115
OpSelectionMerge %139 None
OpBranchConditional %137 %138 %139
%138 = OpLabel
%141 = OpLoad %mat2v4float %testMatrix2x4
%140 = OpTranspose %mat4v2float %141
%142 = OpCompositeConstruct %v2float %float_1 %float_5
%143 = OpCompositeConstruct %v2float %float_2 %float_6
%144 = OpCompositeConstruct %v2float %float_3 %float_7
%145 = OpCompositeConstruct %v2float %float_4 %float_8
%146 = OpCompositeConstruct %mat4v2float %142 %143 %144 %145
%147 = OpCompositeExtract %v2float %140 0
%148 = OpCompositeExtract %v2float %146 0
%149 = OpFOrdEqual %v2bool %147 %148
%150 = OpAll %bool %149
%151 = OpCompositeExtract %v2float %140 1
%152 = OpCompositeExtract %v2float %146 1
%153 = OpFOrdEqual %v2bool %151 %152
%154 = OpAll %bool %153
%155 = OpLogicalAnd %bool %150 %154
%156 = OpCompositeExtract %v2float %140 2
%157 = OpCompositeExtract %v2float %146 2
%158 = OpFOrdEqual %v2bool %156 %157
%159 = OpAll %bool %158
%160 = OpLogicalAnd %bool %155 %159
%161 = OpCompositeExtract %v2float %140 3
%162 = OpCompositeExtract %v2float %146 3
%163 = OpFOrdEqual %v2bool %161 %162
%164 = OpAll %bool %163
%165 = OpLogicalAnd %bool %160 %164
OpBranch %139
%139 = OpLabel
%166 = OpPhi %bool %false %116 %165 %138
OpSelectionMerge %168 None
OpBranchConditional %166 %167 %168
%167 = OpLabel
%170 = OpLoad %mat3v2float %testMatrix3x2
%169 = OpTranspose %mat2v3float %170
%171 = OpCompositeConstruct %v3float %float_1 %float_3 %float_5
%172 = OpCompositeConstruct %v3float %float_2 %float_4 %float_6
%173 = OpCompositeConstruct %mat2v3float %171 %172
%175 = OpCompositeExtract %v3float %169 0
%176 = OpCompositeExtract %v3float %173 0
%177 = OpFOrdEqual %v3bool %175 %176
%178 = OpAll %bool %177
%179 = OpCompositeExtract %v3float %169 1
%180 = OpCompositeExtract %v3float %173 1
%181 = OpFOrdEqual %v3bool %179 %180
%182 = OpAll %bool %181
%183 = OpLogicalAnd %bool %178 %182
OpBranch %168
%168 = OpLabel
%184 = OpPhi %bool %false %139 %183 %167
OpSelectionMerge %186 None
OpBranchConditional %184 %185 %186
%185 = OpLabel
%188 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_1
%191 = OpLoad %mat3v3float %188
%187 = OpTranspose %mat3v3float %191
%192 = OpCompositeConstruct %v3float %float_1 %float_4 %float_7
%193 = OpCompositeConstruct %v3float %float_2 %float_5 %float_8
%194 = OpCompositeConstruct %v3float %float_3 %float_6 %float_9
%195 = OpCompositeConstruct %mat3v3float %192 %193 %194
%196 = OpCompositeExtract %v3float %187 0
%197 = OpCompositeExtract %v3float %195 0
%198 = OpFOrdEqual %v3bool %196 %197
%199 = OpAll %bool %198
%200 = OpCompositeExtract %v3float %187 1
%201 = OpCompositeExtract %v3float %195 1
%202 = OpFOrdEqual %v3bool %200 %201
%203 = OpAll %bool %202
%204 = OpLogicalAnd %bool %199 %203
%205 = OpCompositeExtract %v3float %187 2
%206 = OpCompositeExtract %v3float %195 2
%207 = OpFOrdEqual %v3bool %205 %206
%208 = OpAll %bool %207
%209 = OpLogicalAnd %bool %204 %208
OpBranch %186
%186 = OpLabel
%210 = OpPhi %bool %false %168 %209 %185
OpSelectionMerge %212 None
OpBranchConditional %210 %211 %212
%211 = OpLabel
%214 = OpLoad %mat3v4float %testMatrix3x4
%213 = OpTranspose %mat4v3float %214
%215 = OpCompositeConstruct %v3float %float_1 %float_5 %float_9
%216 = OpCompositeConstruct %v3float %float_2 %float_6 %float_10
%217 = OpCompositeConstruct %v3float %float_3 %float_7 %float_11
%218 = OpCompositeConstruct %v3float %float_4 %float_8 %float_12
%219 = OpCompositeConstruct %mat4v3float %215 %216 %217 %218
%220 = OpCompositeExtract %v3float %213 0
%221 = OpCompositeExtract %v3float %219 0
%222 = OpFOrdEqual %v3bool %220 %221
%223 = OpAll %bool %222
%224 = OpCompositeExtract %v3float %213 1
%225 = OpCompositeExtract %v3float %219 1
%226 = OpFOrdEqual %v3bool %224 %225
%227 = OpAll %bool %226
%228 = OpLogicalAnd %bool %223 %227
%229 = OpCompositeExtract %v3float %213 2
%230 = OpCompositeExtract %v3float %219 2
%231 = OpFOrdEqual %v3bool %229 %230
%232 = OpAll %bool %231
%233 = OpLogicalAnd %bool %228 %232
%234 = OpCompositeExtract %v3float %213 3
%235 = OpCompositeExtract %v3float %219 3
%236 = OpFOrdEqual %v3bool %234 %235
%237 = OpAll %bool %236
%238 = OpLogicalAnd %bool %233 %237
OpBranch %212
%212 = OpLabel
%239 = OpPhi %bool %false %186 %238 %211
OpSelectionMerge %241 None
OpBranchConditional %239 %240 %241
%240 = OpLabel
%243 = OpLoad %mat4v2float %testMatrix4x2
%242 = OpTranspose %mat2v4float %243
%244 = OpCompositeConstruct %v4float %float_1 %float_3 %float_5 %float_7
%245 = OpCompositeConstruct %v4float %float_2 %float_4 %float_6 %float_8
%246 = OpCompositeConstruct %mat2v4float %244 %245
%248 = OpCompositeExtract %v4float %242 0
%249 = OpCompositeExtract %v4float %246 0
%250 = OpFOrdEqual %v4bool %248 %249
%251 = OpAll %bool %250
%252 = OpCompositeExtract %v4float %242 1
%253 = OpCompositeExtract %v4float %246 1
%254 = OpFOrdEqual %v4bool %252 %253
%255 = OpAll %bool %254
%256 = OpLogicalAnd %bool %251 %255
OpBranch %241
%241 = OpLabel
%257 = OpPhi %bool %false %212 %256 %240
OpSelectionMerge %259 None
OpBranchConditional %257 %258 %259
%258 = OpLabel
%261 = OpLoad %mat4v3float %testMatrix4x3
%260 = OpTranspose %mat3v4float %261
%262 = OpCompositeConstruct %v4float %float_1 %float_4 %float_7 %float_10
%263 = OpCompositeConstruct %v4float %float_2 %float_5 %float_8 %float_11
%264 = OpCompositeConstruct %v4float %float_3 %float_6 %float_9 %float_12
%265 = OpCompositeConstruct %mat3v4float %262 %263 %264
%266 = OpCompositeExtract %v4float %260 0
%267 = OpCompositeExtract %v4float %265 0
%268 = OpFOrdEqual %v4bool %266 %267
%269 = OpAll %bool %268
%270 = OpCompositeExtract %v4float %260 1
%271 = OpCompositeExtract %v4float %265 1
%272 = OpFOrdEqual %v4bool %270 %271
%273 = OpAll %bool %272
%274 = OpLogicalAnd %bool %269 %273
%275 = OpCompositeExtract %v4float %260 2
%276 = OpCompositeExtract %v4float %265 2
%277 = OpFOrdEqual %v4bool %275 %276
%278 = OpAll %bool %277
%279 = OpLogicalAnd %bool %274 %278
OpBranch %259
%259 = OpLabel
%280 = OpPhi %bool %false %241 %279 %258
OpSelectionMerge %282 None
OpBranchConditional %280 %281 %282
%281 = OpLabel
%284 = OpLoad %mat4v4float %testMatrix4x4
%283 = OpTranspose %mat4v4float %284
%285 = OpCompositeConstruct %v4float %float_1 %float_5 %float_9 %float_13
%286 = OpCompositeConstruct %v4float %float_2 %float_6 %float_10 %float_14
%287 = OpCompositeConstruct %v4float %float_3 %float_7 %float_11 %float_15
%288 = OpCompositeConstruct %v4float %float_4 %float_8 %float_12 %float_16
%289 = OpCompositeConstruct %mat4v4float %285 %286 %287 %288
%290 = OpCompositeExtract %v4float %283 0
%291 = OpCompositeExtract %v4float %289 0
%292 = OpFOrdEqual %v4bool %290 %291
%293 = OpAll %bool %292
%294 = OpCompositeExtract %v4float %283 1
%295 = OpCompositeExtract %v4float %289 1
%296 = OpFOrdEqual %v4bool %294 %295
%297 = OpAll %bool %296
%298 = OpLogicalAnd %bool %293 %297
%299 = OpCompositeExtract %v4float %283 2
%300 = OpCompositeExtract %v4float %289 2
%301 = OpFOrdEqual %v4bool %299 %300
%302 = OpAll %bool %301
%303 = OpLogicalAnd %bool %298 %302
%304 = OpCompositeExtract %v4float %283 3
%305 = OpCompositeExtract %v4float %289 3
%306 = OpFOrdEqual %v4bool %304 %305
%307 = OpAll %bool %306
%308 = OpLogicalAnd %bool %303 %307
OpBranch %282
%282 = OpLabel
%309 = OpPhi %bool %false %259 %308 %281
OpSelectionMerge %314 None
OpBranchConditional %309 %312 %313
%312 = OpLabel
%315 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%318 = OpLoad %v4float %315
OpStore %310 %318
OpBranch %314
%313 = OpLabel
%319 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%321 = OpLoad %v4float %319
OpStore %310 %321
OpBranch %314
%314 = OpLabel
%322 = OpLoad %v4float %310
OpReturnValue %322
OpFunctionEnd
