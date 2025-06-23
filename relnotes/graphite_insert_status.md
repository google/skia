`Context::insertRecording` now returns an object that behaves like an enum or a true/false bool
to assist migrating from the old bool return type to something that provides more details as
to why the Recording couldn't be played back.

This shouldn't break any existing usage of `insertRecording` but migrating to check against
`InsertStatus::kSuccess` is recommended to avoid future breaking changes.