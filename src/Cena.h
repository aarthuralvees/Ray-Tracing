#ifndef CENAHEADER
#define CENAHEADER

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>
#include "Objeto.h"
#include "Ray.h"

class Cena {
public:
    std::vector<std::unique_ptr<Objeto>> objetos;
    std::vector<LightData> luzes;
    ColorData luzAmbiente;

    Cena() : luzAmbiente(0, 0, 0) {}

    void adicionar(std::unique_ptr<Objeto> obj) {
        objetos.push_back(std::move(obj));
    }

    void setLights(const std::vector<LightData>& lights, const ColorData& ambient) {
        luzes = lights;
        luzAmbiente = ambient;
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
        if (!hit(r, 0.001, 1000000.0, rec)) return color(0, 0, 0);

        const MaterialData& mat = rec.material;

        color kd(mat.color.r, mat.color.g, mat.color.b);
        color ka(mat.ka.r,    mat.ka.g,    mat.ka.b);
        color ks(mat.ks.r,    mat.ks.g,    mat.ks.b);
        color Ia(luzAmbiente.r, luzAmbiente.g, luzAmbiente.b);

        // Ambient term: ka * Ia
        color result = hadamard(ka, Ia);

        Vetor V = unit_vector(-r.direction());

        for (const auto& luz : luzes) {
            Vetor toLight = luz.pos - rec.p;
            double distToLight = toLight.length();
            if (distToLight < 1e-8) continue;
            Vetor Ln = unit_vector(toLight);

            // Shadow: skip this light if something blocks it
            HitRecord shadowRec;
            if (hit(Ray(rec.p, Ln), 0.001, distToLight - 0.001, shadowRec)) continue;

            color ILn(luz.color.r, luz.color.g, luz.color.b);

            // Diffuse: kd * max(Ln.N, 0) * ILn
            double diff = std::max(dot(Ln, rec.normal), 0.0);
            result = result + hadamard(kd, ILn * diff);

            // Specular: ks * max(Rn.V, 0)^ns * ILn
            Vetor Rn = unit_vector(2.0 * dot(Ln, rec.normal) * rec.normal - Ln);
            double spec = (mat.ns > 0.0)
                ? std::pow(std::max(dot(Rn, V), 0.0), mat.ns)
                : 0.0;
            result = result + hadamard(ks, ILn * spec);
        }

        return clamp_color(result);
    }
};

#endif
