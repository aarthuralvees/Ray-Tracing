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

    color ray_color(const Ray& r, int depth = 6) const {
        if (depth <= 0) return color(0, 0, 0);

        HitRecord rec;
        if (!hit(r, 0.001, 1000000.0, rec)) return color(0, 0, 0);

        const MaterialData& mat = rec.material;

        color kd(mat.color.r, mat.color.g, mat.color.b);
        color ka(mat.ka.r,    mat.ka.g,    mat.ka.b);
        color ks(mat.ks.r,    mat.ks.g,    mat.ks.b);
        color kr(mat.kr.r,    mat.kr.g,    mat.kr.b);
        color kt(mat.kt.r,    mat.kt.g,    mat.kt.b);
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

            // Specular: ks * max(Rn.V, 0)^ns * ILn (only when light faces surface)
            if (diff > 0.0 && mat.ns > 0.0) {
                Vetor Rn = unit_vector(2.0 * dot(Ln, rec.normal) * rec.normal - Ln);
                double spec = std::pow(std::max(dot(Rn, V), 0.0), mat.ns);
                result = result + hadamard(ks, ILn * spec);
            }
        }

        if (kr.length_squared() > 1e-12) {
            Vetor reflected = unit_vector(reflect(unit_vector(r.direction()), rec.normal));
            color reflected_color = ray_color(Ray(rec.p, reflected), depth - 1);
            result = result + hadamard(kr, reflected_color);
        }

        if (kt.length_squared() > 1e-12) {
            const double ni = mat.ni >= 1.0 ? mat.ni : 1.0;
            const double eta_ratio = rec.front_face ? (1.0 / ni) : ni;
            Vetor refracted;

            if (refract(unit_vector(r.direction()), rec.normal, eta_ratio, refracted)) {
                color refracted_color = ray_color(Ray(rec.p, unit_vector(refracted)), depth - 1);
                result = result + hadamard(kt, refracted_color);
            } else {
                Vetor reflected = unit_vector(reflect(unit_vector(r.direction()), rec.normal));
                color reflected_color = ray_color(Ray(rec.p, reflected), depth - 1);
                result = result + hadamard(kt, reflected_color);
            }
        }

        return clamp_color(result);
    }
};

#endif
