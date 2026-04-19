#ifndef ESFERAHEADER
#define ESFERAHEADER

#include "Objeto.h"

class Esfera : public Objeto {
public:
    Ponto centro;
    double raio;
    color cor;

    Esfera(Ponto c, double r, color cor) : centro(c), raio(r), cor(cor) {}

    bool hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const override {
        Vetor oc = r.origin() - centro;
        double a = dot(r.direction(), r.direction());
        double b = 2.0 * dot(oc, r.direction());
        double c = dot(oc, oc) - raio * raio;
        
        double discriminante = b * b - 4 * a * c;
        if (discriminante < 0) return false;

        double sqrtd = std::sqrt(discriminante);
        double raiz = (-b - sqrtd) / (2.0 * a);
        if (raiz <= t_min || raiz >= t_max) {
            raiz = (-b + sqrtd) / (2.0 * a);
            if (raiz <= t_min || raiz >= t_max) return false;
        }

        rec.t = raiz;
        rec.p = r.at(rec.t);
        rec.normal = (rec.p - centro) / raio;
        rec.cor_difusa = cor;
        
        return true;
    }
};

#endif