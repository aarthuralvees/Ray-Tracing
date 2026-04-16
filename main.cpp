#include <iostream>
#include "src/Ponto.h"
#include "src/Vetor.h"
using namespace std;

#ifdef RUN_TESTS
#include "tests/test_vetor.h"
#include "tests/test_ponto.h"

static void run_all_tests() {
    run_test("vetor_arithmetic",  vetor_arithmetic);
    run_test("vetor_length",      vetor_length);
    run_test("vetor_dot_cross",   vetor_dot_cross);
    run_test("vetor_unit_vector", vetor_unit_vector);
    run_test("ponto_arithmetic",  ponto_arithmetic);
    run_test("ray_at",            ray_at);
    report_tests();
}
#endif

int main() {
#ifdef RUN_TESTS
    run_all_tests();
    return tests::_failed > 0 ? 1 : 0;
#endif

    // renderiza uma imagem com um gradiente
    // g++ main.cpp -o a && ./a > image.ppm
    int image_width = 256;
    int image_height = 256;

    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    for (int j = 0; j < image_height; j++) {
        std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
        for (int i = 0; i < image_width; i++) {
            auto pixel_color = color(double(i) / (image_width - 1), double(j) / (image_height - 1), 0);
            write_color(std::cout, pixel_color);
        }
    }

    std::clog << "\rDone.           \n";
}
