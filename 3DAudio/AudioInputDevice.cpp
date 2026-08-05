#include "AudioInputDevice.h"
#include "AudioSetup.h"

#include "RingBuffer.h"
#include <mmdeviceapi.h>
#include <Audioclient.h>



void AudioInputDevice::InputThread(RingBuffer<float>& InputRingBuffer)
{
    AudioSetup audioSetup;
    if (FAILED(audioSetup.InitResult) || FAILED(audioSetup.InstanceResult)) return;
    
    ComPtr<IMMDevice> InputDevice = audioSetup.GetDeviceByID(DeviceId);
    if (!InputDevice) return;
    
    ComPtr<IAudioClient> Client;
    ComPtr<IAudioCaptureClient> CaptureClient;
    WAVEFORMATEX* waveFormat = nullptr;
    HRESULT hr = InitializeAudioInput(Client,CaptureClient,&waveFormat);
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
                                UINT32 TotalSamples = FramesAvailable * waveFormat->nChannels;
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
    
    CoTaskMemFree(waveFormat);
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
                                               ComPtr<IAudioCaptureClient>& captureClient, WAVEFORMATEX** waveFormat)
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

    hr = audioClient->GetMixFormat(waveFormat);
    if (FAILED(hr)) return hr;

    REFERENCE_TIME bufferDuration = 300000; // 1 second in 100-ns units
    hr = audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        0,
        bufferDuration,
        0,
        *waveFormat,
        nullptr);
    if (FAILED(hr)) return hr;

    hr = audioClient->GetService(__uuidof(IAudioCaptureClient), &captureClient);
    if (FAILED(hr)) CoTaskMemFree(*waveFormat);
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
}