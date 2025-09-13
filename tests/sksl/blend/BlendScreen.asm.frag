               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %main "main"                  ; id %6

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %20 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %24 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
      %int_1 = OpConstant %int 1


               ; Function main
       %main = OpFunction %void None %15

         %16 = OpLabel
         %17 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %20 =   OpLoad %v4float %17                ; RelaxedPrecision
         %22 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %23 =   OpLoad %v4float %22                ; RelaxedPrecision
         %25 =   OpFSub %v4float %24 %23            ; RelaxedPrecision
         %26 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %28 =   OpLoad %v4float %26                ; RelaxedPrecision
         %29 =   OpFMul %v4float %25 %28            ; RelaxedPrecision
         %30 =   OpFAdd %v4float %20 %29            ; RelaxedPrecision
                 OpStore %sk_FragColor %30
                 OpReturn
               OpFunctionEnd
