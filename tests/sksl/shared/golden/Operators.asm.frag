### Compilation failed:

error: 1: SPIR-V validation error: Expected Constituents to be scalars or vectors of the same type as Result Type components
  %65 = OpCompositeConstruct %v2float %int_6 %int_6

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %x "x"
OpName %y "y"
OpName %z "z"
OpName %b "b"
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %66 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_3 = OpConstant %int 3
%float_n6 = OpConstant %float -6
%float_n1 = OpConstant %float -1
%int_8 = OpConstant %int 8
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%float_12 = OpConstant %float 12
%int_10 = OpConstant %int 10
%int_0 = OpConstant %int 0
%int_n1 = OpConstant %int -1
%int_2 = OpConstant %int 2
%int_4 = OpConstant %int 4
%int_5 = OpConstant %int 5
%v2float = OpTypeVector %float 2
%int_6 = OpConstant %int 6
%float_0 = OpConstant %float 0
%float_6 = OpConstant %float 6
%main = OpFunction %void None %7
%8 = OpLabel
%x = OpVariable %_ptr_Function_float Function
%y = OpVariable %_ptr_Function_float Function
%z = OpVariable %_ptr_Function_int Function
%b = OpVariable %_ptr_Function_bool Function
OpStore %x %float_1
OpStore %y %float_2
OpStore %z %int_3
OpStore %x %float_n6
OpStore %y %float_n1
OpStore %z %int_8
%26 = OpLogicalEqual %bool %false %true
OpSelectionMerge %28 None
OpBranchConditional %26 %28 %27
%27 = OpLabel
%29 = OpExtInst %float %1 Sqrt %float_2
%30 = OpFOrdGreaterThanEqual %bool %float_2 %29
OpBranch %28
%28 = OpLabel
%31 = OpPhi %bool %true %8 %30 %27
OpStore %b %31
%32 = OpLoad %float %x
%34 = OpFAdd %float %32 %float_12
OpStore %x %34
%35 = OpLoad %float %x
%36 = OpFSub %float %35 %float_12
OpStore %x %36
%37 = OpLoad %float %x
%38 = OpLoad %float %y
OpStore %z %int_10
%39 = OpConvertSToF %float %int_10
%41 = OpFDiv %float %38 %39
OpStore %y %41
%42 = OpFMul %float %37 %41
OpStore %x %42
%43 = OpLoad %int %z
%45 = OpBitwiseOr %int %43 %int_0
OpStore %z %45
%46 = OpLoad %int %z
%48 = OpBitwiseAnd %int %46 %int_n1
OpStore %z %48
%49 = OpLoad %int %z
%50 = OpBitwiseXor %int %49 %int_0
OpStore %z %50
%51 = OpLoad %int %z
%53 = OpShiftRightArithmetic %int %51 %int_2
OpStore %z %53
%54 = OpLoad %int %z
%56 = OpShiftLeftLogical %int %54 %int_4
OpStore %z %56
%57 = OpLoad %int %z
%59 = OpSMod %int %57 %int_5
OpStore %z %59
%61 = OpExtInst %float %1 Sqrt %float_1
%62 = OpCompositeConstruct %v2float %61 %61
%65 = OpCompositeConstruct %v2float %int_6 %int_6
%60 = OpConvertSToF %float %65
OpStore %x %60
%66 = OpLoad %bool %b
%67 = OpSelect %float %66 %float_1 %float_0
OpStore %y %float_6
%70 = OpExtInst %float %1 Sqrt %float_1
%71 = OpCompositeConstruct %v2float %70 %70
%72 = OpCompositeConstruct %v2float %int_6 %int_6
OpStore %z %72
OpReturn
OpFunctionEnd

1 error
