#include <portaudio.h>
#include <iostream>

int main() {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }
    std::cout << "PortAudio initialized successfully!" << std::endl;
    Pa_Terminate();
    return 0;
}