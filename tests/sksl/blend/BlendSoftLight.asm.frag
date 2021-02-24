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
OpName %_8_n "_8_n"
OpName %DSqd "DSqd"
OpName %DCub "DCub"
OpName %DaSqd "DaSqd"
OpName %DaCub "DaCub"
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
OpDecorate %32 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
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
%186 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0 = OpConstant %float 0
%_soft_light_component = OpFunction %float None %15
%17 = OpFunctionParameter %_ptr_Function_v2float
%18 = OpFunctionParameter %_ptr_Function_v2float
%19 = OpLabel
%_8_n = OpVariable %_ptr_Function_float Function
%DSqd = OpVariable %_ptr_Function_float Function
%DCub = OpVariable %_ptr_Function_float Function
%DaSqd = OpVariable %_ptr_Function_float Function
%DaCub = OpVariable %_ptr_Function_float Function
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
%32 = OpLoad %v2float %18
%33 = OpCompositeExtract %float %32 0
%34 = OpLoad %v2float %18
%35 = OpCompositeExtract %float %34 0
%36 = OpFMul %float %33 %35
%37 = OpLoad %v2float %17
%38 = OpCompositeExtract %float %37 1
%39 = OpLoad %v2float %17
%40 = OpCompositeExtract %float %39 0
%41 = OpFMul %float %float_2 %40
%42 = OpFSub %float %38 %41
%43 = OpFMul %float %36 %42
OpStore %_8_n %43
%44 = OpLoad %float %_8_n
%45 = OpLoad %v2float %18
%46 = OpCompositeExtract %float %45 1
%47 = OpFDiv %float %44 %46
%49 = OpLoad %v2float %18
%50 = OpCompositeExtract %float %49 1
%51 = OpFSub %float %float_1 %50
%52 = OpLoad %v2float %17
%53 = OpCompositeExtract %float %52 0
%54 = OpFMul %float %51 %53
%55 = OpFAdd %float %47 %54
%56 = OpLoad %v2float %18
%57 = OpCompositeExtract %float %56 0
%59 = OpLoad %v2float %17
%60 = OpCompositeExtract %float %59 1
%58 = OpFNegate %float %60
%61 = OpLoad %v2float %17
%62 = OpCompositeExtract %float %61 0
%63 = OpFMul %float %float_2 %62
%64 = OpFAdd %float %58 %63
%65 = OpFAdd %float %64 %float_1
%66 = OpFMul %float %57 %65
%67 = OpFAdd %float %55 %66
OpReturnValue %67
%28 = OpLabel
%69 = OpLoad %v2float %18
%70 = OpCompositeExtract %float %69 0
%71 = OpFMul %float %float_4 %70
%72 = OpLoad %v2float %18
%73 = OpCompositeExtract %float %72 1
%74 = OpFOrdLessThanEqual %bool %71 %73
OpSelectionMerge %77 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%79 = OpLoad %v2float %18
%80 = OpCompositeExtract %float %79 0
%81 = OpLoad %v2float %18
%82 = OpCompositeExtract %float %81 0
%83 = OpFMul %float %80 %82
OpStore %DSqd %83
%85 = OpLoad %float %DSqd
%86 = OpLoad %v2float %18
%87 = OpCompositeExtract %float %86 0
%88 = OpFMul %float %85 %87
OpStore %DCub %88
%90 = OpLoad %v2float %18
%91 = OpCompositeExtract %float %90 1
%92 = OpLoad %v2float %18
%93 = OpCompositeExtract %float %92 1
%94 = OpFMul %float %91 %93
OpStore %DaSqd %94
%96 = OpLoad %float %DaSqd
%97 = OpLoad %v2float %18
%98 = OpCompositeExtract %float %97 1
%99 = OpFMul %float %96 %98
OpStore %DaCub %99
%101 = OpLoad %float %DaSqd
%102 = OpLoad %v2float %17
%103 = OpCompositeExtract %float %102 0
%104 = OpLoad %v2float %18
%105 = OpCompositeExtract %float %104 0
%107 = OpLoad %v2float %17
%108 = OpCompositeExtract %float %107 1
%109 = OpFMul %float %float_3 %108
%111 = OpLoad %v2float %17
%112 = OpCompositeExtract %float %111 0
%113 = OpFMul %float %float_6 %112
%114 = OpFSub %float %109 %113
%115 = OpFSub %float %114 %float_1
%116 = OpFMul %float %105 %115
%117 = OpFSub %float %103 %116
%118 = OpFMul %float %101 %117
%120 = OpLoad %v2float %18
%121 = OpCompositeExtract %float %120 1
%122 = OpFMul %float %float_12 %121
%123 = OpLoad %float %DSqd
%124 = OpFMul %float %122 %123
%125 = OpLoad %v2float %17
%126 = OpCompositeExtract %float %125 1
%127 = OpLoad %v2float %17
%128 = OpCompositeExtract %float %127 0
%129 = OpFMul %float %float_2 %128
%130 = OpFSub %float %126 %129
%131 = OpFMul %float %124 %130
%132 = OpFAdd %float %118 %131
%134 = OpLoad %float %DCub
%135 = OpFMul %float %float_16 %134
%136 = OpLoad %v2float %17
%137 = OpCompositeExtract %float %136 1
%138 = OpLoad %v2float %17
%139 = OpCompositeExtract %float %138 0
%140 = OpFMul %float %float_2 %139
%141 = OpFSub %float %137 %140
%142 = OpFMul %float %135 %141
%143 = OpFSub %float %132 %142
%144 = OpLoad %float %DaCub
%145 = OpLoad %v2float %17
%146 = OpCompositeExtract %float %145 0
%147 = OpFMul %float %144 %146
%148 = OpFSub %float %143 %147
OpStore %_10_n %148
%149 = OpLoad %float %_10_n
%150 = OpLoad %float %DaSqd
%151 = OpFDiv %float %149 %150
OpReturnValue %151
%76 = OpLabel
%152 = OpLoad %v2float %18
%153 = OpCompositeExtract %float %152 0
%154 = OpLoad %v2float %17
%155 = OpCompositeExtract %float %154 1
%156 = OpLoad %v2float %17
%157 = OpCompositeExtract %float %156 0
%158 = OpFMul %float %float_2 %157
%159 = OpFSub %float %155 %158
%160 = OpFAdd %float %159 %float_1
%161 = OpFMul %float %153 %160
%162 = OpLoad %v2float %17
%163 = OpCompositeExtract %float %162 0
%164 = OpFAdd %float %161 %163
%166 = OpLoad %v2float %18
%167 = OpCompositeExtract %float %166 1
%168 = OpLoad %v2float %18
%169 = OpCompositeExtract %float %168 0
%170 = OpFMul %float %167 %169
%165 = OpExtInst %float %1 Sqrt %170
%171 = OpLoad %v2float %17
%172 = OpCompositeExtract %float %171 1
%173 = OpLoad %v2float %17
%174 = OpCompositeExtract %float %173 0
%175 = OpFMul %float %float_2 %174
%176 = OpFSub %float %172 %175
%177 = OpFMul %float %165 %176
%178 = OpFSub %float %164 %177
%179 = OpLoad %v2float %18
%180 = OpCompositeExtract %float %179 1
%181 = OpLoad %v2float %17
%182 = OpCompositeExtract %float %181 0
%183 = OpFMul %float %180 %182
%184 = OpFSub %float %178 %183
OpReturnValue %184
%77 = OpLabel
OpBranch %29
%29 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %186
%187 = OpLabel
%_0_blend_soft_light = OpVariable %_ptr_Function_v4float Function
%194 = OpVariable %_ptr_Function_v4float Function
%201 = OpVariable %_ptr_Function_v2float Function
%204 = OpVariable %_ptr_Function_v2float Function
%208 = OpVariable %_ptr_Function_v2float Function
%211 = OpVariable %_ptr_Function_v2float Function
%215 = OpVariable %_ptr_Function_v2float Function
%218 = OpVariable %_ptr_Function_v2float Function
%190 = OpLoad %v4float %dst
%191 = OpCompositeExtract %float %190 3
%193 = OpFOrdEqual %bool %191 %float_0
OpSelectionMerge %197 None
OpBranchConditional %193 %195 %196
%195 = OpLabel
%198 = OpLoad %v4float %src
OpStore %194 %198
OpBranch %197
%196 = OpLabel
%199 = OpLoad %v4float %src
%200 = OpVectorShuffle %v2float %199 %199 0 3
OpStore %201 %200
%202 = OpLoad %v4float %dst
%203 = OpVectorShuffle %v2float %202 %202 0 3
OpStore %204 %203
%205 = OpFunctionCall %float %_soft_light_component %201 %204
%206 = OpLoad %v4float %src
%207 = OpVectorShuffle %v2float %206 %206 1 3
OpStore %208 %207
%209 = OpLoad %v4float %dst
%210 = OpVectorShuffle %v2float %209 %209 1 3
OpStore %211 %210
%212 = OpFunctionCall %float %_soft_light_component %208 %211
%213 = OpLoad %v4float %src
%214 = OpVectorShuffle %v2float %213 %213 2 3
OpStore %215 %214
%216 = OpLoad %v4float %dst
%217 = OpVectorShuffle %v2float %216 %216 2 3
OpStore %218 %217
%219 = OpFunctionCall %float %_soft_light_component %215 %218
%220 = OpLoad %v4float %src
%221 = OpCompositeExtract %float %220 3
%222 = OpLoad %v4float %src
%223 = OpCompositeExtract %float %222 3
%224 = OpFSub %float %float_1 %223
%225 = OpLoad %v4float %dst
%226 = OpCompositeExtract %float %225 3
%227 = OpFMul %float %224 %226
%228 = OpFAdd %float %221 %227
%229 = OpCompositeConstruct %v4float %205 %212 %219 %228
OpStore %194 %229
OpBranch %197
%197 = OpLabel
%230 = OpLoad %v4float %194
OpStore %sk_FragColor %230
OpReturn
OpFunctionEnd
