OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorRed"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "testInputs"
OpName %_entrypoint "_entrypoint"
OpName %fn "fn"
OpName %x "x"
OpName %main "main"
OpName %v "v"
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
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %35 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%19 = OpTypeFunction %float %_ptr_Function_v4float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%39 = OpTypeFunction %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%float_0 = OpConstant %float 0
%v3float = OpTypeVector %float 3
%v2float = OpTypeVector %float 2
%float_1 = OpConstant %float 1
%float_123 = OpConstant %float 123
%float_456 = OpConstant %float 456
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%109 = OpConstantComposite %v4float %float_1 %float_1 %float_2 %float_3
%int_0 = OpConstant %int 0
%142 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%v4bool = OpTypeVector %bool 4
%_entrypoint = OpFunction %void None %16
%17 = OpLabel
%18 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %18
OpReturn
OpFunctionEnd
%fn = OpFunction %float None %19
%21 = OpFunctionParameter %_ptr_Function_v4float
%22 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_1
OpBranch %27
%27 = OpLabel
OpLoopMerge %31 %30 None
OpBranch %28
%28 = OpLabel
%32 = OpLoad %int %x
%34 = OpSLessThanEqual %bool %32 %int_2
OpBranchConditional %34 %29 %31
%29 = OpLabel
%35 = OpLoad %v4float %21
%36 = OpCompositeExtract %float %35 0
OpReturnValue %36
%30 = OpLabel
%37 = OpLoad %int %x
%38 = OpIAdd %int %37 %int_1
OpStore %x %38
OpBranch %27
%31 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %v4float None %39
%40 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%78 = OpVariable %_ptr_Function_v4float Function
%85 = OpVariable %_ptr_Function_v4float Function
%90 = OpVariable %_ptr_Function_v4float Function
%94 = OpVariable %_ptr_Function_v4float Function
%98 = OpVariable %_ptr_Function_v4float Function
%103 = OpVariable %_ptr_Function_v4float Function
%146 = OpVariable %_ptr_Function_v4float Function
%42 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
%44 = OpLoad %v4float %42
OpStore %v %44
%46 = OpLoad %v4float %v
%47 = OpVectorShuffle %v3float %46 %46 2 1 0
%49 = OpCompositeExtract %float %47 0
%50 = OpCompositeExtract %float %47 1
%51 = OpCompositeExtract %float %47 2
%52 = OpCompositeConstruct %v4float %float_0 %49 %50 %51
OpStore %v %52
%53 = OpLoad %v4float %v
%54 = OpVectorShuffle %v2float %53 %53 0 3
%56 = OpCompositeExtract %float %54 0
%57 = OpCompositeExtract %float %54 1
%58 = OpCompositeConstruct %v4float %float_0 %float_0 %56 %57
OpStore %v %58
%60 = OpLoad %v4float %v
%61 = OpVectorShuffle %v2float %60 %60 3 0
%62 = OpCompositeExtract %float %61 0
%63 = OpCompositeExtract %float %61 1
%64 = OpCompositeConstruct %v4float %float_1 %float_1 %62 %63
OpStore %v %64
%65 = OpLoad %v4float %v
%66 = OpVectorShuffle %v2float %65 %65 2 1
%67 = OpCompositeExtract %float %66 0
%68 = OpCompositeExtract %float %66 1
%69 = OpCompositeConstruct %v4float %67 %68 %float_1 %float_1
OpStore %v %69
%70 = OpLoad %v4float %v
%71 = OpVectorShuffle %v2float %70 %70 0 0
%72 = OpCompositeExtract %float %71 0
%73 = OpCompositeExtract %float %71 1
%74 = OpCompositeConstruct %v4float %72 %73 %float_1 %float_1
OpStore %v %74
%75 = OpLoad %v4float %v
%76 = OpVectorShuffle %v4float %75 %75 3 2 3 2
OpStore %v %76
%77 = OpLoad %v4float %v
OpStore %78 %77
%79 = OpFunctionCall %float %fn %78
%82 = OpCompositeConstruct %v3float %79 %float_123 %float_456
%83 = OpVectorShuffle %v4float %82 %82 1 1 2 2
OpStore %v %83
%84 = OpLoad %v4float %v
OpStore %85 %84
%86 = OpFunctionCall %float %fn %85
%87 = OpCompositeConstruct %v3float %86 %float_123 %float_456
%88 = OpVectorShuffle %v4float %87 %87 1 1 2 2
OpStore %v %88
%89 = OpLoad %v4float %v
OpStore %90 %89
%91 = OpFunctionCall %float %fn %90
%92 = OpCompositeConstruct %v4float %float_123 %float_456 %float_456 %91
OpStore %v %92
%93 = OpLoad %v4float %v
OpStore %94 %93
%95 = OpFunctionCall %float %fn %94
%96 = OpCompositeConstruct %v4float %float_123 %float_456 %float_456 %95
OpStore %v %96
%97 = OpLoad %v4float %v
OpStore %98 %97
%99 = OpFunctionCall %float %fn %98
%100 = OpCompositeConstruct %v3float %99 %float_123 %float_456
%101 = OpVectorShuffle %v4float %100 %100 1 0 0 2
OpStore %v %101
%102 = OpLoad %v4float %v
OpStore %103 %102
%104 = OpFunctionCall %float %fn %103
%105 = OpCompositeConstruct %v3float %104 %float_123 %float_456
%106 = OpVectorShuffle %v4float %105 %105 1 0 0 2
OpStore %v %106
OpStore %v %109
%110 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%112 = OpLoad %v4float %110
%113 = OpVectorShuffle %v3float %112 %112 0 1 2
%114 = OpCompositeExtract %float %113 0
%115 = OpCompositeExtract %float %113 1
%116 = OpCompositeExtract %float %113 2
%117 = OpCompositeConstruct %v4float %114 %115 %116 %float_1
OpStore %v %117
%118 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%119 = OpLoad %v4float %118
%120 = OpCompositeExtract %float %119 0
%121 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%122 = OpLoad %v4float %121
%123 = OpVectorShuffle %v2float %122 %122 1 2
%124 = OpCompositeExtract %float %123 0
%125 = OpCompositeExtract %float %123 1
%126 = OpCompositeConstruct %v4float %120 %float_1 %124 %125
OpStore %v %126
%127 = OpLoad %v4float %v
%128 = OpLoad %v4float %v
%129 = OpVectorShuffle %v4float %128 %127 7 6 5 4
OpStore %v %129
%130 = OpLoad %v4float %v
%131 = OpVectorShuffle %v2float %130 %130 1 2
%132 = OpLoad %v4float %v
%133 = OpVectorShuffle %v4float %132 %131 4 1 2 5
OpStore %v %133
%134 = OpLoad %v4float %v
%135 = OpVectorShuffle %v2float %134 %134 3 3
%136 = OpCompositeExtract %float %135 0
%137 = OpCompositeExtract %float %135 1
%138 = OpCompositeConstruct %v3float %136 %137 %float_1
%139 = OpLoad %v4float %v
%140 = OpVectorShuffle %v4float %139 %138 6 5 4 3
OpStore %v %140
%141 = OpLoad %v4float %v
%143 = OpFOrdEqual %v4bool %141 %142
%145 = OpAll %bool %143
OpSelectionMerge %149 None
OpBranchConditional %145 %147 %148
%147 = OpLabel
%150 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%151 = OpLoad %v4float %150
OpStore %146 %151
OpBranch %149
%148 = OpLabel
%152 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%153 = OpLoad %v4float %152
OpStore %146 %153
OpBranch %149
%149 = OpLabel
%154 = OpLoad %v4float %146
OpReturnValue %154
OpFunctionEnd
