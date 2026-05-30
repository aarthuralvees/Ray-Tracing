# Entrega 3 — Phong Illumination + Shadows

**Date:** 2026-05-30  
**Scope:** Add Phong illumination model (no recursion) and hard shadows to the ray tracer.

---

## Goal

Replace the current flat-color shading in `Cena::ray_color()` with the Phong illumination model driven by per-object material coefficients and scene light sources. Cast shadow rays to skip occluded lights. Skip the `kr·Ir + kt·It` reflection/refraction terms (those belong to a later deliverable).

---

## Phong Formula

```
I = ka ⊗ Ia  +  Σn [ kd ⊗ (max(Ln·N, 0) · ILn)  +  ks ⊗ (max(Rn·V, 0)^η · ILn) ]
```

- `⊗` — component-wise (Hadamard) multiply
- `Ia` — ambient light color (`globalLight`, RGB in [0,1])
- `ILn` — intensity of light n (RGB in [0,1])
- `Ln` — unit vector from hit point toward light n
- `N` — surface normal at hit point (always pointing toward the incoming ray)
- `V` — unit vector from hit point toward the camera (`-ray.direction()`, normalised)
- `Rn` — reflection of `Ln` about `N`: `2·(Ln·N)·N - Ln`
- `η` — shininess / roughness (`ns` in material)
- Final RGB is clamped component-wise to `[0, 1]`

Shadow test: before adding light n's contribution, cast a ray from `hit_point + ε·Ln` toward the light. If anything is hit at `t < distance_to_light`, that light is skipped entirely.

---

## Files Changed

### `src/Vetor.h`
- Add `hadamard(color a, color b)` — component-wise multiply, returns `color`.
- Add `clamp(color c, double lo, double hi)` — per-component clamp.

### `src/Objeto.h`
- Add `#include "../utils/Scene/sceneSchema.hpp"` (for `MaterialData`).
- Replace `color cor_difusa` in `HitRecord` with `MaterialData material`.

### `src/Esfera.h`
- Change stored field from `color cor` to `MaterialData material`.
- Constructor takes `MaterialData` instead of `color`.
- `hit()` sets `rec.material = material` instead of `rec.cor_difusa = cor`.

### `src/Plano.h`
- Same changes as `Esfera.h`.

### `src/MalhaTriangulos.h`
- Same changes as `Esfera.h`.

### `src/Cena.h`
- Add fields: `std::vector<LightData> luzes` and `ColorData luzAmbiente`.
- Add method `setLights(vector<LightData>, ColorData)` to wire them in from `main.cpp`.
- Replace `ray_color()` body with Phong computation:
  1. Test hit. If nothing hit, return background (black).
  2. Compute ambient term: `ka ⊗ Ia`.
  3. For each light: compute `Ln`, shadow ray, `Rn`, diffuse + specular terms.
  4. Sum and clamp. Return result.

### `main.cpp`
- Pass `MaterialData` (from `objData.material`) to `Esfera`, `Plano`, `MalhaTriangulos` constructors.
- After building `mundo`, call `mundo.setLights(scene_data.lightList, scene_data.globalLight.color)`.

---

## What Does NOT Change

- `sceneSchema.hpp` — already has all material and light fields.
- `sceneParser.cpp` — already parses everything correctly.
- `Camera.h`, `Ray.h`, `Ponto.h`, `Matriz4.h` — no changes.
- JSON scene files — existing files already use the right schema.

---

## Out of Scope

- Reflection (`kr · Ir`) and refraction (`kt · It`) — future deliverable.
- Soft shadows, area lights, anti-aliasing — not required.
