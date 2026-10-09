#include "Jogo.h"
#include "Ranking.h"
#include "Telas.h"

#include <optional>

// Função principal: volta ao menu depois de cada partida ou tela, até o jogador escolher Sair
int main()
{
    while (true) {
        switch (menu()) {
        case OpcaoMenu::Jogar: {
            std::optional<NivelDificuldade> dificuldade = escolherDificuldade();
            if (dificuldade) {
                Jogo jogo;
                jogo.dificuldade(*dificuldade);
                jogo.executar();
            }
            break;
        }
        case OpcaoMenu::IA: {
            Jogo jogo;
            jogo.modoAtual = IA; // Pac-Man controlado pelo computador
            jogo.dificuldade(Facil);
            jogo.executar();
            break;
        }
        case OpcaoMenu::Ranking:
            exibirRanking();
            break;
        case OpcaoMenu::Sair:
            return 0;
        }
    }
}
