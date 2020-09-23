uniform half scalarArray[4];
uniform half2 pointArray[2];

void main() {
   sk_OutColor = half4(scalarArray[0] * pointArray[0].x +
                       scalarArray[1] * pointArray[0].y +
                       scalarArray[2] * pointArray[1].x +
                       scalarArray[3] * pointArray[1].y);
}
