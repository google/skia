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
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
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
%190 = OpTypeFunction %void
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
OpStore %_7_guarded_divide %48
%49 = OpLoad %float %_7_guarded_divide
%51 = OpLoad %v2float %18
%52 = OpCompositeExtract %float %51 1
%53 = OpFSub %float %float_1 %52
%54 = OpLoad %v2float %17
%55 = OpCompositeExtract %float %54 0
%56 = OpFMul %float %53 %55
%57 = OpFAdd %float %49 %56
%58 = OpLoad %v2float %18
%59 = OpCompositeExtract %float %58 0
%61 = OpLoad %v2float %17
%62 = OpCompositeExtract %float %61 1
%60 = OpFNegate %float %62
%63 = OpLoad %v2float %17
%64 = OpCompositeExtract %float %63 0
%65 = OpFMul %float %float_2 %64
%66 = OpFAdd %float %60 %65
%67 = OpFAdd %float %66 %float_1
%68 = OpFMul %float %59 %67
%69 = OpFAdd %float %57 %68
OpReturnValue %69
%28 = OpLabel
%71 = OpLoad %v2float %18
%72 = OpCompositeExtract %float %71 0
%73 = OpFMul %float %float_4 %72
%74 = OpLoad %v2float %18
%75 = OpCompositeExtract %float %74 1
%76 = OpFOrdLessThanEqual %bool %73 %75
OpSelectionMerge %79 None
OpBranchConditional %76 %77 %78
%77 = OpLabel
%81 = OpLoad %v2float %18
%82 = OpCompositeExtract %float %81 0
%83 = OpLoad %v2float %18
%84 = OpCompositeExtract %float %83 0
%85 = OpFMul %float %82 %84
OpStore %DSqd %85
%87 = OpLoad %float %DSqd
%88 = OpLoad %v2float %18
%89 = OpCompositeExtract %float %88 0
%90 = OpFMul %float %87 %89
OpStore %DCub %90
%92 = OpLoad %v2float %18
%93 = OpCompositeExtract %float %92 1
%94 = OpLoad %v2float %18
%95 = OpCompositeExtract %float %94 1
%96 = OpFMul %float %93 %95
OpStore %DaSqd %96
%98 = OpLoad %float %DaSqd
%99 = OpLoad %v2float %18
%100 = OpCompositeExtract %float %99 1
%101 = OpFMul %float %98 %100
OpStore %DaCub %101
%104 = OpLoad %float %DaSqd
%105 = OpLoad %v2float %17
%106 = OpCompositeExtract %float %105 0
%107 = OpLoad %v2float %18
%108 = OpCompositeExtract %float %107 0
%110 = OpLoad %v2float %17
%111 = OpCompositeExtract %float %110 1
%112 = OpFMul %float %float_3 %111
%114 = OpLoad %v2float %17
%115 = OpCompositeExtract %float %114 0
%116 = OpFMul %float %float_6 %115
%117 = OpFSub %float %112 %116
%118 = OpFSub %float %117 %float_1
%119 = OpFMul %float %108 %118
%120 = OpFSub %float %106 %119
%121 = OpFMul %float %104 %120
%123 = OpLoad %v2float %18
%124 = OpCompositeExtract %float %123 1
%125 = OpFMul %float %float_12 %124
%126 = OpLoad %float %DSqd
%127 = OpFMul %float %125 %126
%128 = OpLoad %v2float %17
%129 = OpCompositeExtract %float %128 1
%130 = OpLoad %v2float %17
%131 = OpCompositeExtract %float %130 0
%132 = OpFMul %float %float_2 %131
%133 = OpFSub %float %129 %132
%134 = OpFMul %float %127 %133
%135 = OpFAdd %float %121 %134
%137 = OpLoad %float %DCub
%138 = OpFMul %float %float_16 %137
%139 = OpLoad %v2float %17
%140 = OpCompositeExtract %float %139 1
%141 = OpLoad %v2float %17
%142 = OpCompositeExtract %float %141 0
%143 = OpFMul %float %float_2 %142
%144 = OpFSub %float %140 %143
%145 = OpFMul %float %138 %144
%146 = OpFSub %float %135 %145
%147 = OpLoad %float %DaCub
%148 = OpLoad %v2float %17
%149 = OpCompositeExtract %float %148 0
%150 = OpFMul %float %147 %149
%151 = OpFSub %float %146 %150
OpStore %_10_n %151
%152 = OpLoad %float %_10_n
%153 = OpLoad %float %DaSqd
%154 = OpFDiv %float %152 %153
OpStore %_9_guarded_divide %154
%155 = OpLoad %float %_9_guarded_divide
OpReturnValue %155
%78 = OpLabel
%156 = OpLoad %v2float %18
%157 = OpCompositeExtract %float %156 0
%158 = OpLoad %v2float %17
%159 = OpCompositeExtract %float %158 1
%160 = OpLoad %v2float %17
%161 = OpCompositeExtract %float %160 0
%162 = OpFMul %float %float_2 %161
%163 = OpFSub %float %159 %162
%164 = OpFAdd %float %163 %float_1
%165 = OpFMul %float %157 %164
%166 = OpLoad %v2float %17
%167 = OpCompositeExtract %float %166 0
%168 = OpFAdd %float %165 %167
%170 = OpLoad %v2float %18
%171 = OpCompositeExtract %float %170 1
%172 = OpLoad %v2float %18
%173 = OpCompositeExtract %float %172 0
%174 = OpFMul %float %171 %173
%169 = OpExtInst %float %1 Sqrt %174
%175 = OpLoad %v2float %17
%176 = OpCompositeExtract %float %175 1
%177 = OpLoad %v2float %17
%178 = OpCompositeExtract %float %177 0
%179 = OpFMul %float %float_2 %178
%180 = OpFSub %float %176 %179
%181 = OpFMul %float %169 %180
%182 = OpFSub %float %168 %181
%183 = OpLoad %v2float %18
%184 = OpCompositeExtract %float %183 1
%185 = OpLoad %v2float %17
%186 = OpCompositeExtract %float %185 0
%187 = OpFMul %float %184 %186
%188 = OpFSub %float %182 %187
OpReturnValue %188
%79 = OpLabel
OpBranch %29
%29 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %190
%191 = OpLabel
%_0_blend_soft_light = OpVariable %_ptr_Function_v4float Function
%198 = OpVariable %_ptr_Function_v4float Function
%205 = OpVariable %_ptr_Function_v2float Function
%208 = OpVariable %_ptr_Function_v2float Function
%212 = OpVariable %_ptr_Function_v2float Function
%215 = OpVariable %_ptr_Function_v2float Function
%219 = OpVariable %_ptr_Function_v2float Function
%222 = OpVariable %_ptr_Function_v2float Function
%194 = OpLoad %v4float %dst
%195 = OpCompositeExtract %float %194 3
%197 = OpFOrdEqual %bool %195 %float_0
OpSelectionMerge %201 None
OpBranchConditional %197 %199 %200
%199 = OpLabel
%202 = OpLoad %v4float %src
OpStore %198 %202
OpBranch %201
%200 = OpLabel
%203 = OpLoad %v4float %src
%204 = OpVectorShuffle %v2float %203 %203 0 3
OpStore %205 %204
%206 = OpLoad %v4float %dst
%207 = OpVectorShuffle %v2float %206 %206 0 3
OpStore %208 %207
%209 = OpFunctionCall %float %_soft_light_component %205 %208
%210 = OpLoad %v4float %src
%211 = OpVectorShuffle %v2float %210 %210 1 3
OpStore %212 %211
%213 = OpLoad %v4float %dst
%214 = OpVectorShuffle %v2float %213 %213 1 3
OpStore %215 %214
%216 = OpFunctionCall %float %_soft_light_component %212 %215
%217 = OpLoad %v4float %src
%218 = OpVectorShuffle %v2float %217 %217 2 3
OpStore %219 %218
%220 = OpLoad %v4float %dst
%221 = OpVectorShuffle %v2float %220 %220 2 3
OpStore %222 %221
%223 = OpFunctionCall %float %_soft_light_component %219 %222
%224 = OpLoad %v4float %src
%225 = OpCompositeExtract %float %224 3
%226 = OpLoad %v4float %src
%227 = OpCompositeExtract %float %226 3
%228 = OpFSub %float %float_1 %227
%229 = OpLoad %v4float %dst
%230 = OpCompositeExtract %float %229 3
%231 = OpFMul %float %228 %230
%232 = OpFAdd %float %225 %231
%233 = OpCompositeConstruct %v4float %209 %216 %223 %232
OpStore %198 %233
OpBranch %201
%201 = OpLabel
%234 = OpLoad %v4float %198
OpStore %_0_blend_soft_light %234
%235 = OpLoad %v4float %_0_blend_soft_light
OpStore %sk_FragColor %235
OpReturn
OpFunctionEnd
