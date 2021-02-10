OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "colorBlack"
OpMemberName %_UniformBuffer 3 "colorWhite"
OpMemberName %_UniformBuffer 4 "testInputs"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %expectedBW "expectedBW"
OpName %expectedWT "expectedWT"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 4 Offset 64
OpMemberDecorate %_UniformBuffer 4 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %34 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0_5 = OpConstant %float 0.5
%float_1 = OpConstant %float 1
%24 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_1
%float_2_25 = OpConstant %float 2.25
%27 = OpConstantComposite %v4float %float_1 %float_0_5 %float_1 %float_2_25
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%float_0 = OpConstant %float 0
%40 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
%v4bool = OpTypeVector %bool 4
%float_0_25 = OpConstant %float 0.25
%float_0_75 = OpConstant %float 0.75
%54 = OpConstantComposite %v4float %float_0_25 %float_0_75 %float_0 %float_1
%66 = OpConstantComposite %v4float %float_0_75 %float_0_25 %float_0 %float_1
%78 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%v2float = OpTypeVector %float 2
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%int_4 = OpConstant %int 4
%169 = OpConstantComposite %v2float %float_0 %float_0_5
%184 = OpConstantComposite %v3float %float_0 %float_0_5 %float_0
%199 = OpConstantComposite %v4float %float_0 %float_0_5 %float_0 %float_1
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%expectedBW = OpVariable %_ptr_Function_v4float Function
%expectedWT = OpVariable %_ptr_Function_v4float Function
%205 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedBW %24
OpStore %expectedWT %27
%30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%34 = OpLoad %v4float %30
%35 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%37 = OpLoad %v4float %35
%39 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%29 = OpExtInst %v4float %1 FMix %34 %37 %39
%41 = OpFOrdEqual %v4bool %29 %40
%43 = OpAll %bool %41
OpSelectionMerge %45 None
OpBranchConditional %43 %44 %45
%44 = OpLabel
%47 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%48 = OpLoad %v4float %47
%49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%50 = OpLoad %v4float %49
%52 = OpCompositeConstruct %v4float %float_0_25 %float_0_25 %float_0_25 %float_0_25
%46 = OpExtInst %v4float %1 FMix %48 %50 %52
%55 = OpFOrdEqual %v4bool %46 %54
%56 = OpAll %bool %55
OpBranch %45
%45 = OpLabel
%57 = OpPhi %bool %false %19 %56 %44
OpSelectionMerge %59 None
OpBranchConditional %57 %58 %59
%58 = OpLabel
%61 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%62 = OpLoad %v4float %61
%63 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%64 = OpLoad %v4float %63
%65 = OpCompositeConstruct %v4float %float_0_75 %float_0_75 %float_0_75 %float_0_75
%60 = OpExtInst %v4float %1 FMix %62 %64 %65
%67 = OpFOrdEqual %v4bool %60 %66
%68 = OpAll %bool %67
OpBranch %59
%59 = OpLabel
%69 = OpPhi %bool %false %45 %68 %58
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%73 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%74 = OpLoad %v4float %73
%75 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%76 = OpLoad %v4float %75
%77 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%72 = OpExtInst %v4float %1 FMix %74 %76 %77
%79 = OpFOrdEqual %v4bool %72 %78
%80 = OpAll %bool %79
OpBranch %71
%71 = OpLabel
%81 = OpPhi %bool %false %59 %80 %70
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%85 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%87 = OpLoad %v4float %85
%88 = OpCompositeExtract %float %87 0
%89 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%91 = OpLoad %v4float %89
%92 = OpCompositeExtract %float %91 0
%84 = OpExtInst %float %1 FMix %88 %92 %float_0_5
%93 = OpLoad %v4float %expectedBW
%94 = OpCompositeExtract %float %93 0
%95 = OpFOrdEqual %bool %84 %94
OpBranch %83
%83 = OpLabel
%96 = OpPhi %bool %false %71 %95 %82
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%100 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%101 = OpLoad %v4float %100
%102 = OpVectorShuffle %v2float %101 %101 0 1
%104 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%105 = OpLoad %v4float %104
%106 = OpVectorShuffle %v2float %105 %105 0 1
%107 = OpCompositeConstruct %v2float %float_0_5 %float_0_5
%99 = OpExtInst %v2float %1 FMix %102 %106 %107
%108 = OpLoad %v4float %expectedBW
%109 = OpVectorShuffle %v2float %108 %108 0 1
%110 = OpFOrdEqual %v2bool %99 %109
%112 = OpAll %bool %110
OpBranch %98
%98 = OpLabel
%113 = OpPhi %bool %false %83 %112 %97
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%117 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%118 = OpLoad %v4float %117
%119 = OpVectorShuffle %v3float %118 %118 0 1 2
%121 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%122 = OpLoad %v4float %121
%123 = OpVectorShuffle %v3float %122 %122 0 1 2
%124 = OpCompositeConstruct %v3float %float_0_5 %float_0_5 %float_0_5
%116 = OpExtInst %v3float %1 FMix %119 %123 %124
%125 = OpLoad %v4float %expectedBW
%126 = OpVectorShuffle %v3float %125 %125 0 1 2
%127 = OpFOrdEqual %v3bool %116 %126
%129 = OpAll %bool %127
OpBranch %115
%115 = OpLabel
%130 = OpPhi %bool %false %98 %129 %114
OpSelectionMerge %132 None
OpBranchConditional %130 %131 %132
%131 = OpLabel
%134 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%135 = OpLoad %v4float %134
%136 = OpVectorShuffle %v4float %135 %135 0 1 2 3
%137 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%138 = OpLoad %v4float %137
%139 = OpVectorShuffle %v4float %138 %138 0 1 2 3
%140 = OpCompositeConstruct %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%133 = OpExtInst %v4float %1 FMix %136 %139 %140
%141 = OpLoad %v4float %expectedBW
%142 = OpVectorShuffle %v4float %141 %141 0 1 2 3
%143 = OpFOrdEqual %v4bool %133 %142
%144 = OpAll %bool %143
OpBranch %132
%132 = OpLabel
%145 = OpPhi %bool %false %115 %144 %131
OpSelectionMerge %147 None
OpBranchConditional %145 %146 %147
%146 = OpLabel
%149 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%150 = OpLoad %v4float %149
%151 = OpCompositeExtract %float %150 0
%152 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%154 = OpLoad %v4float %152
%155 = OpCompositeExtract %float %154 0
%148 = OpExtInst %float %1 FMix %151 %155 %float_0
%156 = OpLoad %v4float %expectedWT
%157 = OpCompositeExtract %float %156 0
%158 = OpFOrdEqual %bool %148 %157
OpBranch %147
%147 = OpLabel
%159 = OpPhi %bool %false %132 %158 %146
OpSelectionMerge %161 None
OpBranchConditional %159 %160 %161
%160 = OpLabel
%163 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%164 = OpLoad %v4float %163
%165 = OpVectorShuffle %v2float %164 %164 0 1
%166 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%167 = OpLoad %v4float %166
%168 = OpVectorShuffle %v2float %167 %167 0 1
%162 = OpExtInst %v2float %1 FMix %165 %168 %169
%170 = OpLoad %v4float %expectedWT
%171 = OpVectorShuffle %v2float %170 %170 0 1
%172 = OpFOrdEqual %v2bool %162 %171
%173 = OpAll %bool %172
OpBranch %161
%161 = OpLabel
%174 = OpPhi %bool %false %147 %173 %160
OpSelectionMerge %176 None
OpBranchConditional %174 %175 %176
%175 = OpLabel
%178 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%179 = OpLoad %v4float %178
%180 = OpVectorShuffle %v3float %179 %179 0 1 2
%181 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%182 = OpLoad %v4float %181
%183 = OpVectorShuffle %v3float %182 %182 0 1 2
%177 = OpExtInst %v3float %1 FMix %180 %183 %184
%185 = OpLoad %v4float %expectedWT
%186 = OpVectorShuffle %v3float %185 %185 0 1 2
%187 = OpFOrdEqual %v3bool %177 %186
%188 = OpAll %bool %187
OpBranch %176
%176 = OpLabel
%189 = OpPhi %bool %false %161 %188 %175
OpSelectionMerge %191 None
OpBranchConditional %189 %190 %191
%190 = OpLabel
%193 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%194 = OpLoad %v4float %193
%195 = OpVectorShuffle %v4float %194 %194 0 1 2 3
%196 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%197 = OpLoad %v4float %196
%198 = OpVectorShuffle %v4float %197 %197 0 1 2 3
%192 = OpExtInst %v4float %1 FMix %195 %198 %199
%200 = OpLoad %v4float %expectedWT
%201 = OpVectorShuffle %v4float %200 %200 0 1 2 3
%202 = OpFOrdEqual %v4bool %192 %201
%203 = OpAll %bool %202
OpBranch %191
%191 = OpLabel
%204 = OpPhi %bool %false %176 %203 %190
OpSelectionMerge %208 None
OpBranchConditional %204 %206 %207
%206 = OpLabel
%209 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%210 = OpLoad %v4float %209
OpStore %205 %210
OpBranch %208
%207 = OpLabel
%211 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%212 = OpLoad %v4float %211
OpStore %205 %212
OpBranch %208
%208 = OpLabel
%213 = OpLoad %v4float %205
OpReturnValue %213
OpFunctionEnd
