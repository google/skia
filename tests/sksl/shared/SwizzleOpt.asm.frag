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
OpDecorate %46 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
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
%114 = OpConstantComposite %v4float %float_1 %float_1 %float_2 %float_3
%int_0 = OpConstant %int 0
%150 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
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
%83 = OpVariable %_ptr_Function_v4float Function
%90 = OpVariable %_ptr_Function_v4float Function
%95 = OpVariable %_ptr_Function_v4float Function
%99 = OpVariable %_ptr_Function_v4float Function
%103 = OpVariable %_ptr_Function_v4float Function
%108 = OpVariable %_ptr_Function_v4float Function
%154 = OpVariable %_ptr_Function_v4float Function
%42 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
%44 = OpLoad %v4float %42
OpStore %v %44
%46 = OpLoad %v4float %v
%47 = OpVectorShuffle %v3float %46 %46 0 1 2
%49 = OpVectorShuffle %v3float %47 %47 2 1 0
%50 = OpCompositeExtract %float %49 0
%51 = OpCompositeExtract %float %49 1
%52 = OpCompositeExtract %float %49 2
%53 = OpCompositeConstruct %v4float %float_0 %50 %51 %52
OpStore %v %53
%54 = OpLoad %v4float %v
%55 = OpVectorShuffle %v2float %54 %54 0 3
%57 = OpVectorShuffle %v2float %55 %55 0 1
%58 = OpCompositeExtract %float %57 0
%59 = OpCompositeExtract %float %57 1
%60 = OpCompositeConstruct %v4float %float_0 %float_0 %58 %59
OpStore %v %60
%62 = OpLoad %v4float %v
%63 = OpVectorShuffle %v2float %62 %62 0 3
%64 = OpVectorShuffle %v2float %63 %63 1 0
%65 = OpVectorShuffle %v2float %64 %64 0 1
%66 = OpCompositeExtract %float %65 0
%67 = OpCompositeExtract %float %65 1
%68 = OpCompositeConstruct %v4float %float_1 %float_1 %66 %67
OpStore %v %68
%69 = OpLoad %v4float %v
%70 = OpVectorShuffle %v2float %69 %69 2 1
%71 = OpVectorShuffle %v2float %70 %70 0 1
%72 = OpCompositeExtract %float %71 0
%73 = OpCompositeExtract %float %71 1
%74 = OpCompositeConstruct %v4float %72 %73 %float_1 %float_1
OpStore %v %74
%75 = OpLoad %v4float %v
%76 = OpVectorShuffle %v2float %75 %75 0 0
%77 = OpCompositeExtract %float %76 0
%78 = OpCompositeExtract %float %76 1
%79 = OpCompositeConstruct %v4float %77 %78 %float_1 %float_1
OpStore %v %79
%80 = OpLoad %v4float %v
%81 = OpVectorShuffle %v4float %80 %80 3 2 3 2
OpStore %v %81
%82 = OpLoad %v4float %v
OpStore %83 %82
%84 = OpFunctionCall %float %fn %83
%87 = OpCompositeConstruct %v3float %84 %float_123 %float_456
%88 = OpVectorShuffle %v4float %87 %87 1 1 2 2
OpStore %v %88
%89 = OpLoad %v4float %v
OpStore %90 %89
%91 = OpFunctionCall %float %fn %90
%92 = OpCompositeConstruct %v3float %91 %float_123 %float_456
%93 = OpVectorShuffle %v4float %92 %92 1 1 2 2
OpStore %v %93
%94 = OpLoad %v4float %v
OpStore %95 %94
%96 = OpFunctionCall %float %fn %95
%97 = OpCompositeConstruct %v4float %float_123 %float_456 %float_456 %96
OpStore %v %97
%98 = OpLoad %v4float %v
OpStore %99 %98
%100 = OpFunctionCall %float %fn %99
%101 = OpCompositeConstruct %v4float %float_123 %float_456 %float_456 %100
OpStore %v %101
%102 = OpLoad %v4float %v
OpStore %103 %102
%104 = OpFunctionCall %float %fn %103
%105 = OpCompositeConstruct %v3float %104 %float_123 %float_456
%106 = OpVectorShuffle %v4float %105 %105 1 0 0 2
OpStore %v %106
%107 = OpLoad %v4float %v
OpStore %108 %107
%109 = OpFunctionCall %float %fn %108
%110 = OpCompositeConstruct %v3float %109 %float_123 %float_456
%111 = OpVectorShuffle %v4float %110 %110 1 0 0 2
OpStore %v %111
OpStore %v %114
%115 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%117 = OpLoad %v4float %115
%118 = OpVectorShuffle %v3float %117 %117 0 1 2
%119 = OpVectorShuffle %v3float %118 %118 0 1 2
%120 = OpCompositeExtract %float %119 0
%121 = OpCompositeExtract %float %119 1
%122 = OpCompositeExtract %float %119 2
%123 = OpCompositeConstruct %v4float %120 %121 %122 %float_1
OpStore %v %123
%124 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%125 = OpLoad %v4float %124
%126 = OpVectorShuffle %v3float %125 %125 0 1 2
%127 = OpCompositeExtract %float %126 0
%128 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%129 = OpLoad %v4float %128
%130 = OpVectorShuffle %v3float %129 %129 0 1 2
%131 = OpVectorShuffle %v2float %130 %130 1 2
%132 = OpCompositeExtract %float %131 0
%133 = OpCompositeExtract %float %131 1
%134 = OpCompositeConstruct %v4float %127 %float_1 %132 %133
OpStore %v %134
%135 = OpLoad %v4float %v
%136 = OpLoad %v4float %v
%137 = OpVectorShuffle %v4float %136 %135 7 6 5 4
OpStore %v %137
%138 = OpLoad %v4float %v
%139 = OpVectorShuffle %v2float %138 %138 1 2
%140 = OpLoad %v4float %v
%141 = OpVectorShuffle %v4float %140 %139 4 1 2 5
OpStore %v %141
%142 = OpLoad %v4float %v
%143 = OpVectorShuffle %v2float %142 %142 3 3
%144 = OpCompositeExtract %float %143 0
%145 = OpCompositeExtract %float %143 1
%146 = OpCompositeConstruct %v3float %144 %145 %float_1
%147 = OpLoad %v4float %v
%148 = OpVectorShuffle %v4float %147 %146 6 5 4 3
OpStore %v %148
%149 = OpLoad %v4float %v
%151 = OpFOrdEqual %v4bool %149 %150
%153 = OpAll %bool %151
OpSelectionMerge %157 None
OpBranchConditional %153 %155 %156
%155 = OpLabel
%158 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%159 = OpLoad %v4float %158
OpStore %154 %159
OpBranch %157
%156 = OpLabel
%160 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%161 = OpLoad %v4float %160
OpStore %154 %161
OpBranch %157
%157 = OpLabel
%162 = OpLoad %v4float %154
OpReturnValue %162
OpFunctionEnd
