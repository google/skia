OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %i1 "i1"
OpName %i2 "i2"
OpName %i3 "i3"
OpName %i4 "i4"
OpName %i5 "i5"
OpName %u1 "u1"
OpName %u2 "u2"
OpName %u3 "u3"
OpName %u4 "u4"
OpName %u5 "u5"
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %53 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%7 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_305441741 = OpConstant %int 305441741
%int_2147483647 = OpConstant %int 2147483647
%int_n1 = OpConstant %int -1
%int_n48879 = OpConstant %int -48879
%uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%uint_305441741 = OpConstant %uint 305441741
%uint_2147483647 = OpConstant %uint 2147483647
%uint_4294967295 = OpConstant %uint 4294967295
%uint_65535 = OpConstant %uint 65535
%uint_1_0 = OpConstant %uint 1
%main = OpFunction %void None %7
%8 = OpLabel
%i1 = OpVariable %_ptr_Function_int Function
%i2 = OpVariable %_ptr_Function_int Function
%i3 = OpVariable %_ptr_Function_int Function
%i4 = OpVariable %_ptr_Function_int Function
%i5 = OpVariable %_ptr_Function_int Function
%u1 = OpVariable %_ptr_Function_uint Function
%u2 = OpVariable %_ptr_Function_uint Function
%u3 = OpVariable %_ptr_Function_uint Function
%u4 = OpVariable %_ptr_Function_uint Function
%u5 = OpVariable %_ptr_Function_uint Function
OpStore %i1 %int_0
%13 = OpLoad %int %i1
%15 = OpIAdd %int %13 %int_1
OpStore %i1 %15
OpStore %i2 %int_305441741
%18 = OpLoad %int %i2
%19 = OpIAdd %int %18 %int_1
OpStore %i2 %19
OpStore %i3 %int_2147483647
%22 = OpLoad %int %i3
%23 = OpIAdd %int %22 %int_1
OpStore %i3 %23
OpStore %i4 %int_n1
%26 = OpLoad %int %i4
%27 = OpIAdd %int %26 %int_1
OpStore %i4 %27
OpStore %i5 %int_n48879
%30 = OpLoad %int %i5
%31 = OpIAdd %int %30 %int_1
OpStore %i5 %31
OpStore %u1 %uint_0
%36 = OpLoad %uint %u1
%38 = OpIAdd %uint %36 %uint_1
OpStore %u1 %38
OpStore %u2 %uint_305441741
%41 = OpLoad %uint %u2
%42 = OpIAdd %uint %41 %uint_1
OpStore %u2 %42
OpStore %u3 %uint_2147483647
%45 = OpLoad %uint %u3
%46 = OpIAdd %uint %45 %uint_1
OpStore %u3 %46
OpStore %u4 %uint_4294967295
%49 = OpLoad %uint %u4
%50 = OpIAdd %uint %49 %uint_1
OpStore %u4 %50
OpStore %u5 %uint_65535
%53 = OpLoad %uint %u5
%55 = OpIAdd %uint %53 %uint_1_0
OpStore %u5 %55
OpReturn
OpFunctionEnd
