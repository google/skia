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
OpName %_19_n "_19_n"
OpName %_20_d "_20_d"
OpName %main "main"
OpName %_0_blend_hue "_0_blend_hue"
OpName %_1_alpha "_1_alpha"
OpName %_2_sda "_2_sda"
OpName %_3_dsa "_3_dsa"
OpName %_4_blend_set_color_saturation "_4_blend_set_color_saturation"
OpName %_5_sat "_5_sat"
OpName %_6_blend_set_color_luminance "_6_blend_set_color_luminance"
OpName %_7_lum "_7_lum"
OpName %_8_result "_8_result"
OpName %_9_minComp "_9_minComp"
OpName %_10_maxComp "_10_maxComp"
OpName %_11_d "_11_d"
OpName %_12_n "_12_n"
OpName %_13_d "_13_d"
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
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%178 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%_blend_set_color_saturation_helper = OpFunction %v3float None %15
%18 = OpFunctionParameter %_ptr_Function_v3float
%19 = OpFunctionParameter %_ptr_Function_float
%20 = OpLabel
%_19_n = OpVariable %_ptr_Function_float Function
%_20_d = OpVariable %_ptr_Function_float Function
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
OpStore %_19_n %36
%38 = OpLoad %v3float %18
%39 = OpCompositeExtract %float %38 2
%40 = OpLoad %v3float %18
%41 = OpCompositeExtract %float %40 0
%42 = OpFSub %float %39 %41
OpStore %_20_d %42
%44 = OpLoad %float %_19_n
%45 = OpLoad %float %_20_d
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
%_0_blend_hue = OpVariable %_ptr_Function_v4float Function
%_1_alpha = OpVariable %_ptr_Function_float Function
%_2_sda = OpVariable %_ptr_Function_v3float Function
%_3_dsa = OpVariable %_ptr_Function_v3float Function
%_4_blend_set_color_saturation = OpVariable %_ptr_Function_v3float Function
%_5_sat = OpVariable %_ptr_Function_float Function
%109 = OpVariable %_ptr_Function_v3float Function
%111 = OpVariable %_ptr_Function_float Function
%123 = OpVariable %_ptr_Function_v3float Function
%125 = OpVariable %_ptr_Function_float Function
%130 = OpVariable %_ptr_Function_v3float Function
%132 = OpVariable %_ptr_Function_float Function
%145 = OpVariable %_ptr_Function_v3float Function
%147 = OpVariable %_ptr_Function_float Function
%160 = OpVariable %_ptr_Function_v3float Function
%162 = OpVariable %_ptr_Function_float Function
%167 = OpVariable %_ptr_Function_v3float Function
%169 = OpVariable %_ptr_Function_float Function
%_6_blend_set_color_luminance = OpVariable %_ptr_Function_v3float Function
%_7_lum = OpVariable %_ptr_Function_float Function
%_8_result = OpVariable %_ptr_Function_v3float Function
%_9_minComp = OpVariable %_ptr_Function_float Function
%_10_maxComp = OpVariable %_ptr_Function_float Function
%_11_d = OpVariable %_ptr_Function_float Function
%_12_n = OpVariable %_ptr_Function_v3float Function
%_13_d = OpVariable %_ptr_Function_float Function
%56 = OpLoad %v4float %dst
%57 = OpCompositeExtract %float %56 3
%58 = OpLoad %v4float %src
%59 = OpCompositeExtract %float %58 3
%60 = OpFMul %float %57 %59
OpStore %_1_alpha %60
%62 = OpLoad %v4float %src
%63 = OpVectorShuffle %v3float %62 %62 0 1 2
%64 = OpLoad %v4float %dst
%65 = OpCompositeExtract %float %64 3
%66 = OpVectorTimesScalar %v3float %63 %65
OpStore %_2_sda %66
%68 = OpLoad %v4float %dst
%69 = OpVectorShuffle %v3float %68 %68 0 1 2
%70 = OpLoad %v4float %src
%71 = OpCompositeExtract %float %70 3
%72 = OpVectorTimesScalar %v3float %69 %71
OpStore %_3_dsa %72
%77 = OpLoad %v3float %_3_dsa
%78 = OpCompositeExtract %float %77 0
%79 = OpLoad %v3float %_3_dsa
%80 = OpCompositeExtract %float %79 1
%76 = OpExtInst %float %1 FMax %78 %80
%81 = OpLoad %v3float %_3_dsa
%82 = OpCompositeExtract %float %81 2
%75 = OpExtInst %float %1 FMax %76 %82
%85 = OpLoad %v3float %_3_dsa
%86 = OpCompositeExtract %float %85 0
%87 = OpLoad %v3float %_3_dsa
%88 = OpCompositeExtract %float %87 1
%84 = OpExtInst %float %1 FMin %86 %88
%89 = OpLoad %v3float %_3_dsa
%90 = OpCompositeExtract %float %89 2
%83 = OpExtInst %float %1 FMin %84 %90
%91 = OpFSub %float %75 %83
OpStore %_5_sat %91
%92 = OpLoad %v3float %_2_sda
%93 = OpCompositeExtract %float %92 0
%94 = OpLoad %v3float %_2_sda
%95 = OpCompositeExtract %float %94 1
%96 = OpFOrdLessThanEqual %bool %93 %95
OpSelectionMerge %99 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%100 = OpLoad %v3float %_2_sda
%101 = OpCompositeExtract %float %100 1
%102 = OpLoad %v3float %_2_sda
%103 = OpCompositeExtract %float %102 2
%104 = OpFOrdLessThanEqual %bool %101 %103
OpSelectionMerge %107 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
%108 = OpLoad %v3float %_2_sda
OpStore %109 %108
%110 = OpLoad %float %_5_sat
OpStore %111 %110
%112 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %109 %111
OpStore %_4_blend_set_color_saturation %112
OpBranch %107
%106 = OpLabel
%113 = OpLoad %v3float %_2_sda
%114 = OpCompositeExtract %float %113 0
%115 = OpLoad %v3float %_2_sda
%116 = OpCompositeExtract %float %115 2
%117 = OpFOrdLessThanEqual %bool %114 %116
OpSelectionMerge %120 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%121 = OpLoad %v3float %_2_sda
%122 = OpVectorShuffle %v3float %121 %121 0 2 1
OpStore %123 %122
%124 = OpLoad %float %_5_sat
OpStore %125 %124
%126 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %123 %125
%127 = OpVectorShuffle %v3float %126 %126 0 2 1
OpStore %_4_blend_set_color_saturation %127
OpBranch %120
%119 = OpLabel
%128 = OpLoad %v3float %_2_sda
%129 = OpVectorShuffle %v3float %128 %128 2 0 1
OpStore %130 %129
%131 = OpLoad %float %_5_sat
OpStore %132 %131
%133 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %130 %132
%134 = OpVectorShuffle %v3float %133 %133 1 2 0
OpStore %_4_blend_set_color_saturation %134
OpBranch %120
%120 = OpLabel
OpBranch %107
%107 = OpLabel
OpBranch %99
%98 = OpLabel
%135 = OpLoad %v3float %_2_sda
%136 = OpCompositeExtract %float %135 0
%137 = OpLoad %v3float %_2_sda
%138 = OpCompositeExtract %float %137 2
%139 = OpFOrdLessThanEqual %bool %136 %138
OpSelectionMerge %142 None
OpBranchConditional %139 %140 %141
%140 = OpLabel
%143 = OpLoad %v3float %_2_sda
%144 = OpVectorShuffle %v3float %143 %143 1 0 2
OpStore %145 %144
%146 = OpLoad %float %_5_sat
OpStore %147 %146
%148 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %145 %147
%149 = OpVectorShuffle %v3float %148 %148 1 0 2
OpStore %_4_blend_set_color_saturation %149
OpBranch %142
%141 = OpLabel
%150 = OpLoad %v3float %_2_sda
%151 = OpCompositeExtract %float %150 1
%152 = OpLoad %v3float %_2_sda
%153 = OpCompositeExtract %float %152 2
%154 = OpFOrdLessThanEqual %bool %151 %153
OpSelectionMerge %157 None
OpBranchConditional %154 %155 %156
%155 = OpLabel
%158 = OpLoad %v3float %_2_sda
%159 = OpVectorShuffle %v3float %158 %158 1 2 0
OpStore %160 %159
%161 = OpLoad %float %_5_sat
OpStore %162 %161
%163 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %160 %162
%164 = OpVectorShuffle %v3float %163 %163 2 0 1
OpStore %_4_blend_set_color_saturation %164
OpBranch %157
%156 = OpLabel
%165 = OpLoad %v3float %_2_sda
%166 = OpVectorShuffle %v3float %165 %165 2 1 0
OpStore %167 %166
%168 = OpLoad %float %_5_sat
OpStore %169 %168
%170 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %167 %169
%171 = OpVectorShuffle %v3float %170 %170 2 1 0
OpStore %_4_blend_set_color_saturation %171
OpBranch %157
%157 = OpLabel
OpBranch %142
%142 = OpLabel
OpBranch %99
%99 = OpLabel
%179 = OpLoad %v3float %_3_dsa
%174 = OpDot %float %178 %179
OpStore %_7_lum %174
%181 = OpLoad %float %_7_lum
%183 = OpLoad %v3float %_4_blend_set_color_saturation
%182 = OpDot %float %178 %183
%184 = OpFSub %float %181 %182
%185 = OpLoad %v3float %_4_blend_set_color_saturation
%186 = OpCompositeConstruct %v3float %184 %184 %184
%187 = OpFAdd %v3float %186 %185
OpStore %_8_result %187
%191 = OpLoad %v3float %_8_result
%192 = OpCompositeExtract %float %191 0
%193 = OpLoad %v3float %_8_result
%194 = OpCompositeExtract %float %193 1
%190 = OpExtInst %float %1 FMin %192 %194
%195 = OpLoad %v3float %_8_result
%196 = OpCompositeExtract %float %195 2
%189 = OpExtInst %float %1 FMin %190 %196
OpStore %_9_minComp %189
%200 = OpLoad %v3float %_8_result
%201 = OpCompositeExtract %float %200 0
%202 = OpLoad %v3float %_8_result
%203 = OpCompositeExtract %float %202 1
%199 = OpExtInst %float %1 FMax %201 %203
%204 = OpLoad %v3float %_8_result
%205 = OpCompositeExtract %float %204 2
%198 = OpExtInst %float %1 FMax %199 %205
OpStore %_10_maxComp %198
%207 = OpLoad %float %_9_minComp
%208 = OpFOrdLessThan %bool %207 %float_0
OpSelectionMerge %210 None
OpBranchConditional %208 %209 %210
%209 = OpLabel
%211 = OpLoad %float %_7_lum
%212 = OpLoad %float %_9_minComp
%213 = OpFOrdNotEqual %bool %211 %212
OpBranch %210
%210 = OpLabel
%214 = OpPhi %bool %false %99 %213 %209
OpSelectionMerge %216 None
OpBranchConditional %214 %215 %216
%215 = OpLabel
%218 = OpLoad %float %_7_lum
%219 = OpLoad %float %_9_minComp
%220 = OpFSub %float %218 %219
OpStore %_11_d %220
%221 = OpLoad %float %_7_lum
%222 = OpLoad %v3float %_8_result
%223 = OpLoad %float %_7_lum
%224 = OpCompositeConstruct %v3float %223 %223 %223
%225 = OpFSub %v3float %222 %224
%226 = OpLoad %float %_7_lum
%227 = OpLoad %float %_11_d
%228 = OpFDiv %float %226 %227
%229 = OpVectorTimesScalar %v3float %225 %228
%230 = OpCompositeConstruct %v3float %221 %221 %221
%231 = OpFAdd %v3float %230 %229
OpStore %_8_result %231
OpBranch %216
%216 = OpLabel
%232 = OpLoad %float %_10_maxComp
%233 = OpLoad %float %_1_alpha
%234 = OpFOrdGreaterThan %bool %232 %233
OpSelectionMerge %236 None
OpBranchConditional %234 %235 %236
%235 = OpLabel
%237 = OpLoad %float %_10_maxComp
%238 = OpLoad %float %_7_lum
%239 = OpFOrdNotEqual %bool %237 %238
OpBranch %236
%236 = OpLabel
%240 = OpPhi %bool %false %216 %239 %235
OpSelectionMerge %243 None
OpBranchConditional %240 %241 %242
%241 = OpLabel
%245 = OpLoad %v3float %_8_result
%246 = OpLoad %float %_7_lum
%247 = OpCompositeConstruct %v3float %246 %246 %246
%248 = OpFSub %v3float %245 %247
%249 = OpLoad %float %_1_alpha
%250 = OpLoad %float %_7_lum
%251 = OpFSub %float %249 %250
%252 = OpVectorTimesScalar %v3float %248 %251
OpStore %_12_n %252
%254 = OpLoad %float %_10_maxComp
%255 = OpLoad %float %_7_lum
%256 = OpFSub %float %254 %255
OpStore %_13_d %256
%257 = OpLoad %float %_7_lum
%258 = OpLoad %v3float %_12_n
%259 = OpLoad %float %_13_d
%261 = OpFDiv %float %float_1 %259
%262 = OpVectorTimesScalar %v3float %258 %261
%263 = OpCompositeConstruct %v3float %257 %257 %257
%264 = OpFAdd %v3float %263 %262
OpStore %_6_blend_set_color_luminance %264
OpBranch %243
%242 = OpLabel
%265 = OpLoad %v3float %_8_result
OpStore %_6_blend_set_color_luminance %265
OpBranch %243
%243 = OpLabel
%266 = OpLoad %v3float %_6_blend_set_color_luminance
%267 = OpLoad %v4float %dst
%268 = OpVectorShuffle %v3float %267 %267 0 1 2
%269 = OpFAdd %v3float %266 %268
%270 = OpLoad %v3float %_3_dsa
%271 = OpFSub %v3float %269 %270
%272 = OpLoad %v4float %src
%273 = OpVectorShuffle %v3float %272 %272 0 1 2
%274 = OpFAdd %v3float %271 %273
%275 = OpLoad %v3float %_2_sda
%276 = OpFSub %v3float %274 %275
%277 = OpCompositeExtract %float %276 0
%278 = OpCompositeExtract %float %276 1
%279 = OpCompositeExtract %float %276 2
%280 = OpLoad %v4float %src
%281 = OpCompositeExtract %float %280 3
%282 = OpLoad %v4float %dst
%283 = OpCompositeExtract %float %282 3
%284 = OpFAdd %float %281 %283
%285 = OpLoad %float %_1_alpha
%286 = OpFSub %float %284 %285
%287 = OpCompositeConstruct %v4float %277 %278 %279 %286
OpStore %sk_FragColor %287
OpReturn
OpFunctionEnd
