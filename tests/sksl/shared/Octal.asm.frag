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
               OpName %i1 "i1"                          ; id %27
               OpName %i2 "i2"                          ; id %30
               OpName %i3 "i3"                          ; id %32
               OpName %i4 "i4"                          ; id %34

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
               OpDecorate %56 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision

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
      %int_1 = OpConstant %int 1
 %int_342391 = OpConstant %int 342391
%int_2000000000 = OpConstant %int 2000000000
%int_n2000000000 = OpConstant %int -2000000000
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0


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
         %i1 =   OpVariable %_ptr_Function_int Function
         %i2 =   OpVariable %_ptr_Function_int Function
         %i3 =   OpVariable %_ptr_Function_int Function
         %i4 =   OpVariable %_ptr_Function_int Function
         %48 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %i1 %int_1
                 OpStore %i2 %int_342391
                 OpStore %i3 %int_2000000000
                 OpStore %i4 %int_n2000000000
                 OpSelectionMerge %40 None
                 OpBranchConditional %true %39 %40

         %39 =     OpLabel
                     OpBranch %40

         %40 = OpLabel
         %41 =   OpPhi %bool %false %26 %true %39
                 OpSelectionMerge %43 None
                 OpBranchConditional %41 %42 %43

         %42 =     OpLabel
                     OpBranch %43

         %43 = OpLabel
         %44 =   OpPhi %bool %false %40 %true %42
                 OpSelectionMerge %46 None
                 OpBranchConditional %44 %45 %46

         %45 =     OpLabel
                     OpBranch %46

         %46 = OpLabel
         %47 =   OpPhi %bool %false %43 %true %45
                 OpSelectionMerge %52 None
                 OpBranchConditional %47 %50 %51

         %50 =     OpLabel
         %53 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %56 =       OpLoad %v4float %53            ; RelaxedPrecision
                     OpStore %48 %56
                     OpBranch %52

         %51 =     OpLabel
         %57 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %58 =       OpLoad %v4float %57            ; RelaxedPrecision
                     OpStore %48 %58
                     OpBranch %52

         %52 = OpLabel
         %59 =   OpLoad %v4float %48                ; RelaxedPrecision
                 OpReturnValue %59
               OpFunctionEnd
