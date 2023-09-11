               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %3 %sk_VertexID
               OpName %sk_PerVertex "sk_PerVertex"
               OpMemberName %sk_PerVertex 0 "sk_Position"
               OpMemberName %sk_PerVertex 1 "sk_PointSize"
               OpName %storageBuffer "storageBuffer"
               OpMemberName %storageBuffer 0 "vertices"
               OpName %sk_VertexID "sk_VertexID"
               OpName %main "main"
               OpMemberDecorate %sk_PerVertex 0 BuiltIn Position
               OpMemberDecorate %sk_PerVertex 1 BuiltIn PointSize
               OpDecorate %sk_PerVertex Block
               OpDecorate %_runtimearr_v2float ArrayStride 16
               OpMemberDecorate %storageBuffer 0 Offset 0
               OpDecorate %storageBuffer BufferBlock
               OpDecorate %8 Binding 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %sk_VertexID BuiltIn VertexIndex
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%sk_PerVertex = OpTypeStruct %v4float %float
%_ptr_Output_sk_PerVertex = OpTypePointer Output %sk_PerVertex
          %3 = OpVariable %_ptr_Output_sk_PerVertex Output
    %v2float = OpTypeVector %float 2
%_runtimearr_v2float = OpTypeRuntimeArray %v2float
%storageBuffer = OpTypeStruct %_runtimearr_v2float
%_ptr_Uniform_storageBuffer = OpTypePointer Uniform %storageBuffer
          %8 = OpVariable %_ptr_Uniform_storageBuffer Uniform
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%sk_VertexID = OpVariable %_ptr_Input_int Input
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
    %float_1 = OpConstant %float 1
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %main = OpFunction %void None %17
         %18 = OpLabel
         %20 = OpLoad %int %sk_VertexID
         %21 = OpAccessChain %_ptr_Uniform_v2float %8 %int_0 %20
         %23 = OpLoad %v2float %21
         %24 = OpCompositeExtract %float %23 0
         %25 = OpCompositeExtract %float %23 1
         %27 = OpCompositeConstruct %v4float %24 %25 %float_1 %float_1
         %28 = OpAccessChain %_ptr_Output_v4float %3 %int_0
               OpStore %28 %27
               OpReturn
               OpFunctionEnd
