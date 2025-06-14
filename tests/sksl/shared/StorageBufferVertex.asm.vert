               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %7 %sk_VertexID

               ; Debug Information
               OpName %sk_PerVertex "sk_PerVertex"  ; id %10
               OpMemberName %sk_PerVertex 0 "sk_Position"
               OpMemberName %sk_PerVertex 1 "sk_PointSize"
               OpName %storageBuffer "storageBuffer"    ; id %15
               OpMemberName %storageBuffer 0 "vertices"
               OpName %sk_VertexID "sk_VertexID"    ; id %17
               OpName %main "main"                  ; id %6

               ; Annotations
               OpMemberDecorate %sk_PerVertex 0 BuiltIn Position
               OpMemberDecorate %sk_PerVertex 1 BuiltIn PointSize
               OpDecorate %sk_PerVertex Block
               OpDecorate %_runtimearr_v2float ArrayStride 8
               OpMemberDecorate %storageBuffer 0 Offset 0
               OpDecorate %storageBuffer BufferBlock
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %sk_VertexID BuiltIn VertexIndex

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%sk_PerVertex = OpTypeStruct %v4float %float        ; Block
%_ptr_Output_sk_PerVertex = OpTypePointer Output %sk_PerVertex
          %7 = OpVariable %_ptr_Output_sk_PerVertex Output
    %v2float = OpTypeVector %float 2
%_runtimearr_v2float = OpTypeRuntimeArray %v2float  ; ArrayStride 8
%storageBuffer = OpTypeStruct %_runtimearr_v2float  ; BufferBlock
%_ptr_Uniform_storageBuffer = OpTypePointer Uniform %storageBuffer
         %12 = OpVariable %_ptr_Uniform_storageBuffer Uniform   ; Binding 0, DescriptorSet 0
%sk_VertexID = OpVariable %_ptr_Input_int Input                 ; BuiltIn VertexIndex
       %void = OpTypeVoid
         %19 = OpTypeFunction %void
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
    %float_1 = OpConstant %float 1
%_ptr_Output_v4float = OpTypePointer Output %v4float


               ; Function main
       %main = OpFunction %void None %19

         %20 = OpLabel
         %22 =   OpLoad %int %sk_VertexID
         %23 =   OpAccessChain %_ptr_Uniform_v2float %12 %int_0 %22
         %25 =   OpLoad %v2float %23
         %26 =   OpCompositeExtract %float %25 0
         %27 =   OpCompositeExtract %float %25 1
         %29 =   OpCompositeConstruct %v4float %26 %27 %float_1 %float_1
         %30 =   OpAccessChain %_ptr_Output_v4float %7 %int_0
                 OpStore %30 %29
                 OpReturn
               OpFunctionEnd
