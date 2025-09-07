#include <iostream>
#include <portaudio.h>
#include <fstream>
#include <vector>
#include <cstdint>

// Constants
#define SAMPLE_RATE 16000   // CD-quality audio
#define FRAMES_PER_BUFFER 512
#define NUM_CHANNELS 1      // Mono audio
#define RECORD_SECONDS 5

// WAV header structure
struct WAVHeader {
    char riff[4] = {'R', 'I', 'F', 'F'};
    int chunkSize;
    char wave[4] = {'W', 'A', 'V', 'E'};
    char fmt[4] = {'f', 'm', 't', ' '};
    int subChunk1Size = 16;
    short audioFormat = 1;
    short numChannels = NUM_CHANNELS;
    int sampleRate = SAMPLE_RATE;
    int byteRate;
    short blockAlign;
    short bitsPerSample = 16;
    char data[4] = {'d', 'a', 't', 'a'};
    int subChunk2Size;
};

// Function to write WAV header to file
void writeWAVHeader(std::ofstream &file, int totalFrames) {
    WAVHeader header;
    header.byteRate = SAMPLE_RATE * NUM_CHANNELS * (header.bitsPerSample / 8);
    header.blockAlign = NUM_CHANNELS * (header.bitsPerSample / 8);
    header.subChunk2Size = totalFrames * header.blockAlign;
    header.chunkSize = 36 + header.subChunk2Size;

    file.write(reinterpret_cast<const char *>(&header), sizeof(WAVHeader));
}

int main() {
    PaError err;
    PaStream *stream;
    std::vector<int16_t> recordedData(RECORD_SECONDS * SAMPLE_RATE);

    // Initialize PortAudio
    err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    // Open an audio stream for recording
    err = Pa_OpenDefaultStream(&stream,
                               NUM_CHANNELS, 0,  // Input only
                               paInt16,           // 16-bit integer samples
                               SAMPLE_RATE,
                               FRAMES_PER_BUFFER,
                               nullptr, nullptr); // No callback, use blocking API
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        Pa_Terminate();
        return 1;
    }

    // Start the audio stream
    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }

    std::cout << "Recording for " << RECORD_SECONDS << " seconds..." << std::endl;

    // Record audio data into the buffer
    err = Pa_ReadStream(stream, recordedData.data(), SAMPLE_RATE * RECORD_SECONDS);
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }

    // Stop and close the stream
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    std::cout << "Recording complete! Saving to 'output.wav'..." << std::endl;

    // Save the recorded data to a WAV file
    std::ofstream outFile("output.wav", std::ios::binary);
    writeWAVHeader(outFile, SAMPLE_RATE * RECORD_SECONDS);
    outFile.write(reinterpret_cast<const char *>(recordedData.data()), recordedData.size() * sizeof(int16_t));
    outFile.close();

    std::cout << "Saved 'output.wav' successfully!" << std::endl;
    return 0;
}