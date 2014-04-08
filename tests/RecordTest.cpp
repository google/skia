#include "Test.h"

#include "SkRecord.h"
#include "SkRecords.h"

// Sums the area of any DrawRect command it sees.
class AreaSummer {
public:
    AreaSummer() : fArea(0) {}

    template <typename T> void operator()(const T&) { }

    int area() const { return fArea; }

private:
    int fArea;
};
template <> void AreaSummer::operator()(const SkRecords::DrawRect& record) {
    fArea += (int) (record.rect.width() * record.rect.height());
}

// Scales out the bottom-right corner of any DrawRect command it sees by 2x.
struct Stretch {
    template <typename T> void operator()(T*) {}
};
template <> void Stretch::operator()(SkRecords::DrawRect* record) {
    record->rect.fRight *= 2;
    record->rect.fBottom *= 2;
}

// Basic tests for the low-level SkRecord code.
DEF_TEST(Record, r) {
    SkRecord record;

    // Add a simple DrawRect command.
    SkRect rect = SkRect::MakeWH(10, 10);
    SkPaint paint;
    SkNEW_PLACEMENT_ARGS(record.append<SkRecords::DrawRect>(), SkRecords::DrawRect, (rect, paint));

    // Its area should be 100.
    AreaSummer summer;
    record.visit(summer);
    REPORTER_ASSERT(r, summer.area() == 100);

    // Scale 2x.
    Stretch stretch;
    record.mutate(stretch);

    // Now its area should be 100 + 400.
    record.visit(summer);
    REPORTER_ASSERT(r, summer.area() == 500);
}
