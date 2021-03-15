OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %array "array"
OpName %x "x"
OpName %y "y"
OpName %z "z"
OpName %w "w"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %_arr_float_int_4 ArrayStride 16
OpDecorate %34 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%int = OpTypeInt 32 1
%int_4 = OpConstant %int 4
%_arr_float_int_4 = OpTypeArray %float %int_4
%_ptr_Function__arr_float_int_4 = OpTypePointer Function %_arr_float_int_4
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%uint_1 = OpConstant %uint 1
%int_2 = OpConstant %int 2
%uint_3 = OpConstant %uint 3
%_ptr_Function_float = OpTypePointer Function %float
%main = OpFunction %void None %11
%12 = OpLabel
%array = OpVariable %_ptr_Function__arr_float_int_4 Function
%x = OpVariable %_ptr_Function_int Function
%y = OpVariable %_ptr_Function_uint Function
%z = OpVariable %_ptr_Function_int Function
%w = OpVariable %_ptr_Function_uint Function
%22 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %array %22
OpStore %x %int_0
OpStore %y %uint_1
OpStore %z %int_2
OpStore %w %uint_3
%34 = OpLoad %int %x
%35 = OpAccessChain %_ptr_Function_float %array %34
%37 = OpLoad %float %35
%38 = OpLoad %uint %y
%39 = OpAccessChain %_ptr_Function_float %array %38
%40 = OpLoad %float %39
%41 = OpLoad %int %z
%42 = OpAccessChain %_ptr_Function_float %array %41
%43 = OpLoad %float %42
%44 = OpLoad %uint %w
%45 = OpAccessChain %_ptr_Function_float %array %44
%46 = OpLoad %float %45
%47 = OpCompositeConstruct %v4float %37 %40 %43 %46
OpStore %sk_FragColor %47
OpReturn
OpFunctionEnd
