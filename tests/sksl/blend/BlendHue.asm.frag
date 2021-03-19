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
OpDecorate %37 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
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
%45 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%void = OpTypeVoid
%47 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%182 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%_blend_set_color_saturation_helper = OpFunction %v3float None %15
%18 = OpFunctionParameter %_ptr_Function_v3float
%19 = OpFunctionParameter %_ptr_Function_float
%20 = OpLabel
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
%37 = OpLoad %v3float %18
%38 = OpCompositeExtract %float %37 2
%39 = OpLoad %v3float %18
%40 = OpCompositeExtract %float %39 0
%41 = OpFSub %float %38 %40
%42 = OpFDiv %float %36 %41
%43 = OpLoad %float %19
%44 = OpCompositeConstruct %v3float %float_0 %42 %43
OpReturnValue %44
%27 = OpLabel
OpReturnValue %45
%28 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %47
%48 = OpLabel
%_0_alpha = OpVariable %_ptr_Function_float Function
%_1_sda = OpVariable %_ptr_Function_v3float Function
%_2_dsa = OpVariable %_ptr_Function_v3float Function
%_3_blend_set_color_saturation = OpVariable %_ptr_Function_v3float Function
%_4_sat = OpVariable %_ptr_Function_float Function
%113 = OpVariable %_ptr_Function_v3float Function
%115 = OpVariable %_ptr_Function_float Function
%127 = OpVariable %_ptr_Function_v3float Function
%129 = OpVariable %_ptr_Function_float Function
%134 = OpVariable %_ptr_Function_v3float Function
%136 = OpVariable %_ptr_Function_float Function
%149 = OpVariable %_ptr_Function_v3float Function
%151 = OpVariable %_ptr_Function_float Function
%164 = OpVariable %_ptr_Function_v3float Function
%166 = OpVariable %_ptr_Function_float Function
%171 = OpVariable %_ptr_Function_v3float Function
%173 = OpVariable %_ptr_Function_float Function
%_5_blend_set_color_luminance = OpVariable %_ptr_Function_v3float Function
%_6_lum = OpVariable %_ptr_Function_float Function
%_7_result = OpVariable %_ptr_Function_v3float Function
%_8_minComp = OpVariable %_ptr_Function_float Function
%_9_maxComp = OpVariable %_ptr_Function_float Function
%50 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%54 = OpLoad %v4float %50
%55 = OpCompositeExtract %float %54 3
%56 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%58 = OpLoad %v4float %56
%59 = OpCompositeExtract %float %58 3
%60 = OpFMul %float %55 %59
OpStore %_0_alpha %60
%62 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%63 = OpLoad %v4float %62
%64 = OpVectorShuffle %v3float %63 %63 0 1 2
%65 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%66 = OpLoad %v4float %65
%67 = OpCompositeExtract %float %66 3
%68 = OpVectorTimesScalar %v3float %64 %67
OpStore %_1_sda %68
%70 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%71 = OpLoad %v4float %70
%72 = OpVectorShuffle %v3float %71 %71 0 1 2
%73 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%74 = OpLoad %v4float %73
%75 = OpCompositeExtract %float %74 3
%76 = OpVectorTimesScalar %v3float %72 %75
OpStore %_2_dsa %76
%81 = OpLoad %v3float %_2_dsa
%82 = OpCompositeExtract %float %81 0
%83 = OpLoad %v3float %_2_dsa
%84 = OpCompositeExtract %float %83 1
%80 = OpExtInst %float %1 FMax %82 %84
%85 = OpLoad %v3float %_2_dsa
%86 = OpCompositeExtract %float %85 2
%79 = OpExtInst %float %1 FMax %80 %86
%89 = OpLoad %v3float %_2_dsa
%90 = OpCompositeExtract %float %89 0
%91 = OpLoad %v3float %_2_dsa
%92 = OpCompositeExtract %float %91 1
%88 = OpExtInst %float %1 FMin %90 %92
%93 = OpLoad %v3float %_2_dsa
%94 = OpCompositeExtract %float %93 2
%87 = OpExtInst %float %1 FMin %88 %94
%95 = OpFSub %float %79 %87
OpStore %_4_sat %95
%96 = OpLoad %v3float %_1_sda
%97 = OpCompositeExtract %float %96 0
%98 = OpLoad %v3float %_1_sda
%99 = OpCompositeExtract %float %98 1
%100 = OpFOrdLessThanEqual %bool %97 %99
OpSelectionMerge %103 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%104 = OpLoad %v3float %_1_sda
%105 = OpCompositeExtract %float %104 1
%106 = OpLoad %v3float %_1_sda
%107 = OpCompositeExtract %float %106 2
%108 = OpFOrdLessThanEqual %bool %105 %107
OpSelectionMerge %111 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%112 = OpLoad %v3float %_1_sda
OpStore %113 %112
%114 = OpLoad %float %_4_sat
OpStore %115 %114
%116 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %113 %115
OpStore %_3_blend_set_color_saturation %116
OpBranch %111
%110 = OpLabel
%117 = OpLoad %v3float %_1_sda
%118 = OpCompositeExtract %float %117 0
%119 = OpLoad %v3float %_1_sda
%120 = OpCompositeExtract %float %119 2
%121 = OpFOrdLessThanEqual %bool %118 %120
OpSelectionMerge %124 None
OpBranchConditional %121 %122 %123
%122 = OpLabel
%125 = OpLoad %v3float %_1_sda
%126 = OpVectorShuffle %v3float %125 %125 0 2 1
OpStore %127 %126
%128 = OpLoad %float %_4_sat
OpStore %129 %128
%130 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %127 %129
%131 = OpVectorShuffle %v3float %130 %130 0 2 1
OpStore %_3_blend_set_color_saturation %131
OpBranch %124
%123 = OpLabel
%132 = OpLoad %v3float %_1_sda
%133 = OpVectorShuffle %v3float %132 %132 2 0 1
OpStore %134 %133
%135 = OpLoad %float %_4_sat
OpStore %136 %135
%137 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %134 %136
%138 = OpVectorShuffle %v3float %137 %137 1 2 0
OpStore %_3_blend_set_color_saturation %138
OpBranch %124
%124 = OpLabel
OpBranch %111
%111 = OpLabel
OpBranch %103
%102 = OpLabel
%139 = OpLoad %v3float %_1_sda
%140 = OpCompositeExtract %float %139 0
%141 = OpLoad %v3float %_1_sda
%142 = OpCompositeExtract %float %141 2
%143 = OpFOrdLessThanEqual %bool %140 %142
OpSelectionMerge %146 None
OpBranchConditional %143 %144 %145
%144 = OpLabel
%147 = OpLoad %v3float %_1_sda
%148 = OpVectorShuffle %v3float %147 %147 1 0 2
OpStore %149 %148
%150 = OpLoad %float %_4_sat
OpStore %151 %150
%152 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %149 %151
%153 = OpVectorShuffle %v3float %152 %152 1 0 2
OpStore %_3_blend_set_color_saturation %153
OpBranch %146
%145 = OpLabel
%154 = OpLoad %v3float %_1_sda
%155 = OpCompositeExtract %float %154 1
%156 = OpLoad %v3float %_1_sda
%157 = OpCompositeExtract %float %156 2
%158 = OpFOrdLessThanEqual %bool %155 %157
OpSelectionMerge %161 None
OpBranchConditional %158 %159 %160
%159 = OpLabel
%162 = OpLoad %v3float %_1_sda
%163 = OpVectorShuffle %v3float %162 %162 1 2 0
OpStore %164 %163
%165 = OpLoad %float %_4_sat
OpStore %166 %165
%167 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %164 %166
%168 = OpVectorShuffle %v3float %167 %167 2 0 1
OpStore %_3_blend_set_color_saturation %168
OpBranch %161
%160 = OpLabel
%169 = OpLoad %v3float %_1_sda
%170 = OpVectorShuffle %v3float %169 %169 2 1 0
OpStore %171 %170
%172 = OpLoad %float %_4_sat
OpStore %173 %172
%174 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %171 %173
%175 = OpVectorShuffle %v3float %174 %174 2 1 0
OpStore %_3_blend_set_color_saturation %175
OpBranch %161
%161 = OpLabel
OpBranch %146
%146 = OpLabel
OpBranch %103
%103 = OpLabel
%183 = OpLoad %v3float %_2_dsa
%178 = OpDot %float %182 %183
OpStore %_6_lum %178
%185 = OpLoad %float %_6_lum
%187 = OpLoad %v3float %_3_blend_set_color_saturation
%186 = OpDot %float %182 %187
%188 = OpFSub %float %185 %186
%189 = OpLoad %v3float %_3_blend_set_color_saturation
%190 = OpCompositeConstruct %v3float %188 %188 %188
%191 = OpFAdd %v3float %190 %189
OpStore %_7_result %191
%195 = OpLoad %v3float %_7_result
%196 = OpCompositeExtract %float %195 0
%197 = OpLoad %v3float %_7_result
%198 = OpCompositeExtract %float %197 1
%194 = OpExtInst %float %1 FMin %196 %198
%199 = OpLoad %v3float %_7_result
%200 = OpCompositeExtract %float %199 2
%193 = OpExtInst %float %1 FMin %194 %200
OpStore %_8_minComp %193
%204 = OpLoad %v3float %_7_result
%205 = OpCompositeExtract %float %204 0
%206 = OpLoad %v3float %_7_result
%207 = OpCompositeExtract %float %206 1
%203 = OpExtInst %float %1 FMax %205 %207
%208 = OpLoad %v3float %_7_result
%209 = OpCompositeExtract %float %208 2
%202 = OpExtInst %float %1 FMax %203 %209
OpStore %_9_maxComp %202
%211 = OpLoad %float %_8_minComp
%212 = OpFOrdLessThan %bool %211 %float_0
OpSelectionMerge %214 None
OpBranchConditional %212 %213 %214
%213 = OpLabel
%215 = OpLoad %float %_6_lum
%216 = OpLoad %float %_8_minComp
%217 = OpFOrdNotEqual %bool %215 %216
OpBranch %214
%214 = OpLabel
%218 = OpPhi %bool %false %103 %217 %213
OpSelectionMerge %220 None
OpBranchConditional %218 %219 %220
%219 = OpLabel
%221 = OpLoad %float %_6_lum
%222 = OpLoad %v3float %_7_result
%223 = OpLoad %float %_6_lum
%224 = OpCompositeConstruct %v3float %223 %223 %223
%225 = OpFSub %v3float %222 %224
%226 = OpLoad %float %_6_lum
%227 = OpLoad %float %_6_lum
%228 = OpLoad %float %_8_minComp
%229 = OpFSub %float %227 %228
%230 = OpFDiv %float %226 %229
%231 = OpVectorTimesScalar %v3float %225 %230
%232 = OpCompositeConstruct %v3float %221 %221 %221
%233 = OpFAdd %v3float %232 %231
OpStore %_7_result %233
OpBranch %220
%220 = OpLabel
%234 = OpLoad %float %_9_maxComp
%235 = OpLoad %float %_0_alpha
%236 = OpFOrdGreaterThan %bool %234 %235
OpSelectionMerge %238 None
OpBranchConditional %236 %237 %238
%237 = OpLabel
%239 = OpLoad %float %_9_maxComp
%240 = OpLoad %float %_6_lum
%241 = OpFOrdNotEqual %bool %239 %240
OpBranch %238
%238 = OpLabel
%242 = OpPhi %bool %false %220 %241 %237
OpSelectionMerge %245 None
OpBranchConditional %242 %243 %244
%243 = OpLabel
%246 = OpLoad %float %_6_lum
%247 = OpLoad %v3float %_7_result
%248 = OpLoad %float %_6_lum
%249 = OpCompositeConstruct %v3float %248 %248 %248
%250 = OpFSub %v3float %247 %249
%251 = OpLoad %float %_0_alpha
%252 = OpLoad %float %_6_lum
%253 = OpFSub %float %251 %252
%254 = OpVectorTimesScalar %v3float %250 %253
%255 = OpLoad %float %_9_maxComp
%256 = OpLoad %float %_6_lum
%257 = OpFSub %float %255 %256
%259 = OpFDiv %float %float_1 %257
%260 = OpVectorTimesScalar %v3float %254 %259
%261 = OpCompositeConstruct %v3float %246 %246 %246
%262 = OpFAdd %v3float %261 %260
OpStore %_5_blend_set_color_luminance %262
OpBranch %245
%244 = OpLabel
%263 = OpLoad %v3float %_7_result
OpStore %_5_blend_set_color_luminance %263
OpBranch %245
%245 = OpLabel
%264 = OpLoad %v3float %_5_blend_set_color_luminance
%265 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%266 = OpLoad %v4float %265
%267 = OpVectorShuffle %v3float %266 %266 0 1 2
%268 = OpFAdd %v3float %264 %267
%269 = OpLoad %v3float %_2_dsa
%270 = OpFSub %v3float %268 %269
%271 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%272 = OpLoad %v4float %271
%273 = OpVectorShuffle %v3float %272 %272 0 1 2
%274 = OpFAdd %v3float %270 %273
%275 = OpLoad %v3float %_1_sda
%276 = OpFSub %v3float %274 %275
%277 = OpCompositeExtract %float %276 0
%278 = OpCompositeExtract %float %276 1
%279 = OpCompositeExtract %float %276 2
%280 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%281 = OpLoad %v4float %280
%282 = OpCompositeExtract %float %281 3
%283 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%284 = OpLoad %v4float %283
%285 = OpCompositeExtract %float %284 3
%286 = OpFAdd %float %282 %285
%287 = OpLoad %float %_0_alpha
%288 = OpFSub %float %286 %287
%289 = OpCompositeConstruct %v4float %277 %278 %279 %288
OpStore %sk_FragColor %289
OpReturn
OpFunctionEnd
