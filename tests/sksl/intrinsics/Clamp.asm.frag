OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %a %b %c %d %e %f
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %a "a"
OpName %b "b"
OpName %c "c"
OpName %d "d"
OpName %e "e"
OpName %f "f"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %a RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %c RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Input_float = OpTypePointer Input %float
%a = OpVariable %_ptr_Input_float Input
%b = OpVariable %_ptr_Input_float Input
%c = OpVariable %_ptr_Input_float Input
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%d = OpVariable %_ptr_Input_int Input
%e = OpVariable %_ptr_Input_int Input
%f = OpVariable %_ptr_Input_int Input
%void = OpTypeVoid
%20 = OpTypeFunction %void
%_ptr_Output_float = OpTypePointer Output %float
%int_0 = OpConstant %int 0
%main = OpFunction %void None %20
%21 = OpLabel
%23 = OpLoad %float %a
%24 = OpLoad %float %b
%25 = OpLoad %float %c
%22 = OpExtInst %float %1 FClamp %23 %24 %25
%26 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %26 %22
%30 = OpLoad %int %d
%31 = OpLoad %int %e
%32 = OpLoad %int %f
%29 = OpExtInst %int %1 SClamp %30 %31 %32
%33 = OpConvertSToF %float %29
%34 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %34 %33
OpReturn
OpFunctionEnd
