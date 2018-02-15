#include <stdio.h>

#if defined(ENABLE_GIF)
    void print_gif_info();
#else
    static void print_gif_info() { printf("skia cannot handle GIF\n"); }
#endif

void print_skia_info() {
    printf("This is core Skia.\n");
    print_gif_info();
}
