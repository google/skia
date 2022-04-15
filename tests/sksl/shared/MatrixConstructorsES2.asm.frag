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
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %f4 "f4"
OpName %ok "ok"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %64 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_ptr_Function_bool = OpTypePointer Function %bool
%v3float = OpTypeVector %float 3
%float_4 = OpConstant %float 4
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%54 = OpConstantComposite %v2float %float_1 %float_2
%55 = OpConstantComposite %v2float %float_3 %float_4
%56 = OpConstantComposite %mat2v2float %54 %55
%v2bool = OpTypeVector %bool 2
%false = OpConstantFalse %bool
%mat3v3float = OpTypeMatrix %v3float 3
%87 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%88 = OpConstantComposite %v3float %float_4 %float_1 %float_2
%89 = OpConstantComposite %v3float %float_3 %float_4 %float_1
%90 = OpConstantComposite %mat3v3float %87 %88 %89
%v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
%130 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%131 = OpConstantComposite %mat4v4float %130 %130 %130 %130
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %24
%25 = OpFunctionParameter %_ptr_Function_v2float
%26 = OpLabel
%f4 = OpVariable %_ptr_Function_v4float Function
%ok = OpVariable %_ptr_Function_bool Function
%146 = OpVariable %_ptr_Function_v4float Function
%29 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%33 = OpLoad %mat2v2float %29
%34 = OpCompositeExtract %float %33 0 0
%35 = OpCompositeExtract %float %33 0 1
%36 = OpCompositeExtract %float %33 1 0
%37 = OpCompositeExtract %float %33 1 1
%38 = OpCompositeConstruct %v4float %34 %35 %36 %37
OpStore %f4 %38
%41 = OpLoad %v4float %f4
%42 = OpVectorShuffle %v3float %41 %41 0 1 2
%45 = OpCompositeExtract %float %42 0
%46 = OpCompositeExtract %float %42 1
%47 = OpCompositeConstruct %v2float %45 %46
%48 = OpCompositeExtract %float %42 2
%49 = OpCompositeConstruct %v2float %48 %float_4
%50 = OpCompositeConstruct %mat2v2float %47 %49
%58 = OpFOrdEqual %v2bool %47 %54
%59 = OpAll %bool %58
%60 = OpFOrdEqual %v2bool %49 %55
%61 = OpAll %bool %60
%62 = OpLogicalAnd %bool %59 %61
OpStore %ok %62
%64 = OpLoad %bool %ok
OpSelectionMerge %66 None
OpBranchConditional %64 %65 %66
%65 = OpLabel
%67 = OpLoad %v4float %f4
%68 = OpVectorShuffle %v2float %67 %67 0 1
%69 = OpLoad %v4float %f4
%70 = OpVectorShuffle %v2float %69 %69 2 3
%71 = OpLoad %v4float %f4
%72 = OpLoad %v4float %f4
%73 = OpCompositeExtract %float %72 0
%74 = OpCompositeExtract %float %68 0
%75 = OpCompositeExtract %float %68 1
%76 = OpCompositeExtract %float %70 0
%77 = OpCompositeConstruct %v3float %74 %75 %76
%78 = OpCompositeExtract %float %70 1
%79 = OpCompositeExtract %float %71 0
%80 = OpCompositeExtract %float %71 1
%81 = OpCompositeConstruct %v3float %78 %79 %80
%82 = OpCompositeExtract %float %71 2
%83 = OpCompositeExtract %float %71 3
%84 = OpCompositeConstruct %v3float %82 %83 %73
%86 = OpCompositeConstruct %mat3v3float %77 %81 %84
%92 = OpFOrdEqual %v3bool %77 %87
%93 = OpAll %bool %92
%94 = OpFOrdEqual %v3bool %81 %88
%95 = OpAll %bool %94
%96 = OpLogicalAnd %bool %93 %95
%97 = OpFOrdEqual %v3bool %84 %89
%98 = OpAll %bool %97
%99 = OpLogicalAnd %bool %96 %98
OpBranch %66
%66 = OpLabel
%100 = OpPhi %bool %false %26 %99 %65
OpStore %ok %100
%101 = OpLoad %bool %ok
OpSelectionMerge %103 None
OpBranchConditional %101 %102 %103
%102 = OpLabel
%104 = OpLoad %v4float %f4
%105 = OpVectorShuffle %v3float %104 %104 0 1 2
%106 = OpLoad %v4float %f4
%107 = OpVectorShuffle %v3float %106 %106 3 0 1
%108 = OpLoad %v4float %f4
%109 = OpVectorShuffle %v4float %108 %108 2 3 0 1
%110 = OpLoad %v4float %f4
%111 = OpVectorShuffle %v2float %110 %110 2 3
%112 = OpLoad %v4float %f4
%113 = OpCompositeExtract %float %105 0
%114 = OpCompositeExtract %float %105 1
%115 = OpCompositeExtract %float %105 2
%116 = OpCompositeExtract %float %107 0
%117 = OpCompositeConstruct %v4float %113 %114 %115 %116
%118 = OpCompositeExtract %float %107 1
%119 = OpCompositeExtract %float %107 2
%120 = OpCompositeExtract %float %109 0
%121 = OpCompositeExtract %float %109 1
%122 = OpCompositeConstruct %v4float %118 %119 %120 %121
%123 = OpCompositeExtract %float %109 2
%124 = OpCompositeExtract %float %109 3
%125 = OpCompositeExtract %float %111 0
%126 = OpCompositeExtract %float %111 1
%127 = OpCompositeConstruct %v4float %123 %124 %125 %126
%129 = OpCompositeConstruct %mat4v4float %117 %122 %127 %112
%133 = OpFOrdEqual %v4bool %117 %130
%134 = OpAll %bool %133
%135 = OpFOrdEqual %v4bool %122 %130
%136 = OpAll %bool %135
%137 = OpLogicalAnd %bool %134 %136
%138 = OpFOrdEqual %v4bool %127 %130
%139 = OpAll %bool %138
%140 = OpLogicalAnd %bool %137 %139
%141 = OpFOrdEqual %v4bool %112 %130
%142 = OpAll %bool %141
%143 = OpLogicalAnd %bool %140 %142
OpBranch %103
%103 = OpLabel
%144 = OpPhi %bool %false %66 %143 %102
OpStore %ok %144
%145 = OpLoad %bool %ok
OpSelectionMerge %149 None
OpBranchConditional %145 %147 %148
%147 = OpLabel
%150 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%153 = OpLoad %v4float %150
OpStore %146 %153
OpBranch %149
%148 = OpLabel
%154 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%156 = OpLoad %v4float %154
OpStore %146 %156
OpBranch %149
%149 = OpLabel
%157 = OpLoad %v4float %146
OpReturnValue %157
OpFunctionEnd
