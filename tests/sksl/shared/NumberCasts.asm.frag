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
OpDecorate %30 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
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
%30 = OpLoad %bool %BF
%31 = OpSelect %float %30 %float_1 %float_0
%33 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %33 %31
%36 = OpLoad %bool %BI
%37 = OpSelect %float %36 %float_1 %float_0
%38 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %38 %37
%39 = OpLoad %bool %BB
%40 = OpSelect %float %39 %float_1 %float_0
%41 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %41 %40
%42 = OpLoad %float %FF
%43 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %43 %42
%44 = OpLoad %float %FI
%45 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %45 %44
%46 = OpLoad %float %FB
%47 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %47 %46
%48 = OpLoad %int %IF
%49 = OpConvertSToF %float %48
%50 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %50 %49
%51 = OpLoad %int %II
%52 = OpConvertSToF %float %51
%53 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %53 %52
%54 = OpLoad %int %IB
%55 = OpConvertSToF %float %54
%56 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %56 %55
OpReturn
OpFunctionEnd
