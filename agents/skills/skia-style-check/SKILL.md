---
name: skia-style-check
description: >-
  Checks that changed code on the current git branch matches Skia's style and
  formatting requirements defined in site/docs/dev/contrib/style.md. Use this
  skill when verifying, auditing, or reviewing code changes on the active branch
  for style compliance before committing or submitting. Do not use for compiler
  errors or build workflows.
---

# Skia Style Check Skill

This skill guides the agent in verifying that code modifications on the current git branch
adhere to Skia's coding standards.

## Scope: Current Branch Changes Only

- **Target Scope**: Evaluate **only** the lines and files that have been changed on the
  current branch compared to its tracked upstream branch (`@{u}`, e.g., `origin/main` or
  parent feature branch).
- **Ignore Unchanged Code**: Do **not** review or flag pre-existing style inconsistencies in
  unchanged code or outside the diff.
- **Obtaining Changes**:
  - Determine the tracked upstream branch (falls back to `origin/main` if not set):
    ```bash
    UPSTREAM=$(git rev-parse --abbrev-ref --symbolic-full-name @{u} 2>/dev/null || \
               echo "origin/main")
    ```
  - To view committed changes on the branch against the tracked upstream:
    ```bash
    git diff $(git merge-base "$UPSTREAM" HEAD)..HEAD
    ```
  - To include any uncommitted / staged working tree changes as well:
    ```bash
    git diff $(git merge-base "$UPSTREAM" HEAD)
    ```

## Style Rules Reference

Always inspect `site/docs/dev/contrib/style.md` to determine the requirements applicable to
the modified files.

## Review Workflow

1. **Extract Changed Files and Lines**:
   - Determine the tracked upstream branch and its merge base with `HEAD`.
   - Inspect the diff to identify modified files and line numbers.
2. **Consult Style Guidelines**:
   - Read the relevant sections of `site/docs/dev/contrib/style.md` for the types of changes
     and languages in the diff.
3. **Verify Compliance**:
   - Check all newly added or modified lines against the rules in
     `site/docs/dev/contrib/style.md`.
4. **Report Findings**:
   - List any violations found in the modified code, including file path, line numbers,
     description of the issue, and suggested corrections.
   - If no violations are found, report that the changes comply with the current style
     requirements.
