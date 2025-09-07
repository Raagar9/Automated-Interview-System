#include <iostream>
#include <pocketsphinx.h>
#include <portaudio.h>

#define SAMPLE_RATE 16000
#define FRAMES_PER_BUFFER 512

// Function to handle audio input and speech recognition
void recognize_from_audio() {
    ps_decoder_t *ps;
    cmd_ln_t *config;

    // Configure PocketSphinx
    config = cmd_ln_init(NULL, ps_args(), TRUE,
                         "-hmm", "C:/Program Files (x86)/PocketSphinx/share/pocketsphinx/model/en-us/en-us",
                         "-lm", "C:/Program Files (x86)/PocketSphinx/share/pocketsphinx/model/en-us/en-us.lm.bin",
                         "-dict", "C:/Program Files (x86)/PocketSphinx/share/pocketsphinx/model/en-us/cmudict-en-us.dict",
                         NULL);

    if (config == NULL) {
        std::cerr << "Failed to create config." << std::endl;
        return;
    }

    ps = ps_init(config);
    if (ps == NULL) {
        std::cerr << "Failed to create recognizer." << std::endl;
        cmd_ln_free_r(config);  // Clean up config if initialization fails
        return;
    }

    // Start recording
    PaError err;
    err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        ps_free(ps);
        cmd_ln_free_r(config);
        return;
    }

    PaStream *stream;
    err = Pa_OpenDefaultStream(&stream, 1, 0, paInt16, SAMPLE_RATE, FRAMES_PER_BUFFER, NULL, NULL);
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        ps_free(ps);
        cmd_ln_free_r(config);
        return;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        ps_free(ps);
        cmd_ln_free_r(config);
        return;
    }

    // Process audio
    int16_t buffer[FRAMES_PER_BUFFER];
    while (true) {
        int num_read = Pa_ReadStream(stream, buffer, FRAMES_PER_BUFFER);
        if (num_read < 0) {
            std::cerr << "PortAudio error: " << Pa_GetErrorText(num_read) << std::endl;
            break;
        }
        if (num_read == 0) {
            continue; // No data read, continue to next iteration
        }
        ps_process_raw(ps, buffer, num_read, FALSE, FALSE);
        const char *hyp = ps_get_hyp(ps, NULL);
        if (hyp != NULL) {
            std::cout << "Recognized: " << hyp << std::endl;
        }
    }

    // Clean up
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    ps_free(ps);
    cmd_ln_free_r(config);
}

int main() {
    recognize_from_audio();
    return 0;
}
