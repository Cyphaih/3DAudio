#pragma once

#include <windows.h>
#include <cstdint>
#include <mmreg.h>

enum class AudioSampleFormat : uint8_t
{
    Unknown = 0,
    Int16,
    Int32,
    Float32
};


struct WaveFormatInfo
{
    bool IsSetup = false;
    uint32_t Channels = 0;
    uint32_t SampleRate = 0;
    uint32_t BytesPerFrame = 0;
    uint32_t BitsPerSample = 0;
    AudioSampleFormat Format = AudioSampleFormat::Unknown;
    
    WaveFormatInfo() = default;
    
    explicit WaveFormatInfo(const WAVEFORMATEX* Wave)
    {
        if (!Wave) return;
        
        IsSetup = true;
        Channels = Wave->nChannels;
        SampleRate = Wave->nSamplesPerSec;
        BytesPerFrame = Wave->nBlockAlign;
        BitsPerSample = Wave->wBitsPerSample;
        
        if (Wave->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
        {
            const WAVEFORMATEXTENSIBLE* ext = reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(Wave);
            if (ext->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
            {
                Format = AudioSampleFormat::Float32;
            }
            else if (ext->SubFormat == KSDATAFORMAT_SUBTYPE_PCM)
            {
                Format = (BitsPerSample == 16) ? AudioSampleFormat::Int16 : AudioSampleFormat::Int32;
            }
        }
        else if (Wave->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
        {
            Format = AudioSampleFormat::Float32;
        }
        else if (Wave->wFormatTag == WAVE_FORMAT_PCM)
        {
            Format = (BitsPerSample == 16) ? AudioSampleFormat::Int16 : AudioSampleFormat::Int32;
        }
    }
    
    void Update()
    {
        BytesPerFrame = Channels* BitsPerSample / 8;
    }
};
