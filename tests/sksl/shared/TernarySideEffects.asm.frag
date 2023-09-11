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
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %x "x"
               OpName %y "y"
               OpName %b "b"
               OpName %c "c"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %x RelaxedPrecision
               OpDecorate %y RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
               OpDecorate %157 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %161 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
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
         %31 = OpVariable %_ptr_Function_float Function
         %41 = OpVariable %_ptr_Function_float Function
         %54 = OpVariable %_ptr_Function_float Function
         %67 = OpVariable %_ptr_Function_float Function
         %80 = OpVariable %_ptr_Function_float Function
         %91 = OpVariable %_ptr_Function_float Function
        %103 = OpVariable %_ptr_Function_float Function
        %114 = OpVariable %_ptr_Function_float Function
          %b = OpVariable %_ptr_Function_bool Function
          %c = OpVariable %_ptr_Function_bool Function
        %127 = OpVariable %_ptr_Function_bool Function
        %132 = OpVariable %_ptr_Function_v4float Function
        %151 = OpVariable %_ptr_Function_v4float Function
               OpStore %x %float_1
               OpStore %y %float_1
               OpSelectionMerge %34 None
               OpBranchConditional %true %32 %33
         %32 = OpLabel
         %35 = OpFAdd %float %float_1 %float_1
               OpStore %x %35
               OpStore %31 %35
               OpBranch %34
         %33 = OpLabel
         %36 = OpFAdd %float %float_1 %float_1
               OpStore %y %36
               OpStore %31 %36
               OpBranch %34
         %34 = OpLabel
         %37 = OpLoad %float %31
         %38 = OpLoad %float %x
         %39 = OpLoad %float %y
         %40 = OpFOrdEqual %bool %38 %39
               OpSelectionMerge %44 None
               OpBranchConditional %40 %42 %43
         %42 = OpLabel
         %45 = OpLoad %float %x
         %47 = OpFAdd %float %45 %float_3
               OpStore %x %47
               OpStore %41 %47
               OpBranch %44
         %43 = OpLabel
         %48 = OpLoad %float %y
         %49 = OpFAdd %float %48 %float_3
               OpStore %y %49
               OpStore %41 %49
               OpBranch %44
         %44 = OpLabel
         %50 = OpLoad %float %41
         %51 = OpLoad %float %x
         %52 = OpLoad %float %y
         %53 = OpFOrdLessThan %bool %51 %52
               OpSelectionMerge %57 None
               OpBranchConditional %53 %55 %56
         %55 = OpLabel
         %58 = OpLoad %float %x
         %60 = OpFAdd %float %58 %float_5
               OpStore %x %60
               OpStore %54 %60
               OpBranch %57
         %56 = OpLabel
         %61 = OpLoad %float %y
         %62 = OpFAdd %float %61 %float_5
               OpStore %y %62
               OpStore %54 %62
               OpBranch %57
         %57 = OpLabel
         %63 = OpLoad %float %54
         %64 = OpLoad %float %y
         %65 = OpLoad %float %x
         %66 = OpFOrdGreaterThanEqual %bool %64 %65
               OpSelectionMerge %70 None
               OpBranchConditional %66 %68 %69
         %68 = OpLabel
         %71 = OpLoad %float %x
         %73 = OpFAdd %float %71 %float_9
               OpStore %x %73
               OpStore %67 %73
               OpBranch %70
         %69 = OpLabel
         %74 = OpLoad %float %y
         %75 = OpFAdd %float %74 %float_9
               OpStore %y %75
               OpStore %67 %75
               OpBranch %70
         %70 = OpLabel
         %76 = OpLoad %float %67
         %77 = OpLoad %float %x
         %78 = OpLoad %float %y
         %79 = OpFUnordNotEqual %bool %77 %78
               OpSelectionMerge %83 None
               OpBranchConditional %79 %81 %82
         %81 = OpLabel
         %84 = OpLoad %float %x
         %85 = OpFAdd %float %84 %float_1
               OpStore %x %85
               OpStore %80 %85
               OpBranch %83
         %82 = OpLabel
         %86 = OpLoad %float %y
               OpStore %80 %86
               OpBranch %83
         %83 = OpLabel
         %87 = OpLoad %float %80
         %88 = OpLoad %float %x
         %89 = OpLoad %float %y
         %90 = OpFOrdEqual %bool %88 %89
               OpSelectionMerge %94 None
               OpBranchConditional %90 %92 %93
         %92 = OpLabel
         %95 = OpLoad %float %x
         %97 = OpFAdd %float %95 %float_2
               OpStore %x %97
               OpStore %91 %97
               OpBranch %94
         %93 = OpLabel
         %98 = OpLoad %float %y
               OpStore %91 %98
               OpBranch %94
         %94 = OpLabel
         %99 = OpLoad %float %91
        %100 = OpLoad %float %x
        %101 = OpLoad %float %y
        %102 = OpFUnordNotEqual %bool %100 %101
               OpSelectionMerge %106 None
               OpBranchConditional %102 %104 %105
        %104 = OpLabel
        %107 = OpLoad %float %x
               OpStore %103 %107
               OpBranch %106
        %105 = OpLabel
        %108 = OpLoad %float %y
        %109 = OpFAdd %float %108 %float_3
               OpStore %y %109
               OpStore %103 %109
               OpBranch %106
        %106 = OpLabel
        %110 = OpLoad %float %103
        %111 = OpLoad %float %x
        %112 = OpLoad %float %y
        %113 = OpFOrdEqual %bool %111 %112
               OpSelectionMerge %117 None
               OpBranchConditional %113 %115 %116
        %115 = OpLabel
        %118 = OpLoad %float %x
               OpStore %114 %118
               OpBranch %117
        %116 = OpLabel
        %119 = OpLoad %float %y
        %121 = OpFAdd %float %119 %float_4
               OpStore %y %121
               OpStore %114 %121
               OpBranch %117
        %117 = OpLabel
        %122 = OpLoad %float %114
               OpStore %b %true
               OpStore %b %false
               OpSelectionMerge %130 None
               OpBranchConditional %false %128 %129
        %128 = OpLabel
               OpStore %127 %false
               OpBranch %130
        %129 = OpLabel
               OpStore %127 %false
               OpBranch %130
        %130 = OpLabel
        %131 = OpLoad %bool %127
               OpStore %c %131
               OpSelectionMerge %136 None
               OpBranchConditional %131 %134 %135
        %134 = OpLabel
        %137 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %141 = OpLoad %v4float %137
               OpStore %132 %141
               OpBranch %136
        %135 = OpLabel
        %142 = OpLoad %float %x
        %144 = OpFOrdEqual %bool %142 %float_8
               OpSelectionMerge %146 None
               OpBranchConditional %144 %145 %146
        %145 = OpLabel
        %147 = OpLoad %float %y
        %149 = OpFOrdEqual %bool %147 %float_17
               OpBranch %146
        %146 = OpLabel
        %150 = OpPhi %bool %false %135 %149 %145
               OpSelectionMerge %154 None
               OpBranchConditional %150 %152 %153
        %152 = OpLabel
        %155 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %157 = OpLoad %v4float %155
               OpStore %151 %157
               OpBranch %154
        %153 = OpLabel
        %158 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %159 = OpLoad %v4float %158
               OpStore %151 %159
               OpBranch %154
        %154 = OpLabel
        %160 = OpLoad %v4float %151
               OpStore %132 %160
               OpBranch %136
        %136 = OpLabel
        %161 = OpLoad %v4float %132
               OpReturnValue %161
               OpFunctionEnd
