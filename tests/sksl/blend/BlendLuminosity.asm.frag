### Compilation failed:

error: SPIR-V validation error: Variable must be decorated with a location
  %src = OpVariable %_ptr_Input_v4float Input

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %_blend_set_color_luminance "_blend_set_color_luminance"
OpName %lum "lum"
OpName %result "result"
OpName %minComp "minComp"
OpName %maxComp "maxComp"
OpName %_4_d "_4_d"
OpName %_5_n "_5_n"
OpName %_6_d "_6_d"
OpName %blend_luminosity "blend_luminosity"
OpName %alpha "alpha"
OpName %sda "sda"
OpName %dsa "dsa"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
%16 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float %_ptr_Function_v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%28 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%_ptr_Function_v4float = OpTypePointer Function %v4float
%117 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%void = OpTypeVoid
%169 = OpTypeFunction %void
%_blend_set_color_luminance = OpFunction %v3float None %16
%19 = OpFunctionParameter %_ptr_Function_v3float
%20 = OpFunctionParameter %_ptr_Function_float
%21 = OpFunctionParameter %_ptr_Function_v3float
%22 = OpLabel
%lum = OpVariable %_ptr_Function_float Function
%result = OpVariable %_ptr_Function_v3float Function
%minComp = OpVariable %_ptr_Function_float Function
%maxComp = OpVariable %_ptr_Function_float Function
%_4_d = OpVariable %_ptr_Function_float Function
%_5_n = OpVariable %_ptr_Function_v3float Function
%_6_d = OpVariable %_ptr_Function_float Function
%29 = OpLoad %v3float %21
%24 = OpDot %float %28 %29
OpStore %lum %24
%31 = OpLoad %float %lum
%33 = OpLoad %v3float %19
%32 = OpDot %float %28 %33
%34 = OpFSub %float %31 %32
%35 = OpLoad %v3float %19
%36 = OpCompositeConstruct %v3float %34 %34 %34
%37 = OpFAdd %v3float %36 %35
OpStore %result %37
%41 = OpLoad %v3float %result
%42 = OpCompositeExtract %float %41 0
%43 = OpLoad %v3float %result
%44 = OpCompositeExtract %float %43 1
%40 = OpExtInst %float %1 FMin %42 %44
%45 = OpLoad %v3float %result
%46 = OpCompositeExtract %float %45 2
%39 = OpExtInst %float %1 FMin %40 %46
OpStore %minComp %39
%50 = OpLoad %v3float %result
%51 = OpCompositeExtract %float %50 0
%52 = OpLoad %v3float %result
%53 = OpCompositeExtract %float %52 1
%49 = OpExtInst %float %1 FMax %51 %53
%54 = OpLoad %v3float %result
%55 = OpCompositeExtract %float %54 2
%48 = OpExtInst %float %1 FMax %49 %55
OpStore %maxComp %48
%57 = OpLoad %float %minComp
%59 = OpFOrdLessThan %bool %57 %float_0
OpSelectionMerge %61 None
OpBranchConditional %59 %60 %61
%60 = OpLabel
%62 = OpLoad %float %lum
%63 = OpLoad %float %minComp
%64 = OpFOrdNotEqual %bool %62 %63
OpBranch %61
%61 = OpLabel
%65 = OpPhi %bool %false %22 %64 %60
OpSelectionMerge %67 None
OpBranchConditional %65 %66 %67
%66 = OpLabel
%69 = OpLoad %float %lum
%70 = OpLoad %float %minComp
%71 = OpFSub %float %69 %70
OpStore %_4_d %71
%72 = OpLoad %float %lum
%73 = OpLoad %v3float %result
%74 = OpLoad %float %lum
%75 = OpCompositeConstruct %v3float %74 %74 %74
%76 = OpFSub %v3float %73 %75
%77 = OpLoad %float %lum
%78 = OpLoad %float %_4_d
%79 = OpFDiv %float %77 %78
%80 = OpVectorTimesScalar %v3float %76 %79
%81 = OpCompositeConstruct %v3float %72 %72 %72
%82 = OpFAdd %v3float %81 %80
OpStore %result %82
OpBranch %67
%67 = OpLabel
%83 = OpLoad %float %maxComp
%84 = OpLoad %float %20
%85 = OpFOrdGreaterThan %bool %83 %84
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%88 = OpLoad %float %maxComp
%89 = OpLoad %float %lum
%90 = OpFOrdNotEqual %bool %88 %89
OpBranch %87
%87 = OpLabel
%91 = OpPhi %bool %false %67 %90 %86
OpSelectionMerge %94 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%96 = OpLoad %v3float %result
%97 = OpLoad %float %lum
%98 = OpCompositeConstruct %v3float %97 %97 %97
%99 = OpFSub %v3float %96 %98
%100 = OpLoad %float %20
%101 = OpLoad %float %lum
%102 = OpFSub %float %100 %101
%103 = OpVectorTimesScalar %v3float %99 %102
OpStore %_5_n %103
%105 = OpLoad %float %maxComp
%106 = OpLoad %float %lum
%107 = OpFSub %float %105 %106
OpStore %_6_d %107
%108 = OpLoad %float %lum
%109 = OpLoad %v3float %_5_n
%110 = OpLoad %float %_6_d
%112 = OpFDiv %float %float_1 %110
%113 = OpVectorTimesScalar %v3float %109 %112
%114 = OpCompositeConstruct %v3float %108 %108 %108
%115 = OpFAdd %v3float %114 %113
OpReturnValue %115
%93 = OpLabel
%116 = OpLoad %v3float %result
OpReturnValue %116
%94 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_luminosity = OpFunction %v4float None %117
%119 = OpFunctionParameter %_ptr_Function_v4float
%120 = OpFunctionParameter %_ptr_Function_v4float
%121 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%141 = OpVariable %_ptr_Function_v3float Function
%143 = OpVariable %_ptr_Function_float Function
%145 = OpVariable %_ptr_Function_v3float Function
%123 = OpLoad %v4float %120
%124 = OpCompositeExtract %float %123 3
%125 = OpLoad %v4float %119
%126 = OpCompositeExtract %float %125 3
%127 = OpFMul %float %124 %126
OpStore %alpha %127
%129 = OpLoad %v4float %119
%130 = OpVectorShuffle %v3float %129 %129 0 1 2
%131 = OpLoad %v4float %120
%132 = OpCompositeExtract %float %131 3
%133 = OpVectorTimesScalar %v3float %130 %132
OpStore %sda %133
%135 = OpLoad %v4float %120
%136 = OpVectorShuffle %v3float %135 %135 0 1 2
%137 = OpLoad %v4float %119
%138 = OpCompositeExtract %float %137 3
%139 = OpVectorTimesScalar %v3float %136 %138
OpStore %dsa %139
%140 = OpLoad %v3float %dsa
OpStore %141 %140
%142 = OpLoad %float %alpha
OpStore %143 %142
%144 = OpLoad %v3float %sda
OpStore %145 %144
%146 = OpFunctionCall %v3float %_blend_set_color_luminance %141 %143 %145
%147 = OpLoad %v4float %120
%148 = OpVectorShuffle %v3float %147 %147 0 1 2
%149 = OpFAdd %v3float %146 %148
%150 = OpLoad %v3float %dsa
%151 = OpFSub %v3float %149 %150
%152 = OpLoad %v4float %119
%153 = OpVectorShuffle %v3float %152 %152 0 1 2
%154 = OpFAdd %v3float %151 %153
%155 = OpLoad %v3float %sda
%156 = OpFSub %v3float %154 %155
%157 = OpCompositeExtract %float %156 0
%158 = OpCompositeExtract %float %156 1
%159 = OpCompositeExtract %float %156 2
%160 = OpLoad %v4float %119
%161 = OpCompositeExtract %float %160 3
%162 = OpLoad %v4float %120
%163 = OpCompositeExtract %float %162 3
%164 = OpFAdd %float %161 %163
%165 = OpLoad %float %alpha
%166 = OpFSub %float %164 %165
%167 = OpCompositeConstruct %v4float %157 %158 %159 %166
OpReturnValue %167
OpFunctionEnd
%main = OpFunction %void None %169
%170 = OpLabel
%172 = OpVariable %_ptr_Function_v4float Function
%174 = OpVariable %_ptr_Function_v4float Function
%171 = OpLoad %v4float %src
OpStore %172 %171
%173 = OpLoad %v4float %dst
OpStore %174 %173
%175 = OpFunctionCall %v4float %blend_luminosity %172 %174
OpStore %sk_FragColor %175
OpReturn
OpFunctionEnd

1 error
