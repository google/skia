OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %_1_m3 "_1_m3"
OpName %_2_m5 "_2_m5"
OpName %_3_m6 "_3_m6"
OpName %_4_m11 "_4_m11"
OpName %_6_m3 "_6_m3"
OpName %_7_m5 "_7_m5"
OpName %_8_m6 "_8_m6"
OpName %_9_m11 "_9_m11"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
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
%18 = OpTypeFunction %v4float
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%_1_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m5 = OpVariable %_ptr_Function_mat2v2float Function
%38 = OpVariable %_ptr_Function_mat2v2float Function
%_3_m6 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m11 = OpVariable %_ptr_Function_mat4v4float Function
%_6_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_7_m5 = OpVariable %_ptr_Function_mat2v2float Function
%101 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m6 = OpVariable %_ptr_Function_mat2v2float Function
%_9_m11 = OpVariable %_ptr_Function_mat4v4float Function
%29 = OpCompositeConstruct %v2float %float_1 %float_2
%30 = OpCompositeConstruct %v2float %float_3 %float_4
%28 = OpCompositeConstruct %mat2v2float %29 %30
OpStore %_1_m3 %28
%31 = OpLoad %mat2v2float %_1_m3
%34 = OpCompositeConstruct %v2float %float_1 %float_0
%35 = OpCompositeConstruct %v2float %float_0 %float_1
%32 = OpCompositeConstruct %mat2v2float %34 %35
%36 = OpMatrixTimesMatrix %mat2v2float %31 %32
OpStore %_1_m3 %36
%40 = OpCompositeConstruct %v2float %float_1 %float_2
%41 = OpCompositeConstruct %v2float %float_3 %float_4
%39 = OpCompositeConstruct %mat2v2float %40 %41
OpStore %38 %39
%44 = OpAccessChain %_ptr_Function_v2float %38 %int_0
%46 = OpLoad %v2float %44
%47 = OpCompositeExtract %float %46 0
%49 = OpCompositeConstruct %v2float %47 %float_0
%50 = OpCompositeConstruct %v2float %float_0 %47
%48 = OpCompositeConstruct %mat2v2float %49 %50
OpStore %_2_m5 %48
%53 = OpCompositeConstruct %v2float %float_1 %float_2
%54 = OpCompositeConstruct %v2float %float_3 %float_4
%52 = OpCompositeConstruct %mat2v2float %53 %54
OpStore %_3_m6 %52
%55 = OpLoad %mat2v2float %_3_m6
%56 = OpLoad %mat2v2float %_2_m5
%57 = OpCompositeExtract %v2float %55 0
%58 = OpCompositeExtract %v2float %56 0
%59 = OpFAdd %v2float %57 %58
%60 = OpCompositeExtract %v2float %55 1
%61 = OpCompositeExtract %v2float %56 1
%62 = OpFAdd %v2float %60 %61
%63 = OpCompositeConstruct %mat2v2float %59 %62
OpStore %_3_m6 %63
%68 = OpCompositeConstruct %v4float %float_2 %float_0 %float_0 %float_0
%69 = OpCompositeConstruct %v4float %float_0 %float_2 %float_0 %float_0
%70 = OpCompositeConstruct %v4float %float_0 %float_0 %float_2 %float_0
%71 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_2
%67 = OpCompositeConstruct %mat4v4float %68 %69 %70 %71
OpStore %_4_m11 %67
%72 = OpLoad %mat4v4float %_4_m11
%74 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%75 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%76 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%77 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%73 = OpCompositeConstruct %mat4v4float %74 %75 %76 %77
%78 = OpCompositeExtract %v4float %72 0
%79 = OpCompositeExtract %v4float %73 0
%80 = OpFSub %v4float %78 %79
%81 = OpCompositeExtract %v4float %72 1
%82 = OpCompositeExtract %v4float %73 1
%83 = OpFSub %v4float %81 %82
%84 = OpCompositeExtract %v4float %72 2
%85 = OpCompositeExtract %v4float %73 2
%86 = OpFSub %v4float %84 %85
%87 = OpCompositeExtract %v4float %72 3
%88 = OpCompositeExtract %v4float %73 3
%89 = OpFSub %v4float %87 %88
%90 = OpCompositeConstruct %mat4v4float %80 %83 %86 %89
OpStore %_4_m11 %90
%93 = OpCompositeConstruct %v2float %float_1 %float_2
%94 = OpCompositeConstruct %v2float %float_3 %float_4
%92 = OpCompositeConstruct %mat2v2float %93 %94
OpStore %_6_m3 %92
%95 = OpLoad %mat2v2float %_6_m3
%97 = OpCompositeConstruct %v2float %float_1 %float_0
%98 = OpCompositeConstruct %v2float %float_0 %float_1
%96 = OpCompositeConstruct %mat2v2float %97 %98
%99 = OpMatrixTimesMatrix %mat2v2float %95 %96
OpStore %_6_m3 %99
%103 = OpCompositeConstruct %v2float %float_1 %float_2
%104 = OpCompositeConstruct %v2float %float_3 %float_4
%102 = OpCompositeConstruct %mat2v2float %103 %104
OpStore %101 %102
%105 = OpAccessChain %_ptr_Function_v2float %101 %int_0
%106 = OpLoad %v2float %105
%107 = OpCompositeExtract %float %106 0
%109 = OpCompositeConstruct %v2float %107 %float_0
%110 = OpCompositeConstruct %v2float %float_0 %107
%108 = OpCompositeConstruct %mat2v2float %109 %110
OpStore %_7_m5 %108
%113 = OpCompositeConstruct %v2float %float_1 %float_2
%114 = OpCompositeConstruct %v2float %float_3 %float_4
%112 = OpCompositeConstruct %mat2v2float %113 %114
OpStore %_8_m6 %112
%115 = OpLoad %mat2v2float %_8_m6
%116 = OpLoad %mat2v2float %_7_m5
%117 = OpCompositeExtract %v2float %115 0
%118 = OpCompositeExtract %v2float %116 0
%119 = OpFAdd %v2float %117 %118
%120 = OpCompositeExtract %v2float %115 1
%121 = OpCompositeExtract %v2float %116 1
%122 = OpFAdd %v2float %120 %121
%123 = OpCompositeConstruct %mat2v2float %119 %122
OpStore %_8_m6 %123
%126 = OpCompositeConstruct %v4float %float_2 %float_0 %float_0 %float_0
%127 = OpCompositeConstruct %v4float %float_0 %float_2 %float_0 %float_0
%128 = OpCompositeConstruct %v4float %float_0 %float_0 %float_2 %float_0
%129 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_2
%125 = OpCompositeConstruct %mat4v4float %126 %127 %128 %129
OpStore %_9_m11 %125
%130 = OpLoad %mat4v4float %_9_m11
%132 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%133 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%134 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%135 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%131 = OpCompositeConstruct %mat4v4float %132 %133 %134 %135
%136 = OpCompositeExtract %v4float %130 0
%137 = OpCompositeExtract %v4float %131 0
%138 = OpFSub %v4float %136 %137
%139 = OpCompositeExtract %v4float %130 1
%140 = OpCompositeExtract %v4float %131 1
%141 = OpFSub %v4float %139 %140
%142 = OpCompositeExtract %v4float %130 2
%143 = OpCompositeExtract %v4float %131 2
%144 = OpFSub %v4float %142 %143
%145 = OpCompositeExtract %v4float %130 3
%146 = OpCompositeExtract %v4float %131 3
%147 = OpFSub %v4float %145 %146
%148 = OpCompositeConstruct %mat4v4float %138 %141 %144 %147
OpStore %_9_m11 %148
%149 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%151 = OpLoad %v4float %149
OpReturnValue %151
OpFunctionEnd
