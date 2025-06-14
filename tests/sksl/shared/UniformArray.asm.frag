               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %14
               OpMemberName %_UniformBuffer 0 "testArray"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %16
               OpName %main "main"                      ; id %6
               OpName %index "index"                    ; id %29

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %_arr_float_int_5 ArrayStride 16
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 1 Offset 80
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 96
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %56 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
      %int_5 = OpConstant %int 5
%_arr_float_int_5 = OpTypeArray %float %int_5       ; ArrayStride 16
%_UniformBuffer = OpTypeStruct %_arr_float_int_5 %v4float %v4float  ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %26 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
%_ptr_Uniform__arr_float_int_5 = OpTypePointer Uniform %_arr_float_int_5
%_ptr_Uniform_float = OpTypePointer Uniform %float
      %int_1 = OpConstant %int 1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_2 = OpConstant %int 2


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %18

         %19 = OpLabel
         %23 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %23 %22
         %25 =   OpFunctionCall %v4float %main %23
                 OpStore %sk_FragColor %25
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %26         ; RelaxedPrecision
         %27 = OpFunctionParameter %_ptr_Function_v2float

         %28 = OpLabel
      %index =   OpVariable %_ptr_Function_int Function
                 OpStore %index %int_0
                 OpBranch %32

         %32 = OpLabel
                 OpLoopMerge %36 %35 None
                 OpBranch %33

         %33 =     OpLabel
         %37 =       OpLoad %int %index
         %38 =       OpSLessThan %bool %37 %int_5
                     OpBranchConditional %38 %34 %36

         %34 =         OpLabel
         %40 =           OpAccessChain %_ptr_Uniform__arr_float_int_5 %11 %int_0
         %42 =           OpLoad %int %index
         %43 =           OpAccessChain %_ptr_Uniform_float %40 %42
         %45 =           OpLoad %float %43
         %46 =           OpLoad %int %index
         %48 =           OpIAdd %int %46 %int_1
         %49 =           OpConvertSToF %float %48
         %50 =           OpFUnordNotEqual %bool %45 %49
                         OpSelectionMerge %52 None
                         OpBranchConditional %50 %51 %52

         %51 =             OpLabel
         %53 =               OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %56 =               OpLoad %v4float %53    ; RelaxedPrecision
                             OpReturnValue %56

         %52 =         OpLabel
                         OpBranch %35

         %35 =   OpLabel
         %57 =     OpLoad %int %index
         %58 =     OpIAdd %int %57 %int_1
                   OpStore %index %58
                   OpBranch %32

         %36 = OpLabel
         %59 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %60 =   OpLoad %v4float %59                ; RelaxedPrecision
                 OpReturnValue %60
               OpFunctionEnd
