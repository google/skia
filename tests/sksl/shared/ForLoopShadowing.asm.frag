               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %counter "counter"                ; id %27
               OpName %i "i"                            ; id %30

               ; Annotations
               OpDecorate %main RelaxedPrecision
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
               OpDecorate %59 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision

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
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
     %int_10 = OpConstant %int 10
       %bool = OpTypeBool
      %int_1 = OpConstant %int 1
     %int_90 = OpConstant %int 90
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float


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
    %counter =   OpVariable %_ptr_Function_int Function
          %i =   OpVariable %_ptr_Function_int Function
         %52 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %counter %int_0
                 OpStore %i %int_0
                 OpBranch %31

         %31 = OpLabel
                 OpLoopMerge %35 %34 None
                 OpBranch %32

         %32 =     OpLabel
         %36 =       OpLoad %int %i
         %38 =       OpSLessThan %bool %36 %int_10
                     OpBranchConditional %38 %33 %35

         %33 =         OpLabel
         %40 =           OpLoad %int %i
         %41 =           OpIEqual %bool %40 %int_0
                         OpSelectionMerge %43 None
                         OpBranchConditional %41 %42 %43

         %42 =             OpLabel
                             OpBranch %34

         %43 =         OpLabel
         %44 =           OpLoad %int %counter
         %45 =           OpIAdd %int %44 %int_10
                         OpStore %counter %45
                         OpBranch %34

         %34 =   OpLabel
         %46 =     OpLoad %int %i
         %48 =     OpIAdd %int %46 %int_1
                   OpStore %i %48
                   OpBranch %31

         %35 = OpLabel
         %49 =   OpLoad %int %counter
         %51 =   OpIEqual %bool %49 %int_90
                 OpSelectionMerge %56 None
                 OpBranchConditional %51 %54 %55

         %54 =     OpLabel
         %57 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %59 =       OpLoad %v4float %57            ; RelaxedPrecision
                     OpStore %52 %59
                     OpBranch %56

         %55 =     OpLabel
         %60 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %61 =       OpLoad %v4float %60            ; RelaxedPrecision
                     OpStore %52 %61
                     OpBranch %56

         %56 = OpLabel
         %62 =   OpLoad %v4float %52                ; RelaxedPrecision
                 OpReturnValue %62
               OpFunctionEnd
