#ifndef OBJETOHEADER
#define OBJETOHEADER

#include "Ray.h"
#include "Vetor.h"
#include "../utils/Scene/sceneSchema.hpp"

struct HitRecord {
    double t;
    Ponto p;
    Vetor normal;
    bool front_face;
    MaterialData material;

    void set_face_normal(const Ray& r, const Vetor& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0.0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class Objeto {
public:
    virtual bool hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const = 0;
    virtual ~Objeto() = default;
};

#endif
