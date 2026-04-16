#pragma once
#include <cmath>
#include "../src/Vetor.h"
#include "TestRunner.h"

static void vetor_arithmetic() {
    Vetor a(1, 2, 3), b(4, 5, 6);

    Vetor sum = a + b;
    CHECK(sum.getX() == 5);
    CHECK(sum.getY() == 7);
    CHECK(sum.getZ() == 9);

    Vetor diff = b - a;
    CHECK(diff.getX() == 3);
    CHECK(diff.getY() == 3);
    CHECK(diff.getZ() == 3);

    Vetor scaled = a * 2;
    CHECK(scaled.getX() == 2);
    CHECK(scaled.getY() == 4);
    CHECK(scaled.getZ() == 6);

    Vetor halved = b / 2;
    CHECK(halved.getX() == 2);
    CHECK(halved.getY() == 2.5);
    CHECK(halved.getZ() == 3);

    Vetor neg = -a;
    CHECK(neg.getX() == -1);
    CHECK(neg.getY() == -2);
    CHECK(neg.getZ() == -3);
}

static void vetor_length() {
    Vetor v(3, 4, 0);
    CHECK(fabs(v.length() - 5.0) < 1e-9);
    CHECK(fabs(v.length_squared() - 25.0) < 1e-9);

    Vetor zero(0, 0, 0);
    CHECK(zero.length() == 0.0);
}

static void vetor_dot_cross() {
    Vetor a(1, 2, 3), b(4, 5, 6);
    CHECK(dot(a, b) == 32.0);   // 1*4 + 2*5 + 3*6

    Vetor x(1, 0, 0), y(0, 1, 0);
    CHECK(dot(x, y) == 0.0);    // perpendicular vectors

    Vetor z = cross(x, y);      // right-hand rule: x × y = +z
    CHECK(z.getX() == 0);
    CHECK(z.getY() == 0);
    CHECK(z.getZ() == 1);
}

static void vetor_unit_vector() {
    Vetor v(3, 4, 0);
    Vetor n = unit_vector(v);
    CHECK(fabs(n.length() - 1.0) < 1e-9);
    CHECK(fabs(n.getX() - 0.6) < 1e-9);
    CHECK(fabs(n.getY() - 0.8) < 1e-9);
    CHECK(fabs(n.getZ() - 0.0) < 1e-9);
}
