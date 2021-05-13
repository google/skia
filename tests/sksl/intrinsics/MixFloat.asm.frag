OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "colorBlack"
OpMemberName %_UniformBuffer 3 "colorWhite"
OpMemberName %_UniformBuffer 4 "testInputs"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %expectedBW "expectedBW"
OpName %expectedWT "expectedWT"
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
OpMemberDecorate %_UniformBuffer 4 Offset 64
OpMemberDecorate %_UniformBuffer 4 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %expectedBW RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %expectedWT RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0_5 = OpConstant %float 0.5
%float_1 = OpConstant %float 1
%30 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_1
%float_2_25 = OpConstant %float 2.25
%33 = OpConstantComposite %v4float %float_1 %float_0_5 %float_1 %float_2_25
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%44 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%45 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
%v4bool = OpTypeVector %bool 4
%float_0_25 = OpConstant %float 0.25
%57 = OpConstantComposite %v4float %float_0_25 %float_0_25 %float_0_25 %float_0_25
%float_0_75 = OpConstant %float 0.75
%59 = OpConstantComposite %v4float %float_0_25 %float_0_75 %float_0 %float_1
%70 = OpConstantComposite %v4float %float_0_75 %float_0_75 %float_0_75 %float_0_75
%71 = OpConstantComposite %v4float %float_0_75 %float_0_25 %float_0 %float_1
%82 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%83 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%111 = OpConstantComposite %v2float %float_0_5 %float_0_5
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%128 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
%v3bool = OpTypeVector %bool 3
%142 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%int_4 = OpConstant %int 4
%170 = OpConstantComposite %v2float %float_0 %float_0_5
%185 = OpConstantComposite %v3float %float_0 %float_0_5 %float_0
%198 = OpConstantComposite %v4float %float_0 %float_0_5 %float_0 %float_1
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
%expectedBW = OpVariable %_ptr_Function_v4float Function
%expectedWT = OpVariable %_ptr_Function_v4float Function
%203 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedBW %30
OpStore %expectedWT %33
%36 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%40 = OpLoad %v4float %36
%41 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%43 = OpLoad %v4float %41
%35 = OpExtInst %v4float %1 FMix %40 %43 %44
%46 = OpFOrdEqual %v4bool %35 %45
%48 = OpAll %bool %46
OpSelectionMerge %50 None
OpBranchConditional %48 %49 %50
%49 = OpLabel
%52 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%53 = OpLoad %v4float %52
%54 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%55 = OpLoad %v4float %54
%51 = OpExtInst %v4float %1 FMix %53 %55 %57
%60 = OpFOrdEqual %v4bool %51 %59
%61 = OpAll %bool %60
OpBranch %50
%50 = OpLabel
%62 = OpPhi %bool %false %25 %61 %49
OpSelectionMerge %64 None
OpBranchConditional %62 %63 %64
%63 = OpLabel
%66 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%67 = OpLoad %v4float %66
%68 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%69 = OpLoad %v4float %68
%65 = OpExtInst %v4float %1 FMix %67 %69 %70
%72 = OpFOrdEqual %v4bool %65 %71
%73 = OpAll %bool %72
OpBranch %64
%64 = OpLabel
%74 = OpPhi %bool %false %50 %73 %63
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%78 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%79 = OpLoad %v4float %78
%80 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%81 = OpLoad %v4float %80
%77 = OpExtInst %v4float %1 FMix %79 %81 %82
%84 = OpFOrdEqual %v4bool %77 %83
%85 = OpAll %bool %84
OpBranch %76
%76 = OpLabel
%86 = OpPhi %bool %false %64 %85 %75
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%90 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%92 = OpLoad %v4float %90
%93 = OpCompositeExtract %float %92 0
%94 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%96 = OpLoad %v4float %94
%97 = OpCompositeExtract %float %96 0
%89 = OpExtInst %float %1 FMix %93 %97 %float_0_5
%98 = OpLoad %v4float %expectedBW
%99 = OpCompositeExtract %float %98 0
%100 = OpFOrdEqual %bool %89 %99
OpBranch %88
%88 = OpLabel
%101 = OpPhi %bool %false %76 %100 %87
OpSelectionMerge %103 None
OpBranchConditional %101 %102 %103
%102 = OpLabel
%105 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%106 = OpLoad %v4float %105
%107 = OpVectorShuffle %v2float %106 %106 0 1
%108 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%109 = OpLoad %v4float %108
%110 = OpVectorShuffle %v2float %109 %109 0 1
%104 = OpExtInst %v2float %1 FMix %107 %110 %111
%112 = OpLoad %v4float %expectedBW
%113 = OpVectorShuffle %v2float %112 %112 0 1
%114 = OpFOrdEqual %v2bool %104 %113
%116 = OpAll %bool %114
OpBranch %103
%103 = OpLabel
%117 = OpPhi %bool %false %88 %116 %102
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%121 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%122 = OpLoad %v4float %121
%123 = OpVectorShuffle %v3float %122 %122 0 1 2
%125 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%126 = OpLoad %v4float %125
%127 = OpVectorShuffle %v3float %126 %126 0 1 2
%120 = OpExtInst %v3float %1 FMix %123 %127 %128
%129 = OpLoad %v4float %expectedBW
%130 = OpVectorShuffle %v3float %129 %129 0 1 2
%131 = OpFOrdEqual %v3bool %120 %130
%133 = OpAll %bool %131
OpBranch %119
%119 = OpLabel
%134 = OpPhi %bool %false %103 %133 %118
OpSelectionMerge %136 None
OpBranchConditional %134 %135 %136
%135 = OpLabel
%138 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%139 = OpLoad %v4float %138
%140 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%141 = OpLoad %v4float %140
%137 = OpExtInst %v4float %1 FMix %139 %141 %142
%143 = OpLoad %v4float %expectedBW
%144 = OpFOrdEqual %v4bool %137 %143
%145 = OpAll %bool %144
OpBranch %136
%136 = OpLabel
%146 = OpPhi %bool %false %119 %145 %135
OpSelectionMerge %148 None
OpBranchConditional %146 %147 %148
%147 = OpLabel
%150 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%151 = OpLoad %v4float %150
%152 = OpCompositeExtract %float %151 0
%153 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%155 = OpLoad %v4float %153
%156 = OpCompositeExtract %float %155 0
%149 = OpExtInst %float %1 FMix %152 %156 %float_0
%157 = OpLoad %v4float %expectedWT
%158 = OpCompositeExtract %float %157 0
%159 = OpFOrdEqual %bool %149 %158
OpBranch %148
%148 = OpLabel
%160 = OpPhi %bool %false %136 %159 %147
OpSelectionMerge %162 None
OpBranchConditional %160 %161 %162
%161 = OpLabel
%164 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%165 = OpLoad %v4float %164
%166 = OpVectorShuffle %v2float %165 %165 0 1
%167 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%168 = OpLoad %v4float %167
%169 = OpVectorShuffle %v2float %168 %168 0 1
%163 = OpExtInst %v2float %1 FMix %166 %169 %170
%171 = OpLoad %v4float %expectedWT
%172 = OpVectorShuffle %v2float %171 %171 0 1
%173 = OpFOrdEqual %v2bool %163 %172
%174 = OpAll %bool %173
OpBranch %162
%162 = OpLabel
%175 = OpPhi %bool %false %148 %174 %161
OpSelectionMerge %177 None
OpBranchConditional %175 %176 %177
%176 = OpLabel
%179 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%180 = OpLoad %v4float %179
%181 = OpVectorShuffle %v3float %180 %180 0 1 2
%182 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%183 = OpLoad %v4float %182
%184 = OpVectorShuffle %v3float %183 %183 0 1 2
%178 = OpExtInst %v3float %1 FMix %181 %184 %185
%186 = OpLoad %v4float %expectedWT
%187 = OpVectorShuffle %v3float %186 %186 0 1 2
%188 = OpFOrdEqual %v3bool %178 %187
%189 = OpAll %bool %188
OpBranch %177
%177 = OpLabel
%190 = OpPhi %bool %false %162 %189 %176
OpSelectionMerge %192 None
OpBranchConditional %190 %191 %192
%191 = OpLabel
%194 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%195 = OpLoad %v4float %194
%196 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%197 = OpLoad %v4float %196
%193 = OpExtInst %v4float %1 FMix %195 %197 %198
%199 = OpLoad %v4float %expectedWT
%200 = OpFOrdEqual %v4bool %193 %199
%201 = OpAll %bool %200
OpBranch %192
%192 = OpLabel
%202 = OpPhi %bool %false %177 %201 %191
OpSelectionMerge %206 None
OpBranchConditional %202 %204 %205
%204 = OpLabel
%207 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%208 = OpLoad %v4float %207
OpStore %203 %208
OpBranch %206
%205 = OpLabel
%209 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%210 = OpLoad %v4float %209
OpStore %203 %210
OpBranch %206
%206 = OpLabel
%211 = OpLoad %v4float %203
OpReturnValue %211
OpFunctionEnd
