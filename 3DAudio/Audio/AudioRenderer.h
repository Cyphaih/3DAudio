#pragma once
#include <atomic>
#include <string>
#include <thread>
#include <wrl/client.h>

#include "WaveFormatInfo.h"

struct IAudioClient;
struct IAudioRenderClient;
template<typename T>
class RingBuffer;

using Microsoft::WRL::ComPtr;




class AudioRenderer
{
    std::wstring DeviceId;
    std::atomic<bool> IsRunning;
    std::thread RenderThread;
public:
    AudioRenderer();
    ~AudioRenderer();
    void SetupRendering(std::wstring OutDeviceID,const WaveFormatInfo* info = nullptr);
    void StartRendering(RingBuffer<float>& SampleBuffer);
    HRESULT InitRender (ComPtr<IAudioClient>& AudioClient,
                     ComPtr<IAudioRenderClient>& CaptureClient);
    void OutputThread(RingBuffer<float>& SampleBuffer);
    void StopRendering();
    
    void StoreWaveFormatInfo(const WaveFormatInfo* info);
    void SetStereo();
    
    
    WaveFormatInfo wInfo;
};
