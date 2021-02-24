OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %_soft_light_component "_soft_light_component"
OpName %_7_guarded_divide "_7_guarded_divide"
OpName %_8_n "_8_n"
OpName %DSqd "DSqd"
OpName %DCub "DCub"
OpName %DaSqd "DaSqd"
OpName %DaCub "DaCub"
OpName %_9_guarded_divide "_9_guarded_divide"
OpName %_10_n "_10_n"
OpName %main "main"
OpName %_0_blend_soft_light "_0_blend_soft_light"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
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
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%15 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_2 = OpConstant %float 2
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_4 = OpConstant %float 4
%float_3 = OpConstant %float 3
%float_6 = OpConstant %float 6
%float_12 = OpConstant %float 12
%float_16 = OpConstant %float 16
%void = OpTypeVoid
%188 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0 = OpConstant %float 0
%_soft_light_component = OpFunction %float None %15
%17 = OpFunctionParameter %_ptr_Function_v2float
%18 = OpFunctionParameter %_ptr_Function_v2float
%19 = OpLabel
%_7_guarded_divide = OpVariable %_ptr_Function_float Function
%_8_n = OpVariable %_ptr_Function_float Function
%DSqd = OpVariable %_ptr_Function_float Function
%DCub = OpVariable %_ptr_Function_float Function
%DaSqd = OpVariable %_ptr_Function_float Function
%DaCub = OpVariable %_ptr_Function_float Function
%_9_guarded_divide = OpVariable %_ptr_Function_float Function
%_10_n = OpVariable %_ptr_Function_float Function
%21 = OpLoad %v2float %17
%22 = OpCompositeExtract %float %21 0
%23 = OpFMul %float %float_2 %22
%24 = OpLoad %v2float %17
%25 = OpCompositeExtract %float %24 1
%26 = OpFOrdLessThanEqual %bool %23 %25
OpSelectionMerge %29 None
OpBranchConditional %26 %27 %28
%27 = OpLabel
%33 = OpLoad %v2float %18
%34 = OpCompositeExtract %float %33 0
%35 = OpLoad %v2float %18
%36 = OpCompositeExtract %float %35 0
%37 = OpFMul %float %34 %36
%38 = OpLoad %v2float %17
%39 = OpCompositeExtract %float %38 1
%40 = OpLoad %v2float %17
%41 = OpCompositeExtract %float %40 0
%42 = OpFMul %float %float_2 %41
%43 = OpFSub %float %39 %42
%44 = OpFMul %float %37 %43
OpStore %_8_n %44
%45 = OpLoad %float %_8_n
%46 = OpLoad %v2float %18
%47 = OpCompositeExtract %float %46 1
%48 = OpFDiv %float %45 %47
%50 = OpLoad %v2float %18
%51 = OpCompositeExtract %float %50 1
%52 = OpFSub %float %float_1 %51
%53 = OpLoad %v2float %17
%54 = OpCompositeExtract %float %53 0
%55 = OpFMul %float %52 %54
%56 = OpFAdd %float %48 %55
%57 = OpLoad %v2float %18
%58 = OpCompositeExtract %float %57 0
%60 = OpLoad %v2float %17
%61 = OpCompositeExtract %float %60 1
%59 = OpFNegate %float %61
%62 = OpLoad %v2float %17
%63 = OpCompositeExtract %float %62 0
%64 = OpFMul %float %float_2 %63
%65 = OpFAdd %float %59 %64
%66 = OpFAdd %float %65 %float_1
%67 = OpFMul %float %58 %66
%68 = OpFAdd %float %56 %67
OpReturnValue %68
%28 = OpLabel
%70 = OpLoad %v2float %18
%71 = OpCompositeExtract %float %70 0
%72 = OpFMul %float %float_4 %71
%73 = OpLoad %v2float %18
%74 = OpCompositeExtract %float %73 1
%75 = OpFOrdLessThanEqual %bool %72 %74
OpSelectionMerge %78 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%80 = OpLoad %v2float %18
%81 = OpCompositeExtract %float %80 0
%82 = OpLoad %v2float %18
%83 = OpCompositeExtract %float %82 0
%84 = OpFMul %float %81 %83
OpStore %DSqd %84
%86 = OpLoad %float %DSqd
%87 = OpLoad %v2float %18
%88 = OpCompositeExtract %float %87 0
%89 = OpFMul %float %86 %88
OpStore %DCub %89
%91 = OpLoad %v2float %18
%92 = OpCompositeExtract %float %91 1
%93 = OpLoad %v2float %18
%94 = OpCompositeExtract %float %93 1
%95 = OpFMul %float %92 %94
OpStore %DaSqd %95
%97 = OpLoad %float %DaSqd
%98 = OpLoad %v2float %18
%99 = OpCompositeExtract %float %98 1
%100 = OpFMul %float %97 %99
OpStore %DaCub %100
%103 = OpLoad %float %DaSqd
%104 = OpLoad %v2float %17
%105 = OpCompositeExtract %float %104 0
%106 = OpLoad %v2float %18
%107 = OpCompositeExtract %float %106 0
%109 = OpLoad %v2float %17
%110 = OpCompositeExtract %float %109 1
%111 = OpFMul %float %float_3 %110
%113 = OpLoad %v2float %17
%114 = OpCompositeExtract %float %113 0
%115 = OpFMul %float %float_6 %114
%116 = OpFSub %float %111 %115
%117 = OpFSub %float %116 %float_1
%118 = OpFMul %float %107 %117
%119 = OpFSub %float %105 %118
%120 = OpFMul %float %103 %119
%122 = OpLoad %v2float %18
%123 = OpCompositeExtract %float %122 1
%124 = OpFMul %float %float_12 %123
%125 = OpLoad %float %DSqd
%126 = OpFMul %float %124 %125
%127 = OpLoad %v2float %17
%128 = OpCompositeExtract %float %127 1
%129 = OpLoad %v2float %17
%130 = OpCompositeExtract %float %129 0
%131 = OpFMul %float %float_2 %130
%132 = OpFSub %float %128 %131
%133 = OpFMul %float %126 %132
%134 = OpFAdd %float %120 %133
%136 = OpLoad %float %DCub
%137 = OpFMul %float %float_16 %136
%138 = OpLoad %v2float %17
%139 = OpCompositeExtract %float %138 1
%140 = OpLoad %v2float %17
%141 = OpCompositeExtract %float %140 0
%142 = OpFMul %float %float_2 %141
%143 = OpFSub %float %139 %142
%144 = OpFMul %float %137 %143
%145 = OpFSub %float %134 %144
%146 = OpLoad %float %DaCub
%147 = OpLoad %v2float %17
%148 = OpCompositeExtract %float %147 0
%149 = OpFMul %float %146 %148
%150 = OpFSub %float %145 %149
OpStore %_10_n %150
%151 = OpLoad %float %_10_n
%152 = OpLoad %float %DaSqd
%153 = OpFDiv %float %151 %152
OpReturnValue %153
%77 = OpLabel
%154 = OpLoad %v2float %18
%155 = OpCompositeExtract %float %154 0
%156 = OpLoad %v2float %17
%157 = OpCompositeExtract %float %156 1
%158 = OpLoad %v2float %17
%159 = OpCompositeExtract %float %158 0
%160 = OpFMul %float %float_2 %159
%161 = OpFSub %float %157 %160
%162 = OpFAdd %float %161 %float_1
%163 = OpFMul %float %155 %162
%164 = OpLoad %v2float %17
%165 = OpCompositeExtract %float %164 0
%166 = OpFAdd %float %163 %165
%168 = OpLoad %v2float %18
%169 = OpCompositeExtract %float %168 1
%170 = OpLoad %v2float %18
%171 = OpCompositeExtract %float %170 0
%172 = OpFMul %float %169 %171
%167 = OpExtInst %float %1 Sqrt %172
%173 = OpLoad %v2float %17
%174 = OpCompositeExtract %float %173 1
%175 = OpLoad %v2float %17
%176 = OpCompositeExtract %float %175 0
%177 = OpFMul %float %float_2 %176
%178 = OpFSub %float %174 %177
%179 = OpFMul %float %167 %178
%180 = OpFSub %float %166 %179
%181 = OpLoad %v2float %18
%182 = OpCompositeExtract %float %181 1
%183 = OpLoad %v2float %17
%184 = OpCompositeExtract %float %183 0
%185 = OpFMul %float %182 %184
%186 = OpFSub %float %180 %185
OpReturnValue %186
%78 = OpLabel
OpBranch %29
%29 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %188
%189 = OpLabel
%_0_blend_soft_light = OpVariable %_ptr_Function_v4float Function
%196 = OpVariable %_ptr_Function_v4float Function
%203 = OpVariable %_ptr_Function_v2float Function
%206 = OpVariable %_ptr_Function_v2float Function
%210 = OpVariable %_ptr_Function_v2float Function
%213 = OpVariable %_ptr_Function_v2float Function
%217 = OpVariable %_ptr_Function_v2float Function
%220 = OpVariable %_ptr_Function_v2float Function
%192 = OpLoad %v4float %dst
%193 = OpCompositeExtract %float %192 3
%195 = OpFOrdEqual %bool %193 %float_0
OpSelectionMerge %199 None
OpBranchConditional %195 %197 %198
%197 = OpLabel
%200 = OpLoad %v4float %src
OpStore %196 %200
OpBranch %199
%198 = OpLabel
%201 = OpLoad %v4float %src
%202 = OpVectorShuffle %v2float %201 %201 0 3
OpStore %203 %202
%204 = OpLoad %v4float %dst
%205 = OpVectorShuffle %v2float %204 %204 0 3
OpStore %206 %205
%207 = OpFunctionCall %float %_soft_light_component %203 %206
%208 = OpLoad %v4float %src
%209 = OpVectorShuffle %v2float %208 %208 1 3
OpStore %210 %209
%211 = OpLoad %v4float %dst
%212 = OpVectorShuffle %v2float %211 %211 1 3
OpStore %213 %212
%214 = OpFunctionCall %float %_soft_light_component %210 %213
%215 = OpLoad %v4float %src
%216 = OpVectorShuffle %v2float %215 %215 2 3
OpStore %217 %216
%218 = OpLoad %v4float %dst
%219 = OpVectorShuffle %v2float %218 %218 2 3
OpStore %220 %219
%221 = OpFunctionCall %float %_soft_light_component %217 %220
%222 = OpLoad %v4float %src
%223 = OpCompositeExtract %float %222 3
%224 = OpLoad %v4float %src
%225 = OpCompositeExtract %float %224 3
%226 = OpFSub %float %float_1 %225
%227 = OpLoad %v4float %dst
%228 = OpCompositeExtract %float %227 3
%229 = OpFMul %float %226 %228
%230 = OpFAdd %float %223 %229
%231 = OpCompositeConstruct %v4float %207 %214 %221 %230
OpStore %196 %231
OpBranch %199
%199 = OpLabel
%232 = OpLoad %v4float %196
OpStore %sk_FragColor %232
OpReturn
OpFunctionEnd
