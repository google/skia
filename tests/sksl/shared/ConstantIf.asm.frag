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
               OpName %a "a"                            ; id %27
               OpName %b "b"                            ; id %30
               OpName %c "c"                            ; id %31
               OpName %d "d"                            ; id %32

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
               OpDecorate %55 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision

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
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_5 = OpConstant %int 5
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
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
          %a =   OpVariable %_ptr_Function_int Function
          %b =   OpVariable %_ptr_Function_int Function
          %c =   OpVariable %_ptr_Function_int Function
          %d =   OpVariable %_ptr_Function_int Function
         %48 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %a %int_0
                 OpStore %b %int_0
                 OpStore %c %int_0
                 OpStore %d %int_0
                 OpStore %a %int_1
                 OpStore %b %int_2
                 OpStore %c %int_5
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
         %55 =       OpLoad %v4float %53            ; RelaxedPrecision
                     OpStore %48 %55
                     OpBranch %52

         %51 =     OpLabel
         %56 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %57 =       OpLoad %v4float %56            ; RelaxedPrecision
                     OpStore %48 %57
                     OpBranch %52

         %52 = OpLabel
         %58 =   OpLoad %v4float %48                ; RelaxedPrecision
                 OpReturnValue %58
               OpFunctionEnd
