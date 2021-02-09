OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %_guarded_divide "_guarded_divide"
OpName %_color_burn_component "_color_burn_component"
OpName %_6_n "_6_n"
OpName %delta "delta"
OpName %blend_color_burn "blend_color_burn"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%src = OpVariable %_ptr_Input_v4float Input
%dst = OpVariable %_ptr_Input_v4float Input
%_ptr_Function_float = OpTypePointer Function %float
%16 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%107 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%void = OpTypeVoid
%144 = OpTypeFunction %void
%_guarded_divide = OpFunction %float None %16
%18 = OpFunctionParameter %_ptr_Function_float
%19 = OpFunctionParameter %_ptr_Function_float
%20 = OpLabel
%21 = OpLoad %float %18
%22 = OpLoad %float %19
%23 = OpFDiv %float %21 %22
OpReturnValue %23
OpFunctionEnd
%_color_burn_component = OpFunction %float None %25
%27 = OpFunctionParameter %_ptr_Function_v2float
%28 = OpFunctionParameter %_ptr_Function_v2float
%29 = OpLabel
%_6_n = OpVariable %_ptr_Function_float Function
%delta = OpVariable %_ptr_Function_float Function
%30 = OpLoad %v2float %28
%31 = OpCompositeExtract %float %30 1
%32 = OpLoad %v2float %28
%33 = OpCompositeExtract %float %32 0
%34 = OpFOrdEqual %bool %31 %33
OpSelectionMerge %37 None
OpBranchConditional %34 %35 %36
%35 = OpLabel
%38 = OpLoad %v2float %27
%39 = OpCompositeExtract %float %38 1
%40 = OpLoad %v2float %28
%41 = OpCompositeExtract %float %40 1
%42 = OpFMul %float %39 %41
%43 = OpLoad %v2float %27
%44 = OpCompositeExtract %float %43 0
%46 = OpLoad %v2float %28
%47 = OpCompositeExtract %float %46 1
%48 = OpFSub %float %float_1 %47
%49 = OpFMul %float %44 %48
%50 = OpFAdd %float %42 %49
%51 = OpLoad %v2float %28
%52 = OpCompositeExtract %float %51 0
%53 = OpLoad %v2float %27
%54 = OpCompositeExtract %float %53 1
%55 = OpFSub %float %float_1 %54
%56 = OpFMul %float %52 %55
%57 = OpFAdd %float %50 %56
OpReturnValue %57
%36 = OpLabel
%58 = OpLoad %v2float %27
%59 = OpCompositeExtract %float %58 0
%61 = OpFOrdEqual %bool %59 %float_0
OpSelectionMerge %64 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%65 = OpLoad %v2float %28
%66 = OpCompositeExtract %float %65 0
%67 = OpLoad %v2float %27
%68 = OpCompositeExtract %float %67 1
%69 = OpFSub %float %float_1 %68
%70 = OpFMul %float %66 %69
OpReturnValue %70
%63 = OpLabel
%72 = OpLoad %v2float %28
%73 = OpCompositeExtract %float %72 1
%74 = OpLoad %v2float %28
%75 = OpCompositeExtract %float %74 0
%76 = OpFSub %float %73 %75
%77 = OpLoad %v2float %27
%78 = OpCompositeExtract %float %77 1
%79 = OpFMul %float %76 %78
OpStore %_6_n %79
%82 = OpLoad %v2float %28
%83 = OpCompositeExtract %float %82 1
%84 = OpLoad %float %_6_n
%85 = OpLoad %v2float %27
%86 = OpCompositeExtract %float %85 0
%87 = OpFDiv %float %84 %86
%88 = OpFSub %float %83 %87
%81 = OpExtInst %float %1 FMax %float_0 %88
OpStore %delta %81
%89 = OpLoad %float %delta
%90 = OpLoad %v2float %27
%91 = OpCompositeExtract %float %90 1
%92 = OpFMul %float %89 %91
%93 = OpLoad %v2float %27
%94 = OpCompositeExtract %float %93 0
%95 = OpLoad %v2float %28
%96 = OpCompositeExtract %float %95 1
%97 = OpFSub %float %float_1 %96
%98 = OpFMul %float %94 %97
%99 = OpFAdd %float %92 %98
%100 = OpLoad %v2float %28
%101 = OpCompositeExtract %float %100 0
%102 = OpLoad %v2float %27
%103 = OpCompositeExtract %float %102 1
%104 = OpFSub %float %float_1 %103
%105 = OpFMul %float %101 %104
%106 = OpFAdd %float %99 %105
OpReturnValue %106
%64 = OpLabel
OpBranch %37
%37 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_color_burn = OpFunction %v4float None %107
%109 = OpFunctionParameter %_ptr_Function_v4float
%110 = OpFunctionParameter %_ptr_Function_v4float
%111 = OpLabel
%114 = OpVariable %_ptr_Function_v2float Function
%117 = OpVariable %_ptr_Function_v2float Function
%121 = OpVariable %_ptr_Function_v2float Function
%124 = OpVariable %_ptr_Function_v2float Function
%128 = OpVariable %_ptr_Function_v2float Function
%131 = OpVariable %_ptr_Function_v2float Function
%112 = OpLoad %v4float %109
%113 = OpVectorShuffle %v2float %112 %112 0 3
OpStore %114 %113
%115 = OpLoad %v4float %110
%116 = OpVectorShuffle %v2float %115 %115 0 3
OpStore %117 %116
%118 = OpFunctionCall %float %_color_burn_component %114 %117
%119 = OpLoad %v4float %109
%120 = OpVectorShuffle %v2float %119 %119 1 3
OpStore %121 %120
%122 = OpLoad %v4float %110
%123 = OpVectorShuffle %v2float %122 %122 1 3
OpStore %124 %123
%125 = OpFunctionCall %float %_color_burn_component %121 %124
%126 = OpLoad %v4float %109
%127 = OpVectorShuffle %v2float %126 %126 2 3
OpStore %128 %127
%129 = OpLoad %v4float %110
%130 = OpVectorShuffle %v2float %129 %129 2 3
OpStore %131 %130
%132 = OpFunctionCall %float %_color_burn_component %128 %131
%133 = OpLoad %v4float %109
%134 = OpCompositeExtract %float %133 3
%135 = OpLoad %v4float %109
%136 = OpCompositeExtract %float %135 3
%137 = OpFSub %float %float_1 %136
%138 = OpLoad %v4float %110
%139 = OpCompositeExtract %float %138 3
%140 = OpFMul %float %137 %139
%141 = OpFAdd %float %134 %140
%142 = OpCompositeConstruct %v4float %118 %125 %132 %141
OpReturnValue %142
OpFunctionEnd
%main = OpFunction %void None %144
%145 = OpLabel
%147 = OpVariable %_ptr_Function_v4float Function
%149 = OpVariable %_ptr_Function_v4float Function
%146 = OpLoad %v4float %src
OpStore %147 %146
%148 = OpLoad %v4float %dst
OpStore %149 %148
%150 = OpFunctionCall %v4float %blend_color_burn %147 %149
OpStore %sk_FragColor %150
OpReturn
OpFunctionEnd
