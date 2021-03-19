OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %i "i"
OpName %i_0 "i"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %34 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%float_2 = OpConstant %float 2
%float_5 = OpConstant %float 5
%float_0_75 = OpConstant %float 0.75
%21 = OpConstantComposite %v4float %float_0_75 %float_0_75 %float_0_75 %float_0_75
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%float_0_5 = OpConstant %float 0.5
%int_1 = OpConstant %int 1
%float_0_25 = OpConstant %float 0.25
%int_2 = OpConstant %int 2
%int_100 = OpConstant %int 100
%main = OpFunction %void None %11
%12 = OpLabel
%i = OpVariable %_ptr_Function_int Function
%i_0 = OpVariable %_ptr_Function_int Function
%13 = OpExtInst %float %1 Sqrt %float_2
%16 = OpFOrdGreaterThan %bool %13 %float_5
OpSelectionMerge %19 None
OpBranchConditional %16 %17 %18
%17 = OpLabel
OpStore %sk_FragColor %21
OpBranch %19
%18 = OpLabel
OpKill
%19 = OpLabel
OpStore %i %int_0
OpBranch %26
%26 = OpLabel
OpLoopMerge %30 %29 None
OpBranch %27
%27 = OpLabel
%31 = OpLoad %int %i
%33 = OpSLessThan %bool %31 %int_10
OpBranchConditional %33 %28 %30
%28 = OpLabel
%34 = OpLoad %v4float %sk_FragColor
%36 = OpVectorTimesScalar %v4float %34 %float_0_5
OpStore %sk_FragColor %36
%37 = OpLoad %int %i
%39 = OpIAdd %int %37 %int_1
OpStore %i %39
OpBranch %29
%29 = OpLabel
OpBranch %26
%30 = OpLabel
OpBranch %40
%40 = OpLabel
OpLoopMerge %44 %43 None
OpBranch %41
%41 = OpLabel
%45 = OpLoad %v4float %sk_FragColor
%47 = OpCompositeConstruct %v4float %float_0_25 %float_0_25 %float_0_25 %float_0_25
%48 = OpFAdd %v4float %45 %47
OpStore %sk_FragColor %48
OpBranch %42
%42 = OpLabel
%49 = OpLoad %v4float %sk_FragColor
%50 = OpCompositeExtract %float %49 0
%51 = OpFOrdLessThan %bool %50 %float_0_75
OpBranchConditional %51 %43 %44
%43 = OpLabel
OpBranch %40
%44 = OpLabel
OpStore %i_0 %int_0
OpBranch %53
%53 = OpLabel
OpLoopMerge %57 %56 None
OpBranch %54
%54 = OpLabel
%58 = OpLoad %int %i_0
%59 = OpSLessThan %bool %58 %int_10
OpBranchConditional %59 %55 %57
%55 = OpLabel
%60 = OpLoad %int %i_0
%62 = OpSMod %int %60 %int_2
%63 = OpIEqual %bool %62 %int_1
OpSelectionMerge %66 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
OpBranch %57
%65 = OpLabel
%67 = OpLoad %int %i_0
%69 = OpSGreaterThan %bool %67 %int_100
OpSelectionMerge %72 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
OpReturn
%71 = OpLabel
OpBranch %56
%72 = OpLabel
OpBranch %66
%66 = OpLabel
OpBranch %56
%56 = OpLabel
%73 = OpLoad %int %i_0
%74 = OpIAdd %int %73 %int_1
OpStore %i_0 %74
OpBranch %53
%57 = OpLabel
OpReturn
OpFunctionEnd
