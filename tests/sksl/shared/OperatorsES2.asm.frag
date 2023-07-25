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
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
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
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %61 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
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
        %107 = OpVariable %_ptr_Function_v4float Function
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
         %45 = OpIMul %int %44 %int_3
         %47 = OpIAdd %int %45 %int_4
         %48 = OpISub %int %47 %int_2
               OpStore %z %48
         %53 = OpFOrdGreaterThan %bool %40 %float_4
         %54 = OpFOrdLessThan %bool %40 %float_2
         %55 = OpLogicalEqual %bool %53 %54
               OpSelectionMerge %57 None
               OpBranchConditional %55 %57 %56
         %56 = OpLabel
         %59 = OpAccessChain %_ptr_Uniform_float %10 %int_2
         %61 = OpLoad %float %59
         %62 = OpFOrdGreaterThanEqual %bool %float_2 %61
               OpSelectionMerge %64 None
               OpBranchConditional %62 %63 %64
         %63 = OpLabel
         %65 = OpFOrdLessThanEqual %bool %42 %40
               OpBranch %64
         %64 = OpLabel
         %66 = OpPhi %bool %false %56 %65 %63
               OpBranch %57
         %57 = OpLabel
         %67 = OpPhi %bool %true %25 %66 %64
               OpStore %b %67
         %69 = OpAccessChain %_ptr_Uniform_float %10 %int_2
         %70 = OpLoad %float %69
         %71 = OpFOrdGreaterThan %bool %70 %float_2
               OpStore %c %71
         %73 = OpLogicalNotEqual %bool %67 %71
               OpStore %d %73
               OpSelectionMerge %76 None
               OpBranchConditional %67 %75 %76
         %75 = OpLabel
               OpBranch %76
         %76 = OpLabel
         %77 = OpPhi %bool %false %57 %71 %75
               OpStore %e %77
               OpSelectionMerge %80 None
               OpBranchConditional %67 %80 %79
         %79 = OpLabel
               OpBranch %80
         %80 = OpLabel
         %81 = OpPhi %bool %true %76 %71 %79
               OpStore %f %81
         %83 = OpFAdd %float %40 %float_12
               OpStore %x %83
         %84 = OpFSub %float %83 %float_12
               OpStore %x %84
         %86 = OpFMul %float %42 %float_0_100000001
               OpStore %y %86
         %87 = OpFMul %float %84 %86
               OpStore %x %87
               OpStore %x %float_6
         %89 = OpSelect %float %67 %float_1 %float_0
         %90 = OpSelect %float %71 %float_1 %float_0
         %91 = OpFMul %float %89 %90
         %92 = OpSelect %float %73 %float_1 %float_0
         %93 = OpFMul %float %91 %92
         %94 = OpSelect %float %77 %float_1 %float_0
         %95 = OpFMul %float %93 %94
         %96 = OpSelect %float %81 %float_1 %float_0
         %97 = OpFMul %float %95 %96
               OpStore %y %97
               OpStore %y %float_6
         %99 = OpISub %int %48 %int_1
               OpStore %z %99
               OpStore %z %int_6
               OpSelectionMerge %102 None
               OpBranchConditional %true %101 %102
        %101 = OpLabel
               OpBranch %102
        %102 = OpLabel
        %103 = OpPhi %bool %false %80 %true %101
               OpSelectionMerge %105 None
               OpBranchConditional %103 %104 %105
        %104 = OpLabel
               OpBranch %105
        %105 = OpLabel
        %106 = OpPhi %bool %false %102 %true %104
               OpSelectionMerge %111 None
               OpBranchConditional %106 %109 %110
        %109 = OpLabel
        %112 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %115 = OpLoad %v4float %112
               OpStore %107 %115
               OpBranch %111
        %110 = OpLabel
        %116 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %117 = OpLoad %v4float %116
               OpStore %107 %117
               OpBranch %111
        %111 = OpLabel
        %118 = OpLoad %v4float %107
               OpReturnValue %118
               OpFunctionEnd
