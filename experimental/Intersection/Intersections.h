class Intersections {
public:
    Intersections()
        : fUsed(0)
        , fSwap(0)
    {
        bzero(fT, sizeof(fT));
    }

    void add(double one, double two) {
        if (fUsed > 0 && approximately_equal(fT[fSwap][fUsed - 1], one)
                && approximately_equal(fT[fSwap ^ 1][fUsed - 1], two)) {
            return;
        }
        fT[fSwap][fUsed] = one;
        fT[fSwap ^ 1][fUsed] = two;
        ++fUsed;
    }

    void offset(int base, double start, double end) {
        for (int index = base; index < fUsed; ++index) {
            double val = fT[fSwap][index];
            val *= end - start;
            val += start;
            fT[fSwap][index] = val;
        }
    }

    bool intersected() {
        return fUsed > 0;
    }

    void swap() {
        fSwap ^= 1;
    }
    
    bool swapped() {
        return fSwap;
    }

    int used() {
        return fUsed;
    }

    double fT[2][9];
private:
    int fUsed;
    int fSwap;
};
