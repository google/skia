               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
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
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %59 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
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
    %float_2 = OpConstant %float 2
        %int = OpTypeInt 32 1
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
          %z = OpVariable %_ptr_Function_int Function
          %b = OpVariable %_ptr_Function_bool Function
          %c = OpVariable %_ptr_Function_bool Function
          %d = OpVariable %_ptr_Function_bool Function
          %e = OpVariable %_ptr_Function_bool Function
          %f = OpVariable %_ptr_Function_bool Function
        %105 = OpVariable %_ptr_Function_v4float Function
               OpStore %x %float_1
               OpStore %y %float_2
               OpStore %z %int_3
         %32 = OpFSub %float %float_1 %float_1
         %33 = OpFMul %float %float_2 %float_1
         %34 = OpFMul %float %33 %float_1
         %35 = OpFSub %float %float_2 %float_1
         %36 = OpFMul %float %34 %35
         %37 = OpFAdd %float %32 %36
               OpStore %x %37
         %38 = OpFDiv %float %37 %float_2
         %39 = OpFDiv %float %38 %37
               OpStore %y %39
         %41 = OpSDiv %int %int_3 %int_2
         %42 = OpIMul %int %41 %int_3
         %44 = OpIAdd %int %42 %int_4
         %45 = OpISub %int %44 %int_2
               OpStore %z %45
         %51 = OpFOrdGreaterThan %bool %37 %float_4
         %52 = OpFOrdLessThan %bool %37 %float_2
         %53 = OpLogicalEqual %bool %51 %52
               OpSelectionMerge %55 None
               OpBranchConditional %53 %55 %54
         %54 = OpLabel
         %57 = OpAccessChain %_ptr_Uniform_float %7 %int_2
         %59 = OpLoad %float %57
         %60 = OpFOrdGreaterThanEqual %bool %float_2 %59
               OpSelectionMerge %62 None
               OpBranchConditional %60 %61 %62
         %61 = OpLabel
         %63 = OpFOrdLessThanEqual %bool %39 %37
               OpBranch %62
         %62 = OpLabel
         %64 = OpPhi %bool %false %54 %63 %61
               OpBranch %55
         %55 = OpLabel
         %65 = OpPhi %bool %true %22 %64 %62
               OpStore %b %65
         %67 = OpAccessChain %_ptr_Uniform_float %7 %int_2
         %68 = OpLoad %float %67
         %69 = OpFOrdGreaterThan %bool %68 %float_2
               OpStore %c %69
         %71 = OpLogicalNotEqual %bool %65 %69
               OpStore %d %71
               OpSelectionMerge %74 None
               OpBranchConditional %65 %73 %74
         %73 = OpLabel
               OpBranch %74
         %74 = OpLabel
         %75 = OpPhi %bool %false %55 %69 %73
               OpStore %e %75
               OpSelectionMerge %78 None
               OpBranchConditional %65 %78 %77
         %77 = OpLabel
               OpBranch %78
         %78 = OpLabel
         %79 = OpPhi %bool %true %74 %69 %77
               OpStore %f %79
         %81 = OpFAdd %float %37 %float_12
               OpStore %x %81
         %82 = OpFSub %float %81 %float_12
               OpStore %x %82
         %84 = OpFMul %float %39 %float_0_100000001
               OpStore %y %84
         %85 = OpFMul %float %82 %84
               OpStore %x %85
               OpStore %x %float_6
         %87 = OpSelect %float %65 %float_1 %float_0
         %88 = OpSelect %float %69 %float_1 %float_0
         %89 = OpFMul %float %87 %88
         %90 = OpSelect %float %71 %float_1 %float_0
         %91 = OpFMul %float %89 %90
         %92 = OpSelect %float %75 %float_1 %float_0
         %93 = OpFMul %float %91 %92
         %94 = OpSelect %float %79 %float_1 %float_0
         %95 = OpFMul %float %93 %94
               OpStore %y %95
               OpStore %y %float_6
         %97 = OpISub %int %45 %int_1
               OpStore %z %97
               OpStore %z %int_6
               OpSelectionMerge %100 None
               OpBranchConditional %true %99 %100
         %99 = OpLabel
               OpBranch %100
        %100 = OpLabel
        %101 = OpPhi %bool %false %78 %true %99
               OpSelectionMerge %103 None
               OpBranchConditional %101 %102 %103
        %102 = OpLabel
               OpBranch %103
        %103 = OpLabel
        %104 = OpPhi %bool %false %100 %true %102
               OpSelectionMerge %109 None
               OpBranchConditional %104 %107 %108
        %107 = OpLabel
        %110 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %113 = OpLoad %v4float %110
               OpStore %105 %113
               OpBranch %109
        %108 = OpLabel
        %114 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %115 = OpLoad %v4float %114
               OpStore %105 %115
               OpBranch %109
        %109 = OpLabel
        %116 = OpLoad %v4float %105
               OpReturnValue %116
               OpFunctionEnd
