---
title: 'Testing on iOS'
linkTitle: 'Testing on iOS'
---

Before setting Skia up for automated testing from the command line, please
follow the instructions to run Skia tests (_dm_, _nano-bench_) with the
mainstream iOS tool chain. See the
[quick start guide for ios](/docs/user/build/).

iOS doesn't lend itself well to compiling and running from the command line.
Below are instructions on how to install a set of tools that make this possible.
To see how they are used in automated testing please see the bash scripts used
by the buildbot recipes:
<https://github.com/google/skia/tree/main/platform_tools/ios/bin>.

## Installation

The key tools are

- libimobiledevice <http://www.libimobiledevice.org/>,
  <https://github.com/libimobiledevice/libimobiledevice>

- ios-deploy <https://github.com/phonegap/ios-deploy>

Follow these steps to install them:

- Install Brew at <http://brew.sh/>
- Install _libimobiledevice_ (Note: All these are part of the _libimobiledevice_
  project but packaged/developed under different names. The _cask_ extension to
  _brew_ is necessary to install _osxfuse_ and _ifuse_, which allows to mount
  the application directory on an iOS device).

```
brew install libimobiledevice
brew install ideviceinstaller
brew install caskroom/cask/brew-cask
brew install Caskroom/cask/osxfuse
brew install ifuse
```

- Install node.js and ios-deploy

```
$ brew update
$ brew install node
$ npm install ios-deploy
```
