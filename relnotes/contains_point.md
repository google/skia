Add SkRRect::contains(const SkPoint&). This allows point vs RRectF intersection
testing. As SkRRect::contains(const SkRectF&), returns true if the point is
inside the SkRRect and the SkRRect is not empty.