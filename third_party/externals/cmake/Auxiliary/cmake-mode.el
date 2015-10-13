;;; cmake-mode.el --- major-mode for editing CMake sources

;=============================================================================
; CMake - Cross Platform Makefile Generator
; Copyright 2000-2009 Kitware, Inc., Insight Software Consortium
;
; Distributed under the OSI-approved BSD License (the "License");
; see accompanying file Copyright.txt for details.
;
; This software is distributed WITHOUT ANY WARRANTY; without even the
; implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
; See the License for more information.
;=============================================================================

;------------------------------------------------------------------------------

;;; Commentary:

;; Provides syntax highlighting and indentation for CMakeLists.txt and
;; *.cmake source files.
;;
;; Add this code to your .emacs file to use the mode:
;;
;;  (setq load-path (cons (expand-file-name "/dir/with/cmake-mode") load-path))
;;  (require 'cmake-mode)

;------------------------------------------------------------------------------

;;; Code:
;;
;; cmake executable variable used to run cmake --help-command
;; on commands in cmake-mode
;;
;; cmake-command-help Written by James Bigler
;;

(defcustom cmake-mode-cmake-executable "cmake"
  "*The name of the cmake executable.

This can be either absolute or looked up in $PATH.  You can also
set the path with these commands:
 (setenv \"PATH\" (concat (getenv \"PATH\") \";C:\\\\Program Files\\\\CMake 2.8\\\\bin\"))
 (setenv \"PATH\" (concat (getenv \"PATH\") \":/usr/local/cmake/bin\"))"
  :type 'file
  :group 'cmake)
;;
;; Regular expressions used by line indentation function.
;;
(defconst cmake-regex-blank "^[ \t]*$")
(defconst cmake-regex-comment "#.*")
(defconst cmake-regex-paren-left "(")
(defconst cmake-regex-paren-right ")")
(defconst cmake-regex-argument-quoted
  "\"\\([^\"\\\\]\\|\\\\\\(.\\|\n\\)\\)*\"")
(defconst cmake-regex-argument-unquoted
  "\\([^ \t\r\n()#\"\\\\]\\|\\\\.\\)\\([^ \t\r\n()#\\\\]\\|\\\\.\\)*")
(defconst cmake-regex-token (concat "\\(" cmake-regex-comment
                                    "\\|" cmake-regex-paren-left
                                    "\\|" cmake-regex-paren-right
                                    "\\|" cmake-regex-argument-unquoted
                                    "\\|" cmake-regex-argument-quoted
                                    "\\)"))
(defconst cmake-regex-indented (concat "^\\("
                                       cmake-regex-token
                                       "\\|" "[ \t\r\n]"
                                       "\\)*"))
(defconst cmake-regex-block-open
  "^\\(if\\|macro\\|foreach\\|else\\|elseif\\|while\\|function\\)$")
(defconst cmake-regex-block-close
  "^[ \t]*\\(endif\\|endforeach\\|endmacro\\|else\\|elseif\\|endwhile\\|endfunction\\)[ \t]*(")

;------------------------------------------------------------------------------

;;
;; Helper functions for line indentation function.
;;
(defun cmake-line-starts-inside-string ()
  "Determine whether the beginning of the current line is in a string."
  (if (save-excursion
        (beginning-of-line)
        (let ((parse-end (point)))
          (goto-char (point-min))
          (nth 3 (parse-partial-sexp (point) parse-end))
          )
        )
      t
    nil
    )
  )

(defun cmake-find-last-indented-line ()
  "Move to the beginning of the last line that has meaningful indentation."
  (let ((point-start (point))
        region)
    (forward-line -1)
    (setq region (buffer-substring-no-properties (point) point-start))
    (while (and (not (bobp))
                (or (looking-at cmake-regex-blank)
                    (cmake-line-starts-inside-string)
                    (not (and (string-match cmake-regex-indented region)
                              (= (length region) (match-end 0))))))
      (forward-line -1)
      (setq region (buffer-substring-no-properties (point) point-start))
      )
    )
  )

;------------------------------------------------------------------------------

;;
;; Line indentation function.
;;
(defun cmake-indent ()
  "Indent current line as CMAKE code."
  (interactive)
  (if (cmake-line-starts-inside-string)
      ()
    (if (bobp)
        (cmake-indent-line-to 0)
      (let (cur-indent)

        (save-excursion
          (beginning-of-line)

          (let ((point-start (point))
                (case-fold-search t)  ;; case-insensitive
                token)

            ; Search back for the last indented line.
            (cmake-find-last-indented-line)

            ; Start with the indentation on this line.
            (setq cur-indent (current-indentation))

            ; Search forward counting tokens that adjust indentation.
            (while (re-search-forward cmake-regex-token point-start t)
              (setq token (match-string 0))
              (if (string-match (concat "^" cmake-regex-paren-left "$") token)
                  (setq cur-indent (+ cur-indent cmake-tab-width))
                )
              (if (string-match (concat "^" cmake-regex-paren-right "$") token)
                  (setq cur-indent (- cur-indent cmake-tab-width))
                )
              (if (and
                   (string-match cmake-regex-block-open token)
                   (looking-at (concat "[ \t]*" cmake-regex-paren-left))
                   )
                  (setq cur-indent (+ cur-indent cmake-tab-width))
                )
              )
            (goto-char point-start)

            ; If this is the end of a block, decrease indentation.
            (if (looking-at cmake-regex-block-close)
                (setq cur-indent (- cur-indent cmake-tab-width))
              )
            )
          )

        ; Indent this line by the amount selected.
        (if (< cur-indent 0)
            (cmake-indent-line-to 0)
          (cmake-indent-line-to cur-indent)
          )
        )
      )
    )
  )

(defun cmake-point-in-indendation ()
  (string-match "^[ \\t]*$" (buffer-substring (point-at-bol) (point))))

(defun cmake-indent-line-to (column)
  "Indent the current line to COLUMN.
If point is within the existing indentation it is moved to the end of
the indentation.  Otherwise it retains the same position on the line"
  (if (cmake-point-in-indendation)
      (indent-line-to column)
    (save-excursion (indent-line-to column))))

;------------------------------------------------------------------------------

;;
;; Helper functions for buffer
;;
(defun unscreamify-cmake-buffer ()
  "Convert all CMake commands to lowercase in buffer."
  (interactive)
  (goto-char (point-min))
  (while (re-search-forward "^\\([ \t]*\\)\\(\\w+\\)\\([ \t]*(\\)" nil t)
    (replace-match
     (concat
      (match-string 1)
      (downcase (match-string 2))
      (match-string 3))
     t))
  )

;------------------------------------------------------------------------------

;;
;; Keyword highlighting regex-to-face map.
;;
(defconst cmake-font-lock-keywords
  (list '("^[ \t]*\\([[:word:]_]+\\)[ \t]*(" 1 font-lock-function-name-face))
  "Highlighting expressions for CMAKE mode."
  )

;------------------------------------------------------------------------------

;;
;; Syntax table for this mode.  Initialize to nil so that it is
;; regenerated when the cmake-mode function is called.
;;
(defvar cmake-mode-syntax-table nil "Syntax table for cmake-mode.")
(setq cmake-mode-syntax-table nil)

;;
;; User hook entry point.
;;
(defvar cmake-mode-hook nil)

;;
;; Indentation increment.
;;
(defvar cmake-tab-width 2)

;------------------------------------------------------------------------------

;;
;; CMake mode startup function.
;;
;;;###autoload
(defun cmake-mode ()
  "Major mode for editing CMake listfiles."
  (interactive)
  (kill-all-local-variables)
  (setq major-mode 'cmake-mode)
  (setq mode-name "CMAKE")

  ; Create the syntax table
  (setq cmake-mode-syntax-table (make-syntax-table))
  (set-syntax-table cmake-mode-syntax-table)
  (modify-syntax-entry ?\(  "()" cmake-mode-syntax-table)
  (modify-syntax-entry ?\)  ")(" cmake-mode-syntax-table)
  (modify-syntax-entry ?# "<" cmake-mode-syntax-table)
  (modify-syntax-entry ?\n ">" cmake-mode-syntax-table)

  ; Setup font-lock mode.
  (make-local-variable 'font-lock-defaults)
  (setq font-lock-defaults '(cmake-font-lock-keywords))

  ; Setup indentation function.
  (make-local-variable 'indent-line-function)
  (setq indent-line-function 'cmake-indent)

  ; Setup comment syntax.
  (make-local-variable 'comment-start)
  (setq comment-start "#")

  ; Run user hooks.
  (run-hooks 'cmake-mode-hook))

; Help mode starts here


;;;###autoload
(defun cmake-command-run (type &optional topic buffer)
  "Runs the command cmake with the arguments specified.  The
optional argument topic will be appended to the argument list."
  (interactive "s")
  (let* ((bufname (if buffer buffer (concat "*CMake" type (if topic "-") topic "*")))
         (buffer  (if (get-buffer bufname) (get-buffer bufname) (generate-new-buffer bufname)))
         (command (concat cmake-mode-cmake-executable " " type " " topic))
         ;; Turn of resizing of mini-windows for shell-command.
         (resize-mini-windows nil)
         )
    (shell-command command buffer)
    (save-selected-window
      (select-window (display-buffer buffer 'not-this-window))
      (cmake-mode)
      (toggle-read-only t))
    )
  )

;;;###autoload
(defun cmake-help-list-commands ()
  "Prints out a list of the cmake commands."
  (interactive)
  (cmake-command-run "--help-command-list")
  )

(defvar cmake-commands '() "List of available topics for --help-command.")
(defvar cmake-help-command-history nil "Command read history.")
(defvar cmake-modules '() "List of available topics for --help-module.")
(defvar cmake-help-module-history nil "Module read history.")
(defvar cmake-variables '() "List of available topics for --help-variable.")
(defvar cmake-help-variable-history nil "Variable read history.")
(defvar cmake-properties '() "List of available topics for --help-property.")
(defvar cmake-help-property-history nil "Property read history.")
(defvar cmake-help-complete-history nil "Complete help read history.")
(defvar cmake-string-to-list-symbol
  '(("command" cmake-commands cmake-help-command-history)
    ("module" cmake-modules cmake-help-module-history)
    ("variable"  cmake-variables cmake-help-variable-history)
    ("property" cmake-properties cmake-help-property-history)
    ))

(defun cmake-get-list (listname)
  "If the value of LISTVAR is nil, run cmake --help-LISTNAME-list
and store the result as a list in LISTVAR."
  (let ((listvar (car (cdr (assoc listname cmake-string-to-list-symbol)))))
    (if (not (symbol-value listvar))
        (let ((temp-buffer-name "*CMake Temporary*"))
          (save-window-excursion
            (cmake-command-run (concat "--help-" listname "-list") nil temp-buffer-name)
            (with-current-buffer temp-buffer-name
              (set listvar (cdr (split-string (buffer-substring-no-properties (point-min) (point-max)) "\n" t))))))
      (symbol-value listvar)
      ))
  )

(require 'thingatpt)
(defun cmake-symbol-at-point ()
  (let ((symbol (symbol-at-point)))
    (and (not (null symbol))
         (symbol-name symbol))))

(defun cmake-help-type (type)
  (let* ((default-entry (cmake-symbol-at-point))
         (history (car (cdr (cdr (assoc type cmake-string-to-list-symbol)))))
         (input (completing-read
                 (format "CMake %s: " type) ; prompt
                 (cmake-get-list type) ; completions
                 nil ; predicate
                 t   ; require-match
                 default-entry ; initial-input
                 history
                 )))
    (if (string= input "")
        (error "No argument given")
      input))
  )

;;;###autoload
(defun cmake-help-command ()
  "Prints out the help message for the command the cursor is on."
  (interactive)
  (cmake-command-run "--help-command" (cmake-help-type "command") "*CMake Help*"))

;;;###autoload
(defun cmake-help-module ()
  "Prints out the help message for the module the cursor is on."
  (interactive)
  (cmake-command-run "--help-module" (cmake-help-type "module") "*CMake Help*"))

;;;###autoload
(defun cmake-help-variable ()
  "Prints out the help message for the variable the cursor is on."
  (interactive)
  (cmake-command-run "--help-variable" (cmake-help-type "variable") "*CMake Help*"))

;;;###autoload
(defun cmake-help-property ()
  "Prints out the help message for the property the cursor is on."
  (interactive)
  (cmake-command-run "--help-property" (cmake-help-type "property") "*CMake Help*"))

;;;###autoload
(defun cmake-help ()
  "Queries for any of the four available help topics and prints out the approriate page."
  (interactive)
  (let* ((default-entry (cmake-symbol-at-point))
         (command-list (cmake-get-list "command"))
         (variable-list (cmake-get-list "variable"))
         (module-list (cmake-get-list "module"))
         (property-list (cmake-get-list "property"))
         (all-words (append command-list variable-list module-list property-list))
         (input (completing-read
                 "CMake command/module/variable/property: " ; prompt
                 all-words ; completions
                 nil ; predicate
                 t   ; require-match
                 default-entry ; initial-input
                 'cmake-help-complete-history
                 )))
    (if (string= input "")
        (error "No argument given")
      (if (member input command-list)
          (cmake-command-run "--help-command" input "*CMake Help*")
        (if (member input variable-list)
            (cmake-command-run "--help-variable" input "*CMake Help*")
          (if (member input module-list)
              (cmake-command-run "--help-module" input "*CMake Help*")
            (if (member input property-list)
                (cmake-command-run "--help-property" input "*CMake Help*")
              (error "Not a know help topic.") ; this really should not happen
              ))))))
  )

;;;###autoload
(progn
  (add-to-list 'auto-mode-alist '("CMakeLists\\.txt\\'" . cmake-mode))
  (add-to-list 'auto-mode-alist '("\\.cmake\\'" . cmake-mode)))

; This file provides cmake-mode.
(provide 'cmake-mode)

;;; cmake-mode.el ends here
