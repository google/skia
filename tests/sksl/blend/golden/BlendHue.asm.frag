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
OpName %main "main"
OpName %_1_alpha "_1_alpha"
OpName %_2_sda "_2_sda"
OpName %_3_dsa "_3_dsa"
OpName %_4_blend_set_color_saturation "_4_blend_set_color_saturation"
OpName %_5_sat "_5_sat"
OpName %_7_lum "_7_lum"
OpName %_8_result "_8_result"
OpName %_9_minComp "_9_minComp"
OpName %_10_maxComp "_10_maxComp"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
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
%46 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%void = OpTypeVoid
%49 = OpTypeFunction %void
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%170 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%178 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%_blend_set_color_saturation_helper = OpFunction %v3float None %15
%18 = OpFunctionParameter %_ptr_Function_v3float
%19 = OpFunctionParameter %_ptr_Function_float
%20 = OpLabel
%26 = OpVariable %_ptr_Function_v3float Function
%21 = OpLoad %v3float %18
%22 = OpCompositeExtract %float %21 0
%23 = OpLoad %v3float %18
%24 = OpCompositeExtract %float %23 2
%25 = OpFOrdLessThan %bool %22 %24
OpSelectionMerge %29 None
OpBranchConditional %25 %27 %28
%27 = OpLabel
%31 = OpLoad %float %19
%32 = OpLoad %v3float %18
%33 = OpCompositeExtract %float %32 1
%34 = OpLoad %v3float %18
%35 = OpCompositeExtract %float %34 0
%36 = OpFSub %float %33 %35
%37 = OpFMul %float %31 %36
%38 = OpLoad %v3float %18
%39 = OpCompositeExtract %float %38 2
%40 = OpLoad %v3float %18
%41 = OpCompositeExtract %float %40 0
%42 = OpFSub %float %39 %41
%43 = OpFDiv %float %37 %42
%44 = OpLoad %float %19
%45 = OpCompositeConstruct %v3float %float_0 %43 %44
OpStore %26 %45
OpBranch %29
%28 = OpLabel
OpStore %26 %46
OpBranch %29
%29 = OpLabel
%47 = OpLoad %v3float %26
OpReturnValue %47
OpFunctionEnd
%main = OpFunction %void None %49
%50 = OpLabel
%_1_alpha = OpVariable %_ptr_Function_float Function
%_2_sda = OpVariable %_ptr_Function_v3float Function
%_3_dsa = OpVariable %_ptr_Function_v3float Function
%_4_blend_set_color_saturation = OpVariable %_ptr_Function_v3float Function
%_5_sat = OpVariable %_ptr_Function_float Function
%105 = OpVariable %_ptr_Function_v3float Function
%107 = OpVariable %_ptr_Function_float Function
%119 = OpVariable %_ptr_Function_v3float Function
%121 = OpVariable %_ptr_Function_float Function
%126 = OpVariable %_ptr_Function_v3float Function
%128 = OpVariable %_ptr_Function_float Function
%141 = OpVariable %_ptr_Function_v3float Function
%143 = OpVariable %_ptr_Function_float Function
%156 = OpVariable %_ptr_Function_v3float Function
%158 = OpVariable %_ptr_Function_float Function
%163 = OpVariable %_ptr_Function_v3float Function
%165 = OpVariable %_ptr_Function_float Function
%_7_lum = OpVariable %_ptr_Function_float Function
%_8_result = OpVariable %_ptr_Function_v3float Function
%_9_minComp = OpVariable %_ptr_Function_float Function
%_10_maxComp = OpVariable %_ptr_Function_float Function
%237 = OpVariable %_ptr_Function_v3float Function
%52 = OpLoad %v4float %dst
%53 = OpCompositeExtract %float %52 3
%54 = OpLoad %v4float %src
%55 = OpCompositeExtract %float %54 3
%56 = OpFMul %float %53 %55
OpStore %_1_alpha %56
%58 = OpLoad %v4float %src
%59 = OpVectorShuffle %v3float %58 %58 0 1 2
%60 = OpLoad %v4float %dst
%61 = OpCompositeExtract %float %60 3
%62 = OpVectorTimesScalar %v3float %59 %61
OpStore %_2_sda %62
%64 = OpLoad %v4float %dst
%65 = OpVectorShuffle %v3float %64 %64 0 1 2
%66 = OpLoad %v4float %src
%67 = OpCompositeExtract %float %66 3
%68 = OpVectorTimesScalar %v3float %65 %67
OpStore %_3_dsa %68
%73 = OpLoad %v3float %_3_dsa
%74 = OpCompositeExtract %float %73 0
%75 = OpLoad %v3float %_3_dsa
%76 = OpCompositeExtract %float %75 1
%72 = OpExtInst %float %1 FMax %74 %76
%77 = OpLoad %v3float %_3_dsa
%78 = OpCompositeExtract %float %77 2
%71 = OpExtInst %float %1 FMax %72 %78
%81 = OpLoad %v3float %_3_dsa
%82 = OpCompositeExtract %float %81 0
%83 = OpLoad %v3float %_3_dsa
%84 = OpCompositeExtract %float %83 1
%80 = OpExtInst %float %1 FMin %82 %84
%85 = OpLoad %v3float %_3_dsa
%86 = OpCompositeExtract %float %85 2
%79 = OpExtInst %float %1 FMin %80 %86
%87 = OpFSub %float %71 %79
OpStore %_5_sat %87
%88 = OpLoad %v3float %_2_sda
%89 = OpCompositeExtract %float %88 0
%90 = OpLoad %v3float %_2_sda
%91 = OpCompositeExtract %float %90 1
%92 = OpFOrdLessThanEqual %bool %89 %91
OpSelectionMerge %95 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%96 = OpLoad %v3float %_2_sda
%97 = OpCompositeExtract %float %96 1
%98 = OpLoad %v3float %_2_sda
%99 = OpCompositeExtract %float %98 2
%100 = OpFOrdLessThanEqual %bool %97 %99
OpSelectionMerge %103 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%104 = OpLoad %v3float %_2_sda
OpStore %105 %104
%106 = OpLoad %float %_5_sat
OpStore %107 %106
%108 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %105 %107
OpStore %_4_blend_set_color_saturation %108
OpBranch %103
%102 = OpLabel
%109 = OpLoad %v3float %_2_sda
%110 = OpCompositeExtract %float %109 0
%111 = OpLoad %v3float %_2_sda
%112 = OpCompositeExtract %float %111 2
%113 = OpFOrdLessThanEqual %bool %110 %112
OpSelectionMerge %116 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%117 = OpLoad %v3float %_2_sda
%118 = OpVectorShuffle %v3float %117 %117 0 2 1
OpStore %119 %118
%120 = OpLoad %float %_5_sat
OpStore %121 %120
%122 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %119 %121
%123 = OpVectorShuffle %v3float %122 %122 0 2 1
OpStore %_4_blend_set_color_saturation %123
OpBranch %116
%115 = OpLabel
%124 = OpLoad %v3float %_2_sda
%125 = OpVectorShuffle %v3float %124 %124 2 0 1
OpStore %126 %125
%127 = OpLoad %float %_5_sat
OpStore %128 %127
%129 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %126 %128
%130 = OpVectorShuffle %v3float %129 %129 1 2 0
OpStore %_4_blend_set_color_saturation %130
OpBranch %116
%116 = OpLabel
OpBranch %103
%103 = OpLabel
OpBranch %95
%94 = OpLabel
%131 = OpLoad %v3float %_2_sda
%132 = OpCompositeExtract %float %131 0
%133 = OpLoad %v3float %_2_sda
%134 = OpCompositeExtract %float %133 2
%135 = OpFOrdLessThanEqual %bool %132 %134
OpSelectionMerge %138 None
OpBranchConditional %135 %136 %137
%136 = OpLabel
%139 = OpLoad %v3float %_2_sda
%140 = OpVectorShuffle %v3float %139 %139 1 0 2
OpStore %141 %140
%142 = OpLoad %float %_5_sat
OpStore %143 %142
%144 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %141 %143
%145 = OpVectorShuffle %v3float %144 %144 1 0 2
OpStore %_4_blend_set_color_saturation %145
OpBranch %138
%137 = OpLabel
%146 = OpLoad %v3float %_2_sda
%147 = OpCompositeExtract %float %146 1
%148 = OpLoad %v3float %_2_sda
%149 = OpCompositeExtract %float %148 2
%150 = OpFOrdLessThanEqual %bool %147 %149
OpSelectionMerge %153 None
OpBranchConditional %150 %151 %152
%151 = OpLabel
%154 = OpLoad %v3float %_2_sda
%155 = OpVectorShuffle %v3float %154 %154 1 2 0
OpStore %156 %155
%157 = OpLoad %float %_5_sat
OpStore %158 %157
%159 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %156 %158
%160 = OpVectorShuffle %v3float %159 %159 2 0 1
OpStore %_4_blend_set_color_saturation %160
OpBranch %153
%152 = OpLabel
%161 = OpLoad %v3float %_2_sda
%162 = OpVectorShuffle %v3float %161 %161 2 1 0
OpStore %163 %162
%164 = OpLoad %float %_5_sat
OpStore %165 %164
%166 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %163 %165
%167 = OpVectorShuffle %v3float %166 %166 2 1 0
OpStore %_4_blend_set_color_saturation %167
OpBranch %153
%153 = OpLabel
OpBranch %138
%138 = OpLabel
OpBranch %95
%95 = OpLabel
%174 = OpLoad %v3float %_3_dsa
%169 = OpDot %float %170 %174
OpStore %_7_lum %169
%176 = OpLoad %float %_7_lum
%179 = OpLoad %v3float %_4_blend_set_color_saturation
%177 = OpDot %float %178 %179
%180 = OpFSub %float %176 %177
%181 = OpLoad %v3float %_4_blend_set_color_saturation
%182 = OpCompositeConstruct %v3float %180 %180 %180
%183 = OpFAdd %v3float %182 %181
OpStore %_8_result %183
%187 = OpLoad %v3float %_8_result
%188 = OpCompositeExtract %float %187 0
%189 = OpLoad %v3float %_8_result
%190 = OpCompositeExtract %float %189 1
%186 = OpExtInst %float %1 FMin %188 %190
%191 = OpLoad %v3float %_8_result
%192 = OpCompositeExtract %float %191 2
%185 = OpExtInst %float %1 FMin %186 %192
OpStore %_9_minComp %185
%196 = OpLoad %v3float %_8_result
%197 = OpCompositeExtract %float %196 0
%198 = OpLoad %v3float %_8_result
%199 = OpCompositeExtract %float %198 1
%195 = OpExtInst %float %1 FMax %197 %199
%200 = OpLoad %v3float %_8_result
%201 = OpCompositeExtract %float %200 2
%194 = OpExtInst %float %1 FMax %195 %201
OpStore %_10_maxComp %194
%203 = OpLoad %float %_9_minComp
%204 = OpFOrdLessThan %bool %203 %float_0
OpSelectionMerge %206 None
OpBranchConditional %204 %205 %206
%205 = OpLabel
%207 = OpLoad %float %_7_lum
%208 = OpLoad %float %_9_minComp
%209 = OpFOrdNotEqual %bool %207 %208
OpBranch %206
%206 = OpLabel
%210 = OpPhi %bool %false %95 %209 %205
OpSelectionMerge %212 None
OpBranchConditional %210 %211 %212
%211 = OpLabel
%213 = OpLoad %float %_7_lum
%214 = OpLoad %v3float %_8_result
%215 = OpLoad %float %_7_lum
%216 = OpCompositeConstruct %v3float %215 %215 %215
%217 = OpFSub %v3float %214 %216
%218 = OpLoad %float %_7_lum
%219 = OpVectorTimesScalar %v3float %217 %218
%220 = OpLoad %float %_7_lum
%221 = OpLoad %float %_9_minComp
%222 = OpFSub %float %220 %221
%224 = OpFDiv %float %float_1 %222
%225 = OpVectorTimesScalar %v3float %219 %224
%226 = OpCompositeConstruct %v3float %213 %213 %213
%227 = OpFAdd %v3float %226 %225
OpStore %_8_result %227
OpBranch %212
%212 = OpLabel
%228 = OpLoad %float %_10_maxComp
%229 = OpLoad %float %_1_alpha
%230 = OpFOrdGreaterThan %bool %228 %229
OpSelectionMerge %232 None
OpBranchConditional %230 %231 %232
%231 = OpLabel
%233 = OpLoad %float %_10_maxComp
%234 = OpLoad %float %_7_lum
%235 = OpFOrdNotEqual %bool %233 %234
OpBranch %232
%232 = OpLabel
%236 = OpPhi %bool %false %212 %235 %231
OpSelectionMerge %240 None
OpBranchConditional %236 %238 %239
%238 = OpLabel
%241 = OpLoad %float %_7_lum
%242 = OpLoad %v3float %_8_result
%243 = OpLoad %float %_7_lum
%244 = OpCompositeConstruct %v3float %243 %243 %243
%245 = OpFSub %v3float %242 %244
%246 = OpLoad %float %_1_alpha
%247 = OpLoad %float %_7_lum
%248 = OpFSub %float %246 %247
%249 = OpVectorTimesScalar %v3float %245 %248
%250 = OpLoad %float %_10_maxComp
%251 = OpLoad %float %_7_lum
%252 = OpFSub %float %250 %251
%253 = OpFDiv %float %float_1 %252
%254 = OpVectorTimesScalar %v3float %249 %253
%255 = OpCompositeConstruct %v3float %241 %241 %241
%256 = OpFAdd %v3float %255 %254
OpStore %237 %256
OpBranch %240
%239 = OpLabel
%257 = OpLoad %v3float %_8_result
OpStore %237 %257
OpBranch %240
%240 = OpLabel
%258 = OpLoad %v3float %237
%259 = OpLoad %v4float %dst
%260 = OpVectorShuffle %v3float %259 %259 0 1 2
%261 = OpFAdd %v3float %258 %260
%262 = OpLoad %v3float %_3_dsa
%263 = OpFSub %v3float %261 %262
%264 = OpLoad %v4float %src
%265 = OpVectorShuffle %v3float %264 %264 0 1 2
%266 = OpFAdd %v3float %263 %265
%267 = OpLoad %v3float %_2_sda
%268 = OpFSub %v3float %266 %267
%269 = OpCompositeExtract %float %268 0
%270 = OpCompositeExtract %float %268 1
%271 = OpCompositeExtract %float %268 2
%272 = OpLoad %v4float %src
%273 = OpCompositeExtract %float %272 3
%274 = OpLoad %v4float %dst
%275 = OpCompositeExtract %float %274 3
%276 = OpFAdd %float %273 %275
%277 = OpLoad %float %_1_alpha
%278 = OpFSub %float %276 %277
%279 = OpCompositeConstruct %v4float %269 %270 %271 %278
OpStore %sk_FragColor %279
OpReturn
OpFunctionEnd
