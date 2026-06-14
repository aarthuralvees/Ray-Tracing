#pragma once

#include "../src/Cena.h"
#include "../src/Esfera.h"
#include "../src/Plano.h"
#include "TestRunner.h"

static MaterialData test_material(
    const ColorData& kd = ColorData(0, 0, 0),
    const ColorData& ka = ColorData(0, 0, 0),
    const ColorData& ks = ColorData(0, 0, 0),
    const ColorData& kr = ColorData(0, 0, 0),
    const ColorData& kt = ColorData(0, 0, 0),
    double ns = 1.0,
    double ni = 1.0
) {
    MaterialData mat;
    mat.name = "test";
    mat.color = kd;
    mat.ka = ka;
    mat.ks = ks;
    mat.kr = kr;
    mat.kt = kt;
    mat.ns = ns;
    mat.ni = ni;
    mat.d = 1.0;
    return mat;
}

static void sphere_records_front_and_back_faces() {
    Esfera sphere(Ponto(0, 0, 0), 1.0, test_material());
    HitRecord rec;

    CHECK(sphere.hit(Ray(Ponto(0, 0, -3), Vetor(0, 0, 1)), 0.001, 1000.0, rec));
    CHECK(rec.front_face);
    CHECK(close_double(rec.normal.getZ(), -1.0));

    CHECK(sphere.hit(Ray(Ponto(0, 0, 0), Vetor(0, 0, 1)), 0.001, 1000.0, rec));
    CHECK(!rec.front_face);
    CHECK(close_double(rec.normal.getZ(), -1.0));
}

static void recursive_reflection_contributes_secondary_color() {
    Cena scene;
    scene.setLights({}, ColorData(1, 1, 1));

    MaterialData mirror = test_material(
        ColorData(0, 0, 0),
        ColorData(0, 0, 0),
        ColorData(0, 0, 0),
        ColorData(1, 1, 1)
    );
    MaterialData red = test_material(
        ColorData(0, 0, 0),
        ColorData(1, 0, 0)
    );

    scene.adicionar(std::make_unique<Plano>(Ponto(0, 0, 0), Vetor(0, 0, 1), mirror));
    scene.adicionar(std::make_unique<Esfera>(Ponto(0, 0, 2), 0.5, red));

    color c = scene.ray_color(Ray(Ponto(0, 0, 1), Vetor(0, 0, -1)), 3);
    CHECK(c.getX() > 0.99);
    CHECK(c.getY() < 1e-9);
    CHECK(c.getZ() < 1e-9);
}

static void recursive_refraction_contributes_secondary_color() {
    Cena scene;
    scene.setLights({}, ColorData(1, 1, 1));

    MaterialData glass = test_material(
        ColorData(0, 0, 0),
        ColorData(0, 0, 0),
        ColorData(0, 0, 0),
        ColorData(0, 0, 0),
        ColorData(1, 1, 1),
        1.0,
        1.5
    );
    MaterialData blue = test_material(
        ColorData(0, 0, 0),
        ColorData(0, 0, 1)
    );

    scene.adicionar(std::make_unique<Plano>(Ponto(0, 0, 0), Vetor(0, 0, 1), glass));
    scene.adicionar(std::make_unique<Esfera>(Ponto(0, 0, -2), 0.5, blue));

    color c = scene.ray_color(Ray(Ponto(0, 0, 1), Vetor(0, 0, -1)), 3);
    CHECK(c.getX() < 1e-9);
    CHECK(c.getY() < 1e-9);
    CHECK(c.getZ() > 0.99);
}
