#pragma once

#include <filesystem>
#include <fstream>
#include <string>

#include "../src/SuperficieBezier.h"
#include "TestRunner.h"

static SuperficieBezier::PontosControle planar_bezier_patch() {
    SuperficieBezier::PontosControle control;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            control[i][j] = Ponto(static_cast<double>(i) / 3.0, static_cast<double>(j) / 3.0, 0.0);
    return control;
}

static std::string write_test_bezier(const std::string& filename) {
    const std::string path = (std::filesystem::temp_directory_path() / filename).string();
    std::ofstream out(path);
    auto control = planar_bezier_patch();
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            out << control[i][j].getX() << ' ' << control[i][j].getY() << ' ' << control[i][j].getZ() << '\n';
    return path;
}

static void bezier_tessellates_patch() {
    MaterialData mat; mat.color = ColorData(0.2, 0.4, 0.6);
    SuperficieBezier bezier(planar_bezier_patch(), mat, 4);

    CHECK(bezier.vertices.size() == 25);
    CHECK(bezier.triangulos.size() == 32);
    CHECK(bezier.normais_vertices.size() == 25);
    CHECK(close_double(bezier.vertices.front().getX(), 0.0));
    CHECK(close_double(bezier.vertices.back().getX(), 1.0));
    CHECK(close_double(bezier.vertices.back().getY(), 1.0));
}

static void bezier_hits_planar_patch() {
    MaterialData mat; mat.color = ColorData(0.7, 0.1, 0.2);
    SuperficieBezier bezier(write_test_bezier("ray_tracing_planar_patch.bez"), mat, 8);
    Ray r(Ponto(0.25, 0.25, 1), Vetor(0, 0, -1));
    HitRecord rec;

    CHECK(bezier.hit(r, 0.001, 1000.0, rec));
    CHECK(close_double(rec.t, 1.0));
    CHECK(close_double(rec.p.getX(), 0.25));
    CHECK(close_double(rec.p.getY(), 0.25));
    CHECK(close_double(rec.p.getZ(), 0.0));
    CHECK(close_double(rec.normal.getZ(), 1.0));
    CHECK(close_double(rec.material.color.r, 0.7));
}

static void bezier_applies_transform_before_hit() {
    TransformData translate;
    translate.tType = "translation";
    translate.data = Vetor(0, 0, 2);

    MaterialData mat; mat.color = ColorData(1, 1, 1);
    SuperficieBezier bezier(planar_bezier_patch(), mat, 4, Matriz4::fromTransforms({translate}));
    Ray r(Ponto(0.25, 0.25, 3), Vetor(0, 0, -1));
    HitRecord rec;

    CHECK(bezier.hit(r, 0.001, 1000.0, rec));
    CHECK(close_double(rec.t, 1.0));
    CHECK(close_double(rec.p.getZ(), 2.0));
}

static void scene_parser_reads_bezier_resolution() {
    const std::string json =
        "{"
        "\"objects\":[{"
        "\"type\":\"bezier\","
        "\"path\":\"patch.bez\","
        "\"resolution\":12"
        "}]"
        "}";

    SceneData scene = SceneJsonLoader::loadString(json);
    CHECK(scene.objects.size() == 1);
    CHECK(scene.objects[0].objType == "bezier");
    CHECK(scene.objects[0].getProperty("path") == "patch.bez");
    CHECK(scene.objects[0].getInt("resolution") == 12);
}
