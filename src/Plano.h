#ifndef PLANOHEADER
#define PLANOHEADER

#include "Objeto.h"

class Plano : public Objeto {
public:
    Ponto p0;
    Vetor normal;
    color cor;

    Plano(Ponto p0, Vetor n, color cor) : p0(p0), normal(unit_vector(n)), cor(cor) {}

    bool hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const override {
        double denom = dot(normal, r.direction());
        if (std::abs(denom) < 1e-8) return false; 

        double t = dot(p0 - r.origin(), normal) / denom;
        if (t <= t_min || t >= t_max) return false;

        rec.t = t;
        rec.p = r.at(t);
        rec.normal = normal;
        
        if (dot(r.direction(), rec.normal) > 0) {
            rec.normal = -rec.normal;
        }
        
        rec.cor_difusa = cor;
        return true;
    }
};

#endif