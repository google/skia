               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testInputs"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
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
               OpDecorate %27 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
   %float_n1 = OpConstant %float -1
         %42 = OpConstantComposite %v2float %float_n1 %float_0
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
         %54 = OpConstantComposite %v3float %float_n1 %float_0 %float_0
     %v3bool = OpTypeVector %bool 3
    %float_2 = OpConstant %float 2
         %65 = OpConstantComposite %v4float %float_n1 %float_0 %float_0 %float_2
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
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
         %70 = OpVariable %_ptr_Function_v4float Function
         %28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %32 = OpLoad %v4float %28
         %33 = OpCompositeExtract %float %32 0
         %27 = OpExtInst %float %1 Trunc %33
         %35 = OpFOrdEqual %bool %27 %float_n1
               OpSelectionMerge %37 None
               OpBranchConditional %35 %36 %37
         %36 = OpLabel
         %39 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %40 = OpLoad %v4float %39
         %41 = OpVectorShuffle %v2float %40 %40 0 1
         %38 = OpExtInst %v2float %1 Trunc %41
         %43 = OpFOrdEqual %v2bool %38 %42
         %45 = OpAll %bool %43
               OpBranch %37
         %37 = OpLabel
         %46 = OpPhi %bool %false %25 %45 %36
               OpSelectionMerge %48 None
               OpBranchConditional %46 %47 %48
         %47 = OpLabel
         %50 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %51 = OpLoad %v4float %50
         %52 = OpVectorShuffle %v3float %51 %51 0 1 2
         %49 = OpExtInst %v3float %1 Trunc %52
         %55 = OpFOrdEqual %v3bool %49 %54
         %57 = OpAll %bool %55
               OpBranch %48
         %48 = OpLabel
         %58 = OpPhi %bool %false %37 %57 %47
               OpSelectionMerge %60 None
               OpBranchConditional %58 %59 %60
         %59 = OpLabel
         %62 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %63 = OpLoad %v4float %62
         %61 = OpExtInst %v4float %1 Trunc %63
         %66 = OpFOrdEqual %v4bool %61 %65
         %68 = OpAll %bool %66
               OpBranch %60
         %60 = OpLabel
         %69 = OpPhi %bool %false %48 %68 %59
               OpSelectionMerge %74 None
               OpBranchConditional %69 %72 %73
         %72 = OpLabel
         %75 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %77 = OpLoad %v4float %75
               OpStore %70 %77
               OpBranch %74
         %73 = OpLabel
         %78 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %80 = OpLoad %v4float %78
               OpStore %70 %80
               OpBranch %74
         %74 = OpLabel
         %81 = OpLoad %v4float %70
               OpReturnValue %81
               OpFunctionEnd
