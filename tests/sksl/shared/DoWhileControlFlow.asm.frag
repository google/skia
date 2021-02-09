OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %x "x"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %26 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%12 = OpTypeFunction %void
%15 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%20 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%float_0_25 = OpConstant %float 0.25
%_ptr_Function_float = OpTypePointer Function %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_0 = OpConstant %float 0
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%_entrypoint = OpFunction %void None %12
%13 = OpLabel
%14 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %14
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %15
%16 = OpLabel
%x = OpVariable %_ptr_Function_v4float Function
OpStore %x %20
OpBranch %21
%21 = OpLabel
OpLoopMerge %25 %24 None
OpBranch %22
%22 = OpLabel
%26 = OpLoad %v4float %x
%27 = OpCompositeExtract %float %26 0
%29 = OpFSub %float %27 %float_0_25
%30 = OpAccessChain %_ptr_Function_float %x %int_0
OpStore %30 %29
%34 = OpLoad %v4float %x
%35 = OpCompositeExtract %float %34 0
%37 = OpFOrdLessThanEqual %bool %35 %float_0
OpSelectionMerge %39 None
OpBranchConditional %37 %38 %39
%38 = OpLabel
OpBranch %25
%39 = OpLabel
OpBranch %23
%23 = OpLabel
%40 = OpLoad %v4float %x
%41 = OpCompositeExtract %float %40 3
%42 = OpFOrdEqual %bool %41 %float_1
OpBranchConditional %42 %24 %25
%24 = OpLabel
OpBranch %21
%25 = OpLabel
OpBranch %43
%43 = OpLabel
OpLoopMerge %47 %46 None
OpBranch %44
%44 = OpLabel
%48 = OpLoad %v4float %x
%49 = OpCompositeExtract %float %48 2
%50 = OpFSub %float %49 %float_0_25
%51 = OpAccessChain %_ptr_Function_float %x %int_2
OpStore %51 %50
%53 = OpLoad %v4float %x
%54 = OpCompositeExtract %float %53 3
%55 = OpFOrdEqual %bool %54 %float_1
OpSelectionMerge %57 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
OpBranch %46
%57 = OpLabel
%58 = OpAccessChain %_ptr_Function_float %x %int_1
OpStore %58 %float_0
OpBranch %45
%45 = OpLabel
%60 = OpLoad %v4float %x
%61 = OpCompositeExtract %float %60 2
%62 = OpFOrdGreaterThan %bool %61 %float_0
OpBranchConditional %62 %46 %47
%46 = OpLabel
OpBranch %43
%47 = OpLabel
%63 = OpLoad %v4float %x
OpReturnValue %63
OpFunctionEnd
