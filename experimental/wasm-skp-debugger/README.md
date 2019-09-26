WASM SKP Debugger
=================

The wasm skp debugger is a wasm binary that uses DebugCanvas to show SKP and MSKP files. It is
embedded in the javascript code in debugger-assets from the infra buildbot repository.

The live version is available at debugger.skia.org

Build
-----

```
make debug
make move-assets
```

note that make move-assets just copies the two output files over to the infra repo where they can
be served locally. This requires SKIA_INFRA_ROOT to be set to the root of your checkout of that
repo.

For more information on running the debugger locally, see infra/debugger-assets/README.md in the
buildbot repo.

Test
----

tests are run with

```
make test-continuous
```

tests are defined by files in `tests/*.spec.js`