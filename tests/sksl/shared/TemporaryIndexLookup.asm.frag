               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testMatrix3x3"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %GetTestMatrix_f33 "GetTestMatrix_f33"
               OpName %main "main"
               OpName %expected "expected"
               OpName %i "i"
               OpName %j "j"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 ColMajor
               OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
               OpDecorate %_UniformBuffer Block
               OpDecorate %8 Binding 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %73 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat3v3float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %8 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %mat3v3float
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
         %30 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_3 = OpConstant %int 3
       %bool = OpTypeBool
    %float_1 = OpConstant %float 1
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %15
         %16 = OpLabel
         %20 = OpVariable %_ptr_Function_v2float Function
               OpStore %20 %19
         %22 = OpFunctionCall %v4float %main %20
               OpStore %sk_FragColor %22
               OpReturn
               OpFunctionEnd
%GetTestMatrix_f33 = OpFunction %mat3v3float None %23
         %24 = OpLabel
         %25 = OpAccessChain %_ptr_Uniform_mat3v3float %8 %int_2
         %29 = OpLoad %mat3v3float %25
               OpReturnValue %29
               OpFunctionEnd
       %main = OpFunction %v4float None %30
         %31 = OpFunctionParameter %_ptr_Function_v2float
         %32 = OpLabel
   %expected = OpVariable %_ptr_Function_float Function
          %i = OpVariable %_ptr_Function_int Function
          %j = OpVariable %_ptr_Function_int Function
         %58 = OpVariable %_ptr_Function_mat3v3float Function
               OpStore %expected %float_0
               OpStore %i %int_0
               OpBranch %38
         %38 = OpLabel
               OpLoopMerge %42 %41 None
               OpBranch %39
         %39 = OpLabel
         %43 = OpLoad %int %i
         %45 = OpSLessThan %bool %43 %int_3
               OpBranchConditional %45 %40 %42
         %40 = OpLabel
               OpStore %j %int_0
               OpBranch %48
         %48 = OpLabel
               OpLoopMerge %52 %51 None
               OpBranch %49
         %49 = OpLabel
         %53 = OpLoad %int %j
         %54 = OpSLessThan %bool %53 %int_3
               OpBranchConditional %54 %50 %52
         %50 = OpLabel
         %55 = OpLoad %float %expected
         %57 = OpFAdd %float %55 %float_1
               OpStore %expected %57
         %60 = OpFunctionCall %mat3v3float %GetTestMatrix_f33
               OpStore %58 %60
         %61 = OpLoad %int %i
         %62 = OpAccessChain %_ptr_Function_v3float %58 %61
         %64 = OpLoad %v3float %62
         %65 = OpLoad %int %j
         %66 = OpVectorExtractDynamic %float %64 %65
         %67 = OpFUnordNotEqual %bool %66 %57
               OpSelectionMerge %69 None
               OpBranchConditional %67 %68 %69
         %68 = OpLabel
         %70 = OpAccessChain %_ptr_Uniform_v4float %8 %int_1
         %73 = OpLoad %v4float %70
               OpReturnValue %73
         %69 = OpLabel
               OpBranch %51
         %51 = OpLabel
         %74 = OpLoad %int %j
         %75 = OpIAdd %int %74 %int_1
               OpStore %j %75
               OpBranch %48
         %52 = OpLabel
               OpBranch %41
         %41 = OpLabel
         %76 = OpLoad %int %i
         %77 = OpIAdd %int %76 %int_1
               OpStore %i %77
               OpBranch %38
         %42 = OpLabel
         %78 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %79 = OpLoad %v4float %78
               OpReturnValue %79
               OpFunctionEnd
