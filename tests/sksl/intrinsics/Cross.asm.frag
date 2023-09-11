               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testMatrix3x3"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
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
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %67 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %mat3v3float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %18 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %22 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
      %int_1 = OpConstant %int 1
   %float_n3 = OpConstant %float -3
    %float_6 = OpConstant %float 6
         %41 = OpConstantComposite %v3float %float_n3 %float_6 %float_n3
     %v3bool = OpTypeVector %bool 3
      %int_2 = OpConstant %int 2
  %float_n12 = OpConstant %float -12
         %56 = OpConstantComposite %v3float %float_6 %float_n12 %float_6
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %14
         %15 = OpLabel
         %19 = OpVariable %_ptr_Function_v2float Function
               OpStore %19 %18
         %21 = OpFunctionCall %v4float %main %19
               OpStore %sk_FragColor %21
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %22
         %23 = OpFunctionParameter %_ptr_Function_v2float
         %24 = OpLabel
         %60 = OpVariable %_ptr_Function_v4float Function
         %28 = OpAccessChain %_ptr_Uniform_mat3v3float %7 %int_0
         %32 = OpAccessChain %_ptr_Uniform_v3float %28 %int_0
         %34 = OpLoad %v3float %32
         %35 = OpAccessChain %_ptr_Uniform_mat3v3float %7 %int_0
         %37 = OpAccessChain %_ptr_Uniform_v3float %35 %int_1
         %38 = OpLoad %v3float %37
         %27 = OpExtInst %v3float %1 Cross %34 %38
         %42 = OpFOrdEqual %v3bool %27 %41
         %44 = OpAll %bool %42
               OpSelectionMerge %46 None
               OpBranchConditional %44 %45 %46
         %45 = OpLabel
         %48 = OpAccessChain %_ptr_Uniform_mat3v3float %7 %int_0
         %50 = OpAccessChain %_ptr_Uniform_v3float %48 %int_2
         %51 = OpLoad %v3float %50
         %52 = OpAccessChain %_ptr_Uniform_mat3v3float %7 %int_0
         %53 = OpAccessChain %_ptr_Uniform_v3float %52 %int_0
         %54 = OpLoad %v3float %53
         %47 = OpExtInst %v3float %1 Cross %51 %54
         %57 = OpFOrdEqual %v3bool %47 %56
         %58 = OpAll %bool %57
               OpBranch %46
         %46 = OpLabel
         %59 = OpPhi %bool %false %24 %58 %45
               OpSelectionMerge %64 None
               OpBranchConditional %59 %62 %63
         %62 = OpLabel
         %65 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %67 = OpLoad %v4float %65
               OpStore %60 %67
               OpBranch %64
         %63 = OpLabel
         %68 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %69 = OpLoad %v4float %68
               OpStore %60 %69
               OpBranch %64
         %64 = OpLabel
         %70 = OpLoad %v4float %60
               OpReturnValue %70
               OpFunctionEnd
