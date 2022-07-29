OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %color_burn_component_Qhh2h2 "color_burn_component_Qhh2h2"
OpName %delta "delta"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %20 RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %delta RelaxedPrecision
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
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%16 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%_ptr_Function_float = OpTypePointer Function %float
%void = OpTypeVoid
%96 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%color_burn_component_Qhh2h2 = OpFunction %float None %16
%17 = OpFunctionParameter %_ptr_Function_v2float
%18 = OpFunctionParameter %_ptr_Function_v2float
%19 = OpLabel
%delta = OpVariable %_ptr_Function_float Function
%20 = OpLoad %v2float %18
%21 = OpCompositeExtract %float %20 1
%22 = OpLoad %v2float %18
%23 = OpCompositeExtract %float %22 0
%24 = OpFOrdEqual %bool %21 %23
OpSelectionMerge %27 None
OpBranchConditional %24 %25 %26
%25 = OpLabel
%28 = OpLoad %v2float %17
%29 = OpCompositeExtract %float %28 1
%30 = OpLoad %v2float %18
%31 = OpCompositeExtract %float %30 1
%32 = OpFMul %float %29 %31
%33 = OpLoad %v2float %17
%34 = OpCompositeExtract %float %33 0
%36 = OpLoad %v2float %18
%37 = OpCompositeExtract %float %36 1
%38 = OpFSub %float %float_1 %37
%39 = OpFMul %float %34 %38
%40 = OpFAdd %float %32 %39
%41 = OpLoad %v2float %18
%42 = OpCompositeExtract %float %41 0
%43 = OpLoad %v2float %17
%44 = OpCompositeExtract %float %43 1
%45 = OpFSub %float %float_1 %44
%46 = OpFMul %float %42 %45
%47 = OpFAdd %float %40 %46
OpReturnValue %47
%26 = OpLabel
%48 = OpLoad %v2float %17
%49 = OpCompositeExtract %float %48 0
%51 = OpFOrdEqual %bool %49 %float_0
OpSelectionMerge %54 None
OpBranchConditional %51 %52 %53
%52 = OpLabel
%55 = OpLoad %v2float %18
%56 = OpCompositeExtract %float %55 0
%57 = OpLoad %v2float %17
%58 = OpCompositeExtract %float %57 1
%59 = OpFSub %float %float_1 %58
%60 = OpFMul %float %56 %59
OpReturnValue %60
%53 = OpLabel
%64 = OpLoad %v2float %18
%65 = OpCompositeExtract %float %64 1
%66 = OpLoad %v2float %18
%67 = OpCompositeExtract %float %66 1
%68 = OpLoad %v2float %18
%69 = OpCompositeExtract %float %68 0
%70 = OpFSub %float %67 %69
%71 = OpLoad %v2float %17
%72 = OpCompositeExtract %float %71 1
%73 = OpFMul %float %70 %72
%74 = OpLoad %v2float %17
%75 = OpCompositeExtract %float %74 0
%76 = OpFDiv %float %73 %75
%77 = OpFSub %float %65 %76
%63 = OpExtInst %float %1 FMax %float_0 %77
OpStore %delta %63
%78 = OpLoad %v2float %17
%79 = OpCompositeExtract %float %78 1
%80 = OpFMul %float %63 %79
%81 = OpLoad %v2float %17
%82 = OpCompositeExtract %float %81 0
%83 = OpLoad %v2float %18
%84 = OpCompositeExtract %float %83 1
%85 = OpFSub %float %float_1 %84
%86 = OpFMul %float %82 %85
%87 = OpFAdd %float %80 %86
%88 = OpLoad %v2float %18
%89 = OpCompositeExtract %float %88 0
%90 = OpLoad %v2float %17
%91 = OpCompositeExtract %float %90 1
%92 = OpFSub %float %float_1 %91
%93 = OpFMul %float %89 %92
%94 = OpFAdd %float %87 %93
OpReturnValue %94
%54 = OpLabel
OpBranch %27
%27 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %96
%97 = OpLabel
%104 = OpVariable %_ptr_Function_v2float Function
%109 = OpVariable %_ptr_Function_v2float Function
%114 = OpVariable %_ptr_Function_v2float Function
%118 = OpVariable %_ptr_Function_v2float Function
%123 = OpVariable %_ptr_Function_v2float Function
%127 = OpVariable %_ptr_Function_v2float Function
%98 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%102 = OpLoad %v4float %98
%103 = OpVectorShuffle %v2float %102 %102 0 3
OpStore %104 %103
%105 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%107 = OpLoad %v4float %105
%108 = OpVectorShuffle %v2float %107 %107 0 3
OpStore %109 %108
%110 = OpFunctionCall %float %color_burn_component_Qhh2h2 %104 %109
%111 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%112 = OpLoad %v4float %111
%113 = OpVectorShuffle %v2float %112 %112 1 3
OpStore %114 %113
%115 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%116 = OpLoad %v4float %115
%117 = OpVectorShuffle %v2float %116 %116 1 3
OpStore %118 %117
%119 = OpFunctionCall %float %color_burn_component_Qhh2h2 %114 %118
%120 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%121 = OpLoad %v4float %120
%122 = OpVectorShuffle %v2float %121 %121 2 3
OpStore %123 %122
%124 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%125 = OpLoad %v4float %124
%126 = OpVectorShuffle %v2float %125 %125 2 3
OpStore %127 %126
%128 = OpFunctionCall %float %color_burn_component_Qhh2h2 %123 %127
%129 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%130 = OpLoad %v4float %129
%131 = OpCompositeExtract %float %130 3
%132 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%133 = OpLoad %v4float %132
%134 = OpCompositeExtract %float %133 3
%135 = OpFSub %float %float_1 %134
%136 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%137 = OpLoad %v4float %136
%138 = OpCompositeExtract %float %137 3
%139 = OpFMul %float %135 %138
%140 = OpFAdd %float %131 %139
%141 = OpCompositeConstruct %v4float %110 %119 %128 %140
OpStore %sk_FragColor %141
OpReturn
OpFunctionEnd
