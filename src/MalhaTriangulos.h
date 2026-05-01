#ifndef MALHATRIANGULOSHEADER
#define MALHATRIANGULOSHEADER

#include <array>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Matriz4.h"
#include "Objeto.h"

class MalhaTriangulos : public Objeto {
public:
    using Triangulo = std::array<int, 3>;

    std::vector<Ponto> vertices;
    std::vector<Triangulo> triangulos;
    std::vector<Vetor> normais_triangulos;
    std::vector<Vetor> normais_vertices;
    color cor;

    MalhaTriangulos(const std::string& objPath, color cor)
        : MalhaTriangulos(objPath, cor, Matriz4::identity()) {}

    MalhaTriangulos(const std::string& objPath, color cor, const Matriz4& transform)
        : cor(cor) {
        loadObj(objPath);
        applyTransform(transform);
        computeNormals();
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
    static int parseVertexIndex(const std::string& token, int vertexCount) {
        const size_t slash = token.find('/');
        const std::string raw = slash == std::string::npos ? token : token.substr(0, slash);
        if (raw.empty()) throw std::runtime_error("Invalid OBJ face vertex token: " + token);

        int index = std::stoi(raw);
        if (index < 0) index = vertexCount + index;
        else index -= 1;

        if (index < 0 || index >= vertexCount)
            throw std::runtime_error("OBJ face references vertex out of range: " + token);

        return index;
    }

    void loadObj(const std::string& objPath) {
        std::ifstream file(objPath);
        if (!file.is_open()) throw std::runtime_error("Erro ao abrir o arquivo OBJ: " + objPath);

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;

            if (prefix == "v") {
                double x, y, z;
                if (!(iss >> x >> y >> z)) throw std::runtime_error("Invalid OBJ vertex line");
                vertices.emplace_back(x, y, z);
            } else if (prefix == "f") {
                std::vector<int> face;
                std::string token;
                while (iss >> token) face.push_back(parseVertexIndex(token, static_cast<int>(vertices.size())));

                if (face.size() < 3) throw std::runtime_error("OBJ face must have at least 3 vertices");
                for (size_t i = 1; i + 1 < face.size(); ++i)
                    triangulos.push_back({face[0], face[i], face[i + 1]});
            }
        }

        if (vertices.empty()) throw std::runtime_error("OBJ mesh has no vertices: " + objPath);
        if (triangulos.empty()) throw std::runtime_error("OBJ mesh has no faces: " + objPath);
    }

    void applyTransform(const Matriz4& transform) {
        for (auto& vertex : vertices) vertex = transform.applyToPoint(vertex);
    }

    void computeNormals() {
        normais_triangulos.clear();
        normais_vertices.assign(vertices.size(), Vetor(0, 0, 0));

        for (const auto& tri : triangulos) {
            const Vetor e1 = vertices[tri[1]] - vertices[tri[0]];
            const Vetor e2 = vertices[tri[2]] - vertices[tri[0]];
            Vetor normal = cross(e1, e2);

            if (normal.length_squared() > 0.0) normal = unit_vector(normal);

            normais_triangulos.push_back(normal);
            for (int idx : tri) normais_vertices[idx] = normais_vertices[idx] + normal;
        }

        for (auto& normal : normais_vertices) {
            if (normal.length_squared() > 0.0) normal = unit_vector(normal);
        }
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
        if (normal.length_squared() == 0.0) normal = normais_triangulos[triIndex];
        else normal = unit_vector(normal);

        if (dot(r.direction(), normal) > 0.0) normal = -normal;

        rec.normal = normal;
        rec.cor_difusa = cor;
        return true;
    }
};

#endif
