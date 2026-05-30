# Phong Illumination + Shadows Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace flat-color shading with the Phong illumination model and hard shadows.

**Architecture:** `HitRecord` is extended to carry full `MaterialData`. Each object stores its material and populates the record on hit. `Cena::ray_color()` uses the material + scene lights to compute Phong shading, casting a shadow ray per light before adding its contribution.

**Tech Stack:** C++17, single-TU header-only project compiled with `g++ -std=c++17`. Build: `make` / `make test`.

---

## Files Changed

| File | Change |
|---|---|
| `src/Vetor.h` | Add `hadamard()` and `clamp_color()` |
| `src/Objeto.h` | Replace `cor_difusa` with `MaterialData material` in `HitRecord`; include `sceneSchema.hpp` |
| `src/Esfera.h` | Store `MaterialData`; update constructor + `hit()` |
| `src/Plano.h` | Store `MaterialData`; update constructor + `hit()` |
| `src/MalhaTriangulos.h` | Store `MaterialData`; update constructor + `hit()` |
| `src/Cena.h` | Add `luzes` + `luzAmbiente` fields; add `setLights()`; implement Phong in `ray_color()` |
| `tests/test_vetor.h` | Add `vetor_hadamard` and `color_clamp` tests |
| `tests/test_malha.h` | Update constructor calls and `cor_difusa` reference |
| `main.cpp` | Register new tests; pass `MaterialData` to constructors; call `mundo.setLights()` |

---

## Task 1: Add `hadamard` and `clamp_color` to `Vetor.h`

**Files:**
- Modify: `src/Vetor.h`
- Modify: `tests/test_vetor.h`
- Modify: `main.cpp` (register new tests)

- [ ] **Step 1: Add the two failing tests to `tests/test_vetor.h`**

Add at the end of the file (after `vetor_unit_vector`):

```cpp
static void vetor_hadamard() {
    color a(0.5, 0.25, 1.0), b(0.8, 0.4, 0.5);
    color h = hadamard(a, b);
    CHECK(fabs(h.getX() - 0.40) < 1e-9);
    CHECK(fabs(h.getY() - 0.10) < 1e-9);
    CHECK(fabs(h.getZ() - 0.50) < 1e-9);
}

static void color_clamp() {
    color over(-0.1, 0.5, 1.5);
    color clamped = clamp_color(over);
    CHECK(fabs(clamped.getX() - 0.0) < 1e-9);
    CHECK(fabs(clamped.getY() - 0.5) < 1e-9);
    CHECK(fabs(clamped.getZ() - 1.0) < 1e-9);
}
```

- [ ] **Step 2: Register the tests in `main.cpp`**

Inside `run_all_tests()`, add after `run_test("vetor_unit_vector", vetor_unit_vector);`:

```cpp
run_test("vetor_hadamard", vetor_hadamard);
run_test("color_clamp",    color_clamp);
```

- [ ] **Step 3: Run tests and confirm the new ones FAIL**

```
make test
```

Expected: compilation fails or `vetor_hadamard` / `color_clamp` report FAIL (functions not defined yet).

- [ ] **Step 4: Add `hadamard` and `clamp_color` to `src/Vetor.h`**

Add after the `write_color` function at the end of `src/Vetor.h` (before `#endif`):

```cpp
inline color hadamard(const color& a, const color& b) {
    return color(a.getX() * b.getX(), a.getY() * b.getY(), a.getZ() * b.getZ());
}

inline color clamp_color(const color& c) {
    auto cl = [](double v) { return v < 0.0 ? 0.0 : v > 1.0 ? 1.0 : v; };
    return color(cl(c.getX()), cl(c.getY()), cl(c.getZ()));
}
```

- [ ] **Step 5: Run tests and confirm all pass**

```
make test
```

Expected: all checks pass, including `vetor_hadamard` and `color_clamp`.

- [ ] **Step 6: Commit**

```
git add src/Vetor.h tests/test_vetor.h main.cpp
git commit -m "feat: add hadamard and clamp_color color utilities"
```

---

## Task 2: Migrate `HitRecord` to `MaterialData` and update all object classes

**Files:**
- Modify: `src/Objeto.h`
- Modify: `src/Esfera.h`
- Modify: `src/Plano.h`
- Modify: `src/MalhaTriangulos.h`
- Modify: `tests/test_malha.h`
- Modify: `main.cpp`

- [ ] **Step 1: Update `src/Objeto.h`**

Replace the entire file content with:

```cpp
#ifndef OBJETOHEADER
#define OBJETOHEADER

#include "Ray.h"
#include "Vetor.h"
#include "../utils/Scene/sceneSchema.hpp"

struct HitRecord {
    double t;
    Ponto p;
    Vetor normal;
    MaterialData material;
};

class Objeto {
public:
    virtual bool hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const = 0;
    virtual ~Objeto() = default;
};

#endif
```

- [ ] **Step 2: Update `src/Esfera.h`**

Replace the entire file content with:

```cpp
#ifndef ESFERAHEADER
#define ESFERAHEADER

#include "Objeto.h"

class Esfera : public Objeto {
public:
    Ponto centro;
    double raio;
    MaterialData material;

    Esfera(Ponto c, double r, MaterialData mat) : centro(c), raio(r), material(mat) {}

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
        rec.material = material;
        return true;
    }
};

#endif
```

- [ ] **Step 3: Update `src/Plano.h`**

Replace the entire file content with:

```cpp
#ifndef PLANOHEADER
#define PLANOHEADER

#include "Objeto.h"

class Plano : public Objeto {
public:
    Ponto p0;
    Vetor normal;
    MaterialData material;

    Plano(Ponto p0, Vetor n, MaterialData mat) : p0(p0), normal(unit_vector(n)), material(mat) {}

    bool hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const override {
        double denom = dot(normal, r.direction());
        if (std::abs(denom) < 1e-8) return false;

        double t = dot(p0 - r.origin(), normal) / denom;
        if (t <= t_min || t >= t_max) return false;

        rec.t = t;
        rec.p = r.at(t);
        rec.normal = normal;
        if (dot(r.direction(), rec.normal) > 0) rec.normal = -rec.normal;
        rec.material = material;
        return true;
    }
};

#endif
```

- [ ] **Step 4: Update `src/MalhaTriangulos.h`**

Change the two constructors and the `cor` field. Replace the top of the class (lines 15-32) with:

```cpp
class MalhaTriangulos : public Objeto {
public:
    using Triangulo = std::array<int, 3>;

    std::vector<Ponto> vertices;
    std::vector<Triangulo> triangulos;
    std::vector<Vetor> normais_triangulos;
    std::vector<Vetor> normais_vertices;
    MaterialData material;

    MalhaTriangulos(const std::string& objPath, MaterialData mat)
        : MalhaTriangulos(objPath, mat, Matriz4::identity()) {}

    MalhaTriangulos(const std::string& objPath, MaterialData mat, const Matriz4& transform)
        : material(mat) {
        loadObj(objPath);
        applyTransform(transform);
        computeNormals();
    }
```

Then at the end of `hitTriangle()`, replace `rec.cor_difusa = cor;` with `rec.material = material;`:

```cpp
        rec.normal = normal;
        rec.material = material;
        return true;
```

- [ ] **Step 5: Update `tests/test_malha.h` — fix constructor calls and `cor_difusa` reference**

In `malha_loads_obj_and_computes_normals`, change:
```cpp
// old
MalhaTriangulos malha(write_test_obj("ray_tracing_triangle_normals.obj"), color(0.2, 0.4, 0.6));
// new
MaterialData mat; mat.color = ColorData(0.2, 0.4, 0.6);
MalhaTriangulos malha(write_test_obj("ray_tracing_triangle_normals.obj"), mat);
```

In `malha_hits_triangle`, change the constructor and the color check:
```cpp
// old
MalhaTriangulos malha(write_test_obj("ray_tracing_triangle_hit.obj"), color(0.2, 0.4, 0.6));
// new
MaterialData mat; mat.color = ColorData(0.2, 0.4, 0.6);
MalhaTriangulos malha(write_test_obj("ray_tracing_triangle_hit.obj"), mat);
```
And replace the `cor_difusa` check:
```cpp
// old
CHECK(close_double(rec.cor_difusa.getX(), 0.2));
// new
CHECK(close_double(rec.material.color.r, 0.2));
```

In `malha_applies_transform_before_hit`, change the constructor:
```cpp
// old
MalhaTriangulos malha(
    write_test_obj("ray_tracing_triangle_transform.obj"),
    color(1, 0, 0),
    Matriz4::fromTransforms(transforms)
);
// new
MaterialData matRed; matRed.color = ColorData(1, 0, 0);
MalhaTriangulos malha(
    write_test_obj("ray_tracing_triangle_transform.obj"),
    matRed,
    Matriz4::fromTransforms(transforms)
);
```

- [ ] **Step 6: Update `main.cpp` — pass `MaterialData` to object constructors**

Remove the `color objColor(...)` line and pass `objData.material` directly. Replace the object-building loop:

```cpp
for (auto& objData : scene_data.objects) {
    if (objData.objType == "sphere") {
        Ponto centro = objData.getPonto("center");
        double raio = objData.getNum("radius");
        mundo.adicionar(make_unique<Esfera>(centro, raio, objData.material));
    }
    else if (objData.objType == "plane") {
        Ponto p0 = objData.getPonto("point_on_plane");
        Vetor normal = objData.getVetor("normal");
        mundo.adicionar(make_unique<Plano>(p0, normal, objData.material));
    }
    else if (objData.objType == "mesh") {
        std::string objPath = resolvePath(scenePath, objData.getProperty("path"));
        mundo.adicionar(make_unique<MalhaTriangulos>(objPath, objData.material, Matriz4::fromTransforms(objData.transforms)));
    }
}
```

- [ ] **Step 7: Run tests and confirm all pass**

```
make test
```

Expected: all checks pass (including the matrix, mesh, and scene-parser tests).

- [ ] **Step 8: Commit**

```
git add src/Objeto.h src/Esfera.h src/Plano.h src/MalhaTriangulos.h tests/test_malha.h main.cpp
git commit -m "refactor: store MaterialData in HitRecord and all object classes"
```

---

## Task 3: Implement Phong illumination + shadows in `Cena`

**Files:**
- Modify: `src/Cena.h`
- Modify: `main.cpp`

- [ ] **Step 1: Update `src/Cena.h`**

Replace the entire file with:

```cpp
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

        // Ambient: ka * Ia
        color kd(mat.color.r, mat.color.g, mat.color.b);
        color ka(mat.ka.r,    mat.ka.g,    mat.ka.b);
        color ks(mat.ks.r,    mat.ks.g,    mat.ks.b);
        color Ia(luzAmbiente.r, luzAmbiente.g, luzAmbiente.b);

        color result = hadamard(ka, Ia);

        Vetor V = unit_vector(-r.direction());

        for (const auto& luz : luzes) {
            Vetor toLight = luz.pos - rec.p;
            double distToLight = toLight.length();
            if (distToLight < 1e-8) continue;
            Vetor Ln = unit_vector(toLight);

            // Shadow: skip light if occluded
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
```

- [ ] **Step 2: Wire lights into `Cena` in `main.cpp`**

After the object-building loop (after the closing `}` of the `for` loop over objects), add:

```cpp
mundo.setLights(scene_data.lightList, scene_data.globalLight.color);
```

- [ ] **Step 3: Build the renderer**

```
make
```

Expected: compiles with no errors.

- [ ] **Step 4: Render the sample scene and inspect the output**

```
render.exe utils/input/sampleScene.json > image.ppm
```

Open `image.ppm` in an image viewer (e.g., GIMP, IrfanView, or any PPM-capable viewer).

Expected: colored walls with Phong shading visible — lighter where the ceiling light hits directly, darker near corners. Spheres show specular highlights. Shadow regions are darker than lit regions.

- [ ] **Step 5: Run tests to confirm nothing regressed**

```
make test
```

Expected: all checks pass.

- [ ] **Step 6: Commit**

```
git add src/Cena.h main.cpp
git commit -m "feat: implement Phong illumination model and hard shadows (Entrega 3)"
```
