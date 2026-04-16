#pragma once
#include "Ponto.h"
#include "Vetor.h"

class Camera {
public:
    // Inputs
    Ponto C;
    Ponto M;
    Vetor Vup;
    double d;
    int hres;
    int vres;

    // Computed orthonormal basis
    Vetor W;   // unit_vector(C - M)        — points away from scene
    Vetor U;   // unit_vector(cross(Vup,W)) — points right
    Vetor V;   // cross(W, U)               — corrected up (already unit)

    Camera(Ponto C, Ponto M, Vetor Vup, double d, int hres, int vres)
        : C(C), M(M), Vup(Vup), d(d), hres(hres), vres(vres)
    {
        W = unit_vector(C - M);
        U = unit_vector(cross(Vup, W));
        V = cross(W, U);
    }

    ray getRay(int i, int j) const {
        Ponto screen_center = C + W * (-d);
        Ponto pixel_point = screen_center
            + U * (-0.5 + (i + 0.5) / hres)
            + V * ( 0.5 - (j + 0.5) / vres);
        return ray(C, unit_vector(pixel_point - C));
    }
};
