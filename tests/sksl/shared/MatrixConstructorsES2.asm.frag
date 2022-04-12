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
OpDecorate %66 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
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
%89 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%90 = OpConstantComposite %v3float %float_4 %float_1 %float_2
%91 = OpConstantComposite %v3float %float_3 %float_4 %float_1
%92 = OpConstantComposite %mat3v3float %89 %90 %91
%v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
%135 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%136 = OpConstantComposite %mat4v4float %135 %135 %135 %135
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
%155 = OpVariable %_ptr_Function_v4float Function
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
%58 = OpCompositeExtract %v2float %50 0
%59 = OpFOrdEqual %v2bool %58 %54
%60 = OpAll %bool %59
%61 = OpCompositeExtract %v2float %50 1
%62 = OpFOrdEqual %v2bool %61 %55
%63 = OpAll %bool %62
%64 = OpLogicalAnd %bool %60 %63
OpStore %ok %64
%66 = OpLoad %bool %ok
OpSelectionMerge %68 None
OpBranchConditional %66 %67 %68
%67 = OpLabel
%69 = OpLoad %v4float %f4
%70 = OpVectorShuffle %v2float %69 %69 0 1
%71 = OpLoad %v4float %f4
%72 = OpVectorShuffle %v2float %71 %71 2 3
%73 = OpLoad %v4float %f4
%74 = OpLoad %v4float %f4
%75 = OpCompositeExtract %float %74 0
%76 = OpCompositeExtract %float %70 0
%77 = OpCompositeExtract %float %70 1
%78 = OpCompositeExtract %float %72 0
%79 = OpCompositeConstruct %v3float %76 %77 %78
%80 = OpCompositeExtract %float %72 1
%81 = OpCompositeExtract %float %73 0
%82 = OpCompositeExtract %float %73 1
%83 = OpCompositeConstruct %v3float %80 %81 %82
%84 = OpCompositeExtract %float %73 2
%85 = OpCompositeExtract %float %73 3
%86 = OpCompositeConstruct %v3float %84 %85 %75
%88 = OpCompositeConstruct %mat3v3float %79 %83 %86
%94 = OpCompositeExtract %v3float %88 0
%95 = OpFOrdEqual %v3bool %94 %89
%96 = OpAll %bool %95
%97 = OpCompositeExtract %v3float %88 1
%98 = OpFOrdEqual %v3bool %97 %90
%99 = OpAll %bool %98
%100 = OpLogicalAnd %bool %96 %99
%101 = OpCompositeExtract %v3float %88 2
%102 = OpFOrdEqual %v3bool %101 %91
%103 = OpAll %bool %102
%104 = OpLogicalAnd %bool %100 %103
OpBranch %68
%68 = OpLabel
%105 = OpPhi %bool %false %26 %104 %67
OpStore %ok %105
%106 = OpLoad %bool %ok
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%109 = OpLoad %v4float %f4
%110 = OpVectorShuffle %v3float %109 %109 0 1 2
%111 = OpLoad %v4float %f4
%112 = OpVectorShuffle %v3float %111 %111 3 0 1
%113 = OpLoad %v4float %f4
%114 = OpVectorShuffle %v4float %113 %113 2 3 0 1
%115 = OpLoad %v4float %f4
%116 = OpVectorShuffle %v2float %115 %115 2 3
%117 = OpLoad %v4float %f4
%118 = OpCompositeExtract %float %110 0
%119 = OpCompositeExtract %float %110 1
%120 = OpCompositeExtract %float %110 2
%121 = OpCompositeExtract %float %112 0
%122 = OpCompositeConstruct %v4float %118 %119 %120 %121
%123 = OpCompositeExtract %float %112 1
%124 = OpCompositeExtract %float %112 2
%125 = OpCompositeExtract %float %114 0
%126 = OpCompositeExtract %float %114 1
%127 = OpCompositeConstruct %v4float %123 %124 %125 %126
%128 = OpCompositeExtract %float %114 2
%129 = OpCompositeExtract %float %114 3
%130 = OpCompositeExtract %float %116 0
%131 = OpCompositeExtract %float %116 1
%132 = OpCompositeConstruct %v4float %128 %129 %130 %131
%134 = OpCompositeConstruct %mat4v4float %122 %127 %132 %117
%138 = OpCompositeExtract %v4float %134 0
%139 = OpFOrdEqual %v4bool %138 %135
%140 = OpAll %bool %139
%141 = OpCompositeExtract %v4float %134 1
%142 = OpFOrdEqual %v4bool %141 %135
%143 = OpAll %bool %142
%144 = OpLogicalAnd %bool %140 %143
%145 = OpCompositeExtract %v4float %134 2
%146 = OpFOrdEqual %v4bool %145 %135
%147 = OpAll %bool %146
%148 = OpLogicalAnd %bool %144 %147
%149 = OpCompositeExtract %v4float %134 3
%150 = OpFOrdEqual %v4bool %149 %135
%151 = OpAll %bool %150
%152 = OpLogicalAnd %bool %148 %151
OpBranch %108
%108 = OpLabel
%153 = OpPhi %bool %false %68 %152 %107
OpStore %ok %153
%154 = OpLoad %bool %ok
OpSelectionMerge %158 None
OpBranchConditional %154 %156 %157
%156 = OpLabel
%159 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%162 = OpLoad %v4float %159
OpStore %155 %162
OpBranch %158
%157 = OpLabel
%163 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%165 = OpLoad %v4float %163
OpStore %155 %165
OpBranch %158
%158 = OpLabel
%166 = OpLoad %v4float %155
OpReturnValue %166
OpFunctionEnd
