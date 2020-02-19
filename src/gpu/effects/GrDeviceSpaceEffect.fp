/*
* Copyright 2020 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

in fragmentProcessor fp;

void main() {
     sk_OutColor = sample(fp, sk_InColor, sk_FragCoord.xy);
}
