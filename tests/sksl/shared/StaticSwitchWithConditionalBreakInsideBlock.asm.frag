               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %_UniformBuffer "_UniformBuffer"  ; id %8
               OpMemberName %_UniformBuffer 0 "unknownInput"
               OpName %main "main"                  ; id %2
               OpName %value "value"                ; id %13

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %value RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %float                   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %11 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
    %float_0 = OpConstant %float 0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
    %float_2 = OpConstant %float 2
       %bool = OpTypeBool
         %29 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
    %float_1 = OpConstant %float 1


               ; Function main
       %main = OpFunction %void None %11

         %12 = OpLabel
      %value =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
                 OpStore %value %float_0
                 OpSelectionMerge %18 None
                 OpSwitch %int_0 %18 0 %19 1 %20

         %19 =     OpLabel
                     OpStore %value %float_0
         %21 =       OpAccessChain %_ptr_Uniform_float %7 %int_0
         %23 =       OpLoad %float %21
         %25 =       OpFOrdEqual %bool %23 %float_2
                     OpSelectionMerge %28 None
                     OpBranchConditional %25 %27 %28

         %27 =         OpLabel
                         OpStore %sk_FragColor %29
                         OpBranch %18

         %28 =     OpLabel
                     OpBranch %20

         %20 =     OpLabel
                     OpStore %value %float_1
                     OpBranch %18

         %18 = OpLabel
                 OpReturn
               OpFunctionEnd
