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
OpDecorate %41 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
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
%34 = OpLogicalNot %bool %35
OpSelectionMerge %38 None
OpBranchConditional %34 %36 %37
%36 = OpLabel
%40 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %40 %float_3
OpBranch %38
%37 = OpLabel
%41 = OpLoad %bool %expr2
OpSelectionMerge %44 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
%46 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %46 %float_4
OpBranch %44
%43 = OpLabel
%47 = OpLoad %bool %expr2
OpSelectionMerge %50 None
OpBranchConditional %47 %48 %49
%48 = OpLabel
%52 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %52 %float_5
OpBranch %50
%49 = OpLabel
%54 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %54 %float_6
OpBranch %50
%50 = OpLabel
OpBranch %44
%44 = OpLabel
OpBranch %38
%38 = OpLabel
OpBranch %28
%28 = OpLabel
%55 = OpLoad %bool %expr1
OpSelectionMerge %58 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
%59 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %59 %float_1
OpBranch %58
%57 = OpLabel
%61 = OpLoad %bool %expr1
%60 = OpLogicalNot %bool %61
OpSelectionMerge %64 None
OpBranchConditional %60 %62 %63
%62 = OpLabel
%65 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %65 %float_3
OpBranch %64
%63 = OpLabel
%66 = OpLoad %bool %expr2
OpSelectionMerge %69 None
OpBranchConditional %66 %67 %68
%67 = OpLabel
%70 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %70 %float_4
OpBranch %69
%68 = OpLabel
%71 = OpLoad %bool %expr2
OpSelectionMerge %74 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%75 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %75 %float_5
OpBranch %74
%73 = OpLabel
%76 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %76 %float_6
OpBranch %74
%74 = OpLabel
OpBranch %69
%69 = OpLabel
OpBranch %64
%64 = OpLabel
OpBranch %58
%58 = OpLabel
OpReturn
OpFunctionEnd
