#pragma once
#include <thread>
#include <vector>

#include "../RingBuffer.h"


class AudioProcessor
{
    
    
    RingBuffer<float>* InputRingBuffer;
    RingBuffer<float>* OutputRingBuffer;
    std::thread  ProcessorThread;
    
    
public:
    std::atomic<bool> IsRunning;
    AudioProcessor(RingBuffer<float>* Input, RingBuffer<float>* Output);
    void Process();
    void ProcessThread();
};
