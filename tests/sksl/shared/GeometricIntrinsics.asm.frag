OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %_1_x "_1_x"
OpName %x "x"
OpName %_3_x "_3_x"
OpName %y "y"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%29 = OpConstantComposite %v2float %float_1 %float_2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%36 = OpConstantComposite %v2float %float_3 %float_4
%main = OpFunction %void None %11
%12 = OpLabel
%_1_x = OpVariable %_ptr_Function_float Function
%x = OpVariable %_ptr_Function_float Function
%_3_x = OpVariable %_ptr_Function_v2float Function
%y = OpVariable %_ptr_Function_v2float Function
OpStore %_1_x %float_1
%16 = OpExtInst %float %1 Length %float_1
OpStore %_1_x %16
%18 = OpLoad %float %_1_x
%17 = OpExtInst %float %1 Distance %18 %float_2
OpStore %_1_x %17
%21 = OpLoad %float %_1_x
%20 = OpFMul %float %21 %float_2
OpStore %_1_x %20
%23 = OpLoad %float %_1_x
%22 = OpExtInst %float %1 Normalize %23
OpStore %_1_x %22
%25 = OpLoad %float %_1_x
OpStore %x %25
OpStore %_3_x %29
%30 = OpExtInst %float %1 Length %29
%31 = OpCompositeConstruct %v2float %30 %30
OpStore %_3_x %31
%33 = OpLoad %v2float %_3_x
%32 = OpExtInst %float %1 Distance %33 %36
%37 = OpCompositeConstruct %v2float %32 %32
OpStore %_3_x %37
%39 = OpLoad %v2float %_3_x
%38 = OpDot %float %39 %36
%40 = OpCompositeConstruct %v2float %38 %38
OpStore %_3_x %40
%42 = OpLoad %v2float %_3_x
%41 = OpExtInst %v2float %1 Normalize %42
OpStore %_3_x %41
%44 = OpLoad %v2float %_3_x
OpStore %y %44
%45 = OpLoad %float %x
%46 = OpLoad %v2float %y
%47 = OpCompositeExtract %float %46 0
%48 = OpCompositeExtract %float %46 1
%49 = OpCompositeConstruct %v4float %45 %47 %48 %float_1
OpStore %sk_FragColor %49
OpReturn
OpFunctionEnd
