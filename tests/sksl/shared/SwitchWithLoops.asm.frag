               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %9
               OpName %_UniformBuffer "_UniformBuffer"  ; id %14
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %16
               OpName %switch_with_continue_in_loop_bi "switch_with_continue_in_loop_bi"    ; id %6
               OpName %val "val"                                                            ; id %31
               OpName %i "i"                                                                ; id %37
               OpName %loop_with_break_in_switch_bi "loop_with_break_in_switch_bi"          ; id %7
               OpName %val_0 "val"                                                          ; id %58
               OpName %i_0 "i"                                                              ; id %59
               OpName %main "main"                                                          ; id %8
               OpName %x "x"                                                                ; id %84
               OpName %_0_val "_0_val"                                                      ; id %90
               OpName %_1_i "_1_i"                                                          ; id %94

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
               OpDecorate %13 Binding 0
               OpDecorate %13 DescriptorSet 0
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
%_ptr_Function_int = OpTypePointer Function %int
         %28 = OpTypeFunction %bool %_ptr_Function_int
      %int_0 = OpConstant %int 0
     %int_10 = OpConstant %int 10
      %int_1 = OpConstant %int 1
     %int_11 = OpConstant %int 11
      %false = OpConstantFalse %bool
     %int_20 = OpConstant %int 20
         %81 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_2 = OpConstant %int 2
%_ptr_Function_v4float = OpTypePointer Function %v4float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %18

         %19 = OpLabel
         %23 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %23 %22
         %25 =   OpFunctionCall %v4float %main %23
                 OpStore %sk_FragColor %25
                 OpReturn
               OpFunctionEnd


               ; Function switch_with_continue_in_loop_bi
%switch_with_continue_in_loop_bi = OpFunction %bool None %28
         %29 = OpFunctionParameter %_ptr_Function_int

         %30 = OpLabel
        %val =   OpVariable %_ptr_Function_int Function
          %i =   OpVariable %_ptr_Function_int Function
                 OpStore %val %int_0
         %33 =   OpLoad %int %29
                 OpSelectionMerge %34 None
                 OpSwitch %33 %36 1 %35

         %35 =     OpLabel
                     OpStore %i %int_0
                     OpBranch %38

         %38 =     OpLabel
                     OpLoopMerge %42 %41 None
                     OpBranch %39

         %39 =         OpLabel
         %43 =           OpLoad %int %i
         %45 =           OpSLessThan %bool %43 %int_10
                         OpBranchConditional %45 %40 %42

         %40 =             OpLabel
         %47 =               OpLoad %int %val
         %48 =               OpIAdd %int %47 %int_1
                             OpStore %val %48
                             OpBranch %41

         %41 =       OpLabel
         %49 =         OpLoad %int %i
         %50 =         OpIAdd %int %49 %int_1
                       OpStore %i %50
                       OpBranch %38

         %42 =     OpLabel
                     OpBranch %36

         %36 =     OpLabel
         %51 =       OpLoad %int %val
         %52 =       OpIAdd %int %51 %int_1
                     OpStore %val %52
                     OpBranch %34

         %34 = OpLabel
         %53 =   OpLoad %int %val
         %55 =   OpIEqual %bool %53 %int_11
                 OpReturnValue %55
               OpFunctionEnd


               ; Function loop_with_break_in_switch_bi
%loop_with_break_in_switch_bi = OpFunction %bool None %28
         %56 = OpFunctionParameter %_ptr_Function_int

         %57 = OpLabel
      %val_0 =   OpVariable %_ptr_Function_int Function
        %i_0 =   OpVariable %_ptr_Function_int Function
                 OpStore %val_0 %int_0
                 OpStore %i_0 %int_0
                 OpBranch %60

         %60 = OpLabel
                 OpLoopMerge %64 %63 None
                 OpBranch %61

         %61 =     OpLabel
         %65 =       OpLoad %int %i_0
         %66 =       OpSLessThan %bool %65 %int_10
                     OpBranchConditional %66 %62 %64

         %62 =         OpLabel
         %67 =           OpLoad %int %56
                         OpSelectionMerge %68 None
                         OpSwitch %67 %70 1 %69

         %69 =             OpLabel
         %71 =               OpLoad %int %val_0
         %72 =               OpIAdd %int %71 %int_1
                             OpStore %val_0 %72
                             OpBranch %68

         %70 =             OpLabel
                             OpReturnValue %false

         %68 =         OpLabel
         %74 =           OpLoad %int %val_0
         %75 =           OpIAdd %int %74 %int_1
                         OpStore %val_0 %75
                         OpBranch %63

         %63 =   OpLabel
         %76 =     OpLoad %int %i_0
         %77 =     OpIAdd %int %76 %int_1
                   OpStore %i_0 %77
                   OpBranch %60

         %64 = OpLabel
         %78 =   OpLoad %int %val_0
         %80 =   OpIEqual %bool %78 %int_20
                 OpReturnValue %80
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %81         ; RelaxedPrecision
         %82 = OpFunctionParameter %_ptr_Function_v2float

         %83 = OpLabel
          %x =   OpVariable %_ptr_Function_int Function
     %_0_val =   OpVariable %_ptr_Function_int Function
       %_1_i =   OpVariable %_ptr_Function_int Function
        %114 =   OpVariable %_ptr_Function_int Function
        %120 =   OpVariable %_ptr_Function_int Function
        %123 =   OpVariable %_ptr_Function_v4float Function
         %85 =   OpAccessChain %_ptr_Uniform_v4float %13 %int_0
         %87 =   OpLoad %v4float %85                ; RelaxedPrecision
         %88 =   OpCompositeExtract %float %87 1    ; RelaxedPrecision
         %89 =   OpConvertFToS %int %88
                 OpStore %x %89
                 OpStore %_0_val %int_0
                 OpSelectionMerge %91 None
                 OpSwitch %89 %93 1 %92

         %92 =     OpLabel
                     OpStore %_1_i %int_0
                     OpBranch %95

         %95 =     OpLabel
                     OpLoopMerge %99 %98 None
                     OpBranch %96

         %96 =         OpLabel
        %100 =           OpLoad %int %_1_i
        %101 =           OpSLessThan %bool %100 %int_10
                         OpBranchConditional %101 %97 %99

         %97 =             OpLabel
        %102 =               OpLoad %int %_0_val
        %103 =               OpIAdd %int %102 %int_1
                             OpStore %_0_val %103
                             OpBranch %99

         %98 =       OpLabel
        %104 =         OpLoad %int %_1_i
        %105 =         OpIAdd %int %104 %int_1
                       OpStore %_1_i %105
                       OpBranch %95

         %99 =     OpLabel
                     OpBranch %93

         %93 =     OpLabel
        %106 =       OpLoad %int %_0_val
        %107 =       OpIAdd %int %106 %int_1
                     OpStore %_0_val %107
                     OpBranch %91

         %91 = OpLabel
        %108 =   OpLoad %int %_0_val
        %110 =   OpIEqual %bool %108 %int_2
                 OpSelectionMerge %112 None
                 OpBranchConditional %110 %111 %112

        %111 =     OpLabel
        %113 =       OpLoad %int %x
                     OpStore %114 %113
        %115 =       OpFunctionCall %bool %switch_with_continue_in_loop_bi %114
                     OpBranch %112

        %112 = OpLabel
        %116 =   OpPhi %bool %false %91 %115 %111
                 OpSelectionMerge %118 None
                 OpBranchConditional %116 %117 %118

        %117 =     OpLabel
        %119 =       OpLoad %int %x
                     OpStore %120 %119
        %121 =       OpFunctionCall %bool %loop_with_break_in_switch_bi %120
                     OpBranch %118

        %118 = OpLabel
        %122 =   OpPhi %bool %false %112 %121 %117
                 OpSelectionMerge %127 None
                 OpBranchConditional %122 %125 %126

        %125 =     OpLabel
        %128 =       OpAccessChain %_ptr_Uniform_v4float %13 %int_0
        %129 =       OpLoad %v4float %128           ; RelaxedPrecision
                     OpStore %123 %129
                     OpBranch %127

        %126 =     OpLabel
        %130 =       OpAccessChain %_ptr_Uniform_v4float %13 %int_1
        %131 =       OpLoad %v4float %130           ; RelaxedPrecision
                     OpStore %123 %131
                     OpBranch %127

        %127 = OpLabel
        %132 =   OpLoad %v4float %123               ; RelaxedPrecision
                 OpReturnValue %132
               OpFunctionEnd
