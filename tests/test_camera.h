#pragma once
#include <cmath>
#include "../src/Camera.h"
#include "TestRunner.h"

static void camera_basis() {
    // C=(0,0,5), M=(0,0,0), Vup=(0,1,0)
    // W = unit(C-M) = (0,0,1)
    // U = unit(cross(Vup, W)) = unit(cross((0,1,0),(0,0,1))) = (1,0,0)
    // V = cross(W, U) = cross((0,0,1),(1,0,0)) = (0,1,0)
    Camera cam(Ponto(0,0,5), Ponto(0,0,0), Vetor(0,1,0), 1.0, 4, 4);

    CHECK(fabs(cam.W.getX() - 0.0) < 1e-9);
    CHECK(fabs(cam.W.getY() - 0.0) < 1e-9);
    CHECK(fabs(cam.W.getZ() - 1.0) < 1e-9);

    CHECK(fabs(cam.U.getX() - 1.0) < 1e-9);
    CHECK(fabs(cam.U.getY() - 0.0) < 1e-9);
    CHECK(fabs(cam.U.getZ() - 0.0) < 1e-9);

    CHECK(fabs(cam.V.getX() - 0.0) < 1e-9);
    CHECK(fabs(cam.V.getY() - 1.0) < 1e-9);
    CHECK(fabs(cam.V.getZ() - 0.0) < 1e-9);
}

static void camera_ray_center() {
    // 1x1 resolution: getRay(0,0) hits screen center exactly
    // screen_center = C + W*(-d) = (0,0,5) + (0,0,1)*(-1) = (0,0,4)
    // pixel_point = screen_center + U*(−0.5+0.5/1) + V*(0.5−0.5/1)
    //             = screen_center + U*0 + V*0 = (0,0,4)
    // direction = unit((0,0,4)−(0,0,5)) = unit(0,0,−1) = (0,0,−1) = −W
    Camera cam(Ponto(0,0,5), Ponto(0,0,0), Vetor(0,1,0), 1.0, 1, 1);
    Ray r = cam.getRay(0, 0);

    CHECK(fabs(r.origin().getX() - 0.0) < 1e-9);
    CHECK(fabs(r.origin().getY() - 0.0) < 1e-9);
    CHECK(fabs(r.origin().getZ() - 5.0) < 1e-9);

    CHECK(fabs(r.direction().getX() - 0.0) < 1e-9);
    CHECK(fabs(r.direction().getY() - 0.0) < 1e-9);
    CHECK(fabs(r.direction().getZ() - (-1.0)) < 1e-9);
}

static void camera_ray_normalized() {
    Camera cam(Ponto(0,0,5), Ponto(0,0,0), Vetor(0,1,0), 1.0, 4, 4);

    CHECK(fabs(cam.getRay(0, 0).direction().length() - 1.0) < 1e-9);
    CHECK(fabs(cam.getRay(3, 0).direction().length() - 1.0) < 1e-9);
    CHECK(fabs(cam.getRay(0, 3).direction().length() - 1.0) < 1e-9);
    CHECK(fabs(cam.getRay(3, 3).direction().length() - 1.0) < 1e-9);
}

static void camera_ray_corner_directions() {
    // Verifies the actual pixel mapping formula for a non-center pixel.
    // C=(0,0,5), M=(0,0,0), d=1, hres=4, vres=4
    // W=(0,0,1), U=(1,0,0), V=(0,1,0)
    // screen_center = (0,0,4)
    // pixel (0,0) top-left:
    //   U-offset = -0.5 + 0.5/4 = -0.375
    //   V-offset =  0.5 - 0.5/4 =  0.375
    //   pixel_point = (-0.375, 0.375, 4)
    //   raw direction = pixel_point - C = (-0.375, 0.375, -1)
    //   length = sqrt(0.375^2 + 0.375^2 + 1) = sqrt(1.28125)
    Camera cam(Ponto(0,0,5), Ponto(0,0,0), Vetor(0,1,0), 1.0, 4, 4);
    Ray r = cam.getRay(0, 0);

    double len = sqrt(0.375*0.375 + 0.375*0.375 + 1.0);
    CHECK(fabs(r.direction().getX() - (-0.375 / len)) < 1e-9);
    CHECK(fabs(r.direction().getY() - ( 0.375 / len)) < 1e-9);
    CHECK(fabs(r.direction().getZ() - (-1.0   / len)) < 1e-9);
}
