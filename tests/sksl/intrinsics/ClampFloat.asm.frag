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
OpName %expectedA "expectedA"
OpName %clampLow "clampLow"
OpName %expectedB "expectedB"
OpName %clampHigh "clampHigh"
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
OpDecorate %expectedA RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %clampLow RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %clampHigh RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n1 = OpConstant %float -1
%float_0_75 = OpConstant %float 0.75
%float_1 = OpConstant %float 1
%31 = OpConstantComposite %v4float %float_n1 %float_0 %float_0_75 %float_1
%float_n2 = OpConstant %float -2
%34 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n2 %float_1
%float_0_5 = OpConstant %float 0.5
%float_2_25 = OpConstant %float 2.25
%38 = OpConstantComposite %v4float %float_n1 %float_0 %float_0_5 %float_2_25
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%42 = OpConstantComposite %v4float %float_1 %float_2 %float_0_5 %float_3
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%111 = OpConstantComposite %v2float %float_n1 %float_n2
%112 = OpConstantComposite %v2float %float_1 %float_2
%124 = OpConstantComposite %v3float %float_n1 %float_n2 %float_n2
%125 = OpConstantComposite %v3float %float_1 %float_2 %float_0_5
%150 = OpConstantComposite %v2float %float_n1 %float_0
%158 = OpConstantComposite %v3float %float_n1 %float_0 %float_0_75
%185 = OpConstantComposite %v3float %float_n1 %float_0 %float_0_5
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
%clampLow = OpVariable %_ptr_Function_v4float Function
%expectedB = OpVariable %_ptr_Function_v4float Function
%clampHigh = OpVariable %_ptr_Function_v4float Function
%197 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedA %31
OpStore %clampLow %34
OpStore %expectedB %38
OpStore %clampHigh %42
%45 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%49 = OpLoad %v4float %45
%50 = OpCompositeExtract %float %49 0
%44 = OpExtInst %float %1 FClamp %50 %float_n1 %float_1
%51 = OpLoad %v4float %expectedA
%52 = OpCompositeExtract %float %51 0
%53 = OpFOrdEqual %bool %44 %52
OpSelectionMerge %55 None
OpBranchConditional %53 %54 %55
%54 = OpLabel
%57 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%58 = OpLoad %v4float %57
%59 = OpVectorShuffle %v2float %58 %58 0 1
%60 = OpCompositeConstruct %v2float %float_n1 %float_n1
%61 = OpCompositeConstruct %v2float %float_1 %float_1
%56 = OpExtInst %v2float %1 FClamp %59 %60 %61
%62 = OpLoad %v4float %expectedA
%63 = OpVectorShuffle %v2float %62 %62 0 1
%64 = OpFOrdEqual %v2bool %56 %63
%66 = OpAll %bool %64
OpBranch %55
%55 = OpLabel
%67 = OpPhi %bool %false %25 %66 %54
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%71 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%72 = OpLoad %v4float %71
%73 = OpVectorShuffle %v3float %72 %72 0 1 2
%75 = OpCompositeConstruct %v3float %float_n1 %float_n1 %float_n1
%76 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%70 = OpExtInst %v3float %1 FClamp %73 %75 %76
%77 = OpLoad %v4float %expectedA
%78 = OpVectorShuffle %v3float %77 %77 0 1 2
%79 = OpFOrdEqual %v3bool %70 %78
%81 = OpAll %bool %79
OpBranch %69
%69 = OpLabel
%82 = OpPhi %bool %false %55 %81 %68
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%86 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%87 = OpLoad %v4float %86
%88 = OpCompositeConstruct %v4float %float_n1 %float_n1 %float_n1 %float_n1
%89 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%85 = OpExtInst %v4float %1 FClamp %87 %88 %89
%90 = OpLoad %v4float %expectedA
%91 = OpFOrdEqual %v4bool %85 %90
%93 = OpAll %bool %91
OpBranch %84
%84 = OpLabel
%94 = OpPhi %bool %false %69 %93 %83
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%98 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%99 = OpLoad %v4float %98
%100 = OpCompositeExtract %float %99 0
%97 = OpExtInst %float %1 FClamp %100 %float_n1 %float_1
%101 = OpLoad %v4float %expectedB
%102 = OpCompositeExtract %float %101 0
%103 = OpFOrdEqual %bool %97 %102
OpBranch %96
%96 = OpLabel
%104 = OpPhi %bool %false %84 %103 %95
OpSelectionMerge %106 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
%108 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%109 = OpLoad %v4float %108
%110 = OpVectorShuffle %v2float %109 %109 0 1
%107 = OpExtInst %v2float %1 FClamp %110 %111 %112
%113 = OpLoad %v4float %expectedB
%114 = OpVectorShuffle %v2float %113 %113 0 1
%115 = OpFOrdEqual %v2bool %107 %114
%116 = OpAll %bool %115
OpBranch %106
%106 = OpLabel
%117 = OpPhi %bool %false %96 %116 %105
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%121 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%122 = OpLoad %v4float %121
%123 = OpVectorShuffle %v3float %122 %122 0 1 2
%120 = OpExtInst %v3float %1 FClamp %123 %124 %125
%126 = OpLoad %v4float %expectedB
%127 = OpVectorShuffle %v3float %126 %126 0 1 2
%128 = OpFOrdEqual %v3bool %120 %127
%129 = OpAll %bool %128
OpBranch %119
%119 = OpLabel
%130 = OpPhi %bool %false %106 %129 %118
OpSelectionMerge %132 None
OpBranchConditional %130 %131 %132
%131 = OpLabel
%134 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%135 = OpLoad %v4float %134
%136 = OpLoad %v4float %clampLow
%137 = OpLoad %v4float %clampHigh
%133 = OpExtInst %v4float %1 FClamp %135 %136 %137
%138 = OpLoad %v4float %expectedB
%139 = OpFOrdEqual %v4bool %133 %138
%140 = OpAll %bool %139
OpBranch %132
%132 = OpLabel
%141 = OpPhi %bool %false %119 %140 %131
OpSelectionMerge %143 None
OpBranchConditional %141 %142 %143
%142 = OpLabel
%144 = OpLoad %v4float %expectedA
%145 = OpCompositeExtract %float %144 0
%146 = OpFOrdEqual %bool %float_n1 %145
OpBranch %143
%143 = OpLabel
%147 = OpPhi %bool %false %132 %146 %142
OpSelectionMerge %149 None
OpBranchConditional %147 %148 %149
%148 = OpLabel
%151 = OpLoad %v4float %expectedA
%152 = OpVectorShuffle %v2float %151 %151 0 1
%153 = OpFOrdEqual %v2bool %150 %152
%154 = OpAll %bool %153
OpBranch %149
%149 = OpLabel
%155 = OpPhi %bool %false %143 %154 %148
OpSelectionMerge %157 None
OpBranchConditional %155 %156 %157
%156 = OpLabel
%159 = OpLoad %v4float %expectedA
%160 = OpVectorShuffle %v3float %159 %159 0 1 2
%161 = OpFOrdEqual %v3bool %158 %160
%162 = OpAll %bool %161
OpBranch %157
%157 = OpLabel
%163 = OpPhi %bool %false %149 %162 %156
OpSelectionMerge %165 None
OpBranchConditional %163 %164 %165
%164 = OpLabel
%166 = OpLoad %v4float %expectedA
%167 = OpFOrdEqual %v4bool %31 %166
%168 = OpAll %bool %167
OpBranch %165
%165 = OpLabel
%169 = OpPhi %bool %false %157 %168 %164
OpSelectionMerge %171 None
OpBranchConditional %169 %170 %171
%170 = OpLabel
%172 = OpLoad %v4float %expectedB
%173 = OpCompositeExtract %float %172 0
%174 = OpFOrdEqual %bool %float_n1 %173
OpBranch %171
%171 = OpLabel
%175 = OpPhi %bool %false %165 %174 %170
OpSelectionMerge %177 None
OpBranchConditional %175 %176 %177
%176 = OpLabel
%178 = OpLoad %v4float %expectedB
%179 = OpVectorShuffle %v2float %178 %178 0 1
%180 = OpFOrdEqual %v2bool %150 %179
%181 = OpAll %bool %180
OpBranch %177
%177 = OpLabel
%182 = OpPhi %bool %false %171 %181 %176
OpSelectionMerge %184 None
OpBranchConditional %182 %183 %184
%183 = OpLabel
%186 = OpLoad %v4float %expectedB
%187 = OpVectorShuffle %v3float %186 %186 0 1 2
%188 = OpFOrdEqual %v3bool %185 %187
%189 = OpAll %bool %188
OpBranch %184
%184 = OpLabel
%190 = OpPhi %bool %false %177 %189 %183
OpSelectionMerge %192 None
OpBranchConditional %190 %191 %192
%191 = OpLabel
%193 = OpLoad %v4float %expectedB
%194 = OpFOrdEqual %v4bool %38 %193
%195 = OpAll %bool %194
OpBranch %192
%192 = OpLabel
%196 = OpPhi %bool %false %184 %195 %191
OpSelectionMerge %200 None
OpBranchConditional %196 %198 %199
%198 = OpLabel
%201 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%203 = OpLoad %v4float %201
OpStore %197 %203
OpBranch %200
%199 = OpLabel
%204 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%206 = OpLoad %v4float %204
OpStore %197 %206
OpBranch %200
%200 = OpLabel
%207 = OpLoad %v4float %197
OpReturnValue %207
OpFunctionEnd
