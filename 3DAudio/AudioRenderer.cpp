#include "AudioRenderer.h"

#include <Audioclient.h>
#include <iostream>
#include <mmdeviceapi.h>
#include <ostream>
#include <vector>
#include <wrl/client.h>

#include "AudioSetup.h"
#include "HelperFunctions.h"
#include "RingBuffer.h"

AudioRenderer::AudioRenderer()
{
    IsRunning.store(false);
}

AudioRenderer::~AudioRenderer()
{
    StopRendering();
}

void AudioRenderer::SetupRendering(std::wstring OutDeviceID,const WaveFormatInfo* info )
{
    if (IsRunning.load() )return;
    
    DeviceId = OutDeviceID;
    StoreWaveFormatInfo(info);
}

void AudioRenderer::StartRendering(RingBuffer<float>& SampleBuffer)
{
    if (IsRunning.load()) return;
    
    if (DeviceId.empty()) return;
    
    IsRunning.store(true);
    
    RenderThread = std::thread(&AudioRenderer::OutputThread, this, std::ref(SampleBuffer));
    
}

HRESULT AudioRenderer::InitRender(ComPtr<IAudioClient>& AudioClient, ComPtr<IAudioRenderClient>& RenderClient)
{
    std::cout << "In init" << '\n';
    ComPtr<IMMDeviceEnumerator> DeviceEnumerator = nullptr;
    HRESULT hr =  CoCreateInstance(
            __uuidof(MMDeviceEnumerator), 
            nullptr, 
            CLSCTX_INPROC_SERVER, 
            IID_PPV_ARGS(&DeviceEnumerator) 
        );  
    
    if (FAILED(hr)) return hr;
    
    std::cout << "Got Device Enum" << '\n';
    ComPtr<IMMDevice> Device;
    hr= DeviceEnumerator->GetDevice(DeviceId.c_str(),&Device);
    if (FAILED(hr)) return hr;
    
    std::cout << "Got Device" << '\n';
    
    hr = Device->Activate(__uuidof(IAudioClient),CLSCTX_ALL,nullptr,&AudioClient);
    if (FAILED(hr)) return hr;
    
    std::cout << "Activated Device" << '\n';
    
    
    
    WAVEFORMATEXTENSIBLE RequestedFormat = BuildWaveFormat(wInfo);
    
    
    
    REFERENCE_TIME BufferDuration = 300000;
    hr = AudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,
        BufferDuration,
        0,
        reinterpret_cast<const WAVEFORMATEX*>(&RequestedFormat),
        nullptr);
    if (FAILED(hr)) return hr;
    
    std::cout << "Succesfull in calling AudioClient -> initialize" << '\n';
    hr = AudioClient->GetService(__uuidof(IAudioRenderClient), &RenderClient);
    
    return hr;
}

void AudioRenderer::OutputThread(RingBuffer<float>& SampleBuffer)
{
    AudioSetup audioSetup;
    if (FAILED(audioSetup.InitResult) || FAILED(audioSetup.InstanceResult)) return;
    
    
    ComPtr<IMMDevice> OutputDevice = audioSetup.GetDeviceByID(DeviceId);
    if (!OutputDevice) return;
    
    
    ComPtr<IAudioClient> Client;
    ComPtr<IAudioRenderClient> RenderClient;
    
    HRESULT hr = InitRender(Client,RenderClient);
   
    
    Client ->Start();
    
    
    
    
    UINT32 BufferFrameCount = 0;
    hr = Client->GetBufferSize(&BufferFrameCount);
    
    
    UINT32 NumChannels = wInfo.Channels; 
    std::vector<float> PushBuffer(static_cast<size_t>(BufferFrameCount) * NumChannels);
    
    
    if (SUCCEEDED(hr))
    {
        
        while (IsRunning.load())
        {
            
            UINT32 CurrentPadding = 0;
            hr = Client->GetCurrentPadding(&CurrentPadding);
            if (FAILED(hr)) break;
            
            UINT32 FramesNeeded = BufferFrameCount - CurrentPadding;
            if (FramesNeeded > 0)
            {
                UINT32 SamplesNeeded = FramesNeeded*NumChannels;
                size_t SamplesRead = SampleBuffer.Read(PushBuffer.data(), SamplesNeeded);
                
                if (SamplesRead < SamplesNeeded)
                {
                    std::fill_n(PushBuffer.data() + SamplesRead, SamplesNeeded - SamplesRead, 0.0f);
                }
                BYTE* pData = nullptr;
                hr = RenderClient->GetBuffer(FramesNeeded, &pData);

                if (SUCCEEDED(hr) && pData != nullptr)
                {
                   
                    std::memcpy(pData, PushBuffer.data(), SamplesNeeded * sizeof(float));

                     
                    RenderClient->ReleaseBuffer(FramesNeeded, 0);
                }
            }
                
            Sleep(5);
        }
    }
    Client ->Stop();
}

void AudioRenderer::StopRendering()
{
    
        IsRunning.store(false); 
        if (RenderThread.joinable()) {
            RenderThread.join(); 
        }
    
}

void AudioRenderer::StoreWaveFormatInfo(const WaveFormatInfo* info)
{
    if (info)
    {
        wInfo = *info;
        return;
    }
    wInfo = WaveFormatInfo{};
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
    wInfo = WaveFormatInfo(waveFormat);
}