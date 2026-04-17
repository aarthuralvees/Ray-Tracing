#ifndef RAYHEADER
#define RAYHEADER

#include "Ponto.h"
#include "Vetor.h"

class ray {
private:
    Ponto orig;
    Vetor dir;

public:
    ray() {}
    ray(const Ponto& origin, const Vetor& direction) : orig(origin), dir(direction) {}

    const Ponto& origin() const { return orig; }
    const Vetor& direction() const { return dir; }

    Ponto at(double t) const {
        return orig + (dir * t);
    }
};

#endif
