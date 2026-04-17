#pragma once
#include "../src/Ponto.h"
#include "../src/Ray.h"
#include "TestRunner.h"

static void ponto_arithmetic() {
    Ponto p(1, 2, 3);
    Vetor v(4, 5, 6);

    Ponto q = p + v;            // Ponto + Vetor -> Ponto
    CHECK(q.getX() == 5);
    CHECK(q.getY() == 7);
    CHECK(q.getZ() == 9);

    Vetor diff = q - p;         // Ponto - Ponto -> Vetor
    CHECK(diff.getX() == 4);
    CHECK(diff.getY() == 5);
    CHECK(diff.getZ() == 6);
}

static void ray_at() {
    ray r(Ponto(0, 0, 0), Vetor(1, 0, 0));

    Ponto at0 = r.at(0.0);      // origin at t=0
    CHECK(at0.getX() == 0);
    CHECK(at0.getY() == 0);
    CHECK(at0.getZ() == 0);

    Ponto at2 = r.at(2.0);      // two units along x
    CHECK(at2.getX() == 2);
    CHECK(at2.getY() == 0);
    CHECK(at2.getZ() == 0);

    Ponto atNeg = r.at(-1.0);   // behind origin
    CHECK(atNeg.getX() == -1);
    CHECK(atNeg.getY() == 0);
    CHECK(atNeg.getZ() == 0);
}
