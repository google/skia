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
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
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
%float_12 = OpConstant %float 12
%int_10 = OpConstant %int 10
%int_0 = OpConstant %int 0
%int_n1 = OpConstant %int -1
%int_2 = OpConstant %int 2
%int_4 = OpConstant %int 4
%int_5 = OpConstant %int 5
%v2float = OpTypeVector %float 2
%int_6 = OpConstant %int 6
%main = OpFunction %void None %7
%8 = OpLabel
%x = OpVariable %_ptr_Function_float Function
%y = OpVariable %_ptr_Function_float Function
%z = OpVariable %_ptr_Function_int Function
OpStore %x %float_1
OpStore %y %float_2
OpStore %z %int_3
OpStore %x %float_n6
OpStore %y %float_n1
OpStore %z %int_8
%22 = OpLoad %float %x
%24 = OpFAdd %float %22 %float_12
OpStore %x %24
%25 = OpLoad %float %x
%26 = OpFSub %float %25 %float_12
OpStore %x %26
%27 = OpLoad %float %x
%28 = OpLoad %float %y
OpStore %z %int_10
%29 = OpConvertSToF %float %int_10
%31 = OpFDiv %float %28 %29
OpStore %y %31
%32 = OpFMul %float %27 %31
OpStore %x %32
%33 = OpLoad %int %z
%35 = OpBitwiseOr %int %33 %int_0
OpStore %z %35
%36 = OpLoad %int %z
%38 = OpBitwiseAnd %int %36 %int_n1
OpStore %z %38
%39 = OpLoad %int %z
%40 = OpBitwiseXor %int %39 %int_0
OpStore %z %40
%41 = OpLoad %int %z
%43 = OpShiftRightArithmetic %int %41 %int_2
OpStore %z %43
%44 = OpLoad %int %z
%46 = OpShiftLeftLogical %int %44 %int_4
OpStore %z %46
%47 = OpLoad %int %z
%49 = OpSMod %int %47 %int_5
OpStore %z %49
%51 = OpExtInst %float %1 Sqrt %float_1
%52 = OpCompositeConstruct %v2float %51 %51
%55 = OpCompositeConstruct %v2float %int_6 %int_6
%50 = OpConvertSToF %float %55
OpStore %x %50
%56 = OpExtInst %float %1 Sqrt %float_1
%57 = OpCompositeConstruct %v2float %56 %56
%58 = OpCompositeConstruct %v2float %int_6 %int_6
OpStore %z %58
OpReturn
OpFunctionEnd
