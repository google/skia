OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %expectedA "expectedA"
OpName %intValues "intValues"
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
OpDecorate %32 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
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
%18 = OpTypeFunction %v4float
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_n100 = OpConstant %int -100
%int_0 = OpConstant %int 0
%int_75 = OpConstant %int 75
%int_100 = OpConstant %int 100
%28 = OpConstantComposite %v4int %int_n100 %int_0 %int_75 %int_100
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%float_100 = OpConstant %float 100
%int_n200 = OpConstant %int -200
%46 = OpConstantComposite %v4int %int_n100 %int_n200 %int_n200 %int_100
%int_50 = OpConstant %int 50
%int_225 = OpConstant %int 225
%50 = OpConstantComposite %v4int %int_n100 %int_0 %int_50 %int_225
%int_200 = OpConstant %int 200
%int_300 = OpConstant %int 300
%54 = OpConstantComposite %v4int %int_100 %int_200 %int_50 %int_300
%false = OpConstantFalse %bool
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%expectedA = OpVariable %_ptr_Function_v4int Function
%intValues = OpVariable %_ptr_Function_v4int Function
%clampLow = OpVariable %_ptr_Function_v4int Function
%expectedB = OpVariable %_ptr_Function_v4int Function
%clampHigh = OpVariable %_ptr_Function_v4int Function
%152 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedA %28
%30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %30
%34 = OpVectorTimesScalar %v4float %32 %float_100
%35 = OpCompositeExtract %float %34 0
%36 = OpConvertFToS %int %35
%37 = OpCompositeExtract %float %34 1
%38 = OpConvertFToS %int %37
%39 = OpCompositeExtract %float %34 2
%40 = OpConvertFToS %int %39
%41 = OpCompositeExtract %float %34 3
%42 = OpConvertFToS %int %41
%43 = OpCompositeConstruct %v4int %36 %38 %40 %42
OpStore %intValues %43
OpStore %clampLow %46
OpStore %expectedB %50
OpStore %clampHigh %54
%57 = OpLoad %v4int %intValues
%58 = OpCompositeExtract %int %57 0
%56 = OpExtInst %int %1 SClamp %58 %int_n100 %int_100
%59 = OpLoad %v4int %expectedA
%60 = OpCompositeExtract %int %59 0
%61 = OpIEqual %bool %56 %60
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%65 = OpLoad %v4int %intValues
%66 = OpVectorShuffle %v2int %65 %65 0 1
%68 = OpCompositeConstruct %v2int %int_n100 %int_n100
%69 = OpCompositeConstruct %v2int %int_100 %int_100
%64 = OpExtInst %v2int %1 SClamp %66 %68 %69
%70 = OpLoad %v4int %expectedA
%71 = OpVectorShuffle %v2int %70 %70 0 1
%72 = OpIEqual %v2bool %64 %71
%74 = OpAll %bool %72
OpBranch %63
%63 = OpLabel
%75 = OpPhi %bool %false %19 %74 %62
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%79 = OpLoad %v4int %intValues
%80 = OpVectorShuffle %v3int %79 %79 0 1 2
%82 = OpCompositeConstruct %v3int %int_n100 %int_n100 %int_n100
%83 = OpCompositeConstruct %v3int %int_100 %int_100 %int_100
%78 = OpExtInst %v3int %1 SClamp %80 %82 %83
%84 = OpLoad %v4int %expectedA
%85 = OpVectorShuffle %v3int %84 %84 0 1 2
%86 = OpIEqual %v3bool %78 %85
%88 = OpAll %bool %86
OpBranch %77
%77 = OpLabel
%89 = OpPhi %bool %false %63 %88 %76
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%93 = OpLoad %v4int %intValues
%94 = OpCompositeConstruct %v4int %int_n100 %int_n100 %int_n100 %int_n100
%95 = OpCompositeConstruct %v4int %int_100 %int_100 %int_100 %int_100
%92 = OpExtInst %v4int %1 SClamp %93 %94 %95
%96 = OpLoad %v4int %expectedA
%97 = OpIEqual %v4bool %92 %96
%99 = OpAll %bool %97
OpBranch %91
%91 = OpLabel
%100 = OpPhi %bool %false %77 %99 %90
OpSelectionMerge %102 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%104 = OpLoad %v4int %intValues
%105 = OpCompositeExtract %int %104 0
%106 = OpLoad %v4int %clampLow
%107 = OpCompositeExtract %int %106 0
%108 = OpLoad %v4int %clampHigh
%109 = OpCompositeExtract %int %108 0
%103 = OpExtInst %int %1 SClamp %105 %107 %109
%110 = OpLoad %v4int %expectedB
%111 = OpCompositeExtract %int %110 0
%112 = OpIEqual %bool %103 %111
OpBranch %102
%102 = OpLabel
%113 = OpPhi %bool %false %91 %112 %101
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%117 = OpLoad %v4int %intValues
%118 = OpVectorShuffle %v2int %117 %117 0 1
%119 = OpLoad %v4int %clampLow
%120 = OpVectorShuffle %v2int %119 %119 0 1
%121 = OpLoad %v4int %clampHigh
%122 = OpVectorShuffle %v2int %121 %121 0 1
%116 = OpExtInst %v2int %1 SClamp %118 %120 %122
%123 = OpLoad %v4int %expectedB
%124 = OpVectorShuffle %v2int %123 %123 0 1
%125 = OpIEqual %v2bool %116 %124
%126 = OpAll %bool %125
OpBranch %115
%115 = OpLabel
%127 = OpPhi %bool %false %102 %126 %114
OpSelectionMerge %129 None
OpBranchConditional %127 %128 %129
%128 = OpLabel
%131 = OpLoad %v4int %intValues
%132 = OpVectorShuffle %v3int %131 %131 0 1 2
%133 = OpLoad %v4int %clampLow
%134 = OpVectorShuffle %v3int %133 %133 0 1 2
%135 = OpLoad %v4int %clampHigh
%136 = OpVectorShuffle %v3int %135 %135 0 1 2
%130 = OpExtInst %v3int %1 SClamp %132 %134 %136
%137 = OpLoad %v4int %expectedB
%138 = OpVectorShuffle %v3int %137 %137 0 1 2
%139 = OpIEqual %v3bool %130 %138
%140 = OpAll %bool %139
OpBranch %129
%129 = OpLabel
%141 = OpPhi %bool %false %115 %140 %128
OpSelectionMerge %143 None
OpBranchConditional %141 %142 %143
%142 = OpLabel
%145 = OpLoad %v4int %intValues
%146 = OpLoad %v4int %clampLow
%147 = OpLoad %v4int %clampHigh
%144 = OpExtInst %v4int %1 SClamp %145 %146 %147
%148 = OpLoad %v4int %expectedB
%149 = OpIEqual %v4bool %144 %148
%150 = OpAll %bool %149
OpBranch %143
%143 = OpLabel
%151 = OpPhi %bool %false %129 %150 %142
OpSelectionMerge %156 None
OpBranchConditional %151 %154 %155
%154 = OpLabel
%157 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%159 = OpLoad %v4float %157
OpStore %152 %159
OpBranch %156
%155 = OpLabel
%160 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%162 = OpLoad %v4float %160
OpStore %152 %162
OpBranch %156
%156 = OpLabel
%163 = OpLoad %v4float %152
OpReturnValue %163
OpFunctionEnd
