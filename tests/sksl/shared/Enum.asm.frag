OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %f "f"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %14 RelaxedPrecision
OpDecorate %16 RelaxedPrecision
OpDecorate %18 RelaxedPrecision
OpDecorate %20 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%float_1 = OpConstant %float 1
%14 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%float_2 = OpConstant %float 2
%16 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%float_6 = OpConstant %float 6
%18 = OpConstantComposite %v4float %float_6 %float_6 %float_6 %float_6
%float_7 = OpConstant %float 7
%20 = OpConstantComposite %v4float %float_7 %float_7 %float_7 %float_7
%float_n8 = OpConstant %float -8
%22 = OpConstantComposite %v4float %float_n8 %float_n8 %float_n8 %float_n8
%float_n9 = OpConstant %float -9
%24 = OpConstantComposite %v4float %float_n9 %float_n9 %float_n9 %float_n9
%float_10 = OpConstant %float 10
%26 = OpConstantComposite %v4float %float_10 %float_10 %float_10 %float_10
%float_11 = OpConstant %float 11
%28 = OpConstantComposite %v4float %float_11 %float_11 %float_11 %float_11
%float_13 = OpConstant %float 13
%30 = OpConstantComposite %v4float %float_13 %float_13 %float_13 %float_13
%float_15 = OpConstant %float 15
%32 = OpConstantComposite %v4float %float_15 %float_15 %float_15 %float_15
%float_16 = OpConstant %float 16
%34 = OpConstantComposite %v4float %float_16 %float_16 %float_16 %float_16
%float_18 = OpConstant %float 18
%36 = OpConstantComposite %v4float %float_18 %float_18 %float_18 %float_18
%float_19 = OpConstant %float 19
%38 = OpConstantComposite %v4float %float_19 %float_19 %float_19 %float_19
%float_20 = OpConstant %float 20
%40 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_20
%float_21 = OpConstant %float 21
%42 = OpConstantComposite %v4float %float_21 %float_21 %float_21 %float_21
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_1 = OpConstant %int 1
%float_4 = OpConstant %float 4
%56 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n7 = OpConstant %float -7
%66 = OpConstantComposite %v4float %float_n7 %float_n7 %float_n7 %float_n7
%float_8 = OpConstant %float 8
%75 = OpConstantComposite %v4float %float_8 %float_8 %float_8 %float_8
%float_9 = OpConstant %float 9
%84 = OpConstantComposite %v4float %float_9 %float_9 %float_9 %float_9
%float_n10 = OpConstant %float -10
%93 = OpConstantComposite %v4float %float_n10 %float_n10 %float_n10 %float_n10
%float_12 = OpConstant %float 12
%100 = OpConstantComposite %v4float %float_12 %float_12 %float_12 %float_12
%main = OpFunction %void None %11
%12 = OpLabel
%f = OpVariable %_ptr_Function_int Function
%60 = OpVariable %_ptr_Function_v4float Function
%70 = OpVariable %_ptr_Function_v4float Function
%79 = OpVariable %_ptr_Function_v4float Function
%88 = OpVariable %_ptr_Function_v4float Function
OpStore %sk_FragColor %14
OpStore %sk_FragColor %16
OpStore %sk_FragColor %18
OpStore %sk_FragColor %20
OpStore %sk_FragColor %22
OpStore %sk_FragColor %24
OpStore %sk_FragColor %26
OpStore %sk_FragColor %28
OpStore %sk_FragColor %30
OpStore %sk_FragColor %32
OpStore %sk_FragColor %34
OpStore %sk_FragColor %36
OpStore %sk_FragColor %38
OpStore %sk_FragColor %40
OpStore %sk_FragColor %42
OpStore %f %int_1
%47 = OpLoad %int %f
%48 = OpIEqual %bool %47 %int_1
OpSelectionMerge %50 None
OpBranchConditional %48 %49 %50
%49 = OpLabel
OpStore %sk_FragColor %14
OpBranch %50
%50 = OpLabel
%51 = OpLoad %int %f
%52 = OpINotEqual %bool %51 %int_1
OpSelectionMerge %54 None
OpBranchConditional %52 %53 %54
%53 = OpLabel
OpStore %sk_FragColor %56
OpBranch %54
%54 = OpLabel
%57 = OpLoad %int %f
%59 = OpIEqual %bool %57 %int_0
OpSelectionMerge %64 None
OpBranchConditional %59 %62 %63
%62 = OpLabel
OpStore %60 %20
OpBranch %64
%63 = OpLabel
OpStore %60 %66
OpBranch %64
%64 = OpLabel
%67 = OpLoad %v4float %60
OpStore %sk_FragColor %67
%68 = OpLoad %int %f
%69 = OpINotEqual %bool %68 %int_0
OpSelectionMerge %73 None
OpBranchConditional %69 %71 %72
%71 = OpLabel
OpStore %70 %75
OpBranch %73
%72 = OpLabel
OpStore %70 %22
OpBranch %73
%73 = OpLabel
%76 = OpLoad %v4float %70
OpStore %sk_FragColor %76
%77 = OpLoad %int %f
%78 = OpIEqual %bool %77 %int_1
OpSelectionMerge %82 None
OpBranchConditional %78 %80 %81
%80 = OpLabel
OpStore %79 %84
OpBranch %82
%81 = OpLabel
OpStore %79 %24
OpBranch %82
%82 = OpLabel
%85 = OpLoad %v4float %79
OpStore %sk_FragColor %85
%86 = OpLoad %int %f
%87 = OpINotEqual %bool %86 %int_1
OpSelectionMerge %91 None
OpBranchConditional %87 %89 %90
%89 = OpLabel
OpStore %88 %26
OpBranch %91
%90 = OpLabel
OpStore %88 %93
OpBranch %91
%91 = OpLabel
%94 = OpLoad %v4float %88
OpStore %sk_FragColor %94
%95 = OpLoad %int %f
OpSelectionMerge %96 None
OpSwitch %95 %96 0 %97 1 %98
%97 = OpLabel
OpStore %sk_FragColor %28
OpBranch %96
%98 = OpLabel
OpStore %sk_FragColor %100
OpBranch %96
%96 = OpLabel
OpReturn
OpFunctionEnd
