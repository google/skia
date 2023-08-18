               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorRed"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %result "result"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %35 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_2 = OpConstant %float 2
         %37 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
    %float_3 = OpConstant %float 3
         %40 = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
      %int_1 = OpConstant %int 1
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
     %result = OpVariable %_ptr_Function_v4bool Function
         %45 = OpVariable %_ptr_Function_v4float Function
         %31 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %35 = OpLoad %v4float %31
         %30 = OpFOrdLessThan %v4bool %35 %37
         %41 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %43 = OpLoad %v4float %41
         %38 = OpFOrdGreaterThan %v4bool %40 %43
         %29 = OpLogicalEqual %v4bool %30 %38
               OpStore %result %29
         %44 = OpAll %bool %29
               OpSelectionMerge %49 None
               OpBranchConditional %44 %47 %48
         %47 = OpLabel
         %50 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %51 = OpLoad %v4float %50
               OpStore %45 %51
               OpBranch %49
         %48 = OpLabel
         %52 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %53 = OpLoad %v4float %52
               OpStore %45 %53
               OpBranch %49
         %49 = OpLabel
         %54 = OpLoad %v4float %45
               OpReturnValue %54
               OpFunctionEnd
