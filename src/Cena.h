#ifndef CENAHEADER
#define CENAHEADER

#include <vector>
#include <memory>
#include "Objeto.h"
#include "Ray.h"

class Cena {
public:
    std::vector<std::unique_ptr<Objeto>> objetos;

    void adicionar(std::unique_ptr<Objeto> obj) {
        objetos.push_back(std::move(obj));
    }

    bool hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const {
        HitRecord temp_rec;
        bool hit_anything = false;
        double closest_so_far = t_max;

        for (const auto& object : objetos) {
            if (object->hit(r, t_min, closest_so_far, temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }
        return hit_anything;
    }

    color ray_color(const Ray& r) const {
        HitRecord rec;
        
        if (hit(r, 0.001, 1000000.0, rec)) {
            return rec.cor_difusa; 
        }
        
        return color(0, 0, 0); 
    }
};

#endif