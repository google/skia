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
               OpMemberName %_UniformBuffer 2 "testArray"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %one "one"
               OpName %two "two"
               OpName %three "three"
               OpName %four "four"
               OpName %five "five"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %_arr_float_int_5 ArrayStride 16
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %four RelaxedPrecision
               OpDecorate %five RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
        %int = OpTypeInt 32 1
      %int_5 = OpConstant %int 5
%_arr_float_int_5 = OpTypeArray %float %int_5
%_UniformBuffer = OpTypeStruct %v4float %v4float %_arr_float_int_5
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %26 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform__arr_float_int_5 = OpTypePointer Uniform %_arr_float_int_5
      %int_2 = OpConstant %int 2
      %int_0 = OpConstant %int 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
      %int_4 = OpConstant %int 4
      %false = OpConstantFalse %bool
    %float_5 = OpConstant %float 5
   %float_17 = OpConstant %float 17
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %18
         %19 = OpLabel
         %23 = OpVariable %_ptr_Function_v2float Function
               OpStore %23 %22
         %25 = OpFunctionCall %v4float %main %23
               OpStore %sk_FragColor %25
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %26
         %27 = OpFunctionParameter %_ptr_Function_v2float
         %28 = OpLabel
        %one = OpVariable %_ptr_Function_float Function
        %two = OpVariable %_ptr_Function_float Function
      %three = OpVariable %_ptr_Function_float Function
       %four = OpVariable %_ptr_Function_float Function
       %five = OpVariable %_ptr_Function_float Function
         %67 = OpVariable %_ptr_Function_v4float Function
         %31 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %10 %int_2
         %35 = OpAccessChain %_ptr_Uniform_float %31 %int_0
         %37 = OpLoad %float %35
               OpStore %one %37
         %39 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %10 %int_2
         %41 = OpAccessChain %_ptr_Uniform_float %39 %int_1
         %42 = OpLoad %float %41
               OpStore %two %42
         %44 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %10 %int_2
         %45 = OpAccessChain %_ptr_Uniform_float %44 %int_2
         %46 = OpLoad %float %45
               OpStore %three %46
         %48 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %10 %int_2
         %50 = OpAccessChain %_ptr_Uniform_float %48 %int_3
         %51 = OpLoad %float %50
               OpStore %four %51
         %53 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %10 %int_2
         %55 = OpAccessChain %_ptr_Uniform_float %53 %int_4
         %56 = OpLoad %float %55
               OpStore %five %56
         %58 = OpExtInst %float %1 Fma %37 %42 %46
         %60 = OpFOrdEqual %bool %58 %float_5
               OpSelectionMerge %62 None
               OpBranchConditional %60 %61 %62
         %61 = OpLabel
         %63 = OpExtInst %float %1 Fma %46 %51 %56
         %65 = OpFOrdEqual %bool %63 %float_17
               OpBranch %62
         %62 = OpLabel
         %66 = OpPhi %bool %false %28 %65 %61
               OpSelectionMerge %71 None
               OpBranchConditional %66 %69 %70
         %69 = OpLabel
         %72 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %74 = OpLoad %v4float %72
               OpStore %67 %74
               OpBranch %71
         %70 = OpLabel
         %75 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %76 = OpLoad %v4float %75
               OpStore %67 %76
               OpBranch %71
         %71 = OpLabel
         %77 = OpLoad %v4float %67
               OpReturnValue %77
               OpFunctionEnd
