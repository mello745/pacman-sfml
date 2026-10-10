#pragma once

#include <ctime>
#include <string>
#include <vector>

struct jogador {
    std::string nome;
    int pontos;
    time_t tempo;
};

// Acrescenta UMA partida ao final do arquivo (cada linha: nome pontos tempo)
void salvarNoRanking(const jogador& partida);

// Lê todas as partidas salvas no arquivo
std::vector<jogador> carregarRanking();

// Tela com as partidas ordenadas por pontuação (setas rolam, Esc volta)
void exibirRanking();
