OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %_blend_set_color_luminance "_blend_set_color_luminance"
OpName %lum "lum"
OpName %result "result"
OpName %minComp "minComp"
OpName %maxComp "maxComp"
OpName %_4_d "_4_d"
OpName %_5_n "_5_n"
OpName %_6_d "_6_d"
OpName %_blend_set_color_saturation_helper "_blend_set_color_saturation_helper"
OpName %_7_n "_7_n"
OpName %_8_d "_8_d"
OpName %_blend_set_color_saturation "_blend_set_color_saturation"
OpName %sat "sat"
OpName %main "main"
OpName %_0_alpha "_0_alpha"
OpName %_1_sda "_1_sda"
OpName %_2_dsa "_2_dsa"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpDecorate %30 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Function_float = OpTypePointer Function %float
%17 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float %_ptr_Function_v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%29 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%118 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%149 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%150 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_v3float
%void = OpTypeVoid
%253 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%_blend_set_color_luminance = OpFunction %v3float None %17
%20 = OpFunctionParameter %_ptr_Function_v3float
%21 = OpFunctionParameter %_ptr_Function_float
%22 = OpFunctionParameter %_ptr_Function_v3float
%23 = OpLabel
%lum = OpVariable %_ptr_Function_float Function
%result = OpVariable %_ptr_Function_v3float Function
%minComp = OpVariable %_ptr_Function_float Function
%maxComp = OpVariable %_ptr_Function_float Function
%_4_d = OpVariable %_ptr_Function_float Function
%_5_n = OpVariable %_ptr_Function_v3float Function
%_6_d = OpVariable %_ptr_Function_float Function
%30 = OpLoad %v3float %22
%25 = OpDot %float %29 %30
OpStore %lum %25
%32 = OpLoad %float %lum
%34 = OpLoad %v3float %20
%33 = OpDot %float %29 %34
%35 = OpFSub %float %32 %33
%36 = OpLoad %v3float %20
%37 = OpCompositeConstruct %v3float %35 %35 %35
%38 = OpFAdd %v3float %37 %36
OpStore %result %38
%42 = OpLoad %v3float %result
%43 = OpCompositeExtract %float %42 0
%44 = OpLoad %v3float %result
%45 = OpCompositeExtract %float %44 1
%41 = OpExtInst %float %1 FMin %43 %45
%46 = OpLoad %v3float %result
%47 = OpCompositeExtract %float %46 2
%40 = OpExtInst %float %1 FMin %41 %47
OpStore %minComp %40
%51 = OpLoad %v3float %result
%52 = OpCompositeExtract %float %51 0
%53 = OpLoad %v3float %result
%54 = OpCompositeExtract %float %53 1
%50 = OpExtInst %float %1 FMax %52 %54
%55 = OpLoad %v3float %result
%56 = OpCompositeExtract %float %55 2
%49 = OpExtInst %float %1 FMax %50 %56
OpStore %maxComp %49
%58 = OpLoad %float %minComp
%60 = OpFOrdLessThan %bool %58 %float_0
OpSelectionMerge %62 None
OpBranchConditional %60 %61 %62
%61 = OpLabel
%63 = OpLoad %float %lum
%64 = OpLoad %float %minComp
%65 = OpFOrdNotEqual %bool %63 %64
OpBranch %62
%62 = OpLabel
%66 = OpPhi %bool %false %23 %65 %61
OpSelectionMerge %68 None
OpBranchConditional %66 %67 %68
%67 = OpLabel
%70 = OpLoad %float %lum
%71 = OpLoad %float %minComp
%72 = OpFSub %float %70 %71
OpStore %_4_d %72
%73 = OpLoad %float %lum
%74 = OpLoad %v3float %result
%75 = OpLoad %float %lum
%76 = OpCompositeConstruct %v3float %75 %75 %75
%77 = OpFSub %v3float %74 %76
%78 = OpLoad %float %lum
%79 = OpLoad %float %_4_d
%80 = OpFDiv %float %78 %79
%81 = OpVectorTimesScalar %v3float %77 %80
%82 = OpCompositeConstruct %v3float %73 %73 %73
%83 = OpFAdd %v3float %82 %81
OpStore %result %83
OpBranch %68
%68 = OpLabel
%84 = OpLoad %float %maxComp
%85 = OpLoad %float %21
%86 = OpFOrdGreaterThan %bool %84 %85
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%89 = OpLoad %float %maxComp
%90 = OpLoad %float %lum
%91 = OpFOrdNotEqual %bool %89 %90
OpBranch %88
%88 = OpLabel
%92 = OpPhi %bool %false %68 %91 %87
OpSelectionMerge %95 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%97 = OpLoad %v3float %result
%98 = OpLoad %float %lum
%99 = OpCompositeConstruct %v3float %98 %98 %98
%100 = OpFSub %v3float %97 %99
%101 = OpLoad %float %21
%102 = OpLoad %float %lum
%103 = OpFSub %float %101 %102
%104 = OpVectorTimesScalar %v3float %100 %103
OpStore %_5_n %104
%106 = OpLoad %float %maxComp
%107 = OpLoad %float %lum
%108 = OpFSub %float %106 %107
OpStore %_6_d %108
%109 = OpLoad %float %lum
%110 = OpLoad %v3float %_5_n
%111 = OpLoad %float %_6_d
%113 = OpFDiv %float %float_1 %111
%114 = OpVectorTimesScalar %v3float %110 %113
%115 = OpCompositeConstruct %v3float %109 %109 %109
%116 = OpFAdd %v3float %115 %114
OpReturnValue %116
%94 = OpLabel
%117 = OpLoad %v3float %result
OpReturnValue %117
%95 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_saturation_helper = OpFunction %v3float None %118
%119 = OpFunctionParameter %_ptr_Function_v3float
%120 = OpFunctionParameter %_ptr_Function_float
%121 = OpLabel
%_7_n = OpVariable %_ptr_Function_float Function
%_8_d = OpVariable %_ptr_Function_float Function
%122 = OpLoad %v3float %119
%123 = OpCompositeExtract %float %122 0
%124 = OpLoad %v3float %119
%125 = OpCompositeExtract %float %124 2
%126 = OpFOrdLessThan %bool %123 %125
OpSelectionMerge %129 None
OpBranchConditional %126 %127 %128
%127 = OpLabel
%131 = OpLoad %float %120
%132 = OpLoad %v3float %119
%133 = OpCompositeExtract %float %132 1
%134 = OpLoad %v3float %119
%135 = OpCompositeExtract %float %134 0
%136 = OpFSub %float %133 %135
%137 = OpFMul %float %131 %136
OpStore %_7_n %137
%139 = OpLoad %v3float %119
%140 = OpCompositeExtract %float %139 2
%141 = OpLoad %v3float %119
%142 = OpCompositeExtract %float %141 0
%143 = OpFSub %float %140 %142
OpStore %_8_d %143
%144 = OpLoad %float %_7_n
%145 = OpLoad %float %_8_d
%146 = OpFDiv %float %144 %145
%147 = OpLoad %float %120
%148 = OpCompositeConstruct %v3float %float_0 %146 %147
OpReturnValue %148
%128 = OpLabel
OpReturnValue %149
%129 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_saturation = OpFunction %v3float None %150
%151 = OpFunctionParameter %_ptr_Function_v3float
%152 = OpFunctionParameter %_ptr_Function_v3float
%153 = OpLabel
%sat = OpVariable %_ptr_Function_float Function
%189 = OpVariable %_ptr_Function_v3float Function
%191 = OpVariable %_ptr_Function_float Function
%203 = OpVariable %_ptr_Function_v3float Function
%205 = OpVariable %_ptr_Function_float Function
%210 = OpVariable %_ptr_Function_v3float Function
%212 = OpVariable %_ptr_Function_float Function
%225 = OpVariable %_ptr_Function_v3float Function
%227 = OpVariable %_ptr_Function_float Function
%240 = OpVariable %_ptr_Function_v3float Function
%242 = OpVariable %_ptr_Function_float Function
%247 = OpVariable %_ptr_Function_v3float Function
%249 = OpVariable %_ptr_Function_float Function
%157 = OpLoad %v3float %152
%158 = OpCompositeExtract %float %157 0
%159 = OpLoad %v3float %152
%160 = OpCompositeExtract %float %159 1
%156 = OpExtInst %float %1 FMax %158 %160
%161 = OpLoad %v3float %152
%162 = OpCompositeExtract %float %161 2
%155 = OpExtInst %float %1 FMax %156 %162
%165 = OpLoad %v3float %152
%166 = OpCompositeExtract %float %165 0
%167 = OpLoad %v3float %152
%168 = OpCompositeExtract %float %167 1
%164 = OpExtInst %float %1 FMin %166 %168
%169 = OpLoad %v3float %152
%170 = OpCompositeExtract %float %169 2
%163 = OpExtInst %float %1 FMin %164 %170
%171 = OpFSub %float %155 %163
OpStore %sat %171
%172 = OpLoad %v3float %151
%173 = OpCompositeExtract %float %172 0
%174 = OpLoad %v3float %151
%175 = OpCompositeExtract %float %174 1
%176 = OpFOrdLessThanEqual %bool %173 %175
OpSelectionMerge %179 None
OpBranchConditional %176 %177 %178
%177 = OpLabel
%180 = OpLoad %v3float %151
%181 = OpCompositeExtract %float %180 1
%182 = OpLoad %v3float %151
%183 = OpCompositeExtract %float %182 2
%184 = OpFOrdLessThanEqual %bool %181 %183
OpSelectionMerge %187 None
OpBranchConditional %184 %185 %186
%185 = OpLabel
%188 = OpLoad %v3float %151
OpStore %189 %188
%190 = OpLoad %float %sat
OpStore %191 %190
%192 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %189 %191
OpReturnValue %192
%186 = OpLabel
%193 = OpLoad %v3float %151
%194 = OpCompositeExtract %float %193 0
%195 = OpLoad %v3float %151
%196 = OpCompositeExtract %float %195 2
%197 = OpFOrdLessThanEqual %bool %194 %196
OpSelectionMerge %200 None
OpBranchConditional %197 %198 %199
%198 = OpLabel
%201 = OpLoad %v3float %151
%202 = OpVectorShuffle %v3float %201 %201 0 2 1
OpStore %203 %202
%204 = OpLoad %float %sat
OpStore %205 %204
%206 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %203 %205
%207 = OpVectorShuffle %v3float %206 %206 0 2 1
OpReturnValue %207
%199 = OpLabel
%208 = OpLoad %v3float %151
%209 = OpVectorShuffle %v3float %208 %208 2 0 1
OpStore %210 %209
%211 = OpLoad %float %sat
OpStore %212 %211
%213 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %210 %212
%214 = OpVectorShuffle %v3float %213 %213 1 2 0
OpReturnValue %214
%200 = OpLabel
OpBranch %187
%187 = OpLabel
OpBranch %179
%178 = OpLabel
%215 = OpLoad %v3float %151
%216 = OpCompositeExtract %float %215 0
%217 = OpLoad %v3float %151
%218 = OpCompositeExtract %float %217 2
%219 = OpFOrdLessThanEqual %bool %216 %218
OpSelectionMerge %222 None
OpBranchConditional %219 %220 %221
%220 = OpLabel
%223 = OpLoad %v3float %151
%224 = OpVectorShuffle %v3float %223 %223 1 0 2
OpStore %225 %224
%226 = OpLoad %float %sat
OpStore %227 %226
%228 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %225 %227
%229 = OpVectorShuffle %v3float %228 %228 1 0 2
OpReturnValue %229
%221 = OpLabel
%230 = OpLoad %v3float %151
%231 = OpCompositeExtract %float %230 1
%232 = OpLoad %v3float %151
%233 = OpCompositeExtract %float %232 2
%234 = OpFOrdLessThanEqual %bool %231 %233
OpSelectionMerge %237 None
OpBranchConditional %234 %235 %236
%235 = OpLabel
%238 = OpLoad %v3float %151
%239 = OpVectorShuffle %v3float %238 %238 1 2 0
OpStore %240 %239
%241 = OpLoad %float %sat
OpStore %242 %241
%243 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %240 %242
%244 = OpVectorShuffle %v3float %243 %243 2 0 1
OpReturnValue %244
%236 = OpLabel
%245 = OpLoad %v3float %151
%246 = OpVectorShuffle %v3float %245 %245 2 1 0
OpStore %247 %246
%248 = OpLoad %float %sat
OpStore %249 %248
%250 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %247 %249
%251 = OpVectorShuffle %v3float %250 %250 2 1 0
OpReturnValue %251
%237 = OpLabel
OpBranch %222
%222 = OpLabel
OpBranch %179
%179 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %253
%254 = OpLabel
%_0_alpha = OpVariable %_ptr_Function_float Function
%_1_sda = OpVariable %_ptr_Function_v3float Function
%_2_dsa = OpVariable %_ptr_Function_v3float Function
%284 = OpVariable %_ptr_Function_v3float Function
%286 = OpVariable %_ptr_Function_v3float Function
%288 = OpVariable %_ptr_Function_v3float Function
%290 = OpVariable %_ptr_Function_float Function
%292 = OpVariable %_ptr_Function_v3float Function
%256 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%260 = OpLoad %v4float %256
%261 = OpCompositeExtract %float %260 3
%262 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%264 = OpLoad %v4float %262
%265 = OpCompositeExtract %float %264 3
%266 = OpFMul %float %261 %265
OpStore %_0_alpha %266
%268 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%269 = OpLoad %v4float %268
%270 = OpVectorShuffle %v3float %269 %269 0 1 2
%271 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%272 = OpLoad %v4float %271
%273 = OpCompositeExtract %float %272 3
%274 = OpVectorTimesScalar %v3float %270 %273
OpStore %_1_sda %274
%276 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%277 = OpLoad %v4float %276
%278 = OpVectorShuffle %v3float %277 %277 0 1 2
%279 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%280 = OpLoad %v4float %279
%281 = OpCompositeExtract %float %280 3
%282 = OpVectorTimesScalar %v3float %278 %281
OpStore %_2_dsa %282
%283 = OpLoad %v3float %_2_dsa
OpStore %284 %283
%285 = OpLoad %v3float %_1_sda
OpStore %286 %285
%287 = OpFunctionCall %v3float %_blend_set_color_saturation %284 %286
OpStore %288 %287
%289 = OpLoad %float %_0_alpha
OpStore %290 %289
%291 = OpLoad %v3float %_2_dsa
OpStore %292 %291
%293 = OpFunctionCall %v3float %_blend_set_color_luminance %288 %290 %292
%294 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%295 = OpLoad %v4float %294
%296 = OpVectorShuffle %v3float %295 %295 0 1 2
%297 = OpFAdd %v3float %293 %296
%298 = OpLoad %v3float %_2_dsa
%299 = OpFSub %v3float %297 %298
%300 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%301 = OpLoad %v4float %300
%302 = OpVectorShuffle %v3float %301 %301 0 1 2
%303 = OpFAdd %v3float %299 %302
%304 = OpLoad %v3float %_1_sda
%305 = OpFSub %v3float %303 %304
%306 = OpCompositeExtract %float %305 0
%307 = OpCompositeExtract %float %305 1
%308 = OpCompositeExtract %float %305 2
%309 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%310 = OpLoad %v4float %309
%311 = OpCompositeExtract %float %310 3
%312 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%313 = OpLoad %v4float %312
%314 = OpCompositeExtract %float %313 3
%315 = OpFAdd %float %311 %314
%316 = OpLoad %float %_0_alpha
%317 = OpFSub %float %315 %316
%318 = OpCompositeConstruct %v4float %306 %307 %308 %317
OpStore %sk_FragColor %318
OpReturn
OpFunctionEnd
