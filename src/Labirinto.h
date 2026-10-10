#pragma once

#include "Tipos.h"

#include <SFML/Graphics.hpp>

// Desenho das paredes no estilo arcade: fundo preto com um contorno fino nas bordas que dão para
// os corredores. A imagem é montada uma vez por fase, e não a cada frame.
class Labirinto {
public:
    // Gera a imagem das paredes do mapa (chamar ao iniciar cada fase)
    void montar(const Mapa& mapa);

    // Desenha com a cor indicada (azul normalmente; branco quando a fase termina)
    void desenhar(sf::RenderTarget& alvo, sf::Color cor) const;

private:
    sf::RenderTexture textura;
};
