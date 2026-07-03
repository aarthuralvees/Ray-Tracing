#ifndef SUPERFICIEBEZIERHEADER
#define SUPERFICIEBEZIERHEADER

#include <array>
#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Matriz4.h"
#include "Objeto.h"

class SuperficieBezier : public Objeto {
public:
    using LinhaControle = std::array<Ponto, 4>;
    using PontosControle = std::array<LinhaControle, 4>;
    using Triangulo = std::array<int, 3>;

    PontosControle pontos_controle;
    std::vector<Ponto> vertices;
    std::vector<Triangulo> triangulos;
    std::vector<Vetor> normais_vertices;
    MaterialData material;
    int resolucao;

    SuperficieBezier(const std::string& path, MaterialData mat, int resolution = 16)
        : SuperficieBezier(loadControlPoints(path), mat, resolution, Matriz4::identity()) {}

    SuperficieBezier(const std::string& path, MaterialData mat, int resolution, const Matriz4& transform)
        : SuperficieBezier(loadControlPoints(path), mat, resolution, transform) {}

    SuperficieBezier(const PontosControle& controlPoints, MaterialData mat, int resolution = 16)
        : SuperficieBezier(controlPoints, mat, resolution, Matriz4::identity()) {}

    SuperficieBezier(const PontosControle& controlPoints, MaterialData mat, int resolution, const Matriz4& transform)
        : pontos_controle(controlPoints), material(mat), resolucao(std::max(1, resolution)) {
        applyTransform(transform);
        tessellate();
    }

    bool hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const override {
        bool hit_anything = false;
        double closest = t_max;
        HitRecord temp;

        for (size_t i = 0; i < triangulos.size(); ++i) {
            if (hitTriangle(i, r, t_min, closest, temp)) {
                hit_anything = true;
                closest = temp.t;
                rec = temp;
            }
        }

        return hit_anything;
    }

private:
    static PontosControle loadControlPoints(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) throw std::runtime_error("Erro ao abrir superficie Bezier: " + path);

        std::vector<Ponto> points;
        std::string line;
        while (std::getline(file, line)) {
            const size_t comment = line.find('#');
            if (comment != std::string::npos) line = line.substr(0, comment);

            std::istringstream iss(line);
            double x, y, z;
            if (iss >> x >> y >> z) points.emplace_back(x, y, z);
        }

        if (points.size() != 16)
            throw std::runtime_error("Superficie Bezier precisa de exatamente 16 pontos de controle: " + path);

        PontosControle control;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                control[i][j] = points[i * 4 + j];

        return control;
    }

    static std::array<double, 4> bernstein(double t) {
        const double omt = 1.0 - t;
        return {
            omt * omt * omt,
            3.0 * t * omt * omt,
            3.0 * t * t * omt,
            t * t * t
        };
    }

    static std::array<double, 4> bernsteinDerivative(double t) {
        const double omt = 1.0 - t;
        return {
            -3.0 * omt * omt,
            3.0 * omt * omt - 6.0 * t * omt,
            6.0 * t * omt - 3.0 * t * t,
            3.0 * t * t
        };
    }

    Ponto evaluate(double u, double v) const {
        const auto bu = bernstein(u);
        const auto bv = bernstein(v);

        double x = 0.0, y = 0.0, z = 0.0;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                const double w = bu[i] * bv[j];
                x += w * pontos_controle[i][j].getX();
                y += w * pontos_controle[i][j].getY();
                z += w * pontos_controle[i][j].getZ();
            }
        }

        return Ponto(x, y, z);
    }

    Vetor derivativeU(double u, double v) const {
        const auto dbu = bernsteinDerivative(u);
        const auto bv = bernstein(v);
        return weightedControlSum(dbu, bv);
    }

    Vetor derivativeV(double u, double v) const {
        const auto bu = bernstein(u);
        const auto dbv = bernsteinDerivative(v);
        return weightedControlSum(bu, dbv);
    }

    Vetor weightedControlSum(const std::array<double, 4>& wu, const std::array<double, 4>& wv) const {
        double x = 0.0, y = 0.0, z = 0.0;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                const double w = wu[i] * wv[j];
                x += w * pontos_controle[i][j].getX();
                y += w * pontos_controle[i][j].getY();
                z += w * pontos_controle[i][j].getZ();
            }
        }
        return Vetor(x, y, z);
    }

    void applyTransform(const Matriz4& transform) {
        for (auto& row : pontos_controle)
            for (auto& point : row)
                point = transform.applyToPoint(point);
    }

    void tessellate() {
        vertices.clear();
        triangulos.clear();
        normais_vertices.clear();

        const int n = resolucao;
        vertices.reserve((n + 1) * (n + 1));
        normais_vertices.reserve((n + 1) * (n + 1));

        for (int i = 0; i <= n; ++i) {
            const double u = static_cast<double>(i) / n;
            for (int j = 0; j <= n; ++j) {
                const double v = static_cast<double>(j) / n;
                vertices.push_back(evaluate(u, v));

                Vetor normal = cross(derivativeU(u, v), derivativeV(u, v));
                if (normal.length_squared() > 0.0) normal = unit_vector(normal);
                normais_vertices.push_back(normal);
            }
        }

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                const int a = index(i, j);
                const int b = index(i + 1, j);
                const int c = index(i, j + 1);
                const int d = index(i + 1, j + 1);
                triangulos.push_back({a, b, c});
                triangulos.push_back({b, d, c});
            }
        }
    }

    int index(int i, int j) const {
        return i * (resolucao + 1) + j;
    }

    bool hitTriangle(size_t triIndex, const Ray& r, double t_min, double t_max, HitRecord& rec) const {
        const Triangulo& tri = triangulos[triIndex];
        const Ponto& v0 = vertices[tri[0]];
        const Ponto& v1 = vertices[tri[1]];
        const Ponto& v2 = vertices[tri[2]];

        const Vetor edge1 = v1 - v0;
        const Vetor edge2 = v2 - v0;
        const Vetor h = cross(r.direction(), edge2);
        const double a = dot(edge1, h);

        if (std::abs(a) < 1e-8) return false;

        const double f = 1.0 / a;
        const Vetor s = r.origin() - v0;
        const double u = f * dot(s, h);
        if (u < 0.0 || u > 1.0) return false;

        const Vetor q = cross(s, edge1);
        const double v = f * dot(r.direction(), q);
        if (v < 0.0 || u + v > 1.0) return false;

        const double t = f * dot(edge2, q);
        if (t <= t_min || t >= t_max) return false;

        rec.t = t;
        rec.p = r.at(t);

        const double w = 1.0 - u - v;
        Vetor normal = normais_vertices[tri[0]] * w + normais_vertices[tri[1]] * u + normais_vertices[tri[2]] * v;
        if (normal.length_squared() == 0.0) normal = cross(edge1, edge2);
        if (normal.length_squared() > 0.0) normal = unit_vector(normal);

        rec.set_face_normal(r, normal);
        rec.material = material;
        return true;
    }
};

#endif
