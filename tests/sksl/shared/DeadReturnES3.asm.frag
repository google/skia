               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %15
               OpName %_UniformBuffer "_UniformBuffer"  ; id %20
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %22
               OpName %test_return_b "test_return_b"    ; id %6
               OpName %test_break_b "test_break_b"      ; id %7
               OpName %test_continue_b "test_continue_b"    ; id %8
               OpName %test_if_return_b "test_if_return_b"  ; id %9
               OpName %test_if_break_b "test_if_break_b"    ; id %10
               OpName %test_else_b "test_else_b"            ; id %11
               OpName %test_loop_return_b "test_loop_return_b"  ; id %12
               OpName %test_loop_break_b "test_loop_break_b"    ; id %13
               OpName %x "x"                                    ; id %97
               OpName %main "main"                              ; id %14

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
               OpDecorate %19 Binding 0
               OpDecorate %19 DescriptorSet 0
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %19 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %24 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %28 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %33 = OpTypeFunction %bool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
      %int_1 = OpConstant %int 1
        %109 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %24

         %25 = OpLabel
         %29 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %29 %28
         %31 =   OpFunctionCall %v4float %main %29
                 OpStore %sk_FragColor %31
                 OpReturn
               OpFunctionEnd


               ; Function test_return_b
%test_return_b = OpFunction %bool None %33

         %34 = OpLabel
                 OpBranch %35

         %35 = OpLabel
                 OpLoopMerge %39 %38 None
                 OpBranch %36

         %36 =     OpLabel
                     OpReturnValue %true

         %38 =   OpLabel
                   OpBranchConditional %false %35 %39

         %39 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function test_break_b
%test_break_b = OpFunction %bool None %33

         %42 = OpLabel
                 OpBranch %43

         %43 = OpLabel
                 OpLoopMerge %47 %46 None
                 OpBranch %44

         %44 =     OpLabel
                     OpBranch %47

         %46 =   OpLabel
                   OpBranchConditional %false %43 %47

         %47 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function test_continue_b
%test_continue_b = OpFunction %bool None %33

         %48 = OpLabel
                 OpBranch %49

         %49 = OpLabel
                 OpLoopMerge %53 %52 None
                 OpBranch %50

         %50 =     OpLabel
                     OpBranch %52

         %52 =   OpLabel
                   OpBranchConditional %false %49 %53

         %53 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function test_if_return_b
%test_if_return_b = OpFunction %bool None %33

         %54 = OpLabel
                 OpBranch %55

         %55 = OpLabel
                 OpLoopMerge %59 %58 None
                 OpBranch %56

         %56 =     OpLabel
         %60 =       OpAccessChain %_ptr_Uniform_v4float %19 %int_0
         %63 =       OpLoad %v4float %60            ; RelaxedPrecision
         %64 =       OpCompositeExtract %float %63 1    ; RelaxedPrecision
         %65 =       OpFOrdGreaterThan %bool %64 %float_0
                     OpSelectionMerge %68 None
                     OpBranchConditional %65 %66 %67

         %66 =         OpLabel
                         OpReturnValue %true

         %67 =         OpLabel
                         OpBranch %59

         %68 =     OpLabel
                     OpBranch %58

         %58 =   OpLabel
                   OpBranchConditional %false %55 %59

         %59 = OpLabel
                 OpReturnValue %false
               OpFunctionEnd


               ; Function test_if_break_b
%test_if_break_b = OpFunction %bool None %33

         %69 = OpLabel
                 OpBranch %70

         %70 = OpLabel
                 OpLoopMerge %74 %73 None
                 OpBranch %71

         %71 =     OpLabel
         %75 =       OpAccessChain %_ptr_Uniform_v4float %19 %int_0
         %76 =       OpLoad %v4float %75            ; RelaxedPrecision
         %77 =       OpCompositeExtract %float %76 1    ; RelaxedPrecision
         %78 =       OpFOrdGreaterThan %bool %77 %float_0
                     OpSelectionMerge %81 None
                     OpBranchConditional %78 %79 %80

         %79 =         OpLabel
                         OpBranch %74

         %80 =         OpLabel
                         OpBranch %73

         %81 =     OpLabel
                     OpBranch %72

         %72 =     OpLabel
                     OpBranch %73

         %73 =   OpLabel
                   OpBranchConditional %false %70 %74

         %74 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function test_else_b
%test_else_b = OpFunction %bool None %33

         %82 = OpLabel
                 OpBranch %83

         %83 = OpLabel
                 OpLoopMerge %87 %86 None
                 OpBranch %84

         %84 =     OpLabel
         %88 =       OpAccessChain %_ptr_Uniform_v4float %19 %int_0
         %89 =       OpLoad %v4float %88            ; RelaxedPrecision
         %90 =       OpCompositeExtract %float %89 1    ; RelaxedPrecision
         %91 =       OpFOrdEqual %bool %90 %float_0
                     OpSelectionMerge %94 None
                     OpBranchConditional %91 %92 %93

         %92 =         OpLabel
                         OpReturnValue %false

         %93 =         OpLabel
                         OpReturnValue %true

         %94 =     OpLabel
                     OpBranch %85

         %85 =     OpLabel
                     OpBranch %86

         %86 =   OpLabel
                   OpBranchConditional %false %83 %87

         %87 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function test_loop_return_b
%test_loop_return_b = OpFunction %bool None %33

         %95 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function test_loop_break_b
%test_loop_break_b = OpFunction %bool None %33

         %96 = OpLabel
          %x =   OpVariable %_ptr_Function_int Function
                 OpStore %x %int_0
                 OpBranch %99

         %99 = OpLabel
                 OpLoopMerge %103 %102 None
                 OpBranch %100

        %100 =     OpLabel
        %104 =       OpLoad %int %x
        %106 =       OpSLessThanEqual %bool %104 %int_1
                     OpBranchConditional %106 %101 %103

        %101 =         OpLabel
                         OpBranch %103

        %102 =   OpLabel
        %107 =     OpLoad %int %x
        %108 =     OpIAdd %int %107 %int_1
                   OpStore %x %108
                   OpBranch %99

        %103 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %109        ; RelaxedPrecision
        %110 = OpFunctionParameter %_ptr_Function_v2float

        %111 = OpLabel
        %141 =   OpVariable %_ptr_Function_v4float Function
        %112 =   OpFunctionCall %bool %test_return_b
                 OpSelectionMerge %114 None
                 OpBranchConditional %112 %113 %114

        %113 =     OpLabel
        %115 =       OpFunctionCall %bool %test_break_b
                     OpBranch %114

        %114 = OpLabel
        %116 =   OpPhi %bool %false %111 %115 %113
                 OpSelectionMerge %118 None
                 OpBranchConditional %116 %117 %118

        %117 =     OpLabel
        %119 =       OpFunctionCall %bool %test_continue_b
                     OpBranch %118

        %118 = OpLabel
        %120 =   OpPhi %bool %false %114 %119 %117
                 OpSelectionMerge %122 None
                 OpBranchConditional %120 %121 %122

        %121 =     OpLabel
        %123 =       OpFunctionCall %bool %test_if_return_b
                     OpBranch %122

        %122 = OpLabel
        %124 =   OpPhi %bool %false %118 %123 %121
                 OpSelectionMerge %126 None
                 OpBranchConditional %124 %125 %126

        %125 =     OpLabel
        %127 =       OpFunctionCall %bool %test_if_break_b
                     OpBranch %126

        %126 = OpLabel
        %128 =   OpPhi %bool %false %122 %127 %125
                 OpSelectionMerge %130 None
                 OpBranchConditional %128 %129 %130

        %129 =     OpLabel
        %131 =       OpFunctionCall %bool %test_else_b
                     OpBranch %130

        %130 = OpLabel
        %132 =   OpPhi %bool %false %126 %131 %129
                 OpSelectionMerge %134 None
                 OpBranchConditional %132 %133 %134

        %133 =     OpLabel
        %135 =       OpFunctionCall %bool %test_loop_return_b
                     OpBranch %134

        %134 = OpLabel
        %136 =   OpPhi %bool %false %130 %135 %133
                 OpSelectionMerge %138 None
                 OpBranchConditional %136 %137 %138

        %137 =     OpLabel
        %139 =       OpFunctionCall %bool %test_loop_break_b
                     OpBranch %138

        %138 = OpLabel
        %140 =   OpPhi %bool %false %134 %139 %137
                 OpSelectionMerge %145 None
                 OpBranchConditional %140 %143 %144

        %143 =     OpLabel
        %146 =       OpAccessChain %_ptr_Uniform_v4float %19 %int_0
        %147 =       OpLoad %v4float %146           ; RelaxedPrecision
                     OpStore %141 %147
                     OpBranch %145

        %144 =     OpLabel
        %148 =       OpAccessChain %_ptr_Uniform_v4float %19 %int_1
        %149 =       OpLoad %v4float %148           ; RelaxedPrecision
                     OpStore %141 %149
                     OpBranch %145

        %145 = OpLabel
        %150 =   OpLoad %v4float %141               ; RelaxedPrecision
                 OpReturnValue %150
               OpFunctionEnd
