import wave

def check_sample_rate(file_path):
    with wave.open(file_path, 'rb') as wf:
        sample_rate = wf.getframerate()
        return sample_rate

file_path = 'output.wav'
sample_rate = check_sample_rate(file_path)
if sample_rate == 16000:
    print("The sample rate is 16 kHz.")
else:
    print(f"The sample rate is {sample_rate} Hz.")
