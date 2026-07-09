#ifndef AABBHEADER
#define AABBHEADER

#include <algorithm>
#include <cmath>
#include <limits>

#include "Ponto.h"
#include "Ray.h"
#include "Vetor.h"

// Axis-Aligned Bounding Box: uma caixa alinhada aos eixos X, Y, Z,
// descrita apenas pelos cantos minimo e maximo. Serve para envolver
// grupos de triangulos e descartar rapidamente raios que nem chegam perto.
class AABB {
public:
    Ponto minimo;
    Ponto maximo;

    // Caixa "vazia": min em +infinito, max em -infinito. Qualquer ponto
    // adicionado via expand() a torna valida. Comecar assim faz o primeiro
    // expand() simplesmente adotar o ponto.
    AABB()
        : minimo( std::numeric_limits<double>::infinity(),
                  std::numeric_limits<double>::infinity(),
                  std::numeric_limits<double>::infinity()),
          maximo(-std::numeric_limits<double>::infinity(),
                 -std::numeric_limits<double>::infinity(),
                 -std::numeric_limits<double>::infinity()) {}

    AABB(const Ponto& mn, const Ponto& mx) : minimo(mn), maximo(mx) {}

    // Cresce a caixa para conter o ponto p (atualiza cada eixo).
    void expand(const Ponto& p) {
        minimo = Ponto(std::min(minimo.getX(), p.getX()),
                       std::min(minimo.getY(), p.getY()),
                       std::min(minimo.getZ(), p.getZ()));
        maximo = Ponto(std::max(maximo.getX(), p.getX()),
                       std::max(maximo.getY(), p.getY()),
                       std::max(maximo.getZ(), p.getZ()));
    }

    // Cresce a caixa para conter outra caixa inteira.
    void expand(const AABB& outra) {
        expand(outra.minimo);
        expand(outra.maximo);
    }

    // Centro geometrico da caixa: usado para decidir em qual octante
    // (filho) um triangulo cai durante a subdivisao.
    Ponto centro() const {
        return Ponto((minimo.getX() + maximo.getX()) * 0.5,
                     (minimo.getY() + maximo.getY()) * 0.5,
                     (minimo.getZ() + maximo.getZ()) * 0.5);
    }

    // Teste raio-caixa pelo "slab method": para cada eixo, calcula o
    // intervalo de t em que o raio esta dentro das duas placas (slabs)
    // daquele eixo, e intersecta os tres intervalos. Se sobra intervalo
    // valido, o raio cruza a caixa. Nao calcula ponto de contato, so diz
    // se vale a pena descer neste no. Retorna true tambem quando a origem
    // do raio ja esta dentro da caixa.
    bool hit(const Ray& r, double t_min, double t_max) const {
        const double ox = r.origin().getX(), oy = r.origin().getY(), oz = r.origin().getZ();
        const double dx = r.direction().getX(), dy = r.direction().getY(), dz = r.direction().getZ();

        // Eixo X
        double invD = 1.0 / dx;
        double t0 = (minimo.getX() - ox) * invD;
        double t1 = (maximo.getX() - ox) * invD;
        if (invD < 0.0) std::swap(t0, t1);            // direcao negativa inverte as placas
        t_min = t0 > t_min ? t0 : t_min;
        t_max = t1 < t_max ? t1 : t_max;
        if (t_max <= t_min) return false;

        // Eixo Y
        invD = 1.0 / dy;
        t0 = (minimo.getY() - oy) * invD;
        t1 = (maximo.getY() - oy) * invD;
        if (invD < 0.0) std::swap(t0, t1);
        t_min = t0 > t_min ? t0 : t_min;
        t_max = t1 < t_max ? t1 : t_max;
        if (t_max <= t_min) return false;

        // Eixo Z
        invD = 1.0 / dz;
        t0 = (minimo.getZ() - oz) * invD;
        t1 = (maximo.getZ() - oz) * invD;
        if (invD < 0.0) std::swap(t0, t1);
        t_min = t0 > t_min ? t0 : t_min;
        t_max = t1 < t_max ? t1 : t_max;
        if (t_max <= t_min) return false;

        return true;
    }
};

#endif
