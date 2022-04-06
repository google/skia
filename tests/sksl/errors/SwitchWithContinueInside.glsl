### Compilation failed:

error: 4: continue statement cannot be used in a switch
    while (true) switch (value) { default: continue; }
                                           ^^^^^^^^
error: 5: continue statement cannot be used in a switch
    for (;;) switch (value) { case 5: continue; }
                                      ^^^^^^^^
2 errors
