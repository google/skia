OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %_color_burn_component_hh2h2 "_color_burn_component_hh2h2"
OpName %delta "delta"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
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
OpDecorate %95 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%15 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%_ptr_Function_float = OpTypePointer Function %float
%void = OpTypeVoid
%97 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_color_burn_component_hh2h2 = OpFunction %float None %15
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
%78 = OpLoad %float %delta
%79 = OpLoad %v2float %17
%80 = OpCompositeExtract %float %79 1
%81 = OpFMul %float %78 %80
%82 = OpLoad %v2float %17
%83 = OpCompositeExtract %float %82 0
%84 = OpLoad %v2float %18
%85 = OpCompositeExtract %float %84 1
%86 = OpFSub %float %float_1 %85
%87 = OpFMul %float %83 %86
%88 = OpFAdd %float %81 %87
%89 = OpLoad %v2float %18
%90 = OpCompositeExtract %float %89 0
%91 = OpLoad %v2float %17
%92 = OpCompositeExtract %float %91 1
%93 = OpFSub %float %float_1 %92
%94 = OpFMul %float %90 %93
%95 = OpFAdd %float %88 %94
OpReturnValue %95
%54 = OpLabel
OpBranch %27
%27 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %97
%98 = OpLabel
%105 = OpVariable %_ptr_Function_v2float Function
%110 = OpVariable %_ptr_Function_v2float Function
%115 = OpVariable %_ptr_Function_v2float Function
%119 = OpVariable %_ptr_Function_v2float Function
%124 = OpVariable %_ptr_Function_v2float Function
%128 = OpVariable %_ptr_Function_v2float Function
%99 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%103 = OpLoad %v4float %99
%104 = OpVectorShuffle %v2float %103 %103 0 3
OpStore %105 %104
%106 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%108 = OpLoad %v4float %106
%109 = OpVectorShuffle %v2float %108 %108 0 3
OpStore %110 %109
%111 = OpFunctionCall %float %_color_burn_component_hh2h2 %105 %110
%112 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%113 = OpLoad %v4float %112
%114 = OpVectorShuffle %v2float %113 %113 1 3
OpStore %115 %114
%116 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%117 = OpLoad %v4float %116
%118 = OpVectorShuffle %v2float %117 %117 1 3
OpStore %119 %118
%120 = OpFunctionCall %float %_color_burn_component_hh2h2 %115 %119
%121 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%122 = OpLoad %v4float %121
%123 = OpVectorShuffle %v2float %122 %122 2 3
OpStore %124 %123
%125 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%126 = OpLoad %v4float %125
%127 = OpVectorShuffle %v2float %126 %126 2 3
OpStore %128 %127
%129 = OpFunctionCall %float %_color_burn_component_hh2h2 %124 %128
%130 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%131 = OpLoad %v4float %130
%132 = OpCompositeExtract %float %131 3
%133 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%134 = OpLoad %v4float %133
%135 = OpCompositeExtract %float %134 3
%136 = OpFSub %float %float_1 %135
%137 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%138 = OpLoad %v4float %137
%139 = OpCompositeExtract %float %138 3
%140 = OpFMul %float %136 %139
%141 = OpFAdd %float %132 %140
%142 = OpCompositeConstruct %v4float %111 %120 %129 %141
OpStore %sk_FragColor %142
OpReturn
OpFunctionEnd
