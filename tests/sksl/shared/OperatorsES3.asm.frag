               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "unknownInput"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %x "x"
               OpName %y "y"
               OpName %z "z"
               OpName %b "b"
               OpName %c "c"
               OpName %d "d"
               OpName %e "e"
               OpName %f "f"
               OpName %w "w"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_3 = OpConstant %int 3
      %int_2 = OpConstant %int 2
      %int_4 = OpConstant %int 4
      %int_1 = OpConstant %int 1
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %15
         %16 = OpLabel
         %20 = OpVariable %_ptr_Function_v2float Function
               OpStore %20 %19
         %22 = OpFunctionCall %v4float %main %20
               OpStore %sk_FragColor %22
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %23
         %24 = OpFunctionParameter %_ptr_Function_v2float
         %25 = OpLabel
          %x = OpVariable %_ptr_Function_float Function
          %y = OpVariable %_ptr_Function_float Function
          %z = OpVariable %_ptr_Function_int Function
          %b = OpVariable %_ptr_Function_bool Function
          %c = OpVariable %_ptr_Function_bool Function
          %d = OpVariable %_ptr_Function_bool Function
          %e = OpVariable %_ptr_Function_bool Function
          %f = OpVariable %_ptr_Function_bool Function
          %w = OpVariable %_ptr_Function_v2int Function
        %141 = OpVariable %_ptr_Function_v4float Function
               OpStore %x %float_1
               OpStore %y %float_2
               OpStore %z %int_3
         %35 = OpFSub %float %float_1 %float_1
         %36 = OpFMul %float %float_2 %float_1
         %37 = OpFMul %float %36 %float_1
         %38 = OpFSub %float %float_2 %float_1
         %39 = OpFMul %float %37 %38
         %40 = OpFAdd %float %35 %39
               OpStore %x %40
         %41 = OpFDiv %float %40 %float_2
         %42 = OpFDiv %float %41 %40
               OpStore %y %42
         %44 = OpSDiv %int %int_3 %int_2
         %45 = OpSMod %int %44 %int_3
         %47 = OpShiftLeftLogical %int %45 %int_4
         %48 = OpShiftRightArithmetic %int %47 %int_2
         %50 = OpShiftLeftLogical %int %48 %int_1
               OpStore %z %50
         %55 = OpFOrdGreaterThan %bool %40 %float_4
         %56 = OpFOrdLessThan %bool %40 %float_2
         %57 = OpLogicalEqual %bool %55 %56
               OpSelectionMerge %59 None
               OpBranchConditional %57 %59 %58
         %58 = OpLabel
         %61 = OpAccessChain %_ptr_Uniform_float %10 %int_2
         %63 = OpLoad %float %61
         %64 = OpFOrdGreaterThanEqual %bool %float_2 %63
               OpSelectionMerge %66 None
               OpBranchConditional %64 %65 %66
         %65 = OpLabel
         %67 = OpFOrdLessThanEqual %bool %42 %40
               OpBranch %66
         %66 = OpLabel
         %68 = OpPhi %bool %false %58 %67 %65
               OpBranch %59
         %59 = OpLabel
         %69 = OpPhi %bool %true %25 %68 %66
               OpStore %b %69
         %71 = OpAccessChain %_ptr_Uniform_float %10 %int_2
         %72 = OpLoad %float %71
         %73 = OpFOrdGreaterThan %bool %72 %float_2
               OpStore %c %73
         %75 = OpLogicalNotEqual %bool %69 %73
               OpStore %d %75
               OpSelectionMerge %78 None
               OpBranchConditional %69 %77 %78
         %77 = OpLabel
               OpBranch %78
         %78 = OpLabel
         %79 = OpPhi %bool %false %59 %73 %77
               OpStore %e %79
               OpSelectionMerge %82 None
               OpBranchConditional %69 %82 %81
         %81 = OpLabel
               OpBranch %82
         %82 = OpLabel
         %83 = OpPhi %bool %true %78 %73 %81
               OpStore %f %83
         %85 = OpFAdd %float %40 %float_12
               OpStore %x %85
         %86 = OpFSub %float %85 %float_12
               OpStore %x %86
         %88 = OpFMul %float %42 %float_0_100000001
               OpStore %y %88
         %89 = OpFMul %float %86 %88
               OpStore %x %89
         %91 = OpBitwiseOr %int %50 %int_0
               OpStore %z %91
         %93 = OpBitwiseAnd %int %91 %int_n1
               OpStore %z %93
         %94 = OpBitwiseXor %int %93 %int_0
               OpStore %z %94
         %95 = OpShiftRightArithmetic %int %94 %int_2
               OpStore %z %95
         %96 = OpShiftLeftLogical %int %95 %int_4
               OpStore %z %96
         %98 = OpSMod %int %96 %int_5
               OpStore %z %98
         %99 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %101 = OpLoad %v4float %99
        %102 = OpVectorShuffle %v2float %101 %101 0 1
        %104 = OpConvertSToF %float %int_6
               OpStore %x %104
        %105 = OpSelect %float %69 %float_1 %float_0
        %106 = OpSelect %float %73 %float_1 %float_0
        %107 = OpFMul %float %105 %106
        %108 = OpSelect %float %75 %float_1 %float_0
        %109 = OpFMul %float %107 %108
        %110 = OpSelect %float %79 %float_1 %float_0
        %111 = OpFMul %float %109 %110
        %112 = OpSelect %float %83 %float_1 %float_0
        %113 = OpFMul %float %111 %112
               OpStore %y %float_6
        %115 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %116 = OpLoad %v4float %115
        %117 = OpVectorShuffle %v2float %116 %116 2 3
               OpStore %z %int_6
        %121 = OpNot %int %int_5
        %122 = OpCompositeConstruct %v2int %121 %121
               OpStore %w %122
        %123 = OpNot %v2int %122
               OpStore %w %123
        %124 = OpCompositeExtract %int %123 0
        %125 = OpIEqual %bool %124 %int_5
               OpSelectionMerge %127 None
               OpBranchConditional %125 %126 %127
        %126 = OpLabel
        %128 = OpCompositeExtract %int %123 1
        %129 = OpIEqual %bool %128 %int_5
               OpBranch %127
        %127 = OpLabel
        %130 = OpPhi %bool %false %82 %129 %126
               OpSelectionMerge %132 None
               OpBranchConditional %130 %131 %132
        %131 = OpLabel
        %133 = OpFOrdEqual %bool %104 %float_6
               OpBranch %132
        %132 = OpLabel
        %134 = OpPhi %bool %false %127 %133 %131
               OpSelectionMerge %136 None
               OpBranchConditional %134 %135 %136
        %135 = OpLabel
               OpBranch %136
        %136 = OpLabel
        %137 = OpPhi %bool %false %132 %true %135
               OpSelectionMerge %139 None
               OpBranchConditional %137 %138 %139
        %138 = OpLabel
               OpBranch %139
        %139 = OpLabel
        %140 = OpPhi %bool %false %136 %true %138
               OpSelectionMerge %145 None
               OpBranchConditional %140 %143 %144
        %143 = OpLabel
        %146 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %147 = OpLoad %v4float %146
               OpStore %141 %147
               OpBranch %145
        %144 = OpLabel
        %148 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %149 = OpLoad %v4float %148
               OpStore %141 %149
               OpBranch %145
        %145 = OpLabel
        %150 = OpLoad %v4float %141
               OpReturnValue %150
               OpFunctionEnd
