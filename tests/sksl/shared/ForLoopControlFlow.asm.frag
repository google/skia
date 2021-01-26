### Compilation failed:

error: SPIR-V validation error: OpEntryPoint Entry Point <id> '2[%main]'s function return type is not void.
  OpEntryPoint Fragment %main "main" %sk_Clockwise

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %colorWhite "colorWhite"
OpName %main "main"
OpName %x "x"
OpName %r "r"
OpName %b "b"
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %colorWhite RelaxedPrecision
OpDecorate %colorWhite DescriptorSet 0
OpDecorate %14 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%colorWhite = OpVariable %_ptr_Uniform_v4float Uniform
%10 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_float = OpTypePointer Function %float
%float_n5 = OpConstant %float -5
%float_5 = OpConstant %float 5
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%main = OpFunction %v4float None %10
%11 = OpLabel
%x = OpVariable %_ptr_Function_v4float Function
%r = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_float Function
%14 = OpLoad %v4float %colorWhite
OpStore %x %14
OpStore %r %float_n5
OpBranch %18
%18 = OpLabel
OpLoopMerge %22 %21 None
OpBranch %19
%19 = OpLabel
%23 = OpLoad %float %r
%25 = OpFOrdLessThan %bool %23 %float_5
OpBranchConditional %25 %20 %22
%20 = OpLabel
%27 = OpLoad %float %r
%26 = OpExtInst %float %1 FAbs %27
%28 = OpAccessChain %_ptr_Function_float %x %int_0
OpStore %28 %26
%31 = OpLoad %v4float %x
%32 = OpCompositeExtract %float %31 0
%34 = OpFOrdEqual %bool %32 %float_0
OpSelectionMerge %36 None
OpBranchConditional %34 %35 %36
%35 = OpLabel
OpBranch %22
%36 = OpLabel
OpBranch %21
%21 = OpLabel
%37 = OpLoad %float %r
%39 = OpFAdd %float %37 %float_1
OpStore %r %39
OpBranch %18
%22 = OpLabel
OpStore %b %float_5
OpBranch %41
%41 = OpLabel
OpLoopMerge %45 %44 None
OpBranch %42
%42 = OpLabel
%46 = OpLoad %float %b
%47 = OpFOrdGreaterThanEqual %bool %46 %float_0
OpBranchConditional %47 %43 %45
%43 = OpLabel
%48 = OpLoad %float %b
%49 = OpAccessChain %_ptr_Function_float %x %int_2
OpStore %49 %48
%51 = OpLoad %v4float %x
%52 = OpCompositeExtract %float %51 3
%53 = OpFOrdEqual %bool %52 %float_1
OpSelectionMerge %55 None
OpBranchConditional %53 %54 %55
%54 = OpLabel
OpBranch %44
%55 = OpLabel
%56 = OpAccessChain %_ptr_Function_float %x %int_1
OpStore %56 %float_0
OpBranch %44
%44 = OpLabel
%58 = OpLoad %float %b
%59 = OpFSub %float %58 %float_1
OpStore %b %59
OpBranch %41
%45 = OpLabel
%60 = OpLoad %v4float %x
OpReturnValue %60
OpFunctionEnd

1 error
