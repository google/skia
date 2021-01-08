OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %expr1 "expr1"
OpName %expr2 "expr2"
OpDecorate %sk_FragCoord BuiltIn FragCoord
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %25 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%13 = OpTypeFunction %void
%_ptr_Function_bool = OpTypePointer Function %bool
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%_ptr_Output_float = OpTypePointer Output %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%true = OpConstantTrue %bool
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%main = OpFunction %void None %13
%14 = OpLabel
%expr1 = OpVariable %_ptr_Function_bool Function
%expr2 = OpVariable %_ptr_Function_bool Function
%17 = OpLoad %v4float %sk_FragCoord
%18 = OpCompositeExtract %float %17 0
%20 = OpFOrdGreaterThan %bool %18 %float_0
OpStore %expr1 %20
%22 = OpLoad %v4float %sk_FragCoord
%23 = OpCompositeExtract %float %22 1
%24 = OpFOrdGreaterThan %bool %23 %float_0
OpStore %expr2 %24
%25 = OpLoad %bool %expr1
OpSelectionMerge %28 None
OpBranchConditional %25 %26 %27
%26 = OpLabel
%30 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %30 %float_1
OpBranch %28
%27 = OpLabel
%35 = OpLoad %bool %expr1
%36 = OpLogicalNotEqual %bool %true %35
OpSelectionMerge %39 None
OpBranchConditional %36 %37 %38
%37 = OpLabel
%41 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %41 %float_3
OpBranch %39
%38 = OpLabel
%42 = OpLoad %bool %expr2
OpSelectionMerge %45 None
OpBranchConditional %42 %43 %44
%43 = OpLabel
%47 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %47 %float_4
OpBranch %45
%44 = OpLabel
%48 = OpLoad %bool %expr2
OpSelectionMerge %51 None
OpBranchConditional %48 %49 %50
%49 = OpLabel
%53 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %53 %float_5
OpBranch %51
%50 = OpLabel
%55 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %55 %float_6
OpBranch %51
%51 = OpLabel
OpBranch %45
%45 = OpLabel
OpBranch %39
%39 = OpLabel
OpBranch %28
%28 = OpLabel
%56 = OpLoad %bool %expr1
OpSelectionMerge %59 None
OpBranchConditional %56 %57 %58
%57 = OpLabel
%60 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %60 %float_1
OpBranch %59
%58 = OpLabel
%61 = OpLoad %bool %expr1
%62 = OpLogicalNotEqual %bool %61 %true
OpSelectionMerge %65 None
OpBranchConditional %62 %63 %64
%63 = OpLabel
%66 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %66 %float_3
OpBranch %65
%64 = OpLabel
%67 = OpLoad %bool %expr2
OpSelectionMerge %70 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%71 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %71 %float_4
OpBranch %70
%69 = OpLabel
%72 = OpLoad %bool %expr2
OpSelectionMerge %75 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%76 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %76 %float_5
OpBranch %75
%74 = OpLabel
%77 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %77 %float_6
OpBranch %75
%75 = OpLabel
OpBranch %70
%70 = OpLabel
OpBranch %65
%65 = OpLabel
OpBranch %59
%59 = OpLabel
OpReturn
OpFunctionEnd
