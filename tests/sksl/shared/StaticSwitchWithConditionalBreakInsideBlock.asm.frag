               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "unknownInput"
               OpName %main "main"                  ; id %6
               OpName %value "value"                ; id %17

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %value RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %float                   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
    %float_0 = OpConstant %float 0
      %int_0 = OpConstant %int 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
    %float_2 = OpConstant %float 2
       %bool = OpTypeBool
         %32 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
    %float_1 = OpConstant %float 1


               ; Function main
       %main = OpFunction %void None %15

         %16 = OpLabel
      %value =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
                 OpStore %value %float_0
                 OpSelectionMerge %21 None
                 OpSwitch %int_0 %21 0 %22 1 %23

         %22 =     OpLabel
                     OpStore %value %float_0
         %24 =       OpAccessChain %_ptr_Uniform_float %11 %int_0
         %26 =       OpLoad %float %24
         %28 =       OpFOrdEqual %bool %26 %float_2
                     OpSelectionMerge %31 None
                     OpBranchConditional %28 %30 %31

         %30 =         OpLabel
                         OpStore %sk_FragColor %32
                         OpBranch %21

         %31 =     OpLabel
                     OpBranch %23

         %23 =     OpLabel
                     OpStore %value %float_1
                     OpBranch %21

         %21 = OpLabel
                 OpReturn
               OpFunctionEnd
