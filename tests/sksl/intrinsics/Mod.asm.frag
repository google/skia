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
OpMemberName %_UniformBuffer 3 "colorWhite"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
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
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %expectedA RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0_75 = OpConstant %float 0.75
%float_0_25 = OpConstant %float 0.25
%30 = OpConstantComposite %v4float %float_0_75 %float_0 %float_0_75 %float_0_25
%float_1 = OpConstant %float 1
%33 = OpConstantComposite %v4float %float_0_25 %float_0 %float_0_75 %float_1
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%91 = OpConstantComposite %v2float %float_0_75 %float_0
%99 = OpConstantComposite %v3float %float_0_75 %float_0 %float_0_75
%int_3 = OpConstant %int 3
%172 = OpConstantComposite %v2float %float_0_25 %float_0
%180 = OpConstantComposite %v3float %float_0_25 %float_0 %float_0_75
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
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
%expectedA = OpVariable %_ptr_Function_v4float Function
%expectedB = OpVariable %_ptr_Function_v4float Function
%192 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedA %30
OpStore %expectedB %33
%36 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%40 = OpLoad %v4float %36
%41 = OpCompositeExtract %float %40 0
%35 = OpFMod %float %41 %float_1
%42 = OpLoad %v4float %expectedA
%43 = OpCompositeExtract %float %42 0
%44 = OpFOrdEqual %bool %35 %43
OpSelectionMerge %46 None
OpBranchConditional %44 %45 %46
%45 = OpLabel
%48 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%49 = OpLoad %v4float %48
%50 = OpVectorShuffle %v2float %49 %49 0 1
%51 = OpCompositeConstruct %v2float %float_1 %float_1
%47 = OpFMod %v2float %50 %51
%52 = OpLoad %v4float %expectedA
%53 = OpVectorShuffle %v2float %52 %52 0 1
%54 = OpFOrdEqual %v2bool %47 %53
%56 = OpAll %bool %54
OpBranch %46
%46 = OpLabel
%57 = OpPhi %bool %false %25 %56 %45
OpSelectionMerge %59 None
OpBranchConditional %57 %58 %59
%58 = OpLabel
%61 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%62 = OpLoad %v4float %61
%63 = OpVectorShuffle %v3float %62 %62 0 1 2
%65 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%60 = OpFMod %v3float %63 %65
%66 = OpLoad %v4float %expectedA
%67 = OpVectorShuffle %v3float %66 %66 0 1 2
%68 = OpFOrdEqual %v3bool %60 %67
%70 = OpAll %bool %68
OpBranch %59
%59 = OpLabel
%71 = OpPhi %bool %false %46 %70 %58
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%75 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%76 = OpLoad %v4float %75
%77 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%74 = OpFMod %v4float %76 %77
%78 = OpLoad %v4float %expectedA
%79 = OpFOrdEqual %v4bool %74 %78
%81 = OpAll %bool %79
OpBranch %73
%73 = OpLabel
%82 = OpPhi %bool %false %59 %81 %72
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%85 = OpLoad %v4float %expectedA
%86 = OpCompositeExtract %float %85 0
%87 = OpFOrdEqual %bool %float_0_75 %86
OpBranch %84
%84 = OpLabel
%88 = OpPhi %bool %false %73 %87 %83
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%92 = OpLoad %v4float %expectedA
%93 = OpVectorShuffle %v2float %92 %92 0 1
%94 = OpFOrdEqual %v2bool %91 %93
%95 = OpAll %bool %94
OpBranch %90
%90 = OpLabel
%96 = OpPhi %bool %false %84 %95 %89
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%100 = OpLoad %v4float %expectedA
%101 = OpVectorShuffle %v3float %100 %100 0 1 2
%102 = OpFOrdEqual %v3bool %99 %101
%103 = OpAll %bool %102
OpBranch %98
%98 = OpLabel
%104 = OpPhi %bool %false %90 %103 %97
OpSelectionMerge %106 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
%107 = OpLoad %v4float %expectedA
%108 = OpFOrdEqual %v4bool %30 %107
%109 = OpAll %bool %108
OpBranch %106
%106 = OpLabel
%110 = OpPhi %bool %false %98 %109 %105
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%114 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%115 = OpLoad %v4float %114
%116 = OpCompositeExtract %float %115 0
%117 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%119 = OpLoad %v4float %117
%120 = OpCompositeExtract %float %119 0
%113 = OpFMod %float %116 %120
%121 = OpLoad %v4float %expectedA
%122 = OpCompositeExtract %float %121 0
%123 = OpFOrdEqual %bool %113 %122
OpBranch %112
%112 = OpLabel
%124 = OpPhi %bool %false %106 %123 %111
OpSelectionMerge %126 None
OpBranchConditional %124 %125 %126
%125 = OpLabel
%128 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%129 = OpLoad %v4float %128
%130 = OpVectorShuffle %v2float %129 %129 0 1
%131 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%132 = OpLoad %v4float %131
%133 = OpVectorShuffle %v2float %132 %132 0 1
%127 = OpFMod %v2float %130 %133
%134 = OpLoad %v4float %expectedA
%135 = OpVectorShuffle %v2float %134 %134 0 1
%136 = OpFOrdEqual %v2bool %127 %135
%137 = OpAll %bool %136
OpBranch %126
%126 = OpLabel
%138 = OpPhi %bool %false %112 %137 %125
OpSelectionMerge %140 None
OpBranchConditional %138 %139 %140
%139 = OpLabel
%142 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%143 = OpLoad %v4float %142
%144 = OpVectorShuffle %v3float %143 %143 0 1 2
%145 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%146 = OpLoad %v4float %145
%147 = OpVectorShuffle %v3float %146 %146 0 1 2
%141 = OpFMod %v3float %144 %147
%148 = OpLoad %v4float %expectedA
%149 = OpVectorShuffle %v3float %148 %148 0 1 2
%150 = OpFOrdEqual %v3bool %141 %149
%151 = OpAll %bool %150
OpBranch %140
%140 = OpLabel
%152 = OpPhi %bool %false %126 %151 %139
OpSelectionMerge %154 None
OpBranchConditional %152 %153 %154
%153 = OpLabel
%156 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%157 = OpLoad %v4float %156
%158 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%159 = OpLoad %v4float %158
%155 = OpFMod %v4float %157 %159
%160 = OpLoad %v4float %expectedA
%161 = OpFOrdEqual %v4bool %155 %160
%162 = OpAll %bool %161
OpBranch %154
%154 = OpLabel
%163 = OpPhi %bool %false %140 %162 %153
OpSelectionMerge %165 None
OpBranchConditional %163 %164 %165
%164 = OpLabel
%166 = OpLoad %v4float %expectedB
%167 = OpCompositeExtract %float %166 0
%168 = OpFOrdEqual %bool %float_0_25 %167
OpBranch %165
%165 = OpLabel
%169 = OpPhi %bool %false %154 %168 %164
OpSelectionMerge %171 None
OpBranchConditional %169 %170 %171
%170 = OpLabel
%173 = OpLoad %v4float %expectedB
%174 = OpVectorShuffle %v2float %173 %173 0 1
%175 = OpFOrdEqual %v2bool %172 %174
%176 = OpAll %bool %175
OpBranch %171
%171 = OpLabel
%177 = OpPhi %bool %false %165 %176 %170
OpSelectionMerge %179 None
OpBranchConditional %177 %178 %179
%178 = OpLabel
%181 = OpLoad %v4float %expectedB
%182 = OpVectorShuffle %v3float %181 %181 0 1 2
%183 = OpFOrdEqual %v3bool %180 %182
%184 = OpAll %bool %183
OpBranch %179
%179 = OpLabel
%185 = OpPhi %bool %false %171 %184 %178
OpSelectionMerge %187 None
OpBranchConditional %185 %186 %187
%186 = OpLabel
%188 = OpLoad %v4float %expectedB
%189 = OpFOrdEqual %v4bool %33 %188
%190 = OpAll %bool %189
OpBranch %187
%187 = OpLabel
%191 = OpPhi %bool %false %179 %190 %186
OpSelectionMerge %195 None
OpBranchConditional %191 %193 %194
%193 = OpLabel
%196 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%198 = OpLoad %v4float %196
OpStore %192 %198
OpBranch %195
%194 = OpLabel
%199 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%201 = OpLoad %v4float %199
OpStore %192 %201
OpBranch %195
%195 = OpLabel
%202 = OpLoad %v4float %192
OpReturnValue %202
OpFunctionEnd
