OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "input"
OpMemberName %_UniformBuffer 1 "expected"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %constVal2 "constVal2"
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
OpDecorate %constVal2 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
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
OpDecorate %98 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
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
%191 = OpVariable %_ptr_Function_v4float Function
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
%155 = OpLoad %v4float %constVal2
%152 = OpExtInst %v4float %1 Atan2 %154 %155
%156 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%157 = OpLoad %v4float %156
%158 = OpFOrdEqual %v4bool %152 %157
%159 = OpAll %bool %158
OpBranch %151
%151 = OpLabel
%160 = OpPhi %bool %false %138 %159 %150
OpSelectionMerge %162 None
OpBranchConditional %160 %161 %162
%161 = OpLabel
%163 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%164 = OpLoad %v4float %163
%165 = OpCompositeExtract %float %164 0
%166 = OpFOrdEqual %bool %float_0 %165
OpBranch %162
%162 = OpLabel
%167 = OpPhi %bool %false %151 %166 %161
OpSelectionMerge %169 None
OpBranchConditional %167 %168 %169
%168 = OpLabel
%170 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%171 = OpLoad %v4float %170
%172 = OpVectorShuffle %v2float %171 %171 0 1
%173 = OpFOrdEqual %v2bool %19 %172
%174 = OpAll %bool %173
OpBranch %169
%169 = OpLabel
%175 = OpPhi %bool %false %162 %174 %168
OpSelectionMerge %177 None
OpBranchConditional %175 %176 %177
%176 = OpLabel
%178 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%179 = OpLoad %v4float %178
%180 = OpVectorShuffle %v3float %179 %179 0 1 2
%181 = OpFOrdEqual %v3bool %98 %180
%182 = OpAll %bool %181
OpBranch %177
%177 = OpLabel
%183 = OpPhi %bool %false %169 %182 %176
OpSelectionMerge %185 None
OpBranchConditional %183 %184 %185
%184 = OpLabel
%186 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%187 = OpLoad %v4float %186
%188 = OpFOrdEqual %v4bool %107 %187
%189 = OpAll %bool %188
OpBranch %185
%185 = OpLabel
%190 = OpPhi %bool %false %177 %189 %184
OpSelectionMerge %194 None
OpBranchConditional %190 %192 %193
%192 = OpLabel
%195 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%197 = OpLoad %v4float %195
OpStore %191 %197
OpBranch %194
%193 = OpLabel
%198 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%200 = OpLoad %v4float %198
OpStore %191 %200
OpBranch %194
%194 = OpLabel
%201 = OpLoad %v4float %191
OpReturnValue %201
OpFunctionEnd
