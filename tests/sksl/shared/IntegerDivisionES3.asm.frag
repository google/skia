               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %zero "zero"
               OpName %one "one"
               OpName %x "x"
               OpName %y "y"
               OpName %_0_x "_0_x"
               OpName %_1_result "_1_result"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
    %int_100 = OpConstant %int 100
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %float 1
%float_0_00392156886 = OpConstant %float 0.00392156886
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
       %zero = OpVariable %_ptr_Function_int Function
        %one = OpVariable %_ptr_Function_int Function
          %x = OpVariable %_ptr_Function_int Function
          %y = OpVariable %_ptr_Function_int Function
       %_0_x = OpVariable %_ptr_Function_int Function
  %_1_result = OpVariable %_ptr_Function_int Function
         %29 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %32 = OpLoad %v4float %29
         %33 = OpCompositeExtract %float %32 0
         %34 = OpConvertFToS %int %33
               OpStore %zero %34
         %36 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %37 = OpLoad %v4float %36
         %38 = OpCompositeExtract %float %37 1
         %39 = OpConvertFToS %int %38
               OpStore %one %39
               OpStore %x %34
               OpBranch %41
         %41 = OpLabel
               OpLoopMerge %45 %44 None
               OpBranch %42
         %42 = OpLabel
         %46 = OpLoad %int %x
         %48 = OpSLessThan %bool %46 %int_100
               OpBranchConditional %48 %43 %45
         %43 = OpLabel
         %50 = OpLoad %int %one
               OpStore %y %50
               OpBranch %51
         %51 = OpLabel
               OpLoopMerge %55 %54 None
               OpBranch %52
         %52 = OpLabel
         %56 = OpLoad %int %y
         %57 = OpSLessThan %bool %56 %int_100
               OpBranchConditional %57 %53 %55
         %53 = OpLabel
         %59 = OpLoad %int %x
               OpStore %_0_x %59
               OpStore %_1_result %int_0
               OpBranch %61
         %61 = OpLabel
               OpLoopMerge %65 %64 None
               OpBranch %62
         %62 = OpLabel
         %66 = OpLoad %int %_0_x
         %67 = OpLoad %int %y
         %68 = OpSGreaterThanEqual %bool %66 %67
               OpBranchConditional %68 %63 %65
         %63 = OpLabel
         %70 = OpLoad %int %_1_result
         %71 = OpIAdd %int %70 %int_1
               OpStore %_1_result %71
         %72 = OpLoad %int %_0_x
         %73 = OpLoad %int %y
         %74 = OpISub %int %72 %73
               OpStore %_0_x %74
               OpBranch %64
         %64 = OpLabel
               OpBranch %61
         %65 = OpLabel
         %75 = OpLoad %int %x
         %76 = OpLoad %int %y
         %77 = OpSDiv %int %75 %76
         %78 = OpLoad %int %_1_result
         %79 = OpINotEqual %bool %77 %78
               OpSelectionMerge %81 None
               OpBranchConditional %79 %80 %81
         %80 = OpLabel
         %83 = OpLoad %int %x
         %84 = OpConvertSToF %float %83
         %86 = OpFMul %float %84 %float_0_00392156886
         %87 = OpLoad %int %y
         %88 = OpConvertSToF %float %87
         %89 = OpFMul %float %88 %float_0_00392156886
         %90 = OpCompositeConstruct %v4float %float_1 %86 %89 %float_1
               OpReturnValue %90
         %81 = OpLabel
               OpBranch %54
         %54 = OpLabel
         %91 = OpLoad %int %y
         %92 = OpIAdd %int %91 %int_1
               OpStore %y %92
               OpBranch %51
         %55 = OpLabel
               OpBranch %44
         %44 = OpLabel
         %93 = OpLoad %int %x
         %94 = OpIAdd %int %93 %int_1
               OpStore %x %94
               OpBranch %41
         %45 = OpLabel
         %95 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %96 = OpLoad %v4float %95
               OpReturnValue %96
               OpFunctionEnd
