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
OpDecorate %25 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
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
%25 = OpLoad %float %r
%26 = OpAccessChain %_ptr_Function_float %x %int_0
OpStore %26 %25
%29 = OpLoad %v4float %x
%30 = OpCompositeExtract %float %29 0
%32 = OpFOrdGreaterThanEqual %bool %30 %float_0
OpSelectionMerge %34 None
OpBranchConditional %32 %33 %34
%33 = OpLabel
OpBranch %21
%34 = OpLabel
OpBranch %20
%20 = OpLabel
%35 = OpLoad %float %r
%36 = OpFAdd %float %35 %float_1
OpStore %r %36
OpBranch %17
%21 = OpLabel
OpStore %b %float_5
OpBranch %38
%38 = OpLabel
OpLoopMerge %42 %41 None
OpBranch %39
%39 = OpLabel
%43 = OpLoad %float %b
%44 = OpFOrdGreaterThanEqual %bool %43 %float_0
OpBranchConditional %44 %40 %42
%40 = OpLabel
%45 = OpLoad %float %b
%46 = OpAccessChain %_ptr_Function_float %x %int_2
OpStore %46 %45
%48 = OpLoad %v4float %x
%49 = OpCompositeExtract %float %48 3
%50 = OpFOrdEqual %bool %49 %float_1
OpSelectionMerge %52 None
OpBranchConditional %50 %51 %52
%51 = OpLabel
OpBranch %41
%52 = OpLabel
%53 = OpAccessChain %_ptr_Function_float %x %int_1
OpStore %53 %float_0
OpBranch %41
%41 = OpLabel
%55 = OpLoad %float %b
%56 = OpFSub %float %55 %float_1
OpStore %b %56
OpBranch %38
%42 = OpLabel
%57 = OpLoad %v4float %x
OpReturnValue %57
OpFunctionEnd

1 error
