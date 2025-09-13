               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %7

               ; Debug Information
               OpName %sk_PerVertex "sk_PerVertex"  ; id %10
               OpMemberName %sk_PerVertex 0 "sk_Position"
               OpMemberName %sk_PerVertex 1 "sk_PointSize"
               OpName %_UniformBuffer "_UniformBuffer"  ; id %13
               OpMemberName %_UniformBuffer 0 "sk_RTAdjust"
               OpName %main "main"                  ; id %6

               ; Annotations
               OpMemberDecorate %sk_PerVertex 0 BuiltIn Position
               OpMemberDecorate %sk_PerVertex 1 BuiltIn PointSize
               OpDecorate %sk_PerVertex Block
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpDecorate %_UniformBuffer Block
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%sk_PerVertex = OpTypeStruct %v4float %float        ; Block
%_ptr_Output_sk_PerVertex = OpTypePointer Output %sk_PerVertex
          %7 = OpVariable %_ptr_Output_sk_PerVertex Output
%_UniformBuffer = OpTypeStruct %v4float             ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_1 = OpConstant %float 1
         %19 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
      %int_0 = OpConstant %int 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
    %v2float = OpTypeVector %float 2
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
    %float_0 = OpConstant %float 0


               ; Function main
       %main = OpFunction %void None %16

         %17 = OpLabel
         %21 =   OpAccessChain %_ptr_Output_v4float %7 %int_0
                 OpStore %21 %19
         %23 =   OpAccessChain %_ptr_Output_v4float %7 %int_0
         %24 =   OpLoad %v4float %23
         %25 =   OpVectorShuffle %v2float %24 %24 0 1
         %27 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %29 =   OpLoad %v4float %27
         %30 =   OpVectorShuffle %v2float %29 %29 0 2
         %31 =   OpFMul %v2float %25 %30
         %32 =   OpAccessChain %_ptr_Output_v4float %7 %int_0
         %33 =   OpLoad %v4float %32
         %34 =   OpVectorShuffle %v2float %33 %33 3 3
         %35 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %36 =   OpLoad %v4float %35
         %37 =   OpVectorShuffle %v2float %36 %36 1 3
         %38 =   OpFMul %v2float %34 %37
         %39 =   OpFAdd %v2float %31 %38
         %40 =   OpCompositeExtract %float %39 0
         %41 =   OpCompositeExtract %float %39 1
         %43 =   OpAccessChain %_ptr_Output_v4float %7 %int_0
         %44 =   OpLoad %v4float %43
         %45 =   OpCompositeExtract %float %44 3
         %46 =   OpCompositeConstruct %v4float %40 %41 %float_0 %45
         %47 =   OpAccessChain %_ptr_Output_v4float %7 %int_0
                 OpStore %47 %46
                 OpReturn
               OpFunctionEnd
