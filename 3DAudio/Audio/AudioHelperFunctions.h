#pragma once
#include <mmdeviceapi.h>
#include <string>
#include <wrl/client.h>
#include "WaveFormatInfo.h"
#include  <iostream>

inline std::wstring GetDeviceIDSafely(const Microsoft::WRL::ComPtr<IMMDevice>& Device)
{
    if (!Device) return L"";

    LPWSTR rawId = nullptr;
    HRESULT hr = Device->GetId(&rawId);
    
    if (FAILED(hr) || !rawId) return L"";

    std::wstring DeviceId(rawId);

    CoTaskMemFree(rawId); 

    return DeviceId;
}

inline void PrintWaveFormatInfo(const WaveFormatInfo* Info)
{
    std::cout 
        << " Is Setup: " << Info->IsSetup << " Channels: " << Info->Channels 
        << " SampleRate: " << Info->SampleRate << " BytesPerFrame: " << Info->BytesPerFrame
        << " BitsPerSample: " << Info->BitsPerSample << " Format: " << static_cast<int>(Info->Format) 
        << '\n' ;
}

inline WAVEFORMATEXTENSIBLE BuildWaveFormat(const WaveFormatInfo& info)
{
    WAVEFORMATEXTENSIBLE format = {};
    
    format.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    format.Format.nChannels = static_cast<WORD>(info.Channels);
    format.Format.nSamplesPerSec = info.SampleRate;
    format.Format.wBitsPerSample = static_cast<WORD>(info.BitsPerSample);
    format.Format.nBlockAlign = static_cast<WORD>(info.BytesPerFrame);
    format.Format.nAvgBytesPerSec = info.SampleRate * info.BytesPerFrame;
    format.Format.cbSize = 22; // Extra size required for WAVEFORMATEXTENSIBLE

    format.Samples.wValidBitsPerSample = static_cast<WORD>(info.BitsPerSample);
    
    
    if (info.Channels == 1) {
        format.dwChannelMask = SPEAKER_FRONT_CENTER;
    } else if (info.Channels == 2) {
        format.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
    } else {
        format.dwChannelMask = 0; 
    }

    
    if (info.Format == AudioSampleFormat::Float32) {
        format.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
    } else {
        format.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
    }

    return format;
}