STRINGIFY(

sk_has_side_effects void print(float f);

$genType cos($genType y);
$genHType cos($genHType y);
float2x2 inverse(float2x2 m);
float3x3 inverse(float3x3 m);
float4x4 inverse(float4x4 m);
$genType sin($genType x);
$genHType sin($genHType x);
$genType sqrt($genType x);
$genHType sqrt($genHType x);
$genType tan($genType x);
$genHType tan($genHType x);

$genType  mix($genType  x, $genType  y, $genBType a);
$genHType mix($genHType x, $genHType y, $genBType a);
$genIType mix($genIType x, $genIType y, $genBType a);
$genBType mix($genBType x, $genBType y, $genBType a);

$bvec lessThan($vec  x, $vec  y);
$bvec lessThan($hvec x, $hvec y);
$bvec lessThan($ivec x, $ivec y);
$bvec lessThan($uvec x, $uvec y);
$bvec lessThanEqual($vec  x, $vec  y);
$bvec lessThanEqual($hvec x, $hvec y);
$bvec lessThanEqual($ivec x, $ivec y);
$bvec lessThanEqual($uvec x, $uvec y);
$bvec greaterThan($vec  x, $vec  y);
$bvec greaterThan($hvec x, $hvec y);
$bvec greaterThan($ivec x, $ivec y);
$bvec greaterThan($uvec x, $uvec y);
$bvec greaterThanEqual($vec  x, $vec  y);
$bvec greaterThanEqual($hvec x, $hvec y);
$bvec greaterThanEqual($ivec x, $ivec y);
$bvec greaterThanEqual($uvec x, $uvec y);
$bvec equal($vec  x, $vec  y);
$bvec equal($hvec x, $hvec y);
$bvec equal($ivec x, $ivec y);
$bvec equal($uvec x, $uvec y);
$bvec equal($bvec x, $bvec y);
$bvec notEqual($vec  x, $vec  y);
$bvec notEqual($hvec x, $hvec y);
$bvec notEqual($ivec x, $ivec y);
$bvec notEqual($uvec x, $uvec y);
$bvec notEqual($bvec x, $bvec y);

bool  any($bvec x);
bool  all($bvec x);
$bvec not($bvec x);

)
