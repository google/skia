OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %e "e"
OpName %m "m"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %17 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%22 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%float_2 = OpConstant %float 2
%28 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%float_3 = OpConstant %float 3
%34 = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
%int_1 = OpConstant %int 1
%float_4 = OpConstant %float 4
%41 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%float_5 = OpConstant %float 5
%47 = OpConstantComposite %v4float %float_5 %float_5 %float_5 %float_5
%float_6 = OpConstant %float 6
%53 = OpConstantComposite %v4float %float_6 %float_6 %float_6 %float_6
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_7 = OpConstant %float 7
%62 = OpConstantComposite %v4float %float_7 %float_7 %float_7 %float_7
%float_n7 = OpConstant %float -7
%64 = OpConstantComposite %v4float %float_n7 %float_n7 %float_n7 %float_n7
%float_8 = OpConstant %float 8
%73 = OpConstantComposite %v4float %float_8 %float_8 %float_8 %float_8
%float_n8 = OpConstant %float -8
%75 = OpConstantComposite %v4float %float_n8 %float_n8 %float_n8 %float_n8
%float_9 = OpConstant %float 9
%84 = OpConstantComposite %v4float %float_9 %float_9 %float_9 %float_9
%float_n9 = OpConstant %float -9
%86 = OpConstantComposite %v4float %float_n9 %float_n9 %float_n9 %float_n9
%float_10 = OpConstant %float 10
%95 = OpConstantComposite %v4float %float_10 %float_10 %float_10 %float_10
%float_n10 = OpConstant %float -10
%97 = OpConstantComposite %v4float %float_n10 %float_n10 %float_n10 %float_n10
%float_11 = OpConstant %float 11
%104 = OpConstantComposite %v4float %float_11 %float_11 %float_11 %float_11
%float_12 = OpConstant %float 12
%106 = OpConstantComposite %v4float %float_12 %float_12 %float_12 %float_12
%float_13 = OpConstant %float 13
%112 = OpConstantComposite %v4float %float_13 %float_13 %float_13 %float_13
%float_14 = OpConstant %float 14
%114 = OpConstantComposite %v4float %float_14 %float_14 %float_14 %float_14
%float_15 = OpConstant %float 15
%121 = OpConstantComposite %v4float %float_15 %float_15 %float_15 %float_15
%float_16 = OpConstant %float 16
%127 = OpConstantComposite %v4float %float_16 %float_16 %float_16 %float_16
%float_17 = OpConstant %float 17
%133 = OpConstantComposite %v4float %float_17 %float_17 %float_17 %float_17
%int_2 = OpConstant %int 2
%float_18 = OpConstant %float 18
%140 = OpConstantComposite %v4float %float_18 %float_18 %float_18 %float_18
%float_19 = OpConstant %float 19
%148 = OpConstantComposite %v4float %float_19 %float_19 %float_19 %float_19
%float_n19 = OpConstant %float -19
%150 = OpConstantComposite %v4float %float_n19 %float_n19 %float_n19 %float_n19
%float_20 = OpConstant %float 20
%159 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_20
%float_n20 = OpConstant %float -20
%161 = OpConstantComposite %v4float %float_n20 %float_n20 %float_n20 %float_n20
%float_21 = OpConstant %float 21
%169 = OpConstantComposite %v4float %float_21 %float_21 %float_21 %float_21
%float_22 = OpConstant %float 22
%171 = OpConstantComposite %v4float %float_22 %float_22 %float_22 %float_22
%float_23 = OpConstant %float 23
%173 = OpConstantComposite %v4float %float_23 %float_23 %float_23 %float_23
%main = OpFunction %void None %11
%12 = OpLabel
%e = OpVariable %_ptr_Function_int Function
%56 = OpVariable %_ptr_Function_v4float Function
%68 = OpVariable %_ptr_Function_v4float Function
%79 = OpVariable %_ptr_Function_v4float Function
%90 = OpVariable %_ptr_Function_v4float Function
%m = OpVariable %_ptr_Function_int Function
%143 = OpVariable %_ptr_Function_v4float Function
%154 = OpVariable %_ptr_Function_v4float Function
OpStore %e %int_0
%17 = OpLoad %int %e
%18 = OpIEqual %bool %17 %int_0
OpSelectionMerge %20 None
OpBranchConditional %18 %19 %20
%19 = OpLabel
OpStore %sk_FragColor %22
OpBranch %20
%20 = OpLabel
%23 = OpLoad %int %e
%24 = OpIEqual %bool %23 %int_0
OpSelectionMerge %26 None
OpBranchConditional %24 %25 %26
%25 = OpLabel
OpStore %sk_FragColor %28
OpBranch %26
%26 = OpLabel
%29 = OpLoad %int %e
%30 = OpINotEqual %bool %29 %int_0
OpSelectionMerge %32 None
OpBranchConditional %30 %31 %32
%31 = OpLabel
OpStore %sk_FragColor %34
OpBranch %32
%32 = OpLabel
%35 = OpLoad %int %e
%37 = OpIEqual %bool %35 %int_1
OpSelectionMerge %39 None
OpBranchConditional %37 %38 %39
%38 = OpLabel
OpStore %sk_FragColor %41
OpBranch %39
%39 = OpLabel
%42 = OpLoad %int %e
%43 = OpIEqual %bool %42 %int_1
OpSelectionMerge %45 None
OpBranchConditional %43 %44 %45
%44 = OpLabel
OpStore %sk_FragColor %47
OpBranch %45
%45 = OpLabel
%48 = OpLoad %int %e
%49 = OpINotEqual %bool %48 %int_1
OpSelectionMerge %51 None
OpBranchConditional %49 %50 %51
%50 = OpLabel
OpStore %sk_FragColor %53
OpBranch %51
%51 = OpLabel
%54 = OpLoad %int %e
%55 = OpIEqual %bool %54 %int_0
OpSelectionMerge %60 None
OpBranchConditional %55 %58 %59
%58 = OpLabel
OpStore %56 %62
OpBranch %60
%59 = OpLabel
OpStore %56 %64
OpBranch %60
%60 = OpLabel
%65 = OpLoad %v4float %56
OpStore %sk_FragColor %65
%66 = OpLoad %int %e
%67 = OpINotEqual %bool %66 %int_0
OpSelectionMerge %71 None
OpBranchConditional %67 %69 %70
%69 = OpLabel
OpStore %68 %73
OpBranch %71
%70 = OpLabel
OpStore %68 %75
OpBranch %71
%71 = OpLabel
%76 = OpLoad %v4float %68
OpStore %sk_FragColor %76
%77 = OpLoad %int %e
%78 = OpIEqual %bool %77 %int_1
OpSelectionMerge %82 None
OpBranchConditional %78 %80 %81
%80 = OpLabel
OpStore %79 %84
OpBranch %82
%81 = OpLabel
OpStore %79 %86
OpBranch %82
%82 = OpLabel
%87 = OpLoad %v4float %79
OpStore %sk_FragColor %87
%88 = OpLoad %int %e
%89 = OpINotEqual %bool %88 %int_1
OpSelectionMerge %93 None
OpBranchConditional %89 %91 %92
%91 = OpLabel
OpStore %90 %95
OpBranch %93
%92 = OpLabel
OpStore %90 %97
OpBranch %93
%93 = OpLabel
%98 = OpLoad %v4float %90
OpStore %sk_FragColor %98
%99 = OpLoad %int %e
OpSelectionMerge %100 None
OpSwitch %99 %100 0 %101 1 %102
%101 = OpLabel
OpStore %sk_FragColor %104
OpBranch %100
%102 = OpLabel
OpStore %sk_FragColor %106
OpBranch %100
%100 = OpLabel
%107 = OpLoad %int %e
OpSelectionMerge %108 None
OpSwitch %107 %108 0 %109 1 %110
%109 = OpLabel
OpStore %sk_FragColor %112
OpBranch %108
%110 = OpLabel
OpStore %sk_FragColor %114
OpBranch %108
%108 = OpLabel
OpStore %m %int_0
%116 = OpLoad %int %m
%117 = OpIEqual %bool %116 %int_0
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
OpStore %sk_FragColor %121
OpBranch %119
%119 = OpLabel
%122 = OpLoad %int %m
%123 = OpIEqual %bool %122 %int_0
OpSelectionMerge %125 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
OpStore %sk_FragColor %127
OpBranch %125
%125 = OpLabel
%128 = OpLoad %int %m
%129 = OpIEqual %bool %128 %int_1
OpSelectionMerge %131 None
OpBranchConditional %129 %130 %131
%130 = OpLabel
OpStore %sk_FragColor %133
OpBranch %131
%131 = OpLabel
%134 = OpLoad %int %m
%136 = OpINotEqual %bool %134 %int_2
OpSelectionMerge %138 None
OpBranchConditional %136 %137 %138
%137 = OpLabel
OpStore %sk_FragColor %140
OpBranch %138
%138 = OpLabel
%141 = OpLoad %int %m
%142 = OpIEqual %bool %141 %int_0
OpSelectionMerge %146 None
OpBranchConditional %142 %144 %145
%144 = OpLabel
OpStore %143 %148
OpBranch %146
%145 = OpLabel
OpStore %143 %150
OpBranch %146
%146 = OpLabel
%151 = OpLoad %v4float %143
OpStore %sk_FragColor %151
%152 = OpLoad %int %m
%153 = OpINotEqual %bool %152 %int_1
OpSelectionMerge %157 None
OpBranchConditional %153 %155 %156
%155 = OpLabel
OpStore %154 %159
OpBranch %157
%156 = OpLabel
OpStore %154 %161
OpBranch %157
%157 = OpLabel
%162 = OpLoad %v4float %154
OpStore %sk_FragColor %162
%163 = OpLoad %int %m
OpSelectionMerge %164 None
OpSwitch %163 %164 0 %165 1 %166 2 %167
%165 = OpLabel
OpStore %sk_FragColor %169
OpBranch %164
%166 = OpLabel
OpStore %sk_FragColor %171
OpBranch %164
%167 = OpLabel
OpStore %sk_FragColor %173
OpBranch %164
%164 = OpLabel
OpReturn
OpFunctionEnd
