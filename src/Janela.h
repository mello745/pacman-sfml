#pragma once

#include <SFML/Graphics.hpp>
#include <string>

// Janelas que se ajustam à tela do jogador. Cada tela desenha num tamanho "virtual" fixo
// (o jogo em 342x500, os menus em 850x600) e o SFML amplia para o tamanho real da janela,
// mantendo a proporção (com faixas pretas se sobrar espaço).
//
// - Ao abrir, a janela fica com o maior tamanho que cabe na tela (em 1920x1080, o jogo fica 2x).
// - A janela pode ser redimensionada ou maximizada.
// - F11 alterna entre janela e tela cheia; a escolha vale para todas as telas até fechar o jogo.

// Cria (ou recria) a janela já ampliada e centralizada na tela
void abrirJanela(sf::RenderWindow& janela, sf::Vector2u tamanhoVirtual, const std::string& titulo);

// Trata os eventos de janela (redimensionar e F11). Chamar para cada evento do pollEvent;
// retorna true se o evento foi usado aqui.
bool tratarEventoDeJanela(sf::RenderWindow& janela, const sf::Event& evento,
    sf::Vector2u tamanhoVirtual, const std::string& titulo);
