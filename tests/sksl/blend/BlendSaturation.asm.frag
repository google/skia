OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
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
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
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
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%src = OpVariable %_ptr_Input_v4float Input
%dst = OpVariable %_ptr_Input_v4float Input
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Function_float = OpTypePointer Function %float
%15 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%float_0 = OpConstant %float 0
%49 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%void = OpTypeVoid
%51 = OpTypeFunction %void
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%176 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
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
%107 = OpVariable %_ptr_Function_v3float Function
%109 = OpVariable %_ptr_Function_float Function
%121 = OpVariable %_ptr_Function_v3float Function
%123 = OpVariable %_ptr_Function_float Function
%128 = OpVariable %_ptr_Function_v3float Function
%130 = OpVariable %_ptr_Function_float Function
%143 = OpVariable %_ptr_Function_v3float Function
%145 = OpVariable %_ptr_Function_float Function
%158 = OpVariable %_ptr_Function_v3float Function
%160 = OpVariable %_ptr_Function_float Function
%165 = OpVariable %_ptr_Function_v3float Function
%167 = OpVariable %_ptr_Function_float Function
%_5_blend_set_color_luminance = OpVariable %_ptr_Function_v3float Function
%_6_lum = OpVariable %_ptr_Function_float Function
%_7_result = OpVariable %_ptr_Function_v3float Function
%_8_minComp = OpVariable %_ptr_Function_float Function
%_9_maxComp = OpVariable %_ptr_Function_float Function
%_10_d = OpVariable %_ptr_Function_float Function
%_11_n = OpVariable %_ptr_Function_v3float Function
%_12_d = OpVariable %_ptr_Function_float Function
%54 = OpLoad %v4float %dst
%55 = OpCompositeExtract %float %54 3
%56 = OpLoad %v4float %src
%57 = OpCompositeExtract %float %56 3
%58 = OpFMul %float %55 %57
OpStore %_0_alpha %58
%60 = OpLoad %v4float %src
%61 = OpVectorShuffle %v3float %60 %60 0 1 2
%62 = OpLoad %v4float %dst
%63 = OpCompositeExtract %float %62 3
%64 = OpVectorTimesScalar %v3float %61 %63
OpStore %_1_sda %64
%66 = OpLoad %v4float %dst
%67 = OpVectorShuffle %v3float %66 %66 0 1 2
%68 = OpLoad %v4float %src
%69 = OpCompositeExtract %float %68 3
%70 = OpVectorTimesScalar %v3float %67 %69
OpStore %_2_dsa %70
%75 = OpLoad %v3float %_1_sda
%76 = OpCompositeExtract %float %75 0
%77 = OpLoad %v3float %_1_sda
%78 = OpCompositeExtract %float %77 1
%74 = OpExtInst %float %1 FMax %76 %78
%79 = OpLoad %v3float %_1_sda
%80 = OpCompositeExtract %float %79 2
%73 = OpExtInst %float %1 FMax %74 %80
%83 = OpLoad %v3float %_1_sda
%84 = OpCompositeExtract %float %83 0
%85 = OpLoad %v3float %_1_sda
%86 = OpCompositeExtract %float %85 1
%82 = OpExtInst %float %1 FMin %84 %86
%87 = OpLoad %v3float %_1_sda
%88 = OpCompositeExtract %float %87 2
%81 = OpExtInst %float %1 FMin %82 %88
%89 = OpFSub %float %73 %81
OpStore %_4_sat %89
%90 = OpLoad %v3float %_2_dsa
%91 = OpCompositeExtract %float %90 0
%92 = OpLoad %v3float %_2_dsa
%93 = OpCompositeExtract %float %92 1
%94 = OpFOrdLessThanEqual %bool %91 %93
OpSelectionMerge %97 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%98 = OpLoad %v3float %_2_dsa
%99 = OpCompositeExtract %float %98 1
%100 = OpLoad %v3float %_2_dsa
%101 = OpCompositeExtract %float %100 2
%102 = OpFOrdLessThanEqual %bool %99 %101
OpSelectionMerge %105 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%106 = OpLoad %v3float %_2_dsa
OpStore %107 %106
%108 = OpLoad %float %_4_sat
OpStore %109 %108
%110 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %107 %109
OpStore %_3_blend_set_color_saturation %110
OpBranch %105
%104 = OpLabel
%111 = OpLoad %v3float %_2_dsa
%112 = OpCompositeExtract %float %111 0
%113 = OpLoad %v3float %_2_dsa
%114 = OpCompositeExtract %float %113 2
%115 = OpFOrdLessThanEqual %bool %112 %114
OpSelectionMerge %118 None
OpBranchConditional %115 %116 %117
%116 = OpLabel
%119 = OpLoad %v3float %_2_dsa
%120 = OpVectorShuffle %v3float %119 %119 0 2 1
OpStore %121 %120
%122 = OpLoad %float %_4_sat
OpStore %123 %122
%124 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %121 %123
%125 = OpVectorShuffle %v3float %124 %124 0 2 1
OpStore %_3_blend_set_color_saturation %125
OpBranch %118
%117 = OpLabel
%126 = OpLoad %v3float %_2_dsa
%127 = OpVectorShuffle %v3float %126 %126 2 0 1
OpStore %128 %127
%129 = OpLoad %float %_4_sat
OpStore %130 %129
%131 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %128 %130
%132 = OpVectorShuffle %v3float %131 %131 1 2 0
OpStore %_3_blend_set_color_saturation %132
OpBranch %118
%118 = OpLabel
OpBranch %105
%105 = OpLabel
OpBranch %97
%96 = OpLabel
%133 = OpLoad %v3float %_2_dsa
%134 = OpCompositeExtract %float %133 0
%135 = OpLoad %v3float %_2_dsa
%136 = OpCompositeExtract %float %135 2
%137 = OpFOrdLessThanEqual %bool %134 %136
OpSelectionMerge %140 None
OpBranchConditional %137 %138 %139
%138 = OpLabel
%141 = OpLoad %v3float %_2_dsa
%142 = OpVectorShuffle %v3float %141 %141 1 0 2
OpStore %143 %142
%144 = OpLoad %float %_4_sat
OpStore %145 %144
%146 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %143 %145
%147 = OpVectorShuffle %v3float %146 %146 1 0 2
OpStore %_3_blend_set_color_saturation %147
OpBranch %140
%139 = OpLabel
%148 = OpLoad %v3float %_2_dsa
%149 = OpCompositeExtract %float %148 1
%150 = OpLoad %v3float %_2_dsa
%151 = OpCompositeExtract %float %150 2
%152 = OpFOrdLessThanEqual %bool %149 %151
OpSelectionMerge %155 None
OpBranchConditional %152 %153 %154
%153 = OpLabel
%156 = OpLoad %v3float %_2_dsa
%157 = OpVectorShuffle %v3float %156 %156 1 2 0
OpStore %158 %157
%159 = OpLoad %float %_4_sat
OpStore %160 %159
%161 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %158 %160
%162 = OpVectorShuffle %v3float %161 %161 2 0 1
OpStore %_3_blend_set_color_saturation %162
OpBranch %155
%154 = OpLabel
%163 = OpLoad %v3float %_2_dsa
%164 = OpVectorShuffle %v3float %163 %163 2 1 0
OpStore %165 %164
%166 = OpLoad %float %_4_sat
OpStore %167 %166
%168 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %165 %167
%169 = OpVectorShuffle %v3float %168 %168 2 1 0
OpStore %_3_blend_set_color_saturation %169
OpBranch %155
%155 = OpLabel
OpBranch %140
%140 = OpLabel
OpBranch %97
%97 = OpLabel
%177 = OpLoad %v3float %_2_dsa
%172 = OpDot %float %176 %177
OpStore %_6_lum %172
%179 = OpLoad %float %_6_lum
%181 = OpLoad %v3float %_3_blend_set_color_saturation
%180 = OpDot %float %176 %181
%182 = OpFSub %float %179 %180
%183 = OpLoad %v3float %_3_blend_set_color_saturation
%184 = OpCompositeConstruct %v3float %182 %182 %182
%185 = OpFAdd %v3float %184 %183
OpStore %_7_result %185
%189 = OpLoad %v3float %_7_result
%190 = OpCompositeExtract %float %189 0
%191 = OpLoad %v3float %_7_result
%192 = OpCompositeExtract %float %191 1
%188 = OpExtInst %float %1 FMin %190 %192
%193 = OpLoad %v3float %_7_result
%194 = OpCompositeExtract %float %193 2
%187 = OpExtInst %float %1 FMin %188 %194
OpStore %_8_minComp %187
%198 = OpLoad %v3float %_7_result
%199 = OpCompositeExtract %float %198 0
%200 = OpLoad %v3float %_7_result
%201 = OpCompositeExtract %float %200 1
%197 = OpExtInst %float %1 FMax %199 %201
%202 = OpLoad %v3float %_7_result
%203 = OpCompositeExtract %float %202 2
%196 = OpExtInst %float %1 FMax %197 %203
OpStore %_9_maxComp %196
%205 = OpLoad %float %_8_minComp
%206 = OpFOrdLessThan %bool %205 %float_0
OpSelectionMerge %208 None
OpBranchConditional %206 %207 %208
%207 = OpLabel
%209 = OpLoad %float %_6_lum
%210 = OpLoad %float %_8_minComp
%211 = OpFOrdNotEqual %bool %209 %210
OpBranch %208
%208 = OpLabel
%212 = OpPhi %bool %false %97 %211 %207
OpSelectionMerge %214 None
OpBranchConditional %212 %213 %214
%213 = OpLabel
%216 = OpLoad %float %_6_lum
%217 = OpLoad %float %_8_minComp
%218 = OpFSub %float %216 %217
OpStore %_10_d %218
%219 = OpLoad %float %_6_lum
%220 = OpLoad %v3float %_7_result
%221 = OpLoad %float %_6_lum
%222 = OpCompositeConstruct %v3float %221 %221 %221
%223 = OpFSub %v3float %220 %222
%224 = OpLoad %float %_6_lum
%225 = OpLoad %float %_10_d
%226 = OpFDiv %float %224 %225
%227 = OpVectorTimesScalar %v3float %223 %226
%228 = OpCompositeConstruct %v3float %219 %219 %219
%229 = OpFAdd %v3float %228 %227
OpStore %_7_result %229
OpBranch %214
%214 = OpLabel
%230 = OpLoad %float %_9_maxComp
%231 = OpLoad %float %_0_alpha
%232 = OpFOrdGreaterThan %bool %230 %231
OpSelectionMerge %234 None
OpBranchConditional %232 %233 %234
%233 = OpLabel
%235 = OpLoad %float %_9_maxComp
%236 = OpLoad %float %_6_lum
%237 = OpFOrdNotEqual %bool %235 %236
OpBranch %234
%234 = OpLabel
%238 = OpPhi %bool %false %214 %237 %233
OpSelectionMerge %241 None
OpBranchConditional %238 %239 %240
%239 = OpLabel
%243 = OpLoad %v3float %_7_result
%244 = OpLoad %float %_6_lum
%245 = OpCompositeConstruct %v3float %244 %244 %244
%246 = OpFSub %v3float %243 %245
%247 = OpLoad %float %_0_alpha
%248 = OpLoad %float %_6_lum
%249 = OpFSub %float %247 %248
%250 = OpVectorTimesScalar %v3float %246 %249
OpStore %_11_n %250
%252 = OpLoad %float %_9_maxComp
%253 = OpLoad %float %_6_lum
%254 = OpFSub %float %252 %253
OpStore %_12_d %254
%255 = OpLoad %float %_6_lum
%256 = OpLoad %v3float %_11_n
%257 = OpLoad %float %_12_d
%259 = OpFDiv %float %float_1 %257
%260 = OpVectorTimesScalar %v3float %256 %259
%261 = OpCompositeConstruct %v3float %255 %255 %255
%262 = OpFAdd %v3float %261 %260
OpStore %_5_blend_set_color_luminance %262
OpBranch %241
%240 = OpLabel
%263 = OpLoad %v3float %_7_result
OpStore %_5_blend_set_color_luminance %263
OpBranch %241
%241 = OpLabel
%264 = OpLoad %v3float %_5_blend_set_color_luminance
%265 = OpLoad %v4float %dst
%266 = OpVectorShuffle %v3float %265 %265 0 1 2
%267 = OpFAdd %v3float %264 %266
%268 = OpLoad %v3float %_2_dsa
%269 = OpFSub %v3float %267 %268
%270 = OpLoad %v4float %src
%271 = OpVectorShuffle %v3float %270 %270 0 1 2
%272 = OpFAdd %v3float %269 %271
%273 = OpLoad %v3float %_1_sda
%274 = OpFSub %v3float %272 %273
%275 = OpCompositeExtract %float %274 0
%276 = OpCompositeExtract %float %274 1
%277 = OpCompositeExtract %float %274 2
%278 = OpLoad %v4float %src
%279 = OpCompositeExtract %float %278 3
%280 = OpLoad %v4float %dst
%281 = OpCompositeExtract %float %280 3
%282 = OpFAdd %float %279 %281
%283 = OpLoad %float %_0_alpha
%284 = OpFSub %float %282 %283
%285 = OpCompositeConstruct %v4float %275 %276 %277 %284
OpStore %sk_FragColor %285
OpReturn
OpFunctionEnd
