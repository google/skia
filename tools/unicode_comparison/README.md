Unicode comparison utilities
============================

This directory contains the following

cpp/*
-----
This folder contains the bridge code between SkUnicode and golang

go/*
----
This folder contains the set on golang utilities that allow to download, preprocess data and then generate the comparison table itself

download_wiki.go
----------------
make download_wiki
./download_wiki
You may need to update go-wiki package: go get -u github.com/trietmn/go-wiki
List of wiki languages: https://meta.wikimedia.org/wiki/List_of_Wikipedias

extract_info.go
---------------

generate_table.go
-----------------

