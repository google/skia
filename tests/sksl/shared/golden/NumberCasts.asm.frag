OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %BF "BF"
OpName %BI "BI"
OpName %BB "BB"
OpName %FF "FF"
OpName %FI "FI"
OpName %FB "FB"
OpName %IF "IF"
OpName %II "II"
OpName %IB "IB"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %BF RelaxedPrecision
OpDecorate %BI RelaxedPrecision
OpDecorate %BB RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Private_bool = OpTypePointer Private %bool
%BF = OpVariable %_ptr_Private_bool Private
%true = OpConstantTrue %bool
%BI = OpVariable %_ptr_Private_bool Private
%BB = OpVariable %_ptr_Private_bool Private
%_ptr_Private_float = OpTypePointer Private %float
%FF = OpVariable %_ptr_Private_float Private
%float_1_23000002 = OpConstant %float 1.23000002
%FI = OpVariable %_ptr_Private_float Private
%float_1 = OpConstant %float 1
%FB = OpVariable %_ptr_Private_float Private
%int = OpTypeInt 32 1
%_ptr_Private_int = OpTypePointer Private %int
%IF = OpVariable %_ptr_Private_int Private
%int_1 = OpConstant %int 1
%II = OpVariable %_ptr_Private_int Private
%IB = OpVariable %_ptr_Private_int Private
%void = OpTypeVoid
%28 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%_ptr_Output_float = OpTypePointer Output %float
%int_0 = OpConstant %int 0
%main = OpFunction %void None %28
%29 = OpLabel
OpStore %BF %true
OpStore %BI %true
OpStore %BB %true
OpStore %FF %float_1_23000002
OpStore %FI %float_1
OpStore %FB %float_1
OpStore %IF %int_1
OpStore %II %int_1
OpStore %IB %int_1
%31 = OpLoad %bool %BF
%30 = OpSelect %float %31 %float_1 %float_0
%33 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %33 %30
%37 = OpLoad %bool %BI
%36 = OpSelect %float %37 %float_1 %float_0
%38 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %38 %36
%40 = OpLoad %bool %BB
%39 = OpSelect %float %40 %float_1 %float_0
%41 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %41 %39
%42 = OpLoad %float %FF
%43 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %43 %42
%44 = OpLoad %float %FI
%45 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %45 %44
%46 = OpLoad %float %FB
%47 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %47 %46
%49 = OpLoad %int %IF
%48 = OpConvertSToF %float %49
%50 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %50 %48
%52 = OpLoad %int %II
%51 = OpConvertSToF %float %52
%53 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %53 %51
%55 = OpLoad %int %IB
%54 = OpConvertSToF %float %55
%56 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %56 %54
OpReturn
OpFunctionEnd
