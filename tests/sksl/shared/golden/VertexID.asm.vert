OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %sk_VertexID %id
OpName %sk_VertexID "sk_VertexID"
OpName %id "id"
OpName %main "main"
OpDecorate %sk_VertexID BuiltIn VertexIndex
OpDecorate %id Location 1
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%sk_VertexID = OpVariable %_ptr_Input_int Input
%_ptr_Output_int = OpTypePointer Output %int
%id = OpVariable %_ptr_Output_int Output
%void = OpTypeVoid
%9 = OpTypeFunction %void
%main = OpFunction %void None %9
%10 = OpLabel
%11 = OpLoad %int %sk_VertexID
OpStore %id %11
OpReturn
OpFunctionEnd
