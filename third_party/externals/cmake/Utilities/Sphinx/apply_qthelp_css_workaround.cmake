
file(READ "${CSS_DIR}/basic.css" BasicCssContent)

file(READ "${CSS_DIR}/default.css" DefaultCssContent)
string(REPLACE
  "@import url(\"basic.css\")" "${BasicCssContent}"
  DefaultCssContent "${DefaultCssContent}"
)

file(READ "${CSS_DIR}/cmake.css" CMakeCssContent)
string(REPLACE
  "@import url(\"default.css\")" "${DefaultCssContent}"
  CMakeCssContent "${CMakeCssContent}"
)
file(WRITE "${CSS_DIR}/cmake.css" "${CMakeCssContent}")
