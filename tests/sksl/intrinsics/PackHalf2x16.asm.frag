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
               OpMemberName %_UniformBuffer 2 "testInputs"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %xy "xy"
               OpName %zw "zw"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %64 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
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
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
      %false = OpConstantFalse %bool
%float_n1_25 = OpConstant %float -1.25
         %44 = OpConstantComposite %v2float %float_n1_25 %float_0
     %v2bool = OpTypeVector %bool 2
 %float_0_75 = OpConstant %float 0.75
 %float_2_25 = OpConstant %float 2.25
         %53 = OpConstantComposite %v2float %float_0_75 %float_2_25
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
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
         %xy = OpVariable %_ptr_Function_uint Function
         %zw = OpVariable %_ptr_Function_uint Function
         %57 = OpVariable %_ptr_Function_v4float Function
         %30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %34 = OpLoad %v4float %30
         %35 = OpVectorShuffle %v2float %34 %34 0 1
         %29 = OpExtInst %uint %1 PackHalf2x16 %35
               OpStore %xy %29
         %38 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %39 = OpLoad %v4float %38
         %40 = OpVectorShuffle %v2float %39 %39 2 3
         %37 = OpExtInst %uint %1 PackHalf2x16 %40
               OpStore %zw %37
         %42 = OpExtInst %v2float %1 UnpackHalf2x16 %29
         %45 = OpFOrdEqual %v2bool %42 %44
         %47 = OpAll %bool %45
               OpSelectionMerge %49 None
               OpBranchConditional %47 %48 %49
         %48 = OpLabel
         %50 = OpExtInst %v2float %1 UnpackHalf2x16 %37
         %54 = OpFOrdEqual %v2bool %50 %53
         %55 = OpAll %bool %54
               OpBranch %49
         %49 = OpLabel
         %56 = OpPhi %bool %false %25 %55 %48
               OpSelectionMerge %61 None
               OpBranchConditional %56 %59 %60
         %59 = OpLabel
         %62 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %64 = OpLoad %v4float %62
               OpStore %57 %64
               OpBranch %61
         %60 = OpLabel
         %65 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %67 = OpLoad %v4float %65
               OpStore %57 %67
               OpBranch %61
         %61 = OpLabel
         %68 = OpLoad %v4float %57
               OpReturnValue %68
               OpFunctionEnd
