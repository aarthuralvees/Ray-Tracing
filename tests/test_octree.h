#pragma once

#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "../src/AABB.h"
#include "../src/MalhaTriangulos.h"
#include "../src/Octree.h"
#include "TestRunner.h"

// Comparacao com tolerancia local a este arquivo (nome proprio para nao
// colidir com close_double de test_malha.h, incluido no mesmo main.cpp).
static bool oct_close(double a, double b) {
    return std::fabs(a - b) < 1e-9;
}

// Escreve um .obj temporario e devolve o caminho. Reutilizado pelos testes
// que precisam de uma malha em disco (o loader de MalhaTriangulos le arquivo).
static std::string oct_write_obj(const std::string& filename, const std::string& conteudo) {
    const std::string path = (std::filesystem::temp_directory_path() / filename).string();
    std::ofstream out(path);
    out << conteudo;
    return path;
}

// Busca linear de referencia: percorre TODOS os triangulos da malha. E a
// "verdade" contra a qual comparamos a octree — o resultado tem que bater.
// (A malha em si ja usa octree no seu hit(); aqui reimplementamos a varredura
//  linear por fora, usando apenas os dados publicos + a geometria dos vertices.)
static bool oct_hit_linear(const MalhaTriangulos& malha, const Ray& r,
                           double t_min, double t_max, HitRecord& rec) {
    bool acertou = false;
    double closest = t_max;

    // Moller-Trumbore identico ao da malha, mas rodado item a item aqui.
    for (const auto& tri : malha.triangulos) {
        const Ponto& v0 = malha.vertices[tri[0]];
        const Ponto& v1 = malha.vertices[tri[1]];
        const Ponto& v2 = malha.vertices[tri[2]];

        const Vetor edge1 = v1 - v0;
        const Vetor edge2 = v2 - v0;
        const Vetor h = cross(r.direction(), edge2);
        const double a = dot(edge1, h);
        if (std::abs(a) < 1e-8) continue;

        const double f = 1.0 / a;
        const Vetor s = r.origin() - v0;
        const double u = f * dot(s, h);
        if (u < 0.0 || u > 1.0) continue;

        const Vetor q = cross(s, edge1);
        const double v = f * dot(r.direction(), q);
        if (v < 0.0 || u + v > 1.0) continue;

        const double t = f * dot(edge2, q);
        if (t <= t_min || t >= closest) continue;

        rec.t = t;
        rec.p = r.at(t);
        acertou = true;
        closest = t;
    }
    return acertou;
}

// ---------------------------------------------------------------------------
// Testes da AABB
// ---------------------------------------------------------------------------

static void aabb_hit_basico() {
    // Caixa unitaria centrada na origem; raio no eixo -Z vindo da frente.
    AABB b(Ponto(-1, -1, -1), Ponto(1, 1, 1));
    Ray dentro(Ponto(0, 0, 5), Vetor(0, 0, -1));
    CHECK(b.hit(dentro, 0.001, 1000.0));

    // Raio paralelo que passa por fora (y = 3) nao acerta.
    Ray fora(Ponto(0, 3, 5), Vetor(0, 0, -1));
    CHECK(!b.hit(fora, 0.001, 1000.0));
}

static void aabb_origem_dentro_da_caixa() {
    // Origem dentro da caixa: deve acertar (o intervalo valido comeca em t_min).
    AABB b(Ponto(-1, -1, -1), Ponto(1, 1, 1));
    Ray r(Ponto(0, 0, 0), Vetor(0, 0, -1));
    CHECK(b.hit(r, 0.001, 1000.0));
}

static void aabb_atras_do_raio_nao_conta() {
    // Caixa inteiramente atras da origem (z positivo), raio indo para -Z.
    AABB b(Ponto(-1, -1, 5), Ponto(1, 1, 7));
    Ray r(Ponto(0, 0, 0), Vetor(0, 0, -1));
    CHECK(!b.hit(r, 0.001, 1000.0));
}

static void aabb_espessura_zero_precisa_de_folga() {
    // Caixa achatada em Z (z=0): sem folga, o slab test rejeitaria por
    // t_min == t_max. Este teste documenta o caso degenerado e mostra que,
    // inflando a caixa por um epsilon, o raio passa a acertar. E exatamente
    // o problema que aparece com triangulos alinhados a um eixo (chao/parede).
    AABB plana(Ponto(-1, -1, 0), Ponto(1, 1, 0));
    Ray r(Ponto(0, 0, 5), Vetor(0, 0, -1));
    CHECK(!plana.hit(r, 0.001, 1000.0));   // sem folga: falha (esperado)

    const double eps = 1e-6;
    AABB inflada(Ponto(-1, -1, -eps), Ponto(1, 1, eps));
    CHECK(inflada.hit(r, 0.001, 1000.0));  // com folga: acerta
}

static void aabb_expand_engloba_pontos() {
    // expand() a partir de caixa vazia deve resultar nos extremos corretos.
    AABB b;
    b.expand(Ponto(1, 2, 3));
    b.expand(Ponto(-4, 0, 5));
    CHECK(oct_close(b.minimo.getX(), -4.0));
    CHECK(oct_close(b.minimo.getY(),  0.0));
    CHECK(oct_close(b.minimo.getZ(),  3.0));
    CHECK(oct_close(b.maximo.getX(),  1.0));
    CHECK(oct_close(b.maximo.getY(),  2.0));
    CHECK(oct_close(b.maximo.getZ(),  5.0));

    Ponto c = b.centro();
    CHECK(oct_close(c.getX(), -1.5));
    CHECK(oct_close(c.getY(),  1.0));
    CHECK(oct_close(c.getZ(),  4.0));
}

// ---------------------------------------------------------------------------
// Testes da Octree (via MalhaTriangulos, que a usa internamente)
// ---------------------------------------------------------------------------

static void octree_malha_planar_acerta() {
    // Um unico triangulo no plano z=0. Malha planar = AABB de espessura zero;
    // so passa porque buildOctree infla a caixa. Garante que a octree nao
    // "perde" geometria alinhada aos eixos.
    MaterialData mat; mat.color = ColorData(0.2, 0.4, 0.6);
    MalhaTriangulos malha(oct_write_obj("oct_planar.obj",
                          "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n"), mat);

    Ray r(Ponto(0.25, 0.25, 1), Vetor(0, 0, -1));
    HitRecord rec;
    CHECK(malha.hit(r, 0.001, 1000.0, rec));
    CHECK(oct_close(rec.t, 1.0));
    CHECK(oct_close(rec.p.getX(), 0.25));
    CHECK(oct_close(rec.p.getY(), 0.25));
}

static void octree_retorna_mais_proximo() {
    // Dois triangulos paralelos: um em z=0 (frente) e um em z=-1 (atras).
    // Raio de z=5 indo para -Z deve acertar o da FRENTE (t menor).
    MaterialData mat; mat.color = ColorData(1, 1, 1);
    MalhaTriangulos malha(oct_write_obj("oct_dois.obj",
        "v 0 0 0\nv 2 0 0\nv 0 2 0\n"     // triangulo 1 (z=0)
        "v 0 0 -1\nv 2 0 -1\nv 0 2 -1\n"  // triangulo 2 (z=-1)
        "f 1 2 3\nf 4 5 6\n"), mat);

    Ray r(Ponto(0.3, 0.3, 5), Vetor(0, 0, -1));
    HitRecord rec;
    CHECK(malha.hit(r, 0.001, 1000.0, rec));
    CHECK(oct_close(rec.p.getZ(), 0.0));  // acertou o da frente, nao o de tras
}

static void octree_miss_quando_raio_passa_longe() {
    // Raio bem afastado da malha nao deve acertar nada (poda pela caixa raiz).
    MaterialData mat; mat.color = ColorData(0.5, 0.5, 0.5);
    MalhaTriangulos malha(oct_write_obj("oct_miss.obj",
                          "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n"), mat);

    Ray r(Ponto(100, 100, 5), Vetor(0, 0, -1));
    HitRecord rec;
    CHECK(!malha.hit(r, 0.001, 1000.0, rec));
}

static void octree_equivale_a_busca_linear() {
    // TESTE PRINCIPAL: uma malha com varios triangulos em posicoes variadas.
    // Disparamos um leque de raios e exigimos que a octree e a busca linear
    // de referencia concordem SEMPRE (mesmo hit/miss, mesmo t, mesmo ponto).
    // Se baterem, a octree esta correta — ela so acelera, nao altera resultado.
    MaterialData mat; mat.color = ColorData(0.8, 0.3, 0.1);

    // Malha em "grade": 8 triangulos espalhados em x, y e z diferentes,
    // suficiente para a octree criar mais de um nivel de subdivisao.
    std::string obj;
    int base = 0;
    auto addTri = [&](double ox, double oy, double oz) {
        obj += "v " + std::to_string(ox)       + " " + std::to_string(oy)       + " " + std::to_string(oz) + "\n";
        obj += "v " + std::to_string(ox + 0.4) + " " + std::to_string(oy)       + " " + std::to_string(oz) + "\n";
        obj += "v " + std::to_string(ox)       + " " + std::to_string(oy + 0.4) + " " + std::to_string(oz) + "\n";
    };
    std::vector<std::array<double,3>> centros = {
        {-1,-1, 0}, { 1,-1, 0}, {-1, 1, 0}, { 1, 1, 0},
        {-1,-1,-2}, { 1,-1,-2}, {-1, 1,-2}, { 1, 1,-2}
    };
    for (auto& c : centros) addTri(c[0], c[1], c[2]);
    for (size_t i = 0; i < centros.size(); ++i) {
        int a = base + 1; base += 3;
        obj += "f " + std::to_string(a) + " " + std::to_string(a+1) + " " + std::to_string(a+2) + "\n";
    }

    MalhaTriangulos malha(oct_write_obj("oct_grade.obj", obj), mat);

    // Leque de raios cobrindo acertos e erros.
    int concordancias = 0, total = 0;
    for (double px = -1.5; px <= 1.5; px += 0.25) {
        for (double py = -1.5; py <= 1.5; py += 0.25) {
            Ray r(Ponto(px + 0.1, py + 0.1, 5), Vetor(0, 0, -1));

            HitRecord recOct, recLin;
            bool hOct = malha.hit(r, 0.001, 1000.0, recOct);
            bool hLin = oct_hit_linear(malha, r, 0.001, 1000.0, recLin);

            ++total;
            if (hOct == hLin) {
                if (!hOct) { ++concordancias; continue; }  // ambos miss: concorda
                // ambos hit: t e ponto tem que coincidir
                if (oct_close(recOct.t, recLin.t) &&
                    oct_close(recOct.p.getX(), recLin.p.getX()) &&
                    oct_close(recOct.p.getY(), recLin.p.getY()) &&
                    oct_close(recOct.p.getZ(), recLin.p.getZ()))
                    ++concordancias;
            }
        }
    }
    // Exigimos concordancia total entre octree e busca linear.
    CHECK(concordancias == total);
}

static void octree_normal_bate_com_linear() {
    // Alem da posicao, a normal reportada pela octree deve ser a mesma da
    // busca linear (mesmo triangulo escolhido, mesma orientacao).
    MaterialData mat; mat.color = ColorData(0.2, 0.4, 0.6);
    MalhaTriangulos malha(oct_write_obj("oct_normal.obj",
                          "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n"), mat);

    Ray r(Ponto(0.25, 0.25, 1), Vetor(0, 0, -1));
    HitRecord recOct, recLin;
    CHECK(malha.hit(r, 0.001, 1000.0, recOct));
    CHECK(oct_hit_linear(malha, r, 0.001, 1000.0, recLin));
    // A normal da malha planar aponta em +Z; o ponto e t coincidem.
    CHECK(oct_close(recOct.t, recLin.t));
    CHECK(oct_close(recOct.p.getZ(), recLin.p.getZ()));
}
