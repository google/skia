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
               OpName %TrueFalse_b "TrueFalse_b"
               OpName %x "x"
               OpName %y "y"
               OpName %FalseTrue_b "FalseTrue_b"
               OpName %x_0 "x"
               OpName %y_0 "y"
               OpName %FalseFalse_b "FalseFalse_b"
               OpName %x_1 "x"
               OpName %y_1 "y"
               OpName %main "main"
               OpName %_0_TrueTrue "_0_TrueTrue"
               OpName %_2_y "_2_y"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
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
               OpDecorate %96 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %26 = OpTypeFunction %bool
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_1 = OpConstant %int 1
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
      %int_3 = OpConstant %int 3
      %int_2 = OpConstant %int 2
         %84 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %18
         %19 = OpLabel
         %23 = OpVariable %_ptr_Function_v2float Function
               OpStore %23 %22
         %25 = OpFunctionCall %v4float %main %23
               OpStore %sk_FragColor %25
               OpReturn
               OpFunctionEnd
%TrueFalse_b = OpFunction %bool None %26
         %27 = OpLabel
          %x = OpVariable %_ptr_Function_int Function
          %y = OpVariable %_ptr_Function_int Function
               OpStore %x %int_1
               OpStore %y %int_1
               OpSelectionMerge %36 None
               OpBranchConditional %true %35 %36
         %35 = OpLabel
         %37 = OpIAdd %int %int_1 %int_1
               OpStore %y %37
         %39 = OpIEqual %bool %37 %int_3
               OpBranch %36
         %36 = OpLabel
         %40 = OpPhi %bool %false %27 %39 %35
               OpSelectionMerge %43 None
               OpBranchConditional %40 %41 %42
         %41 = OpLabel
               OpReturnValue %false
         %42 = OpLabel
               OpSelectionMerge %45 None
               OpBranchConditional %true %44 %45
         %44 = OpLabel
         %46 = OpLoad %int %y
         %48 = OpIEqual %bool %46 %int_2
               OpBranch %45
         %45 = OpLabel
         %49 = OpPhi %bool %false %42 %48 %44
               OpReturnValue %49
         %43 = OpLabel
               OpUnreachable
               OpFunctionEnd
%FalseTrue_b = OpFunction %bool None %26
         %50 = OpLabel
        %x_0 = OpVariable %_ptr_Function_int Function
        %y_0 = OpVariable %_ptr_Function_int Function
               OpStore %x_0 %int_1
               OpStore %y_0 %int_1
         %53 = OpIEqual %bool %int_1 %int_2
               OpSelectionMerge %55 None
               OpBranchConditional %53 %54 %55
         %54 = OpLabel
         %56 = OpIAdd %int %int_1 %int_1
               OpStore %y_0 %56
         %57 = OpIEqual %bool %56 %int_2
               OpBranch %55
         %55 = OpLabel
         %58 = OpPhi %bool %false %50 %57 %54
               OpSelectionMerge %61 None
               OpBranchConditional %58 %59 %60
         %59 = OpLabel
               OpReturnValue %false
         %60 = OpLabel
               OpSelectionMerge %63 None
               OpBranchConditional %true %62 %63
         %62 = OpLabel
         %64 = OpLoad %int %y_0
         %65 = OpIEqual %bool %64 %int_1
               OpBranch %63
         %63 = OpLabel
         %66 = OpPhi %bool %false %60 %65 %62
               OpReturnValue %66
         %61 = OpLabel
               OpUnreachable
               OpFunctionEnd
%FalseFalse_b = OpFunction %bool None %26
         %67 = OpLabel
        %x_1 = OpVariable %_ptr_Function_int Function
        %y_1 = OpVariable %_ptr_Function_int Function
               OpStore %x_1 %int_1
               OpStore %y_1 %int_1
         %70 = OpIEqual %bool %int_1 %int_2
               OpSelectionMerge %72 None
               OpBranchConditional %70 %71 %72
         %71 = OpLabel
         %73 = OpIAdd %int %int_1 %int_1
               OpStore %y_1 %73
         %74 = OpIEqual %bool %73 %int_3
               OpBranch %72
         %72 = OpLabel
         %75 = OpPhi %bool %false %67 %74 %71
               OpSelectionMerge %78 None
               OpBranchConditional %75 %76 %77
         %76 = OpLabel
               OpReturnValue %false
         %77 = OpLabel
               OpSelectionMerge %80 None
               OpBranchConditional %true %79 %80
         %79 = OpLabel
         %81 = OpLoad %int %y_1
         %82 = OpIEqual %bool %81 %int_1
               OpBranch %80
         %80 = OpLabel
         %83 = OpPhi %bool %false %77 %82 %79
               OpReturnValue %83
         %78 = OpLabel
               OpUnreachable
               OpFunctionEnd
       %main = OpFunction %v4float None %84
         %85 = OpFunctionParameter %_ptr_Function_v2float
         %86 = OpLabel
%_0_TrueTrue = OpVariable %_ptr_Function_bool Function
       %_2_y = OpVariable %_ptr_Function_int Function
        %109 = OpVariable %_ptr_Function_v4float Function
               OpStore %_2_y %int_1
         %90 = OpIAdd %int %int_1 %int_1
               OpStore %_2_y %90
         %91 = OpIEqual %bool %90 %int_2
               OpSelectionMerge %94 None
               OpBranchConditional %91 %92 %93
         %92 = OpLabel
         %95 = OpIEqual %bool %90 %int_2
               OpStore %_0_TrueTrue %95
               OpBranch %94
         %93 = OpLabel
               OpStore %_0_TrueTrue %false
               OpBranch %94
         %94 = OpLabel
         %96 = OpLoad %bool %_0_TrueTrue
               OpSelectionMerge %98 None
               OpBranchConditional %96 %97 %98
         %97 = OpLabel
         %99 = OpFunctionCall %bool %TrueFalse_b
               OpBranch %98
         %98 = OpLabel
        %100 = OpPhi %bool %false %94 %99 %97
               OpSelectionMerge %102 None
               OpBranchConditional %100 %101 %102
        %101 = OpLabel
        %103 = OpFunctionCall %bool %FalseTrue_b
               OpBranch %102
        %102 = OpLabel
        %104 = OpPhi %bool %false %98 %103 %101
               OpSelectionMerge %106 None
               OpBranchConditional %104 %105 %106
        %105 = OpLabel
        %107 = OpFunctionCall %bool %FalseFalse_b
               OpBranch %106
        %106 = OpLabel
        %108 = OpPhi %bool %false %102 %107 %105
               OpSelectionMerge %113 None
               OpBranchConditional %108 %111 %112
        %111 = OpLabel
        %114 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
        %117 = OpLoad %v4float %114
               OpStore %109 %117
               OpBranch %113
        %112 = OpLabel
        %118 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
        %119 = OpLoad %v4float %118
               OpStore %109 %119
               OpBranch %113
        %113 = OpLabel
        %120 = OpLoad %v4float %109
               OpReturnValue %120
               OpFunctionEnd
