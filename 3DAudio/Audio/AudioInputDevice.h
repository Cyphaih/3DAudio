#pragma once
#include <atomic>
#include <string>
#include <thread>
#include <wrl/client.h>
#include "WaveFormatInfo.h"


struct IMMDevice;
struct IAudioClient;

template <typename T>
class RingBuffer;

struct IAudioCaptureClient;


using Microsoft::WRL::ComPtr;




class AudioInputDevice
{
private:
    std::wstring DeviceId;
    std::thread CaptureThread;
    std::atomic<bool> IsRunning{ false };
    
    void InputThread(RingBuffer<float>& InputRingBuffer);
    
    
    
    
public:
    AudioInputDevice();
    ~AudioInputDevice();
    
    AudioInputDevice(const AudioInputDevice&) = delete;
    AudioInputDevice& operator=(const AudioInputDevice&) = delete;

    AudioInputDevice(AudioInputDevice&&) = delete;
    AudioInputDevice& operator=(AudioInputDevice&&) = delete;
    
   
    bool StartCapture(RingBuffer<float>& InputRingBuffer);
    void StopCapture();
    
    void SetupInput(const std::wstring& NewDeviceId);
    
    HRESULT InitializeAudioInput(
        ComPtr<IAudioClient>& audioClient,
        ComPtr<IAudioCaptureClient>& captureClient);
    
    void StoreWaveFormatInfo();
    
    WaveFormatInfo wFormatInfo;
};






