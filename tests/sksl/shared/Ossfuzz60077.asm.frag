               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_entrypoint_v "_entrypoint_v"    ; id %11
               OpName %d_vi "d_vi"                      ; id %2
               OpName %b "b"                            ; id %26
               OpName %c_vi "c_vi"                      ; id %3
               OpName %b_vi "b_vi"                      ; id %4
               OpName %a_vi "a_vi"                      ; id %5
               OpName %main "main"                      ; id %6
               OpName %i "i"                            ; id %49

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
         %23 = OpTypeFunction %void %_ptr_Function_int
      %int_4 = OpConstant %int 4
         %46 = OpTypeFunction %v4float %_ptr_Function_v2float
         %53 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %13

         %14 = OpLabel
         %18 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %18 %17
         %20 =   OpFunctionCall %v4float %main %18
                 OpStore %sk_FragColor %20
                 OpReturn
               OpFunctionEnd


               ; Function d_vi
       %d_vi = OpFunction %void None %23
         %24 = OpFunctionParameter %_ptr_Function_int

         %25 = OpLabel
          %b =   OpVariable %_ptr_Function_int Function
                 OpStore %b %int_4
                 OpReturn
               OpFunctionEnd


               ; Function c_vi
       %c_vi = OpFunction %void None %23
         %28 = OpFunctionParameter %_ptr_Function_int

         %29 = OpLabel
         %31 =   OpVariable %_ptr_Function_int Function
         %30 =   OpLoad %int %28
                 OpStore %31 %30
         %32 =   OpFunctionCall %void %d_vi %31
                 OpReturn
               OpFunctionEnd


               ; Function b_vi
       %b_vi = OpFunction %void None %23
         %33 = OpFunctionParameter %_ptr_Function_int

         %34 = OpLabel
         %36 =   OpVariable %_ptr_Function_int Function
         %35 =   OpLoad %int %33
                 OpStore %36 %35
         %37 =   OpFunctionCall %void %c_vi %36
                 OpReturn
               OpFunctionEnd


               ; Function a_vi
       %a_vi = OpFunction %void None %23
         %38 = OpFunctionParameter %_ptr_Function_int

         %39 = OpLabel
         %41 =   OpVariable %_ptr_Function_int Function
         %44 =   OpVariable %_ptr_Function_int Function
         %40 =   OpLoad %int %38
                 OpStore %41 %40
         %42 =   OpFunctionCall %void %b_vi %41
         %43 =   OpLoad %int %38
                 OpStore %44 %43
         %45 =   OpFunctionCall %void %b_vi %44
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %46         ; RelaxedPrecision
         %47 = OpFunctionParameter %_ptr_Function_v2float

         %48 = OpLabel
          %i =   OpVariable %_ptr_Function_int Function
         %51 =   OpVariable %_ptr_Function_int Function
         %50 =   OpLoad %int %i
                 OpStore %51 %50
         %52 =   OpFunctionCall %void %a_vi %51
                 OpReturnValue %53
               OpFunctionEnd
