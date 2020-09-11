#include <png.h>

int main(int argc, char** argv) {
    return png_access_version_number() == 10637
        ? 0 : 1;
}
