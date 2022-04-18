OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %constVal "constVal"
OpName %expectedA "expectedA"
OpName %expectedB "expectedB"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %constVal RelaxedPrecision
OpDecorate %expectedA RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n1_25 = OpConstant %float -1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%31 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
%float_0_84375 = OpConstant %float 0.84375
%float_1 = OpConstant %float 1
%35 = OpConstantComposite %v4float %float_0 %float_0 %float_0_84375 %float_1
%37 = OpConstantComposite %v4float %float_1 %float_0 %float_1 %float_1
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%50 = OpConstantComposite %v3float %float_0 %float_0 %float_0_84375
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%109 = OpConstantComposite %v2float %float_n1_25 %float_0
%125 = OpConstantComposite %v3float %float_n1_25 %float_0 %float_0_75
%150 = OpConstantComposite %v2float %float_1 %float_0
%157 = OpConstantComposite %v3float %float_1 %float_0 %float_1
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%20 = OpVariable %_ptr_Function_v2float Function
OpStore %20 %19
%22 = OpFunctionCall %v4float %main %20
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %23
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpLabel
%constVal = OpVariable %_ptr_Function_v4float Function
%expectedA = OpVariable %_ptr_Function_v4float Function
%expectedB = OpVariable %_ptr_Function_v4float Function
%214 = OpVariable %_ptr_Function_v4float Function
OpStore %constVal %31
OpStore %expectedA %35
OpStore %expectedB %37
%39 = OpFOrdEqual %bool %float_0 %float_0
OpSelectionMerge %41 None
OpBranchConditional %39 %40 %41
%40 = OpLabel
%42 = OpVectorShuffle %v2float %35 %35 0 1
%43 = OpFOrdEqual %v2bool %19 %42
%45 = OpAll %bool %43
OpBranch %41
%41 = OpLabel
%46 = OpPhi %bool %false %25 %45 %40
OpSelectionMerge %48 None
OpBranchConditional %46 %47 %48
%47 = OpLabel
%51 = OpVectorShuffle %v3float %35 %35 0 1 2
%52 = OpFOrdEqual %v3bool %50 %51
%54 = OpAll %bool %52
OpBranch %48
%48 = OpLabel
%55 = OpPhi %bool %false %41 %54 %47
OpSelectionMerge %57 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
%58 = OpFOrdEqual %v4bool %35 %35
%60 = OpAll %bool %58
OpBranch %57
%57 = OpLabel
%61 = OpPhi %bool %false %48 %60 %56
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%64 = OpFOrdEqual %bool %float_0 %float_0
OpBranch %63
%63 = OpLabel
%65 = OpPhi %bool %false %57 %64 %62
OpSelectionMerge %67 None
OpBranchConditional %65 %66 %67
%66 = OpLabel
%68 = OpVectorShuffle %v2float %35 %35 0 1
%69 = OpFOrdEqual %v2bool %19 %68
%70 = OpAll %bool %69
OpBranch %67
%67 = OpLabel
%71 = OpPhi %bool %false %63 %70 %66
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%74 = OpVectorShuffle %v3float %35 %35 0 1 2
%75 = OpFOrdEqual %v3bool %50 %74
%76 = OpAll %bool %75
OpBranch %73
%73 = OpLabel
%77 = OpPhi %bool %false %67 %76 %72
OpSelectionMerge %79 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
%80 = OpFOrdEqual %v4bool %35 %35
%81 = OpAll %bool %80
OpBranch %79
%79 = OpLabel
%82 = OpPhi %bool %false %73 %81 %78
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%86 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%90 = OpLoad %v4float %86
%91 = OpCompositeExtract %float %90 1
%92 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%94 = OpLoad %v4float %92
%95 = OpCompositeExtract %float %94 1
%85 = OpExtInst %float %1 SmoothStep %91 %95 %float_n1_25
%96 = OpFOrdEqual %bool %85 %float_0
OpBranch %84
%84 = OpLabel
%97 = OpPhi %bool %false %79 %96 %83
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%101 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%102 = OpLoad %v4float %101
%103 = OpCompositeExtract %float %102 1
%104 = OpCompositeConstruct %v2float %103 %103
%105 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%106 = OpLoad %v4float %105
%107 = OpCompositeExtract %float %106 1
%108 = OpCompositeConstruct %v2float %107 %107
%100 = OpExtInst %v2float %1 SmoothStep %104 %108 %109
%110 = OpVectorShuffle %v2float %35 %35 0 1
%111 = OpFOrdEqual %v2bool %100 %110
%112 = OpAll %bool %111
OpBranch %99
%99 = OpLabel
%113 = OpPhi %bool %false %84 %112 %98
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%117 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%118 = OpLoad %v4float %117
%119 = OpCompositeExtract %float %118 1
%120 = OpCompositeConstruct %v3float %119 %119 %119
%121 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%122 = OpLoad %v4float %121
%123 = OpCompositeExtract %float %122 1
%124 = OpCompositeConstruct %v3float %123 %123 %123
%116 = OpExtInst %v3float %1 SmoothStep %120 %124 %125
%126 = OpVectorShuffle %v3float %35 %35 0 1 2
%127 = OpFOrdEqual %v3bool %116 %126
%128 = OpAll %bool %127
OpBranch %115
%115 = OpLabel
%129 = OpPhi %bool %false %99 %128 %114
OpSelectionMerge %131 None
OpBranchConditional %129 %130 %131
%130 = OpLabel
%133 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%134 = OpLoad %v4float %133
%135 = OpCompositeExtract %float %134 1
%136 = OpCompositeConstruct %v4float %135 %135 %135 %135
%137 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%138 = OpLoad %v4float %137
%139 = OpCompositeExtract %float %138 1
%140 = OpCompositeConstruct %v4float %139 %139 %139 %139
%132 = OpExtInst %v4float %1 SmoothStep %136 %140 %31
%141 = OpFOrdEqual %v4bool %132 %35
%142 = OpAll %bool %141
OpBranch %131
%131 = OpLabel
%143 = OpPhi %bool %false %115 %142 %130
OpSelectionMerge %145 None
OpBranchConditional %143 %144 %145
%144 = OpLabel
%146 = OpFOrdEqual %bool %float_1 %float_1
OpBranch %145
%145 = OpLabel
%147 = OpPhi %bool %false %131 %146 %144
OpSelectionMerge %149 None
OpBranchConditional %147 %148 %149
%148 = OpLabel
%151 = OpVectorShuffle %v2float %37 %37 0 1
%152 = OpFOrdEqual %v2bool %150 %151
%153 = OpAll %bool %152
OpBranch %149
%149 = OpLabel
%154 = OpPhi %bool %false %145 %153 %148
OpSelectionMerge %156 None
OpBranchConditional %154 %155 %156
%155 = OpLabel
%158 = OpVectorShuffle %v3float %37 %37 0 1 2
%159 = OpFOrdEqual %v3bool %157 %158
%160 = OpAll %bool %159
OpBranch %156
%156 = OpLabel
%161 = OpPhi %bool %false %149 %160 %155
OpSelectionMerge %163 None
OpBranchConditional %161 %162 %163
%162 = OpLabel
%164 = OpFOrdEqual %v4bool %37 %37
%165 = OpAll %bool %164
OpBranch %163
%163 = OpLabel
%166 = OpPhi %bool %false %156 %165 %162
OpSelectionMerge %168 None
OpBranchConditional %166 %167 %168
%167 = OpLabel
%170 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%171 = OpLoad %v4float %170
%172 = OpCompositeExtract %float %171 0
%173 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%174 = OpLoad %v4float %173
%175 = OpCompositeExtract %float %174 0
%169 = OpExtInst %float %1 SmoothStep %172 %175 %float_n1_25
%176 = OpFOrdEqual %bool %169 %float_1
OpBranch %168
%168 = OpLabel
%177 = OpPhi %bool %false %163 %176 %167
OpSelectionMerge %179 None
OpBranchConditional %177 %178 %179
%178 = OpLabel
%181 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%182 = OpLoad %v4float %181
%183 = OpVectorShuffle %v2float %182 %182 0 1
%184 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%185 = OpLoad %v4float %184
%186 = OpVectorShuffle %v2float %185 %185 0 1
%180 = OpExtInst %v2float %1 SmoothStep %183 %186 %109
%187 = OpVectorShuffle %v2float %37 %37 0 1
%188 = OpFOrdEqual %v2bool %180 %187
%189 = OpAll %bool %188
OpBranch %179
%179 = OpLabel
%190 = OpPhi %bool %false %168 %189 %178
OpSelectionMerge %192 None
OpBranchConditional %190 %191 %192
%191 = OpLabel
%194 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%195 = OpLoad %v4float %194
%196 = OpVectorShuffle %v3float %195 %195 0 1 2
%197 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%198 = OpLoad %v4float %197
%199 = OpVectorShuffle %v3float %198 %198 0 1 2
%193 = OpExtInst %v3float %1 SmoothStep %196 %199 %125
%200 = OpVectorShuffle %v3float %37 %37 0 1 2
%201 = OpFOrdEqual %v3bool %193 %200
%202 = OpAll %bool %201
OpBranch %192
%192 = OpLabel
%203 = OpPhi %bool %false %179 %202 %191
OpSelectionMerge %205 None
OpBranchConditional %203 %204 %205
%204 = OpLabel
%207 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%208 = OpLoad %v4float %207
%209 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%210 = OpLoad %v4float %209
%206 = OpExtInst %v4float %1 SmoothStep %208 %210 %31
%211 = OpFOrdEqual %v4bool %206 %37
%212 = OpAll %bool %211
OpBranch %205
%205 = OpLabel
%213 = OpPhi %bool %false %192 %212 %204
OpSelectionMerge %217 None
OpBranchConditional %213 %215 %216
%215 = OpLabel
%218 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%219 = OpLoad %v4float %218
OpStore %214 %219
OpBranch %217
%216 = OpLabel
%220 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%221 = OpLoad %v4float %220
OpStore %214 %221
OpBranch %217
%217 = OpLabel
%222 = OpLoad %v4float %214
OpReturnValue %222
OpFunctionEnd
