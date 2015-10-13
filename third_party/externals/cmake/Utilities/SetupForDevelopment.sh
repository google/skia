#!/usr/bin/env bash

cd "${BASH_SOURCE%/*}/.." &&
Utilities/GitSetup/setup-user && echo &&
Utilities/GitSetup/setup-hooks && echo &&
Utilities/GitSetup/setup-stage && echo &&
(Utilities/GitSetup/setup-ssh ||
 echo 'Failed to setup SSH.  Run this again to retry.') && echo &&
Utilities/GitSetup/tips

# Rebase master by default
git config rebase.stat true
git config branch.master.rebase true
