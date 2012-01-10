#include "DataTypes.h"

// Sources
// computer-aided design - volume 22 number 9 november 1990 pp 538 - 549
// online at http://cagd.cs.byu.edu/~tom/papers/bezclip.pdf

class LineParameters {
public:
    void cubicEndPoints(const Cubic& pts) {
        a = pts[0].y - pts[3].y;
        b = pts[3].x - pts[0].x;
        c = pts[0].x * pts[3].y - pts[3].x * pts[0].y;
    }
    
    void cubicEndPoints(const Cubic& pts, int s, int e) {
        a = pts[s].y - pts[e].y;
        b = pts[e].x - pts[s].x;
        c = pts[s].x * pts[e].y - pts[e].x * pts[s].y;
    }
    
    void lineEndPoints(const _Line& pts) {
        a = pts[0].y - pts[1].y;
        b = pts[1].x - pts[0].x;
        c = pts[0].x * pts[1].y - pts[1].x * pts[0].y;
    }
    
    void quadEndPoints(const Quadratic& pts) {
        a = pts[0].y - pts[2].y;
        b = pts[2].x - pts[0].x;
        c = pts[0].x * pts[2].y - pts[2].x * pts[0].y;
    }

    void quadEndPoints(const Quadratic& pts, int s, int e) {
        a = pts[s].y - pts[e].y;
        b = pts[e].x - pts[s].x;
        c = pts[s].x * pts[e].y - pts[e].x * pts[s].y;
    }

    double normalSquared() {
        return a * a + b * b;
    }

    bool normalize() {
        double normal = sqrt(normalSquared());
        if (normal < SquaredEpsilon) {
            a = b = c = 0;
            return false;
        }
        double reciprocal = 1 / normal;
        a *= reciprocal;
        b *= reciprocal;
        c *= reciprocal;
        return true;
    }
    
    void cubicDistanceY(const Cubic& pts, Cubic& distance) {
        double oneThird = 1 / 3.0;
        for (int index = 0; index < 4; ++index) {
            distance[index].x = index * oneThird;
            distance[index].y = a * pts[index].x + b * pts[index].y + c;
        }
    }

    void quadDistanceY(const Quadratic& pts, Quadratic& distance) {
        double oneHalf = 1 / 2.0;
        for (int index = 0; index < 3; ++index) {
            distance[index].x = index * oneHalf;
            distance[index].y = a * pts[index].x + b * pts[index].y + c;
        }
    }

    void controlPtDistance(const Cubic& pts, double distance[2]) {
        for (int index = 0; index < 2; ++index) {
            distance[index] = a * pts[index + 1].x + b * pts[index + 1].y + c;
        }
    }
    
    void controlPtDistance(const Cubic& pts, int i, int j, double distance[2]) {
        distance[0] = a * pts[i].x + b * pts[i].y + c;
        distance[1] = a * pts[j].x + b * pts[j].y + c;
    }
    
    double controlPtDistance(const Quadratic& pts) {
        return a * pts[1].x + b * pts[1].y + c;
    }
    
    double pointDistance(const _Point& pt) {
        return a * pt.x + b * pt.y + c;
    }
private:
    double a;
    double b;
    double c;
};
