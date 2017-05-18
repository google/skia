Assets
======

This directory contains tooling for managing assets used by the bots.  The
primary entry point is assets.py, which allows a user to add, remove, upload,
and download assets.

Assets are stored in Google Storage, named for their version number.


Individual Assets
-----------------

Each asset has its own subdirectory with the following contents:
* VERSION:  The current version number of the asset.
* download.py:  Convenience script for downloading the current version of the asset.
* upload.py:  Convenience script for uploading a new version of the asset.
* [optional] create.py:  Script which creates the asset, implemented by the user.
* [optional] create\_and\_upload.py:  Convenience script which combines create.py with upload.py.


Examples
-------

Add a new asset and upload an initial version.

```
$ infra/bots/assets/assets.py add myasset
Creating asset in infra/bots/assets/myasset
Creating infra/bots/assets/myasset/download.py
Creating infra/bots/assets/myasset/upload.py
Creating infra/bots/assets/myasset/common.py
Add script to automate creation of this asset? (y/n) n
$ infra/bots/assets/myasset/upload.py -t ${MY_ASSET_LOCATION}
$ git commit
```

Add an asset whose creation can be automated.

```
$ infra/bots/assets/assets.py add myasset
Add script to automate creation of this asset? (y/n) y
$ vi infra/bots/assets/myasset/create.py
(implement the create_asset function)
$ infra/bots/assets/myasset/create_and_upload.py
$ git commit
```
