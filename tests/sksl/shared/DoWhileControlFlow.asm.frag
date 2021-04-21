OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %x "x"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %x RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%12 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%20 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%26 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Function_float = OpTypePointer Function %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_0_25 = OpConstant %float 0.25
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %12
%13 = OpLabel
%17 = OpVariable %_ptr_Function_v2float Function
OpStore %17 %16
%19 = OpFunctionCall %v4float %main %17
OpStore %sk_FragColor %19
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %20
%21 = OpFunctionParameter %_ptr_Function_v2float
%22 = OpLabel
%x = OpVariable %_ptr_Function_v4float Function
OpStore %x %26
OpBranch %27
%27 = OpLabel
OpLoopMerge %31 %30 None
OpBranch %28
%28 = OpLabel
%32 = OpAccessChain %_ptr_Function_float %x %int_0
%36 = OpLoad %float %32
%38 = OpFSub %float %36 %float_0_25
OpStore %32 %38
%39 = OpLoad %v4float %x
%40 = OpCompositeExtract %float %39 0
%41 = OpFOrdLessThanEqual %bool %40 %float_0
OpSelectionMerge %43 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
OpBranch %31
%43 = OpLabel
OpBranch %29
%29 = OpLabel
OpBranch %30
%30 = OpLabel
%44 = OpLoad %v4float %x
%45 = OpCompositeExtract %float %44 3
%46 = OpFOrdEqual %bool %45 %float_1
OpBranchConditional %46 %27 %31
%31 = OpLabel
OpBranch %47
%47 = OpLabel
OpLoopMerge %51 %50 None
OpBranch %48
%48 = OpLabel
%52 = OpAccessChain %_ptr_Function_float %x %int_2
%54 = OpLoad %float %52
%55 = OpFSub %float %54 %float_0_25
OpStore %52 %55
%56 = OpLoad %v4float %x
%57 = OpCompositeExtract %float %56 3
%58 = OpFOrdEqual %bool %57 %float_1
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
OpBranch %50
%60 = OpLabel
%61 = OpAccessChain %_ptr_Function_float %x %int_1
OpStore %61 %float_0
OpBranch %49
%49 = OpLabel
OpBranch %50
%50 = OpLabel
%63 = OpLoad %v4float %x
%64 = OpCompositeExtract %float %63 2
%65 = OpFOrdGreaterThan %bool %64 %float_0
OpBranchConditional %65 %47 %51
%51 = OpLabel
%66 = OpLoad %v4float %x
OpReturnValue %66
OpFunctionEnd
