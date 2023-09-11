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
               OpName %x "x"
               OpName %y "y"
               OpName %b "b"
               OpName %c "c"
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
               OpDecorate %x RelaxedPrecision
               OpDecorate %y RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %157 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
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
    %float_1 = OpConstant %float 1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
    %float_3 = OpConstant %float 3
    %float_5 = OpConstant %float 5
    %float_9 = OpConstant %float 9
    %float_2 = OpConstant %float 2
    %float_4 = OpConstant %float 4
%_ptr_Function_bool = OpTypePointer Function %bool
      %false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
    %float_8 = OpConstant %float 8
   %float_17 = OpConstant %float 17
      %int_0 = OpConstant %int 0
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
          %x = OpVariable %_ptr_Function_float Function
          %y = OpVariable %_ptr_Function_float Function
         %29 = OpVariable %_ptr_Function_float Function
         %39 = OpVariable %_ptr_Function_float Function
         %52 = OpVariable %_ptr_Function_float Function
         %65 = OpVariable %_ptr_Function_float Function
         %78 = OpVariable %_ptr_Function_float Function
         %89 = OpVariable %_ptr_Function_float Function
        %101 = OpVariable %_ptr_Function_float Function
        %112 = OpVariable %_ptr_Function_float Function
          %b = OpVariable %_ptr_Function_bool Function
          %c = OpVariable %_ptr_Function_bool Function
        %125 = OpVariable %_ptr_Function_bool Function
        %130 = OpVariable %_ptr_Function_v4float Function
        %149 = OpVariable %_ptr_Function_v4float Function
               OpStore %x %float_1
               OpStore %y %float_1
               OpSelectionMerge %32 None
               OpBranchConditional %true %30 %31
         %30 = OpLabel
         %33 = OpFAdd %float %float_1 %float_1
               OpStore %x %33
               OpStore %29 %33
               OpBranch %32
         %31 = OpLabel
         %34 = OpFAdd %float %float_1 %float_1
               OpStore %y %34
               OpStore %29 %34
               OpBranch %32
         %32 = OpLabel
         %35 = OpLoad %float %29
         %36 = OpLoad %float %x
         %37 = OpLoad %float %y
         %38 = OpFOrdEqual %bool %36 %37
               OpSelectionMerge %42 None
               OpBranchConditional %38 %40 %41
         %40 = OpLabel
         %43 = OpLoad %float %x
         %45 = OpFAdd %float %43 %float_3
               OpStore %x %45
               OpStore %39 %45
               OpBranch %42
         %41 = OpLabel
         %46 = OpLoad %float %y
         %47 = OpFAdd %float %46 %float_3
               OpStore %y %47
               OpStore %39 %47
               OpBranch %42
         %42 = OpLabel
         %48 = OpLoad %float %39
         %49 = OpLoad %float %x
         %50 = OpLoad %float %y
         %51 = OpFOrdLessThan %bool %49 %50
               OpSelectionMerge %55 None
               OpBranchConditional %51 %53 %54
         %53 = OpLabel
         %56 = OpLoad %float %x
         %58 = OpFAdd %float %56 %float_5
               OpStore %x %58
               OpStore %52 %58
               OpBranch %55
         %54 = OpLabel
         %59 = OpLoad %float %y
         %60 = OpFAdd %float %59 %float_5
               OpStore %y %60
               OpStore %52 %60
               OpBranch %55
         %55 = OpLabel
         %61 = OpLoad %float %52
         %62 = OpLoad %float %y
         %63 = OpLoad %float %x
         %64 = OpFOrdGreaterThanEqual %bool %62 %63
               OpSelectionMerge %68 None
               OpBranchConditional %64 %66 %67
         %66 = OpLabel
         %69 = OpLoad %float %x
         %71 = OpFAdd %float %69 %float_9
               OpStore %x %71
               OpStore %65 %71
               OpBranch %68
         %67 = OpLabel
         %72 = OpLoad %float %y
         %73 = OpFAdd %float %72 %float_9
               OpStore %y %73
               OpStore %65 %73
               OpBranch %68
         %68 = OpLabel
         %74 = OpLoad %float %65
         %75 = OpLoad %float %x
         %76 = OpLoad %float %y
         %77 = OpFUnordNotEqual %bool %75 %76
               OpSelectionMerge %81 None
               OpBranchConditional %77 %79 %80
         %79 = OpLabel
         %82 = OpLoad %float %x
         %83 = OpFAdd %float %82 %float_1
               OpStore %x %83
               OpStore %78 %83
               OpBranch %81
         %80 = OpLabel
         %84 = OpLoad %float %y
               OpStore %78 %84
               OpBranch %81
         %81 = OpLabel
         %85 = OpLoad %float %78
         %86 = OpLoad %float %x
         %87 = OpLoad %float %y
         %88 = OpFOrdEqual %bool %86 %87
               OpSelectionMerge %92 None
               OpBranchConditional %88 %90 %91
         %90 = OpLabel
         %93 = OpLoad %float %x
         %95 = OpFAdd %float %93 %float_2
               OpStore %x %95
               OpStore %89 %95
               OpBranch %92
         %91 = OpLabel
         %96 = OpLoad %float %y
               OpStore %89 %96
               OpBranch %92
         %92 = OpLabel
         %97 = OpLoad %float %89
         %98 = OpLoad %float %x
         %99 = OpLoad %float %y
        %100 = OpFUnordNotEqual %bool %98 %99
               OpSelectionMerge %104 None
               OpBranchConditional %100 %102 %103
        %102 = OpLabel
        %105 = OpLoad %float %x
               OpStore %101 %105
               OpBranch %104
        %103 = OpLabel
        %106 = OpLoad %float %y
        %107 = OpFAdd %float %106 %float_3
               OpStore %y %107
               OpStore %101 %107
               OpBranch %104
        %104 = OpLabel
        %108 = OpLoad %float %101
        %109 = OpLoad %float %x
        %110 = OpLoad %float %y
        %111 = OpFOrdEqual %bool %109 %110
               OpSelectionMerge %115 None
               OpBranchConditional %111 %113 %114
        %113 = OpLabel
        %116 = OpLoad %float %x
               OpStore %112 %116
               OpBranch %115
        %114 = OpLabel
        %117 = OpLoad %float %y
        %119 = OpFAdd %float %117 %float_4
               OpStore %y %119
               OpStore %112 %119
               OpBranch %115
        %115 = OpLabel
        %120 = OpLoad %float %112
               OpStore %b %true
               OpStore %b %false
               OpSelectionMerge %128 None
               OpBranchConditional %false %126 %127
        %126 = OpLabel
               OpStore %125 %false
               OpBranch %128
        %127 = OpLabel
               OpStore %125 %false
               OpBranch %128
        %128 = OpLabel
        %129 = OpLoad %bool %125
               OpStore %c %129
               OpSelectionMerge %134 None
               OpBranchConditional %129 %132 %133
        %132 = OpLabel
        %135 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %139 = OpLoad %v4float %135
               OpStore %130 %139
               OpBranch %134
        %133 = OpLabel
        %140 = OpLoad %float %x
        %142 = OpFOrdEqual %bool %140 %float_8
               OpSelectionMerge %144 None
               OpBranchConditional %142 %143 %144
        %143 = OpLabel
        %145 = OpLoad %float %y
        %147 = OpFOrdEqual %bool %145 %float_17
               OpBranch %144
        %144 = OpLabel
        %148 = OpPhi %bool %false %133 %147 %143
               OpSelectionMerge %152 None
               OpBranchConditional %148 %150 %151
        %150 = OpLabel
        %153 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %155 = OpLoad %v4float %153
               OpStore %149 %155
               OpBranch %152
        %151 = OpLabel
        %156 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %157 = OpLoad %v4float %156
               OpStore %149 %157
               OpBranch %152
        %152 = OpLabel
        %158 = OpLoad %v4float %149
               OpStore %130 %158
               OpBranch %134
        %134 = OpLabel
        %159 = OpLoad %v4float %130
               OpReturnValue %159
               OpFunctionEnd
