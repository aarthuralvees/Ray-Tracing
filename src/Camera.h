#ifndef CAMERAHEADER
#define CAMERAHEADER

#include "Ponto.h"
#include "Vetor.h"
#include "Ray.h"

class Camera {
public:
    // Inputs
    Ponto C;
    Ponto M;
    Vetor Vup;
    double d;
    int hres;
    int vres;

    Vetor W;   
    Vetor U;   
    Vetor V;   

    Camera(Ponto C, Ponto M, Vetor Vup, double d, int hres, int vres)
        : C(C), M(M), Vup(Vup), d(d), hres(hres), vres(vres)
    {
        W = unit_vector(C - M);
        U = unit_vector(cross(W, Vup));
        V = cross(W, U);
    }

    Ray getRay(int i, int j) const {
        Ponto screen_center = C + W * (-d);
        Ponto pixel_point = screen_center
            + U * (-0.5 + (i + 0.5) / static_cast<double>(hres))
            + V * ( 0.5 - (j + 0.5) / static_cast<double>(vres));
        return Ray(C, unit_vector(pixel_point - C));
    }
};

#endif
