OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%float_0_5 = OpConstant %float 0.5
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%float_1 = OpConstant %float 1
%main = OpFunction %void None %7
%8 = OpLabel
%13 = OpNot %int %int_0
%14 = OpIMul %int %int_0 %13
%15 = OpConvertSToF %float %14
%16 = OpFSub %float %float_0_5 %15
%20 = OpCompositeConstruct %v2float %16 %16
%21 = OpFSub %v2float %20 %19
%23 = OpCompositeConstruct %v2float %float_1 %float_1
%24 = OpFAdd %v2float %21 %23
OpReturn
OpFunctionEnd
