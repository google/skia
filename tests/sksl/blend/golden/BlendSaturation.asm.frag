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
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
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
%173 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
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
%236 = OpVariable %_ptr_Function_v3float Function
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
%73 = OpLoad %v3float %_2_sda
%74 = OpCompositeExtract %float %73 0
%75 = OpLoad %v3float %_2_sda
%76 = OpCompositeExtract %float %75 1
%72 = OpExtInst %float %1 FMax %74 %76
%77 = OpLoad %v3float %_2_sda
%78 = OpCompositeExtract %float %77 2
%71 = OpExtInst %float %1 FMax %72 %78
%81 = OpLoad %v3float %_2_sda
%82 = OpCompositeExtract %float %81 0
%83 = OpLoad %v3float %_2_sda
%84 = OpCompositeExtract %float %83 1
%80 = OpExtInst %float %1 FMin %82 %84
%85 = OpLoad %v3float %_2_sda
%86 = OpCompositeExtract %float %85 2
%79 = OpExtInst %float %1 FMin %80 %86
%87 = OpFSub %float %71 %79
OpStore %_5_sat %87
%88 = OpLoad %v3float %_3_dsa
%89 = OpCompositeExtract %float %88 0
%90 = OpLoad %v3float %_3_dsa
%91 = OpCompositeExtract %float %90 1
%92 = OpFOrdLessThanEqual %bool %89 %91
OpSelectionMerge %95 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%96 = OpLoad %v3float %_3_dsa
%97 = OpCompositeExtract %float %96 1
%98 = OpLoad %v3float %_3_dsa
%99 = OpCompositeExtract %float %98 2
%100 = OpFOrdLessThanEqual %bool %97 %99
OpSelectionMerge %103 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%104 = OpLoad %v3float %_3_dsa
OpStore %105 %104
%106 = OpLoad %float %_5_sat
OpStore %107 %106
%108 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %105 %107
OpStore %_4_blend_set_color_saturation %108
OpBranch %103
%102 = OpLabel
%109 = OpLoad %v3float %_3_dsa
%110 = OpCompositeExtract %float %109 0
%111 = OpLoad %v3float %_3_dsa
%112 = OpCompositeExtract %float %111 2
%113 = OpFOrdLessThanEqual %bool %110 %112
OpSelectionMerge %116 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%117 = OpLoad %v3float %_3_dsa
%118 = OpVectorShuffle %v3float %117 %117 0 2 1
OpStore %119 %118
%120 = OpLoad %float %_5_sat
OpStore %121 %120
%122 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %119 %121
%123 = OpVectorShuffle %v3float %122 %122 0 2 1
OpStore %_4_blend_set_color_saturation %123
OpBranch %116
%115 = OpLabel
%124 = OpLoad %v3float %_3_dsa
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
%131 = OpLoad %v3float %_3_dsa
%132 = OpCompositeExtract %float %131 0
%133 = OpLoad %v3float %_3_dsa
%134 = OpCompositeExtract %float %133 2
%135 = OpFOrdLessThanEqual %bool %132 %134
OpSelectionMerge %138 None
OpBranchConditional %135 %136 %137
%136 = OpLabel
%139 = OpLoad %v3float %_3_dsa
%140 = OpVectorShuffle %v3float %139 %139 1 0 2
OpStore %141 %140
%142 = OpLoad %float %_5_sat
OpStore %143 %142
%144 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %141 %143
%145 = OpVectorShuffle %v3float %144 %144 1 0 2
OpStore %_4_blend_set_color_saturation %145
OpBranch %138
%137 = OpLabel
%146 = OpLoad %v3float %_3_dsa
%147 = OpCompositeExtract %float %146 1
%148 = OpLoad %v3float %_3_dsa
%149 = OpCompositeExtract %float %148 2
%150 = OpFOrdLessThanEqual %bool %147 %149
OpSelectionMerge %153 None
OpBranchConditional %150 %151 %152
%151 = OpLabel
%154 = OpLoad %v3float %_3_dsa
%155 = OpVectorShuffle %v3float %154 %154 1 2 0
OpStore %156 %155
%157 = OpLoad %float %_5_sat
OpStore %158 %157
%159 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %156 %158
%160 = OpVectorShuffle %v3float %159 %159 2 0 1
OpStore %_4_blend_set_color_saturation %160
OpBranch %153
%152 = OpLabel
%161 = OpLoad %v3float %_3_dsa
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
%169 = OpDot %float %173 %174
OpStore %_7_lum %169
%176 = OpLoad %float %_7_lum
%178 = OpLoad %v3float %_4_blend_set_color_saturation
%177 = OpDot %float %173 %178
%179 = OpFSub %float %176 %177
%180 = OpLoad %v3float %_4_blend_set_color_saturation
%181 = OpCompositeConstruct %v3float %179 %179 %179
%182 = OpFAdd %v3float %181 %180
OpStore %_8_result %182
%186 = OpLoad %v3float %_8_result
%187 = OpCompositeExtract %float %186 0
%188 = OpLoad %v3float %_8_result
%189 = OpCompositeExtract %float %188 1
%185 = OpExtInst %float %1 FMin %187 %189
%190 = OpLoad %v3float %_8_result
%191 = OpCompositeExtract %float %190 2
%184 = OpExtInst %float %1 FMin %185 %191
OpStore %_9_minComp %184
%195 = OpLoad %v3float %_8_result
%196 = OpCompositeExtract %float %195 0
%197 = OpLoad %v3float %_8_result
%198 = OpCompositeExtract %float %197 1
%194 = OpExtInst %float %1 FMax %196 %198
%199 = OpLoad %v3float %_8_result
%200 = OpCompositeExtract %float %199 2
%193 = OpExtInst %float %1 FMax %194 %200
OpStore %_10_maxComp %193
%202 = OpLoad %float %_9_minComp
%203 = OpFOrdLessThan %bool %202 %float_0
OpSelectionMerge %205 None
OpBranchConditional %203 %204 %205
%204 = OpLabel
%206 = OpLoad %float %_7_lum
%207 = OpLoad %float %_9_minComp
%208 = OpFOrdNotEqual %bool %206 %207
OpBranch %205
%205 = OpLabel
%209 = OpPhi %bool %false %95 %208 %204
OpSelectionMerge %211 None
OpBranchConditional %209 %210 %211
%210 = OpLabel
%212 = OpLoad %float %_7_lum
%213 = OpLoad %v3float %_8_result
%214 = OpLoad %float %_7_lum
%215 = OpCompositeConstruct %v3float %214 %214 %214
%216 = OpFSub %v3float %213 %215
%217 = OpLoad %float %_7_lum
%218 = OpVectorTimesScalar %v3float %216 %217
%219 = OpLoad %float %_7_lum
%220 = OpLoad %float %_9_minComp
%221 = OpFSub %float %219 %220
%223 = OpFDiv %float %float_1 %221
%224 = OpVectorTimesScalar %v3float %218 %223
%225 = OpCompositeConstruct %v3float %212 %212 %212
%226 = OpFAdd %v3float %225 %224
OpStore %_8_result %226
OpBranch %211
%211 = OpLabel
%227 = OpLoad %float %_10_maxComp
%228 = OpLoad %float %_1_alpha
%229 = OpFOrdGreaterThan %bool %227 %228
OpSelectionMerge %231 None
OpBranchConditional %229 %230 %231
%230 = OpLabel
%232 = OpLoad %float %_10_maxComp
%233 = OpLoad %float %_7_lum
%234 = OpFOrdNotEqual %bool %232 %233
OpBranch %231
%231 = OpLabel
%235 = OpPhi %bool %false %211 %234 %230
OpSelectionMerge %239 None
OpBranchConditional %235 %237 %238
%237 = OpLabel
%240 = OpLoad %float %_7_lum
%241 = OpLoad %v3float %_8_result
%242 = OpLoad %float %_7_lum
%243 = OpCompositeConstruct %v3float %242 %242 %242
%244 = OpFSub %v3float %241 %243
%245 = OpLoad %float %_1_alpha
%246 = OpLoad %float %_7_lum
%247 = OpFSub %float %245 %246
%248 = OpVectorTimesScalar %v3float %244 %247
%249 = OpLoad %float %_10_maxComp
%250 = OpLoad %float %_7_lum
%251 = OpFSub %float %249 %250
%252 = OpFDiv %float %float_1 %251
%253 = OpVectorTimesScalar %v3float %248 %252
%254 = OpCompositeConstruct %v3float %240 %240 %240
%255 = OpFAdd %v3float %254 %253
OpStore %236 %255
OpBranch %239
%238 = OpLabel
%256 = OpLoad %v3float %_8_result
OpStore %236 %256
OpBranch %239
%239 = OpLabel
%257 = OpLoad %v3float %236
%258 = OpLoad %v4float %dst
%259 = OpVectorShuffle %v3float %258 %258 0 1 2
%260 = OpFAdd %v3float %257 %259
%261 = OpLoad %v3float %_3_dsa
%262 = OpFSub %v3float %260 %261
%263 = OpLoad %v4float %src
%264 = OpVectorShuffle %v3float %263 %263 0 1 2
%265 = OpFAdd %v3float %262 %264
%266 = OpLoad %v3float %_2_sda
%267 = OpFSub %v3float %265 %266
%268 = OpCompositeExtract %float %267 0
%269 = OpCompositeExtract %float %267 1
%270 = OpCompositeExtract %float %267 2
%271 = OpLoad %v4float %src
%272 = OpCompositeExtract %float %271 3
%273 = OpLoad %v4float %dst
%274 = OpCompositeExtract %float %273 3
%275 = OpFAdd %float %272 %274
%276 = OpLoad %float %_1_alpha
%277 = OpFSub %float %275 %276
%278 = OpCompositeConstruct %v4float %268 %269 %270 %277
OpStore %sk_FragColor %278
OpReturn
OpFunctionEnd
