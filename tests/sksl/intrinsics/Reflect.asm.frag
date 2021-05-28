OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "I"
OpMemberName %_UniformBuffer 1 "N"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %expectedX "expectedX"
OpName %expectedXY "expectedXY"
OpName %expectedXYZ "expectedXYZ"
OpName %expectedXYZW "expectedXYZW"
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
OpDecorate %expectedX RelaxedPrecision
OpDecorate %expectedXY RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %expectedXYZ RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %expectedXYZW RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
%float_n49 = OpConstant %float -49
%float_n169 = OpConstant %float -169
%float_202 = OpConstant %float 202
%32 = OpConstantComposite %v2float %float_n169 %float_202
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%float_n379 = OpConstant %float -379
%float_454 = OpConstant %float 454
%float_n529 = OpConstant %float -529
%39 = OpConstantComposite %v3float %float_n379 %float_454 %float_n529
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n699 = OpConstant %float -699
%float_838 = OpConstant %float 838
%float_n977 = OpConstant %float -977
%float_1116 = OpConstant %float 1116
%46 = OpConstantComposite %v4float %float_n699 %float_838 %float_n977 %float_1116
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%v2bool = OpTypeVector %bool 2
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
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
%expectedX = OpVariable %_ptr_Function_float Function
%expectedXY = OpVariable %_ptr_Function_v2float Function
%expectedXYZ = OpVariable %_ptr_Function_v3float Function
%expectedXYZW = OpVariable %_ptr_Function_v4float Function
%124 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedX %float_n49
OpStore %expectedXY %32
OpStore %expectedXYZ %39
OpStore %expectedXYZW %46
%49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%53 = OpLoad %v4float %49
%54 = OpCompositeExtract %float %53 0
%55 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%57 = OpLoad %v4float %55
%58 = OpCompositeExtract %float %57 0
%48 = OpExtInst %float %1 Reflect %54 %58
%59 = OpLoad %float %expectedX
%60 = OpFOrdEqual %bool %48 %59
OpSelectionMerge %62 None
OpBranchConditional %60 %61 %62
%61 = OpLabel
%64 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%65 = OpLoad %v4float %64
%66 = OpVectorShuffle %v2float %65 %65 0 1
%67 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%68 = OpLoad %v4float %67
%69 = OpVectorShuffle %v2float %68 %68 0 1
%63 = OpExtInst %v2float %1 Reflect %66 %69
%70 = OpLoad %v2float %expectedXY
%71 = OpFOrdEqual %v2bool %63 %70
%73 = OpAll %bool %71
OpBranch %62
%62 = OpLabel
%74 = OpPhi %bool %false %25 %73 %61
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%78 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%79 = OpLoad %v4float %78
%80 = OpVectorShuffle %v3float %79 %79 0 1 2
%81 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%82 = OpLoad %v4float %81
%83 = OpVectorShuffle %v3float %82 %82 0 1 2
%77 = OpExtInst %v3float %1 Reflect %80 %83
%84 = OpLoad %v3float %expectedXYZ
%85 = OpFOrdEqual %v3bool %77 %84
%87 = OpAll %bool %85
OpBranch %76
%76 = OpLabel
%88 = OpPhi %bool %false %62 %87 %75
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%92 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%93 = OpLoad %v4float %92
%94 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%95 = OpLoad %v4float %94
%91 = OpExtInst %v4float %1 Reflect %93 %95
%96 = OpLoad %v4float %expectedXYZW
%97 = OpFOrdEqual %v4bool %91 %96
%99 = OpAll %bool %97
OpBranch %90
%90 = OpLabel
%100 = OpPhi %bool %false %76 %99 %89
OpSelectionMerge %102 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%103 = OpLoad %float %expectedX
%104 = OpFOrdEqual %bool %float_n49 %103
OpBranch %102
%102 = OpLabel
%105 = OpPhi %bool %false %90 %104 %101
OpSelectionMerge %107 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
%108 = OpLoad %v2float %expectedXY
%109 = OpFOrdEqual %v2bool %32 %108
%110 = OpAll %bool %109
OpBranch %107
%107 = OpLabel
%111 = OpPhi %bool %false %102 %110 %106
OpSelectionMerge %113 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%114 = OpLoad %v3float %expectedXYZ
%115 = OpFOrdEqual %v3bool %39 %114
%116 = OpAll %bool %115
OpBranch %113
%113 = OpLabel
%117 = OpPhi %bool %false %107 %116 %112
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%120 = OpLoad %v4float %expectedXYZW
%121 = OpFOrdEqual %v4bool %46 %120
%122 = OpAll %bool %121
OpBranch %119
%119 = OpLabel
%123 = OpPhi %bool %false %113 %122 %118
OpSelectionMerge %127 None
OpBranchConditional %123 %125 %126
%125 = OpLabel
%128 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%130 = OpLoad %v4float %128
OpStore %124 %130
OpBranch %127
%126 = OpLabel
%131 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%133 = OpLoad %v4float %131
OpStore %124 %133
OpBranch %127
%127 = OpLabel
%134 = OpLoad %v4float %124
OpReturnValue %134
OpFunctionEnd
