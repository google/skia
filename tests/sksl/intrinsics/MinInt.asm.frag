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
OpName %intValues "intValues"
OpName %intGreen "intGreen"
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
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%float_100 = OpConstant %float 100
%int_1 = OpConstant %int 1
%int_n125 = OpConstant %int -125
%int_50 = OpConstant %int 50
%62 = OpConstantComposite %v4int %int_n125 %int_0 %int_50 %int_50
%int_100 = OpConstant %int 100
%65 = OpConstantComposite %v4int %int_n125 %int_0 %int_0 %int_100
%false = OpConstantFalse %bool
%v2int = OpTypeVector %int 2
%79 = OpConstantComposite %v2int %int_50 %int_50
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%92 = OpConstantComposite %v3int %int_50 %int_50 %int_50
%v3bool = OpTypeVector %bool 3
%103 = OpConstantComposite %v4int %int_50 %int_50 %int_50 %int_50
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
%intValues = OpVariable %_ptr_Function_v4int Function
%intGreen = OpVariable %_ptr_Function_v4int Function
%expectedA = OpVariable %_ptr_Function_v4int Function
%expectedB = OpVariable %_ptr_Function_v4int Function
%153 = OpVariable %_ptr_Function_v4float Function
%30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%33 = OpLoad %v4float %30
%35 = OpVectorTimesScalar %v4float %33 %float_100
%36 = OpCompositeExtract %float %35 0
%37 = OpConvertFToS %int %36
%38 = OpCompositeExtract %float %35 1
%39 = OpConvertFToS %int %38
%40 = OpCompositeExtract %float %35 2
%41 = OpConvertFToS %int %40
%42 = OpCompositeExtract %float %35 3
%43 = OpConvertFToS %int %42
%44 = OpCompositeConstruct %v4int %37 %39 %41 %43
OpStore %intValues %44
%46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%48 = OpLoad %v4float %46
%49 = OpVectorTimesScalar %v4float %48 %float_100
%50 = OpCompositeExtract %float %49 0
%51 = OpConvertFToS %int %50
%52 = OpCompositeExtract %float %49 1
%53 = OpConvertFToS %int %52
%54 = OpCompositeExtract %float %49 2
%55 = OpConvertFToS %int %54
%56 = OpCompositeExtract %float %49 3
%57 = OpConvertFToS %int %56
%58 = OpCompositeConstruct %v4int %51 %53 %55 %57
OpStore %intGreen %58
OpStore %expectedA %62
OpStore %expectedB %65
%68 = OpLoad %v4int %intValues
%69 = OpCompositeExtract %int %68 0
%67 = OpExtInst %int %1 SMin %69 %int_50
%70 = OpLoad %v4int %expectedA
%71 = OpCompositeExtract %int %70 0
%72 = OpIEqual %bool %67 %71
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%76 = OpLoad %v4int %intValues
%77 = OpVectorShuffle %v2int %76 %76 0 1
%75 = OpExtInst %v2int %1 SMin %77 %79
%80 = OpLoad %v4int %expectedA
%81 = OpVectorShuffle %v2int %80 %80 0 1
%82 = OpIEqual %v2bool %75 %81
%84 = OpAll %bool %82
OpBranch %74
%74 = OpLabel
%85 = OpPhi %bool %false %25 %84 %73
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%89 = OpLoad %v4int %intValues
%90 = OpVectorShuffle %v3int %89 %89 0 1 2
%88 = OpExtInst %v3int %1 SMin %90 %92
%93 = OpLoad %v4int %expectedA
%94 = OpVectorShuffle %v3int %93 %93 0 1 2
%95 = OpIEqual %v3bool %88 %94
%97 = OpAll %bool %95
OpBranch %87
%87 = OpLabel
%98 = OpPhi %bool %false %74 %97 %86
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%102 = OpLoad %v4int %intValues
%101 = OpExtInst %v4int %1 SMin %102 %103
%104 = OpLoad %v4int %expectedA
%105 = OpIEqual %v4bool %101 %104
%107 = OpAll %bool %105
OpBranch %100
%100 = OpLabel
%108 = OpPhi %bool %false %87 %107 %99
OpSelectionMerge %110 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%112 = OpLoad %v4int %intValues
%113 = OpCompositeExtract %int %112 0
%114 = OpLoad %v4int %intGreen
%115 = OpCompositeExtract %int %114 0
%111 = OpExtInst %int %1 SMin %113 %115
%116 = OpLoad %v4int %expectedB
%117 = OpCompositeExtract %int %116 0
%118 = OpIEqual %bool %111 %117
OpBranch %110
%110 = OpLabel
%119 = OpPhi %bool %false %100 %118 %109
OpSelectionMerge %121 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%123 = OpLoad %v4int %intValues
%124 = OpVectorShuffle %v2int %123 %123 0 1
%125 = OpLoad %v4int %intGreen
%126 = OpVectorShuffle %v2int %125 %125 0 1
%122 = OpExtInst %v2int %1 SMin %124 %126
%127 = OpLoad %v4int %expectedB
%128 = OpVectorShuffle %v2int %127 %127 0 1
%129 = OpIEqual %v2bool %122 %128
%130 = OpAll %bool %129
OpBranch %121
%121 = OpLabel
%131 = OpPhi %bool %false %110 %130 %120
OpSelectionMerge %133 None
OpBranchConditional %131 %132 %133
%132 = OpLabel
%135 = OpLoad %v4int %intValues
%136 = OpVectorShuffle %v3int %135 %135 0 1 2
%137 = OpLoad %v4int %intGreen
%138 = OpVectorShuffle %v3int %137 %137 0 1 2
%134 = OpExtInst %v3int %1 SMin %136 %138
%139 = OpLoad %v4int %expectedB
%140 = OpVectorShuffle %v3int %139 %139 0 1 2
%141 = OpIEqual %v3bool %134 %140
%142 = OpAll %bool %141
OpBranch %133
%133 = OpLabel
%143 = OpPhi %bool %false %121 %142 %132
OpSelectionMerge %145 None
OpBranchConditional %143 %144 %145
%144 = OpLabel
%147 = OpLoad %v4int %intValues
%148 = OpLoad %v4int %intGreen
%146 = OpExtInst %v4int %1 SMin %147 %148
%149 = OpLoad %v4int %expectedB
%150 = OpIEqual %v4bool %146 %149
%151 = OpAll %bool %150
OpBranch %145
%145 = OpLabel
%152 = OpPhi %bool %false %133 %151 %144
OpSelectionMerge %157 None
OpBranchConditional %152 %155 %156
%155 = OpLabel
%158 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%159 = OpLoad %v4float %158
OpStore %153 %159
OpBranch %157
%156 = OpLabel
%160 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%162 = OpLoad %v4float %160
OpStore %153 %162
OpBranch %157
%157 = OpLabel
%163 = OpLoad %v4float %153
OpReturnValue %163
OpFunctionEnd
