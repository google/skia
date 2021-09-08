OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorRed"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "testInputs"
OpName %_entrypoint_v "_entrypoint_v"
OpName %fn_hh4 "fn_hh4"
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
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %v RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%24 = OpTypeFunction %float %_ptr_Function_v4float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%44 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%v3float = OpTypeVector %float 3
%float_1 = OpConstant %float 1
%float_123 = OpConstant %float 123
%float_456 = OpConstant %float 456
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%113 = OpConstantComposite %v4float %float_1 %float_1 %float_2 %float_3
%int_0 = OpConstant %int 0
%146 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%v4bool = OpTypeVector %bool 4
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%fn_hh4 = OpFunction %float None %24
%26 = OpFunctionParameter %_ptr_Function_v4float
%27 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_1
OpBranch %32
%32 = OpLabel
OpLoopMerge %36 %35 None
OpBranch %33
%33 = OpLabel
%37 = OpLoad %int %x
%39 = OpSLessThanEqual %bool %37 %int_2
OpBranchConditional %39 %34 %36
%34 = OpLabel
%40 = OpLoad %v4float %26
%41 = OpCompositeExtract %float %40 0
OpReturnValue %41
%35 = OpLabel
%42 = OpLoad %int %x
%43 = OpIAdd %int %42 %int_1
OpStore %x %43
OpBranch %32
%36 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %v4float None %44
%45 = OpFunctionParameter %_ptr_Function_v2float
%46 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%82 = OpVariable %_ptr_Function_v4float Function
%89 = OpVariable %_ptr_Function_v4float Function
%94 = OpVariable %_ptr_Function_v4float Function
%98 = OpVariable %_ptr_Function_v4float Function
%102 = OpVariable %_ptr_Function_v4float Function
%107 = OpVariable %_ptr_Function_v4float Function
%150 = OpVariable %_ptr_Function_v4float Function
%48 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
%50 = OpLoad %v4float %48
OpStore %v %50
%51 = OpLoad %v4float %v
%52 = OpVectorShuffle %v3float %51 %51 2 1 0
%54 = OpCompositeExtract %float %52 0
%55 = OpCompositeExtract %float %52 1
%56 = OpCompositeExtract %float %52 2
%57 = OpCompositeConstruct %v4float %float_0 %54 %55 %56
OpStore %v %57
%58 = OpLoad %v4float %v
%59 = OpVectorShuffle %v2float %58 %58 0 3
%60 = OpCompositeExtract %float %59 0
%61 = OpCompositeExtract %float %59 1
%62 = OpCompositeConstruct %v4float %float_0 %float_0 %60 %61
OpStore %v %62
%64 = OpLoad %v4float %v
%65 = OpVectorShuffle %v2float %64 %64 3 0
%66 = OpCompositeExtract %float %65 0
%67 = OpCompositeExtract %float %65 1
%68 = OpCompositeConstruct %v4float %float_1 %float_1 %66 %67
OpStore %v %68
%69 = OpLoad %v4float %v
%70 = OpVectorShuffle %v2float %69 %69 2 1
%71 = OpCompositeExtract %float %70 0
%72 = OpCompositeExtract %float %70 1
%73 = OpCompositeConstruct %v4float %71 %72 %float_1 %float_1
OpStore %v %73
%74 = OpLoad %v4float %v
%75 = OpVectorShuffle %v2float %74 %74 0 0
%76 = OpCompositeExtract %float %75 0
%77 = OpCompositeExtract %float %75 1
%78 = OpCompositeConstruct %v4float %76 %77 %float_1 %float_1
OpStore %v %78
%79 = OpLoad %v4float %v
%80 = OpVectorShuffle %v4float %79 %79 3 2 3 2
OpStore %v %80
%81 = OpLoad %v4float %v
OpStore %82 %81
%83 = OpFunctionCall %float %fn_hh4 %82
%86 = OpCompositeConstruct %v3float %83 %float_123 %float_456
%87 = OpVectorShuffle %v4float %86 %86 1 1 2 2
OpStore %v %87
%88 = OpLoad %v4float %v
OpStore %89 %88
%90 = OpFunctionCall %float %fn_hh4 %89
%91 = OpCompositeConstruct %v3float %90 %float_123 %float_456
%92 = OpVectorShuffle %v4float %91 %91 1 1 2 2
OpStore %v %92
%93 = OpLoad %v4float %v
OpStore %94 %93
%95 = OpFunctionCall %float %fn_hh4 %94
%96 = OpCompositeConstruct %v4float %float_123 %float_456 %float_456 %95
OpStore %v %96
%97 = OpLoad %v4float %v
OpStore %98 %97
%99 = OpFunctionCall %float %fn_hh4 %98
%100 = OpCompositeConstruct %v4float %float_123 %float_456 %float_456 %99
OpStore %v %100
%101 = OpLoad %v4float %v
OpStore %102 %101
%103 = OpFunctionCall %float %fn_hh4 %102
%104 = OpCompositeConstruct %v3float %103 %float_123 %float_456
%105 = OpVectorShuffle %v4float %104 %104 1 0 0 2
OpStore %v %105
%106 = OpLoad %v4float %v
OpStore %107 %106
%108 = OpFunctionCall %float %fn_hh4 %107
%109 = OpCompositeConstruct %v3float %108 %float_123 %float_456
%110 = OpVectorShuffle %v4float %109 %109 1 0 0 2
OpStore %v %110
OpStore %v %113
%114 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%116 = OpLoad %v4float %114
%117 = OpVectorShuffle %v3float %116 %116 0 1 2
%118 = OpCompositeExtract %float %117 0
%119 = OpCompositeExtract %float %117 1
%120 = OpCompositeExtract %float %117 2
%121 = OpCompositeConstruct %v4float %118 %119 %120 %float_1
OpStore %v %121
%122 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%123 = OpLoad %v4float %122
%124 = OpCompositeExtract %float %123 0
%125 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%126 = OpLoad %v4float %125
%127 = OpVectorShuffle %v2float %126 %126 1 2
%128 = OpCompositeExtract %float %127 0
%129 = OpCompositeExtract %float %127 1
%130 = OpCompositeConstruct %v4float %124 %float_1 %128 %129
OpStore %v %130
%131 = OpLoad %v4float %v
%132 = OpLoad %v4float %v
%133 = OpVectorShuffle %v4float %132 %131 7 6 5 4
OpStore %v %133
%134 = OpLoad %v4float %v
%135 = OpVectorShuffle %v2float %134 %134 1 2
%136 = OpLoad %v4float %v
%137 = OpVectorShuffle %v4float %136 %135 4 1 2 5
OpStore %v %137
%138 = OpLoad %v4float %v
%139 = OpVectorShuffle %v2float %138 %138 3 3
%140 = OpCompositeExtract %float %139 0
%141 = OpCompositeExtract %float %139 1
%142 = OpCompositeConstruct %v3float %140 %141 %float_1
%143 = OpLoad %v4float %v
%144 = OpVectorShuffle %v4float %143 %142 6 5 4 3
OpStore %v %144
%145 = OpLoad %v4float %v
%147 = OpFOrdEqual %v4bool %145 %146
%149 = OpAll %bool %147
OpSelectionMerge %153 None
OpBranchConditional %149 %151 %152
%151 = OpLabel
%154 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%155 = OpLoad %v4float %154
OpStore %150 %155
OpBranch %153
%152 = OpLabel
%156 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%157 = OpLoad %v4float %156
OpStore %150 %157
OpBranch %153
%153 = OpLabel
%158 = OpLoad %v4float %150
OpReturnValue %158
OpFunctionEnd
