/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
//
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

//
// JSON5 support from @sheredom
//

#include <third_party/json.h/json.h>

//
// 
//

#ifndef CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#endif

#include <assert_cl.h>
#include <find_cl.h>
//
//
//

static is_verbose = false;

//
//
//

#define MAX_ARG_NAME_SIZE    64
#define MAX_ARG_TYPE_SIZE    32

struct skc_kernel_input {
  struct {
    char                 const * name;
    void                       * host;
    char                 const * type;
    size_t                       size;
    size_t                       count;
  } input;

  struct {
    char                 const * filename;
  } output;

  struct {
    char                         name[MAX_ARG_NAME_SIZE];
    char                         type[MAX_ARG_TYPE_SIZE];
    cl_kernel_arg_type_qualifier qualifier;
  } kernel;
  
  void                         * device_copy;
  void                         * device;
};

//
// If we're on Intel HD Graphics then diagnostics are available
//

typedef cl_bitfield cl_diagnostic_verbose_level_intel;

#define CL_CONTEXT_SHOW_DIAGNOSTICS_INTEL           0x4106
#define CL_CONTEXT_DIAGNOSTICS_LEVEL_BAD_INTEL      0x2
#define CL_CONTEXT_DIAGNOSTICS_LEVEL_GOOD_INTEL     0x1
#define CL_CONTEXT_DIAGNOSTICS_LEVEL_NEUTRAL_INTEL  0x4

static
void
CL_CALLBACK
skc_context_callback(char const * error, void const * info, size_t size, void * user)
{
  if (info != NULL )
    {
      fprintf(stderr,"%s\n",error);
    }
}

//
//
//

void
skc_launch_cl_assert(bool const pred, char const * const file, int const line, bool const abort)
{
  if (!pred)
    {
      fprintf(stderr,"skc_runtime_assert: %s %d\n",file,line);      

      if (abort)
        {
          // stop profiling and reset device here if necessary
          exit(EXIT_FAILURE);
        }
    }
}


#define skc_assert(pred) skc_launch_cl_assert(pred, __FILE__, __LINE__, true);

//
//
//

static
void
skc_read_fp(char const * const fname, FILE * fp, void * * const buf, size_t * const length)
{
  //
  // determine size and rewind
  //
  errno_t ferr = fseek(fp,0L,SEEK_END);

  *length = ftell(fp);

  rewind(fp);

  //
  // allocate buffern
  //
  *buf = malloc(*length);

  //
  // load json file
  //
  size_t const fcount = fread(*buf,*length,1,fp);

  if (fcount != 1) {
    fprintf(stderr,"file read failed: %s",fname);
    exit(EXIT_FAILURE);
  }

  //
  // close json file
  //
  fclose(fp);
}

static
void
skc_read_file(char const * const fname, void * * const buf, size_t * const length)
{
  FILE *  fp;
  errno_t ferr = fopen_s(&fp,fname,"rb");

  if (ferr) {
    fprintf(stderr,"file open failed: %s",fname);
    exit(EXIT_FAILURE);
  }

  skc_read_fp(fname,fp,buf,length);
}

//
//
//

static
void
skc_write_file(char const * const fname, void const * const buf, size_t const length)
{
  FILE *  fp;
  errno_t ferr = fopen_s(&fp,fname,"wb");

  if (ferr) {
    fprintf(stderr,"file open failed: %s",fname);
    exit(EXIT_FAILURE);
  }

  //
  // load json file
  //
  size_t const fcount = fwrite(buf,length,1,fp);

  if (fcount != 1) {
    fprintf(stderr,"file write failed: %s",fname);
    exit(EXIT_FAILURE);
  }

  //
  // close json file
  //
  fclose(fp);
}

//
//
//

static
char const *
skc_json_v_to_string(struct json_value_s const * jv)
{
  skc_assert(jv->type == json_type_string);

  struct json_string_s const * js = jv->payload;

  return js->string;
}

static
struct json_object_element_s const *
skc_json_oe_find(struct json_object_element_s const * joe_next, char const * const match)
{
  while (joe_next != NULL)
    {
      if (strcmp(joe_next->name->string,match) == 0)
        break;
      else
        joe_next = joe_next->next;
    }

  return joe_next;
}

//
//
//

static
void
skc_json_find_target(struct json_object_s const   * const jo,
                     char                 const * * const platform_substring,
                     char                 const * * const device_substring)
{
  *platform_substring = NULL;
  *device_substring   = NULL;

  struct json_object_element_s const * joe = skc_json_oe_find(jo->start,"target");

  if (joe != NULL)
    {
      skc_assert(joe->value->type == json_type_object);

      struct json_object_s const * member_jo = joe->value->payload;

      struct json_object_element_s const * joe_platform = skc_json_oe_find(member_jo->start,"platform");
      *platform_substring = skc_json_v_to_string(joe_platform->value);

      struct json_object_element_s const * joe_device = skc_json_oe_find(member_jo->start,"device");
      *device_substring = skc_json_v_to_string(joe_device->value);
    }

  skc_assert(*platform_substring != NULL);
  skc_assert(*device_substring   != NULL);
}

//
//
//

static
void
skc_json_find_program(struct json_object_s const *   const jo,
                      struct json_object_s const * * const jo_source,
                      char                 const * * const kernel_name,
                      char                 const * * const build_options,
                      struct json_array_s  const * * const ja_inputs)
{
  *jo_source     = NULL;
  *kernel_name   = NULL;
  *build_options = "";
  *ja_inputs     = NULL;

  struct json_object_element_s const * joe = skc_json_oe_find(jo->start,"program");

  if (joe != NULL)
    {
      skc_assert(joe->value->type == json_type_object);

      struct json_object_s const * jo = joe->value->payload;

      // json object kernel source
      struct json_object_element_s const * joe_source = skc_json_oe_find(jo->start,"source");
      skc_assert(joe_source->value->type == json_type_object);
      *jo_source = joe_source->value->payload;

      // kernel name
      struct json_object_element_s const * joe_kernel = skc_json_oe_find(jo->start,"kernel");
      *kernel_name = skc_json_v_to_string(joe_kernel->value);

      // kernel options -- optional
      struct json_object_element_s const * joe_options = skc_json_oe_find(jo->start,"options");
      if (joe_options != NULL)
        *build_options = skc_json_v_to_string(joe_options->value);

      // json object kernel inputs
      struct json_object_element_s const * joe_inputs = skc_json_oe_find(jo->start,"inputs");
      skc_assert(joe_inputs->value->type == json_type_array);
      *ja_inputs = joe_inputs->value->payload;
    }

  skc_assert(*jo_source   != NULL);
  skc_assert(*kernel_name != NULL);
  skc_assert(*ja_inputs   != NULL);
}

//
//
//

static
void
skc_create_program(cl_context                         context,
                   cl_device_id                       device_id,
                   struct json_object_s const *       jo_source,
                   cl_program                 * const program)
{
  //
  // might want to do something involving linking later on thus the
  // json_object_s instead of json_object_element_s
  //
  skc_assert(jo_source->length >= 1);

  struct json_object_element_s const * joe = jo_source->start;

  //
  // get source filename
  //
  skc_assert(joe->value->type == json_type_string);

  char const * const fname = skc_json_v_to_string(joe->value);
  void*              fbuf;
  size_t             fsize;

  //
  // open source file
  //
  skc_read_file(fname,&fbuf,&fsize);

  //
  // verbose
  //
  if (is_verbose)
    fprintf(stderr,"Creating (%s)    ... ",joe->name->string);

  //
  // how should program be created form source?
  //
  cl_int cl_err;

  if      (strcmp(joe->name->string,"spir-v") == 0)
    {
      *program = clCreateProgramWithIL(context,fbuf,fsize,&cl_err);
    }
  else if (strcmp(joe->name->string,"binary") == 0)
    {
      cl_int status;

      *program = clCreateProgramWithBinary(context,1,&device_id,
                                           &fsize,(char**)&fbuf,&status,&cl_err);
    }
  else if (strcmp(joe->name->string,"source") == 0)
    {
      *program = clCreateProgramWithSource(context,1,(char**)&fbuf,&fsize,&cl_err);
    }
  else
    {
      fprintf(stderr,"program source type not recognized: %s\n",joe->name->string);
      exit(EXIT_FAILURE);
    }

  cl_ok(cl_err);

  if (is_verbose)
    fprintf(stderr,"done\n");

  free(fbuf);
}

//
//
//

static
void
skc_create_input_element(cl_context                                 context,
                         struct json_object_element_s const * const joe,
                         struct skc_kernel_input            * const kernel_input)
{
  if (strcmp(joe->name->string,"buffer") == 0)
    {
      if (joe->value->type == json_type_string)
        {
          // load the file
          kernel_input->input.size = 1; // we're loading bytes

          skc_read_file(skc_json_v_to_string(joe->value),
                        &kernel_input->input.host,
                        &kernel_input->input.count);
        }
      else
        {
          skc_assert(joe->value->type == json_type_object);

          struct json_object_s         const * const alloc_jo  = joe->value->payload;
          struct json_object_element_s const * const alloc_joe = alloc_jo->start;

          //
          // save type string for verbose feedback
          //
          kernel_input->input.type = alloc_joe->name->string;

          //
          // valid objects for a given type:
          //
          //   allocate: size_t
          //
          //   allocate: [ size_t ]
          //
          //   memset:   [ size_t, <type> ]
          //
          //   sequence: [ size_t, <type>, <type> ]
          //
          char const * start = NULL;
          char const * delta = "0";

          if (alloc_joe->value->type == json_type_number)
            {
              struct json_number_s const * count_jn = alloc_joe->value->payload;

              skc_assert(sscanf_s(count_jn->number,"%zu",&kernel_input->input.count) == 1);
            }
          else if (alloc_joe->value->type == json_type_array)
            {
              struct json_array_s         const * const ja  = alloc_joe->value->payload;
              struct json_array_element_s const * const jae = ja->start;

              if ((ja->length >= 1) && (ja->length <= 3))
                {
                  skc_assert(jae->value->type == json_type_number);

                  struct json_number_s const * count_jn = jae->value->payload;

                  skc_assert(sscanf_s(count_jn->number,"%zu",&kernel_input->input.count) == 1);

                  if (ja->length >= 2)
                    {
                      struct json_array_element_s const * const start_jae = jae->next;

                      skc_assert(start_jae->value->type == json_type_number);

                      struct json_number_s const * start_jn = start_jae->value->payload;

                      start = start_jn->number;

                      if (ja->length == 3)
                        {
                          struct json_array_element_s const * const delta_jae = start_jae->next;

                          skc_assert(delta_jae->value->type == json_type_number);

                          struct json_number_s const * delta_jn = delta_jae->value->payload;

                          delta = delta_jn->number;
                        }
                    }
                }
              else
                {
                  fprintf(stderr,"buffer initialization array size not valid: %zu\n",ja->length);
                  exit(EXIT_FAILURE);
                }
            }
          else
            {
              fprintf(stderr,"buffer initialization type not recognized: %zu\n",alloc_joe->value->type);
              exit(EXIT_FAILURE);
            }

          //
          // allocate and initialize an extent
          //
#define SKC_KERNEL_INPUT_INIT(TYPE,STR,SPEC)                            \
          (strcmp(alloc_joe->name->string,STR) == 0) {                  \
            kernel_input->input.size = sizeof(TYPE);                    \
            kernel_input->input.host = malloc(sizeof(TYPE) * kernel_input->input.count); \
            if (start != NULL) {                                        \
              TYPE vs=0,vd=0;                                           \
              skc_assert(sscanf_s(start,SPEC,&vs) == 1);                    \
              skc_assert(sscanf_s(delta,SPEC,&vd) == 1);                    \
              for (size_t ii=0; ii<kernel_input->input.count; ii++) {   \
                ((TYPE*)kernel_input->input.host)[ii] = vs;             \
                vs += vd;                                               \
              }                                                         \
            }                                                           \
          }

          //
          // what is type?
          //
          if      SKC_KERNEL_INPUT_INIT(cl_char,  "char",  "%hhd")
          else if SKC_KERNEL_INPUT_INIT(cl_uchar, "uchar", "%hhu")
          else if SKC_KERNEL_INPUT_INIT(cl_short, "short", "%hd")
          else if SKC_KERNEL_INPUT_INIT(cl_ushort,"ushort","%hu")
          else if SKC_KERNEL_INPUT_INIT(cl_int,   "int",   "%d")
          else if SKC_KERNEL_INPUT_INIT(cl_uint,  "uint",  "%u")
          else if SKC_KERNEL_INPUT_INIT(cl_long,  "long",  "%lld")
          else if SKC_KERNEL_INPUT_INIT(cl_ulong, "ulong", "%llu")
          else if SKC_KERNEL_INPUT_INIT(cl_float, "float", "%f")
          else if SKC_KERNEL_INPUT_INIT(cl_double,"double","%lf")
          else
            {
              fprintf(stderr,"type not recognized: %s\n",alloc_joe->name->string);
              exit(EXIT_FAILURE);
            }
        }
    }
  else if (strcmp(joe->name->string,"output") == 0)
    {
      skc_assert(joe->value->type == json_type_string);
      // save path to output file
      kernel_input->output.filename = skc_json_v_to_string(joe->value);
    }
  else if (strcmp(joe->name->string,"local") == 0)
    {
      fprintf(stderr,"\"local\" not implemented\n");
    }
  else if (strcmp(joe->name->string,"image") == 0)
    {
      fprintf(stderr,"\"image\" not implemented\n");
    }
  else if (strcmp(joe->name->string,"sampler") == 0)
    {
      fprintf(stderr,"\"sampler\" not implemented\n");
    }
  else if (strcmp(joe->name->string,"pipe") == 0)
    {
      fprintf(stderr,"\"pipe\" not implemented\n");
    }
  else
    {
      fprintf(stderr,"input type not recognized: %s\n",joe->name->string);
      exit(EXIT_FAILURE);
    }
}

static
void
skc_create_inputs(cl_context                                context,
                  cl_kernel                                 kernel,
                  cl_uint                                   kernel_arg_count,
                  struct json_array_element_s const *       jae,
                  struct skc_kernel_input           * const kernel_inputs)
{
  //
  // probe kernel argument name, type and qualifier(s)
  //
  for (cl_uint ii=0; ii<kernel_arg_count; ii++)
    {
      cl(GetKernelArgInfo(kernel,ii,
                          CL_KERNEL_ARG_NAME,
                          MAX_ARG_NAME_SIZE,
                          kernel_inputs[ii].kernel.name,
                          NULL));

      cl(GetKernelArgInfo(kernel,ii,
                          CL_KERNEL_ARG_TYPE_NAME,
                          MAX_ARG_TYPE_SIZE,
                          kernel_inputs[ii].kernel.type,
                          NULL));
      
      cl(GetKernelArgInfo(kernel,ii,
                          CL_KERNEL_ARG_TYPE_QUALIFIER,
                          sizeof(cl_kernel_arg_type_qualifier),
                          &kernel_inputs[ii].kernel.qualifier,
                          NULL));
    }

  //
  // allocate host side
  //
  for (cl_uint ii=0; ii<kernel_arg_count; ii++)
    {
      // make sure array element is object
      skc_assert(jae->value->type == json_type_object);
      struct json_object_s         const * const elem_jo  = jae->value->payload;
      struct json_object_element_s const * const elem_joe = elem_jo->start;

      // save string and initialize defaults
      kernel_inputs[ii].input.name      = elem_joe->name->string;
      kernel_inputs[ii].input.type      = "<unknown>";
      kernel_inputs[ii].input.size      = 0;
      kernel_inputs[ii].input.count     = 0;
      kernel_inputs[ii].output.filename = NULL;

      // make sure object value is object
      skc_assert(elem_joe->value->type == json_type_object);
      struct json_object_s         const * const val_jo  = elem_joe->value->payload;
      struct json_object_element_s const *       val_joe = val_jo->start;

      while (val_joe != NULL)
        {
          // allocate host and device inputs
          skc_create_input_element(context,val_joe,kernel_inputs+ii);
          // next element
          val_joe = val_joe->next;
        }

      // next!
      jae = jae->next;
    }

  //
  // now allocate device side buffers with hints from kernel quals
  //
  for (cl_uint ii=0; ii<kernel_arg_count; ii++)
    {
      bool const   is_readonly = kernel_inputs[ii].kernel.qualifier & CL_KERNEL_ARG_TYPE_CONST;

      cl_mem_flags flags       = CL_MEM_COPY_HOST_PTR | (is_readonly ? CL_MEM_READ_ONLY : CL_MEM_READ_WRITE);
      size_t       size        = kernel_inputs[ii].input.size * kernel_inputs[ii].input.count;

      cl_int       cl_err;
      cl_mem       mem         = clCreateBuffer(context,flags,size,kernel_inputs[ii].input.host,&cl_err);

      cl_ok(cl_err);
                               
      if (is_readonly)
        {
          kernel_inputs[ii].device_copy = NULL;
          kernel_inputs[ii].device      = mem;

          free(kernel_inputs[ii].input.host); // we don't need to keep host data

          kernel_inputs[ii].input.host  = NULL;
        }
      else
        {
          kernel_inputs[ii].device_copy = mem;
          kernel_inputs[ii].device      = clCreateBuffer(context,CL_MEM_READ_WRITE,size,NULL,&cl_err);

          free(kernel_inputs[ii].input.host); // we don't need to keep host data

          kernel_inputs[ii].input.host  = NULL;
        }
    }
}

//
//
//

static 
void
skc_json_find_execution(struct json_object_s const * const jo,
                        cl_uint                    * const work_dim,
                        size_t                     * const work_global,
                        size_t                     * const work_local,
                        size_t                     * const bench_loops)
{
  //
  // initialize to defaults
  //
  *work_dim      = 1;

  work_global[0] = 1;
  work_global[1] = 1;
  work_global[2] = 1;

  work_local [0] = 1;
  work_local [1] = 1;
  work_local [2] = 1;

  *bench_loops   = 10;

  // 
  // find execution member 
  //
  struct json_object_element_s const * joe = skc_json_oe_find(jo->start,"execution");

  if (joe != NULL)
    {
      skc_assert(joe->value->type == json_type_object);

      struct json_object_s const * member_jo = joe->value->payload;

      //
      // set up grid
      //
      struct json_object_element_s const * joe_uniform = skc_json_oe_find(member_jo->start,"uniform");

      if (joe_uniform != NULL)
        {
          skc_assert(joe_uniform->value->type == json_type_array);      

          struct json_array_s         const * const ja        = joe_uniform->value->payload;
          struct json_array_element_s const * const scale_jae = ja->start;

          skc_assert(ja->length             == 2);
          skc_assert(scale_jae->value->type == json_type_number);

          struct json_number_s        const * const scale_jn = scale_jae->value->payload;
          
          size_t scale_global = 0; 

          skc_assert(sscanf_s(scale_jn->number,"%zu",&scale_global) == 1);          

          struct json_array_element_s const * const local_jae = scale_jae->next;

          skc_assert(local_jae->value->type == json_type_array);

          struct json_array_s         const * const local_ja = local_jae->value->payload;
          struct json_array_element_s const *       dim_jae  = local_ja->start;

          *work_dim = min((cl_uint)local_ja->length,3);

          for (cl_uint ii=0; ii<*work_dim; ii++)
            {
              skc_assert(dim_jae->value->type == json_type_number);

              struct json_number_s const * const dim_jn = dim_jae->value->payload;
          
              skc_assert(sscanf_s(dim_jn->number,"%zu",work_local+ii) == 1);       

              // scale up global grid
              work_global[ii] = work_local[ii] * scale_global;
            }
        }

      //
      // set up iterations
      //
      struct json_object_element_s const * joe_iterations = skc_json_oe_find(member_jo->start,"iterations");

      if (joe_iterations != NULL)
        {
          skc_assert(joe_iterations->value->type == json_type_number);
          
          struct json_number_s const * bench_loops_jn = joe_iterations->value->payload;

          skc_assert(sscanf_s(bench_loops_jn->number,"%zu",bench_loops) == 1);
        }
    }
}

//
//
//

static
void
skc_verbose_kernel_info(char const * const device_name,
                        cl_device_id       device_id,
                        char const * const kernel_name,
                        cl_kernel          kernel)
{
  if (strstr(device_name,"HD Graphics") != NULL)
    {
#define CL_DEVICE_SUB_GROUP_SIZES_INTEL         0x4108
#define CL_KERNEL_SPILL_MEM_SIZE_INTEL          0x4109
#define CL_KERNEL_COMPILE_SUB_GROUP_SIZE_INTEL  0x410A

#if SKC_IS_THIS_AN_OPENCL_2_1_PLATFORM_NO_NO_NOT_YET
      size_t sub_group_size;
  
      cl(GetKernelSubGroupInfo(kernel,device_id,
                               CL_KERNEL_COMPILE_SUB_GROUP_SIZE_INTEL,
                               0,NULL,
                               sizeof(sub_group_size),&sub_group_size,
                               NULL));

      printf("kernel \"%s\" subgroup size:  %5zu bytes\n",
             kernel_name,sub_group_size);
#endif

      cl_ulong spill_mem_size;

      cl(GetKernelWorkGroupInfo(kernel,device_id,
                                CL_KERNEL_SPILL_MEM_SIZE_INTEL,
                                sizeof(spill_mem_size),&spill_mem_size,
                                NULL));

      printf("kernel \"%s\" spill mem size: %5llu bytes\n",
             kernel_name,spill_mem_size);

      cl_ulong local_mem_size;

      cl(GetKernelWorkGroupInfo(kernel,device_id,
                                CL_KERNEL_LOCAL_MEM_SIZE,
                                sizeof(local_mem_size),&local_mem_size,
                                NULL));

      printf("kernel \"%s\" local mem size: %5llu bytes\n",
             kernel_name,local_mem_size);
    }
}

//
//
//

int
main(int argc, char** argv)
{
  //
  // Usage: <exe> [-v] [-|filename]
  //

  //
  // defaults
  //
  FILE * fp    = stdin;
  char * fname = "<stdin>";

  //
  // test for switches and filename
  //
  if (argc > 1)
    {
      int arg_idx = 1;

      if (strcmp(argv[arg_idx],"-v") == 0)
        {
          is_verbose = true;
          arg_idx   += 1;
        }

      if (arg_idx < argc)
        {
          if (strcmp(argv[arg_idx],"-") != 0)
            {
              fname        = argv[arg_idx];
              errno_t ferr = fopen_s(&fp,fname,"rb");

              if (ferr) {
                fprintf(stderr,"file open failed: %s",fname);
                exit(EXIT_FAILURE);
              }
            }
        }
    }

  void * fbuf;
  size_t fsize;

  skc_read_fp(fname,fp,&fbuf,&fsize);

  //
  // parse json source
  //
  struct json_parse_result_s  jres;
  struct json_value_s * const json = json_parse_ex(fbuf,fsize,
                                                   json_parse_flags_allow_json5,
                                                   NULL,NULL,&jres);
  //
  // dispose of buf
  //
  free(fbuf);

  //
  // was there an error?
  //
  if (jres.error)
    {
      fprintf(stderr,"%s:%zu error %zu\n",fname,jres.error_line_no,jres.error);
      exit(EXIT_FAILURE);
    }

  //
  // we are expecting a top level object
  //
  skc_assert(json->type == json_type_object);

  struct json_object_s const * const jo = json->payload;

  //
  // debug print
  //
#if 0
  char * jstr = json_write_pretty(json,NULL,NULL,NULL);
  fprintf(stderr,"%s\n",jstr);
  free(jstr);
#endif

  //
  // find platform and device strings
  //
  char const * target_platform_substring;
  char const * target_device_substring;

  skc_json_find_target(jo,&target_platform_substring,&target_device_substring);

  //
  // find platform and device ids
  //
  cl_platform_id platform_id;
  cl_device_id   device_id;

#define HS_DEVICE_NAME_SIZE  64

  char   device_name[HS_DEVICE_NAME_SIZE];
  size_t device_name_size;

  cl(FindIdsByName(target_platform_substring,
                   target_device_substring,
                   &platform_id,
                   &device_id,
                   HS_DEVICE_NAME_SIZE,
                   device_name,
                   &device_name_size,
                   true));
  
  //
  // create context
  //
  cl_context_properties context_properties[] =
    {
      CL_CONTEXT_PLATFORM,(cl_context_properties)platform_id,
      0
    };

  cl_int     cl_err;
  cl_context context = clCreateContext(context_properties,
                                       1,
                                       &device_id,
                                       skc_context_callback,
                                       NULL,
                                       &cl_err);
  cl_ok(cl_err);

  //
  // create command queue
  //
  cl_command_queue_properties const props = CL_QUEUE_PROFILING_ENABLE;

#if 0 // OPENCL 2.0
  cl_queue_properties queue_properties[] =
    {
      CL_QUEUE_PROPERTIES, (cl_queue_properties)props,
      0
    };

  cl_command_queue cq = clCreateCommandQueueWithProperties(context,
                                                           device_id,
                                                           queue_properties,
                                                           &cl_err); cl_ok(cl_err);
#else // OPENCL 1.2
  cl_command_queue cq = clCreateCommandQueue(context,
                                             device_id,
                                             props,
                                             &cl_err); cl_ok(cl_err);
#endif

  //
  // find the program, kernel name and its args
  //
  struct json_object_s const * jo_source;
  char                 const * kernel_name;
  char                 const * build_options;
  struct json_array_s  const * ja_inputs;

  skc_json_find_program(jo,&jo_source,&kernel_name,&build_options,&ja_inputs);

  //
  // CREATE THE PROGRAM
  //
  cl_program program;

  skc_create_program(context,device_id,jo_source,&program);

  //
  // BUILD THE PROGRAM
  //
  if (is_verbose)
    fprintf(stderr,"Building             ... ");

  cl(BuildProgram(program,
                  1,
                  &device_id,
                  build_options,
                  NULL,
                  NULL));

  if (is_verbose)
    fprintf(stderr,"done\n");

  //
  // CREATE THE KERNEL
  //
  cl_kernel kernel = clCreateKernel(program,kernel_name,&cl_err);

  cl_ok(cl_err);

  //
  // we don't need the program anymore
  //
  cl(ReleaseProgram(program));

  //
  // probe the kernel to see how many args there are
  //
  cl_uint kernel_arg_count;

  cl(GetKernelInfo(kernel,CL_KERNEL_NUM_ARGS,sizeof(kernel_arg_count),&kernel_arg_count,NULL));

  //
  // first safety check -- do the arg counts match?
  //
  if (kernel_arg_count != ja_inputs->length)
    {
      fprintf(stderr,"kernel args does not equal input args: %zu != %u\n",
              ja_inputs->length,kernel_arg_count);
      exit(EXIT_FAILURE);
    }

  //
  // ALLOCATE INPUTS
  //
  // - BUFFER, LOCAL, IMAGE, SAMPLER, PIPE
  //

  struct skc_kernel_input * kernel_inputs = malloc(sizeof(*kernel_inputs) * kernel_arg_count);

  skc_create_inputs(context,kernel,kernel_arg_count,ja_inputs->start,kernel_inputs);

  //
  // SET KERNEL ARGS
  //
  for (cl_uint ii=0; ii<kernel_arg_count; ii++)
    {
      cl(SetKernelArg(kernel,ii,sizeof(void*),&kernel_inputs[ii].device));
    }

  //
  // FIND LAUNCH CONFIGURATION
  //
  cl_uint work_dim;
  size_t  work_global[3], work_local[3], bench_loops;

  skc_json_find_execution(jo,&work_dim,work_global,work_local,&bench_loops);

  //
  // how large is the grid?
  //
  if (is_verbose)
    {
      fprintf(stderr,"Executing            ... { %zu, %zu, %zu } x { %zu, %zu, %zu }\n",
              work_global[0],work_global[1],work_global[2],
              work_local[0],work_local[1],work_local[2]);
    }

  //
  // LAUNCH AND TIME KERNEL
  //
  cl_ulong elapsed_ns_avg = 0L;
  cl_ulong elapsed_ns_min = UINT64_MAX;
  cl_ulong elapsed_ns_max = 0L;

  if (is_verbose)
    fprintf(stderr,"Benchmarking (%5zu) ... ",bench_loops);

  for (size_t ii=0; ii<bench_loops; ii++)
    {
      //
      // refresh all mutable args
      //
      for (cl_uint ii=0; ii<kernel_arg_count; ii++)
        {
          if (kernel_inputs[ii].device_copy != NULL)
            {
              size_t size = kernel_inputs[ii].input.size * kernel_inputs[ii].input.count;

              cl(EnqueueCopyBuffer(cq,
                                   kernel_inputs[ii].device_copy,
                                   kernel_inputs[ii].device,
                                   0,0,size,0,NULL,NULL));
            }
        }

      cl_event start_end;
      
      cl(EnqueueNDRangeKernel(cq,kernel,
                              work_dim,NULL,work_global,work_local,
                              0,NULL,&start_end));

      cl(Finish(cq));

      cl_ulong start_time,end_time;

      cl(GetEventProfilingInfo(start_end,CL_PROFILING_COMMAND_START,sizeof(cl_ulong),&start_time,NULL));
      cl(GetEventProfilingInfo(start_end,CL_PROFILING_COMMAND_END,  sizeof(cl_ulong),&end_time,  NULL));

      cl_ulong elapsed_ns = end_time - start_time;

      elapsed_ns_avg += elapsed_ns;
      elapsed_ns_min  = min(elapsed_ns_min,elapsed_ns);
      elapsed_ns_max  = max(elapsed_ns_max,elapsed_ns);

      cl(ReleaseEvent(start_end));
    }

  if (is_verbose)
    fprintf(stderr,"done\n");

  //
  // SAVE OUTPUTS THAT HAVE NON-NULL PATH
  //
  if (is_verbose)
    fprintf(stderr,"Writing              ... ");

  for (cl_uint ii=0; ii<kernel_arg_count; ii++)
    {
      if (kernel_inputs[ii].output.filename != NULL)
        {
          size_t size = kernel_inputs[ii].input.size * kernel_inputs[ii].input.count;

          // allocate temporary buffer
          void * buf = malloc(size);;

          // read buffer
          cl(EnqueueReadBuffer(cq,
                               kernel_inputs[ii].device,
                               CL_TRUE, // blocking read,
                               0,size,buf,
                               0,NULL,NULL));
          

          // write it out
          skc_write_file(kernel_inputs[ii].output.filename,buf,size);

          // free temporary buffer
          free(buf);
        }
    }

  if (is_verbose)
    fprintf(stderr,"done\n");

  if (is_verbose)
    fprintf(stderr,"\n");

  //
  // dump any extra device-specific info
  //
  if (is_verbose) 
    {
      skc_verbose_kernel_info(device_name,device_id,kernel_name,kernel);
      fprintf(stderr,"\n");
    }

  //
  // REPORT BENCHMARK RESULTS
  //
  fprintf(stderr,
          "device, driver, iterations, avg (usecs), min (usecs), max (usecs)\n"
          "%s, %s, %zu, %6.2f, %6.2f, %6.2f\n",
          device_name,
          "FIXME",
          bench_loops,
          (double)elapsed_ns_avg / 1000.0 / bench_loops,
          (double)elapsed_ns_min / 1000.0,
          (double)elapsed_ns_max / 1000.0);


  //
  // FIXME -- missing a bunch of resource cleanup 
  //

  //
  // RELEASE KERNEL, PROGRAM, OBJECT RESOURCES
  //

  //
  // SHUT DOWN CL ENVIRONMENT
  //

  //
  // free everything
  //
  free(json);
  free(kernel_inputs); // allocations inside need to be freed!

  return 0;
}
