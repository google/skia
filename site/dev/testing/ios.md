Testing on iOS
==============
Before setting Skia up for automated testing from the command line, please
follow the instructions to run Skia tests (*dm*, *nano-bench*) with the
mainstream iOS tool chain. See the [quick start guide for ios](../../user/quick/ios).

iOS doesn't lend itself well to compiling and running from the command line.
Below are instructions on how to install a set of tools that make this possible.
To see how they are used in automated testing please see the bash scripts
used by the buildbot recipes: <https://github.com/google/skia/tree/master/platform_tools/ios/bin>.

Installation
------------
The key tools are

* libimobiledevice <http://www.libimobiledevice.org/>, <https://github.com/libimobiledevice/libimobiledevice>

* ios-deploy <https://github.com/phonegap/ios-deploy>

Follow these steps to install them:

* Install Brew at <http://brew.sh/>
* Install *libimobiledevice*
  (Note: All these are part of the *libimobiledevice* project but packaged/developed
  under different names. The *cask* extension to *brew* is necessary to install
  *osxfuse* and *ifuse*, which allows to mount the application directory on an iOS device).

```
brew install libimobiledevice
brew install ideviceinstaller
brew install caskroom/cask/brew-cask
brew install Caskroom/cask/osxfuse
brew install ifuse
```

* Install node.js and ios-deploy

```
$ brew update
$ brew install node
$ npm install ios-deploy
```
