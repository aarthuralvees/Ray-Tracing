#pragma once

#include <cmath>
#include <fstream>
#include <string>

#include "../src/MalhaTriangulos.h"
#include "TestRunner.h"

static bool close_double(double a, double b) {
    return std::fabs(a - b) < 1e-9;
}

static std::string write_test_obj(const std::string& filename) {
    const std::string path = "/tmp/" + filename;
    std::ofstream out(path);
    out << "v 0 0 0\n";
    out << "v 1 0 0\n";
    out << "v 0 1 0\n";
    out << "f 1 2 3\n";
    return path;
}

static void matriz_transform_point_and_vector() {
    Matriz4 m = Matriz4::translation(2, 3, 4) * Matriz4::scaling(2, 2, 2);

    Ponto p = m.applyToPoint(Ponto(1, 1, 1));
    CHECK(close_double(p.getX(), 4.0));
    CHECK(close_double(p.getY(), 5.0));
    CHECK(close_double(p.getZ(), 6.0));

    Vetor v = m.applyToVector(Vetor(1, 1, 1));
    CHECK(close_double(v.getX(), 2.0));
    CHECK(close_double(v.getY(), 2.0));
    CHECK(close_double(v.getZ(), 2.0));
}

static void matriz_rotation_z_degrees() {
    Ponto p = Matriz4::rotationZ(90).applyToPoint(Ponto(1, 0, 0));
    CHECK(std::fabs(p.getX()) < 1e-9);
    CHECK(close_double(p.getY(), 1.0));
    CHECK(close_double(p.getZ(), 0.0));
}

static void matriz_transform_order() {
    TransformData scale;
    scale.tType = "scaling";
    scale.data = Vetor(2, 2, 2);

    TransformData translate;
    translate.tType = "translation";
    translate.data = Vetor(1, 0, 0);

    Matriz4 m = Matriz4::fromTransforms({scale, translate});
    Ponto p = m.applyToPoint(Ponto(1, 0, 0));
    CHECK(close_double(p.getX(), 3.0));
}

static void malha_loads_obj_and_computes_normals() {
    MalhaTriangulos malha(write_test_obj("ray_tracing_triangle_normals.obj"), color(0.2, 0.4, 0.6));

    CHECK(malha.vertices.size() == 3);
    CHECK(malha.triangulos.size() == 1);
    CHECK(malha.normais_triangulos.size() == 1);
    CHECK(malha.normais_vertices.size() == 3);
    CHECK(close_double(malha.normais_triangulos[0].getZ(), 1.0));
    CHECK(close_double(malha.normais_vertices[0].getZ(), 1.0));
}

static void malha_hits_triangle() {
    MalhaTriangulos malha(write_test_obj("ray_tracing_triangle_hit.obj"), color(0.2, 0.4, 0.6));
    Ray r(Ponto(0.25, 0.25, 1), Vetor(0, 0, -1));
    HitRecord rec;

    CHECK(malha.hit(r, 0.001, 1000.0, rec));
    CHECK(close_double(rec.t, 1.0));
    CHECK(close_double(rec.p.getX(), 0.25));
    CHECK(close_double(rec.p.getY(), 0.25));
    CHECK(close_double(rec.p.getZ(), 0.0));
    CHECK(close_double(rec.normal.getZ(), 1.0));
    CHECK(close_double(rec.cor_difusa.getX(), 0.2));
}

static void malha_applies_transform_before_hit() {
    std::vector<TransformData> transforms;

    TransformData translate;
    translate.tType = "translation";
    translate.data = Vetor(0, 0, 2);
    transforms.push_back(translate);

    MalhaTriangulos malha(
        write_test_obj("ray_tracing_triangle_transform.obj"),
        color(1, 0, 0),
        Matriz4::fromTransforms(transforms)
    );

    Ray r(Ponto(0.25, 0.25, 3), Vetor(0, 0, -1));
    HitRecord rec;

    CHECK(malha.hit(r, 0.001, 1000.0, rec));
    CHECK(close_double(rec.t, 1.0));
    CHECK(close_double(rec.p.getZ(), 2.0));
}

static void scene_parser_accepts_transforms_key() {
    const std::string json =
        "{"
        "\"objects\":[{"
        "\"type\":\"mesh\","
        "\"path\":\"triangle.obj\","
        "\"transforms\":[{\"type\":\"translation\",\"vector\":[1,2,3]}]"
        "}]"
        "}";

    SceneData scene = SceneJsonLoader::loadString(json);
    CHECK(scene.objects.size() == 1);
    CHECK(scene.objects[0].transforms.size() == 1);
    CHECK(scene.objects[0].transforms[0].tType == "translation");
    CHECK(close_double(scene.objects[0].transforms[0].data.getX(), 1.0));
    CHECK(close_double(scene.objects[0].transforms[0].data.getY(), 2.0));
    CHECK(close_double(scene.objects[0].transforms[0].data.getZ(), 3.0));
}

static void scene_parser_reads_upvector_key() {
    const std::string json =
        "{"
        "\"camera\":{"
        "\"upVector\":[1,2,3]"
        "}"
        "}";

    SceneData scene = SceneJsonLoader::loadString(json);
    CHECK(close_double(scene.camera.upVector.getX(), 1.0));
    CHECK(close_double(scene.camera.upVector.getY(), 2.0));
    CHECK(close_double(scene.camera.upVector.getZ(), 3.0));
}
