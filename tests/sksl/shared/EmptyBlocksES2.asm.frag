               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "unknownInput"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %color "color"                    ; id %27

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %color RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision

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
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %29 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Uniform_float = OpTypePointer Uniform %float
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
       %bool = OpTypeBool
%_ptr_Function_float = OpTypePointer Function %float
      %int_1 = OpConstant %int 1
    %float_2 = OpConstant %float 2
      %int_3 = OpConstant %int 3


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %16

         %17 = OpLabel
         %21 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %21 %20
         %23 =   OpFunctionCall %v4float %main %21
                 OpStore %sk_FragColor %23
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %24         ; RelaxedPrecision
         %25 = OpFunctionParameter %_ptr_Function_v2float

         %26 = OpLabel
      %color =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
                 OpStore %color %29
         %30 =   OpAccessChain %_ptr_Uniform_float %11 %int_0
         %33 =   OpLoad %float %30                  ; RelaxedPrecision
         %35 =   OpFOrdEqual %bool %33 %float_1
                 OpSelectionMerge %38 None
                 OpBranchConditional %35 %37 %38

         %37 =     OpLabel
         %39 =       OpAccessChain %_ptr_Function_float %color %int_1
                     OpStore %39 %float_1
                     OpBranch %38

         %38 = OpLabel
         %42 =   OpAccessChain %_ptr_Uniform_float %11 %int_0
         %43 =   OpLoad %float %42                  ; RelaxedPrecision
         %45 =   OpFOrdEqual %bool %43 %float_2
                 OpSelectionMerge %48 None
                 OpBranchConditional %45 %46 %47

         %46 =     OpLabel
                     OpBranch %48

         %47 =     OpLabel
         %49 =       OpAccessChain %_ptr_Function_float %color %int_3
                     OpStore %49 %float_1
                     OpBranch %48

         %48 = OpLabel
         %51 =   OpLoad %v4float %color             ; RelaxedPrecision
                 OpReturnValue %51
               OpFunctionEnd
