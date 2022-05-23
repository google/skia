# PathKit Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
 - `res_scale` expoed in StrokeOpts object. (skbug.com/13301)

## [1.0.0] 2022-02-03

### Changed
 - Now compile with emsdk 3.1.3
 - Internal calls no longer use dynamic dispatch (skbug.com/12795)
 - WASM Memory can grow beyond initial 32MB
 - The debug and test builds of PathKit are no longer published to NPM.

## [0.7.0] 2019-07-20

### Changed
 - Updated to emscripten 1.39.16
 - Support wombat-dressing-room. go/npm-publish

### Breaking
 - `PathKitInit(...)` now directly returns a Promise. As such, `PathKitInit(...).ready()`
   has been removed.

## [0.6.0] 2019-02-25

### Fixed
 - Potential bug in `ready()` if already loaded.

### Removed
 - Deprecated `PathKitInit.then()` see 0.5.1 notes.

## [0.5.1] 2019-01-04

### Changed
 - `PathKitInit(...).then()` is no longer the recommended way to initialize things.
It will be removed in 0.6.0. Use `PathKitInit(...).ready()`, which returns a real Promise.

## [0.5.0] 2018-12-17

Updated PathKit to use same FOSS license as Skia proper.

## [0.4.2] 2018-11-07

Beginning of changelog.
