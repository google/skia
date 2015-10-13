#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
  if ( argc < 3 )
    {
    fprintf(stderr, "Usage: %s <file1> <file2>\n", argv[0]);
    return 1;
    }
  FILE *fp = fopen(argv[1],"w");
  fprintf(fp,"extern int wrapped_help();\n");
  fprintf(fp,"int wrapped() { return wrapped_help(); }\n");
  fclose(fp);
  fp = fopen(argv[2],"w");
  fprintf(fp,"int wrapped_help() { return 5; }\n");
  fclose(fp);
#ifdef CMAKE_INTDIR
  /* The VS6 IDE passes a leading ".\\" in its variable expansion.  */
# if defined(_MSC_VER) && _MSC_VER == 1200
#  define CFG_DIR ".\\" CMAKE_INTDIR
# else
#  define CFG_DIR CMAKE_INTDIR
# endif
  const char* cfg = (argc >= 4)? argv[3] : "";
  if(strcmp(cfg, CFG_DIR) != 0)
    {
    fprintf(stderr,
            "Did not receive expected configuration argument:\n"
            "  expected [" CFG_DIR "]\n"
            "  received [%s]\n", cfg);
    return 1;
    }
#endif
  return 0;
}
