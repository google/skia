half4 main() {
    int x = 12, y = 34;

    // Basic if statement.
    if (x == y + 1) return half4(2);

    // Basic if statement with Block.
    if (x == y) {
        return half4(1);
    }

    // If statement with comma-expression, no Block.
    if (x == y + 2) x = 0, y = 0;

    // Basic if-else statement.
    if (x == y + 3) x *= 2; else y *= 2;

    // Chained if-else statements.
    if (x == y + 4) {
        x += 1;
        y -= 1;
        return half4(2);
    } else if (x == y + 5) {
        y -= 1;
        return half4(3);
    } else {
        y += 1;
    }

    // Nested if-else statements.
    if (x == y + 6) {
        if (x == 99) {
            x *= 2;
        } else {
            y *= 2;
        }
    } else {
        if (x == 99) {
            x /= 2;
        } else {
            y /= 2;
        }
    }

    return half4(0);
}
