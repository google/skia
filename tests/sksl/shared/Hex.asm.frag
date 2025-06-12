               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %_UniformBuffer "_UniformBuffer"  ; id %8
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %10
               OpName %main "main"                      ; id %2
               OpName %i1 "i1"                          ; id %23
               OpName %i2 "i2"                          ; id %29
               OpName %i3 "i3"                          ; id %32
               OpName %i4 "i4"                          ; id %35
               OpName %i5 "i5"                          ; id %38

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %43 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float                 ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
   %int_4660 = OpConstant %int 4660
  %int_32766 = OpConstant %int 32766
 %int_n32766 = OpConstant %int -32766
  %int_19132 = OpConstant %int 19132
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %12

         %13 = OpLabel
         %17 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %17 %16
         %19 =   OpFunctionCall %v4float %main %17
                 OpStore %sk_FragColor %19
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %20         ; RelaxedPrecision
         %21 = OpFunctionParameter %_ptr_Function_v2float

         %22 = OpLabel
         %i1 =   OpVariable %_ptr_Function_int Function
         %i2 =   OpVariable %_ptr_Function_int Function
         %i3 =   OpVariable %_ptr_Function_int Function
         %i4 =   OpVariable %_ptr_Function_int Function
         %i5 =   OpVariable %_ptr_Function_int Function
                 OpStore %i1 %int_0
         %28 =   OpIAdd %int %int_0 %int_1
                 OpStore %i1 %28
                 OpStore %i2 %int_4660
         %31 =   OpIAdd %int %int_4660 %int_1
                 OpStore %i2 %31
                 OpStore %i3 %int_32766
         %34 =   OpIAdd %int %int_32766 %int_1
                 OpStore %i3 %34
                 OpStore %i4 %int_n32766
         %37 =   OpIAdd %int %int_n32766 %int_1
                 OpStore %i4 %37
                 OpStore %i5 %int_19132
         %40 =   OpIAdd %int %int_19132 %int_1
                 OpStore %i5 %40
         %41 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %43 =   OpLoad %v4float %41                ; RelaxedPrecision
                 OpReturnValue %43
               OpFunctionEnd
