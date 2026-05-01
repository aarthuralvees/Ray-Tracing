#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include "src/Ponto.h"
#include "src/Vetor.h"
#include "src/Ray.h"
#include "src/Camera.h"
#include "src/Objeto.h"
#include "src/Esfera.h"
#include "src/Plano.h"
#include "src/MalhaTriangulos.h"
#include "src/Cena.h"

#include "utils/Scene/sceneParser.cpp" 

using namespace std;

#ifdef RUN_TESTS
#include "tests/test_vetor.h"
#include "tests/test_ponto.h"
#include "tests/test_camera.h"
#include "tests/test_malha.h"

static void run_all_tests() {
    run_test("vetor_arithmetic",          vetor_arithmetic);
    run_test("vetor_length",              vetor_length);
    run_test("vetor_dot_cross",           vetor_dot_cross);
    run_test("vetor_unit_vector",         vetor_unit_vector);
    run_test("ponto_arithmetic",          ponto_arithmetic);
    run_test("ray_at",                    ray_at);
    run_test("camera_basis",              camera_basis);
    run_test("camera_ray_center",         camera_ray_center);
    run_test("camera_ray_normalized",     camera_ray_normalized);
    run_test("camera_ray_corner_directions", camera_ray_corner_directions);
    run_test("matriz_transform_point_and_vector", matriz_transform_point_and_vector);
    run_test("matriz_rotation_z_degrees", matriz_rotation_z_degrees);
    run_test("matriz_transform_order",    matriz_transform_order);
    run_test("malha_loads_obj_and_computes_normals", malha_loads_obj_and_computes_normals);
    run_test("malha_hits_triangle",       malha_hits_triangle);
    run_test("malha_applies_transform_before_hit", malha_applies_transform_before_hit);
    run_test("scene_parser_accepts_transforms_key", scene_parser_accepts_transforms_key);
    run_test("scene_parser_reads_upvector_key", scene_parser_reads_upvector_key);
    report_tests();
}
#endif

static std::string dirnameOf(const std::string& path) {
    const size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) return ".";
    return path.substr(0, pos);
}

static bool fileExists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

static std::string resolvePath(const std::string& baseFile, const std::string& path) {
    if (path.empty()) return path;
    if (path[0] == '/' || fileExists(path)) return path;

    const std::string relativeToScene = dirnameOf(baseFile) + "/" + path;
    if (fileExists(relativeToScene)) return relativeToScene;

    return path;
}

int main(int argc, char** argv) {
#ifdef RUN_TESTS
    run_all_tests();
    return tests::_failed > 0 ? 1 : 0;
#endif

    const std::string scenePath = argc > 1 ? argv[1] : "utils/input/sampleScene.json";
    SceneData scene_data = SceneJsonLoader::loadFile(scenePath);

    Ponto lookfrom = scene_data.camera.lookfrom;
    Ponto lookat   = scene_data.camera.lookat;
    Vetor vup      = scene_data.camera.upVector;
    
    Camera cam(lookfrom, lookat, vup, scene_data.camera.screen_distance, scene_data.camera.image_width, scene_data.camera.image_height);

    Cena mundo;

    for (auto& objData : scene_data.objects) {
        color objColor(objData.material.color.r, objData.material.color.g, objData.material.color.b);
        
        if (objData.objType == "sphere") {
            Ponto centro = objData.getPonto("center");
            double raio = objData.getNum("radius");
            mundo.adicionar(make_unique<Esfera>(centro, raio, objColor));
        } 
        else if (objData.objType == "plane") {
            Ponto p0 = objData.getPonto("point_on_plane"); 
            Vetor normal = objData.getVetor("normal");
            mundo.adicionar(make_unique<Plano>(p0, normal, objColor));
        }
        else if (objData.objType == "mesh") {
            std::string objPath = resolvePath(scenePath, objData.getProperty("path"));
            mundo.adicionar(make_unique<MalhaTriangulos>(objPath, objColor, Matriz4::fromTransforms(objData.transforms)));
        }
    }

    std::cout << "P3\n" << cam.hres << ' ' << cam.vres << "\n255\n";

    for (int j = 0; j < cam.vres; j++) {
        std::clog << "\rScanlines remaining: " << (cam.vres - j) << ' ' << std::flush;
        for (int i = 0; i < cam.hres; i++) {
            Ray r = cam.getRay(i, j);
            
            color pixel_color = mundo.ray_color(r); 
            
            write_color(std::cout, pixel_color);
        }
    }

    std::clog << "\rDone.           \n";
    return 0;
}
