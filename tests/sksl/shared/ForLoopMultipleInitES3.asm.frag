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
               OpName %sumA "sumA"                      ; id %27
               OpName %sumB "sumB"                      ; id %29
               OpName %a "a"                            ; id %30
               OpName %b "b"                            ; id %31
               OpName %sumC "sumC"                      ; id %74
               OpName %c "c"                            ; id %77
               OpName %sumE "sumE"                      ; id %98
               OpName %d "d"                            ; id %99
               OpName %e "e"                            ; id %104

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
               OpDecorate %sumA RelaxedPrecision
               OpDecorate %sumB RelaxedPrecision
               OpDecorate %a RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %_arr_float_int_2 ArrayStride 16
               OpDecorate %_arr_float_int_4 ArrayStride 16
               OpDecorate %134 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision

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
%_ptr_Function_float = OpTypePointer Function %float
   %float_10 = OpConstant %float 10
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
    %float_1 = OpConstant %float 1
       %true = OpConstantTrue %bool
   %float_45 = OpConstant %float 45
   %float_55 = OpConstant %float 55
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
     %int_10 = OpConstant %int 10
     %int_45 = OpConstant %int 45
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2       ; ArrayStride 16
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
      %int_4 = OpConstant %int 4
%_arr_float_int_4 = OpTypeArray %float %int_4       ; ArrayStride 16
%_ptr_Function__arr_float_int_4 = OpTypePointer Function %_arr_float_int_4
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4


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
       %sumA =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
       %sumB =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
          %a =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
          %b =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
       %sumC =   OpVariable %_ptr_Function_int Function
          %c =   OpVariable %_ptr_Function_int Function
       %sumE =   OpVariable %_ptr_Function_float Function
          %d =   OpVariable %_ptr_Function__arr_float_int_2 Function
          %e =   OpVariable %_ptr_Function__arr_float_int_4 Function
                 OpStore %sumA %float_0
                 OpStore %sumB %float_0
                 OpStore %a %float_0
                 OpStore %b %float_10
                 OpBranch %33

         %33 = OpLabel
                 OpLoopMerge %37 %36 None
                 OpBranch %34

         %34 =     OpLabel
         %40 =       OpLoad %float %a               ; RelaxedPrecision
         %41 =       OpFOrdLessThan %bool %40 %float_10
                     OpSelectionMerge %43 None
                     OpBranchConditional %41 %42 %43

         %42 =         OpLabel
         %44 =           OpLoad %float %b           ; RelaxedPrecision
         %45 =           OpFOrdGreaterThan %bool %44 %float_0
                         OpBranch %43

         %43 =     OpLabel
         %46 =       OpPhi %bool %false %34 %45 %42
                     OpBranchConditional %46 %35 %37

         %35 =         OpLabel
         %47 =           OpLoad %float %sumA        ; RelaxedPrecision
         %48 =           OpLoad %float %a           ; RelaxedPrecision
         %49 =           OpFAdd %float %47 %48      ; RelaxedPrecision
                         OpStore %sumA %49
         %50 =           OpLoad %float %sumB        ; RelaxedPrecision
         %51 =           OpLoad %float %b           ; RelaxedPrecision
         %52 =           OpFAdd %float %50 %51      ; RelaxedPrecision
                         OpStore %sumB %52
                         OpBranch %36

         %36 =   OpLabel
         %54 =     OpLoad %float %a                 ; RelaxedPrecision
         %55 =     OpFAdd %float %54 %float_1       ; RelaxedPrecision
                   OpStore %a %55
         %56 =     OpLoad %float %b                 ; RelaxedPrecision
         %57 =     OpFSub %float %56 %float_1       ; RelaxedPrecision
                   OpStore %b %57
                   OpBranch %33

         %37 = OpLabel
         %59 =   OpLoad %float %sumA                ; RelaxedPrecision
         %61 =   OpFUnordNotEqual %bool %59 %float_45
                 OpSelectionMerge %63 None
                 OpBranchConditional %61 %63 %62

         %62 =     OpLabel
         %64 =       OpLoad %float %sumB            ; RelaxedPrecision
         %66 =       OpFUnordNotEqual %bool %64 %float_55
                     OpBranch %63

         %63 = OpLabel
         %67 =   OpPhi %bool %true %37 %66 %62
                 OpSelectionMerge %69 None
                 OpBranchConditional %67 %68 %69

         %68 =     OpLabel
         %70 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %73 =       OpLoad %v4float %70            ; RelaxedPrecision
                     OpReturnValue %73

         %69 = OpLabel
                 OpStore %sumC %int_0
                 OpStore %c %int_0
                 OpBranch %78

         %78 = OpLabel
                 OpLoopMerge %82 %81 None
                 OpBranch %79

         %79 =     OpLabel
         %83 =       OpLoad %int %c
         %85 =       OpSLessThan %bool %83 %int_10
                     OpBranchConditional %85 %80 %82

         %80 =         OpLabel
         %86 =           OpLoad %int %sumC
         %87 =           OpLoad %int %c
         %88 =           OpIAdd %int %86 %87
                         OpStore %sumC %88
                         OpBranch %81

         %81 =   OpLabel
         %89 =     OpLoad %int %c
         %90 =     OpIAdd %int %89 %int_1
                   OpStore %c %90
                   OpBranch %78

         %82 = OpLabel
         %91 =   OpLoad %int %sumC
         %93 =   OpINotEqual %bool %91 %int_45
                 OpSelectionMerge %95 None
                 OpBranchConditional %93 %94 %95

         %94 =     OpLabel
         %96 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %97 =       OpLoad %v4float %96            ; RelaxedPrecision
                     OpReturnValue %97

         %95 = OpLabel
                 OpStore %sumE %float_0
        %103 =   OpCompositeConstruct %_arr_float_int_2 %float_0 %float_10
                 OpStore %d %103
        %111 =   OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
                 OpStore %e %111
                 OpBranch %112

        %112 = OpLabel
                 OpLoopMerge %116 %115 None
                 OpBranch %113

        %113 =     OpLabel
        %117 =       OpAccessChain %_ptr_Function_float %d %int_0
        %118 =       OpLoad %float %117
        %119 =       OpAccessChain %_ptr_Function_float %d %int_1
        %120 =       OpLoad %float %119
        %121 =       OpFOrdLessThan %bool %118 %120
                     OpBranchConditional %121 %114 %116

        %114 =         OpLabel
        %122 =           OpLoad %float %sumE
        %123 =           OpAccessChain %_ptr_Function_float %e %int_0
        %124 =           OpLoad %float %123
        %125 =           OpFAdd %float %122 %124
                         OpStore %sumE %125
                         OpBranch %115

        %115 =   OpLabel
        %126 =     OpAccessChain %_ptr_Function_float %d %int_0
        %127 =     OpLoad %float %126
        %128 =     OpFAdd %float %127 %float_1
                   OpStore %126 %128
                   OpBranch %112

        %116 = OpLabel
        %129 =   OpLoad %float %sumE
        %130 =   OpFUnordNotEqual %bool %129 %float_10
                 OpSelectionMerge %132 None
                 OpBranchConditional %130 %131 %132

        %131 =     OpLabel
        %133 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %134 =       OpLoad %v4float %133           ; RelaxedPrecision
                     OpReturnValue %134

        %132 = OpLabel
                 OpBranch %135

        %135 = OpLabel
                 OpLoopMerge %139 %138 None
                 OpBranch %136

        %136 =     OpLabel
                     OpBranch %137

        %137 =     OpLabel
                     OpBranch %139

        %138 =   OpLabel
                   OpBranch %135

        %139 = OpLabel
                 OpBranch %140

        %140 = OpLabel
                 OpLoopMerge %144 %143 None
                 OpBranch %141

        %141 =     OpLabel
                     OpBranch %142

        %142 =     OpLabel
        %145 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %146 =       OpLoad %v4float %145           ; RelaxedPrecision
                     OpReturnValue %146

        %143 =   OpLabel
                   OpBranch %140

        %144 = OpLabel
                 OpUnreachable
               OpFunctionEnd
