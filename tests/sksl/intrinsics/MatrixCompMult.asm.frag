OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %a "a"
OpName %b "b"
OpName %c "c"
OpName %d "d"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %a RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Private_mat3v3float = OpTypePointer Private %mat3v3float
%a = OpVariable %_ptr_Private_mat3v3float Private
%b = OpVariable %_ptr_Private_mat3v3float Private
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Private_mat4v4float = OpTypePointer Private %mat4v4float
%c = OpVariable %_ptr_Private_mat4v4float Private
%d = OpVariable %_ptr_Private_mat4v4float Private
%void = OpTypeVoid
%20 = OpTypeFunction %void
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%main = OpFunction %void None %20
%21 = OpLabel
%22 = OpVariable %_ptr_Function_mat3v3float Function
%44 = OpVariable %_ptr_Function_mat4v4float Function
%25 = OpLoad %mat3v3float %a
%26 = OpLoad %mat3v3float %b
%27 = OpCompositeExtract %v3float %25 0
%28 = OpCompositeExtract %v3float %26 0
%29 = OpFMul %v3float %27 %28
%30 = OpCompositeExtract %v3float %25 1
%31 = OpCompositeExtract %v3float %26 1
%32 = OpFMul %v3float %30 %31
%33 = OpCompositeExtract %v3float %25 2
%34 = OpCompositeExtract %v3float %26 2
%35 = OpFMul %v3float %33 %34
%36 = OpCompositeConstruct %mat3v3float %29 %32 %35
OpStore %22 %36
%39 = OpAccessChain %_ptr_Function_v3float %22 %int_0
%41 = OpLoad %v3float %39
%42 = OpLoad %v4float %sk_FragColor
%43 = OpVectorShuffle %v4float %42 %41 4 5 6 3
OpStore %sk_FragColor %43
%47 = OpLoad %mat4v4float %c
%48 = OpLoad %mat4v4float %d
%49 = OpCompositeExtract %v4float %47 0
%50 = OpCompositeExtract %v4float %48 0
%51 = OpFMul %v4float %49 %50
%52 = OpCompositeExtract %v4float %47 1
%53 = OpCompositeExtract %v4float %48 1
%54 = OpFMul %v4float %52 %53
%55 = OpCompositeExtract %v4float %47 2
%56 = OpCompositeExtract %v4float %48 2
%57 = OpFMul %v4float %55 %56
%58 = OpCompositeExtract %v4float %47 3
%59 = OpCompositeExtract %v4float %48 3
%60 = OpFMul %v4float %58 %59
%61 = OpCompositeConstruct %mat4v4float %51 %54 %57 %60
OpStore %44 %61
%62 = OpAccessChain %_ptr_Function_v4float %44 %int_0
%64 = OpLoad %v4float %62
OpStore %sk_FragColor %64
OpReturn
OpFunctionEnd
