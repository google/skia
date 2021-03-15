### Compilation failed:

error: SPIR-V validation error: Variable must be decorated with a location
  %src = OpVariable %_ptr_Input_v4float Input

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
OpName %DSqd "DSqd"
OpName %DCub "DCub"
OpName %DaSqd "DaSqd"
OpName %DaCub "DaCub"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %20 RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
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
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
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
%15 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_2 = OpConstant %float 2
%float_1 = OpConstant %float 1
%float_4 = OpConstant %float 4
%float_3 = OpConstant %float 3
%float_6 = OpConstant %float 6
%float_12 = OpConstant %float 12
%float_16 = OpConstant %float 16
%void = OpTypeVoid
%194 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_guarded_divide = OpFunction %float None %15
%17 = OpFunctionParameter %_ptr_Function_float
%18 = OpFunctionParameter %_ptr_Function_float
%19 = OpLabel
%20 = OpLoad %float %17
%21 = OpLoad %float %18
%22 = OpFDiv %float %20 %21
OpReturnValue %22
OpFunctionEnd
%_soft_light_component = OpFunction %float None %24
%26 = OpFunctionParameter %_ptr_Function_v2float
%27 = OpFunctionParameter %_ptr_Function_v2float
%28 = OpLabel
%51 = OpVariable %_ptr_Function_float Function
%54 = OpVariable %_ptr_Function_float Function
%DSqd = OpVariable %_ptr_Function_float Function
%DCub = OpVariable %_ptr_Function_float Function
%DaSqd = OpVariable %_ptr_Function_float Function
%DaCub = OpVariable %_ptr_Function_float Function
%156 = OpVariable %_ptr_Function_float Function
%158 = OpVariable %_ptr_Function_float Function
%30 = OpLoad %v2float %26
%31 = OpCompositeExtract %float %30 0
%32 = OpFMul %float %float_2 %31
%33 = OpLoad %v2float %26
%34 = OpCompositeExtract %float %33 1
%35 = OpFOrdLessThanEqual %bool %32 %34
OpSelectionMerge %38 None
OpBranchConditional %35 %36 %37
%36 = OpLabel
%39 = OpLoad %v2float %27
%40 = OpCompositeExtract %float %39 0
%41 = OpLoad %v2float %27
%42 = OpCompositeExtract %float %41 0
%43 = OpFMul %float %40 %42
%44 = OpLoad %v2float %26
%45 = OpCompositeExtract %float %44 1
%46 = OpLoad %v2float %26
%47 = OpCompositeExtract %float %46 0
%48 = OpFMul %float %float_2 %47
%49 = OpFSub %float %45 %48
%50 = OpFMul %float %43 %49
OpStore %51 %50
%52 = OpLoad %v2float %27
%53 = OpCompositeExtract %float %52 1
OpStore %54 %53
%55 = OpFunctionCall %float %_guarded_divide %51 %54
%57 = OpLoad %v2float %27
%58 = OpCompositeExtract %float %57 1
%59 = OpFSub %float %float_1 %58
%60 = OpLoad %v2float %26
%61 = OpCompositeExtract %float %60 0
%62 = OpFMul %float %59 %61
%63 = OpFAdd %float %55 %62
%64 = OpLoad %v2float %27
%65 = OpCompositeExtract %float %64 0
%67 = OpLoad %v2float %26
%68 = OpCompositeExtract %float %67 1
%66 = OpFNegate %float %68
%69 = OpLoad %v2float %26
%70 = OpCompositeExtract %float %69 0
%71 = OpFMul %float %float_2 %70
%72 = OpFAdd %float %66 %71
%73 = OpFAdd %float %72 %float_1
%74 = OpFMul %float %65 %73
%75 = OpFAdd %float %63 %74
OpReturnValue %75
%37 = OpLabel
%77 = OpLoad %v2float %27
%78 = OpCompositeExtract %float %77 0
%79 = OpFMul %float %float_4 %78
%80 = OpLoad %v2float %27
%81 = OpCompositeExtract %float %80 1
%82 = OpFOrdLessThanEqual %bool %79 %81
OpSelectionMerge %85 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%87 = OpLoad %v2float %27
%88 = OpCompositeExtract %float %87 0
%89 = OpLoad %v2float %27
%90 = OpCompositeExtract %float %89 0
%91 = OpFMul %float %88 %90
OpStore %DSqd %91
%93 = OpLoad %float %DSqd
%94 = OpLoad %v2float %27
%95 = OpCompositeExtract %float %94 0
%96 = OpFMul %float %93 %95
OpStore %DCub %96
%98 = OpLoad %v2float %27
%99 = OpCompositeExtract %float %98 1
%100 = OpLoad %v2float %27
%101 = OpCompositeExtract %float %100 1
%102 = OpFMul %float %99 %101
OpStore %DaSqd %102
%104 = OpLoad %float %DaSqd
%105 = OpLoad %v2float %27
%106 = OpCompositeExtract %float %105 1
%107 = OpFMul %float %104 %106
OpStore %DaCub %107
%108 = OpLoad %float %DaSqd
%109 = OpLoad %v2float %26
%110 = OpCompositeExtract %float %109 0
%111 = OpLoad %v2float %27
%112 = OpCompositeExtract %float %111 0
%114 = OpLoad %v2float %26
%115 = OpCompositeExtract %float %114 1
%116 = OpFMul %float %float_3 %115
%118 = OpLoad %v2float %26
%119 = OpCompositeExtract %float %118 0
%120 = OpFMul %float %float_6 %119
%121 = OpFSub %float %116 %120
%122 = OpFSub %float %121 %float_1
%123 = OpFMul %float %112 %122
%124 = OpFSub %float %110 %123
%125 = OpFMul %float %108 %124
%127 = OpLoad %v2float %27
%128 = OpCompositeExtract %float %127 1
%129 = OpFMul %float %float_12 %128
%130 = OpLoad %float %DSqd
%131 = OpFMul %float %129 %130
%132 = OpLoad %v2float %26
%133 = OpCompositeExtract %float %132 1
%134 = OpLoad %v2float %26
%135 = OpCompositeExtract %float %134 0
%136 = OpFMul %float %float_2 %135
%137 = OpFSub %float %133 %136
%138 = OpFMul %float %131 %137
%139 = OpFAdd %float %125 %138
%141 = OpLoad %float %DCub
%142 = OpFMul %float %float_16 %141
%143 = OpLoad %v2float %26
%144 = OpCompositeExtract %float %143 1
%145 = OpLoad %v2float %26
%146 = OpCompositeExtract %float %145 0
%147 = OpFMul %float %float_2 %146
%148 = OpFSub %float %144 %147
%149 = OpFMul %float %142 %148
%150 = OpFSub %float %139 %149
%151 = OpLoad %float %DaCub
%152 = OpLoad %v2float %26
%153 = OpCompositeExtract %float %152 0
%154 = OpFMul %float %151 %153
%155 = OpFSub %float %150 %154
OpStore %156 %155
%157 = OpLoad %float %DaSqd
OpStore %158 %157
%159 = OpFunctionCall %float %_guarded_divide %156 %158
OpReturnValue %159
%84 = OpLabel
%160 = OpLoad %v2float %27
%161 = OpCompositeExtract %float %160 0
%162 = OpLoad %v2float %26
%163 = OpCompositeExtract %float %162 1
%164 = OpLoad %v2float %26
%165 = OpCompositeExtract %float %164 0
%166 = OpFMul %float %float_2 %165
%167 = OpFSub %float %163 %166
%168 = OpFAdd %float %167 %float_1
%169 = OpFMul %float %161 %168
%170 = OpLoad %v2float %26
%171 = OpCompositeExtract %float %170 0
%172 = OpFAdd %float %169 %171
%174 = OpLoad %v2float %27
%175 = OpCompositeExtract %float %174 1
%176 = OpLoad %v2float %27
%177 = OpCompositeExtract %float %176 0
%178 = OpFMul %float %175 %177
%173 = OpExtInst %float %1 Sqrt %178
%179 = OpLoad %v2float %26
%180 = OpCompositeExtract %float %179 1
%181 = OpLoad %v2float %26
%182 = OpCompositeExtract %float %181 0
%183 = OpFMul %float %float_2 %182
%184 = OpFSub %float %180 %183
%185 = OpFMul %float %173 %184
%186 = OpFSub %float %172 %185
%187 = OpLoad %v2float %27
%188 = OpCompositeExtract %float %187 1
%189 = OpLoad %v2float %26
%190 = OpCompositeExtract %float %189 0
%191 = OpFMul %float %188 %190
%192 = OpFSub %float %186 %191
OpReturnValue %192
%85 = OpLabel
OpBranch %38
%38 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %194
%195 = OpLabel
%200 = OpVariable %_ptr_Function_v4float Function
%208 = OpVariable %_ptr_Function_v2float Function
%211 = OpVariable %_ptr_Function_v2float Function
%215 = OpVariable %_ptr_Function_v2float Function
%218 = OpVariable %_ptr_Function_v2float Function
%222 = OpVariable %_ptr_Function_v2float Function
%225 = OpVariable %_ptr_Function_v2float Function
%196 = OpLoad %v4float %dst
%197 = OpCompositeExtract %float %196 3
%199 = OpFOrdEqual %bool %197 %float_0
OpSelectionMerge %204 None
OpBranchConditional %199 %202 %203
%202 = OpLabel
%205 = OpLoad %v4float %src
OpStore %200 %205
OpBranch %204
%203 = OpLabel
%206 = OpLoad %v4float %src
%207 = OpVectorShuffle %v2float %206 %206 0 3
OpStore %208 %207
%209 = OpLoad %v4float %dst
%210 = OpVectorShuffle %v2float %209 %209 0 3
OpStore %211 %210
%212 = OpFunctionCall %float %_soft_light_component %208 %211
%213 = OpLoad %v4float %src
%214 = OpVectorShuffle %v2float %213 %213 1 3
OpStore %215 %214
%216 = OpLoad %v4float %dst
%217 = OpVectorShuffle %v2float %216 %216 1 3
OpStore %218 %217
%219 = OpFunctionCall %float %_soft_light_component %215 %218
%220 = OpLoad %v4float %src
%221 = OpVectorShuffle %v2float %220 %220 2 3
OpStore %222 %221
%223 = OpLoad %v4float %dst
%224 = OpVectorShuffle %v2float %223 %223 2 3
OpStore %225 %224
%226 = OpFunctionCall %float %_soft_light_component %222 %225
%227 = OpLoad %v4float %src
%228 = OpCompositeExtract %float %227 3
%229 = OpLoad %v4float %src
%230 = OpCompositeExtract %float %229 3
%231 = OpFSub %float %float_1 %230
%232 = OpLoad %v4float %dst
%233 = OpCompositeExtract %float %232 3
%234 = OpFMul %float %231 %233
%235 = OpFAdd %float %228 %234
%236 = OpCompositeConstruct %v4float %212 %219 %226 %235
OpStore %200 %236
OpBranch %204
%204 = OpLabel
%237 = OpLoad %v4float %200
OpStore %sk_FragColor %237
OpReturn
OpFunctionEnd

1 error
