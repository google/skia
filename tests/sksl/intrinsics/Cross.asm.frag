               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testMatrix3x3"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 ColMajor
               OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 1 Offset 48
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 64
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %69 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %mat3v3float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %25 = OpTypeFunction %v4float %_ptr_Function_v2float
      %false = OpConstantFalse %bool
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
      %int_1 = OpConstant %int 1
   %float_n3 = OpConstant %float -3
    %float_6 = OpConstant %float 6
         %43 = OpConstantComposite %v3float %float_n3 %float_6 %float_n3
     %v3bool = OpTypeVector %bool 3
      %int_2 = OpConstant %int 2
  %float_n12 = OpConstant %float -12
         %58 = OpConstantComposite %v3float %float_6 %float_n12 %float_6
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %17
         %18 = OpLabel
         %22 = OpVariable %_ptr_Function_v2float Function
               OpStore %22 %21
         %24 = OpFunctionCall %v4float %main %22
               OpStore %sk_FragColor %24
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %25
         %26 = OpFunctionParameter %_ptr_Function_v2float
         %27 = OpLabel
         %62 = OpVariable %_ptr_Function_v4float Function
         %30 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_0
         %34 = OpAccessChain %_ptr_Uniform_v3float %30 %int_0
         %36 = OpLoad %v3float %34
         %37 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_0
         %39 = OpAccessChain %_ptr_Uniform_v3float %37 %int_1
         %40 = OpLoad %v3float %39
         %29 = OpExtInst %v3float %1 Cross %36 %40
         %44 = OpFOrdEqual %v3bool %29 %43
         %46 = OpAll %bool %44
               OpSelectionMerge %48 None
               OpBranchConditional %46 %47 %48
         %47 = OpLabel
         %50 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_0
         %52 = OpAccessChain %_ptr_Uniform_v3float %50 %int_2
         %53 = OpLoad %v3float %52
         %54 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_0
         %55 = OpAccessChain %_ptr_Uniform_v3float %54 %int_0
         %56 = OpLoad %v3float %55
         %49 = OpExtInst %v3float %1 Cross %53 %56
         %59 = OpFOrdEqual %v3bool %49 %58
         %60 = OpAll %bool %59
               OpBranch %48
         %48 = OpLabel
         %61 = OpPhi %bool %false %27 %60 %47
               OpSelectionMerge %66 None
               OpBranchConditional %61 %64 %65
         %64 = OpLabel
         %67 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %69 = OpLoad %v4float %67
               OpStore %62 %69
               OpBranch %66
         %65 = OpLabel
         %70 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %71 = OpLoad %v4float %70
               OpStore %62 %71
               OpBranch %66
         %66 = OpLabel
         %72 = OpLoad %v4float %62
               OpReturnValue %72
               OpFunctionEnd
