#pragma once

#include "AudioSetup.h"
#include "AudioInputDevice.h"
#include "AudioRenderer.h"
#include "AudioProcessor.h"
#include "../RingBuffer.h"


class AudioEngine
{
    
    bool IsRunning;
public:
    AudioEngine();
    
    void StartAudioEngine();
    //void StopAudioEngine();
};
