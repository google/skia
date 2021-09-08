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
OpDecorate %minComp RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %maxComp RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
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
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
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
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
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
OpDecorate %sat RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
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
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %_0_alpha RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %_1_sda RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %_2_dsa RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
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
%112 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%139 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%140 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_v3float
%void = OpTypeVoid
%243 = OpTypeFunction %void
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
%69 = OpLoad %float %lum
%70 = OpLoad %v3float %result
%71 = OpLoad %float %lum
%72 = OpCompositeConstruct %v3float %71 %71 %71
%73 = OpFSub %v3float %70 %72
%74 = OpLoad %float %lum
%75 = OpLoad %float %lum
%76 = OpLoad %float %minComp
%77 = OpFSub %float %75 %76
%78 = OpFDiv %float %74 %77
%79 = OpVectorTimesScalar %v3float %73 %78
%80 = OpCompositeConstruct %v3float %69 %69 %69
%81 = OpFAdd %v3float %80 %79
OpStore %result %81
OpBranch %68
%68 = OpLabel
%82 = OpLoad %float %maxComp
%83 = OpLoad %float %21
%84 = OpFOrdGreaterThan %bool %82 %83
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%87 = OpLoad %float %maxComp
%88 = OpLoad %float %lum
%89 = OpFOrdNotEqual %bool %87 %88
OpBranch %86
%86 = OpLabel
%90 = OpPhi %bool %false %68 %89 %85
OpSelectionMerge %93 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%94 = OpLoad %float %lum
%95 = OpLoad %v3float %result
%96 = OpLoad %float %lum
%97 = OpCompositeConstruct %v3float %96 %96 %96
%98 = OpFSub %v3float %95 %97
%99 = OpLoad %float %21
%100 = OpLoad %float %lum
%101 = OpFSub %float %99 %100
%102 = OpVectorTimesScalar %v3float %98 %101
%103 = OpLoad %float %maxComp
%104 = OpLoad %float %lum
%105 = OpFSub %float %103 %104
%107 = OpFDiv %float %float_1 %105
%108 = OpVectorTimesScalar %v3float %102 %107
%109 = OpCompositeConstruct %v3float %94 %94 %94
%110 = OpFAdd %v3float %109 %108
OpReturnValue %110
%92 = OpLabel
%111 = OpLoad %v3float %result
OpReturnValue %111
%93 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_saturation_helper_h3h3h = OpFunction %v3float None %112
%113 = OpFunctionParameter %_ptr_Function_v3float
%114 = OpFunctionParameter %_ptr_Function_float
%115 = OpLabel
%116 = OpLoad %v3float %113
%117 = OpCompositeExtract %float %116 0
%118 = OpLoad %v3float %113
%119 = OpCompositeExtract %float %118 2
%120 = OpFOrdLessThan %bool %117 %119
OpSelectionMerge %123 None
OpBranchConditional %120 %121 %122
%121 = OpLabel
%124 = OpLoad %float %114
%125 = OpLoad %v3float %113
%126 = OpCompositeExtract %float %125 1
%127 = OpLoad %v3float %113
%128 = OpCompositeExtract %float %127 0
%129 = OpFSub %float %126 %128
%130 = OpFMul %float %124 %129
%131 = OpLoad %v3float %113
%132 = OpCompositeExtract %float %131 2
%133 = OpLoad %v3float %113
%134 = OpCompositeExtract %float %133 0
%135 = OpFSub %float %132 %134
%136 = OpFDiv %float %130 %135
%137 = OpLoad %float %114
%138 = OpCompositeConstruct %v3float %float_0 %136 %137
OpReturnValue %138
%122 = OpLabel
OpReturnValue %139
%123 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_saturation_h3h3h3 = OpFunction %v3float None %140
%141 = OpFunctionParameter %_ptr_Function_v3float
%142 = OpFunctionParameter %_ptr_Function_v3float
%143 = OpLabel
%sat = OpVariable %_ptr_Function_float Function
%179 = OpVariable %_ptr_Function_v3float Function
%181 = OpVariable %_ptr_Function_float Function
%193 = OpVariable %_ptr_Function_v3float Function
%195 = OpVariable %_ptr_Function_float Function
%200 = OpVariable %_ptr_Function_v3float Function
%202 = OpVariable %_ptr_Function_float Function
%215 = OpVariable %_ptr_Function_v3float Function
%217 = OpVariable %_ptr_Function_float Function
%230 = OpVariable %_ptr_Function_v3float Function
%232 = OpVariable %_ptr_Function_float Function
%237 = OpVariable %_ptr_Function_v3float Function
%239 = OpVariable %_ptr_Function_float Function
%147 = OpLoad %v3float %142
%148 = OpCompositeExtract %float %147 0
%149 = OpLoad %v3float %142
%150 = OpCompositeExtract %float %149 1
%146 = OpExtInst %float %1 FMax %148 %150
%151 = OpLoad %v3float %142
%152 = OpCompositeExtract %float %151 2
%145 = OpExtInst %float %1 FMax %146 %152
%155 = OpLoad %v3float %142
%156 = OpCompositeExtract %float %155 0
%157 = OpLoad %v3float %142
%158 = OpCompositeExtract %float %157 1
%154 = OpExtInst %float %1 FMin %156 %158
%159 = OpLoad %v3float %142
%160 = OpCompositeExtract %float %159 2
%153 = OpExtInst %float %1 FMin %154 %160
%161 = OpFSub %float %145 %153
OpStore %sat %161
%162 = OpLoad %v3float %141
%163 = OpCompositeExtract %float %162 0
%164 = OpLoad %v3float %141
%165 = OpCompositeExtract %float %164 1
%166 = OpFOrdLessThanEqual %bool %163 %165
OpSelectionMerge %169 None
OpBranchConditional %166 %167 %168
%167 = OpLabel
%170 = OpLoad %v3float %141
%171 = OpCompositeExtract %float %170 1
%172 = OpLoad %v3float %141
%173 = OpCompositeExtract %float %172 2
%174 = OpFOrdLessThanEqual %bool %171 %173
OpSelectionMerge %177 None
OpBranchConditional %174 %175 %176
%175 = OpLabel
%178 = OpLoad %v3float %141
OpStore %179 %178
%180 = OpLoad %float %sat
OpStore %181 %180
%182 = OpFunctionCall %v3float %_blend_set_color_saturation_helper_h3h3h %179 %181
OpReturnValue %182
%176 = OpLabel
%183 = OpLoad %v3float %141
%184 = OpCompositeExtract %float %183 0
%185 = OpLoad %v3float %141
%186 = OpCompositeExtract %float %185 2
%187 = OpFOrdLessThanEqual %bool %184 %186
OpSelectionMerge %190 None
OpBranchConditional %187 %188 %189
%188 = OpLabel
%191 = OpLoad %v3float %141
%192 = OpVectorShuffle %v3float %191 %191 0 2 1
OpStore %193 %192
%194 = OpLoad %float %sat
OpStore %195 %194
%196 = OpFunctionCall %v3float %_blend_set_color_saturation_helper_h3h3h %193 %195
%197 = OpVectorShuffle %v3float %196 %196 0 2 1
OpReturnValue %197
%189 = OpLabel
%198 = OpLoad %v3float %141
%199 = OpVectorShuffle %v3float %198 %198 2 0 1
OpStore %200 %199
%201 = OpLoad %float %sat
OpStore %202 %201
%203 = OpFunctionCall %v3float %_blend_set_color_saturation_helper_h3h3h %200 %202
%204 = OpVectorShuffle %v3float %203 %203 1 2 0
OpReturnValue %204
%190 = OpLabel
OpBranch %177
%177 = OpLabel
OpBranch %169
%168 = OpLabel
%205 = OpLoad %v3float %141
%206 = OpCompositeExtract %float %205 0
%207 = OpLoad %v3float %141
%208 = OpCompositeExtract %float %207 2
%209 = OpFOrdLessThanEqual %bool %206 %208
OpSelectionMerge %212 None
OpBranchConditional %209 %210 %211
%210 = OpLabel
%213 = OpLoad %v3float %141
%214 = OpVectorShuffle %v3float %213 %213 1 0 2
OpStore %215 %214
%216 = OpLoad %float %sat
OpStore %217 %216
%218 = OpFunctionCall %v3float %_blend_set_color_saturation_helper_h3h3h %215 %217
%219 = OpVectorShuffle %v3float %218 %218 1 0 2
OpReturnValue %219
%211 = OpLabel
%220 = OpLoad %v3float %141
%221 = OpCompositeExtract %float %220 1
%222 = OpLoad %v3float %141
%223 = OpCompositeExtract %float %222 2
%224 = OpFOrdLessThanEqual %bool %221 %223
OpSelectionMerge %227 None
OpBranchConditional %224 %225 %226
%225 = OpLabel
%228 = OpLoad %v3float %141
%229 = OpVectorShuffle %v3float %228 %228 1 2 0
OpStore %230 %229
%231 = OpLoad %float %sat
OpStore %232 %231
%233 = OpFunctionCall %v3float %_blend_set_color_saturation_helper_h3h3h %230 %232
%234 = OpVectorShuffle %v3float %233 %233 2 0 1
OpReturnValue %234
%226 = OpLabel
%235 = OpLoad %v3float %141
%236 = OpVectorShuffle %v3float %235 %235 2 1 0
OpStore %237 %236
%238 = OpLoad %float %sat
OpStore %239 %238
%240 = OpFunctionCall %v3float %_blend_set_color_saturation_helper_h3h3h %237 %239
%241 = OpVectorShuffle %v3float %240 %240 2 1 0
OpReturnValue %241
%227 = OpLabel
OpBranch %212
%212 = OpLabel
OpBranch %169
%169 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %243
%244 = OpLabel
%_0_alpha = OpVariable %_ptr_Function_float Function
%_1_sda = OpVariable %_ptr_Function_v3float Function
%_2_dsa = OpVariable %_ptr_Function_v3float Function
%274 = OpVariable %_ptr_Function_v3float Function
%276 = OpVariable %_ptr_Function_v3float Function
%278 = OpVariable %_ptr_Function_v3float Function
%280 = OpVariable %_ptr_Function_float Function
%282 = OpVariable %_ptr_Function_v3float Function
%246 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%250 = OpLoad %v4float %246
%251 = OpCompositeExtract %float %250 3
%252 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%254 = OpLoad %v4float %252
%255 = OpCompositeExtract %float %254 3
%256 = OpFMul %float %251 %255
OpStore %_0_alpha %256
%258 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%259 = OpLoad %v4float %258
%260 = OpVectorShuffle %v3float %259 %259 0 1 2
%261 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%262 = OpLoad %v4float %261
%263 = OpCompositeExtract %float %262 3
%264 = OpVectorTimesScalar %v3float %260 %263
OpStore %_1_sda %264
%266 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%267 = OpLoad %v4float %266
%268 = OpVectorShuffle %v3float %267 %267 0 1 2
%269 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%270 = OpLoad %v4float %269
%271 = OpCompositeExtract %float %270 3
%272 = OpVectorTimesScalar %v3float %268 %271
OpStore %_2_dsa %272
%273 = OpLoad %v3float %_2_dsa
OpStore %274 %273
%275 = OpLoad %v3float %_1_sda
OpStore %276 %275
%277 = OpFunctionCall %v3float %_blend_set_color_saturation_h3h3h3 %274 %276
OpStore %278 %277
%279 = OpLoad %float %_0_alpha
OpStore %280 %279
%281 = OpLoad %v3float %_2_dsa
OpStore %282 %281
%283 = OpFunctionCall %v3float %_blend_set_color_luminance_h3h3hh3 %278 %280 %282
%284 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%285 = OpLoad %v4float %284
%286 = OpVectorShuffle %v3float %285 %285 0 1 2
%287 = OpFAdd %v3float %283 %286
%288 = OpLoad %v3float %_2_dsa
%289 = OpFSub %v3float %287 %288
%290 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%291 = OpLoad %v4float %290
%292 = OpVectorShuffle %v3float %291 %291 0 1 2
%293 = OpFAdd %v3float %289 %292
%294 = OpLoad %v3float %_1_sda
%295 = OpFSub %v3float %293 %294
%296 = OpCompositeExtract %float %295 0
%297 = OpCompositeExtract %float %295 1
%298 = OpCompositeExtract %float %295 2
%299 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%300 = OpLoad %v4float %299
%301 = OpCompositeExtract %float %300 3
%302 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%303 = OpLoad %v4float %302
%304 = OpCompositeExtract %float %303 3
%305 = OpFAdd %float %301 %304
%306 = OpLoad %float %_0_alpha
%307 = OpFSub %float %305 %306
%308 = OpCompositeConstruct %v4float %296 %297 %298 %307
OpStore %sk_FragColor %308
OpReturn
OpFunctionEnd
