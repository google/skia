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
OpDecorate %64 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
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
%true = OpConstantTrue %bool
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%50 = OpConstantComposite %v3float %float_0 %float_0 %float_0_84375
%v3bool = OpTypeVector %bool 3
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%103 = OpConstantComposite %v2float %float_n1_25 %float_0
%119 = OpConstantComposite %v3float %float_n1_25 %float_0 %float_0_75
%v4bool = OpTypeVector %bool 4
%144 = OpConstantComposite %v2float %float_1 %float_0
%151 = OpConstantComposite %v3float %float_1 %float_0 %float_1
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
%206 = OpVariable %_ptr_Function_v4float Function
OpStore %constVal %31
OpStore %expectedA %35
OpStore %expectedB %37
OpSelectionMerge %41 None
OpBranchConditional %true %40 %41
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
OpBranch %57
%57 = OpLabel
%58 = OpPhi %bool %false %48 %true %56
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
OpBranch %60
%60 = OpLabel
%61 = OpPhi %bool %false %57 %true %59
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%64 = OpVectorShuffle %v2float %35 %35 0 1
%65 = OpFOrdEqual %v2bool %19 %64
%66 = OpAll %bool %65
OpBranch %63
%63 = OpLabel
%67 = OpPhi %bool %false %60 %66 %62
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%70 = OpVectorShuffle %v3float %35 %35 0 1 2
%71 = OpFOrdEqual %v3bool %50 %70
%72 = OpAll %bool %71
OpBranch %69
%69 = OpLabel
%73 = OpPhi %bool %false %63 %72 %68
OpSelectionMerge %75 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
OpBranch %75
%75 = OpLabel
%76 = OpPhi %bool %false %69 %true %74
OpSelectionMerge %78 None
OpBranchConditional %76 %77 %78
%77 = OpLabel
%80 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%84 = OpLoad %v4float %80
%85 = OpCompositeExtract %float %84 1
%86 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%88 = OpLoad %v4float %86
%89 = OpCompositeExtract %float %88 1
%79 = OpExtInst %float %1 SmoothStep %85 %89 %float_n1_25
%90 = OpFOrdEqual %bool %79 %float_0
OpBranch %78
%78 = OpLabel
%91 = OpPhi %bool %false %75 %90 %77
OpSelectionMerge %93 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%95 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%96 = OpLoad %v4float %95
%97 = OpCompositeExtract %float %96 1
%98 = OpCompositeConstruct %v2float %97 %97
%99 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%100 = OpLoad %v4float %99
%101 = OpCompositeExtract %float %100 1
%102 = OpCompositeConstruct %v2float %101 %101
%94 = OpExtInst %v2float %1 SmoothStep %98 %102 %103
%104 = OpVectorShuffle %v2float %35 %35 0 1
%105 = OpFOrdEqual %v2bool %94 %104
%106 = OpAll %bool %105
OpBranch %93
%93 = OpLabel
%107 = OpPhi %bool %false %78 %106 %92
OpSelectionMerge %109 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
%111 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%112 = OpLoad %v4float %111
%113 = OpCompositeExtract %float %112 1
%114 = OpCompositeConstruct %v3float %113 %113 %113
%115 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%116 = OpLoad %v4float %115
%117 = OpCompositeExtract %float %116 1
%118 = OpCompositeConstruct %v3float %117 %117 %117
%110 = OpExtInst %v3float %1 SmoothStep %114 %118 %119
%120 = OpVectorShuffle %v3float %35 %35 0 1 2
%121 = OpFOrdEqual %v3bool %110 %120
%122 = OpAll %bool %121
OpBranch %109
%109 = OpLabel
%123 = OpPhi %bool %false %93 %122 %108
OpSelectionMerge %125 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
%127 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%128 = OpLoad %v4float %127
%129 = OpCompositeExtract %float %128 1
%130 = OpCompositeConstruct %v4float %129 %129 %129 %129
%131 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%132 = OpLoad %v4float %131
%133 = OpCompositeExtract %float %132 1
%134 = OpCompositeConstruct %v4float %133 %133 %133 %133
%126 = OpExtInst %v4float %1 SmoothStep %130 %134 %31
%135 = OpFOrdEqual %v4bool %126 %35
%137 = OpAll %bool %135
OpBranch %125
%125 = OpLabel
%138 = OpPhi %bool %false %109 %137 %124
OpSelectionMerge %140 None
OpBranchConditional %138 %139 %140
%139 = OpLabel
OpBranch %140
%140 = OpLabel
%141 = OpPhi %bool %false %125 %true %139
OpSelectionMerge %143 None
OpBranchConditional %141 %142 %143
%142 = OpLabel
%145 = OpVectorShuffle %v2float %37 %37 0 1
%146 = OpFOrdEqual %v2bool %144 %145
%147 = OpAll %bool %146
OpBranch %143
%143 = OpLabel
%148 = OpPhi %bool %false %140 %147 %142
OpSelectionMerge %150 None
OpBranchConditional %148 %149 %150
%149 = OpLabel
%152 = OpVectorShuffle %v3float %37 %37 0 1 2
%153 = OpFOrdEqual %v3bool %151 %152
%154 = OpAll %bool %153
OpBranch %150
%150 = OpLabel
%155 = OpPhi %bool %false %143 %154 %149
OpSelectionMerge %157 None
OpBranchConditional %155 %156 %157
%156 = OpLabel
OpBranch %157
%157 = OpLabel
%158 = OpPhi %bool %false %150 %true %156
OpSelectionMerge %160 None
OpBranchConditional %158 %159 %160
%159 = OpLabel
%162 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%163 = OpLoad %v4float %162
%164 = OpCompositeExtract %float %163 0
%165 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%166 = OpLoad %v4float %165
%167 = OpCompositeExtract %float %166 0
%161 = OpExtInst %float %1 SmoothStep %164 %167 %float_n1_25
%168 = OpFOrdEqual %bool %161 %float_1
OpBranch %160
%160 = OpLabel
%169 = OpPhi %bool %false %157 %168 %159
OpSelectionMerge %171 None
OpBranchConditional %169 %170 %171
%170 = OpLabel
%173 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%174 = OpLoad %v4float %173
%175 = OpVectorShuffle %v2float %174 %174 0 1
%176 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%177 = OpLoad %v4float %176
%178 = OpVectorShuffle %v2float %177 %177 0 1
%172 = OpExtInst %v2float %1 SmoothStep %175 %178 %103
%179 = OpVectorShuffle %v2float %37 %37 0 1
%180 = OpFOrdEqual %v2bool %172 %179
%181 = OpAll %bool %180
OpBranch %171
%171 = OpLabel
%182 = OpPhi %bool %false %160 %181 %170
OpSelectionMerge %184 None
OpBranchConditional %182 %183 %184
%183 = OpLabel
%186 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%187 = OpLoad %v4float %186
%188 = OpVectorShuffle %v3float %187 %187 0 1 2
%189 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%190 = OpLoad %v4float %189
%191 = OpVectorShuffle %v3float %190 %190 0 1 2
%185 = OpExtInst %v3float %1 SmoothStep %188 %191 %119
%192 = OpVectorShuffle %v3float %37 %37 0 1 2
%193 = OpFOrdEqual %v3bool %185 %192
%194 = OpAll %bool %193
OpBranch %184
%184 = OpLabel
%195 = OpPhi %bool %false %171 %194 %183
OpSelectionMerge %197 None
OpBranchConditional %195 %196 %197
%196 = OpLabel
%199 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%200 = OpLoad %v4float %199
%201 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%202 = OpLoad %v4float %201
%198 = OpExtInst %v4float %1 SmoothStep %200 %202 %31
%203 = OpFOrdEqual %v4bool %198 %37
%204 = OpAll %bool %203
OpBranch %197
%197 = OpLabel
%205 = OpPhi %bool %false %184 %204 %196
OpSelectionMerge %209 None
OpBranchConditional %205 %207 %208
%207 = OpLabel
%210 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%211 = OpLoad %v4float %210
OpStore %206 %211
OpBranch %209
%208 = OpLabel
%212 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%213 = OpLoad %v4float %212
OpStore %206 %213
OpBranch %209
%209 = OpLabel
%214 = OpLoad %v4float %206
OpReturnValue %214
OpFunctionEnd
