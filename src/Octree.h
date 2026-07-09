#ifndef OCTREEHEADER
#define OCTREEHEADER

#include <cstddef>
#include <functional>
#include <memory>
#include <vector>

#include "AABB.h"
#include "Objeto.h"   // HitRecord
#include "Ray.h"

// Octree espacial generica para acelerar interseccao raio-cena.
//
// Ideia: em vez de testar o raio contra TODOS os itens (busca linear),
// dividimos o espaco recursivamente em 8 caixas (octantes). Cada raio
// so desce nos ramos cuja caixa ele realmente atravessa, pulando
// grandes blocos de itens de uma vez.
//
// A octree nao conhece o tipo dos itens: ela guarda apenas indices
// (inteiros). Quem constroi passa a AABB de cada item; quem consulta
// passa um callback que sabe testar o raio contra o item de indice i.
// Assim a mesma estrutura serve para triangulos de uma malha (uso atual)
// ou, no futuro, para outros conjuntos de objetos.
class Octree {
public:
    // Callback de teste: recebe indice do item, raio e intervalo [t_min, t_max),
    // preenche rec e devolve true se houve acerto mais proximo que t_max.
    using HitFn = std::function<bool(size_t idx, const Ray& r,
                                     double t_min, double t_max, HitRecord& rec)>;

    // --- Criterios de parada da subdivisao ---
    // Profundidade maxima da arvore: trava o crescimento em casos ruins
    // (muitos itens quase no mesmo ponto). Com 8 filhos por nivel, 10
    // niveis ja dao resolucao espacial de sobra.
    static constexpr int MAX_DEPTH = 10;
    // Minimo de itens para valer a pena subdividir um no. Abaixo disso,
    // testar linearmente e mais barato que criar 8 filhos.
    static constexpr size_t MIN_ITEMS = 8;

    Octree() = default;

    // Constroi a octree. `limites` = AABB de cada item (indexada por item);
    // o numero de itens e limites.size(). `raiz` = caixa que engloba tudo.
    void build(const std::vector<AABB>& limites, const AABB& raizBox) {
        limitesItens = &limites;

        std::vector<size_t> todos;
        todos.reserve(limites.size());
        for (size_t i = 0; i < limites.size(); ++i) todos.push_back(i);

        raiz = std::make_unique<No>();
        raiz->caixa = raizBox;
        subdividir(*raiz, todos, 0);
    }

    // Consulta: percorre a arvore seguindo o raio e chama hitFn nos itens
    // dos nos visitados, mantendo o acerto mais proximo. `carimbo` e um
    // vetor de flags (um por item) usado para nao testar o mesmo item duas
    // vezes quando ele aparece em varios octantes; o chamador o fornece e
    // incrementa `marca` a cada nova consulta (truque para "limpar" o vetor
    // sem percorre-lo inteiro).
    bool hit(const Ray& r, double t_min, double t_max, HitRecord& rec,
             const HitFn& hitFn,
             std::vector<unsigned>& carimbo, unsigned marca) const {
        if (!raiz) return false;
        bool acertou = false;
        double closest = t_max;
        percorrer(*raiz, r, t_min, closest, rec, hitFn, acertou, carimbo, marca);
        return acertou;
    }

private:
    // Um no da octree: sua caixa, os indices que guarda (apenas em folhas)
    // e ate 8 filhos (apenas em nos internos).
    struct No {
        AABB caixa;
        std::vector<size_t> itens;              // preenchido so em folhas
        std::unique_ptr<No> filhos[8];          // nulos em folhas
        bool folha = true;
    };

    std::unique_ptr<No> raiz;
    const std::vector<AABB>* limitesItens = nullptr;  // nao possui; aponta para os limites do chamador

    // Decide o octante (0..7) de um ponto em relacao ao centro c:
    // bit 0 = X, bit 1 = Y, bit 2 = Z (1 = lado "maior" do eixo).
    static int octanteDe(const Ponto& p, const Ponto& c) {
        int oct = 0;
        if (p.getX() >= c.getX()) oct |= 1;
        if (p.getY() >= c.getY()) oct |= 2;
        if (p.getZ() >= c.getZ()) oct |= 4;
        return oct;
    }

    // Constroi a AABB de um dos 8 octantes filhos, a partir da caixa do pai
    // e do seu centro. `oct` seleciona metade baixa/alta em cada eixo.
    static AABB caixaFilho(const AABB& pai, const Ponto& c, int oct) {
        Ponto mn = pai.minimo, mx = pai.maximo;
        double x0 = (oct & 1) ? c.getX() : mn.getX();
        double x1 = (oct & 1) ? mx.getX() : c.getX();
        double y0 = (oct & 2) ? c.getY() : mn.getY();
        double y1 = (oct & 2) ? mx.getY() : c.getY();
        double z0 = (oct & 4) ? c.getZ() : mn.getZ();
        double z1 = (oct & 4) ? mx.getZ() : c.getZ();
        return AABB(Ponto(x0, y0, z0), Ponto(x1, y1, z1));
    }

    // Subdivide recursivamente um no. Se atingiu criterio de parada
    // (poucos itens ou profundidade maxima), o no vira folha e guarda os
    // itens. Senao, distribui cada item nos octantes que sua caixa toca
    // (um item pode cair em mais de um octante se cruza a fronteira) e
    // recorre em cada filho nao-vazio.
    void subdividir(No& no, const std::vector<size_t>& itens, int profundidade) {
        if (itens.size() <= MIN_ITEMS || profundidade >= MAX_DEPTH) {
            no.folha = true;
            no.itens = itens;   // folha: teste linear entre estes poucos itens
            return;
        }

        no.folha = false;
        const Ponto c = no.caixa.centro();

        // Baldes de itens por octante.
        std::vector<size_t> baldes[8];

        for (size_t idx : itens) {
            const AABB& b = (*limitesItens)[idx];
            // Octantes tocados pelos cantos min e max da caixa do item.
            // Se o item cabe num unico octante, os dois cantos dao o mesmo;
            // se cruza fronteiras, cai em varios (insercao redundante segura).
            int oMin = octanteDe(b.minimo, c);
            int oMax = octanteDe(b.maximo, c);
            // Cobre todos os octantes no "intervalo" entre oMin e oMax por eixo.
            for (int ox = 0; ox <= 1; ++ox)
                for (int oy = 0; oy <= 1; ++oy)
                    for (int oz = 0; oz <= 1; ++oz) {
                        int oct = ox | (oy << 1) | (oz << 2);
                        bool okX = ((oMin & 1) <= ox && ox <= (oMax & 1)) || ((oMax & 1) <= ox && ox <= (oMin & 1));
                        bool okY = (((oMin >> 1) & 1) <= oy && oy <= ((oMax >> 1) & 1)) || (((oMax >> 1) & 1) <= oy && oy <= ((oMin >> 1) & 1));
                        bool okZ = (((oMin >> 2) & 1) <= oz && oz <= ((oMax >> 2) & 1)) || (((oMax >> 2) & 1) <= oz && oz <= ((oMin >> 2) & 1));
                        if (okX && okY && okZ) baldes[oct].push_back(idx);
                    }
        }

        // Salvaguarda: se um balde ficou com quase tudo (item gigante ou
        // itens colineares), subdividir de novo nao ajuda e pode nao
        // terminar. Vira folha para evitar recursao inutil.
        for (int i = 0; i < 8; ++i) {
            if (baldes[i].size() == itens.size()) {
                no.folha = true;
                no.itens = itens;
                for (int k = 0; k < 8; ++k) no.filhos[k].reset();
                return;
            }
        }

        for (int i = 0; i < 8; ++i) {
            if (baldes[i].empty()) continue;    // nao cria filho vazio
            no.filhos[i] = std::make_unique<No>();
            no.filhos[i]->caixa = caixaFilho(no.caixa, c, i);
            subdividir(*no.filhos[i], baldes[i], profundidade + 1);
        }
    }

    // Travessia guiada pelo raio. Poda nós cuja caixa o raio nao cruza
    // dentro de [t_min, closest). Em folhas, testa cada item uma unica vez
    // (controle por carimbo/marca) e encolhe `closest` a cada acerto.
    void percorrer(const No& no, const Ray& r, double t_min, double& closest,
                   HitRecord& rec, const HitFn& hitFn, bool& acertou,
                   std::vector<unsigned>& carimbo, unsigned marca) const {
        // Poda: se o raio nem toca a caixa deste no ate o acerto mais
        // proximo ja encontrado, ignora o ramo inteiro.
        if (!no.caixa.hit(r, t_min, closest)) return;

        if (no.folha) {
            for (size_t idx : no.itens) {
                if (carimbo[idx] == marca) continue;  // ja testado nesta consulta
                carimbo[idx] = marca;

                HitRecord temp;
                if (hitFn(idx, r, t_min, closest, temp)) {
                    acertou = true;
                    closest = temp.t;   // proximos testes so aceitam algo mais perto
                    rec = temp;
                }
            }
            return;
        }

        // No interno: desce nos 8 filhos existentes. (Visitar em ordem
        // arbitraria e correto; a poda por `closest` ja evita trabalho
        // superfluo. Ordenar por proximidade seria uma otimizacao extra.)
        for (int i = 0; i < 8; ++i) {
            if (no.filhos[i])
                percorrer(*no.filhos[i], r, t_min, closest, rec, hitFn, acertou, carimbo, marca);
        }
    }
};

#endif
