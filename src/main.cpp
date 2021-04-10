#include <iostream>

#include "AudioPlayer.h"


const char *ogg = "../shared/bg.ogg";
const char *wav = "../shared/powerup.wav";

int main()
{
    AudioPlayer ap;

    Sound& bg = ap.addSound(ogg, "bg");
    ap.addSound(wav, "du");

    ap.play("du");
    ap.play(bg, false, 0.6f);

    while(!bg.isStopped())
    {

    }

    return 0;
}
