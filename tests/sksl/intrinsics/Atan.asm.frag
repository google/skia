OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "inputVal"
OpMemberName %_UniformBuffer 1 "expected"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %constVal2 "constVal2"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
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
OpDecorate %constVal2 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
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
%float_1 = OpConstant %float 1
%29 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%98 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%107 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%130 = OpConstantComposite %v2float %float_1 %float_1
%143 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
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
%constVal2 = OpVariable %_ptr_Function_v4float Function
%190 = OpVariable %_ptr_Function_v4float Function
OpStore %constVal2 %29
%32 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%36 = OpLoad %v4float %32
%37 = OpCompositeExtract %float %36 0
%31 = OpExtInst %float %1 Atan %37
%38 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%40 = OpLoad %v4float %38
%41 = OpCompositeExtract %float %40 0
%42 = OpFOrdEqual %bool %31 %41
OpSelectionMerge %44 None
OpBranchConditional %42 %43 %44
%43 = OpLabel
%46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%47 = OpLoad %v4float %46
%48 = OpVectorShuffle %v2float %47 %47 0 1
%45 = OpExtInst %v2float %1 Atan %48
%49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%50 = OpLoad %v4float %49
%51 = OpVectorShuffle %v2float %50 %50 0 1
%52 = OpFOrdEqual %v2bool %45 %51
%54 = OpAll %bool %52
OpBranch %44
%44 = OpLabel
%55 = OpPhi %bool %false %25 %54 %43
OpSelectionMerge %57 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
%59 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%60 = OpLoad %v4float %59
%61 = OpVectorShuffle %v3float %60 %60 0 1 2
%58 = OpExtInst %v3float %1 Atan %61
%63 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%64 = OpLoad %v4float %63
%65 = OpVectorShuffle %v3float %64 %64 0 1 2
%66 = OpFOrdEqual %v3bool %58 %65
%68 = OpAll %bool %66
OpBranch %57
%57 = OpLabel
%69 = OpPhi %bool %false %44 %68 %56
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%73 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%74 = OpLoad %v4float %73
%72 = OpExtInst %v4float %1 Atan %74
%75 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%76 = OpLoad %v4float %75
%77 = OpFOrdEqual %v4bool %72 %76
%79 = OpAll %bool %77
OpBranch %71
%71 = OpLabel
%80 = OpPhi %bool %false %57 %79 %70
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%83 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%84 = OpLoad %v4float %83
%85 = OpCompositeExtract %float %84 0
%86 = OpFOrdEqual %bool %float_0 %85
OpBranch %82
%82 = OpLabel
%87 = OpPhi %bool %false %71 %86 %81
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%90 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%91 = OpLoad %v4float %90
%92 = OpVectorShuffle %v2float %91 %91 0 1
%93 = OpFOrdEqual %v2bool %19 %92
%94 = OpAll %bool %93
OpBranch %89
%89 = OpLabel
%95 = OpPhi %bool %false %82 %94 %88
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%99 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%100 = OpLoad %v4float %99
%101 = OpVectorShuffle %v3float %100 %100 0 1 2
%102 = OpFOrdEqual %v3bool %98 %101
%103 = OpAll %bool %102
OpBranch %97
%97 = OpLabel
%104 = OpPhi %bool %false %89 %103 %96
OpSelectionMerge %106 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
%108 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%109 = OpLoad %v4float %108
%110 = OpFOrdEqual %v4bool %107 %109
%111 = OpAll %bool %110
OpBranch %106
%106 = OpLabel
%112 = OpPhi %bool %false %97 %111 %105
OpSelectionMerge %114 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
%116 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%117 = OpLoad %v4float %116
%118 = OpCompositeExtract %float %117 0
%115 = OpExtInst %float %1 Atan2 %118 %float_1
%119 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%120 = OpLoad %v4float %119
%121 = OpCompositeExtract %float %120 0
%122 = OpFOrdEqual %bool %115 %121
OpBranch %114
%114 = OpLabel
%123 = OpPhi %bool %false %106 %122 %113
OpSelectionMerge %125 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
%127 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%128 = OpLoad %v4float %127
%129 = OpVectorShuffle %v2float %128 %128 0 1
%126 = OpExtInst %v2float %1 Atan2 %129 %130
%131 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%132 = OpLoad %v4float %131
%133 = OpVectorShuffle %v2float %132 %132 0 1
%134 = OpFOrdEqual %v2bool %126 %133
%135 = OpAll %bool %134
OpBranch %125
%125 = OpLabel
%136 = OpPhi %bool %false %114 %135 %124
OpSelectionMerge %138 None
OpBranchConditional %136 %137 %138
%137 = OpLabel
%140 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%141 = OpLoad %v4float %140
%142 = OpVectorShuffle %v3float %141 %141 0 1 2
%139 = OpExtInst %v3float %1 Atan2 %142 %143
%144 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%145 = OpLoad %v4float %144
%146 = OpVectorShuffle %v3float %145 %145 0 1 2
%147 = OpFOrdEqual %v3bool %139 %146
%148 = OpAll %bool %147
OpBranch %138
%138 = OpLabel
%149 = OpPhi %bool %false %125 %148 %137
OpSelectionMerge %151 None
OpBranchConditional %149 %150 %151
%150 = OpLabel
%153 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%154 = OpLoad %v4float %153
%152 = OpExtInst %v4float %1 Atan2 %154 %29
%155 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%156 = OpLoad %v4float %155
%157 = OpFOrdEqual %v4bool %152 %156
%158 = OpAll %bool %157
OpBranch %151
%151 = OpLabel
%159 = OpPhi %bool %false %138 %158 %150
OpSelectionMerge %161 None
OpBranchConditional %159 %160 %161
%160 = OpLabel
%162 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%163 = OpLoad %v4float %162
%164 = OpCompositeExtract %float %163 0
%165 = OpFOrdEqual %bool %float_0 %164
OpBranch %161
%161 = OpLabel
%166 = OpPhi %bool %false %151 %165 %160
OpSelectionMerge %168 None
OpBranchConditional %166 %167 %168
%167 = OpLabel
%169 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%170 = OpLoad %v4float %169
%171 = OpVectorShuffle %v2float %170 %170 0 1
%172 = OpFOrdEqual %v2bool %19 %171
%173 = OpAll %bool %172
OpBranch %168
%168 = OpLabel
%174 = OpPhi %bool %false %161 %173 %167
OpSelectionMerge %176 None
OpBranchConditional %174 %175 %176
%175 = OpLabel
%177 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%178 = OpLoad %v4float %177
%179 = OpVectorShuffle %v3float %178 %178 0 1 2
%180 = OpFOrdEqual %v3bool %98 %179
%181 = OpAll %bool %180
OpBranch %176
%176 = OpLabel
%182 = OpPhi %bool %false %168 %181 %175
OpSelectionMerge %184 None
OpBranchConditional %182 %183 %184
%183 = OpLabel
%185 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%186 = OpLoad %v4float %185
%187 = OpFOrdEqual %v4bool %107 %186
%188 = OpAll %bool %187
OpBranch %184
%184 = OpLabel
%189 = OpPhi %bool %false %176 %188 %183
OpSelectionMerge %193 None
OpBranchConditional %189 %191 %192
%191 = OpLabel
%194 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%196 = OpLoad %v4float %194
OpStore %190 %196
OpBranch %193
%192 = OpLabel
%197 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%199 = OpLoad %v4float %197
OpStore %190 %199
OpBranch %193
%193 = OpLabel
%200 = OpLoad %v4float %190
OpReturnValue %200
OpFunctionEnd
