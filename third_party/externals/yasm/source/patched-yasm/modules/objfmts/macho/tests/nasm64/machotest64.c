/*
 * test source file for assembling to Mach-O 
 * copied from cofftest.c, adapted to current limitations 
 * in Mach-O module
 * build with (under OSX Tiger/Leopard, for example):
 *    yasm -f macho -m amd64 machotest64.asm
 *    gcc -m64 -o machotest64 machotest64.c machotest64.o
 */

#include <stdio.h>

extern long lrotate(long, long);
extern void greet(void);
extern long readgreet(void);
extern char asmstr[];
extern void *selfptr;
extern void *textptr;
extern int integer, commvar;
extern char *getstr(void);

int main(void) {

    printf("Testing lrotate: should get 0x0000000000400000, 0x0000000000000001\n");
    printf("lrotate(0x00040000, 4 ) = 0x%016lx\n", lrotate(0x40000,4));
    printf("lrotate(0x00040000, 46) = 0x%016lx\n", lrotate(0x40000,46));

    printf("This string should read `hello, world': `%s'\n", asmstr);
    {
        long a,b;
        a = (long)asmstr;
        b = (long)getstr();
        printf("The pointers %lx and %lx should be equal\n",a,b);
    }
   printf("This string should read `hello, world': `%s'\n", getstr());

    printf("The integers here should be 1234, 1235 and 4321:\n");
    integer = 1234;
    commvar = 4321;
    greet();
    printf("The absolute addressing to the asm-local integer should yield in 1235:\n%ld\n",readgreet());

    printf("These pointers should be equal: %p and %p\n",
           &greet, textptr);

    printf("So should these: %p and %p\n", selfptr, &selfptr);
}

/*
  there is no support for dynamically linkable objects in current
  mach-o module. Therefore put "printf" statement here and redirect 
  the asm call to druck()
*/
void druck( char *string, int a, int b, int c )
{
 printf(string,a,b,c);
}
