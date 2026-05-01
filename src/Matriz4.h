#ifndef MATRIZ4HEADER
#define MATRIZ4HEADER

#include <array>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

#include "Ponto.h"
#include "Vetor.h"
#include "../utils/Scene/sceneSchema.hpp"

class Matriz4 {
public:
    std::array<std::array<double, 4>, 4> m;

    Matriz4() {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                m[i][j] = (i == j) ? 1.0 : 0.0;
    }

    static Matriz4 identity() {
        return Matriz4();
    }

    static Matriz4 translation(double dx, double dy, double dz) {
        Matriz4 t;
        t.m[0][3] = dx;
        t.m[1][3] = dy;
        t.m[2][3] = dz;
        return t;
    }

    static Matriz4 scaling(double sx, double sy, double sz) {
        Matriz4 s;
        s.m[0][0] = sx;
        s.m[1][1] = sy;
        s.m[2][2] = sz;
        return s;
    }

    static Matriz4 rotationX(double degrees) {
        const double a = toRadians(degrees);
        const double c = std::cos(a);
        const double s = std::sin(a);

        Matriz4 r;
        r.m[1][1] = c;
        r.m[1][2] = -s;
        r.m[2][1] = s;
        r.m[2][2] = c;
        return r;
    }

    static Matriz4 rotationY(double degrees) {
        const double a = toRadians(degrees);
        const double c = std::cos(a);
        const double s = std::sin(a);

        Matriz4 r;
        r.m[0][0] = c;
        r.m[0][2] = s;
        r.m[2][0] = -s;
        r.m[2][2] = c;
        return r;
    }

    static Matriz4 rotationZ(double degrees) {
        const double a = toRadians(degrees);
        const double c = std::cos(a);
        const double s = std::sin(a);

        Matriz4 r;
        r.m[0][0] = c;
        r.m[0][1] = -s;
        r.m[1][0] = s;
        r.m[1][1] = c;
        return r;
    }

    static Matriz4 rotation(double alpha, double theta, double gamma) {
        return rotationZ(gamma) * rotationY(theta) * rotationX(alpha);
    }

    static Matriz4 fromTransform(const TransformData& transform) {
        const double x = transform.data.getX();
        const double y = transform.data.getY();
        const double z = transform.data.getZ();

        if (transform.tType == "translation") return translation(x, y, z);
        if (transform.tType == "scaling") return scaling(x, y, z);
        if (transform.tType == "rotation") return rotation(x, y, z);

        throw std::runtime_error("Unsupported transform type: " + transform.tType);
    }

    static Matriz4 fromTransforms(const std::vector<TransformData>& transforms) {
        Matriz4 total = identity();
        for (const auto& transform : transforms) {
            total = fromTransform(transform) * total;
        }
        return total;
    }

    Matriz4 operator*(const Matriz4& other) const {
        Matriz4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.m[i][j] = 0.0;
                for (int k = 0; k < 4; ++k)
                    result.m[i][j] += m[i][k] * other.m[k][j];
            }
        }
        return result;
    }

    Ponto applyToPoint(const Ponto& p) const {
        const double x = p.getX();
        const double y = p.getY();
        const double z = p.getZ();

        const double xp = m[0][0] * x + m[0][1] * y + m[0][2] * z + m[0][3];
        const double yp = m[1][0] * x + m[1][1] * y + m[1][2] * z + m[1][3];
        const double zp = m[2][0] * x + m[2][1] * y + m[2][2] * z + m[2][3];
        const double wp = m[3][0] * x + m[3][1] * y + m[3][2] * z + m[3][3];

        if (std::abs(wp) < 1e-12) return Ponto(xp, yp, zp);
        return Ponto(xp / wp, yp / wp, zp / wp);
    }

    Vetor applyToVector(const Vetor& v) const {
        const double x = v.getX();
        const double y = v.getY();
        const double z = v.getZ();

        return Vetor(
            m[0][0] * x + m[0][1] * y + m[0][2] * z,
            m[1][0] * x + m[1][1] * y + m[1][2] * z,
            m[2][0] * x + m[2][1] * y + m[2][2] * z
        );
    }

private:
    static double toRadians(double degrees) {
        return degrees * std::acos(-1.0) / 180.0;
    }
};

#endif
