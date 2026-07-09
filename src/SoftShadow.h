#ifndef SOFTSHADOWHEADER
#define SOFTSHADOWHEADER

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

// M_PI nao e garantido pelo padrao C++; define se necessario (portabilidade).
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "Ponto.h"
#include "Vetor.h"
#include "../utils/Scene/sceneSchema.hpp"

// ---------------------------------------------------------------------------
// Sombras suaves (soft shadows)
//
// Uma luz pontual gera sombra DURA: o ponto da cena ou "ve" a luz ou nao ve,
// resultado binario, borda nitida. Uma luz de AREA gera penumbra: parte da
// area da luz fica visivel e parte fica escondida atras de um objeto. A
// transicao gradual entre "toda a luz visivel" e "nenhuma visivel" e a
// sombra suave.
//
// Implementacao: em vez de 1 raio de sombra por luz, amostramos VARIOS
// pontos espalhados sobre a area da luz e disparamos um raio de sombra para
// cada um. A contribuicao da luz e multiplicada pela FRACAO de amostras
// desobstruidas. Este arquivo so gera os pontos de amostra; quem testa a
// oclusao e conta a fracao e a Cena.
//
// A "area" da luz aqui e um disco de raio `radius` centrado na posicao da
// luz, orientado de frente para o ponto sombreado. Um disco e uma boa
// aproximacao generica de uma fonte extensa e evita precisar guardar
// orientacao/retangulo na cena.
// ---------------------------------------------------------------------------

// Gerador de numeros pseudoaleatorios simples (xorshift de 32 bits).
// Usamos um proprio, sem <random>, para manter dependencia minima (STL basica)
// e ter controle da semente — util para o jitter das amostras.
class RNGSimples {
public:
    explicit RNGSimples(uint32_t seed = 1u) : estado(seed ? seed : 1u) {}

    // Proximo inteiro pseudoaleatorio.
    uint32_t proximo() {
        estado ^= estado << 13;
        estado ^= estado >> 17;
        estado ^= estado << 5;
        return estado;
    }

    // Double em [0,1).
    double proximoUnitario() {
        return (proximo() & 0xFFFFFFu) / static_cast<double>(0x1000000u);
    }

private:
    uint32_t estado;
};

// Modo de distribuicao das amostras sobre a area da luz.
enum class ModoAmostra {
    GRADE,      // pontos em grade regular (previsivel, mas pode gerar padroes)
    JITTER,     // grade regular com pequena perturbacao aleatoria (padrao — melhor visual)
    ALEATORIO   // pontos totalmente aleatorios (quebra padroes, mais ruidoso)
};

class SoftShadow {
public:
    // Le a configuracao de uma luz a partir do seu extraData (preenchido pelo
    // parser JSON). Campos opcionais:
    //   "radius"  -> raio do disco de luz (0 ou ausente = luz pontual, sombra dura)
    //   "samples" -> numero de amostras (default 16; 1 tambem = sombra dura)
    //   "shadow_mode" -> "grade" | "jitter" | "aleatorio" (default jitter)
    static double lerRaio(const LightData& luz) {
        auto it = luz.extraData.find("radius");
        if (it == luz.extraData.end()) return 0.0;
        return std::stod(it->second);
    }

    static int lerNumAmostras(const LightData& luz) {
        auto it = luz.extraData.find("samples");
        if (it == luz.extraData.end()) return 16;   // default razoavel
        int n = static_cast<int>(std::stod(it->second));
        return n < 1 ? 1 : n;
    }

    static ModoAmostra lerModo(const LightData& luz) {
        auto it = luz.extraData.find("shadow_mode");
        if (it == luz.extraData.end()) return ModoAmostra::JITTER;
        const std::string& m = it->second;
        if (m == "grade")     return ModoAmostra::GRADE;
        if (m == "aleatorio") return ModoAmostra::ALEATORIO;
        return ModoAmostra::JITTER;
    }

    // Gera os pontos de amostra sobre o disco de luz.
    //  - centro:  posicao da luz
    //  - raio:    raio do disco
    //  - normalDisco: direcao para onde o disco "olha" (do ponto sombreado ate a luz)
    //  - n:       numero de amostras pedido
    //  - modo:    grade | jitter | aleatorio
    //  - rng:     gerador (passado por referencia para variar entre pixels)
    // Retorna uma lista de pontos no espaco do mundo. Se raio <= 0, devolve
    // so o centro (luz pontual -> comportamento identico ao antigo).
    static std::vector<Ponto> amostrar(const Ponto& centro, double raio,
                                       const Vetor& normalDisco, int n,
                                       ModoAmostra modo, RNGSimples& rng) {
        std::vector<Ponto> pontos;

        // Luz pontual: uma unica amostra no centro (sombra dura).
        if (raio <= 0.0 || n <= 1) {
            pontos.push_back(centro);
            return pontos;
        }

        // Base ortonormal (u, v) no plano do disco, perpendicular a normalDisco.
        // Escolhemos um vetor auxiliar nao-paralelo a normalDisco e tiramos
        // dois vetores no plano via produtos vetoriais.
        Vetor n_hat = unit_vector(normalDisco);
        Vetor aux = (std::abs(n_hat.getX()) > 0.9) ? Vetor(0, 1, 0) : Vetor(1, 0, 0);
        Vetor u = unit_vector(cross(aux, n_hat));
        Vetor v = cross(n_hat, u);

        if (modo == ModoAmostra::ALEATORIO) {
            // n pontos aleatorios uniformes no disco. Amostragem uniforme:
            // r = R*sqrt(rand) garante densidade uniforme por area (senao
            // os pontos se acumulariam no centro).
            for (int i = 0; i < n; ++i) {
                double ang = 2.0 * M_PI * rng.proximoUnitario();
                double rr  = raio * std::sqrt(rng.proximoUnitario());
                double dx = rr * std::cos(ang);
                double dy = rr * std::sin(ang);
                pontos.push_back(centro + u * dx + v * dy);
            }
            return pontos;
        }

        // GRADE e JITTER: montam uma grade quadrada g x g que cobre o disco,
        // e descartam os pontos que caem fora do circulo. g e escolhido para
        // que a grade tenha ~n celulas.
        int g = static_cast<int>(std::ceil(std::sqrt(static_cast<double>(n))));
        if (g < 1) g = 1;

        for (int j = 0; j < g; ++j) {
            for (int i = 0; i < g; ++i) {
                // Posicao da celula (i,j) em [0,1]x[0,1].
                double cx, cy;
                if (modo == ModoAmostra::JITTER) {
                    // Centro da celula + perturbacao aleatoria dentro dela.
                    cx = (i + rng.proximoUnitario()) / g;
                    cy = (j + rng.proximoUnitario()) / g;
                } else { // GRADE
                    // Centro exato da celula (sem perturbacao).
                    cx = (i + 0.5) / g;
                    cy = (j + 0.5) / g;
                }

                // Mapeia [0,1]^2 para [-1,1]^2 (quadrado que envolve o disco).
                double sx = 2.0 * cx - 1.0;
                double sy = 2.0 * cy - 1.0;

                // Descarta cantos fora do circulo unitario.
                if (sx * sx + sy * sy > 1.0) continue;

                pontos.push_back(centro + u * (sx * raio) + v * (sy * raio));
            }
        }

        // Garante ao menos uma amostra (se a grade for muito pequena).
        if (pontos.empty()) pontos.push_back(centro);
        return pontos;
    }
};

#endif
