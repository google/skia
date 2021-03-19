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
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %26 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_0_25 = OpConstant %float 0.25
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
%27 = OpCompositeExtract %float %26 3
%28 = OpFOrdEqual %bool %27 %float_1
OpBranchConditional %28 %23 %25
%23 = OpLabel
%29 = OpAccessChain %_ptr_Function_float %x %int_0
%33 = OpLoad %float %29
%35 = OpFSub %float %33 %float_0_25
OpStore %29 %35
%36 = OpLoad %v4float %x
%37 = OpCompositeExtract %float %36 0
%39 = OpFOrdLessThanEqual %bool %37 %float_0
OpSelectionMerge %41 None
OpBranchConditional %39 %40 %41
%40 = OpLabel
OpBranch %25
%41 = OpLabel
OpBranch %24
%24 = OpLabel
OpBranch %21
%25 = OpLabel
OpBranch %42
%42 = OpLabel
OpLoopMerge %46 %45 None
OpBranch %43
%43 = OpLabel
%47 = OpLoad %v4float %x
%48 = OpCompositeExtract %float %47 2
%49 = OpFOrdGreaterThan %bool %48 %float_0
OpBranchConditional %49 %44 %46
%44 = OpLabel
%50 = OpAccessChain %_ptr_Function_float %x %int_2
%52 = OpLoad %float %50
%53 = OpFSub %float %52 %float_0_25
OpStore %50 %53
%54 = OpLoad %v4float %x
%55 = OpCompositeExtract %float %54 3
%56 = OpFOrdEqual %bool %55 %float_1
OpSelectionMerge %58 None
OpBranchConditional %56 %57 %58
%57 = OpLabel
OpBranch %45
%58 = OpLabel
%59 = OpAccessChain %_ptr_Function_float %x %int_1
OpStore %59 %float_0
OpBranch %45
%45 = OpLabel
OpBranch %42
%46 = OpLabel
%61 = OpLoad %v4float %x
OpReturnValue %61
OpFunctionEnd
