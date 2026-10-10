#include "Sons.h"

#include "Config.h"

#include <iostream>

namespace {
bool semSom = false;
}

Sons::Sons() {
    const std::map<Som, std::string> arquivos = {
        { Som::Inicio, "pacman_beginning.wav" },
        { Som::Comer, "pacman_chomp.wav" },
        { Som::Item, "pacman_eatfruit.wav" },
        { Som::ComerFantasma, "pacman_eatghost.wav" },
        { Som::VidaExtra, "pacman_extrapac.wav" },
        { Som::Morte, "pacman_death.wav" },
    };

    for (const auto& [som, arquivo] : arquivos) {
        if (!buffers[som].loadFromFile(pastaAssets + "audio/" + arquivo)) {
            std::cerr << "Aviso: som nao encontrado (" << arquivo << "), o jogo segue sem ele." << std::endl;
            continue;
        }
        sons[som].setBuffer(buffers[som]);
    }
}

void Sons::tocar(Som som) {
    auto it = sons.find(som);
    if (semSom || it == sons.end()) return;
    it->second.play();
}

void Sons::tocarSeLivre(Som som) {
    auto it = sons.find(som);
    if (semSom || it == sons.end() || it->second.getStatus() == sf::Sound::Playing) return;
    it->second.play();
}

void Sons::pararTodos() {
    for (auto& [som, s] : sons) s.stop();
}

void Sons::alternarMudo() {
    semSom = !semSom;
}

bool Sons::mudo() {
    return semSom;
}
