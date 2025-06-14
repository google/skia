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
               OpName %x_0 "x"                          ; id %52
               OpName %y_0 "y"                          ; id %53
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
               OpDecorate %97 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision

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
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
      %int_3 = OpConstant %int 3
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
                 OpSelectionMerge %37 None
                 OpBranchConditional %true %36 %37

         %36 =     OpLabel
         %38 =       OpIAdd %int %int_1 %int_1
                     OpStore %y %38
         %40 =       OpIEqual %bool %38 %int_3
                     OpBranch %37

         %37 = OpLabel
         %41 =   OpPhi %bool %false %29 %40 %36
                 OpSelectionMerge %44 None
                 OpBranchConditional %41 %42 %43

         %42 =     OpLabel
                     OpReturnValue %false

         %43 =     OpLabel
                     OpSelectionMerge %46 None
                     OpBranchConditional %true %45 %46

         %45 =         OpLabel
         %47 =           OpLoad %int %y
         %49 =           OpIEqual %bool %47 %int_2
                         OpBranch %46

         %46 =     OpLabel
         %50 =       OpPhi %bool %false %43 %49 %45
                     OpReturnValue %50

         %44 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function FalseTrue_b
%FalseTrue_b = OpFunction %bool None %28

         %51 = OpLabel
        %x_0 =   OpVariable %_ptr_Function_int Function
        %y_0 =   OpVariable %_ptr_Function_int Function
                 OpStore %x_0 %int_1
                 OpStore %y_0 %int_1
         %54 =   OpIEqual %bool %int_1 %int_2
                 OpSelectionMerge %56 None
                 OpBranchConditional %54 %55 %56

         %55 =     OpLabel
         %57 =       OpIAdd %int %int_1 %int_1
                     OpStore %y_0 %57
         %58 =       OpIEqual %bool %57 %int_2
                     OpBranch %56

         %56 = OpLabel
         %59 =   OpPhi %bool %false %51 %58 %55
                 OpSelectionMerge %62 None
                 OpBranchConditional %59 %60 %61

         %60 =     OpLabel
                     OpReturnValue %false

         %61 =     OpLabel
                     OpSelectionMerge %64 None
                     OpBranchConditional %true %63 %64

         %63 =         OpLabel
         %65 =           OpLoad %int %y_0
         %66 =           OpIEqual %bool %65 %int_1
                         OpBranch %64

         %64 =     OpLabel
         %67 =       OpPhi %bool %false %61 %66 %63
                     OpReturnValue %67

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
                 OpBranchConditional %71 %72 %73

         %72 =     OpLabel
         %74 =       OpIAdd %int %int_1 %int_1
                     OpStore %y_1 %74
         %75 =       OpIEqual %bool %74 %int_3
                     OpBranch %73

         %73 = OpLabel
         %76 =   OpPhi %bool %false %68 %75 %72
                 OpSelectionMerge %79 None
                 OpBranchConditional %76 %77 %78

         %77 =     OpLabel
                     OpReturnValue %false

         %78 =     OpLabel
                     OpSelectionMerge %81 None
                     OpBranchConditional %true %80 %81

         %80 =         OpLabel
         %82 =           OpLoad %int %y_1
         %83 =           OpIEqual %bool %82 %int_1
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
        %110 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %_2_y %int_1
         %91 =   OpIAdd %int %int_1 %int_1
                 OpStore %_2_y %91
         %92 =   OpIEqual %bool %91 %int_2
                 OpSelectionMerge %95 None
                 OpBranchConditional %92 %93 %94

         %93 =     OpLabel
         %96 =       OpIEqual %bool %91 %int_2
                     OpStore %_0_TrueTrue %96
                     OpBranch %95

         %94 =     OpLabel
                     OpStore %_0_TrueTrue %false
                     OpBranch %95

         %95 = OpLabel
         %97 =   OpLoad %bool %_0_TrueTrue          ; RelaxedPrecision
                 OpSelectionMerge %99 None
                 OpBranchConditional %97 %98 %99

         %98 =     OpLabel
        %100 =       OpFunctionCall %bool %TrueFalse_b
                     OpBranch %99

         %99 = OpLabel
        %101 =   OpPhi %bool %false %95 %100 %98
                 OpSelectionMerge %103 None
                 OpBranchConditional %101 %102 %103

        %102 =     OpLabel
        %104 =       OpFunctionCall %bool %FalseTrue_b
                     OpBranch %103

        %103 = OpLabel
        %105 =   OpPhi %bool %false %99 %104 %102
                 OpSelectionMerge %107 None
                 OpBranchConditional %105 %106 %107

        %106 =     OpLabel
        %108 =       OpFunctionCall %bool %FalseFalse_b
                     OpBranch %107

        %107 = OpLabel
        %109 =   OpPhi %bool %false %103 %108 %106
                 OpSelectionMerge %114 None
                 OpBranchConditional %109 %112 %113

        %112 =     OpLabel
        %115 =       OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %118 =       OpLoad %v4float %115           ; RelaxedPrecision
                     OpStore %110 %118
                     OpBranch %114

        %113 =     OpLabel
        %119 =       OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %120 =       OpLoad %v4float %119           ; RelaxedPrecision
                     OpStore %110 %120
                     OpBranch %114

        %114 = OpLabel
        %121 =   OpLoad %v4float %110               ; RelaxedPrecision
                 OpReturnValue %121
               OpFunctionEnd
