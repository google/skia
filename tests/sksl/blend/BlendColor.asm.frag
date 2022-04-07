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
OpName %blend_hslc_h4h4h4bb "blend_hslc_h4h4h4bb"
OpName %alpha "alpha"
OpName %sda "sda"
OpName %dsa "dsa"
OpName %l "l"
OpName %r "r"
OpName %_2_hueLumColor "_2_hueLumColor"
OpName %_3_midPt "_3_midPt"
OpName %_4_satPt "_4_satPt"
OpName %_5_lum "_5_lum"
OpName %_6_result "_6_result"
OpName %_7_minComp "_7_minComp"
OpName %_8_maxComp "_8_maxComp"
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
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %alpha RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %sda RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %dsa RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %l RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %r RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %_2_hueLumColor RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %_3_midPt RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %_4_satPt RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %_5_lum RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %_6_result RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %_7_minComp RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %_8_maxComp RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%14 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float %_ptr_Function_bool %_ptr_Function_bool
%_ptr_Function_float = OpTypePointer Function %float
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%float_0 = OpConstant %float 0
%68 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%float_1 = OpConstant %float 1
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%196 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%void = OpTypeVoid
%298 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%blend_hslc_h4h4h4bb = OpFunction %v4float None %14
%17 = OpFunctionParameter %_ptr_Function_v4float
%18 = OpFunctionParameter %_ptr_Function_v4float
%19 = OpFunctionParameter %_ptr_Function_bool
%20 = OpFunctionParameter %_ptr_Function_bool
%21 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%l = OpVariable %_ptr_Function_v3float Function
%45 = OpVariable %_ptr_Function_v3float Function
%r = OpVariable %_ptr_Function_v3float Function
%54 = OpVariable %_ptr_Function_v3float Function
%_2_hueLumColor = OpVariable %_ptr_Function_v3float Function
%_3_midPt = OpVariable %_ptr_Function_v3float Function
%_4_satPt = OpVariable %_ptr_Function_v3float Function
%_5_lum = OpVariable %_ptr_Function_float Function
%_6_result = OpVariable %_ptr_Function_v3float Function
%_7_minComp = OpVariable %_ptr_Function_float Function
%_8_maxComp = OpVariable %_ptr_Function_float Function
%24 = OpLoad %v4float %18
%25 = OpCompositeExtract %float %24 3
%26 = OpLoad %v4float %17
%27 = OpCompositeExtract %float %26 3
%28 = OpFMul %float %25 %27
OpStore %alpha %28
%32 = OpLoad %v4float %17
%33 = OpVectorShuffle %v3float %32 %32 0 1 2
%34 = OpLoad %v4float %18
%35 = OpCompositeExtract %float %34 3
%36 = OpVectorTimesScalar %v3float %33 %35
OpStore %sda %36
%38 = OpLoad %v4float %18
%39 = OpVectorShuffle %v3float %38 %38 0 1 2
%40 = OpLoad %v4float %17
%41 = OpCompositeExtract %float %40 3
%42 = OpVectorTimesScalar %v3float %39 %41
OpStore %dsa %42
%44 = OpLoad %bool %19
OpSelectionMerge %48 None
OpBranchConditional %44 %46 %47
%46 = OpLabel
%49 = OpLoad %v3float %dsa
OpStore %45 %49
OpBranch %48
%47 = OpLabel
%50 = OpLoad %v3float %sda
OpStore %45 %50
OpBranch %48
%48 = OpLabel
%51 = OpLoad %v3float %45
OpStore %l %51
%53 = OpLoad %bool %19
OpSelectionMerge %57 None
OpBranchConditional %53 %55 %56
%55 = OpLabel
%58 = OpLoad %v3float %sda
OpStore %54 %58
OpBranch %57
%56 = OpLabel
%59 = OpLoad %v3float %dsa
OpStore %54 %59
OpBranch %57
%57 = OpLabel
%60 = OpLoad %v3float %54
OpStore %r %60
%61 = OpLoad %bool %20
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%65 = OpLoad %v3float %l
OpStore %_2_hueLumColor %65
OpStore %_3_midPt %68
OpStore %_4_satPt %68
%70 = OpLoad %v3float %_2_hueLumColor
%71 = OpCompositeExtract %float %70 0
%72 = OpLoad %v3float %_2_hueLumColor
%73 = OpCompositeExtract %float %72 1
%74 = OpFOrdLessThanEqual %bool %71 %73
OpSelectionMerge %77 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%78 = OpLoad %v3float %_2_hueLumColor
%79 = OpCompositeExtract %float %78 1
%80 = OpLoad %v3float %_2_hueLumColor
%81 = OpCompositeExtract %float %80 2
%82 = OpFOrdLessThanEqual %bool %79 %81
OpSelectionMerge %85 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%86 = OpLoad %v3float %_2_hueLumColor
%87 = OpLoad %v3float %_2_hueLumColor
%88 = OpVectorShuffle %v3float %87 %87 0 0 0
%89 = OpFSub %v3float %86 %88
OpStore %_2_hueLumColor %89
%91 = OpAccessChain %_ptr_Function_float %_4_satPt %int_2
OpStore %91 %float_1
%94 = OpAccessChain %_ptr_Function_float %_3_midPt %int_1
OpStore %94 %float_1
OpBranch %85
%84 = OpLabel
%96 = OpLoad %v3float %_2_hueLumColor
%97 = OpCompositeExtract %float %96 0
%98 = OpLoad %v3float %_2_hueLumColor
%99 = OpCompositeExtract %float %98 2
%100 = OpFOrdLessThanEqual %bool %97 %99
OpSelectionMerge %103 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%104 = OpLoad %v3float %_2_hueLumColor
%105 = OpVectorShuffle %v3float %104 %104 0 2 1
%106 = OpLoad %v3float %_2_hueLumColor
%107 = OpVectorShuffle %v3float %106 %106 0 0 0
%108 = OpFSub %v3float %105 %107
OpStore %_2_hueLumColor %108
%109 = OpAccessChain %_ptr_Function_float %_4_satPt %int_1
OpStore %109 %float_1
%110 = OpAccessChain %_ptr_Function_float %_3_midPt %int_2
OpStore %110 %float_1
OpBranch %103
%102 = OpLabel
%111 = OpLoad %v3float %_2_hueLumColor
%112 = OpVectorShuffle %v3float %111 %111 2 0 1
%113 = OpLoad %v3float %_2_hueLumColor
%114 = OpVectorShuffle %v3float %113 %113 2 2 2
%115 = OpFSub %v3float %112 %114
OpStore %_2_hueLumColor %115
%116 = OpAccessChain %_ptr_Function_float %_4_satPt %int_1
OpStore %116 %float_1
%117 = OpAccessChain %_ptr_Function_float %_3_midPt %int_0
OpStore %117 %float_1
OpBranch %103
%103 = OpLabel
OpBranch %85
%85 = OpLabel
OpBranch %77
%76 = OpLabel
%119 = OpLoad %v3float %_2_hueLumColor
%120 = OpCompositeExtract %float %119 0
%121 = OpLoad %v3float %_2_hueLumColor
%122 = OpCompositeExtract %float %121 2
%123 = OpFOrdLessThanEqual %bool %120 %122
OpSelectionMerge %126 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
%127 = OpLoad %v3float %_2_hueLumColor
%128 = OpVectorShuffle %v3float %127 %127 1 0 2
%129 = OpLoad %v3float %_2_hueLumColor
%130 = OpVectorShuffle %v3float %129 %129 1 1 1
%131 = OpFSub %v3float %128 %130
OpStore %_2_hueLumColor %131
%132 = OpAccessChain %_ptr_Function_float %_4_satPt %int_2
OpStore %132 %float_1
%133 = OpAccessChain %_ptr_Function_float %_3_midPt %int_0
OpStore %133 %float_1
OpBranch %126
%125 = OpLabel
%134 = OpLoad %v3float %_2_hueLumColor
%135 = OpCompositeExtract %float %134 1
%136 = OpLoad %v3float %_2_hueLumColor
%137 = OpCompositeExtract %float %136 2
%138 = OpFOrdLessThanEqual %bool %135 %137
OpSelectionMerge %141 None
OpBranchConditional %138 %139 %140
%139 = OpLabel
%142 = OpLoad %v3float %_2_hueLumColor
%143 = OpVectorShuffle %v3float %142 %142 1 2 0
%144 = OpLoad %v3float %_2_hueLumColor
%145 = OpVectorShuffle %v3float %144 %144 1 1 1
%146 = OpFSub %v3float %143 %145
OpStore %_2_hueLumColor %146
%147 = OpAccessChain %_ptr_Function_float %_4_satPt %int_0
OpStore %147 %float_1
%148 = OpAccessChain %_ptr_Function_float %_3_midPt %int_2
OpStore %148 %float_1
OpBranch %141
%140 = OpLabel
%149 = OpLoad %v3float %_2_hueLumColor
%150 = OpVectorShuffle %v3float %149 %149 2 1 0
%151 = OpLoad %v3float %_2_hueLumColor
%152 = OpVectorShuffle %v3float %151 %151 2 2 2
%153 = OpFSub %v3float %150 %152
OpStore %_2_hueLumColor %153
%154 = OpAccessChain %_ptr_Function_float %_4_satPt %int_0
OpStore %154 %float_1
%155 = OpAccessChain %_ptr_Function_float %_3_midPt %int_1
OpStore %155 %float_1
OpBranch %141
%141 = OpLabel
OpBranch %126
%126 = OpLabel
OpBranch %77
%77 = OpLabel
%156 = OpLoad %v3float %_2_hueLumColor
%157 = OpCompositeExtract %float %156 2
%158 = OpFOrdGreaterThan %bool %157 %float_0
OpSelectionMerge %160 None
OpBranchConditional %158 %159 %160
%159 = OpLabel
%161 = OpLoad %v3float %_3_midPt
%162 = OpLoad %v3float %_2_hueLumColor
%163 = OpCompositeExtract %float %162 1
%164 = OpLoad %v3float %_2_hueLumColor
%165 = OpCompositeExtract %float %164 2
%166 = OpFDiv %float %163 %165
%167 = OpVectorTimesScalar %v3float %161 %166
OpStore %_3_midPt %167
%168 = OpLoad %v3float %_3_midPt
%169 = OpLoad %v3float %_4_satPt
%170 = OpFAdd %v3float %168 %169
%173 = OpLoad %v3float %r
%174 = OpCompositeExtract %float %173 0
%175 = OpLoad %v3float %r
%176 = OpCompositeExtract %float %175 1
%172 = OpExtInst %float %1 FMax %174 %176
%177 = OpLoad %v3float %r
%178 = OpCompositeExtract %float %177 2
%171 = OpExtInst %float %1 FMax %172 %178
%181 = OpLoad %v3float %r
%182 = OpCompositeExtract %float %181 0
%183 = OpLoad %v3float %r
%184 = OpCompositeExtract %float %183 1
%180 = OpExtInst %float %1 FMin %182 %184
%185 = OpLoad %v3float %r
%186 = OpCompositeExtract %float %185 2
%179 = OpExtInst %float %1 FMin %180 %186
%187 = OpFSub %float %171 %179
%188 = OpVectorTimesScalar %v3float %170 %187
OpStore %_2_hueLumColor %188
OpBranch %160
%160 = OpLabel
%189 = OpLoad %v3float %_2_hueLumColor
OpStore %l %189
%190 = OpLoad %v3float %dsa
OpStore %r %190
OpBranch %63
%63 = OpLabel
%197 = OpLoad %v3float %r
%192 = OpDot %float %196 %197
OpStore %_5_lum %192
%199 = OpLoad %float %_5_lum
%201 = OpLoad %v3float %l
%200 = OpDot %float %196 %201
%202 = OpFSub %float %199 %200
%203 = OpLoad %v3float %l
%204 = OpCompositeConstruct %v3float %202 %202 %202
%205 = OpFAdd %v3float %204 %203
OpStore %_6_result %205
%209 = OpLoad %v3float %_6_result
%210 = OpCompositeExtract %float %209 0
%211 = OpLoad %v3float %_6_result
%212 = OpCompositeExtract %float %211 1
%208 = OpExtInst %float %1 FMin %210 %212
%213 = OpLoad %v3float %_6_result
%214 = OpCompositeExtract %float %213 2
%207 = OpExtInst %float %1 FMin %208 %214
OpStore %_7_minComp %207
%218 = OpLoad %v3float %_6_result
%219 = OpCompositeExtract %float %218 0
%220 = OpLoad %v3float %_6_result
%221 = OpCompositeExtract %float %220 1
%217 = OpExtInst %float %1 FMax %219 %221
%222 = OpLoad %v3float %_6_result
%223 = OpCompositeExtract %float %222 2
%216 = OpExtInst %float %1 FMax %217 %223
OpStore %_8_maxComp %216
%225 = OpLoad %float %_7_minComp
%226 = OpFOrdLessThan %bool %225 %float_0
OpSelectionMerge %228 None
OpBranchConditional %226 %227 %228
%227 = OpLabel
%229 = OpLoad %float %_5_lum
%230 = OpLoad %float %_7_minComp
%231 = OpFUnordNotEqual %bool %229 %230
OpBranch %228
%228 = OpLabel
%232 = OpPhi %bool %false %63 %231 %227
OpSelectionMerge %234 None
OpBranchConditional %232 %233 %234
%233 = OpLabel
%235 = OpLoad %float %_5_lum
%236 = OpLoad %v3float %_6_result
%237 = OpLoad %float %_5_lum
%238 = OpCompositeConstruct %v3float %237 %237 %237
%239 = OpFSub %v3float %236 %238
%240 = OpLoad %float %_5_lum
%241 = OpLoad %float %_5_lum
%242 = OpLoad %float %_7_minComp
%243 = OpFSub %float %241 %242
%244 = OpFDiv %float %240 %243
%245 = OpVectorTimesScalar %v3float %239 %244
%246 = OpCompositeConstruct %v3float %235 %235 %235
%247 = OpFAdd %v3float %246 %245
OpStore %_6_result %247
OpBranch %234
%234 = OpLabel
%248 = OpLoad %float %_8_maxComp
%249 = OpLoad %float %alpha
%250 = OpFOrdGreaterThan %bool %248 %249
OpSelectionMerge %252 None
OpBranchConditional %250 %251 %252
%251 = OpLabel
%253 = OpLoad %float %_8_maxComp
%254 = OpLoad %float %_5_lum
%255 = OpFUnordNotEqual %bool %253 %254
OpBranch %252
%252 = OpLabel
%256 = OpPhi %bool %false %234 %255 %251
OpSelectionMerge %258 None
OpBranchConditional %256 %257 %258
%257 = OpLabel
%259 = OpLoad %float %_5_lum
%260 = OpLoad %v3float %_6_result
%261 = OpLoad %float %_5_lum
%262 = OpCompositeConstruct %v3float %261 %261 %261
%263 = OpFSub %v3float %260 %262
%264 = OpLoad %float %alpha
%265 = OpLoad %float %_5_lum
%266 = OpFSub %float %264 %265
%267 = OpVectorTimesScalar %v3float %263 %266
%268 = OpLoad %float %_8_maxComp
%269 = OpLoad %float %_5_lum
%270 = OpFSub %float %268 %269
%271 = OpFDiv %float %float_1 %270
%272 = OpVectorTimesScalar %v3float %267 %271
%273 = OpCompositeConstruct %v3float %259 %259 %259
%274 = OpFAdd %v3float %273 %272
OpStore %_6_result %274
OpBranch %258
%258 = OpLabel
%275 = OpLoad %v3float %_6_result
%276 = OpLoad %v4float %18
%277 = OpVectorShuffle %v3float %276 %276 0 1 2
%278 = OpFAdd %v3float %275 %277
%279 = OpLoad %v3float %dsa
%280 = OpFSub %v3float %278 %279
%281 = OpLoad %v4float %17
%282 = OpVectorShuffle %v3float %281 %281 0 1 2
%283 = OpFAdd %v3float %280 %282
%284 = OpLoad %v3float %sda
%285 = OpFSub %v3float %283 %284
%286 = OpCompositeExtract %float %285 0
%287 = OpCompositeExtract %float %285 1
%288 = OpCompositeExtract %float %285 2
%289 = OpLoad %v4float %17
%290 = OpCompositeExtract %float %289 3
%291 = OpLoad %v4float %18
%292 = OpCompositeExtract %float %291 3
%293 = OpFAdd %float %290 %292
%294 = OpLoad %float %alpha
%295 = OpFSub %float %293 %294
%296 = OpCompositeConstruct %v4float %286 %287 %288 %295
OpReturnValue %296
OpFunctionEnd
%main = OpFunction %void None %298
%299 = OpLabel
%303 = OpVariable %_ptr_Function_v4float Function
%306 = OpVariable %_ptr_Function_v4float Function
%307 = OpVariable %_ptr_Function_bool Function
%308 = OpVariable %_ptr_Function_bool Function
%300 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%302 = OpLoad %v4float %300
OpStore %303 %302
%304 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%305 = OpLoad %v4float %304
OpStore %306 %305
OpStore %307 %false
OpStore %308 %false
%309 = OpFunctionCall %v4float %blend_hslc_h4h4h4bb %303 %306 %307 %308
OpStore %sk_FragColor %309
OpReturn
OpFunctionEnd
