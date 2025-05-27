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
               OpDecorate %62 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
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
%float_n1_25 = OpConstant %float -1.25
         %42 = OpConstantComposite %v2float %float_n1_25 %float_0
     %v2bool = OpTypeVector %bool 2
 %float_0_75 = OpConstant %float 0.75
 %float_2_25 = OpConstant %float 2.25
         %51 = OpConstantComposite %v2float %float_0_75 %float_2_25
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
         %55 = OpVariable %_ptr_Function_v4float Function
         %27 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %31 = OpLoad %v4float %27
         %32 = OpVectorShuffle %v2float %31 %31 0 1
         %26 = OpExtInst %uint %1 PackHalf2x16 %32
               OpStore %xy %26
         %35 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %36 = OpLoad %v4float %35
         %37 = OpVectorShuffle %v2float %36 %36 2 3
         %34 = OpExtInst %uint %1 PackHalf2x16 %37
               OpStore %zw %34
         %40 = OpExtInst %v2float %1 UnpackHalf2x16 %26
         %43 = OpFOrdEqual %v2bool %40 %42
         %45 = OpAll %bool %43
               OpSelectionMerge %47 None
               OpBranchConditional %45 %46 %47
         %46 = OpLabel
         %48 = OpExtInst %v2float %1 UnpackHalf2x16 %34
         %52 = OpFOrdEqual %v2bool %48 %51
         %53 = OpAll %bool %52
               OpBranch %47
         %47 = OpLabel
         %54 = OpPhi %bool %false %22 %53 %46
               OpSelectionMerge %59 None
               OpBranchConditional %54 %57 %58
         %57 = OpLabel
         %60 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %62 = OpLoad %v4float %60
               OpStore %55 %62
               OpBranch %59
         %58 = OpLabel
         %63 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %65 = OpLoad %v4float %63
               OpStore %55 %65
               OpBranch %59
         %59 = OpLabel
         %66 = OpLoad %v4float %55
               OpReturnValue %66
               OpFunctionEnd
