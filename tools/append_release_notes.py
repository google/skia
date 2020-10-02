# Copyright (c) 2020 Google LLC. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import re

RELEASE_NOTES_FILE="RELEASE_NOTES.txt"
RELEASE_NOTES_DRAFT_DIR="release_notes"
SEPARATOR = "\n* * *\n\n"

new_notes = ""
for filename in os.listdir(RELEASE_NOTES_DRAFT_DIR):
    if filename.endswith(".txt"):
        file = open(os.path.join(RELEASE_NOTES_DRAFT_DIR, filename))
        note = file.read()
        # Add bullet (*) before first line and add spaces before subsequent
        # lines.
        note = "  * " + note
        note = note.replace("\n", "\n    ")
        note = note.rstrip("n")
        new_notes = new_notes + "\n" + note;
        file.close()
    else:
        continue

notes_file = open(RELEASE_NOTES_FILE)
all_notes = notes_file.read()

search_re = SEPARATOR + "Milestone (\d+)"
next_milestone = int((re.split(search_re, all_notes))[1]) + 1
tag = "Milestone " + str(next_milestone)

underline = "\n" + len(tag)*"-" + "\n"
new_notes = SEPARATOR + tag + underline + new_notes + SEPARATOR
all_notes = all_notes.replace(SEPARATOR, new_notes, 1)
print all_notes
#overwrite file with all_notes
#delete individual notes files

