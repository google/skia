OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %uintValues "uintValues"
OpName %expectedA "expectedA"
OpName %clampLow "clampLow"
OpName %expectedB "expectedB"
OpName %clampHigh "clampHigh"
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
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
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
%uint = OpTypeInt 32 0
%v4uint = OpTypeVector %uint 4
%_ptr_Function_v4uint = OpTypePointer Function %v4uint
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_100 = OpConstant %float 100
%float_200 = OpConstant %float 200
%38 = OpConstantComposite %v4float %float_200 %float_200 %float_200 %float_200
%uint_100 = OpConstant %uint 100
%uint_200 = OpConstant %uint 200
%uint_275 = OpConstant %uint 275
%uint_300 = OpConstant %uint 300
%54 = OpConstantComposite %v4uint %uint_100 %uint_200 %uint_275 %uint_300
%uint_0 = OpConstant %uint 0
%57 = OpConstantComposite %v4uint %uint_100 %uint_0 %uint_0 %uint_300
%uint_250 = OpConstant %uint 250
%uint_425 = OpConstant %uint 425
%61 = OpConstantComposite %v4uint %uint_100 %uint_200 %uint_250 %uint_425
%uint_400 = OpConstant %uint 400
%uint_500 = OpConstant %uint 500
%65 = OpConstantComposite %v4uint %uint_300 %uint_400 %uint_250 %uint_500
%false = OpConstantFalse %bool
%v2uint = OpTypeVector %uint 2
%75 = OpConstantComposite %v2uint %uint_100 %uint_100
%76 = OpConstantComposite %v2uint %uint_300 %uint_300
%v2bool = OpTypeVector %bool 2
%v3uint = OpTypeVector %uint 3
%87 = OpConstantComposite %v3uint %uint_100 %uint_100 %uint_100
%88 = OpConstantComposite %v3uint %uint_300 %uint_300 %uint_300
%v3bool = OpTypeVector %bool 3
%97 = OpConstantComposite %v4uint %uint_100 %uint_100 %uint_100 %uint_100
%98 = OpConstantComposite %v4uint %uint_300 %uint_300 %uint_300 %uint_300
%v4bool = OpTypeVector %bool 4
%true = OpConstantTrue %bool
%109 = OpConstantComposite %v2uint %uint_100 %uint_200
%116 = OpConstantComposite %v3uint %uint_100 %uint_200 %uint_275
%133 = OpConstantComposite %v2uint %uint_100 %uint_0
%134 = OpConstantComposite %v2uint %uint_300 %uint_400
%143 = OpConstantComposite %v3uint %uint_100 %uint_0 %uint_0
%144 = OpConstantComposite %v3uint %uint_300 %uint_400 %uint_250
%166 = OpConstantComposite %v3uint %uint_100 %uint_200 %uint_250
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
%uintValues = OpVariable %_ptr_Function_v4uint Function
%expectedA = OpVariable %_ptr_Function_v4uint Function
%clampLow = OpVariable %_ptr_Function_v4uint Function
%expectedB = OpVariable %_ptr_Function_v4uint Function
%clampHigh = OpVariable %_ptr_Function_v4uint Function
%174 = OpVariable %_ptr_Function_v4float Function
%30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%34 = OpLoad %v4float %30
%36 = OpVectorTimesScalar %v4float %34 %float_100
%39 = OpFAdd %v4float %36 %38
%40 = OpCompositeExtract %float %39 0
%41 = OpConvertFToU %uint %40
%42 = OpCompositeExtract %float %39 1
%43 = OpConvertFToU %uint %42
%44 = OpCompositeExtract %float %39 2
%45 = OpConvertFToU %uint %44
%46 = OpCompositeExtract %float %39 3
%47 = OpConvertFToU %uint %46
%48 = OpCompositeConstruct %v4uint %41 %43 %45 %47
OpStore %uintValues %48
OpStore %expectedA %54
OpStore %clampLow %57
OpStore %expectedB %61
OpStore %clampHigh %65
%68 = OpCompositeExtract %uint %48 0
%67 = OpExtInst %uint %1 UClamp %68 %uint_100 %uint_300
%69 = OpIEqual %bool %67 %uint_100
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%73 = OpVectorShuffle %v2uint %48 %48 0 1
%72 = OpExtInst %v2uint %1 UClamp %73 %75 %76
%77 = OpVectorShuffle %v2uint %54 %54 0 1
%78 = OpIEqual %v2bool %72 %77
%80 = OpAll %bool %78
OpBranch %71
%71 = OpLabel
%81 = OpPhi %bool %false %25 %80 %70
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%85 = OpVectorShuffle %v3uint %48 %48 0 1 2
%84 = OpExtInst %v3uint %1 UClamp %85 %87 %88
%89 = OpVectorShuffle %v3uint %54 %54 0 1 2
%90 = OpIEqual %v3bool %84 %89
%92 = OpAll %bool %90
OpBranch %83
%83 = OpLabel
%93 = OpPhi %bool %false %71 %92 %82
OpSelectionMerge %95 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%96 = OpExtInst %v4uint %1 UClamp %48 %97 %98
%99 = OpIEqual %v4bool %96 %54
%101 = OpAll %bool %99
OpBranch %95
%95 = OpLabel
%102 = OpPhi %bool %false %83 %101 %94
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
OpBranch %104
%104 = OpLabel
%106 = OpPhi %bool %false %95 %true %103
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%110 = OpVectorShuffle %v2uint %54 %54 0 1
%111 = OpIEqual %v2bool %109 %110
%112 = OpAll %bool %111
OpBranch %108
%108 = OpLabel
%113 = OpPhi %bool %false %104 %112 %107
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%117 = OpVectorShuffle %v3uint %54 %54 0 1 2
%118 = OpIEqual %v3bool %116 %117
%119 = OpAll %bool %118
OpBranch %115
%115 = OpLabel
%120 = OpPhi %bool %false %108 %119 %114
OpSelectionMerge %122 None
OpBranchConditional %120 %121 %122
%121 = OpLabel
OpBranch %122
%122 = OpLabel
%123 = OpPhi %bool %false %115 %true %121
OpSelectionMerge %125 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
%126 = OpExtInst %uint %1 UClamp %68 %uint_100 %uint_300
%127 = OpIEqual %bool %126 %uint_100
OpBranch %125
%125 = OpLabel
%128 = OpPhi %bool %false %122 %127 %124
OpSelectionMerge %130 None
OpBranchConditional %128 %129 %130
%129 = OpLabel
%132 = OpVectorShuffle %v2uint %48 %48 0 1
%131 = OpExtInst %v2uint %1 UClamp %132 %133 %134
%135 = OpVectorShuffle %v2uint %61 %61 0 1
%136 = OpIEqual %v2bool %131 %135
%137 = OpAll %bool %136
OpBranch %130
%130 = OpLabel
%138 = OpPhi %bool %false %125 %137 %129
OpSelectionMerge %140 None
OpBranchConditional %138 %139 %140
%139 = OpLabel
%142 = OpVectorShuffle %v3uint %48 %48 0 1 2
%141 = OpExtInst %v3uint %1 UClamp %142 %143 %144
%145 = OpVectorShuffle %v3uint %61 %61 0 1 2
%146 = OpIEqual %v3bool %141 %145
%147 = OpAll %bool %146
OpBranch %140
%140 = OpLabel
%148 = OpPhi %bool %false %130 %147 %139
OpSelectionMerge %150 None
OpBranchConditional %148 %149 %150
%149 = OpLabel
%151 = OpExtInst %v4uint %1 UClamp %48 %57 %65
%152 = OpIEqual %v4bool %151 %61
%153 = OpAll %bool %152
OpBranch %150
%150 = OpLabel
%154 = OpPhi %bool %false %140 %153 %149
OpSelectionMerge %156 None
OpBranchConditional %154 %155 %156
%155 = OpLabel
OpBranch %156
%156 = OpLabel
%157 = OpPhi %bool %false %150 %true %155
OpSelectionMerge %159 None
OpBranchConditional %157 %158 %159
%158 = OpLabel
%160 = OpVectorShuffle %v2uint %61 %61 0 1
%161 = OpIEqual %v2bool %109 %160
%162 = OpAll %bool %161
OpBranch %159
%159 = OpLabel
%163 = OpPhi %bool %false %156 %162 %158
OpSelectionMerge %165 None
OpBranchConditional %163 %164 %165
%164 = OpLabel
%167 = OpVectorShuffle %v3uint %61 %61 0 1 2
%168 = OpIEqual %v3bool %166 %167
%169 = OpAll %bool %168
OpBranch %165
%165 = OpLabel
%170 = OpPhi %bool %false %159 %169 %164
OpSelectionMerge %172 None
OpBranchConditional %170 %171 %172
%171 = OpLabel
OpBranch %172
%172 = OpLabel
%173 = OpPhi %bool %false %165 %true %171
OpSelectionMerge %178 None
OpBranchConditional %173 %176 %177
%176 = OpLabel
%179 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%181 = OpLoad %v4float %179
OpStore %174 %181
OpBranch %178
%177 = OpLabel
%182 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%184 = OpLoad %v4float %182
OpStore %174 %184
OpBranch %178
%178 = OpLabel
%185 = OpLoad %v4float %174
OpReturnValue %185
OpFunctionEnd
