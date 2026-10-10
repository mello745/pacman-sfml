#pragma once

#include <SFML/Audio.hpp>

#include <map>

enum class Som { Inicio, Comer, Item, ComerFantasma, VidaExtra, Morte };

// Carrega os efeitos sonoros de assets/audio e toca sob demanda.
// O mudo (tecla M) vale para todas as partidas enquanto o programa estiver aberto.
class Sons {
public:
    Sons();

    void tocar(Som som);

    // Para "Comer": só recomeça quando o som anterior terminou, para soar contínuo enquanto come
    void tocarSeLivre(Som som);

    void pararTodos();

    static void alternarMudo();
    static bool mudo();

private:
    std::map<Som, sf::SoundBuffer> buffers;
    std::map<Som, sf::Sound> sons;
};
