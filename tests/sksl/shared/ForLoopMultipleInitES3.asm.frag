               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %sumA "sumA"
               OpName %sumB "sumB"
               OpName %a "a"
               OpName %b "b"
               OpName %sumC "sumC"
               OpName %c "c"
               OpName %sumE "sumE"
               OpName %d "d"
               OpName %e "e"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %sumA RelaxedPrecision
               OpDecorate %sumB RelaxedPrecision
               OpDecorate %a RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %_arr_float_int_2 ArrayStride 16
               OpDecorate %_arr_float_int_4 ArrayStride 16
               OpDecorate %131 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
   %float_10 = OpConstant %float 10
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
    %float_1 = OpConstant %float 1
       %true = OpConstantTrue %bool
   %float_45 = OpConstant %float 45
   %float_55 = OpConstant %float 55
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
     %int_10 = OpConstant %int 10
     %int_45 = OpConstant %int 45
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
      %int_4 = OpConstant %int 4
%_arr_float_int_4 = OpTypeArray %float %int_4
%_ptr_Function__arr_float_int_4 = OpTypePointer Function %_arr_float_int_4
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
%_entrypoint_v = OpFunction %void None %12
         %13 = OpLabel
         %17 = OpVariable %_ptr_Function_v2float Function
               OpStore %17 %16
         %19 = OpFunctionCall %v4float %main %17
               OpStore %sk_FragColor %19
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %20
         %21 = OpFunctionParameter %_ptr_Function_v2float
         %22 = OpLabel
       %sumA = OpVariable %_ptr_Function_float Function
       %sumB = OpVariable %_ptr_Function_float Function
          %a = OpVariable %_ptr_Function_float Function
          %b = OpVariable %_ptr_Function_float Function
       %sumC = OpVariable %_ptr_Function_int Function
          %c = OpVariable %_ptr_Function_int Function
       %sumE = OpVariable %_ptr_Function_float Function
          %d = OpVariable %_ptr_Function__arr_float_int_2 Function
          %e = OpVariable %_ptr_Function__arr_float_int_4 Function
               OpStore %sumA %float_0
               OpStore %sumB %float_0
               OpStore %a %float_0
               OpStore %b %float_10
               OpBranch %29
         %29 = OpLabel
               OpLoopMerge %33 %32 None
               OpBranch %30
         %30 = OpLabel
         %36 = OpLoad %float %a
         %37 = OpFOrdLessThan %bool %36 %float_10
               OpSelectionMerge %39 None
               OpBranchConditional %37 %38 %39
         %38 = OpLabel
         %40 = OpLoad %float %b
         %41 = OpFOrdGreaterThan %bool %40 %float_0
               OpBranch %39
         %39 = OpLabel
         %42 = OpPhi %bool %false %30 %41 %38
               OpBranchConditional %42 %31 %33
         %31 = OpLabel
         %43 = OpLoad %float %sumA
         %44 = OpLoad %float %a
         %45 = OpFAdd %float %43 %44
               OpStore %sumA %45
         %46 = OpLoad %float %sumB
         %47 = OpLoad %float %b
         %48 = OpFAdd %float %46 %47
               OpStore %sumB %48
               OpBranch %32
         %32 = OpLabel
         %50 = OpLoad %float %a
         %51 = OpFAdd %float %50 %float_1
               OpStore %a %51
         %52 = OpLoad %float %b
         %53 = OpFSub %float %52 %float_1
               OpStore %b %53
               OpBranch %29
         %33 = OpLabel
         %55 = OpLoad %float %sumA
         %57 = OpFUnordNotEqual %bool %55 %float_45
               OpSelectionMerge %59 None
               OpBranchConditional %57 %59 %58
         %58 = OpLabel
         %60 = OpLoad %float %sumB
         %62 = OpFUnordNotEqual %bool %60 %float_55
               OpBranch %59
         %59 = OpLabel
         %63 = OpPhi %bool %true %33 %62 %58
               OpSelectionMerge %65 None
               OpBranchConditional %63 %64 %65
         %64 = OpLabel
         %66 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %70 = OpLoad %v4float %66
               OpReturnValue %70
         %65 = OpLabel
               OpStore %sumC %int_0
               OpStore %c %int_0
               OpBranch %75
         %75 = OpLabel
               OpLoopMerge %79 %78 None
               OpBranch %76
         %76 = OpLabel
         %80 = OpLoad %int %c
         %82 = OpSLessThan %bool %80 %int_10
               OpBranchConditional %82 %77 %79
         %77 = OpLabel
         %83 = OpLoad %int %sumC
         %84 = OpLoad %int %c
         %85 = OpIAdd %int %83 %84
               OpStore %sumC %85
               OpBranch %78
         %78 = OpLabel
         %86 = OpLoad %int %c
         %87 = OpIAdd %int %86 %int_1
               OpStore %c %87
               OpBranch %75
         %79 = OpLabel
         %88 = OpLoad %int %sumC
         %90 = OpINotEqual %bool %88 %int_45
               OpSelectionMerge %92 None
               OpBranchConditional %90 %91 %92
         %91 = OpLabel
         %93 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %94 = OpLoad %v4float %93
               OpReturnValue %94
         %92 = OpLabel
               OpStore %sumE %float_0
        %100 = OpCompositeConstruct %_arr_float_int_2 %float_0 %float_10
               OpStore %d %100
        %108 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
               OpStore %e %108
               OpBranch %109
        %109 = OpLabel
               OpLoopMerge %113 %112 None
               OpBranch %110
        %110 = OpLabel
        %114 = OpAccessChain %_ptr_Function_float %d %int_0
        %115 = OpLoad %float %114
        %116 = OpAccessChain %_ptr_Function_float %d %int_1
        %117 = OpLoad %float %116
        %118 = OpFOrdLessThan %bool %115 %117
               OpBranchConditional %118 %111 %113
        %111 = OpLabel
        %119 = OpLoad %float %sumE
        %120 = OpAccessChain %_ptr_Function_float %e %int_0
        %121 = OpLoad %float %120
        %122 = OpFAdd %float %119 %121
               OpStore %sumE %122
               OpBranch %112
        %112 = OpLabel
        %123 = OpAccessChain %_ptr_Function_float %d %int_0
        %124 = OpLoad %float %123
        %125 = OpFAdd %float %124 %float_1
               OpStore %123 %125
               OpBranch %109
        %113 = OpLabel
        %126 = OpLoad %float %sumE
        %127 = OpFUnordNotEqual %bool %126 %float_10
               OpSelectionMerge %129 None
               OpBranchConditional %127 %128 %129
        %128 = OpLabel
        %130 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %131 = OpLoad %v4float %130
               OpReturnValue %131
        %129 = OpLabel
               OpBranch %132
        %132 = OpLabel
               OpLoopMerge %136 %135 None
               OpBranch %133
        %133 = OpLabel
               OpBranch %134
        %134 = OpLabel
               OpBranch %136
        %135 = OpLabel
               OpBranch %132
        %136 = OpLabel
               OpBranch %137
        %137 = OpLabel
               OpLoopMerge %141 %140 None
               OpBranch %138
        %138 = OpLabel
               OpBranch %139
        %139 = OpLabel
        %142 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %143 = OpLoad %v4float %142
               OpReturnValue %143
        %140 = OpLabel
               OpBranch %137
        %141 = OpLabel
               OpUnreachable
               OpFunctionEnd
