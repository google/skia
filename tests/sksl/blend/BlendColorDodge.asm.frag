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
OpName %_color_dodge_component_hh2h2 "_color_dodge_component_hh2h2"
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
OpDecorate %27 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %delta RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
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
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%_ptr_Function_float = OpTypePointer Function %float
%void = OpTypeVoid
%94 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_color_dodge_component_hh2h2 = OpFunction %float None %15
%17 = OpFunctionParameter %_ptr_Function_v2float
%18 = OpFunctionParameter %_ptr_Function_v2float
%19 = OpLabel
%delta = OpVariable %_ptr_Function_float Function
%20 = OpLoad %v2float %18
%21 = OpCompositeExtract %float %20 0
%23 = OpFOrdEqual %bool %21 %float_0
OpSelectionMerge %26 None
OpBranchConditional %23 %24 %25
%24 = OpLabel
%27 = OpLoad %v2float %17
%28 = OpCompositeExtract %float %27 0
%30 = OpLoad %v2float %18
%31 = OpCompositeExtract %float %30 1
%32 = OpFSub %float %float_1 %31
%33 = OpFMul %float %28 %32
OpReturnValue %33
%25 = OpLabel
%36 = OpLoad %v2float %17
%37 = OpCompositeExtract %float %36 1
%38 = OpLoad %v2float %17
%39 = OpCompositeExtract %float %38 0
%40 = OpFSub %float %37 %39
OpStore %delta %40
%41 = OpLoad %float %delta
%42 = OpFOrdEqual %bool %41 %float_0
OpSelectionMerge %45 None
OpBranchConditional %42 %43 %44
%43 = OpLabel
%46 = OpLoad %v2float %17
%47 = OpCompositeExtract %float %46 1
%48 = OpLoad %v2float %18
%49 = OpCompositeExtract %float %48 1
%50 = OpFMul %float %47 %49
%51 = OpLoad %v2float %17
%52 = OpCompositeExtract %float %51 0
%53 = OpLoad %v2float %18
%54 = OpCompositeExtract %float %53 1
%55 = OpFSub %float %float_1 %54
%56 = OpFMul %float %52 %55
%57 = OpFAdd %float %50 %56
%58 = OpLoad %v2float %18
%59 = OpCompositeExtract %float %58 0
%60 = OpLoad %v2float %17
%61 = OpCompositeExtract %float %60 1
%62 = OpFSub %float %float_1 %61
%63 = OpFMul %float %59 %62
%64 = OpFAdd %float %57 %63
OpReturnValue %64
%44 = OpLabel
%66 = OpLoad %v2float %18
%67 = OpCompositeExtract %float %66 1
%68 = OpLoad %v2float %18
%69 = OpCompositeExtract %float %68 0
%70 = OpLoad %v2float %17
%71 = OpCompositeExtract %float %70 1
%72 = OpFMul %float %69 %71
%73 = OpLoad %float %delta
%74 = OpFDiv %float %72 %73
%65 = OpExtInst %float %1 FMin %67 %74
OpStore %delta %65
%75 = OpLoad %float %delta
%76 = OpLoad %v2float %17
%77 = OpCompositeExtract %float %76 1
%78 = OpFMul %float %75 %77
%79 = OpLoad %v2float %17
%80 = OpCompositeExtract %float %79 0
%81 = OpLoad %v2float %18
%82 = OpCompositeExtract %float %81 1
%83 = OpFSub %float %float_1 %82
%84 = OpFMul %float %80 %83
%85 = OpFAdd %float %78 %84
%86 = OpLoad %v2float %18
%87 = OpCompositeExtract %float %86 0
%88 = OpLoad %v2float %17
%89 = OpCompositeExtract %float %88 1
%90 = OpFSub %float %float_1 %89
%91 = OpFMul %float %87 %90
%92 = OpFAdd %float %85 %91
OpReturnValue %92
%45 = OpLabel
OpBranch %26
%26 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %94
%95 = OpLabel
%102 = OpVariable %_ptr_Function_v2float Function
%107 = OpVariable %_ptr_Function_v2float Function
%112 = OpVariable %_ptr_Function_v2float Function
%116 = OpVariable %_ptr_Function_v2float Function
%121 = OpVariable %_ptr_Function_v2float Function
%125 = OpVariable %_ptr_Function_v2float Function
%96 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%100 = OpLoad %v4float %96
%101 = OpVectorShuffle %v2float %100 %100 0 3
OpStore %102 %101
%103 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%105 = OpLoad %v4float %103
%106 = OpVectorShuffle %v2float %105 %105 0 3
OpStore %107 %106
%108 = OpFunctionCall %float %_color_dodge_component_hh2h2 %102 %107
%109 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%110 = OpLoad %v4float %109
%111 = OpVectorShuffle %v2float %110 %110 1 3
OpStore %112 %111
%113 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%114 = OpLoad %v4float %113
%115 = OpVectorShuffle %v2float %114 %114 1 3
OpStore %116 %115
%117 = OpFunctionCall %float %_color_dodge_component_hh2h2 %112 %116
%118 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%119 = OpLoad %v4float %118
%120 = OpVectorShuffle %v2float %119 %119 2 3
OpStore %121 %120
%122 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%123 = OpLoad %v4float %122
%124 = OpVectorShuffle %v2float %123 %123 2 3
OpStore %125 %124
%126 = OpFunctionCall %float %_color_dodge_component_hh2h2 %121 %125
%127 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%128 = OpLoad %v4float %127
%129 = OpCompositeExtract %float %128 3
%130 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%131 = OpLoad %v4float %130
%132 = OpCompositeExtract %float %131 3
%133 = OpFSub %float %float_1 %132
%134 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%135 = OpLoad %v4float %134
%136 = OpCompositeExtract %float %135 3
%137 = OpFMul %float %133 %136
%138 = OpFAdd %float %129 %137
%139 = OpCompositeConstruct %v4float %108 %117 %126 %138
OpStore %sk_FragColor %139
OpReturn
OpFunctionEnd
