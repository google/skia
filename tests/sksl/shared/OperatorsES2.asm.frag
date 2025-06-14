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
               OpMemberName %_UniformBuffer 2 "unknownInput"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %x "x"                            ; id %27
               OpName %y "y"                            ; id %30
               OpName %z "z"                            ; id %32
               OpName %b "b"                            ; id %49
               OpName %c "c"                            ; id %69
               OpName %d "d"                            ; id %73
               OpName %e "e"                            ; id %75
               OpName %f "f"                            ; id %79

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
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %62 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float %float     ; Block
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
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
%_ptr_Function_int = OpTypePointer Function %int
      %int_3 = OpConstant %int 3
      %int_2 = OpConstant %int 2
      %int_4 = OpConstant %int 4
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
    %float_4 = OpConstant %float 4
      %false = OpConstantFalse %bool
%_ptr_Uniform_float = OpTypePointer Uniform %float
   %float_12 = OpConstant %float 12
%float_0_100000001 = OpConstant %float 0.100000001
    %float_6 = OpConstant %float 6
      %int_1 = OpConstant %int 1
      %int_6 = OpConstant %int 6
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
          %x =   OpVariable %_ptr_Function_float Function
          %y =   OpVariable %_ptr_Function_float Function
          %z =   OpVariable %_ptr_Function_int Function
          %b =   OpVariable %_ptr_Function_bool Function
          %c =   OpVariable %_ptr_Function_bool Function
          %d =   OpVariable %_ptr_Function_bool Function
          %e =   OpVariable %_ptr_Function_bool Function
          %f =   OpVariable %_ptr_Function_bool Function
        %108 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %x %float_1
                 OpStore %y %float_2
                 OpStore %z %int_3
         %35 =   OpFSub %float %float_1 %float_1
         %36 =   OpFMul %float %float_2 %float_1
         %37 =   OpFMul %float %36 %float_1
         %38 =   OpFSub %float %float_2 %float_1
         %39 =   OpFMul %float %37 %38
         %40 =   OpFAdd %float %35 %39
                 OpStore %x %40
         %41 =   OpFDiv %float %40 %float_2
         %42 =   OpFDiv %float %41 %40
                 OpStore %y %42
         %44 =   OpSDiv %int %int_3 %int_2
         %45 =   OpIMul %int %44 %int_3
         %47 =   OpIAdd %int %45 %int_4
         %48 =   OpISub %int %47 %int_2
                 OpStore %z %48
         %54 =   OpFOrdGreaterThan %bool %40 %float_4
         %55 =   OpFOrdLessThan %bool %40 %float_2
         %56 =   OpLogicalEqual %bool %54 %55
                 OpSelectionMerge %58 None
                 OpBranchConditional %56 %58 %57

         %57 =     OpLabel
         %60 =       OpAccessChain %_ptr_Uniform_float %11 %int_2
         %62 =       OpLoad %float %60              ; RelaxedPrecision
         %63 =       OpFOrdGreaterThanEqual %bool %float_2 %62
                     OpSelectionMerge %65 None
                     OpBranchConditional %63 %64 %65

         %64 =         OpLabel
         %66 =           OpFOrdLessThanEqual %bool %42 %40
                         OpBranch %65

         %65 =     OpLabel
         %67 =       OpPhi %bool %false %57 %66 %64
                     OpBranch %58

         %58 = OpLabel
         %68 =   OpPhi %bool %true %26 %67 %65
                 OpStore %b %68
         %70 =   OpAccessChain %_ptr_Uniform_float %11 %int_2
         %71 =   OpLoad %float %70                  ; RelaxedPrecision
         %72 =   OpFOrdGreaterThan %bool %71 %float_2
                 OpStore %c %72
         %74 =   OpLogicalNotEqual %bool %68 %72
                 OpStore %d %74
                 OpSelectionMerge %77 None
                 OpBranchConditional %68 %76 %77

         %76 =     OpLabel
                     OpBranch %77

         %77 = OpLabel
         %78 =   OpPhi %bool %false %58 %72 %76
                 OpStore %e %78
                 OpSelectionMerge %81 None
                 OpBranchConditional %68 %81 %80

         %80 =     OpLabel
                     OpBranch %81

         %81 = OpLabel
         %82 =   OpPhi %bool %true %77 %72 %80
                 OpStore %f %82
         %84 =   OpFAdd %float %40 %float_12
                 OpStore %x %84
         %85 =   OpFSub %float %84 %float_12
                 OpStore %x %85
         %87 =   OpFMul %float %42 %float_0_100000001
                 OpStore %y %87
         %88 =   OpFMul %float %85 %87
                 OpStore %x %88
                 OpStore %x %float_6
         %90 =   OpSelect %float %68 %float_1 %float_0
         %91 =   OpSelect %float %72 %float_1 %float_0
         %92 =   OpFMul %float %90 %91
         %93 =   OpSelect %float %74 %float_1 %float_0
         %94 =   OpFMul %float %92 %93
         %95 =   OpSelect %float %78 %float_1 %float_0
         %96 =   OpFMul %float %94 %95
         %97 =   OpSelect %float %82 %float_1 %float_0
         %98 =   OpFMul %float %96 %97
                 OpStore %y %98
                 OpStore %y %float_6
        %100 =   OpISub %int %48 %int_1
                 OpStore %z %100
                 OpStore %z %int_6
                 OpSelectionMerge %103 None
                 OpBranchConditional %true %102 %103

        %102 =     OpLabel
                     OpBranch %103

        %103 = OpLabel
        %104 =   OpPhi %bool %false %81 %true %102
                 OpSelectionMerge %106 None
                 OpBranchConditional %104 %105 %106

        %105 =     OpLabel
                     OpBranch %106

        %106 = OpLabel
        %107 =   OpPhi %bool %false %103 %true %105
                 OpSelectionMerge %112 None
                 OpBranchConditional %107 %110 %111

        %110 =     OpLabel
        %113 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %116 =       OpLoad %v4float %113           ; RelaxedPrecision
                     OpStore %108 %116
                     OpBranch %112

        %111 =     OpLabel
        %117 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %118 =       OpLoad %v4float %117           ; RelaxedPrecision
                     OpStore %108 %118
                     OpBranch %112

        %112 = OpLabel
        %119 =   OpLoad %v4float %108               ; RelaxedPrecision
                 OpReturnValue %119
               OpFunctionEnd
