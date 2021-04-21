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
OpName %takes_float2_bf2 "takes_float2_bf2"
OpName %takes_float3_bf3 "takes_float3_bf3"
OpName %takes_float4_bf4 "takes_float4_bf4"
OpName %takes_float2x2_bf22 "takes_float2x2_bf22"
OpName %takes_float3x3_bf33 "takes_float3x3_bf33"
OpName %takes_float4x4_bf44 "takes_float4x4_bf44"
OpName %takes_half_bh "takes_half_bh"
OpName %takes_half2_bh2 "takes_half2_bh2"
OpName %takes_half3_bh3 "takes_half3_bh3"
OpName %takes_half4_bh4 "takes_half4_bh4"
OpName %takes_half2x2_bh22 "takes_half2x2_bh22"
OpName %takes_half3x3_bh33 "takes_half3x3_bh33"
OpName %takes_half4x4_bh44 "takes_half4x4_bh44"
OpName %takes_bool_bb "takes_bool_bb"
OpName %takes_bool2_bb2 "takes_bool2_bb2"
OpName %takes_bool3_bb3 "takes_bool3_bb3"
OpName %takes_bool4_bb4 "takes_bool4_bb4"
OpName %takes_int_bi "takes_int_bi"
OpName %takes_int2_bi2 "takes_int2_bi2"
OpName %takes_int3_bi3 "takes_int3_bi3"
OpName %takes_int4_bi4 "takes_int4_bi4"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %31 Binding 0
OpDecorate %31 DescriptorSet 0
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%31 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%36 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%40 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%44 = OpTypeFunction %bool %_ptr_Function_v2float
%true = OpConstantTrue %bool
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%49 = OpTypeFunction %bool %_ptr_Function_v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%53 = OpTypeFunction %bool %_ptr_Function_v4float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%58 = OpTypeFunction %bool %_ptr_Function_mat2v2float
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%63 = OpTypeFunction %bool %_ptr_Function_mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%68 = OpTypeFunction %bool %_ptr_Function_mat4v4float
%_ptr_Function_float = OpTypePointer Function %float
%72 = OpTypeFunction %bool %_ptr_Function_float
%_ptr_Function_bool = OpTypePointer Function %bool
%88 = OpTypeFunction %bool %_ptr_Function_bool
%v2bool = OpTypeVector %bool 2
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
%93 = OpTypeFunction %bool %_ptr_Function_v2bool
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%98 = OpTypeFunction %bool %_ptr_Function_v3bool
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%103 = OpTypeFunction %bool %_ptr_Function_v4bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%108 = OpTypeFunction %bool %_ptr_Function_int
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%113 = OpTypeFunction %bool %_ptr_Function_v2int
%v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
%118 = OpTypeFunction %bool %_ptr_Function_v3int
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%123 = OpTypeFunction %bool %_ptr_Function_v4int
%127 = OpTypeFunction %v4float %_ptr_Function_v2float
%false = OpConstantFalse %bool
%float_2 = OpConstant %float 2
%134 = OpConstantComposite %v2float %float_2 %float_2
%float_3 = OpConstant %float 3
%141 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%float_4 = OpConstant %float 4
%148 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%float_1 = OpConstant %float 1
%234 = OpConstantComposite %v2bool %true %true
%240 = OpConstantComposite %v3bool %true %true %true
%246 = OpConstantComposite %v4bool %true %true %true %true
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%259 = OpConstantComposite %v2int %int_2 %int_2
%int_3 = OpConstant %int 3
%266 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%int_4 = OpConstant %int 4
%273 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %36
%37 = OpLabel
%41 = OpVariable %_ptr_Function_v2float Function
OpStore %41 %40
%43 = OpFunctionCall %v4float %main %41
OpStore %sk_FragColor %43
OpReturn
OpFunctionEnd
%takes_float2_bf2 = OpFunction %bool None %44
%45 = OpFunctionParameter %_ptr_Function_v2float
%46 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float3_bf3 = OpFunction %bool None %49
%51 = OpFunctionParameter %_ptr_Function_v3float
%52 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float4_bf4 = OpFunction %bool None %53
%55 = OpFunctionParameter %_ptr_Function_v4float
%56 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float2x2_bf22 = OpFunction %bool None %58
%60 = OpFunctionParameter %_ptr_Function_mat2v2float
%61 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float3x3_bf33 = OpFunction %bool None %63
%65 = OpFunctionParameter %_ptr_Function_mat3v3float
%66 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float4x4_bf44 = OpFunction %bool None %68
%70 = OpFunctionParameter %_ptr_Function_mat4v4float
%71 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half_bh = OpFunction %bool None %72
%74 = OpFunctionParameter %_ptr_Function_float
%75 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half2_bh2 = OpFunction %bool None %44
%76 = OpFunctionParameter %_ptr_Function_v2float
%77 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half3_bh3 = OpFunction %bool None %49
%78 = OpFunctionParameter %_ptr_Function_v3float
%79 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half4_bh4 = OpFunction %bool None %53
%80 = OpFunctionParameter %_ptr_Function_v4float
%81 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half2x2_bh22 = OpFunction %bool None %58
%82 = OpFunctionParameter %_ptr_Function_mat2v2float
%83 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half3x3_bh33 = OpFunction %bool None %63
%84 = OpFunctionParameter %_ptr_Function_mat3v3float
%85 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half4x4_bh44 = OpFunction %bool None %68
%86 = OpFunctionParameter %_ptr_Function_mat4v4float
%87 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool_bb = OpFunction %bool None %88
%90 = OpFunctionParameter %_ptr_Function_bool
%91 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool2_bb2 = OpFunction %bool None %93
%95 = OpFunctionParameter %_ptr_Function_v2bool
%96 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool3_bb3 = OpFunction %bool None %98
%100 = OpFunctionParameter %_ptr_Function_v3bool
%101 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool4_bb4 = OpFunction %bool None %103
%105 = OpFunctionParameter %_ptr_Function_v4bool
%106 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int_bi = OpFunction %bool None %108
%110 = OpFunctionParameter %_ptr_Function_int
%111 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int2_bi2 = OpFunction %bool None %113
%115 = OpFunctionParameter %_ptr_Function_v2int
%116 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int3_bi3 = OpFunction %bool None %118
%120 = OpFunctionParameter %_ptr_Function_v3int
%121 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int4_bi4 = OpFunction %bool None %123
%125 = OpFunctionParameter %_ptr_Function_v4int
%126 = OpLabel
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %127
%128 = OpFunctionParameter %_ptr_Function_v2float
%129 = OpLabel
%135 = OpVariable %_ptr_Function_v2float Function
%142 = OpVariable %_ptr_Function_v3float Function
%149 = OpVariable %_ptr_Function_v4float Function
%157 = OpVariable %_ptr_Function_mat2v2float Function
%166 = OpVariable %_ptr_Function_mat3v3float Function
%176 = OpVariable %_ptr_Function_mat4v4float Function
%182 = OpVariable %_ptr_Function_float Function
%187 = OpVariable %_ptr_Function_v2float Function
%192 = OpVariable %_ptr_Function_v3float Function
%197 = OpVariable %_ptr_Function_v4float Function
%205 = OpVariable %_ptr_Function_mat2v2float Function
%214 = OpVariable %_ptr_Function_mat3v3float Function
%224 = OpVariable %_ptr_Function_mat4v4float Function
%229 = OpVariable %_ptr_Function_bool Function
%235 = OpVariable %_ptr_Function_v2bool Function
%241 = OpVariable %_ptr_Function_v3bool Function
%247 = OpVariable %_ptr_Function_v4bool Function
%253 = OpVariable %_ptr_Function_int Function
%260 = OpVariable %_ptr_Function_v2int Function
%267 = OpVariable %_ptr_Function_v3int Function
%274 = OpVariable %_ptr_Function_v4int Function
%277 = OpVariable %_ptr_Function_v4float Function
OpSelectionMerge %132 None
OpBranchConditional %true %131 %132
%131 = OpLabel
OpStore %135 %134
%136 = OpFunctionCall %bool %takes_float2_bf2 %135
OpBranch %132
%132 = OpLabel
%137 = OpPhi %bool %false %129 %136 %131
OpSelectionMerge %139 None
OpBranchConditional %137 %138 %139
%138 = OpLabel
OpStore %142 %141
%143 = OpFunctionCall %bool %takes_float3_bf3 %142
OpBranch %139
%139 = OpLabel
%144 = OpPhi %bool %false %132 %143 %138
OpSelectionMerge %146 None
OpBranchConditional %144 %145 %146
%145 = OpLabel
OpStore %149 %148
%150 = OpFunctionCall %bool %takes_float4_bf4 %149
OpBranch %146
%146 = OpLabel
%151 = OpPhi %bool %false %139 %150 %145
OpSelectionMerge %153 None
OpBranchConditional %151 %152 %153
%152 = OpLabel
%155 = OpCompositeConstruct %v2float %float_2 %float_0
%156 = OpCompositeConstruct %v2float %float_0 %float_2
%154 = OpCompositeConstruct %mat2v2float %155 %156
OpStore %157 %154
%158 = OpFunctionCall %bool %takes_float2x2_bf22 %157
OpBranch %153
%153 = OpLabel
%159 = OpPhi %bool %false %146 %158 %152
OpSelectionMerge %161 None
OpBranchConditional %159 %160 %161
%160 = OpLabel
%163 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%164 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%165 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%162 = OpCompositeConstruct %mat3v3float %163 %164 %165
OpStore %166 %162
%167 = OpFunctionCall %bool %takes_float3x3_bf33 %166
OpBranch %161
%161 = OpLabel
%168 = OpPhi %bool %false %153 %167 %160
OpSelectionMerge %170 None
OpBranchConditional %168 %169 %170
%169 = OpLabel
%172 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%173 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%174 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%175 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%171 = OpCompositeConstruct %mat4v4float %172 %173 %174 %175
OpStore %176 %171
%177 = OpFunctionCall %bool %takes_float4x4_bf44 %176
OpBranch %170
%170 = OpLabel
%178 = OpPhi %bool %false %161 %177 %169
OpSelectionMerge %180 None
OpBranchConditional %178 %179 %180
%179 = OpLabel
OpStore %182 %float_1
%183 = OpFunctionCall %bool %takes_half_bh %182
OpBranch %180
%180 = OpLabel
%184 = OpPhi %bool %false %170 %183 %179
OpSelectionMerge %186 None
OpBranchConditional %184 %185 %186
%185 = OpLabel
OpStore %187 %134
%188 = OpFunctionCall %bool %takes_half2_bh2 %187
OpBranch %186
%186 = OpLabel
%189 = OpPhi %bool %false %180 %188 %185
OpSelectionMerge %191 None
OpBranchConditional %189 %190 %191
%190 = OpLabel
OpStore %192 %141
%193 = OpFunctionCall %bool %takes_half3_bh3 %192
OpBranch %191
%191 = OpLabel
%194 = OpPhi %bool %false %186 %193 %190
OpSelectionMerge %196 None
OpBranchConditional %194 %195 %196
%195 = OpLabel
OpStore %197 %148
%198 = OpFunctionCall %bool %takes_half4_bh4 %197
OpBranch %196
%196 = OpLabel
%199 = OpPhi %bool %false %191 %198 %195
OpSelectionMerge %201 None
OpBranchConditional %199 %200 %201
%200 = OpLabel
%203 = OpCompositeConstruct %v2float %float_2 %float_0
%204 = OpCompositeConstruct %v2float %float_0 %float_2
%202 = OpCompositeConstruct %mat2v2float %203 %204
OpStore %205 %202
%206 = OpFunctionCall %bool %takes_half2x2_bh22 %205
OpBranch %201
%201 = OpLabel
%207 = OpPhi %bool %false %196 %206 %200
OpSelectionMerge %209 None
OpBranchConditional %207 %208 %209
%208 = OpLabel
%211 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%212 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%213 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%210 = OpCompositeConstruct %mat3v3float %211 %212 %213
OpStore %214 %210
%215 = OpFunctionCall %bool %takes_half3x3_bh33 %214
OpBranch %209
%209 = OpLabel
%216 = OpPhi %bool %false %201 %215 %208
OpSelectionMerge %218 None
OpBranchConditional %216 %217 %218
%217 = OpLabel
%220 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%221 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%222 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%223 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%219 = OpCompositeConstruct %mat4v4float %220 %221 %222 %223
OpStore %224 %219
%225 = OpFunctionCall %bool %takes_half4x4_bh44 %224
OpBranch %218
%218 = OpLabel
%226 = OpPhi %bool %false %209 %225 %217
OpSelectionMerge %228 None
OpBranchConditional %226 %227 %228
%227 = OpLabel
OpStore %229 %true
%230 = OpFunctionCall %bool %takes_bool_bb %229
OpBranch %228
%228 = OpLabel
%231 = OpPhi %bool %false %218 %230 %227
OpSelectionMerge %233 None
OpBranchConditional %231 %232 %233
%232 = OpLabel
OpStore %235 %234
%236 = OpFunctionCall %bool %takes_bool2_bb2 %235
OpBranch %233
%233 = OpLabel
%237 = OpPhi %bool %false %228 %236 %232
OpSelectionMerge %239 None
OpBranchConditional %237 %238 %239
%238 = OpLabel
OpStore %241 %240
%242 = OpFunctionCall %bool %takes_bool3_bb3 %241
OpBranch %239
%239 = OpLabel
%243 = OpPhi %bool %false %233 %242 %238
OpSelectionMerge %245 None
OpBranchConditional %243 %244 %245
%244 = OpLabel
OpStore %247 %246
%248 = OpFunctionCall %bool %takes_bool4_bb4 %247
OpBranch %245
%245 = OpLabel
%249 = OpPhi %bool %false %239 %248 %244
OpSelectionMerge %251 None
OpBranchConditional %249 %250 %251
%250 = OpLabel
OpStore %253 %int_1
%254 = OpFunctionCall %bool %takes_int_bi %253
OpBranch %251
%251 = OpLabel
%255 = OpPhi %bool %false %245 %254 %250
OpSelectionMerge %257 None
OpBranchConditional %255 %256 %257
%256 = OpLabel
OpStore %260 %259
%261 = OpFunctionCall %bool %takes_int2_bi2 %260
OpBranch %257
%257 = OpLabel
%262 = OpPhi %bool %false %251 %261 %256
OpSelectionMerge %264 None
OpBranchConditional %262 %263 %264
%263 = OpLabel
OpStore %267 %266
%268 = OpFunctionCall %bool %takes_int3_bi3 %267
OpBranch %264
%264 = OpLabel
%269 = OpPhi %bool %false %257 %268 %263
OpSelectionMerge %271 None
OpBranchConditional %269 %270 %271
%270 = OpLabel
OpStore %274 %273
%275 = OpFunctionCall %bool %takes_int4_bi4 %274
OpBranch %271
%271 = OpLabel
%276 = OpPhi %bool %false %264 %275 %270
OpSelectionMerge %280 None
OpBranchConditional %276 %278 %279
%278 = OpLabel
%281 = OpAccessChain %_ptr_Uniform_v4float %31 %int_0
%284 = OpLoad %v4float %281
OpStore %277 %284
OpBranch %280
%279 = OpLabel
%285 = OpAccessChain %_ptr_Uniform_v4float %31 %int_1
%286 = OpLoad %v4float %285
OpStore %277 %286
OpBranch %280
%280 = OpLabel
%287 = OpLoad %v4float %277
OpReturnValue %287
OpFunctionEnd
