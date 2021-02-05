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
OpDecorate %sk_Clockwise RelaxedPrecision
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
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
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
%111 = OpConstantComposite %v4float %float_1 %float_1 %float_2 %float_3
%int_0 = OpConstant %int 0
%147 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
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
%80 = OpVariable %_ptr_Function_v4float Function
%87 = OpVariable %_ptr_Function_v4float Function
%92 = OpVariable %_ptr_Function_v4float Function
%96 = OpVariable %_ptr_Function_v4float Function
%100 = OpVariable %_ptr_Function_v4float Function
%105 = OpVariable %_ptr_Function_v4float Function
%151 = OpVariable %_ptr_Function_v4float Function
%42 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
%44 = OpLoad %v4float %42
OpStore %v %44
%45 = OpLoad %v4float %v
OpStore %v %45
%47 = OpLoad %v4float %v
%48 = OpVectorShuffle %v3float %47 %47 2 1 0
%50 = OpCompositeExtract %float %48 0
%51 = OpCompositeExtract %float %48 1
%52 = OpCompositeExtract %float %48 2
%53 = OpCompositeConstruct %v4float %float_0 %50 %51 %52
OpStore %v %53
%54 = OpLoad %v4float %v
%55 = OpVectorShuffle %v2float %54 %54 0 3
%57 = OpCompositeExtract %float %55 0
%58 = OpCompositeExtract %float %55 1
%59 = OpCompositeConstruct %v4float %float_0 %float_0 %57 %58
OpStore %v %59
%61 = OpLoad %v4float %v
%62 = OpVectorShuffle %v2float %61 %61 3 0
%63 = OpCompositeExtract %float %62 0
%64 = OpCompositeExtract %float %62 1
%65 = OpCompositeConstruct %v4float %float_1 %float_1 %63 %64
OpStore %v %65
%66 = OpLoad %v4float %v
%67 = OpVectorShuffle %v2float %66 %66 2 1
%68 = OpCompositeExtract %float %67 0
%69 = OpCompositeExtract %float %67 1
%70 = OpCompositeConstruct %v4float %68 %69 %float_1 %float_1
OpStore %v %70
%71 = OpLoad %v4float %v
OpStore %v %71
%72 = OpLoad %v4float %v
%73 = OpVectorShuffle %v2float %72 %72 0 0
%74 = OpCompositeExtract %float %73 0
%75 = OpCompositeExtract %float %73 1
%76 = OpCompositeConstruct %v4float %74 %75 %float_1 %float_1
OpStore %v %76
%77 = OpLoad %v4float %v
%78 = OpVectorShuffle %v4float %77 %77 3 2 3 2
OpStore %v %78
%79 = OpLoad %v4float %v
OpStore %80 %79
%81 = OpFunctionCall %float %fn %80
%84 = OpCompositeConstruct %v3float %81 %float_123 %float_456
%85 = OpVectorShuffle %v4float %84 %84 1 1 2 2
OpStore %v %85
%86 = OpLoad %v4float %v
OpStore %87 %86
%88 = OpFunctionCall %float %fn %87
%89 = OpCompositeConstruct %v3float %88 %float_123 %float_456
%90 = OpVectorShuffle %v4float %89 %89 1 1 2 2
OpStore %v %90
%91 = OpLoad %v4float %v
OpStore %92 %91
%93 = OpFunctionCall %float %fn %92
%94 = OpCompositeConstruct %v4float %float_123 %float_456 %float_456 %93
OpStore %v %94
%95 = OpLoad %v4float %v
OpStore %96 %95
%97 = OpFunctionCall %float %fn %96
%98 = OpCompositeConstruct %v4float %float_123 %float_456 %float_456 %97
OpStore %v %98
%99 = OpLoad %v4float %v
OpStore %100 %99
%101 = OpFunctionCall %float %fn %100
%102 = OpCompositeConstruct %v3float %101 %float_123 %float_456
%103 = OpVectorShuffle %v4float %102 %102 1 0 0 2
OpStore %v %103
%104 = OpLoad %v4float %v
OpStore %105 %104
%106 = OpFunctionCall %float %fn %105
%107 = OpCompositeConstruct %v3float %106 %float_123 %float_456
%108 = OpVectorShuffle %v4float %107 %107 1 0 0 2
OpStore %v %108
OpStore %v %111
%112 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%114 = OpLoad %v4float %112
%115 = OpVectorShuffle %v3float %114 %114 0 1 2
%116 = OpCompositeExtract %float %115 0
%117 = OpCompositeExtract %float %115 1
%118 = OpCompositeExtract %float %115 2
%119 = OpCompositeConstruct %v4float %116 %117 %118 %float_1
OpStore %v %119
%120 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%121 = OpLoad %v4float %120
%122 = OpCompositeExtract %float %121 0
%123 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%124 = OpLoad %v4float %123
%125 = OpVectorShuffle %v2float %124 %124 1 2
%126 = OpCompositeExtract %float %125 0
%127 = OpCompositeExtract %float %125 1
%128 = OpCompositeConstruct %v4float %122 %float_1 %126 %127
OpStore %v %128
%129 = OpLoad %v4float %v
%130 = OpLoad %v4float %v
%131 = OpVectorShuffle %v4float %130 %129 4 5 6 7
OpStore %v %131
%132 = OpLoad %v4float %v
%133 = OpLoad %v4float %v
%134 = OpVectorShuffle %v4float %133 %132 7 6 5 4
OpStore %v %134
%135 = OpLoad %v4float %v
%136 = OpVectorShuffle %v2float %135 %135 1 2
%137 = OpLoad %v4float %v
%138 = OpVectorShuffle %v4float %137 %136 4 1 2 5
OpStore %v %138
%139 = OpLoad %v4float %v
%140 = OpVectorShuffle %v2float %139 %139 3 3
%141 = OpCompositeExtract %float %140 0
%142 = OpCompositeExtract %float %140 1
%143 = OpCompositeConstruct %v3float %141 %142 %float_1
%144 = OpLoad %v4float %v
%145 = OpVectorShuffle %v4float %144 %143 6 5 4 3
OpStore %v %145
%146 = OpLoad %v4float %v
%148 = OpFOrdEqual %v4bool %146 %147
%150 = OpAll %bool %148
OpSelectionMerge %154 None
OpBranchConditional %150 %152 %153
%152 = OpLabel
%155 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%156 = OpLoad %v4float %155
OpStore %151 %156
OpBranch %154
%153 = OpLabel
%157 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%158 = OpLoad %v4float %157
OpStore %151 %158
OpBranch %154
%154 = OpLabel
%159 = OpLoad %v4float %151
OpReturnValue %159
OpFunctionEnd
