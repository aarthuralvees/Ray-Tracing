#ifndef OBJETOHEADER
#define OBJETOHEADER

#include "Ray.h"
#include "Vetor.h"

struct HitRecord {
    double t;
    Ponto p;
    Vetor normal;
    color cor_difusa;
};

class Objeto {
public:
    virtual bool hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const = 0;
    virtual ~Objeto() = default;
};

#endif