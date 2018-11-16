// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "gm.h"

#include "SkShaper.h"

static constexpr int W = 768;
static constexpr int H = 768;
DEF_SIMPLE_GM(shaper_test, canvas, W, H) {
    const char string1[] = "ข้อ 1 มนุษย์ทั้งหลายเกิดมามีอิสระและเสมอภาคกันในเกียรติศักด[เกี"
        "ยรติศักดิ์] هذا هو الخط และสิทธิ ต่างมีเหตุผลและมโนธรรม และควรปฏิบัติต่อกันด้วยเจตนา"
        "รมณ์แห่งภราดรภาพ ข้อ 2 ทุกคนย่อมมีสิทธิและอิสรภาพบรรดาที่กำหนดไว้ในปฏิญญานี้ โดยปราศ"
        "จากคi";
    const char string2[] = "วามแตกต่างไม่ว่าชนิดใด ๆ ดังเช่น เชื้อชาติ ผิว เพศ ภาษา ศาสน"
        "า ความคิดเห็นทางการเมืองหรือทางอื่น เผ่าพันธุ์แห่งชาติ หรือสังคม ทรัพย์สิน กำเนิด หรือสถ"
        "านะอื่น ๆ อนึ่งจะไม่มีความแตกต่างใด ๆ ตามมูลฐานแห่งสถานะทางการเมือง ทางการศาล ห"
        "รือทางการระหว่างประเทศของป";
    const char string3[] = "ระเทศหรือดินแดนที่บุคคลสังกัด ไม่ว่าดินแดนนี้จะเป็นเอกราช อยู่ในค"
        "วามพิทักษ์มิได้ปกครองตนเอง หรืออยู่ภายใต้การจำกัดอธิปไตยใด ๆ ทั้งสิ้น";

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(24);
    float margin = 16;
    SkPoint xy = {margin, margin + paint.getFontSpacing()};
    xy.fY = SkShaper::DrawString(canvas, string1, xy, paint, W - 2 * margin).y();
    xy.fY = SkShaper::DrawString(canvas, string2, xy, paint, W - 2 * margin).y();
    xy.fY = SkShaper::DrawString(canvas, string3, xy, paint, W - 2 * margin).y();
}
