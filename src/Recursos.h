#pragma once

#include <SFML/Graphics.hpp>
#include <string>

// Carrega uma textura de assets/. Se o arquivo não existir, usa um quadrado
// colorido do tamanho de um bloco para o jogo continuar rodando.
void carregarTextura(sf::Texture& textura, const std::string& caminho, sf::Color corReserva = sf::Color::Magenta);

// Escala o sprite para ocupar exatamente um bloco do mapa, seja qual for o tamanho da imagem
void ajustarAoBloco(sf::Sprite& sprite);
