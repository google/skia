
SkPath is migrating to become immutable (its geometry).

In this new version, SkPath will lose all of its methds like moveTo(), lineTo(), etc. and rely on SkPathBuilder for creating paths. Additionally, there are now additional Factories for creating paths in one-call, so often a pathbuilder object may not be needed.

    static SkPath Raw(...);
    static SkPath Rect(...);
    static SkPath Oval(...);
    static SkPath Circle(...);
    static SkPath RRect(...);
    static SkPath Polygon(...);
    static SkPath Line(...);

Clients that create or edit paths need to switch over to using these factories and/or SkPathBuilder.

The flag that triggers this is SK_HIDE_PATH_EDIT_METHODS. This means that for now Skia can be built in either way -- but in a subsequent release, this flag will be removed, and SkPath will permanently be in its immutable form.
