'''
Copyright 2011 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
'''

import datetime
import re

def CreateParser(filepath):
    """Returns a Parser as appropriate for the file at this filepath.
    """
    if (filepath.endswith('.cpp') or
        filepath.endswith('.h') or
        filepath.endswith('.c')):
        return CParser()
    else:
        return None


class Parser(object):
    """Base class for all language-specific parsers.
    """

    def __init__(self):
        self._copyright_pattern = re.compile('copyright', re.IGNORECASE)
        self._attribute_pattern = re.compile(
            'copyright.*\D(\d{4})\W*(\w.*[\w.])', re.IGNORECASE)

    def FindCopyrightBlock(self, comment_blocks):
        """Given a list of comment block strings, return the one that seems
        like the most likely copyright block.

        Returns None if comment_blocks was empty, or if we couldn't find
        a comment block that contains copyright info."""
        if not comment_blocks:
            return None
        for block in comment_blocks:
            if self._copyright_pattern.search(block):
                return block

    def GetCopyrightBlockAttributes(self, comment_block):
        """Given a comment block, return a tuple of attributes: (year, holder).

        If comment_block is None, or none of the attributes are found,
        this will return (None, None)."""
        if not comment_block:
            return (None, None)
        matches = self._attribute_pattern.findall(comment_block)
        if not matches:
            return (None, None)
        first_match = matches[0]
        return (first_match[0], first_match[1])


class CParser(Parser):
    """Parser that knows how to parse C/C++ files.
    """

    DEFAULT_YEAR = datetime.date.today().year
    DEFAULT_HOLDER = 'Google Inc.'
    COPYRIGHT_BLOCK_FORMAT = '''
/*
 * Copyright %s %s
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
'''

    def __init__(self):
        super(CParser, self).__init__()
        self._comment_pattern = re.compile('/\*.*?\*/', re.DOTALL)

    def FindAllCommentBlocks(self, file_contents):
        """Returns a list of all comment blocks within these file contents.
        """
        return self._comment_pattern.findall(file_contents)

    def CreateCopyrightBlock(self, year, holder):
        """Returns a copyright block suitable for this language, with the
        given attributes.

        @param year year in which to hold copyright (defaults to DEFAULT_YEAR)
        @param holder holder of copyright (defaults to DEFAULT_HOLDER)
        """
        if not year:
            year = self.DEFAULT_YEAR
        if not holder:
            holder = self.DEFAULT_HOLDER
        return self.COPYRIGHT_BLOCK_FORMAT % (year, holder)
