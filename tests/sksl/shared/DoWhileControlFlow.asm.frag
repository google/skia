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
OpDecorate %30 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
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
%26 = OpAccessChain %_ptr_Function_float %x %int_0
%30 = OpLoad %float %26
%32 = OpFSub %float %30 %float_0_25
OpStore %26 %32
%33 = OpLoad %v4float %x
%34 = OpCompositeExtract %float %33 0
%36 = OpFOrdLessThanEqual %bool %34 %float_0
OpSelectionMerge %38 None
OpBranchConditional %36 %37 %38
%37 = OpLabel
OpBranch %25
%38 = OpLabel
OpBranch %23
%23 = OpLabel
%39 = OpLoad %v4float %x
%40 = OpCompositeExtract %float %39 3
%41 = OpFOrdEqual %bool %40 %float_1
OpBranchConditional %41 %24 %25
%24 = OpLabel
OpBranch %21
%25 = OpLabel
OpBranch %42
%42 = OpLabel
OpLoopMerge %46 %45 None
OpBranch %43
%43 = OpLabel
%47 = OpAccessChain %_ptr_Function_float %x %int_2
%49 = OpLoad %float %47
%50 = OpFSub %float %49 %float_0_25
OpStore %47 %50
%51 = OpLoad %v4float %x
%52 = OpCompositeExtract %float %51 3
%53 = OpFOrdEqual %bool %52 %float_1
OpSelectionMerge %55 None
OpBranchConditional %53 %54 %55
%54 = OpLabel
OpBranch %45
%55 = OpLabel
%56 = OpAccessChain %_ptr_Function_float %x %int_1
OpStore %56 %float_0
OpBranch %44
%44 = OpLabel
%58 = OpLoad %v4float %x
%59 = OpCompositeExtract %float %58 2
%60 = OpFOrdGreaterThan %bool %59 %float_0
OpBranchConditional %60 %45 %46
%45 = OpLabel
OpBranch %42
%46 = OpLabel
%61 = OpLoad %v4float %x
OpReturnValue %61
OpFunctionEnd
