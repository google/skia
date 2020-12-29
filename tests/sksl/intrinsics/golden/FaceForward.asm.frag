### Compilation failed:

error: SPIR-V validation error: The following forward referenced IDs have not been defined:
2[%2]
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
OpDecorate %d RelaxedPrecision
OpDecorate %e RelaxedPrecision
OpDecorate %f RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
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
%_ptr_Input_v4float = OpTypePointer Input %v4float
%d = OpVariable %_ptr_Input_v4float Input
%e = OpVariable %_ptr_Input_v4float Input
%f = OpVariable %_ptr_Input_v4float Input
%void = OpTypeVoid
%20 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Output_float = OpTypePointer Output %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%main = OpFunction %void None %20
%21 = OpLabel
%23 = OpVariable %_ptr_Function_float Function
%26 = OpVariable %_ptr_Function_float Function
%28 = OpVariable %_ptr_Function_float Function
%22 = OpLoad %float %a
OpStore %23 %22
%25 = OpLoad %float %b
OpStore %26 %25
%27 = OpLoad %float %c
OpStore %28 %27
%29 = OpFunctionCall %float %2 %23 %26 %28
%30 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %30 %29
%35 = OpLoad %v4float %d
%36 = OpLoad %v4float %e
%37 = OpLoad %v4float %f
%34 = OpExtInst %v4float %1 FaceForward %35 %36 %37
OpStore %sk_FragColor %34
OpReturn
OpFunctionEnd

1 error
