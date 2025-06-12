               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %b "b"                        ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %11
               OpMemberName %_UniformBuffer 0 "a"
               OpName %main "main"                  ; id %2

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
        %int = OpTypeInt 32 1
%_ptr_Private_int = OpTypePointer Private %int
          %b = OpVariable %_ptr_Private_int Private
%_UniformBuffer = OpTypeStruct %float               ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
%_ptr_Uniform_float = OpTypePointer Uniform %float
      %int_0 = OpConstant %int 0
%_ptr_Output_float = OpTypePointer Output %float


               ; Function main
       %main = OpFunction %void None %14

         %15 = OpLabel
         %17 =   OpAccessChain %_ptr_Uniform_float %10 %int_0
         %20 =   OpLoad %float %17
         %21 =   OpLoad %int %b
         %16 =   OpExtInst %float %1 Ldexp %20 %21
         %22 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
                 OpStore %22 %16
                 OpReturn
               OpFunctionEnd
