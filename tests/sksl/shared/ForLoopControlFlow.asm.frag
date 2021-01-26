### Compilation failed:

error: SPIR-V validation error: OpEntryPoint Entry Point <id> '2[%main]'s function return type is not void.
  OpEntryPoint Fragment %main "main" %sk_Clockwise

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %x "x"
OpName %r "r"
OpName %b "b"
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %22 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%8 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%13 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Function_float = OpTypePointer Function %float
%float_n5 = OpConstant %float -5
%float_5 = OpConstant %float 5
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_0 = OpConstant %float 0
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%main = OpFunction %v4float None %8
%9 = OpLabel
%x = OpVariable %_ptr_Function_v4float Function
%r = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_float Function
OpStore %x %13
OpStore %r %float_n5
OpBranch %17
%17 = OpLabel
OpLoopMerge %21 %20 None
OpBranch %18
%18 = OpLabel
%22 = OpLoad %float %r
%24 = OpFOrdLessThan %bool %22 %float_5
OpBranchConditional %24 %19 %21
%19 = OpLabel
%26 = OpLoad %float %r
%25 = OpExtInst %float %1 FAbs %26
%27 = OpAccessChain %_ptr_Function_float %x %int_0
OpStore %27 %25
%30 = OpLoad %v4float %x
%31 = OpCompositeExtract %float %30 0
%33 = OpFOrdEqual %bool %31 %float_0
OpSelectionMerge %35 None
OpBranchConditional %33 %34 %35
%34 = OpLabel
OpBranch %21
%35 = OpLabel
OpBranch %20
%20 = OpLabel
%36 = OpLoad %float %r
%37 = OpFAdd %float %36 %float_1
OpStore %r %37
OpBranch %17
%21 = OpLabel
OpStore %b %float_5
OpBranch %39
%39 = OpLabel
OpLoopMerge %43 %42 None
OpBranch %40
%40 = OpLabel
%44 = OpLoad %float %b
%45 = OpFOrdGreaterThanEqual %bool %44 %float_0
OpBranchConditional %45 %41 %43
%41 = OpLabel
%46 = OpLoad %float %b
%47 = OpAccessChain %_ptr_Function_float %x %int_2
OpStore %47 %46
%49 = OpLoad %v4float %x
%50 = OpCompositeExtract %float %49 3
%51 = OpFOrdEqual %bool %50 %float_1
OpSelectionMerge %53 None
OpBranchConditional %51 %52 %53
%52 = OpLabel
OpBranch %42
%53 = OpLabel
%54 = OpAccessChain %_ptr_Function_float %x %int_1
OpStore %54 %float_0
OpBranch %42
%42 = OpLabel
%56 = OpLoad %float %b
%57 = OpFSub %float %56 %float_1
OpStore %b %57
OpBranch %39
%43 = OpLabel
%58 = OpLoad %v4float %x
OpReturnValue %58
OpFunctionEnd

1 error
