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
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
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
%130 = OpConstantComposite %v2float %float_2 %float_2
%float_3 = OpConstant %float 3
%137 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%float_4 = OpConstant %float 4
%144 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%231 = OpConstantComposite %v2bool %true %true
%237 = OpConstantComposite %v3bool %true %true %true
%243 = OpConstantComposite %v4bool %true %true %true %true
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%256 = OpConstantComposite %v2int %int_2 %int_2
%int_3 = OpConstant %int 3
%263 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%int_4 = OpConstant %int 4
%270 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
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
%131 = OpVariable %_ptr_Function_v2float Function
%138 = OpVariable %_ptr_Function_v3float Function
%145 = OpVariable %_ptr_Function_v4float Function
%154 = OpVariable %_ptr_Function_mat2v2float Function
%163 = OpVariable %_ptr_Function_mat3v3float Function
%173 = OpVariable %_ptr_Function_mat4v4float Function
%179 = OpVariable %_ptr_Function_float Function
%184 = OpVariable %_ptr_Function_v2float Function
%189 = OpVariable %_ptr_Function_v3float Function
%194 = OpVariable %_ptr_Function_v4float Function
%202 = OpVariable %_ptr_Function_mat2v2float Function
%211 = OpVariable %_ptr_Function_mat3v3float Function
%221 = OpVariable %_ptr_Function_mat4v4float Function
%226 = OpVariable %_ptr_Function_bool Function
%232 = OpVariable %_ptr_Function_v2bool Function
%238 = OpVariable %_ptr_Function_v3bool Function
%244 = OpVariable %_ptr_Function_v4bool Function
%250 = OpVariable %_ptr_Function_int Function
%257 = OpVariable %_ptr_Function_v2int Function
%264 = OpVariable %_ptr_Function_v3int Function
%271 = OpVariable %_ptr_Function_v4int Function
%274 = OpVariable %_ptr_Function_v4float Function
OpSelectionMerge %128 None
OpBranchConditional %true %127 %128
%127 = OpLabel
OpStore %131 %130
%132 = OpFunctionCall %bool %takes_float2 %131
OpBranch %128
%128 = OpLabel
%133 = OpPhi %bool %false %125 %132 %127
OpSelectionMerge %135 None
OpBranchConditional %133 %134 %135
%134 = OpLabel
OpStore %138 %137
%139 = OpFunctionCall %bool %takes_float3 %138
OpBranch %135
%135 = OpLabel
%140 = OpPhi %bool %false %128 %139 %134
OpSelectionMerge %142 None
OpBranchConditional %140 %141 %142
%141 = OpLabel
OpStore %145 %144
%146 = OpFunctionCall %bool %takes_float4 %145
OpBranch %142
%142 = OpLabel
%147 = OpPhi %bool %false %135 %146 %141
OpSelectionMerge %149 None
OpBranchConditional %147 %148 %149
%148 = OpLabel
%152 = OpCompositeConstruct %v2float %float_2 %float_0
%153 = OpCompositeConstruct %v2float %float_0 %float_2
%150 = OpCompositeConstruct %mat2v2float %152 %153
OpStore %154 %150
%155 = OpFunctionCall %bool %takes_float2x2 %154
OpBranch %149
%149 = OpLabel
%156 = OpPhi %bool %false %142 %155 %148
OpSelectionMerge %158 None
OpBranchConditional %156 %157 %158
%157 = OpLabel
%160 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%161 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%162 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%159 = OpCompositeConstruct %mat3v3float %160 %161 %162
OpStore %163 %159
%164 = OpFunctionCall %bool %takes_float3x3 %163
OpBranch %158
%158 = OpLabel
%165 = OpPhi %bool %false %149 %164 %157
OpSelectionMerge %167 None
OpBranchConditional %165 %166 %167
%166 = OpLabel
%169 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%170 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%171 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%172 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%168 = OpCompositeConstruct %mat4v4float %169 %170 %171 %172
OpStore %173 %168
%174 = OpFunctionCall %bool %takes_float4x4 %173
OpBranch %167
%167 = OpLabel
%175 = OpPhi %bool %false %158 %174 %166
OpSelectionMerge %177 None
OpBranchConditional %175 %176 %177
%176 = OpLabel
OpStore %179 %float_1
%180 = OpFunctionCall %bool %takes_half %179
OpBranch %177
%177 = OpLabel
%181 = OpPhi %bool %false %167 %180 %176
OpSelectionMerge %183 None
OpBranchConditional %181 %182 %183
%182 = OpLabel
OpStore %184 %130
%185 = OpFunctionCall %bool %takes_half2 %184
OpBranch %183
%183 = OpLabel
%186 = OpPhi %bool %false %177 %185 %182
OpSelectionMerge %188 None
OpBranchConditional %186 %187 %188
%187 = OpLabel
OpStore %189 %137
%190 = OpFunctionCall %bool %takes_half3 %189
OpBranch %188
%188 = OpLabel
%191 = OpPhi %bool %false %183 %190 %187
OpSelectionMerge %193 None
OpBranchConditional %191 %192 %193
%192 = OpLabel
OpStore %194 %144
%195 = OpFunctionCall %bool %takes_half4 %194
OpBranch %193
%193 = OpLabel
%196 = OpPhi %bool %false %188 %195 %192
OpSelectionMerge %198 None
OpBranchConditional %196 %197 %198
%197 = OpLabel
%200 = OpCompositeConstruct %v2float %float_2 %float_0
%201 = OpCompositeConstruct %v2float %float_0 %float_2
%199 = OpCompositeConstruct %mat2v2float %200 %201
OpStore %202 %199
%203 = OpFunctionCall %bool %takes_half2x2 %202
OpBranch %198
%198 = OpLabel
%204 = OpPhi %bool %false %193 %203 %197
OpSelectionMerge %206 None
OpBranchConditional %204 %205 %206
%205 = OpLabel
%208 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%209 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%210 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%207 = OpCompositeConstruct %mat3v3float %208 %209 %210
OpStore %211 %207
%212 = OpFunctionCall %bool %takes_half3x3 %211
OpBranch %206
%206 = OpLabel
%213 = OpPhi %bool %false %198 %212 %205
OpSelectionMerge %215 None
OpBranchConditional %213 %214 %215
%214 = OpLabel
%217 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%218 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%219 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%220 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%216 = OpCompositeConstruct %mat4v4float %217 %218 %219 %220
OpStore %221 %216
%222 = OpFunctionCall %bool %takes_half4x4 %221
OpBranch %215
%215 = OpLabel
%223 = OpPhi %bool %false %206 %222 %214
OpSelectionMerge %225 None
OpBranchConditional %223 %224 %225
%224 = OpLabel
OpStore %226 %true
%227 = OpFunctionCall %bool %takes_bool %226
OpBranch %225
%225 = OpLabel
%228 = OpPhi %bool %false %215 %227 %224
OpSelectionMerge %230 None
OpBranchConditional %228 %229 %230
%229 = OpLabel
OpStore %232 %231
%233 = OpFunctionCall %bool %takes_bool2 %232
OpBranch %230
%230 = OpLabel
%234 = OpPhi %bool %false %225 %233 %229
OpSelectionMerge %236 None
OpBranchConditional %234 %235 %236
%235 = OpLabel
OpStore %238 %237
%239 = OpFunctionCall %bool %takes_bool3 %238
OpBranch %236
%236 = OpLabel
%240 = OpPhi %bool %false %230 %239 %235
OpSelectionMerge %242 None
OpBranchConditional %240 %241 %242
%241 = OpLabel
OpStore %244 %243
%245 = OpFunctionCall %bool %takes_bool4 %244
OpBranch %242
%242 = OpLabel
%246 = OpPhi %bool %false %236 %245 %241
OpSelectionMerge %248 None
OpBranchConditional %246 %247 %248
%247 = OpLabel
OpStore %250 %int_1
%251 = OpFunctionCall %bool %takes_int %250
OpBranch %248
%248 = OpLabel
%252 = OpPhi %bool %false %242 %251 %247
OpSelectionMerge %254 None
OpBranchConditional %252 %253 %254
%253 = OpLabel
OpStore %257 %256
%258 = OpFunctionCall %bool %takes_int2 %257
OpBranch %254
%254 = OpLabel
%259 = OpPhi %bool %false %248 %258 %253
OpSelectionMerge %261 None
OpBranchConditional %259 %260 %261
%260 = OpLabel
OpStore %264 %263
%265 = OpFunctionCall %bool %takes_int3 %264
OpBranch %261
%261 = OpLabel
%266 = OpPhi %bool %false %254 %265 %260
OpSelectionMerge %268 None
OpBranchConditional %266 %267 %268
%267 = OpLabel
OpStore %271 %270
%272 = OpFunctionCall %bool %takes_int4 %271
OpBranch %268
%268 = OpLabel
%273 = OpPhi %bool %false %261 %272 %267
OpSelectionMerge %277 None
OpBranchConditional %273 %275 %276
%275 = OpLabel
%278 = OpAccessChain %_ptr_Uniform_v4float %31 %int_0
%281 = OpLoad %v4float %278
OpStore %274 %281
OpBranch %277
%276 = OpLabel
%282 = OpAccessChain %_ptr_Uniform_v4float %31 %int_1
%283 = OpLoad %v4float %282
OpStore %274 %283
OpBranch %277
%277 = OpLabel
%284 = OpLoad %v4float %274
OpReturnValue %284
OpFunctionEnd
