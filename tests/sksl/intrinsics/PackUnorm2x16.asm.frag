               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testInputs"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %xy "xy"
               OpName %zw "zw"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %65 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%float_0_015625 = OpConstant %float 0.015625
         %45 = OpConstantComposite %v2float %float_0_015625 %float_0_015625
     %v2bool = OpTypeVector %bool 2
 %float_0_75 = OpConstant %float 0.75
    %float_1 = OpConstant %float 1
         %55 = OpConstantComposite %v2float %float_0_75 %float_1
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
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
         %xy = OpVariable %_ptr_Function_uint Function
         %zw = OpVariable %_ptr_Function_uint Function
         %58 = OpVariable %_ptr_Function_v4float Function
         %27 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %31 = OpLoad %v4float %27
         %32 = OpVectorShuffle %v2float %31 %31 0 1
         %26 = OpExtInst %uint %1 PackUnorm2x16 %32
               OpStore %xy %26
         %35 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %36 = OpLoad %v4float %35
         %37 = OpVectorShuffle %v2float %36 %36 2 3
         %34 = OpExtInst %uint %1 PackUnorm2x16 %37
               OpStore %zw %34
         %43 = OpExtInst %v2float %1 UnpackUnorm2x16 %26
         %42 = OpExtInst %v2float %1 FAbs %43
         %41 = OpFOrdLessThan %v2bool %42 %45
         %40 = OpAll %bool %41
               OpSelectionMerge %48 None
               OpBranchConditional %40 %47 %48
         %47 = OpLabel
         %52 = OpExtInst %v2float %1 UnpackUnorm2x16 %34
         %56 = OpFSub %v2float %52 %55
         %51 = OpExtInst %v2float %1 FAbs %56
         %50 = OpFOrdLessThan %v2bool %51 %45
         %49 = OpAll %bool %50
               OpBranch %48
         %48 = OpLabel
         %57 = OpPhi %bool %false %22 %49 %47
               OpSelectionMerge %62 None
               OpBranchConditional %57 %60 %61
         %60 = OpLabel
         %63 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %65 = OpLoad %v4float %63
               OpStore %58 %65
               OpBranch %62
         %61 = OpLabel
         %66 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %68 = OpLoad %v4float %66
               OpStore %58 %68
               OpBranch %62
         %62 = OpLabel
         %69 = OpLoad %v4float %58
               OpReturnValue %69
               OpFunctionEnd
