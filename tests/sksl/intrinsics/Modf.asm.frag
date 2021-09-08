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
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %value "value"
OpName %expectedWhole "expectedWhole"
OpName %expectedFraction "expectedFraction"
OpName %ok "ok"
OpName %whole "whole"
OpName %fraction "fraction"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %137 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
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
%float_2_5 = OpConstant %float 2.5
%float_n2_5 = OpConstant %float -2.5
%float_8 = OpConstant %float 8
%float_n0_125 = OpConstant %float -0.125
%32 = OpConstantComposite %v4float %float_2_5 %float_n2_5 %float_8 %float_n0_125
%float_2 = OpConstant %float 2
%float_n2 = OpConstant %float -2
%36 = OpConstantComposite %v4float %float_2 %float_n2 %float_8 %float_0
%float_0_5 = OpConstant %float 0.5
%float_n0_5 = OpConstant %float -0.5
%40 = OpConstantComposite %v4float %float_0_5 %float_n0_5 %float_0 %float_n0_125
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%false = OpConstantFalse %bool
%45 = OpConstantComposite %v4bool %false %false %false %false
%_ptr_Function_float = OpTypePointer Function %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_bool = OpTypePointer Function %bool
%80 = OpConstantComposite %v2float %float_2 %float_n2
%v2bool = OpTypeVector %bool 2
%88 = OpConstantComposite %v2float %float_0_5 %float_n0_5
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%107 = OpConstantComposite %v3float %float_2 %float_n2 %float_8
%v3bool = OpTypeVector %bool 3
%115 = OpConstantComposite %v3float %float_0_5 %float_n0_5 %float_0
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
%value = OpVariable %_ptr_Function_v4float Function
%expectedWhole = OpVariable %_ptr_Function_v4float Function
%expectedFraction = OpVariable %_ptr_Function_v4float Function
%ok = OpVariable %_ptr_Function_v4bool Function
%whole = OpVariable %_ptr_Function_v4float Function
%fraction = OpVariable %_ptr_Function_v4float Function
%55 = OpVariable %_ptr_Function_float Function
%72 = OpVariable %_ptr_Function_v2float Function
%98 = OpVariable %_ptr_Function_v3float Function
%138 = OpVariable %_ptr_Function_v4float Function
OpStore %value %32
OpStore %expectedWhole %36
OpStore %expectedFraction %40
OpStore %ok %45
%49 = OpLoad %v4float %value
%50 = OpCompositeExtract %float %49 0
%51 = OpAccessChain %_ptr_Function_float %whole %int_0
%48 = OpExtInst %float %1 Modf %50 %55
%56 = OpLoad %float %55
OpStore %51 %56
%57 = OpAccessChain %_ptr_Function_float %fraction %int_0
OpStore %57 %48
%58 = OpLoad %v4float %whole
%59 = OpCompositeExtract %float %58 0
%60 = OpFOrdEqual %bool %59 %float_2
OpSelectionMerge %62 None
OpBranchConditional %60 %61 %62
%61 = OpLabel
%63 = OpLoad %v4float %fraction
%64 = OpCompositeExtract %float %63 0
%65 = OpFOrdEqual %bool %64 %float_0_5
OpBranch %62
%62 = OpLabel
%66 = OpPhi %bool %false %25 %65 %61
%67 = OpAccessChain %_ptr_Function_bool %ok %int_0
OpStore %67 %66
%70 = OpLoad %v4float %value
%71 = OpVectorShuffle %v2float %70 %70 0 1
%69 = OpExtInst %v2float %1 Modf %71 %72
%73 = OpLoad %v2float %72
%74 = OpLoad %v4float %whole
%75 = OpVectorShuffle %v4float %74 %73 4 5 2 3
OpStore %whole %75
%76 = OpLoad %v4float %fraction
%77 = OpVectorShuffle %v4float %76 %69 4 5 2 3
OpStore %fraction %77
%78 = OpLoad %v4float %whole
%79 = OpVectorShuffle %v2float %78 %78 0 1
%81 = OpFOrdEqual %v2bool %79 %80
%83 = OpAll %bool %81
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%86 = OpLoad %v4float %fraction
%87 = OpVectorShuffle %v2float %86 %86 0 1
%89 = OpFOrdEqual %v2bool %87 %88
%90 = OpAll %bool %89
OpBranch %85
%85 = OpLabel
%91 = OpPhi %bool %false %62 %90 %84
%92 = OpAccessChain %_ptr_Function_bool %ok %int_1
OpStore %92 %91
%95 = OpLoad %v4float %value
%96 = OpVectorShuffle %v3float %95 %95 0 1 2
%94 = OpExtInst %v3float %1 Modf %96 %98
%100 = OpLoad %v3float %98
%101 = OpLoad %v4float %whole
%102 = OpVectorShuffle %v4float %101 %100 4 5 6 3
OpStore %whole %102
%103 = OpLoad %v4float %fraction
%104 = OpVectorShuffle %v4float %103 %94 4 5 6 3
OpStore %fraction %104
%105 = OpLoad %v4float %whole
%106 = OpVectorShuffle %v3float %105 %105 0 1 2
%108 = OpFOrdEqual %v3bool %106 %107
%110 = OpAll %bool %108
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%113 = OpLoad %v4float %fraction
%114 = OpVectorShuffle %v3float %113 %113 0 1 2
%116 = OpFOrdEqual %v3bool %114 %115
%117 = OpAll %bool %116
OpBranch %112
%112 = OpLabel
%118 = OpPhi %bool %false %85 %117 %111
%119 = OpAccessChain %_ptr_Function_bool %ok %int_2
OpStore %119 %118
%122 = OpLoad %v4float %value
%121 = OpExtInst %v4float %1 Modf %122 %whole
OpStore %fraction %121
%123 = OpLoad %v4float %whole
%124 = OpLoad %v4float %expectedWhole
%125 = OpFOrdEqual %v4bool %123 %124
%126 = OpAll %bool %125
OpSelectionMerge %128 None
OpBranchConditional %126 %127 %128
%127 = OpLabel
%129 = OpLoad %v4float %fraction
%130 = OpLoad %v4float %expectedFraction
%131 = OpFOrdEqual %v4bool %129 %130
%132 = OpAll %bool %131
OpBranch %128
%128 = OpLabel
%133 = OpPhi %bool %false %112 %132 %127
%134 = OpAccessChain %_ptr_Function_bool %ok %int_3
OpStore %134 %133
%137 = OpLoad %v4bool %ok
%136 = OpAll %bool %137
OpSelectionMerge %141 None
OpBranchConditional %136 %139 %140
%139 = OpLabel
%142 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%144 = OpLoad %v4float %142
OpStore %138 %144
OpBranch %141
%140 = OpLabel
%145 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%146 = OpLoad %v4float %145
OpStore %138 %146
OpBranch %141
%141 = OpLabel
%147 = OpLoad %v4float %138
OpReturnValue %147
OpFunctionEnd
