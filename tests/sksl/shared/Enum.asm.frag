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
OpDecorate %35 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
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
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_1 = OpConstant %int 1
%float_4 = OpConstant %float 4
%44 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n7 = OpConstant %float -7
%54 = OpConstantComposite %v4float %float_n7 %float_n7 %float_n7 %float_n7
%float_8 = OpConstant %float 8
%63 = OpConstantComposite %v4float %float_8 %float_8 %float_8 %float_8
%float_9 = OpConstant %float 9
%72 = OpConstantComposite %v4float %float_9 %float_9 %float_9 %float_9
%float_n10 = OpConstant %float -10
%81 = OpConstantComposite %v4float %float_n10 %float_n10 %float_n10 %float_n10
%float_12 = OpConstant %float 12
%88 = OpConstantComposite %v4float %float_12 %float_12 %float_12 %float_12
%main = OpFunction %void None %11
%12 = OpLabel
%f = OpVariable %_ptr_Function_int Function
%48 = OpVariable %_ptr_Function_v4float Function
%58 = OpVariable %_ptr_Function_v4float Function
%67 = OpVariable %_ptr_Function_v4float Function
%76 = OpVariable %_ptr_Function_v4float Function
OpStore %sk_FragColor %14
OpStore %sk_FragColor %16
OpStore %sk_FragColor %18
OpStore %sk_FragColor %20
OpStore %sk_FragColor %22
OpStore %sk_FragColor %24
OpStore %sk_FragColor %26
OpStore %sk_FragColor %28
OpStore %sk_FragColor %30
OpStore %f %int_1
%35 = OpLoad %int %f
%36 = OpIEqual %bool %35 %int_1
OpSelectionMerge %38 None
OpBranchConditional %36 %37 %38
%37 = OpLabel
OpStore %sk_FragColor %14
OpBranch %38
%38 = OpLabel
%39 = OpLoad %int %f
%40 = OpINotEqual %bool %39 %int_1
OpSelectionMerge %42 None
OpBranchConditional %40 %41 %42
%41 = OpLabel
OpStore %sk_FragColor %44
OpBranch %42
%42 = OpLabel
%45 = OpLoad %int %f
%47 = OpIEqual %bool %45 %int_0
OpSelectionMerge %52 None
OpBranchConditional %47 %50 %51
%50 = OpLabel
OpStore %48 %20
OpBranch %52
%51 = OpLabel
OpStore %48 %54
OpBranch %52
%52 = OpLabel
%55 = OpLoad %v4float %48
OpStore %sk_FragColor %55
%56 = OpLoad %int %f
%57 = OpINotEqual %bool %56 %int_0
OpSelectionMerge %61 None
OpBranchConditional %57 %59 %60
%59 = OpLabel
OpStore %58 %63
OpBranch %61
%60 = OpLabel
OpStore %58 %22
OpBranch %61
%61 = OpLabel
%64 = OpLoad %v4float %58
OpStore %sk_FragColor %64
%65 = OpLoad %int %f
%66 = OpIEqual %bool %65 %int_1
OpSelectionMerge %70 None
OpBranchConditional %66 %68 %69
%68 = OpLabel
OpStore %67 %72
OpBranch %70
%69 = OpLabel
OpStore %67 %24
OpBranch %70
%70 = OpLabel
%73 = OpLoad %v4float %67
OpStore %sk_FragColor %73
%74 = OpLoad %int %f
%75 = OpINotEqual %bool %74 %int_1
OpSelectionMerge %79 None
OpBranchConditional %75 %77 %78
%77 = OpLabel
OpStore %76 %26
OpBranch %79
%78 = OpLabel
OpStore %76 %81
OpBranch %79
%79 = OpLabel
%82 = OpLoad %v4float %76
OpStore %sk_FragColor %82
%83 = OpLoad %int %f
OpSelectionMerge %84 None
OpSwitch %83 %84 0 %85 1 %86
%85 = OpLabel
OpStore %sk_FragColor %28
OpBranch %84
%86 = OpLabel
OpStore %sk_FragColor %88
OpBranch %84
%84 = OpLabel
OpReturn
OpFunctionEnd
