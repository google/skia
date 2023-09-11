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
               OpMemberName %_UniformBuffer 2 "testMatrix3x3"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %GetTestMatrix_f33 "GetTestMatrix_f33"
               OpName %main "main"
               OpName %expected "expected"
               OpName %i "i"
               OpName %j "j"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
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
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %75 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat3v3float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %26 = OpTypeFunction %mat3v3float
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
         %33 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_3 = OpConstant %int 3
    %float_1 = OpConstant %float 1
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %18
         %19 = OpLabel
         %23 = OpVariable %_ptr_Function_v2float Function
               OpStore %23 %22
         %25 = OpFunctionCall %v4float %main %23
               OpStore %sk_FragColor %25
               OpReturn
               OpFunctionEnd
%GetTestMatrix_f33 = OpFunction %mat3v3float None %26
         %27 = OpLabel
         %28 = OpAccessChain %_ptr_Uniform_mat3v3float %11 %int_2
         %32 = OpLoad %mat3v3float %28
               OpReturnValue %32
               OpFunctionEnd
       %main = OpFunction %v4float None %33
         %34 = OpFunctionParameter %_ptr_Function_v2float
         %35 = OpLabel
   %expected = OpVariable %_ptr_Function_float Function
          %i = OpVariable %_ptr_Function_int Function
          %j = OpVariable %_ptr_Function_int Function
         %60 = OpVariable %_ptr_Function_mat3v3float Function
               OpStore %expected %float_0
               OpStore %i %int_0
               OpBranch %41
         %41 = OpLabel
               OpLoopMerge %45 %44 None
               OpBranch %42
         %42 = OpLabel
         %46 = OpLoad %int %i
         %48 = OpSLessThan %bool %46 %int_3
               OpBranchConditional %48 %43 %45
         %43 = OpLabel
               OpStore %j %int_0
               OpBranch %50
         %50 = OpLabel
               OpLoopMerge %54 %53 None
               OpBranch %51
         %51 = OpLabel
         %55 = OpLoad %int %j
         %56 = OpSLessThan %bool %55 %int_3
               OpBranchConditional %56 %52 %54
         %52 = OpLabel
         %57 = OpLoad %float %expected
         %59 = OpFAdd %float %57 %float_1
               OpStore %expected %59
         %62 = OpFunctionCall %mat3v3float %GetTestMatrix_f33
               OpStore %60 %62
         %63 = OpLoad %int %i
         %64 = OpAccessChain %_ptr_Function_v3float %60 %63
         %66 = OpLoad %v3float %64
         %67 = OpLoad %int %j
         %68 = OpVectorExtractDynamic %float %66 %67
         %69 = OpFUnordNotEqual %bool %68 %59
               OpSelectionMerge %71 None
               OpBranchConditional %69 %70 %71
         %70 = OpLabel
         %72 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %75 = OpLoad %v4float %72
               OpReturnValue %75
         %71 = OpLabel
               OpBranch %53
         %53 = OpLabel
         %76 = OpLoad %int %j
         %77 = OpIAdd %int %76 %int_1
               OpStore %j %77
               OpBranch %50
         %54 = OpLabel
               OpBranch %44
         %44 = OpLabel
         %78 = OpLoad %int %i
         %79 = OpIAdd %int %78 %int_1
               OpStore %i %79
               OpBranch %41
         %45 = OpLabel
         %80 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %81 = OpLoad %v4float %80
               OpReturnValue %81
               OpFunctionEnd
