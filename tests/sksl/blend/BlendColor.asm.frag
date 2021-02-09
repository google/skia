OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %_blend_color_luminance "_blend_color_luminance"
OpName %_blend_set_color_luminance "_blend_set_color_luminance"
OpName %lum "lum"
OpName %result "result"
OpName %minComp "minComp"
OpName %maxComp "maxComp"
OpName %blend_color "blend_color"
OpName %alpha "alpha"
OpName %sda "sda"
OpName %dsa "dsa"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
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
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%17 = OpTypeFunction %float %_ptr_Function_v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%25 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%_ptr_Function_float = OpTypePointer Function %float
%27 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float %_ptr_Function_v3float
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%_ptr_Function_v4float = OpTypePointer Function %v4float
%120 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%void = OpTypeVoid
%172 = OpTypeFunction %void
%_blend_color_luminance = OpFunction %float None %17
%19 = OpFunctionParameter %_ptr_Function_v3float
%20 = OpLabel
%26 = OpLoad %v3float %19
%21 = OpDot %float %25 %26
OpReturnValue %21
OpFunctionEnd
%_blend_set_color_luminance = OpFunction %v3float None %27
%29 = OpFunctionParameter %_ptr_Function_v3float
%30 = OpFunctionParameter %_ptr_Function_float
%31 = OpFunctionParameter %_ptr_Function_v3float
%32 = OpLabel
%lum = OpVariable %_ptr_Function_float Function
%result = OpVariable %_ptr_Function_v3float Function
%minComp = OpVariable %_ptr_Function_float Function
%maxComp = OpVariable %_ptr_Function_float Function
%98 = OpVariable %_ptr_Function_v3float Function
%35 = OpLoad %v3float %31
%34 = OpDot %float %25 %35
OpStore %lum %34
%37 = OpLoad %float %lum
%39 = OpLoad %v3float %29
%38 = OpDot %float %25 %39
%40 = OpFSub %float %37 %38
%41 = OpLoad %v3float %29
%42 = OpCompositeConstruct %v3float %40 %40 %40
%43 = OpFAdd %v3float %42 %41
OpStore %result %43
%47 = OpLoad %v3float %result
%48 = OpCompositeExtract %float %47 0
%49 = OpLoad %v3float %result
%50 = OpCompositeExtract %float %49 1
%46 = OpExtInst %float %1 FMin %48 %50
%51 = OpLoad %v3float %result
%52 = OpCompositeExtract %float %51 2
%45 = OpExtInst %float %1 FMin %46 %52
OpStore %minComp %45
%56 = OpLoad %v3float %result
%57 = OpCompositeExtract %float %56 0
%58 = OpLoad %v3float %result
%59 = OpCompositeExtract %float %58 1
%55 = OpExtInst %float %1 FMax %57 %59
%60 = OpLoad %v3float %result
%61 = OpCompositeExtract %float %60 2
%54 = OpExtInst %float %1 FMax %55 %61
OpStore %maxComp %54
%63 = OpLoad %float %minComp
%65 = OpFOrdLessThan %bool %63 %float_0
OpSelectionMerge %67 None
OpBranchConditional %65 %66 %67
%66 = OpLabel
%68 = OpLoad %float %lum
%69 = OpLoad %float %minComp
%70 = OpFOrdNotEqual %bool %68 %69
OpBranch %67
%67 = OpLabel
%71 = OpPhi %bool %false %32 %70 %66
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%74 = OpLoad %float %lum
%75 = OpLoad %v3float %result
%76 = OpLoad %float %lum
%77 = OpCompositeConstruct %v3float %76 %76 %76
%78 = OpFSub %v3float %75 %77
%79 = OpLoad %float %lum
%80 = OpVectorTimesScalar %v3float %78 %79
%81 = OpLoad %float %lum
%82 = OpLoad %float %minComp
%83 = OpFSub %float %81 %82
%85 = OpFDiv %float %float_1 %83
%86 = OpVectorTimesScalar %v3float %80 %85
%87 = OpCompositeConstruct %v3float %74 %74 %74
%88 = OpFAdd %v3float %87 %86
OpStore %result %88
OpBranch %73
%73 = OpLabel
%89 = OpLoad %float %maxComp
%90 = OpLoad %float %30
%91 = OpFOrdGreaterThan %bool %89 %90
OpSelectionMerge %93 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%94 = OpLoad %float %maxComp
%95 = OpLoad %float %lum
%96 = OpFOrdNotEqual %bool %94 %95
OpBranch %93
%93 = OpLabel
%97 = OpPhi %bool %false %73 %96 %92
OpSelectionMerge %101 None
OpBranchConditional %97 %99 %100
%99 = OpLabel
%102 = OpLoad %float %lum
%103 = OpLoad %v3float %result
%104 = OpLoad %float %lum
%105 = OpCompositeConstruct %v3float %104 %104 %104
%106 = OpFSub %v3float %103 %105
%107 = OpLoad %float %30
%108 = OpLoad %float %lum
%109 = OpFSub %float %107 %108
%110 = OpVectorTimesScalar %v3float %106 %109
%111 = OpLoad %float %maxComp
%112 = OpLoad %float %lum
%113 = OpFSub %float %111 %112
%114 = OpFDiv %float %float_1 %113
%115 = OpVectorTimesScalar %v3float %110 %114
%116 = OpCompositeConstruct %v3float %102 %102 %102
%117 = OpFAdd %v3float %116 %115
OpStore %98 %117
OpBranch %101
%100 = OpLabel
%118 = OpLoad %v3float %result
OpStore %98 %118
OpBranch %101
%101 = OpLabel
%119 = OpLoad %v3float %98
OpReturnValue %119
OpFunctionEnd
%blend_color = OpFunction %v4float None %120
%122 = OpFunctionParameter %_ptr_Function_v4float
%123 = OpFunctionParameter %_ptr_Function_v4float
%124 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%144 = OpVariable %_ptr_Function_v3float Function
%146 = OpVariable %_ptr_Function_float Function
%148 = OpVariable %_ptr_Function_v3float Function
%126 = OpLoad %v4float %123
%127 = OpCompositeExtract %float %126 3
%128 = OpLoad %v4float %122
%129 = OpCompositeExtract %float %128 3
%130 = OpFMul %float %127 %129
OpStore %alpha %130
%132 = OpLoad %v4float %122
%133 = OpVectorShuffle %v3float %132 %132 0 1 2
%134 = OpLoad %v4float %123
%135 = OpCompositeExtract %float %134 3
%136 = OpVectorTimesScalar %v3float %133 %135
OpStore %sda %136
%138 = OpLoad %v4float %123
%139 = OpVectorShuffle %v3float %138 %138 0 1 2
%140 = OpLoad %v4float %122
%141 = OpCompositeExtract %float %140 3
%142 = OpVectorTimesScalar %v3float %139 %141
OpStore %dsa %142
%143 = OpLoad %v3float %sda
OpStore %144 %143
%145 = OpLoad %float %alpha
OpStore %146 %145
%147 = OpLoad %v3float %dsa
OpStore %148 %147
%149 = OpFunctionCall %v3float %_blend_set_color_luminance %144 %146 %148
%150 = OpLoad %v4float %123
%151 = OpVectorShuffle %v3float %150 %150 0 1 2
%152 = OpFAdd %v3float %149 %151
%153 = OpLoad %v3float %dsa
%154 = OpFSub %v3float %152 %153
%155 = OpLoad %v4float %122
%156 = OpVectorShuffle %v3float %155 %155 0 1 2
%157 = OpFAdd %v3float %154 %156
%158 = OpLoad %v3float %sda
%159 = OpFSub %v3float %157 %158
%160 = OpCompositeExtract %float %159 0
%161 = OpCompositeExtract %float %159 1
%162 = OpCompositeExtract %float %159 2
%163 = OpLoad %v4float %122
%164 = OpCompositeExtract %float %163 3
%165 = OpLoad %v4float %123
%166 = OpCompositeExtract %float %165 3
%167 = OpFAdd %float %164 %166
%168 = OpLoad %float %alpha
%169 = OpFSub %float %167 %168
%170 = OpCompositeConstruct %v4float %160 %161 %162 %169
OpReturnValue %170
OpFunctionEnd
%main = OpFunction %void None %172
%173 = OpLabel
%175 = OpVariable %_ptr_Function_v4float Function
%177 = OpVariable %_ptr_Function_v4float Function
%174 = OpLoad %v4float %src
OpStore %175 %174
%176 = OpLoad %v4float %dst
OpStore %177 %176
%178 = OpFunctionCall %v4float %blend_color %175 %177
OpStore %sk_FragColor %178
OpReturn
OpFunctionEnd
