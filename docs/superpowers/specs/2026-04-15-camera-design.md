# Camera Design

**Date:** 2026-04-15
**Topic:** Movable camera class with ray generation

---

## Goal

Implement a `Camera` class that takes 6 parameters, computes an orthonormal basis (W, V, U), and can generate a ray for any pixel (i, j) on the screen.

---

## Constraints

- Pure C++17, depends only on `src/Ponto.h` and `src/Vetor.h`
- No dependency on the utils/Scene layer
- Consistent with existing project style (public attributes, no getters)
- Tests added to `tests/test_camera.h` and wired into `main.cpp`

---

## File Structure

```
src/Camera.h          ← Camera class (new)
tests/test_camera.h   ← test functions (new)
main.cpp              ← add #include + run_test calls inside RUN_TESTS block
```

---

## Class Definition

```cpp
// src/Camera.h
class Camera {
public:
    // Inputs
    Ponto C;       // camera position (lookfrom)
    Ponto M;       // look-at point
    Vetor Vup;     // up hint vector
    double d;      // screen distance
    int hres;      // horizontal resolution
    int vres;      // vertical resolution

    // Computed orthonormal basis
    Vetor W;       // unit_vector(C - M)  — points away from scene
    Vetor U;       // unit_vector(cross(Vup, W))  — points right
    Vetor V;       // cross(W, U)  — corrected up (already unit)

    Camera(Ponto C, Ponto M, Vetor Vup, double d, int hres, int vres);

    ray getRay(int i, int j) const;
};
```

---

## Basis Computation

Computed in the constructor in this order (each step depends on the previous):

```
W = unit_vector(C - M)
U = unit_vector(cross(Vup, W))
V = cross(W, U)
```

- **W** — opposite direction to (M − C), per the spec convention. Points away from the scene.
- **U** — right vector. `cross(Vup, W)` gives a vector pointing to the right; normalizing makes it unit.
- **V** — corrected up. Since W and U are both unit and orthogonal, their cross product is already unit.

---

## Ray Generation

The screen is 1 unit wide and 1 unit tall, centered at `C − d·W`. Pixel (i=0, j=0) is the top-left corner.

```
screen_center = C + W * (-d)

pixel_point = screen_center
            + U * (-0.5 + (i + 0.5) / hres)
            + V * ( 0.5 - (j + 0.5) / vres)

return ray(C, unit_vector(pixel_point - C))
```

The horizontal offset `(-0.5 + (i + 0.5) / hres)` maps i ∈ [0, hres-1] to the range (-0.5, 0.5).
The vertical offset `(0.5 - (j + 0.5) / vres)` maps j=0 (top) to near +0.5 and j=vres-1 (bottom) to near -0.5.

---

## Tests

Three test functions in `tests/test_camera.h`, registered in `main.cpp`:

### `camera_basis`

Input: `C=(0,0,5)`, `M=(0,0,0)`, `Vup=(0,1,0)`, `d=1`, `hres=4`, `vres=4`

Expected (exact integer arithmetic):
```
W = (0, 0, 1)
U = (1, 0, 0)
V = (0, 1, 0)
```

### `camera_ray_center`

Input: same camera, but `hres=1`, `vres=1`

`getRay(0, 0)` with a 1×1 screen hits the screen center exactly, so the ray direction equals `-W = (0, 0, -1)`.

Checks:
- `ray.origin() == C`
- `ray.direction() ≈ (0, 0, -1)` (epsilon comparison)

### `camera_ray_normalized`

Input: `C=(0,0,5)`, `M=(0,0,0)`, `Vup=(0,1,0)`, `d=1`, `hres=4`, `vres=4`

`getRay` at four corner pixels (0,0), (3,0), (0,3), (3,3) — all directions must have `length ≈ 1.0`.

---

## How to Use

```cpp
Camera cam(
    Ponto(0, 0, 5),   // C
    Ponto(0, 0, 0),   // M
    Vetor(0, 1, 0),   // Vup
    1.0,              // d
    256, 256          // hres, vres
);

for (int j = 0; j < cam.vres; j++)
    for (int i = 0; i < cam.hres; i++) {
        ray r = cam.getRay(i, j);
        // ... trace r ...
    }
```

---

## Compile & Test

```bash
# Run tests
g++ -std=c++17 -DRUN_TESTS main.cpp -o raytracer && raytracer

# Normal render
g++ -std=c++17 main.cpp -o raytracer && raytracer > image.ppm
```
