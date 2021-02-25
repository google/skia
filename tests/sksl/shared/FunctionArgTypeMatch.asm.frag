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
OpName %_entrypoint "_entrypoint"
OpName %takes_float2 "takes_float2"
OpName %takes_float3 "takes_float3"
OpName %takes_float4 "takes_float4"
OpName %takes_float2x2 "takes_float2x2"
OpName %takes_float3x3 "takes_float3x3"
OpName %takes_float4x4 "takes_float4x4"
OpName %takes_half "takes_half"
OpName %takes_half2 "takes_half2"
OpName %takes_half3 "takes_half3"
OpName %takes_half4 "takes_half4"
OpName %takes_half2x2 "takes_half2x2"
OpName %takes_half3x3 "takes_half3x3"
OpName %takes_half4x4 "takes_half4x4"
OpName %takes_bool "takes_bool"
OpName %takes_bool2 "takes_bool2"
OpName %takes_bool3 "takes_bool3"
OpName %takes_bool4 "takes_bool4"
OpName %takes_int "takes_int"
OpName %takes_int2 "takes_int2"
OpName %takes_int3 "takes_int3"
OpName %takes_int4 "takes_int4"
OpName %main "main"
OpName %_0_takes_float "_0_takes_float"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %31 Binding 0
OpDecorate %31 DescriptorSet 0
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
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
%_ptr_Function_v2float = OpTypePointer Function %v2float
%40 = OpTypeFunction %bool %_ptr_Function_v2float
%true = OpConstantTrue %bool
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%46 = OpTypeFunction %bool %_ptr_Function_v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%50 = OpTypeFunction %bool %_ptr_Function_v4float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%55 = OpTypeFunction %bool %_ptr_Function_mat2v2float
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%60 = OpTypeFunction %bool %_ptr_Function_mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%65 = OpTypeFunction %bool %_ptr_Function_mat4v4float
%_ptr_Function_float = OpTypePointer Function %float
%69 = OpTypeFunction %bool %_ptr_Function_float
%_ptr_Function_bool = OpTypePointer Function %bool
%85 = OpTypeFunction %bool %_ptr_Function_bool
%v2bool = OpTypeVector %bool 2
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
%90 = OpTypeFunction %bool %_ptr_Function_v2bool
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%95 = OpTypeFunction %bool %_ptr_Function_v3bool
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%100 = OpTypeFunction %bool %_ptr_Function_v4bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%105 = OpTypeFunction %bool %_ptr_Function_int
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%110 = OpTypeFunction %bool %_ptr_Function_v2int
%v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
%115 = OpTypeFunction %bool %_ptr_Function_v3int
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%120 = OpTypeFunction %bool %_ptr_Function_v4int
%124 = OpTypeFunction %v4float
%false = OpConstantFalse %bool
%float_2 = OpConstant %float 2
%131 = OpConstantComposite %v2float %float_2 %float_2
%float_3 = OpConstant %float 3
%138 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%float_4 = OpConstant %float 4
%145 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%232 = OpConstantComposite %v2bool %true %true
%238 = OpConstantComposite %v3bool %true %true %true
%244 = OpConstantComposite %v4bool %true %true %true %true
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%257 = OpConstantComposite %v2int %int_2 %int_2
%int_3 = OpConstant %int 3
%264 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%int_4 = OpConstant %int 4
%271 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint = OpFunction %void None %36
%37 = OpLabel
%38 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %38
OpReturn
OpFunctionEnd
%takes_float2 = OpFunction %bool None %40
%42 = OpFunctionParameter %_ptr_Function_v2float
%43 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float3 = OpFunction %bool None %46
%48 = OpFunctionParameter %_ptr_Function_v3float
%49 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float4 = OpFunction %bool None %50
%52 = OpFunctionParameter %_ptr_Function_v4float
%53 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float2x2 = OpFunction %bool None %55
%57 = OpFunctionParameter %_ptr_Function_mat2v2float
%58 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float3x3 = OpFunction %bool None %60
%62 = OpFunctionParameter %_ptr_Function_mat3v3float
%63 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float4x4 = OpFunction %bool None %65
%67 = OpFunctionParameter %_ptr_Function_mat4v4float
%68 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half = OpFunction %bool None %69
%71 = OpFunctionParameter %_ptr_Function_float
%72 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half2 = OpFunction %bool None %40
%73 = OpFunctionParameter %_ptr_Function_v2float
%74 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half3 = OpFunction %bool None %46
%75 = OpFunctionParameter %_ptr_Function_v3float
%76 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half4 = OpFunction %bool None %50
%77 = OpFunctionParameter %_ptr_Function_v4float
%78 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half2x2 = OpFunction %bool None %55
%79 = OpFunctionParameter %_ptr_Function_mat2v2float
%80 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half3x3 = OpFunction %bool None %60
%81 = OpFunctionParameter %_ptr_Function_mat3v3float
%82 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half4x4 = OpFunction %bool None %65
%83 = OpFunctionParameter %_ptr_Function_mat4v4float
%84 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool = OpFunction %bool None %85
%87 = OpFunctionParameter %_ptr_Function_bool
%88 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool2 = OpFunction %bool None %90
%92 = OpFunctionParameter %_ptr_Function_v2bool
%93 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool3 = OpFunction %bool None %95
%97 = OpFunctionParameter %_ptr_Function_v3bool
%98 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool4 = OpFunction %bool None %100
%102 = OpFunctionParameter %_ptr_Function_v4bool
%103 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int = OpFunction %bool None %105
%107 = OpFunctionParameter %_ptr_Function_int
%108 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int2 = OpFunction %bool None %110
%112 = OpFunctionParameter %_ptr_Function_v2int
%113 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int3 = OpFunction %bool None %115
%117 = OpFunctionParameter %_ptr_Function_v3int
%118 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int4 = OpFunction %bool None %120
%122 = OpFunctionParameter %_ptr_Function_v4int
%123 = OpLabel
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %124
%125 = OpLabel
%_0_takes_float = OpVariable %_ptr_Function_bool Function
%132 = OpVariable %_ptr_Function_v2float Function
%139 = OpVariable %_ptr_Function_v3float Function
%146 = OpVariable %_ptr_Function_v4float Function
%155 = OpVariable %_ptr_Function_mat2v2float Function
%164 = OpVariable %_ptr_Function_mat3v3float Function
%174 = OpVariable %_ptr_Function_mat4v4float Function
%180 = OpVariable %_ptr_Function_float Function
%185 = OpVariable %_ptr_Function_v2float Function
%190 = OpVariable %_ptr_Function_v3float Function
%195 = OpVariable %_ptr_Function_v4float Function
%203 = OpVariable %_ptr_Function_mat2v2float Function
%212 = OpVariable %_ptr_Function_mat3v3float Function
%222 = OpVariable %_ptr_Function_mat4v4float Function
%227 = OpVariable %_ptr_Function_bool Function
%233 = OpVariable %_ptr_Function_v2bool Function
%239 = OpVariable %_ptr_Function_v3bool Function
%245 = OpVariable %_ptr_Function_v4bool Function
%251 = OpVariable %_ptr_Function_int Function
%258 = OpVariable %_ptr_Function_v2int Function
%265 = OpVariable %_ptr_Function_v3int Function
%272 = OpVariable %_ptr_Function_v4int Function
%275 = OpVariable %_ptr_Function_v4float Function
OpSelectionMerge %129 None
OpBranchConditional %true %128 %129
%128 = OpLabel
OpStore %132 %131
%133 = OpFunctionCall %bool %takes_float2 %132
OpBranch %129
%129 = OpLabel
%134 = OpPhi %bool %false %125 %133 %128
OpSelectionMerge %136 None
OpBranchConditional %134 %135 %136
%135 = OpLabel
OpStore %139 %138
%140 = OpFunctionCall %bool %takes_float3 %139
OpBranch %136
%136 = OpLabel
%141 = OpPhi %bool %false %129 %140 %135
OpSelectionMerge %143 None
OpBranchConditional %141 %142 %143
%142 = OpLabel
OpStore %146 %145
%147 = OpFunctionCall %bool %takes_float4 %146
OpBranch %143
%143 = OpLabel
%148 = OpPhi %bool %false %136 %147 %142
OpSelectionMerge %150 None
OpBranchConditional %148 %149 %150
%149 = OpLabel
%153 = OpCompositeConstruct %v2float %float_2 %float_0
%154 = OpCompositeConstruct %v2float %float_0 %float_2
%151 = OpCompositeConstruct %mat2v2float %153 %154
OpStore %155 %151
%156 = OpFunctionCall %bool %takes_float2x2 %155
OpBranch %150
%150 = OpLabel
%157 = OpPhi %bool %false %143 %156 %149
OpSelectionMerge %159 None
OpBranchConditional %157 %158 %159
%158 = OpLabel
%161 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%162 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%163 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%160 = OpCompositeConstruct %mat3v3float %161 %162 %163
OpStore %164 %160
%165 = OpFunctionCall %bool %takes_float3x3 %164
OpBranch %159
%159 = OpLabel
%166 = OpPhi %bool %false %150 %165 %158
OpSelectionMerge %168 None
OpBranchConditional %166 %167 %168
%167 = OpLabel
%170 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%171 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%172 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%173 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%169 = OpCompositeConstruct %mat4v4float %170 %171 %172 %173
OpStore %174 %169
%175 = OpFunctionCall %bool %takes_float4x4 %174
OpBranch %168
%168 = OpLabel
%176 = OpPhi %bool %false %159 %175 %167
OpSelectionMerge %178 None
OpBranchConditional %176 %177 %178
%177 = OpLabel
OpStore %180 %float_1
%181 = OpFunctionCall %bool %takes_half %180
OpBranch %178
%178 = OpLabel
%182 = OpPhi %bool %false %168 %181 %177
OpSelectionMerge %184 None
OpBranchConditional %182 %183 %184
%183 = OpLabel
OpStore %185 %131
%186 = OpFunctionCall %bool %takes_half2 %185
OpBranch %184
%184 = OpLabel
%187 = OpPhi %bool %false %178 %186 %183
OpSelectionMerge %189 None
OpBranchConditional %187 %188 %189
%188 = OpLabel
OpStore %190 %138
%191 = OpFunctionCall %bool %takes_half3 %190
OpBranch %189
%189 = OpLabel
%192 = OpPhi %bool %false %184 %191 %188
OpSelectionMerge %194 None
OpBranchConditional %192 %193 %194
%193 = OpLabel
OpStore %195 %145
%196 = OpFunctionCall %bool %takes_half4 %195
OpBranch %194
%194 = OpLabel
%197 = OpPhi %bool %false %189 %196 %193
OpSelectionMerge %199 None
OpBranchConditional %197 %198 %199
%198 = OpLabel
%201 = OpCompositeConstruct %v2float %float_2 %float_0
%202 = OpCompositeConstruct %v2float %float_0 %float_2
%200 = OpCompositeConstruct %mat2v2float %201 %202
OpStore %203 %200
%204 = OpFunctionCall %bool %takes_half2x2 %203
OpBranch %199
%199 = OpLabel
%205 = OpPhi %bool %false %194 %204 %198
OpSelectionMerge %207 None
OpBranchConditional %205 %206 %207
%206 = OpLabel
%209 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%210 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%211 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%208 = OpCompositeConstruct %mat3v3float %209 %210 %211
OpStore %212 %208
%213 = OpFunctionCall %bool %takes_half3x3 %212
OpBranch %207
%207 = OpLabel
%214 = OpPhi %bool %false %199 %213 %206
OpSelectionMerge %216 None
OpBranchConditional %214 %215 %216
%215 = OpLabel
%218 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%219 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%220 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%221 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%217 = OpCompositeConstruct %mat4v4float %218 %219 %220 %221
OpStore %222 %217
%223 = OpFunctionCall %bool %takes_half4x4 %222
OpBranch %216
%216 = OpLabel
%224 = OpPhi %bool %false %207 %223 %215
OpSelectionMerge %226 None
OpBranchConditional %224 %225 %226
%225 = OpLabel
OpStore %227 %true
%228 = OpFunctionCall %bool %takes_bool %227
OpBranch %226
%226 = OpLabel
%229 = OpPhi %bool %false %216 %228 %225
OpSelectionMerge %231 None
OpBranchConditional %229 %230 %231
%230 = OpLabel
OpStore %233 %232
%234 = OpFunctionCall %bool %takes_bool2 %233
OpBranch %231
%231 = OpLabel
%235 = OpPhi %bool %false %226 %234 %230
OpSelectionMerge %237 None
OpBranchConditional %235 %236 %237
%236 = OpLabel
OpStore %239 %238
%240 = OpFunctionCall %bool %takes_bool3 %239
OpBranch %237
%237 = OpLabel
%241 = OpPhi %bool %false %231 %240 %236
OpSelectionMerge %243 None
OpBranchConditional %241 %242 %243
%242 = OpLabel
OpStore %245 %244
%246 = OpFunctionCall %bool %takes_bool4 %245
OpBranch %243
%243 = OpLabel
%247 = OpPhi %bool %false %237 %246 %242
OpSelectionMerge %249 None
OpBranchConditional %247 %248 %249
%248 = OpLabel
OpStore %251 %int_1
%252 = OpFunctionCall %bool %takes_int %251
OpBranch %249
%249 = OpLabel
%253 = OpPhi %bool %false %243 %252 %248
OpSelectionMerge %255 None
OpBranchConditional %253 %254 %255
%254 = OpLabel
OpStore %258 %257
%259 = OpFunctionCall %bool %takes_int2 %258
OpBranch %255
%255 = OpLabel
%260 = OpPhi %bool %false %249 %259 %254
OpSelectionMerge %262 None
OpBranchConditional %260 %261 %262
%261 = OpLabel
OpStore %265 %264
%266 = OpFunctionCall %bool %takes_int3 %265
OpBranch %262
%262 = OpLabel
%267 = OpPhi %bool %false %255 %266 %261
OpSelectionMerge %269 None
OpBranchConditional %267 %268 %269
%268 = OpLabel
OpStore %272 %271
%273 = OpFunctionCall %bool %takes_int4 %272
OpBranch %269
%269 = OpLabel
%274 = OpPhi %bool %false %262 %273 %268
OpSelectionMerge %278 None
OpBranchConditional %274 %276 %277
%276 = OpLabel
%279 = OpAccessChain %_ptr_Uniform_v4float %31 %int_0
%282 = OpLoad %v4float %279
OpStore %275 %282
OpBranch %278
%277 = OpLabel
%283 = OpAccessChain %_ptr_Uniform_v4float %31 %int_1
%284 = OpLoad %v4float %283
OpStore %275 %284
OpBranch %278
%278 = OpLabel
%285 = OpLoad %v4float %275
OpReturnValue %285
OpFunctionEnd
