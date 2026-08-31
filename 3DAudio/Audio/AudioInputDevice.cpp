#include "AudioInputDevice.h"
#include "AudioSetup.h"

#include "../RingBuffer.h"
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <iostream>
#include <ostream>

#include "WaveFormatInfo.h"
#include  "AudioHelperFunctions.h"

void AudioInputDevice::InputThread(RingBuffer<float>& InputRingBuffer)
{
    AudioSetup audioSetup;
    if (FAILED(audioSetup.InitResult) || FAILED(audioSetup.InstanceResult)) return;
    
    ComPtr<IMMDevice> InputDevice = audioSetup.GetDeviceByID(DeviceId);
    if (!InputDevice) return;
    
    ComPtr<IAudioClient> Client;
    ComPtr<IAudioCaptureClient> CaptureClient;
    
    HRESULT hr = InitializeAudioInput(Client,CaptureClient);
    if (SUCCEEDED(hr))
    {
        hr = Client->Start();
        if (SUCCEEDED(hr))
        {
            while (IsRunning.load())
            {
                UINT32 PacketLength = 0;
                hr = CaptureClient->GetNextPacketSize(&PacketLength);
                if (SUCCEEDED(hr))
                {
                    while (PacketLength > 0 && IsRunning.load())
                    {
                        BYTE* Data = nullptr;
                        UINT32 FramesAvailable;
                        DWORD Flags = 0;
                    
                        hr = CaptureClient->GetBuffer(
                            &Data, 
                            &FramesAvailable, 
                            &Flags, 
                            nullptr, 
                            nullptr
                            );
                        if (SUCCEEDED(hr))
                        {
                            if (Data != nullptr)
                            {
                                float* FloatSamples = reinterpret_cast<float*>(Data);
                                UINT32 TotalSamples = FramesAvailable * wFormatInfo.Channels;
                                InputRingBuffer.Write(FloatSamples, TotalSamples);
                            
                            }
                            CaptureClient->ReleaseBuffer(FramesAvailable);
                        }
                        CaptureClient->GetNextPacketSize(&PacketLength);
                    
                    }
                }
                
                Sleep(10);
            }
        }
        Client->Stop();
    }
    
}

AudioInputDevice::AudioInputDevice()= default;

AudioInputDevice::~AudioInputDevice()
{
    StopCapture();
}



bool AudioInputDevice::StartCapture(RingBuffer<float>& InputRingBuffer)
{
    if (IsRunning.load()) return false; 
    
    if (DeviceId.empty()) return false;
    
    
    
    
    IsRunning.store(true);
    CaptureThread = std::thread(&AudioInputDevice::InputThread, this, std::ref(InputRingBuffer));
    return true;
}

void AudioInputDevice::StopCapture()
{
    
        IsRunning.store(false); 
        if (CaptureThread.joinable()) {
            CaptureThread.join(); 
        }
    
}

void AudioInputDevice::SetupInput(const std::wstring& NewDeviceId)
{
    
    if (IsRunning.load())return;
    
    
    DeviceId = NewDeviceId;
    
    StoreWaveFormatInfo();
    
    
    
    
}




HRESULT AudioInputDevice::InitializeAudioInput(ComPtr<IAudioClient>& audioClient,
                                               ComPtr<IAudioCaptureClient>& captureClient)
{
    ComPtr<IMMDeviceEnumerator> enumerator;
    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), &enumerator);
    if (FAILED(hr)) return hr;

    ComPtr<IMMDevice> Device;
    hr = enumerator->GetDevice(DeviceId.c_str(), &Device);
    if (FAILED(hr))return hr;

    hr = Device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, &audioClient);
    if (FAILED(hr)) return hr;

    if (!wFormatInfo.IsSetup)return S_FALSE;
    REFERENCE_TIME bufferDuration = 300000; // 1 second in 100-ns units
    
    
    
    WAVEFORMATEXTENSIBLE RequestedFormat = BuildWaveFormat(wFormatInfo);
  
    
    
    hr = audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,
        bufferDuration,
        0,
        reinterpret_cast<const WAVEFORMATEX*>(&RequestedFormat),
        nullptr);
    if (FAILED(hr))
    {
        
        std::cout << " Failed to Initialize MONO Waveformat" << std::endl;
        return hr;
    }

    
    hr = audioClient->GetService(__uuidof(IAudioCaptureClient), &captureClient);
    return hr;
}

void AudioInputDevice::StoreWaveFormatInfo()
{
    wFormatInfo = WaveFormatInfo{};
    ComPtr<IMMDeviceEnumerator> enumerator;
    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), &enumerator);
    if (FAILED(hr)) return;

    ComPtr<IMMDevice> Device;
    hr = enumerator->GetDevice(DeviceId.c_str(), &Device);
    if (FAILED(hr))return;

    ComPtr<IAudioClient> audioClient;
    hr = Device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, &audioClient);
    if (FAILED(hr)) return;

    WAVEFORMATEX* waveFormat;
    hr = audioClient->GetMixFormat(&waveFormat);
    if (FAILED(hr)) return;
    
    wFormatInfo = WaveFormatInfo(waveFormat);
    wFormatInfo.Channels = 1;
    
    wFormatInfo.Update();
    
    CoTaskMemFree(waveFormat);
}