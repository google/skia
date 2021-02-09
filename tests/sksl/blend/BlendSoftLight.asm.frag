OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %_guarded_divide "_guarded_divide"
OpName %_soft_light_component "_soft_light_component"
OpName %_8_n "_8_n"
OpName %DSqd "DSqd"
OpName %DCub "DCub"
OpName %DaSqd "DaSqd"
OpName %DaCub "DaCub"
OpName %_10_n "_10_n"
OpName %blend_soft_light "blend_soft_light"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
%16 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_2 = OpConstant %float 2
%float_1 = OpConstant %float 1
%float_4 = OpConstant %float 4
%float_3 = OpConstant %float 3
%float_6 = OpConstant %float 6
%float_12 = OpConstant %float 12
%float_16 = OpConstant %float 16
%_ptr_Function_v4float = OpTypePointer Function %v4float
%194 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%float_0 = OpConstant %float 0
%void = OpTypeVoid
%241 = OpTypeFunction %void
%_guarded_divide = OpFunction %float None %16
%18 = OpFunctionParameter %_ptr_Function_float
%19 = OpFunctionParameter %_ptr_Function_float
%20 = OpLabel
%21 = OpLoad %float %18
%22 = OpLoad %float %19
%23 = OpFDiv %float %21 %22
OpReturnValue %23
OpFunctionEnd
%_soft_light_component = OpFunction %float None %25
%27 = OpFunctionParameter %_ptr_Function_v2float
%28 = OpFunctionParameter %_ptr_Function_v2float
%29 = OpLabel
%_8_n = OpVariable %_ptr_Function_float Function
%DSqd = OpVariable %_ptr_Function_float Function
%DCub = OpVariable %_ptr_Function_float Function
%DaSqd = OpVariable %_ptr_Function_float Function
%DaCub = OpVariable %_ptr_Function_float Function
%_10_n = OpVariable %_ptr_Function_float Function
%31 = OpLoad %v2float %27
%32 = OpCompositeExtract %float %31 0
%33 = OpFMul %float %float_2 %32
%34 = OpLoad %v2float %27
%35 = OpCompositeExtract %float %34 1
%36 = OpFOrdLessThanEqual %bool %33 %35
OpSelectionMerge %39 None
OpBranchConditional %36 %37 %38
%37 = OpLabel
%41 = OpLoad %v2float %28
%42 = OpCompositeExtract %float %41 0
%43 = OpLoad %v2float %28
%44 = OpCompositeExtract %float %43 0
%45 = OpFMul %float %42 %44
%46 = OpLoad %v2float %27
%47 = OpCompositeExtract %float %46 1
%48 = OpLoad %v2float %27
%49 = OpCompositeExtract %float %48 0
%50 = OpFMul %float %float_2 %49
%51 = OpFSub %float %47 %50
%52 = OpFMul %float %45 %51
OpStore %_8_n %52
%53 = OpLoad %float %_8_n
%54 = OpLoad %v2float %28
%55 = OpCompositeExtract %float %54 1
%56 = OpFDiv %float %53 %55
%58 = OpLoad %v2float %28
%59 = OpCompositeExtract %float %58 1
%60 = OpFSub %float %float_1 %59
%61 = OpLoad %v2float %27
%62 = OpCompositeExtract %float %61 0
%63 = OpFMul %float %60 %62
%64 = OpFAdd %float %56 %63
%65 = OpLoad %v2float %28
%66 = OpCompositeExtract %float %65 0
%68 = OpLoad %v2float %27
%69 = OpCompositeExtract %float %68 1
%67 = OpFNegate %float %69
%70 = OpLoad %v2float %27
%71 = OpCompositeExtract %float %70 0
%72 = OpFMul %float %float_2 %71
%73 = OpFAdd %float %67 %72
%74 = OpFAdd %float %73 %float_1
%75 = OpFMul %float %66 %74
%76 = OpFAdd %float %64 %75
OpReturnValue %76
%38 = OpLabel
%78 = OpLoad %v2float %28
%79 = OpCompositeExtract %float %78 0
%80 = OpFMul %float %float_4 %79
%81 = OpLoad %v2float %28
%82 = OpCompositeExtract %float %81 1
%83 = OpFOrdLessThanEqual %bool %80 %82
OpSelectionMerge %86 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%88 = OpLoad %v2float %28
%89 = OpCompositeExtract %float %88 0
%90 = OpLoad %v2float %28
%91 = OpCompositeExtract %float %90 0
%92 = OpFMul %float %89 %91
OpStore %DSqd %92
%94 = OpLoad %float %DSqd
%95 = OpLoad %v2float %28
%96 = OpCompositeExtract %float %95 0
%97 = OpFMul %float %94 %96
OpStore %DCub %97
%99 = OpLoad %v2float %28
%100 = OpCompositeExtract %float %99 1
%101 = OpLoad %v2float %28
%102 = OpCompositeExtract %float %101 1
%103 = OpFMul %float %100 %102
OpStore %DaSqd %103
%105 = OpLoad %float %DaSqd
%106 = OpLoad %v2float %28
%107 = OpCompositeExtract %float %106 1
%108 = OpFMul %float %105 %107
OpStore %DaCub %108
%110 = OpLoad %float %DaSqd
%111 = OpLoad %v2float %27
%112 = OpCompositeExtract %float %111 0
%113 = OpLoad %v2float %28
%114 = OpCompositeExtract %float %113 0
%116 = OpLoad %v2float %27
%117 = OpCompositeExtract %float %116 1
%118 = OpFMul %float %float_3 %117
%120 = OpLoad %v2float %27
%121 = OpCompositeExtract %float %120 0
%122 = OpFMul %float %float_6 %121
%123 = OpFSub %float %118 %122
%124 = OpFSub %float %123 %float_1
%125 = OpFMul %float %114 %124
%126 = OpFSub %float %112 %125
%127 = OpFMul %float %110 %126
%129 = OpLoad %v2float %28
%130 = OpCompositeExtract %float %129 1
%131 = OpFMul %float %float_12 %130
%132 = OpLoad %float %DSqd
%133 = OpFMul %float %131 %132
%134 = OpLoad %v2float %27
%135 = OpCompositeExtract %float %134 1
%136 = OpLoad %v2float %27
%137 = OpCompositeExtract %float %136 0
%138 = OpFMul %float %float_2 %137
%139 = OpFSub %float %135 %138
%140 = OpFMul %float %133 %139
%141 = OpFAdd %float %127 %140
%143 = OpLoad %float %DCub
%144 = OpFMul %float %float_16 %143
%145 = OpLoad %v2float %27
%146 = OpCompositeExtract %float %145 1
%147 = OpLoad %v2float %27
%148 = OpCompositeExtract %float %147 0
%149 = OpFMul %float %float_2 %148
%150 = OpFSub %float %146 %149
%151 = OpFMul %float %144 %150
%152 = OpFSub %float %141 %151
%153 = OpLoad %float %DaCub
%154 = OpLoad %v2float %27
%155 = OpCompositeExtract %float %154 0
%156 = OpFMul %float %153 %155
%157 = OpFSub %float %152 %156
OpStore %_10_n %157
%158 = OpLoad %float %_10_n
%159 = OpLoad %float %DaSqd
%160 = OpFDiv %float %158 %159
OpReturnValue %160
%85 = OpLabel
%161 = OpLoad %v2float %28
%162 = OpCompositeExtract %float %161 0
%163 = OpLoad %v2float %27
%164 = OpCompositeExtract %float %163 1
%165 = OpLoad %v2float %27
%166 = OpCompositeExtract %float %165 0
%167 = OpFMul %float %float_2 %166
%168 = OpFSub %float %164 %167
%169 = OpFAdd %float %168 %float_1
%170 = OpFMul %float %162 %169
%171 = OpLoad %v2float %27
%172 = OpCompositeExtract %float %171 0
%173 = OpFAdd %float %170 %172
%175 = OpLoad %v2float %28
%176 = OpCompositeExtract %float %175 1
%177 = OpLoad %v2float %28
%178 = OpCompositeExtract %float %177 0
%179 = OpFMul %float %176 %178
%174 = OpExtInst %float %1 Sqrt %179
%180 = OpLoad %v2float %27
%181 = OpCompositeExtract %float %180 1
%182 = OpLoad %v2float %27
%183 = OpCompositeExtract %float %182 0
%184 = OpFMul %float %float_2 %183
%185 = OpFSub %float %181 %184
%186 = OpFMul %float %174 %185
%187 = OpFSub %float %173 %186
%188 = OpLoad %v2float %28
%189 = OpCompositeExtract %float %188 1
%190 = OpLoad %v2float %27
%191 = OpCompositeExtract %float %190 0
%192 = OpFMul %float %189 %191
%193 = OpFSub %float %187 %192
OpReturnValue %193
%86 = OpLabel
OpBranch %39
%39 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_soft_light = OpFunction %v4float None %194
%196 = OpFunctionParameter %_ptr_Function_v4float
%197 = OpFunctionParameter %_ptr_Function_v4float
%198 = OpLabel
%203 = OpVariable %_ptr_Function_v4float Function
%210 = OpVariable %_ptr_Function_v2float Function
%213 = OpVariable %_ptr_Function_v2float Function
%217 = OpVariable %_ptr_Function_v2float Function
%220 = OpVariable %_ptr_Function_v2float Function
%224 = OpVariable %_ptr_Function_v2float Function
%227 = OpVariable %_ptr_Function_v2float Function
%199 = OpLoad %v4float %197
%200 = OpCompositeExtract %float %199 3
%202 = OpFOrdEqual %bool %200 %float_0
OpSelectionMerge %206 None
OpBranchConditional %202 %204 %205
%204 = OpLabel
%207 = OpLoad %v4float %196
OpStore %203 %207
OpBranch %206
%205 = OpLabel
%208 = OpLoad %v4float %196
%209 = OpVectorShuffle %v2float %208 %208 0 3
OpStore %210 %209
%211 = OpLoad %v4float %197
%212 = OpVectorShuffle %v2float %211 %211 0 3
OpStore %213 %212
%214 = OpFunctionCall %float %_soft_light_component %210 %213
%215 = OpLoad %v4float %196
%216 = OpVectorShuffle %v2float %215 %215 1 3
OpStore %217 %216
%218 = OpLoad %v4float %197
%219 = OpVectorShuffle %v2float %218 %218 1 3
OpStore %220 %219
%221 = OpFunctionCall %float %_soft_light_component %217 %220
%222 = OpLoad %v4float %196
%223 = OpVectorShuffle %v2float %222 %222 2 3
OpStore %224 %223
%225 = OpLoad %v4float %197
%226 = OpVectorShuffle %v2float %225 %225 2 3
OpStore %227 %226
%228 = OpFunctionCall %float %_soft_light_component %224 %227
%229 = OpLoad %v4float %196
%230 = OpCompositeExtract %float %229 3
%231 = OpLoad %v4float %196
%232 = OpCompositeExtract %float %231 3
%233 = OpFSub %float %float_1 %232
%234 = OpLoad %v4float %197
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
%244 = OpVariable %_ptr_Function_v4float Function
%246 = OpVariable %_ptr_Function_v4float Function
%243 = OpLoad %v4float %src
OpStore %244 %243
%245 = OpLoad %v4float %dst
OpStore %246 %245
%247 = OpFunctionCall %v4float %blend_soft_light %244 %246
OpStore %sk_FragColor %247
OpReturn
OpFunctionEnd
