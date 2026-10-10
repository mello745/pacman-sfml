#include "Labirinto.h"

#include "Config.h"

namespace {

const float recuo = 3.0f;     // Distância do contorno até a borda do bloco
const float espessura = 2.0f; // Espessura do contorno

void retangulo(sf::RenderTexture& destino, float x, float y, float largura, float altura) {
    sf::RectangleShape r({ largura, altura });
    r.setPosition(x, y);
    r.setFillColor(sf::Color::White); // Branco: a cor final é aplicada ao desenhar
    destino.draw(r);
}

} // namespace

void Labirinto::montar(const Mapa& mapa) {
    const int linhas = static_cast<int>(mapa.size());
    const int colunas = static_cast<int>(mapa[0].size());
    textura.create(colunas * tamanhoBloco, linhas * tamanhoBloco);
    textura.clear(sf::Color::Transparent);

    // Fora do mapa conta como parede, para não desenhar contorno na borda da janela
    auto parede = [&](int x, int y) {
        return x < 0 || y < 0 || x >= colunas || y >= linhas || mapa[y][x] == Parede;
    };

    const float b = static_cast<float>(tamanhoBloco);
    const float perto = recuo;                  // Começo do contorno (lado de cima/esquerda)
    const float longe = b - recuo - espessura;  // Começo do contorno (lado de baixo/direita)

    for (int y = 0; y < linhas; ++y) {
        for (int x = 0; x < colunas; ++x) {
            if (!parede(x, y)) continue;
            const float px = x * b, py = y * b;

            bool cima = !parede(x, y - 1), baixo = !parede(x, y + 1);
            bool esquerda = !parede(x - 1, y), direita = !parede(x + 1, y);

            // Linhas retas nos lados que dão para um corredor. Onde dois lados têm linha, elas se
            // encontram num canto; onde o vizinho também é parede, a linha vai até a borda do bloco.
            float x0 = esquerda ? perto : 0.0f, x1 = direita ? longe + espessura : b;
            float y0 = cima ? perto : 0.0f, y1 = baixo ? longe + espessura : b;
            if (cima) retangulo(textura, px + x0, py + perto, x1 - x0, espessura);
            if (baixo) retangulo(textura, px + x0, py + longe, x1 - x0, espessura);
            if (esquerda) retangulo(textura, px + perto, py + y0, espessura, y1 - y0);
            if (direita) retangulo(textura, px + longe, py + y0, espessura, y1 - y0);

            // Cantos internos: os dois vizinhos são parede, mas a diagonal é corredor
            if (!cima && !direita && !parede(x + 1, y - 1)) {
                retangulo(textura, px + longe, py, espessura, perto + espessura);
                retangulo(textura, px + longe, py + perto, b - longe, espessura);
            }
            if (!cima && !esquerda && !parede(x - 1, y - 1)) {
                retangulo(textura, px + perto, py, espessura, perto + espessura);
                retangulo(textura, px, py + perto, perto + espessura, espessura);
            }
            if (!baixo && !direita && !parede(x + 1, y + 1)) {
                retangulo(textura, px + longe, py + longe, espessura, b - longe);
                retangulo(textura, px + longe, py + longe, b - longe, espessura);
            }
            if (!baixo && !esquerda && !parede(x - 1, y + 1)) {
                retangulo(textura, px + perto, py + longe, espessura, b - longe);
                retangulo(textura, px, py + longe, perto + espessura, espessura);
            }
        }
    }

    textura.display();
}

void Labirinto::desenhar(sf::RenderTarget& alvo, sf::Color cor) const {
    sf::Sprite sprite(textura.getTexture());
    sprite.setColor(cor);
    alvo.draw(sprite);
}
