OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %3 %pos
OpName %sk_PerVertex "sk_PerVertex"
OpMemberName %sk_PerVertex 0 "sk_Position"
OpMemberName %sk_PerVertex 1 "sk_PointSize"
OpName %pos "pos"
OpName %main "main"
OpMemberDecorate %sk_PerVertex 0 BuiltIn Position
OpMemberDecorate %sk_PerVertex 1 BuiltIn PointSize
OpDecorate %sk_PerVertex Block
OpDecorate %pos Location 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%sk_PerVertex = OpTypeStruct %v4float %float
%_ptr_Output_sk_PerVertex = OpTypePointer Output %sk_PerVertex
%3 = OpVariable %_ptr_Output_sk_PerVertex Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
%pos = OpVariable %_ptr_Input_v4float Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
%main = OpFunction %void None %11
%12 = OpLabel
%13 = OpLoad %v4float %pos
%16 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %16 %13
OpReturn
OpFunctionEnd
