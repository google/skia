OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %_GuardedDivideEpsilon "$GuardedDivideEpsilon"
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %guarded_divide_Qhhh "guarded_divide_Qhhh"
OpName %soft_light_component_Qhh2h2 "soft_light_component_Qhh2h2"
OpName %DSqd "DSqd"
OpName %DCub "DCub"
OpName %DaSqd "DaSqd"
OpName %DaCub "DaCub"
OpName %blend_soft_light_h4h4h4 "blend_soft_light_h4h4h4"
OpName %main "main"
OpDecorate %_GuardedDivideEpsilon RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %19 Binding 0
OpDecorate %19 DescriptorSet 0
OpDecorate %27 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
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
OpDecorate %84 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %DSqd RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %DCub RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %DaSqd RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %DaCub RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
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
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
%float = OpTypeFloat 32
%_ptr_Private_float = OpTypePointer Private %float
%_GuardedDivideEpsilon = OpVariable %_ptr_Private_float Private
%bool = OpTypeBool
%false = OpConstantFalse %bool
%float_9_99999994en09 = OpConstant %float 9.99999994e-09
%float_0 = OpConstant %float 0
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%19 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Function_float = OpTypePointer Function %float
%23 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%34 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_2 = OpConstant %float 2
%float_1 = OpConstant %float 1
%float_4 = OpConstant %float 4
%float_3 = OpConstant %float 3
%float_6 = OpConstant %float 6
%float_12 = OpConstant %float 12
%float_16 = OpConstant %float 16
%_ptr_Function_v4float = OpTypePointer Function %v4float
%196 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%void = OpTypeVoid
%241 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%guarded_divide_Qhhh = OpFunction %float None %23
%24 = OpFunctionParameter %_ptr_Function_float
%25 = OpFunctionParameter %_ptr_Function_float
%26 = OpLabel
%27 = OpLoad %float %24
%28 = OpLoad %float %25
%29 = OpLoad %float %_GuardedDivideEpsilon
%30 = OpFAdd %float %28 %29
%31 = OpFDiv %float %27 %30
OpReturnValue %31
OpFunctionEnd
%soft_light_component_Qhh2h2 = OpFunction %float None %34
%35 = OpFunctionParameter %_ptr_Function_v2float
%36 = OpFunctionParameter %_ptr_Function_v2float
%37 = OpLabel
%60 = OpVariable %_ptr_Function_float Function
%63 = OpVariable %_ptr_Function_float Function
%DSqd = OpVariable %_ptr_Function_float Function
%DCub = OpVariable %_ptr_Function_float Function
%DaSqd = OpVariable %_ptr_Function_float Function
%DaCub = OpVariable %_ptr_Function_float Function
%159 = OpVariable %_ptr_Function_float Function
%160 = OpVariable %_ptr_Function_float Function
%39 = OpLoad %v2float %35
%40 = OpCompositeExtract %float %39 0
%41 = OpFMul %float %float_2 %40
%42 = OpLoad %v2float %35
%43 = OpCompositeExtract %float %42 1
%44 = OpFOrdLessThanEqual %bool %41 %43
OpSelectionMerge %47 None
OpBranchConditional %44 %45 %46
%45 = OpLabel
%48 = OpLoad %v2float %36
%49 = OpCompositeExtract %float %48 0
%50 = OpLoad %v2float %36
%51 = OpCompositeExtract %float %50 0
%52 = OpFMul %float %49 %51
%53 = OpLoad %v2float %35
%54 = OpCompositeExtract %float %53 1
%55 = OpLoad %v2float %35
%56 = OpCompositeExtract %float %55 0
%57 = OpFMul %float %float_2 %56
%58 = OpFSub %float %54 %57
%59 = OpFMul %float %52 %58
OpStore %60 %59
%61 = OpLoad %v2float %36
%62 = OpCompositeExtract %float %61 1
OpStore %63 %62
%64 = OpFunctionCall %float %guarded_divide_Qhhh %60 %63
%66 = OpLoad %v2float %36
%67 = OpCompositeExtract %float %66 1
%68 = OpFSub %float %float_1 %67
%69 = OpLoad %v2float %35
%70 = OpCompositeExtract %float %69 0
%71 = OpFMul %float %68 %70
%72 = OpFAdd %float %64 %71
%73 = OpLoad %v2float %36
%74 = OpCompositeExtract %float %73 0
%75 = OpLoad %v2float %35
%76 = OpCompositeExtract %float %75 1
%77 = OpFNegate %float %76
%78 = OpLoad %v2float %35
%79 = OpCompositeExtract %float %78 0
%80 = OpFMul %float %float_2 %79
%81 = OpFAdd %float %77 %80
%82 = OpFAdd %float %81 %float_1
%83 = OpFMul %float %74 %82
%84 = OpFAdd %float %72 %83
OpReturnValue %84
%46 = OpLabel
%86 = OpLoad %v2float %36
%87 = OpCompositeExtract %float %86 0
%88 = OpFMul %float %float_4 %87
%89 = OpLoad %v2float %36
%90 = OpCompositeExtract %float %89 1
%91 = OpFOrdLessThanEqual %bool %88 %90
OpSelectionMerge %94 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%96 = OpLoad %v2float %36
%97 = OpCompositeExtract %float %96 0
%98 = OpLoad %v2float %36
%99 = OpCompositeExtract %float %98 0
%100 = OpFMul %float %97 %99
OpStore %DSqd %100
%102 = OpLoad %v2float %36
%103 = OpCompositeExtract %float %102 0
%104 = OpFMul %float %100 %103
OpStore %DCub %104
%106 = OpLoad %v2float %36
%107 = OpCompositeExtract %float %106 1
%108 = OpLoad %v2float %36
%109 = OpCompositeExtract %float %108 1
%110 = OpFMul %float %107 %109
OpStore %DaSqd %110
%112 = OpLoad %v2float %36
%113 = OpCompositeExtract %float %112 1
%114 = OpFMul %float %110 %113
OpStore %DaCub %114
%115 = OpLoad %v2float %35
%116 = OpCompositeExtract %float %115 0
%117 = OpLoad %v2float %36
%118 = OpCompositeExtract %float %117 0
%120 = OpLoad %v2float %35
%121 = OpCompositeExtract %float %120 1
%122 = OpFMul %float %float_3 %121
%124 = OpLoad %v2float %35
%125 = OpCompositeExtract %float %124 0
%126 = OpFMul %float %float_6 %125
%127 = OpFSub %float %122 %126
%128 = OpFSub %float %127 %float_1
%129 = OpFMul %float %118 %128
%130 = OpFSub %float %116 %129
%131 = OpFMul %float %110 %130
%133 = OpLoad %v2float %36
%134 = OpCompositeExtract %float %133 1
%135 = OpFMul %float %float_12 %134
%136 = OpFMul %float %135 %100
%137 = OpLoad %v2float %35
%138 = OpCompositeExtract %float %137 1
%139 = OpLoad %v2float %35
%140 = OpCompositeExtract %float %139 0
%141 = OpFMul %float %float_2 %140
%142 = OpFSub %float %138 %141
%143 = OpFMul %float %136 %142
%144 = OpFAdd %float %131 %143
%146 = OpFMul %float %float_16 %104
%147 = OpLoad %v2float %35
%148 = OpCompositeExtract %float %147 1
%149 = OpLoad %v2float %35
%150 = OpCompositeExtract %float %149 0
%151 = OpFMul %float %float_2 %150
%152 = OpFSub %float %148 %151
%153 = OpFMul %float %146 %152
%154 = OpFSub %float %144 %153
%155 = OpLoad %v2float %35
%156 = OpCompositeExtract %float %155 0
%157 = OpFMul %float %114 %156
%158 = OpFSub %float %154 %157
OpStore %159 %158
OpStore %160 %110
%161 = OpFunctionCall %float %guarded_divide_Qhhh %159 %160
OpReturnValue %161
%93 = OpLabel
%162 = OpLoad %v2float %36
%163 = OpCompositeExtract %float %162 0
%164 = OpLoad %v2float %35
%165 = OpCompositeExtract %float %164 1
%166 = OpLoad %v2float %35
%167 = OpCompositeExtract %float %166 0
%168 = OpFMul %float %float_2 %167
%169 = OpFSub %float %165 %168
%170 = OpFAdd %float %169 %float_1
%171 = OpFMul %float %163 %170
%172 = OpLoad %v2float %35
%173 = OpCompositeExtract %float %172 0
%174 = OpFAdd %float %171 %173
%176 = OpLoad %v2float %36
%177 = OpCompositeExtract %float %176 1
%178 = OpLoad %v2float %36
%179 = OpCompositeExtract %float %178 0
%180 = OpFMul %float %177 %179
%175 = OpExtInst %float %1 Sqrt %180
%181 = OpLoad %v2float %35
%182 = OpCompositeExtract %float %181 1
%183 = OpLoad %v2float %35
%184 = OpCompositeExtract %float %183 0
%185 = OpFMul %float %float_2 %184
%186 = OpFSub %float %182 %185
%187 = OpFMul %float %175 %186
%188 = OpFSub %float %174 %187
%189 = OpLoad %v2float %36
%190 = OpCompositeExtract %float %189 1
%191 = OpLoad %v2float %35
%192 = OpCompositeExtract %float %191 0
%193 = OpFMul %float %190 %192
%194 = OpFSub %float %188 %193
OpReturnValue %194
%94 = OpLabel
OpBranch %47
%47 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_soft_light_h4h4h4 = OpFunction %v4float None %196
%197 = OpFunctionParameter %_ptr_Function_v4float
%198 = OpFunctionParameter %_ptr_Function_v4float
%199 = OpLabel
%203 = OpVariable %_ptr_Function_v4float Function
%210 = OpVariable %_ptr_Function_v2float Function
%213 = OpVariable %_ptr_Function_v2float Function
%217 = OpVariable %_ptr_Function_v2float Function
%220 = OpVariable %_ptr_Function_v2float Function
%224 = OpVariable %_ptr_Function_v2float Function
%227 = OpVariable %_ptr_Function_v2float Function
%200 = OpLoad %v4float %198
%201 = OpCompositeExtract %float %200 3
%202 = OpFOrdEqual %bool %201 %float_0
OpSelectionMerge %206 None
OpBranchConditional %202 %204 %205
%204 = OpLabel
%207 = OpLoad %v4float %197
OpStore %203 %207
OpBranch %206
%205 = OpLabel
%208 = OpLoad %v4float %197
%209 = OpVectorShuffle %v2float %208 %208 0 3
OpStore %210 %209
%211 = OpLoad %v4float %198
%212 = OpVectorShuffle %v2float %211 %211 0 3
OpStore %213 %212
%214 = OpFunctionCall %float %soft_light_component_Qhh2h2 %210 %213
%215 = OpLoad %v4float %197
%216 = OpVectorShuffle %v2float %215 %215 1 3
OpStore %217 %216
%218 = OpLoad %v4float %198
%219 = OpVectorShuffle %v2float %218 %218 1 3
OpStore %220 %219
%221 = OpFunctionCall %float %soft_light_component_Qhh2h2 %217 %220
%222 = OpLoad %v4float %197
%223 = OpVectorShuffle %v2float %222 %222 2 3
OpStore %224 %223
%225 = OpLoad %v4float %198
%226 = OpVectorShuffle %v2float %225 %225 2 3
OpStore %227 %226
%228 = OpFunctionCall %float %soft_light_component_Qhh2h2 %224 %227
%229 = OpLoad %v4float %197
%230 = OpCompositeExtract %float %229 3
%231 = OpLoad %v4float %197
%232 = OpCompositeExtract %float %231 3
%233 = OpFSub %float %float_1 %232
%234 = OpLoad %v4float %198
%235 = OpCompositeExtract %float %234 3
%236 = OpFMul %float %233 %235
%237 = OpFAdd %float %230 %236
%238 = OpCompositeConstruct %v4float %214 %221 %228 %237
OpStore %203 %238
OpBranch %206
%206 = OpLabel
%239 = OpLoad %v4float %203
OpReturnValue %239
OpFunctionEnd
%main = OpFunction %void None %241
%242 = OpLabel
%248 = OpVariable %_ptr_Function_v4float Function
%252 = OpVariable %_ptr_Function_v4float Function
%11 = OpSelect %float %false %float_9_99999994en09 %float_0
OpStore %_GuardedDivideEpsilon %11
%243 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%247 = OpLoad %v4float %243
OpStore %248 %247
%249 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%251 = OpLoad %v4float %249
OpStore %252 %251
%253 = OpFunctionCall %v4float %blend_soft_light_h4h4h4 %248 %252
OpStore %sk_FragColor %253
OpReturn
OpFunctionEnd
