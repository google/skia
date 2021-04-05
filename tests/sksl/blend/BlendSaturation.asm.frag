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
OpName %_blend_set_color_luminance_h3h3hh3 "_blend_set_color_luminance_h3h3hh3"
OpName %lum "lum"
OpName %result "result"
OpName %minComp "minComp"
OpName %maxComp "maxComp"
OpName %_blend_set_color_saturation_helper_h3h3h "_blend_set_color_saturation_helper_h3h3h"
OpName %_blend_set_color_saturation_h3h3h3 "_blend_set_color_saturation_h3h3h3"
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
OpDecorate %lum RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %result RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %minComp RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %maxComp RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %sat RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %_0_alpha RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %_1_sda RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %_2_dsa RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
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
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%113 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%140 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%141 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_v3float
%void = OpTypeVoid
%244 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%_blend_set_color_luminance_h3h3hh3 = OpFunction %v3float None %17
%20 = OpFunctionParameter %_ptr_Function_v3float
%21 = OpFunctionParameter %_ptr_Function_float
%22 = OpFunctionParameter %_ptr_Function_v3float
%23 = OpLabel
%lum = OpVariable %_ptr_Function_float Function
%result = OpVariable %_ptr_Function_v3float Function
%minComp = OpVariable %_ptr_Function_float Function
%maxComp = OpVariable %_ptr_Function_float Function
%29 = OpCompositeConstruct %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%30 = OpLoad %v3float %22
%25 = OpDot %float %29 %30
OpStore %lum %25
%32 = OpLoad %float %lum
%34 = OpCompositeConstruct %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%35 = OpLoad %v3float %20
%33 = OpDot %float %34 %35
%36 = OpFSub %float %32 %33
%37 = OpLoad %v3float %20
%38 = OpCompositeConstruct %v3float %36 %36 %36
%39 = OpFAdd %v3float %38 %37
OpStore %result %39
%43 = OpLoad %v3float %result
%44 = OpCompositeExtract %float %43 0
%45 = OpLoad %v3float %result
%46 = OpCompositeExtract %float %45 1
%42 = OpExtInst %float %1 FMin %44 %46
%47 = OpLoad %v3float %result
%48 = OpCompositeExtract %float %47 2
%41 = OpExtInst %float %1 FMin %42 %48
OpStore %minComp %41
%52 = OpLoad %v3float %result
%53 = OpCompositeExtract %float %52 0
%54 = OpLoad %v3float %result
%55 = OpCompositeExtract %float %54 1
%51 = OpExtInst %float %1 FMax %53 %55
%56 = OpLoad %v3float %result
%57 = OpCompositeExtract %float %56 2
%50 = OpExtInst %float %1 FMax %51 %57
OpStore %maxComp %50
%59 = OpLoad %float %minComp
%61 = OpFOrdLessThan %bool %59 %float_0
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%64 = OpLoad %float %lum
%65 = OpLoad %float %minComp
%66 = OpFOrdNotEqual %bool %64 %65
OpBranch %63
%63 = OpLabel
%67 = OpPhi %bool %false %23 %66 %62
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%70 = OpLoad %float %lum
%71 = OpLoad %v3float %result
%72 = OpLoad %float %lum
%73 = OpCompositeConstruct %v3float %72 %72 %72
%74 = OpFSub %v3float %71 %73
%75 = OpLoad %float %lum
%76 = OpLoad %float %lum
%77 = OpLoad %float %minComp
%78 = OpFSub %float %76 %77
%79 = OpFDiv %float %75 %78
%80 = OpVectorTimesScalar %v3float %74 %79
%81 = OpCompositeConstruct %v3float %70 %70 %70
%82 = OpFAdd %v3float %81 %80
OpStore %result %82
OpBranch %69
%69 = OpLabel
%83 = OpLoad %float %maxComp
%84 = OpLoad %float %21
%85 = OpFOrdGreaterThan %bool %83 %84
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%88 = OpLoad %float %maxComp
%89 = OpLoad %float %lum
%90 = OpFOrdNotEqual %bool %88 %89
OpBranch %87
%87 = OpLabel
%91 = OpPhi %bool %false %69 %90 %86
OpSelectionMerge %94 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%95 = OpLoad %float %lum
%96 = OpLoad %v3float %result
%97 = OpLoad %float %lum
%98 = OpCompositeConstruct %v3float %97 %97 %97
%99 = OpFSub %v3float %96 %98
%100 = OpLoad %float %21
%101 = OpLoad %float %lum
%102 = OpFSub %float %100 %101
%103 = OpVectorTimesScalar %v3float %99 %102
%104 = OpLoad %float %maxComp
%105 = OpLoad %float %lum
%106 = OpFSub %float %104 %105
%108 = OpFDiv %float %float_1 %106
%109 = OpVectorTimesScalar %v3float %103 %108
%110 = OpCompositeConstruct %v3float %95 %95 %95
%111 = OpFAdd %v3float %110 %109
OpReturnValue %111
%93 = OpLabel
%112 = OpLoad %v3float %result
OpReturnValue %112
%94 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_saturation_helper_h3h3h = OpFunction %v3float None %113
%114 = OpFunctionParameter %_ptr_Function_v3float
%115 = OpFunctionParameter %_ptr_Function_float
%116 = OpLabel
%117 = OpLoad %v3float %114
%118 = OpCompositeExtract %float %117 0
%119 = OpLoad %v3float %114
%120 = OpCompositeExtract %float %119 2
%121 = OpFOrdLessThan %bool %118 %120
OpSelectionMerge %124 None
OpBranchConditional %121 %122 %123
%122 = OpLabel
%125 = OpLoad %float %115
%126 = OpLoad %v3float %114
%127 = OpCompositeExtract %float %126 1
%128 = OpLoad %v3float %114
%129 = OpCompositeExtract %float %128 0
%130 = OpFSub %float %127 %129
%131 = OpFMul %float %125 %130
%132 = OpLoad %v3float %114
%133 = OpCompositeExtract %float %132 2
%134 = OpLoad %v3float %114
%135 = OpCompositeExtract %float %134 0
%136 = OpFSub %float %133 %135
%137 = OpFDiv %float %131 %136
%138 = OpLoad %float %115
%139 = OpCompositeConstruct %v3float %float_0 %137 %138
OpReturnValue %139
%123 = OpLabel
OpReturnValue %140
%124 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_saturation_h3h3h3 = OpFunction %v3float None %141
%142 = OpFunctionParameter %_ptr_Function_v3float
%143 = OpFunctionParameter %_ptr_Function_v3float
%144 = OpLabel
%sat = OpVariable %_ptr_Function_float Function
%180 = OpVariable %_ptr_Function_v3float Function
%182 = OpVariable %_ptr_Function_float Function
%194 = OpVariable %_ptr_Function_v3float Function
%196 = OpVariable %_ptr_Function_float Function
%201 = OpVariable %_ptr_Function_v3float Function
%203 = OpVariable %_ptr_Function_float Function
%216 = OpVariable %_ptr_Function_v3float Function
%218 = OpVariable %_ptr_Function_float Function
%231 = OpVariable %_ptr_Function_v3float Function
%233 = OpVariable %_ptr_Function_float Function
%238 = OpVariable %_ptr_Function_v3float Function
%240 = OpVariable %_ptr_Function_float Function
%148 = OpLoad %v3float %143
%149 = OpCompositeExtract %float %148 0
%150 = OpLoad %v3float %143
%151 = OpCompositeExtract %float %150 1
%147 = OpExtInst %float %1 FMax %149 %151
%152 = OpLoad %v3float %143
%153 = OpCompositeExtract %float %152 2
%146 = OpExtInst %float %1 FMax %147 %153
%156 = OpLoad %v3float %143
%157 = OpCompositeExtract %float %156 0
%158 = OpLoad %v3float %143
%159 = OpCompositeExtract %float %158 1
%155 = OpExtInst %float %1 FMin %157 %159
%160 = OpLoad %v3float %143
%161 = OpCompositeExtract %float %160 2
%154 = OpExtInst %float %1 FMin %155 %161
%162 = OpFSub %float %146 %154
OpStore %sat %162
%163 = OpLoad %v3float %142
%164 = OpCompositeExtract %float %163 0
%165 = OpLoad %v3float %142
%166 = OpCompositeExtract %float %165 1
%167 = OpFOrdLessThanEqual %bool %164 %166
OpSelectionMerge %170 None
OpBranchConditional %167 %168 %169
%168 = OpLabel
%171 = OpLoad %v3float %142
%172 = OpCompositeExtract %float %171 1
%173 = OpLoad %v3float %142
%174 = OpCompositeExtract %float %173 2
%175 = OpFOrdLessThanEqual %bool %172 %174
OpSelectionMerge %178 None
OpBranchConditional %175 %176 %177
%176 = OpLabel
%179 = OpLoad %v3float %142
OpStore %180 %179
%181 = OpLoad %float %sat
OpStore %182 %181
%183 = OpFunctionCall %v3float %_blend_set_color_saturation_helper_h3h3h %180 %182
OpReturnValue %183
%177 = OpLabel
%184 = OpLoad %v3float %142
%185 = OpCompositeExtract %float %184 0
%186 = OpLoad %v3float %142
%187 = OpCompositeExtract %float %186 2
%188 = OpFOrdLessThanEqual %bool %185 %187
OpSelectionMerge %191 None
OpBranchConditional %188 %189 %190
%189 = OpLabel
%192 = OpLoad %v3float %142
%193 = OpVectorShuffle %v3float %192 %192 0 2 1
OpStore %194 %193
%195 = OpLoad %float %sat
OpStore %196 %195
%197 = OpFunctionCall %v3float %_blend_set_color_saturation_helper_h3h3h %194 %196
%198 = OpVectorShuffle %v3float %197 %197 0 2 1
OpReturnValue %198
%190 = OpLabel
%199 = OpLoad %v3float %142
%200 = OpVectorShuffle %v3float %199 %199 2 0 1
OpStore %201 %200
%202 = OpLoad %float %sat
OpStore %203 %202
%204 = OpFunctionCall %v3float %_blend_set_color_saturation_helper_h3h3h %201 %203
%205 = OpVectorShuffle %v3float %204 %204 1 2 0
OpReturnValue %205
%191 = OpLabel
OpBranch %178
%178 = OpLabel
OpBranch %170
%169 = OpLabel
%206 = OpLoad %v3float %142
%207 = OpCompositeExtract %float %206 0
%208 = OpLoad %v3float %142
%209 = OpCompositeExtract %float %208 2
%210 = OpFOrdLessThanEqual %bool %207 %209
OpSelectionMerge %213 None
OpBranchConditional %210 %211 %212
%211 = OpLabel
%214 = OpLoad %v3float %142
%215 = OpVectorShuffle %v3float %214 %214 1 0 2
OpStore %216 %215
%217 = OpLoad %float %sat
OpStore %218 %217
%219 = OpFunctionCall %v3float %_blend_set_color_saturation_helper_h3h3h %216 %218
%220 = OpVectorShuffle %v3float %219 %219 1 0 2
OpReturnValue %220
%212 = OpLabel
%221 = OpLoad %v3float %142
%222 = OpCompositeExtract %float %221 1
%223 = OpLoad %v3float %142
%224 = OpCompositeExtract %float %223 2
%225 = OpFOrdLessThanEqual %bool %222 %224
OpSelectionMerge %228 None
OpBranchConditional %225 %226 %227
%226 = OpLabel
%229 = OpLoad %v3float %142
%230 = OpVectorShuffle %v3float %229 %229 1 2 0
OpStore %231 %230
%232 = OpLoad %float %sat
OpStore %233 %232
%234 = OpFunctionCall %v3float %_blend_set_color_saturation_helper_h3h3h %231 %233
%235 = OpVectorShuffle %v3float %234 %234 2 0 1
OpReturnValue %235
%227 = OpLabel
%236 = OpLoad %v3float %142
%237 = OpVectorShuffle %v3float %236 %236 2 1 0
OpStore %238 %237
%239 = OpLoad %float %sat
OpStore %240 %239
%241 = OpFunctionCall %v3float %_blend_set_color_saturation_helper_h3h3h %238 %240
%242 = OpVectorShuffle %v3float %241 %241 2 1 0
OpReturnValue %242
%228 = OpLabel
OpBranch %213
%213 = OpLabel
OpBranch %170
%170 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %244
%245 = OpLabel
%_0_alpha = OpVariable %_ptr_Function_float Function
%_1_sda = OpVariable %_ptr_Function_v3float Function
%_2_dsa = OpVariable %_ptr_Function_v3float Function
%275 = OpVariable %_ptr_Function_v3float Function
%277 = OpVariable %_ptr_Function_v3float Function
%279 = OpVariable %_ptr_Function_v3float Function
%281 = OpVariable %_ptr_Function_float Function
%283 = OpVariable %_ptr_Function_v3float Function
%247 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%251 = OpLoad %v4float %247
%252 = OpCompositeExtract %float %251 3
%253 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%255 = OpLoad %v4float %253
%256 = OpCompositeExtract %float %255 3
%257 = OpFMul %float %252 %256
OpStore %_0_alpha %257
%259 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%260 = OpLoad %v4float %259
%261 = OpVectorShuffle %v3float %260 %260 0 1 2
%262 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%263 = OpLoad %v4float %262
%264 = OpCompositeExtract %float %263 3
%265 = OpVectorTimesScalar %v3float %261 %264
OpStore %_1_sda %265
%267 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%268 = OpLoad %v4float %267
%269 = OpVectorShuffle %v3float %268 %268 0 1 2
%270 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%271 = OpLoad %v4float %270
%272 = OpCompositeExtract %float %271 3
%273 = OpVectorTimesScalar %v3float %269 %272
OpStore %_2_dsa %273
%274 = OpLoad %v3float %_2_dsa
OpStore %275 %274
%276 = OpLoad %v3float %_1_sda
OpStore %277 %276
%278 = OpFunctionCall %v3float %_blend_set_color_saturation_h3h3h3 %275 %277
OpStore %279 %278
%280 = OpLoad %float %_0_alpha
OpStore %281 %280
%282 = OpLoad %v3float %_2_dsa
OpStore %283 %282
%284 = OpFunctionCall %v3float %_blend_set_color_luminance_h3h3hh3 %279 %281 %283
%285 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%286 = OpLoad %v4float %285
%287 = OpVectorShuffle %v3float %286 %286 0 1 2
%288 = OpFAdd %v3float %284 %287
%289 = OpLoad %v3float %_2_dsa
%290 = OpFSub %v3float %288 %289
%291 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%292 = OpLoad %v4float %291
%293 = OpVectorShuffle %v3float %292 %292 0 1 2
%294 = OpFAdd %v3float %290 %293
%295 = OpLoad %v3float %_1_sda
%296 = OpFSub %v3float %294 %295
%297 = OpCompositeExtract %float %296 0
%298 = OpCompositeExtract %float %296 1
%299 = OpCompositeExtract %float %296 2
%300 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%301 = OpLoad %v4float %300
%302 = OpCompositeExtract %float %301 3
%303 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%304 = OpLoad %v4float %303
%305 = OpCompositeExtract %float %304 3
%306 = OpFAdd %float %302 %305
%307 = OpLoad %float %_0_alpha
%308 = OpFSub %float %306 %307
%309 = OpCompositeConstruct %v4float %297 %298 %299 %308
OpStore %sk_FragColor %309
OpReturn
OpFunctionEnd
