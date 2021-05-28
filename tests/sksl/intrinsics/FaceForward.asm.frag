OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "N"
OpMemberName %_UniformBuffer 1 "I"
OpMemberName %_UniformBuffer 2 "NRef"
OpMemberName %_UniformBuffer 3 "colorGreen"
OpMemberName %_UniformBuffer 4 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %expectedPos "expectedPos"
OpName %expectedNeg "expectedNeg"
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
OpDecorate %expectedPos RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %expectedNeg RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%32 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%float_n1 = OpConstant %float -1
%float_n2 = OpConstant %float -2
%float_n3 = OpConstant %float -3
%float_n4 = OpConstant %float -4
%38 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n3 %float_n4
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%117 = OpConstantComposite %v2float %float_n1 %float_n2
%125 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
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
%expectedPos = OpVariable %_ptr_Function_v4float Function
%expectedNeg = OpVariable %_ptr_Function_v4float Function
%137 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedPos %32
OpStore %expectedNeg %38
%41 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%45 = OpLoad %v4float %41
%46 = OpCompositeExtract %float %45 0
%47 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%49 = OpLoad %v4float %47
%50 = OpCompositeExtract %float %49 0
%51 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%53 = OpLoad %v4float %51
%54 = OpCompositeExtract %float %53 0
%40 = OpExtInst %float %1 FaceForward %46 %50 %54
%55 = OpLoad %v4float %expectedNeg
%56 = OpCompositeExtract %float %55 0
%57 = OpFOrdEqual %bool %40 %56
OpSelectionMerge %59 None
OpBranchConditional %57 %58 %59
%58 = OpLabel
%61 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%62 = OpLoad %v4float %61
%63 = OpVectorShuffle %v2float %62 %62 0 1
%64 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%65 = OpLoad %v4float %64
%66 = OpVectorShuffle %v2float %65 %65 0 1
%67 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%68 = OpLoad %v4float %67
%69 = OpVectorShuffle %v2float %68 %68 0 1
%60 = OpExtInst %v2float %1 FaceForward %63 %66 %69
%70 = OpLoad %v4float %expectedNeg
%71 = OpVectorShuffle %v2float %70 %70 0 1
%72 = OpFOrdEqual %v2bool %60 %71
%74 = OpAll %bool %72
OpBranch %59
%59 = OpLabel
%75 = OpPhi %bool %false %25 %74 %58
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%79 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%80 = OpLoad %v4float %79
%81 = OpVectorShuffle %v3float %80 %80 0 1 2
%83 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%84 = OpLoad %v4float %83
%85 = OpVectorShuffle %v3float %84 %84 0 1 2
%86 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%87 = OpLoad %v4float %86
%88 = OpVectorShuffle %v3float %87 %87 0 1 2
%78 = OpExtInst %v3float %1 FaceForward %81 %85 %88
%89 = OpLoad %v4float %expectedPos
%90 = OpVectorShuffle %v3float %89 %89 0 1 2
%91 = OpFOrdEqual %v3bool %78 %90
%93 = OpAll %bool %91
OpBranch %77
%77 = OpLabel
%94 = OpPhi %bool %false %59 %93 %76
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%98 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%99 = OpLoad %v4float %98
%100 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%101 = OpLoad %v4float %100
%102 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%103 = OpLoad %v4float %102
%97 = OpExtInst %v4float %1 FaceForward %99 %101 %103
%104 = OpLoad %v4float %expectedPos
%105 = OpFOrdEqual %v4bool %97 %104
%107 = OpAll %bool %105
OpBranch %96
%96 = OpLabel
%108 = OpPhi %bool %false %77 %107 %95
OpSelectionMerge %110 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%111 = OpLoad %v4float %expectedNeg
%112 = OpCompositeExtract %float %111 0
%113 = OpFOrdEqual %bool %float_n1 %112
OpBranch %110
%110 = OpLabel
%114 = OpPhi %bool %false %96 %113 %109
OpSelectionMerge %116 None
OpBranchConditional %114 %115 %116
%115 = OpLabel
%118 = OpLoad %v4float %expectedNeg
%119 = OpVectorShuffle %v2float %118 %118 0 1
%120 = OpFOrdEqual %v2bool %117 %119
%121 = OpAll %bool %120
OpBranch %116
%116 = OpLabel
%122 = OpPhi %bool %false %110 %121 %115
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%126 = OpLoad %v4float %expectedPos
%127 = OpVectorShuffle %v3float %126 %126 0 1 2
%128 = OpFOrdEqual %v3bool %125 %127
%129 = OpAll %bool %128
OpBranch %124
%124 = OpLabel
%130 = OpPhi %bool %false %116 %129 %123
OpSelectionMerge %132 None
OpBranchConditional %130 %131 %132
%131 = OpLabel
%133 = OpLoad %v4float %expectedPos
%134 = OpFOrdEqual %v4bool %32 %133
%135 = OpAll %bool %134
OpBranch %132
%132 = OpLabel
%136 = OpPhi %bool %false %124 %135 %131
OpSelectionMerge %140 None
OpBranchConditional %136 %138 %139
%138 = OpLabel
%141 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%143 = OpLoad %v4float %141
OpStore %137 %143
OpBranch %140
%139 = OpLabel
%144 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%146 = OpLoad %v4float %144
OpStore %137 %146
OpBranch %140
%140 = OpLabel
%147 = OpLoad %v4float %137
OpReturnValue %147
OpFunctionEnd
