               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %7 %pos

               ; Debug Information
               OpName %sk_PerVertex "sk_PerVertex"  ; id %10
               OpMemberName %sk_PerVertex 0 "sk_Position"
               OpMemberName %sk_PerVertex 1 "sk_PointSize"
               OpName %pos "pos"                    ; id %12
               OpName %_UniformBuffer "_UniformBuffer"  ; id %15
               OpMemberName %_UniformBuffer 0 "sk_RTAdjust"
               OpName %main "main"                  ; id %6

               ; Annotations
               OpMemberDecorate %sk_PerVertex 0 BuiltIn Position
               OpMemberDecorate %sk_PerVertex 1 BuiltIn PointSize
               OpDecorate %sk_PerVertex Block
               OpDecorate %pos Location 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpDecorate %_UniformBuffer Block
               OpDecorate %14 Binding 0
               OpDecorate %14 DescriptorSet 0

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%sk_PerVertex = OpTypeStruct %v4float %float        ; Block
%_ptr_Output_sk_PerVertex = OpTypePointer Output %sk_PerVertex
          %7 = OpVariable %_ptr_Output_sk_PerVertex Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
        %pos = OpVariable %_ptr_Input_v4float Input     ; Location 0
%_UniformBuffer = OpTypeStruct %v4float                 ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %14 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
      %int_0 = OpConstant %int 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
    %v2float = OpTypeVector %float 2
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
    %float_0 = OpConstant %float 0


               ; Function main
       %main = OpFunction %void None %18

         %19 = OpLabel
         %20 =   OpLoad %v4float %pos
         %22 =   OpAccessChain %_ptr_Output_v4float %7 %int_0
                 OpStore %22 %20
         %24 =   OpAccessChain %_ptr_Output_v4float %7 %int_0
         %25 =   OpLoad %v4float %24
         %26 =   OpVectorShuffle %v2float %25 %25 0 1
         %28 =   OpAccessChain %_ptr_Uniform_v4float %14 %int_0
         %30 =   OpLoad %v4float %28
         %31 =   OpVectorShuffle %v2float %30 %30 0 2
         %32 =   OpFMul %v2float %26 %31
         %33 =   OpAccessChain %_ptr_Output_v4float %7 %int_0
         %34 =   OpLoad %v4float %33
         %35 =   OpVectorShuffle %v2float %34 %34 3 3
         %36 =   OpAccessChain %_ptr_Uniform_v4float %14 %int_0
         %37 =   OpLoad %v4float %36
         %38 =   OpVectorShuffle %v2float %37 %37 1 3
         %39 =   OpFMul %v2float %35 %38
         %40 =   OpFAdd %v2float %32 %39
         %41 =   OpCompositeExtract %float %40 0
         %42 =   OpCompositeExtract %float %40 1
         %44 =   OpAccessChain %_ptr_Output_v4float %7 %int_0
         %45 =   OpLoad %v4float %44
         %46 =   OpCompositeExtract %float %45 3
         %47 =   OpCompositeConstruct %v4float %41 %42 %float_0 %46
         %48 =   OpAccessChain %_ptr_Output_v4float %7 %int_0
                 OpStore %48 %47
                 OpReturn
               OpFunctionEnd
