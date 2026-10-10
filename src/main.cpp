#include "Jogo.h"
#include "Ranking.h"
#include "Telas.h"

#include <filesystem>
#include <optional>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

// Os caminhos do jogo (assets/ e ranking.txt) são relativos à pasta atual. Para funcionar de
// qualquer lugar (botão do VS Code, terminal em outra pasta, atalho), a pasta atual passa a ser
// a do próprio executável, onde o build coloca a cópia de assets/.
void usarPastaDoExecutavel() {
    std::error_code erro;
#ifdef _WIN32
    wchar_t caminho[MAX_PATH];
    if (GetModuleFileNameW(nullptr, caminho, MAX_PATH) > 0) {
        std::filesystem::current_path(std::filesystem::path(caminho).parent_path(), erro);
    }
#else
    std::filesystem::path caminho = std::filesystem::read_symlink("/proc/self/exe", erro);
    if (!erro) std::filesystem::current_path(caminho.parent_path(), erro);
#endif
}

// Função principal: volta ao menu depois de cada partida ou tela, até o jogador escolher Sair
int main()
{
    usarPastaDoExecutavel();

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
