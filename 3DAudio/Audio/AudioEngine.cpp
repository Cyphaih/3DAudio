#include "AudioEngine.h"

#include "AudioHelperFunctions.h"


AudioEngine::AudioEngine()
{
    IsRunning = false;
}

void AudioEngine::StartAudioEngine()
{
    IsRunning = true;
    
    AudioSetup AudioSetup;
    AudioInputDevice InputDevice;
    AudioRenderer AudioRenderer;
    RingBuffer<float> InputRingBuffer(48000);
    RingBuffer<float> OutputRingBuffer(96000);
    
    AudioProcessor AudioProcessor(&InputRingBuffer, &OutputRingBuffer);
    
    InputDevice.SetupInput(GetDeviceIDSafely(AudioSetup.GetStandardInputDevice()));
    AudioRenderer.SetupRendering(GetDeviceIDSafely(AudioSetup.GetStandardOutputDevice()), &InputDevice.wFormatInfo);
    AudioRenderer.SetStereo();
    uint32_t SampleRate = InputDevice.wFormatInfo.SampleRate;
    
    InputDevice.StartCapture(InputRingBuffer);
    AudioRenderer.StartRendering(OutputRingBuffer);
    AudioProcessor.Process();
    
    while (IsRunning)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}
