#pragma once

#include "Tipos.h"

#include <optional>

enum class OpcaoMenu { Jogar, IA, Ranking, Sair };

// Tela de escolha de dificuldade. Retorna vazio se o jogador fechar a janela ou apertar Esc
std::optional<NivelDificuldade> escolherDificuldade();

// Tela inicial. Só mostra as opções e retorna a escolhida; quem age é o main()
OpcaoMenu menu();
