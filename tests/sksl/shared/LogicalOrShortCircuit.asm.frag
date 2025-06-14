               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %10
               OpName %_UniformBuffer "_UniformBuffer"  ; id %15
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %17
               OpName %TrueFalse_b "TrueFalse_b"        ; id %6
               OpName %x "x"                            ; id %30
               OpName %y "y"                            ; id %33
               OpName %FalseTrue_b "FalseTrue_b"        ; id %7
               OpName %x_0 "x"                          ; id %51
               OpName %y_0 "y"                          ; id %52
               OpName %FalseFalse_b "FalseFalse_b"      ; id %8
               OpName %x_1 "x"                          ; id %69
               OpName %y_1 "y"                          ; id %70
               OpName %main "main"                      ; id %9
               OpName %_0_TrueTrue "_0_TrueTrue"        ; id %88
               OpName %_2_y "_2_y"                      ; id %90

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
               OpDecorate %14 Binding 0
               OpDecorate %14 DescriptorSet 0
               OpDecorate %111 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %14 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %19 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %23 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %28 = OpTypeFunction %bool
%_ptr_Function_int = OpTypePointer Function %int
      %int_1 = OpConstant %int 1
       %true = OpConstantTrue %bool
      %int_3 = OpConstant %int 3
      %false = OpConstantFalse %bool
      %int_2 = OpConstant %int 2
         %85 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %19

         %20 = OpLabel
         %24 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %24 %23
         %26 =   OpFunctionCall %v4float %main %24
                 OpStore %sk_FragColor %26
                 OpReturn
               OpFunctionEnd


               ; Function TrueFalse_b
%TrueFalse_b = OpFunction %bool None %28

         %29 = OpLabel
          %x =   OpVariable %_ptr_Function_int Function
          %y =   OpVariable %_ptr_Function_int Function
                 OpStore %x %int_1
                 OpStore %y %int_1
                 OpSelectionMerge %36 None
                 OpBranchConditional %true %36 %35

         %35 =     OpLabel
         %37 =       OpIAdd %int %int_1 %int_1
                     OpStore %y %37
         %39 =       OpIEqual %bool %37 %int_3
                     OpBranch %36

         %36 = OpLabel
         %40 =   OpPhi %bool %true %29 %39 %35
                 OpSelectionMerge %43 None
                 OpBranchConditional %40 %41 %42

         %41 =     OpLabel
                     OpSelectionMerge %46 None
                     OpBranchConditional %true %45 %46

         %45 =         OpLabel
         %47 =           OpLoad %int %y
         %48 =           OpIEqual %bool %47 %int_1
                         OpBranch %46

         %46 =     OpLabel
         %49 =       OpPhi %bool %false %41 %48 %45
                     OpReturnValue %49

         %42 =     OpLabel
                     OpReturnValue %false

         %43 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function FalseTrue_b
%FalseTrue_b = OpFunction %bool None %28

         %50 = OpLabel
        %x_0 =   OpVariable %_ptr_Function_int Function
        %y_0 =   OpVariable %_ptr_Function_int Function
                 OpStore %x_0 %int_1
                 OpStore %y_0 %int_1
         %54 =   OpIEqual %bool %int_1 %int_2
                 OpSelectionMerge %56 None
                 OpBranchConditional %54 %56 %55

         %55 =     OpLabel
         %57 =       OpIAdd %int %int_1 %int_1
                     OpStore %y_0 %57
         %58 =       OpIEqual %bool %57 %int_2
                     OpBranch %56

         %56 = OpLabel
         %59 =   OpPhi %bool %true %50 %58 %55
                 OpSelectionMerge %62 None
                 OpBranchConditional %59 %60 %61

         %60 =     OpLabel
                     OpSelectionMerge %64 None
                     OpBranchConditional %true %63 %64

         %63 =         OpLabel
         %65 =           OpLoad %int %y_0
         %66 =           OpIEqual %bool %65 %int_2
                         OpBranch %64

         %64 =     OpLabel
         %67 =       OpPhi %bool %false %60 %66 %63
                     OpReturnValue %67

         %61 =     OpLabel
                     OpReturnValue %false

         %62 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function FalseFalse_b
%FalseFalse_b = OpFunction %bool None %28

         %68 = OpLabel
        %x_1 =   OpVariable %_ptr_Function_int Function
        %y_1 =   OpVariable %_ptr_Function_int Function
                 OpStore %x_1 %int_1
                 OpStore %y_1 %int_1
         %71 =   OpIEqual %bool %int_1 %int_2
                 OpSelectionMerge %73 None
                 OpBranchConditional %71 %73 %72

         %72 =     OpLabel
         %74 =       OpIAdd %int %int_1 %int_1
                     OpStore %y_1 %74
         %75 =       OpIEqual %bool %74 %int_3
                     OpBranch %73

         %73 = OpLabel
         %76 =   OpPhi %bool %true %68 %75 %72
                 OpSelectionMerge %79 None
                 OpBranchConditional %76 %77 %78

         %77 =     OpLabel
                     OpReturnValue %false

         %78 =     OpLabel
                     OpSelectionMerge %81 None
                     OpBranchConditional %true %80 %81

         %80 =         OpLabel
         %82 =           OpLoad %int %y_1
         %83 =           OpIEqual %bool %82 %int_2
                         OpBranch %81

         %81 =     OpLabel
         %84 =       OpPhi %bool %false %78 %83 %80
                     OpReturnValue %84

         %79 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %85         ; RelaxedPrecision
         %86 = OpFunctionParameter %_ptr_Function_v2float

         %87 = OpLabel
%_0_TrueTrue =   OpVariable %_ptr_Function_bool Function
       %_2_y =   OpVariable %_ptr_Function_int Function
        %103 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %_2_y %int_1
                 OpStore %_0_TrueTrue %true
                 OpSelectionMerge %92 None
                 OpBranchConditional %true %91 %92

         %91 =     OpLabel
         %93 =       OpFunctionCall %bool %TrueFalse_b
                     OpBranch %92

         %92 = OpLabel
         %94 =   OpPhi %bool %false %87 %93 %91
                 OpSelectionMerge %96 None
                 OpBranchConditional %94 %95 %96

         %95 =     OpLabel
         %97 =       OpFunctionCall %bool %FalseTrue_b
                     OpBranch %96

         %96 = OpLabel
         %98 =   OpPhi %bool %false %92 %97 %95
                 OpSelectionMerge %100 None
                 OpBranchConditional %98 %99 %100

         %99 =     OpLabel
        %101 =       OpFunctionCall %bool %FalseFalse_b
                     OpBranch %100

        %100 = OpLabel
        %102 =   OpPhi %bool %false %96 %101 %99
                 OpSelectionMerge %107 None
                 OpBranchConditional %102 %105 %106

        %105 =     OpLabel
        %108 =       OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %111 =       OpLoad %v4float %108           ; RelaxedPrecision
                     OpStore %103 %111
                     OpBranch %107

        %106 =     OpLabel
        %112 =       OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %113 =       OpLoad %v4float %112           ; RelaxedPrecision
                     OpStore %103 %113
                     OpBranch %107

        %107 = OpLabel
        %114 =   OpLoad %v4float %103               ; RelaxedPrecision
                 OpReturnValue %114
               OpFunctionEnd
