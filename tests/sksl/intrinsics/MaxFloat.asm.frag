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
OpDecorate %expectedA RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
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
OpDecorate %197 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
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
%float_0_5 = OpConstant %float 0.5
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%31 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_75 %float_2_25
%float_1 = OpConstant %float 1
%34 = OpConstantComposite %v4float %float_0 %float_1 %float_0_75 %float_2_25
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%52 = OpConstantComposite %v2float %float_0_5 %float_0_5
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%66 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
%v3bool = OpTypeVector %bool 3
%78 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%v4bool = OpTypeVector %bool 4
%99 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_75
%int_1 = OpConstant %int 1
%172 = OpConstantComposite %v2float %float_0 %float_1
%180 = OpConstantComposite %v3float %float_0 %float_1 %float_0_75
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
OpStore %expectedA %31
OpStore %expectedB %34
%37 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%41 = OpLoad %v4float %37
%42 = OpCompositeExtract %float %41 0
%36 = OpExtInst %float %1 FMax %42 %float_0_5
%43 = OpLoad %v4float %expectedA
%44 = OpCompositeExtract %float %43 0
%45 = OpFOrdEqual %bool %36 %44
OpSelectionMerge %47 None
OpBranchConditional %45 %46 %47
%46 = OpLabel
%49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%50 = OpLoad %v4float %49
%51 = OpVectorShuffle %v2float %50 %50 0 1
%48 = OpExtInst %v2float %1 FMax %51 %52
%53 = OpLoad %v4float %expectedA
%54 = OpVectorShuffle %v2float %53 %53 0 1
%55 = OpFOrdEqual %v2bool %48 %54
%57 = OpAll %bool %55
OpBranch %47
%47 = OpLabel
%58 = OpPhi %bool %false %25 %57 %46
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%62 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%63 = OpLoad %v4float %62
%64 = OpVectorShuffle %v3float %63 %63 0 1 2
%61 = OpExtInst %v3float %1 FMax %64 %66
%67 = OpLoad %v4float %expectedA
%68 = OpVectorShuffle %v3float %67 %67 0 1 2
%69 = OpFOrdEqual %v3bool %61 %68
%71 = OpAll %bool %69
OpBranch %60
%60 = OpLabel
%72 = OpPhi %bool %false %47 %71 %59
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%76 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%77 = OpLoad %v4float %76
%75 = OpExtInst %v4float %1 FMax %77 %78
%79 = OpLoad %v4float %expectedA
%80 = OpFOrdEqual %v4bool %75 %79
%82 = OpAll %bool %80
OpBranch %74
%74 = OpLabel
%83 = OpPhi %bool %false %60 %82 %73
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%86 = OpLoad %v4float %expectedA
%87 = OpCompositeExtract %float %86 0
%88 = OpFOrdEqual %bool %float_0_5 %87
OpBranch %85
%85 = OpLabel
%89 = OpPhi %bool %false %74 %88 %84
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%92 = OpLoad %v4float %expectedA
%93 = OpVectorShuffle %v2float %92 %92 0 1
%94 = OpFOrdEqual %v2bool %52 %93
%95 = OpAll %bool %94
OpBranch %91
%91 = OpLabel
%96 = OpPhi %bool %false %85 %95 %90
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%100 = OpLoad %v4float %expectedA
%101 = OpVectorShuffle %v3float %100 %100 0 1 2
%102 = OpFOrdEqual %v3bool %99 %101
%103 = OpAll %bool %102
OpBranch %98
%98 = OpLabel
%104 = OpPhi %bool %false %91 %103 %97
OpSelectionMerge %106 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
%107 = OpLoad %v4float %expectedA
%108 = OpFOrdEqual %v4bool %31 %107
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
%117 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%119 = OpLoad %v4float %117
%120 = OpCompositeExtract %float %119 0
%113 = OpExtInst %float %1 FMax %116 %120
%121 = OpLoad %v4float %expectedB
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
%131 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%132 = OpLoad %v4float %131
%133 = OpVectorShuffle %v2float %132 %132 0 1
%127 = OpExtInst %v2float %1 FMax %130 %133
%134 = OpLoad %v4float %expectedB
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
%145 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%146 = OpLoad %v4float %145
%147 = OpVectorShuffle %v3float %146 %146 0 1 2
%141 = OpExtInst %v3float %1 FMax %144 %147
%148 = OpLoad %v4float %expectedB
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
%158 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%159 = OpLoad %v4float %158
%155 = OpExtInst %v4float %1 FMax %157 %159
%160 = OpLoad %v4float %expectedB
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
%168 = OpFOrdEqual %bool %float_0 %167
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
%189 = OpFOrdEqual %v4bool %34 %188
%190 = OpAll %bool %189
OpBranch %187
%187 = OpLabel
%191 = OpPhi %bool %false %179 %190 %186
OpSelectionMerge %195 None
OpBranchConditional %191 %193 %194
%193 = OpLabel
%196 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%197 = OpLoad %v4float %196
OpStore %192 %197
OpBranch %195
%194 = OpLabel
%198 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%200 = OpLoad %v4float %198
OpStore %192 %200
OpBranch %195
%195 = OpLabel
%201 = OpLoad %v4float %192
OpReturnValue %201
OpFunctionEnd
