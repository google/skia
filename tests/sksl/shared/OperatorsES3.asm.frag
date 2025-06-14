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
               OpName %b "b"                            ; id %51
               OpName %c "c"                            ; id %71
               OpName %d "d"                            ; id %75
               OpName %e "e"                            ; id %77
               OpName %f "f"                            ; id %81
               OpName %w "w"                            ; id %119

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
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %148 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision

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
      %int_1 = OpConstant %int 1
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
    %float_4 = OpConstant %float 4
      %false = OpConstantFalse %bool
%_ptr_Uniform_float = OpTypePointer Uniform %float
   %float_12 = OpConstant %float 12
%float_0_100000001 = OpConstant %float 0.100000001
      %int_0 = OpConstant %int 0
     %int_n1 = OpConstant %int -1
      %int_5 = OpConstant %int 5
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_6 = OpConstant %int 6
    %float_6 = OpConstant %float 6
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
     %int_n6 = OpConstant %int -6
        %123 = OpConstantComposite %v2int %int_n6 %int_n6
%_ptr_Function_v4float = OpTypePointer Function %v4float


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
          %w =   OpVariable %_ptr_Function_v2int Function
        %142 =   OpVariable %_ptr_Function_v4float Function
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
         %45 =   OpSMod %int %44 %int_3
         %47 =   OpShiftLeftLogical %int %45 %int_4
         %48 =   OpShiftRightArithmetic %int %47 %int_2
         %50 =   OpShiftLeftLogical %int %48 %int_1
                 OpStore %z %50
         %56 =   OpFOrdGreaterThan %bool %40 %float_4
         %57 =   OpFOrdLessThan %bool %40 %float_2
         %58 =   OpLogicalEqual %bool %56 %57
                 OpSelectionMerge %60 None
                 OpBranchConditional %58 %60 %59

         %59 =     OpLabel
         %62 =       OpAccessChain %_ptr_Uniform_float %11 %int_2
         %64 =       OpLoad %float %62
         %65 =       OpFOrdGreaterThanEqual %bool %float_2 %64
                     OpSelectionMerge %67 None
                     OpBranchConditional %65 %66 %67

         %66 =         OpLabel
         %68 =           OpFOrdLessThanEqual %bool %42 %40
                         OpBranch %67

         %67 =     OpLabel
         %69 =       OpPhi %bool %false %59 %68 %66
                     OpBranch %60

         %60 = OpLabel
         %70 =   OpPhi %bool %true %26 %69 %67
                 OpStore %b %70
         %72 =   OpAccessChain %_ptr_Uniform_float %11 %int_2
         %73 =   OpLoad %float %72
         %74 =   OpFOrdGreaterThan %bool %73 %float_2
                 OpStore %c %74
         %76 =   OpLogicalNotEqual %bool %70 %74
                 OpStore %d %76
                 OpSelectionMerge %79 None
                 OpBranchConditional %70 %78 %79

         %78 =     OpLabel
                     OpBranch %79

         %79 = OpLabel
         %80 =   OpPhi %bool %false %60 %74 %78
                 OpStore %e %80
                 OpSelectionMerge %83 None
                 OpBranchConditional %70 %83 %82

         %82 =     OpLabel
                     OpBranch %83

         %83 = OpLabel
         %84 =   OpPhi %bool %true %79 %74 %82
                 OpStore %f %84
         %86 =   OpFAdd %float %40 %float_12
                 OpStore %x %86
         %87 =   OpFSub %float %86 %float_12
                 OpStore %x %87
         %89 =   OpFMul %float %42 %float_0_100000001
                 OpStore %y %89
         %90 =   OpFMul %float %87 %89
                 OpStore %x %90
         %92 =   OpBitwiseOr %int %50 %int_0
                 OpStore %z %92
         %94 =   OpBitwiseAnd %int %92 %int_n1
                 OpStore %z %94
         %95 =   OpBitwiseXor %int %94 %int_0
                 OpStore %z %95
         %96 =   OpShiftRightArithmetic %int %95 %int_2
                 OpStore %z %96
         %97 =   OpShiftLeftLogical %int %96 %int_4
                 OpStore %z %97
         %99 =   OpSMod %int %97 %int_5
                 OpStore %z %99
        %100 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %102 =   OpLoad %v4float %100               ; RelaxedPrecision
        %103 =   OpVectorShuffle %v2float %102 %102 0 1     ; RelaxedPrecision
        %105 =   OpConvertSToF %float %int_6
                 OpStore %x %105
        %106 =   OpSelect %float %70 %float_1 %float_0
        %107 =   OpSelect %float %74 %float_1 %float_0
        %108 =   OpFMul %float %106 %107
        %109 =   OpSelect %float %76 %float_1 %float_0
        %110 =   OpFMul %float %108 %109
        %111 =   OpSelect %float %80 %float_1 %float_0
        %112 =   OpFMul %float %110 %111
        %113 =   OpSelect %float %84 %float_1 %float_0
        %114 =   OpFMul %float %112 %113
                 OpStore %y %float_6
        %116 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %117 =   OpLoad %v4float %116               ; RelaxedPrecision
        %118 =   OpVectorShuffle %v2float %117 %117 2 3     ; RelaxedPrecision
                 OpStore %z %int_6
                 OpStore %w %123
        %124 =   OpNot %v2int %123
                 OpStore %w %124
        %125 =   OpCompositeExtract %int %124 0
        %126 =   OpIEqual %bool %125 %int_5
                 OpSelectionMerge %128 None
                 OpBranchConditional %126 %127 %128

        %127 =     OpLabel
        %129 =       OpCompositeExtract %int %124 1
        %130 =       OpIEqual %bool %129 %int_5
                     OpBranch %128

        %128 = OpLabel
        %131 =   OpPhi %bool %false %83 %130 %127
                 OpSelectionMerge %133 None
                 OpBranchConditional %131 %132 %133

        %132 =     OpLabel
        %134 =       OpFOrdEqual %bool %105 %float_6
                     OpBranch %133

        %133 = OpLabel
        %135 =   OpPhi %bool %false %128 %134 %132
                 OpSelectionMerge %137 None
                 OpBranchConditional %135 %136 %137

        %136 =     OpLabel
                     OpBranch %137

        %137 = OpLabel
        %138 =   OpPhi %bool %false %133 %true %136
                 OpSelectionMerge %140 None
                 OpBranchConditional %138 %139 %140

        %139 =     OpLabel
                     OpBranch %140

        %140 = OpLabel
        %141 =   OpPhi %bool %false %137 %true %139
                 OpSelectionMerge %146 None
                 OpBranchConditional %141 %144 %145

        %144 =     OpLabel
        %147 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %148 =       OpLoad %v4float %147           ; RelaxedPrecision
                     OpStore %142 %148
                     OpBranch %146

        %145 =     OpLabel
        %149 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %150 =       OpLoad %v4float %149           ; RelaxedPrecision
                     OpStore %142 %150
                     OpBranch %146

        %146 = OpLabel
        %151 =   OpLoad %v4float %142               ; RelaxedPrecision
                 OpReturnValue %151
               OpFunctionEnd
