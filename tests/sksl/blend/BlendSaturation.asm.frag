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
OpName %_blend_set_color_saturation_helper "_blend_set_color_saturation_helper"
OpName %_7_n "_7_n"
OpName %_8_d "_8_d"
OpName %main "main"
OpName %_0_alpha "_0_alpha"
OpName %_1_sda "_1_sda"
OpName %_2_dsa "_2_dsa"
OpName %_3_blend_set_color_saturation "_3_blend_set_color_saturation"
OpName %_4_sat "_4_sat"
OpName %_5_blend_set_color_luminance "_5_blend_set_color_luminance"
OpName %_6_lum "_6_lum"
OpName %_7_result "_7_result"
OpName %_8_minComp "_8_minComp"
OpName %_9_maxComp "_9_maxComp"
OpName %_10_d "_10_d"
OpName %_11_n "_11_n"
OpName %_12_d "_12_d"
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
OpDecorate %21 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
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
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Function_float = OpTypePointer Function %float
%15 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%float_0 = OpConstant %float 0
%49 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%void = OpTypeVoid
%51 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%186 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%_blend_set_color_saturation_helper = OpFunction %v3float None %15
%18 = OpFunctionParameter %_ptr_Function_v3float
%19 = OpFunctionParameter %_ptr_Function_float
%20 = OpLabel
%_7_n = OpVariable %_ptr_Function_float Function
%_8_d = OpVariable %_ptr_Function_float Function
%21 = OpLoad %v3float %18
%22 = OpCompositeExtract %float %21 0
%23 = OpLoad %v3float %18
%24 = OpCompositeExtract %float %23 2
%25 = OpFOrdLessThan %bool %22 %24
OpSelectionMerge %28 None
OpBranchConditional %25 %26 %27
%26 = OpLabel
%30 = OpLoad %float %19
%31 = OpLoad %v3float %18
%32 = OpCompositeExtract %float %31 1
%33 = OpLoad %v3float %18
%34 = OpCompositeExtract %float %33 0
%35 = OpFSub %float %32 %34
%36 = OpFMul %float %30 %35
OpStore %_7_n %36
%38 = OpLoad %v3float %18
%39 = OpCompositeExtract %float %38 2
%40 = OpLoad %v3float %18
%41 = OpCompositeExtract %float %40 0
%42 = OpFSub %float %39 %41
OpStore %_8_d %42
%44 = OpLoad %float %_7_n
%45 = OpLoad %float %_8_d
%46 = OpFDiv %float %44 %45
%47 = OpLoad %float %19
%48 = OpCompositeConstruct %v3float %float_0 %46 %47
OpReturnValue %48
%27 = OpLabel
OpReturnValue %49
%28 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %51
%52 = OpLabel
%_0_alpha = OpVariable %_ptr_Function_float Function
%_1_sda = OpVariable %_ptr_Function_v3float Function
%_2_dsa = OpVariable %_ptr_Function_v3float Function
%_3_blend_set_color_saturation = OpVariable %_ptr_Function_v3float Function
%_4_sat = OpVariable %_ptr_Function_float Function
%117 = OpVariable %_ptr_Function_v3float Function
%119 = OpVariable %_ptr_Function_float Function
%131 = OpVariable %_ptr_Function_v3float Function
%133 = OpVariable %_ptr_Function_float Function
%138 = OpVariable %_ptr_Function_v3float Function
%140 = OpVariable %_ptr_Function_float Function
%153 = OpVariable %_ptr_Function_v3float Function
%155 = OpVariable %_ptr_Function_float Function
%168 = OpVariable %_ptr_Function_v3float Function
%170 = OpVariable %_ptr_Function_float Function
%175 = OpVariable %_ptr_Function_v3float Function
%177 = OpVariable %_ptr_Function_float Function
%_5_blend_set_color_luminance = OpVariable %_ptr_Function_v3float Function
%_6_lum = OpVariable %_ptr_Function_float Function
%_7_result = OpVariable %_ptr_Function_v3float Function
%_8_minComp = OpVariable %_ptr_Function_float Function
%_9_maxComp = OpVariable %_ptr_Function_float Function
%_10_d = OpVariable %_ptr_Function_float Function
%_11_n = OpVariable %_ptr_Function_v3float Function
%_12_d = OpVariable %_ptr_Function_float Function
%54 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%58 = OpLoad %v4float %54
%59 = OpCompositeExtract %float %58 3
%60 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%62 = OpLoad %v4float %60
%63 = OpCompositeExtract %float %62 3
%64 = OpFMul %float %59 %63
OpStore %_0_alpha %64
%66 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%67 = OpLoad %v4float %66
%68 = OpVectorShuffle %v3float %67 %67 0 1 2
%69 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%70 = OpLoad %v4float %69
%71 = OpCompositeExtract %float %70 3
%72 = OpVectorTimesScalar %v3float %68 %71
OpStore %_1_sda %72
%74 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%75 = OpLoad %v4float %74
%76 = OpVectorShuffle %v3float %75 %75 0 1 2
%77 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%78 = OpLoad %v4float %77
%79 = OpCompositeExtract %float %78 3
%80 = OpVectorTimesScalar %v3float %76 %79
OpStore %_2_dsa %80
%85 = OpLoad %v3float %_1_sda
%86 = OpCompositeExtract %float %85 0
%87 = OpLoad %v3float %_1_sda
%88 = OpCompositeExtract %float %87 1
%84 = OpExtInst %float %1 FMax %86 %88
%89 = OpLoad %v3float %_1_sda
%90 = OpCompositeExtract %float %89 2
%83 = OpExtInst %float %1 FMax %84 %90
%93 = OpLoad %v3float %_1_sda
%94 = OpCompositeExtract %float %93 0
%95 = OpLoad %v3float %_1_sda
%96 = OpCompositeExtract %float %95 1
%92 = OpExtInst %float %1 FMin %94 %96
%97 = OpLoad %v3float %_1_sda
%98 = OpCompositeExtract %float %97 2
%91 = OpExtInst %float %1 FMin %92 %98
%99 = OpFSub %float %83 %91
OpStore %_4_sat %99
%100 = OpLoad %v3float %_2_dsa
%101 = OpCompositeExtract %float %100 0
%102 = OpLoad %v3float %_2_dsa
%103 = OpCompositeExtract %float %102 1
%104 = OpFOrdLessThanEqual %bool %101 %103
OpSelectionMerge %107 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
%108 = OpLoad %v3float %_2_dsa
%109 = OpCompositeExtract %float %108 1
%110 = OpLoad %v3float %_2_dsa
%111 = OpCompositeExtract %float %110 2
%112 = OpFOrdLessThanEqual %bool %109 %111
OpSelectionMerge %115 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
%116 = OpLoad %v3float %_2_dsa
OpStore %117 %116
%118 = OpLoad %float %_4_sat
OpStore %119 %118
%120 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %117 %119
OpStore %_3_blend_set_color_saturation %120
OpBranch %115
%114 = OpLabel
%121 = OpLoad %v3float %_2_dsa
%122 = OpCompositeExtract %float %121 0
%123 = OpLoad %v3float %_2_dsa
%124 = OpCompositeExtract %float %123 2
%125 = OpFOrdLessThanEqual %bool %122 %124
OpSelectionMerge %128 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%129 = OpLoad %v3float %_2_dsa
%130 = OpVectorShuffle %v3float %129 %129 0 2 1
OpStore %131 %130
%132 = OpLoad %float %_4_sat
OpStore %133 %132
%134 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %131 %133
%135 = OpVectorShuffle %v3float %134 %134 0 2 1
OpStore %_3_blend_set_color_saturation %135
OpBranch %128
%127 = OpLabel
%136 = OpLoad %v3float %_2_dsa
%137 = OpVectorShuffle %v3float %136 %136 2 0 1
OpStore %138 %137
%139 = OpLoad %float %_4_sat
OpStore %140 %139
%141 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %138 %140
%142 = OpVectorShuffle %v3float %141 %141 1 2 0
OpStore %_3_blend_set_color_saturation %142
OpBranch %128
%128 = OpLabel
OpBranch %115
%115 = OpLabel
OpBranch %107
%106 = OpLabel
%143 = OpLoad %v3float %_2_dsa
%144 = OpCompositeExtract %float %143 0
%145 = OpLoad %v3float %_2_dsa
%146 = OpCompositeExtract %float %145 2
%147 = OpFOrdLessThanEqual %bool %144 %146
OpSelectionMerge %150 None
OpBranchConditional %147 %148 %149
%148 = OpLabel
%151 = OpLoad %v3float %_2_dsa
%152 = OpVectorShuffle %v3float %151 %151 1 0 2
OpStore %153 %152
%154 = OpLoad %float %_4_sat
OpStore %155 %154
%156 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %153 %155
%157 = OpVectorShuffle %v3float %156 %156 1 0 2
OpStore %_3_blend_set_color_saturation %157
OpBranch %150
%149 = OpLabel
%158 = OpLoad %v3float %_2_dsa
%159 = OpCompositeExtract %float %158 1
%160 = OpLoad %v3float %_2_dsa
%161 = OpCompositeExtract %float %160 2
%162 = OpFOrdLessThanEqual %bool %159 %161
OpSelectionMerge %165 None
OpBranchConditional %162 %163 %164
%163 = OpLabel
%166 = OpLoad %v3float %_2_dsa
%167 = OpVectorShuffle %v3float %166 %166 1 2 0
OpStore %168 %167
%169 = OpLoad %float %_4_sat
OpStore %170 %169
%171 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %168 %170
%172 = OpVectorShuffle %v3float %171 %171 2 0 1
OpStore %_3_blend_set_color_saturation %172
OpBranch %165
%164 = OpLabel
%173 = OpLoad %v3float %_2_dsa
%174 = OpVectorShuffle %v3float %173 %173 2 1 0
OpStore %175 %174
%176 = OpLoad %float %_4_sat
OpStore %177 %176
%178 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %175 %177
%179 = OpVectorShuffle %v3float %178 %178 2 1 0
OpStore %_3_blend_set_color_saturation %179
OpBranch %165
%165 = OpLabel
OpBranch %150
%150 = OpLabel
OpBranch %107
%107 = OpLabel
%187 = OpLoad %v3float %_2_dsa
%182 = OpDot %float %186 %187
OpStore %_6_lum %182
%189 = OpLoad %float %_6_lum
%191 = OpLoad %v3float %_3_blend_set_color_saturation
%190 = OpDot %float %186 %191
%192 = OpFSub %float %189 %190
%193 = OpLoad %v3float %_3_blend_set_color_saturation
%194 = OpCompositeConstruct %v3float %192 %192 %192
%195 = OpFAdd %v3float %194 %193
OpStore %_7_result %195
%199 = OpLoad %v3float %_7_result
%200 = OpCompositeExtract %float %199 0
%201 = OpLoad %v3float %_7_result
%202 = OpCompositeExtract %float %201 1
%198 = OpExtInst %float %1 FMin %200 %202
%203 = OpLoad %v3float %_7_result
%204 = OpCompositeExtract %float %203 2
%197 = OpExtInst %float %1 FMin %198 %204
OpStore %_8_minComp %197
%208 = OpLoad %v3float %_7_result
%209 = OpCompositeExtract %float %208 0
%210 = OpLoad %v3float %_7_result
%211 = OpCompositeExtract %float %210 1
%207 = OpExtInst %float %1 FMax %209 %211
%212 = OpLoad %v3float %_7_result
%213 = OpCompositeExtract %float %212 2
%206 = OpExtInst %float %1 FMax %207 %213
OpStore %_9_maxComp %206
%215 = OpLoad %float %_8_minComp
%216 = OpFOrdLessThan %bool %215 %float_0
OpSelectionMerge %218 None
OpBranchConditional %216 %217 %218
%217 = OpLabel
%219 = OpLoad %float %_6_lum
%220 = OpLoad %float %_8_minComp
%221 = OpFOrdNotEqual %bool %219 %220
OpBranch %218
%218 = OpLabel
%222 = OpPhi %bool %false %107 %221 %217
OpSelectionMerge %224 None
OpBranchConditional %222 %223 %224
%223 = OpLabel
%226 = OpLoad %float %_6_lum
%227 = OpLoad %float %_8_minComp
%228 = OpFSub %float %226 %227
OpStore %_10_d %228
%229 = OpLoad %float %_6_lum
%230 = OpLoad %v3float %_7_result
%231 = OpLoad %float %_6_lum
%232 = OpCompositeConstruct %v3float %231 %231 %231
%233 = OpFSub %v3float %230 %232
%234 = OpLoad %float %_6_lum
%235 = OpLoad %float %_10_d
%236 = OpFDiv %float %234 %235
%237 = OpVectorTimesScalar %v3float %233 %236
%238 = OpCompositeConstruct %v3float %229 %229 %229
%239 = OpFAdd %v3float %238 %237
OpStore %_7_result %239
OpBranch %224
%224 = OpLabel
%240 = OpLoad %float %_9_maxComp
%241 = OpLoad %float %_0_alpha
%242 = OpFOrdGreaterThan %bool %240 %241
OpSelectionMerge %244 None
OpBranchConditional %242 %243 %244
%243 = OpLabel
%245 = OpLoad %float %_9_maxComp
%246 = OpLoad %float %_6_lum
%247 = OpFOrdNotEqual %bool %245 %246
OpBranch %244
%244 = OpLabel
%248 = OpPhi %bool %false %224 %247 %243
OpSelectionMerge %251 None
OpBranchConditional %248 %249 %250
%249 = OpLabel
%253 = OpLoad %v3float %_7_result
%254 = OpLoad %float %_6_lum
%255 = OpCompositeConstruct %v3float %254 %254 %254
%256 = OpFSub %v3float %253 %255
%257 = OpLoad %float %_0_alpha
%258 = OpLoad %float %_6_lum
%259 = OpFSub %float %257 %258
%260 = OpVectorTimesScalar %v3float %256 %259
OpStore %_11_n %260
%262 = OpLoad %float %_9_maxComp
%263 = OpLoad %float %_6_lum
%264 = OpFSub %float %262 %263
OpStore %_12_d %264
%265 = OpLoad %float %_6_lum
%266 = OpLoad %v3float %_11_n
%267 = OpLoad %float %_12_d
%269 = OpFDiv %float %float_1 %267
%270 = OpVectorTimesScalar %v3float %266 %269
%271 = OpCompositeConstruct %v3float %265 %265 %265
%272 = OpFAdd %v3float %271 %270
OpStore %_5_blend_set_color_luminance %272
OpBranch %251
%250 = OpLabel
%273 = OpLoad %v3float %_7_result
OpStore %_5_blend_set_color_luminance %273
OpBranch %251
%251 = OpLabel
%274 = OpLoad %v3float %_5_blend_set_color_luminance
%275 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%276 = OpLoad %v4float %275
%277 = OpVectorShuffle %v3float %276 %276 0 1 2
%278 = OpFAdd %v3float %274 %277
%279 = OpLoad %v3float %_2_dsa
%280 = OpFSub %v3float %278 %279
%281 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%282 = OpLoad %v4float %281
%283 = OpVectorShuffle %v3float %282 %282 0 1 2
%284 = OpFAdd %v3float %280 %283
%285 = OpLoad %v3float %_1_sda
%286 = OpFSub %v3float %284 %285
%287 = OpCompositeExtract %float %286 0
%288 = OpCompositeExtract %float %286 1
%289 = OpCompositeExtract %float %286 2
%290 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%291 = OpLoad %v4float %290
%292 = OpCompositeExtract %float %291 3
%293 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%294 = OpLoad %v4float %293
%295 = OpCompositeExtract %float %294 3
%296 = OpFAdd %float %292 %295
%297 = OpLoad %float %_0_alpha
%298 = OpFSub %float %296 %297
%299 = OpCompositeConstruct %v4float %287 %288 %289 %298
OpStore %sk_FragColor %299
OpReturn
OpFunctionEnd
