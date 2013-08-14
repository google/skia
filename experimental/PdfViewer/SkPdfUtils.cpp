#include "SkPdfUtils.h"

#ifdef PDF_TRACE
void SkTraceMatrix(const SkMatrix& matrix, const char* sz) {
    printf("SkMatrix %s ", sz);
    for (int i = 0 ; i < 9 ; i++) {
        printf("%f ", SkScalarToDouble(matrix.get(i)));
    }
    printf("\n");
}

void SkTraceRect(const SkRect& rect, const char* sz) {
    printf("SkRect %s ", sz);
    printf("x = %f ", SkScalarToDouble(rect.x()));
    printf("y = %f ", SkScalarToDouble(rect.y()));
    printf("w = %f ", SkScalarToDouble(rect.width()));
    printf("h = %f ", SkScalarToDouble(rect.height()));
    printf("\n");
}
#endif
