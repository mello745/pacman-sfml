#include "Recursos.h"

#include "Config.h"

#include <iostream>

// Carrega uma textura de assets/. Se o arquivo não existir, usa um quadrado
// colorido do tamanho de um bloco para o jogo continuar rodando.
void carregarTextura(sf::Texture& textura, const std::string& caminho, sf::Color corReserva) {
    if (!textura.loadFromFile(pastaAssets + caminho)) {
        std::cerr << "Aviso: textura nao encontrada (" << caminho << "), usando substituta." << std::endl;
        sf::Image imagem;
        imagem.create(tamanhoBloco, tamanhoBloco, corReserva);
        textura.loadFromImage(imagem);
    }
}

// Escala o sprite para ocupar exatamente um bloco do mapa, seja qual for o tamanho da imagem
void ajustarAoBloco(sf::Sprite& sprite) {
    sf::Vector2u tamanho = sprite.getTexture()->getSize();
    sprite.setScale(static_cast<float>(tamanhoBloco) / tamanho.x, static_cast<float>(tamanhoBloco) / tamanho.y);
}
