               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %12
               OpName %_UniformBuffer "_UniformBuffer"  ; id %17
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "unknownInput"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %19
               OpName %return_on_both_sides_b "return_on_both_sides_b"  ; id %6
               OpName %for_inside_body_b "for_inside_body_b"            ; id %7
               OpName %x "x"                                            ; id %43
               OpName %after_for_body_b "after_for_body_b"              ; id %8
               OpName %x_0 "x"                                          ; id %58
               OpName %for_with_double_sided_conditional_return_b "for_with_double_sided_conditional_return_b"  ; id %9
               OpName %x_1 "x"                                                                                  ; id %69
               OpName %if_else_chain_b "if_else_chain_b"                                                        ; id %10
               OpName %main "main"                                                                              ; id %11

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %16 Binding 0
               OpDecorate %16 DescriptorSet 0
               OpDecorate %35 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float %float     ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %16 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %21 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %25 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %30 = OpTypeFunction %bool
%_ptr_Uniform_float = OpTypePointer Uniform %float
      %int_2 = OpConstant %int 2
    %float_1 = OpConstant %float 1
       %true = OpConstantTrue %bool
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
     %int_10 = OpConstant %int 10
      %int_1 = OpConstant %int 1
    %float_2 = OpConstant %float 2
      %false = OpConstantFalse %bool
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
        %114 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %21

         %22 = OpLabel
         %26 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %26 %25
         %28 =   OpFunctionCall %v4float %main %26
                 OpStore %sk_FragColor %28
                 OpReturn
               OpFunctionEnd


               ; Function return_on_both_sides_b
%return_on_both_sides_b = OpFunction %bool None %30

         %31 = OpLabel
         %32 =   OpAccessChain %_ptr_Uniform_float %16 %int_2
         %35 =   OpLoad %float %32                  ; RelaxedPrecision
         %37 =   OpFOrdEqual %bool %35 %float_1
                 OpSelectionMerge %40 None
                 OpBranchConditional %37 %38 %39

         %38 =     OpLabel
                     OpReturnValue %true

         %39 =     OpLabel
                     OpReturnValue %true

         %40 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function for_inside_body_b
%for_inside_body_b = OpFunction %bool None %30

         %42 = OpLabel
          %x =   OpVariable %_ptr_Function_int Function
                 OpStore %x %int_0
                 OpBranch %46

         %46 = OpLabel
                 OpLoopMerge %50 %49 None
                 OpBranch %47

         %47 =     OpLabel
         %51 =       OpLoad %int %x
         %53 =       OpSLessThanEqual %bool %51 %int_10
                     OpBranchConditional %53 %48 %50

         %48 =         OpLabel
                         OpReturnValue %true

         %49 =   OpLabel
         %55 =     OpLoad %int %x
         %56 =     OpIAdd %int %55 %int_1
                   OpStore %x %56
                   OpBranch %46

         %50 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function after_for_body_b
%after_for_body_b = OpFunction %bool None %30

         %57 = OpLabel
        %x_0 =   OpVariable %_ptr_Function_int Function
                 OpStore %x_0 %int_0
                 OpBranch %59

         %59 = OpLabel
                 OpLoopMerge %63 %62 None
                 OpBranch %60

         %60 =     OpLabel
         %64 =       OpLoad %int %x_0
         %65 =       OpSLessThanEqual %bool %64 %int_10
                     OpBranchConditional %65 %61 %63

         %61 =         OpLabel
                         OpBranch %62

         %62 =   OpLabel
         %66 =     OpLoad %int %x_0
         %67 =     OpIAdd %int %66 %int_1
                   OpStore %x_0 %67
                   OpBranch %59

         %63 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function for_with_double_sided_conditional_return_b
%for_with_double_sided_conditional_return_b = OpFunction %bool None %30

         %68 = OpLabel
        %x_1 =   OpVariable %_ptr_Function_int Function
                 OpStore %x_1 %int_0
                 OpBranch %70

         %70 = OpLabel
                 OpLoopMerge %74 %73 None
                 OpBranch %71

         %71 =     OpLabel
         %75 =       OpLoad %int %x_1
         %76 =       OpSLessThanEqual %bool %75 %int_10
                     OpBranchConditional %76 %72 %74

         %72 =         OpLabel
         %77 =           OpAccessChain %_ptr_Uniform_float %16 %int_2
         %78 =           OpLoad %float %77          ; RelaxedPrecision
         %79 =           OpFOrdEqual %bool %78 %float_1
                         OpSelectionMerge %82 None
                         OpBranchConditional %79 %80 %81

         %80 =             OpLabel
                             OpReturnValue %true

         %81 =             OpLabel
                             OpReturnValue %true

         %82 =         OpLabel
                         OpBranch %73

         %73 =   OpLabel
         %83 =     OpLoad %int %x_1
         %84 =     OpIAdd %int %83 %int_1
                   OpStore %x_1 %84
                   OpBranch %70

         %74 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function if_else_chain_b
%if_else_chain_b = OpFunction %bool None %30

         %85 = OpLabel
         %86 =   OpAccessChain %_ptr_Uniform_float %16 %int_2
         %87 =   OpLoad %float %86                  ; RelaxedPrecision
         %88 =   OpFOrdEqual %bool %87 %float_1
                 OpSelectionMerge %91 None
                 OpBranchConditional %88 %89 %90

         %89 =     OpLabel
                     OpReturnValue %true

         %90 =     OpLabel
         %92 =       OpAccessChain %_ptr_Uniform_float %16 %int_2
         %93 =       OpLoad %float %92              ; RelaxedPrecision
         %95 =       OpFOrdEqual %bool %93 %float_2
                     OpSelectionMerge %98 None
                     OpBranchConditional %95 %96 %97

         %96 =         OpLabel
                         OpReturnValue %false

         %97 =         OpLabel
        %100 =           OpAccessChain %_ptr_Uniform_float %16 %int_2
        %101 =           OpLoad %float %100         ; RelaxedPrecision
        %103 =           OpFOrdEqual %bool %101 %float_3
                         OpSelectionMerge %106 None
                         OpBranchConditional %103 %104 %105

        %104 =             OpLabel
                             OpReturnValue %true

        %105 =             OpLabel
        %107 =               OpAccessChain %_ptr_Uniform_float %16 %int_2
        %108 =               OpLoad %float %107     ; RelaxedPrecision
        %110 =               OpFOrdEqual %bool %108 %float_4
                             OpSelectionMerge %113 None
                             OpBranchConditional %110 %111 %112

        %111 =                 OpLabel
                                 OpReturnValue %false

        %112 =                 OpLabel
                                 OpReturnValue %true

        %113 =             OpLabel
                             OpBranch %106

        %106 =         OpLabel
                         OpBranch %98

         %98 =     OpLabel
                     OpBranch %91

         %91 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %114        ; RelaxedPrecision
        %115 = OpFunctionParameter %_ptr_Function_v2float

        %116 = OpLabel
        %137 =   OpVariable %_ptr_Function_v4float Function
                 OpSelectionMerge %118 None
                 OpBranchConditional %true %117 %118

        %117 =     OpLabel
        %119 =       OpFunctionCall %bool %return_on_both_sides_b
                     OpBranch %118

        %118 = OpLabel
        %120 =   OpPhi %bool %false %116 %119 %117
                 OpSelectionMerge %122 None
                 OpBranchConditional %120 %121 %122

        %121 =     OpLabel
        %123 =       OpFunctionCall %bool %for_inside_body_b
                     OpBranch %122

        %122 = OpLabel
        %124 =   OpPhi %bool %false %118 %123 %121
                 OpSelectionMerge %126 None
                 OpBranchConditional %124 %125 %126

        %125 =     OpLabel
        %127 =       OpFunctionCall %bool %after_for_body_b
                     OpBranch %126

        %126 = OpLabel
        %128 =   OpPhi %bool %false %122 %127 %125
                 OpSelectionMerge %130 None
                 OpBranchConditional %128 %129 %130

        %129 =     OpLabel
        %131 =       OpFunctionCall %bool %for_with_double_sided_conditional_return_b
                     OpBranch %130

        %130 = OpLabel
        %132 =   OpPhi %bool %false %126 %131 %129
                 OpSelectionMerge %134 None
                 OpBranchConditional %132 %133 %134

        %133 =     OpLabel
        %135 =       OpFunctionCall %bool %if_else_chain_b
                     OpBranch %134

        %134 = OpLabel
        %136 =   OpPhi %bool %false %130 %135 %133
                 OpSelectionMerge %141 None
                 OpBranchConditional %136 %139 %140

        %139 =     OpLabel
        %142 =       OpAccessChain %_ptr_Uniform_v4float %16 %int_0
        %144 =       OpLoad %v4float %142           ; RelaxedPrecision
                     OpStore %137 %144
                     OpBranch %141

        %140 =     OpLabel
        %145 =       OpAccessChain %_ptr_Uniform_v4float %16 %int_1
        %146 =       OpLoad %v4float %145           ; RelaxedPrecision
                     OpStore %137 %146
                     OpBranch %141

        %141 = OpLabel
        %147 =   OpLoad %v4float %137               ; RelaxedPrecision
                 OpReturnValue %147
               OpFunctionEnd
