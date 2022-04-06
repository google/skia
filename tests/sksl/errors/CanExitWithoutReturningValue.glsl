### Compilation failed:

error: 6: function 'if_only' can exit without returning a value
int if_only()                       { if (variable == 1) return 0; }
                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 7: function 'return_on_if_but_not_else' can exit without returning a value
int return_on_if_but_not_else()     { if (variable == 1) return 0; else variable *= 2; }
                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 8: function 'return_on_else_but_not_if' can exit without returning a value
int return_on_else_but_not_if()     { if (variable == 1) variable *= 2; else return 1; }
                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 10: function 'for_with_conditional_return' can exit without returning a value
int for_with_conditional_return()   { for (;;) { if (variable == 1) return 0; } }
                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 11: function 'for_with_conditional_break' can exit without returning a value
int for_with_conditional_break()    { for (;;) { if (variable == 1) break; return 0; } }
                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 12: function 'for_with_conditional_continue' can exit without returning a value
int for_with_conditional_continue() { for (;;) { if (variable == 1) continue; return 0; } }
                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 14: function 'do_with_conditional_return' can exit without returning a value
bool do_with_conditional_return()   { do { if (mystery == 1) return true; } while(true); }
                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 15: function 'do_with_conditional_break' can exit without returning a value
bool do_with_conditional_break()    { do { if (mystery == 1) break; return true; } while(true); }
                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 16: function 'do_with_conditional_continue' can exit without returning a value
bool do_with_conditional_continue() { do { if (mystery == 1) continue; return true; } while(true); }
                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 18: function 'bad_if_else_chain' can exit without returning a value
bool bad_if_else_chain() {
                         ^...
error: 32: function 'switch_empty' can exit without returning a value
int switch_empty() {
                   ^...
error: 36: function 'switch_with_no_default' can exit without returning a value
int switch_with_no_default() {
                             ^...
error: 43: function 'switch_with_break' can exit without returning a value
int switch_with_break() {
                        ^...
error: 55: continue statement cannot be used in a switch
        default: continue;
                 ^^^^^^^^
error: 51: function 'switch_with_continue' can exit without returning a value
int switch_with_continue() {
                           ^...
error: 59: function 'switch_with_fallthrough_off_bottom' can exit without returning a value
int switch_with_fallthrough_off_bottom() {
                                         ^...
error: 67: function 'switch_with_conditional_break' can exit without returning a value
int switch_with_conditional_break() {
                                    ^...
error: 77: continue statement cannot be used in a switch
        case 1:  if (mystery == 123) ; else continue; return 1;
                                            ^^^^^^^^
error: 75: function 'switch_with_conditional_continue' can exit without returning a value
int switch_with_conditional_continue() {
                                       ^...
error: 83: function 'switch_with_conditional_if_then_return' can exit without returning a value
int switch_with_conditional_if_then_return() {
                                             ^...
error: 90: function 'switch_with_conditional_break_then_fallthrough' can exit without returning a value
int switch_with_conditional_break_then_fallthrough() {
                                                     ^...
21 errors
