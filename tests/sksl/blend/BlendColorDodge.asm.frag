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
OpName %color_dodge_component_Qhh2h2 "color_dodge_component_Qhh2h2"
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
OpDecorate %45 RelaxedPrecision
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
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
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
%16 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%_ptr_Function_float = OpTypePointer Function %float
%void = OpTypeVoid
%91 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%color_dodge_component_Qhh2h2 = OpFunction %float None %16
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
%41 = OpFOrdEqual %bool %40 %float_0
OpSelectionMerge %44 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
%45 = OpLoad %v2float %17
%46 = OpCompositeExtract %float %45 1
%47 = OpLoad %v2float %18
%48 = OpCompositeExtract %float %47 1
%49 = OpFMul %float %46 %48
%50 = OpLoad %v2float %17
%51 = OpCompositeExtract %float %50 0
%52 = OpLoad %v2float %18
%53 = OpCompositeExtract %float %52 1
%54 = OpFSub %float %float_1 %53
%55 = OpFMul %float %51 %54
%56 = OpFAdd %float %49 %55
%57 = OpLoad %v2float %18
%58 = OpCompositeExtract %float %57 0
%59 = OpLoad %v2float %17
%60 = OpCompositeExtract %float %59 1
%61 = OpFSub %float %float_1 %60
%62 = OpFMul %float %58 %61
%63 = OpFAdd %float %56 %62
OpReturnValue %63
%43 = OpLabel
%65 = OpLoad %v2float %18
%66 = OpCompositeExtract %float %65 1
%67 = OpLoad %v2float %18
%68 = OpCompositeExtract %float %67 0
%69 = OpLoad %v2float %17
%70 = OpCompositeExtract %float %69 1
%71 = OpFMul %float %68 %70
%72 = OpFDiv %float %71 %40
%64 = OpExtInst %float %1 FMin %66 %72
OpStore %delta %64
%73 = OpLoad %v2float %17
%74 = OpCompositeExtract %float %73 1
%75 = OpFMul %float %64 %74
%76 = OpLoad %v2float %17
%77 = OpCompositeExtract %float %76 0
%78 = OpLoad %v2float %18
%79 = OpCompositeExtract %float %78 1
%80 = OpFSub %float %float_1 %79
%81 = OpFMul %float %77 %80
%82 = OpFAdd %float %75 %81
%83 = OpLoad %v2float %18
%84 = OpCompositeExtract %float %83 0
%85 = OpLoad %v2float %17
%86 = OpCompositeExtract %float %85 1
%87 = OpFSub %float %float_1 %86
%88 = OpFMul %float %84 %87
%89 = OpFAdd %float %82 %88
OpReturnValue %89
%44 = OpLabel
OpBranch %26
%26 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %91
%92 = OpLabel
%99 = OpVariable %_ptr_Function_v2float Function
%104 = OpVariable %_ptr_Function_v2float Function
%109 = OpVariable %_ptr_Function_v2float Function
%113 = OpVariable %_ptr_Function_v2float Function
%118 = OpVariable %_ptr_Function_v2float Function
%122 = OpVariable %_ptr_Function_v2float Function
%93 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%97 = OpLoad %v4float %93
%98 = OpVectorShuffle %v2float %97 %97 0 3
OpStore %99 %98
%100 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%102 = OpLoad %v4float %100
%103 = OpVectorShuffle %v2float %102 %102 0 3
OpStore %104 %103
%105 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %99 %104
%106 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%107 = OpLoad %v4float %106
%108 = OpVectorShuffle %v2float %107 %107 1 3
OpStore %109 %108
%110 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%111 = OpLoad %v4float %110
%112 = OpVectorShuffle %v2float %111 %111 1 3
OpStore %113 %112
%114 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %109 %113
%115 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%116 = OpLoad %v4float %115
%117 = OpVectorShuffle %v2float %116 %116 2 3
OpStore %118 %117
%119 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%120 = OpLoad %v4float %119
%121 = OpVectorShuffle %v2float %120 %120 2 3
OpStore %122 %121
%123 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %118 %122
%124 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%125 = OpLoad %v4float %124
%126 = OpCompositeExtract %float %125 3
%127 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%128 = OpLoad %v4float %127
%129 = OpCompositeExtract %float %128 3
%130 = OpFSub %float %float_1 %129
%131 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%132 = OpLoad %v4float %131
%133 = OpCompositeExtract %float %132 3
%134 = OpFMul %float %130 %133
%135 = OpFAdd %float %126 %134
%136 = OpCompositeConstruct %v4float %105 %114 %123 %135
OpStore %sk_FragColor %136
OpReturn
OpFunctionEnd
