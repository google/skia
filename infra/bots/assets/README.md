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
* [optional] create.py:  Script which creates the asset, implemented by the user and called by `sk asset upload`.
* [optional] create\_and\_upload.py:  User-implemented convenience script which wraps `sk asset upload` in whatever way makes sense for the asset.


Examples
-------

Add a new asset and upload an initial version.

```
$ sk asset add myasset
Do you want to add a creation script for this asset? (y/n): n
$ sk asset upload --in ${MY_ASSET_LOCATION} myasset
$ git commit
```

Add an asset whose creation can be automated.

```
$ sk asset add myasset
Do you want to add a creation script for this asset? (y/n): y
Created infra/bots/assets/myasset/create.py; you will need to add implementation before uploading the asset.
$ vi infra/bots/assets/myasset/create.py
(implement the create_asset function)
$ sk asset upload myasset
$ git commit
```
