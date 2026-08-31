#include "AudioProcessor.h"
#include <thread>

void AudioProcessor::ProcessThread()
{
    if (IsRunning.load() || InputRingBuffer == nullptr || OutputRingBuffer == nullptr) return;
    
    IsRunning.store(true);
    
    //Setup Processing
    const size_t BufferSize = 1024;
    std::vector<float> HistoryBuffer(BufferSize);
    float TempArray[BufferSize];
    float StereoOutputArray[BufferSize * 2];
    size_t SampleAmount; 
    size_t writeHead = 0;
    
    
    // MyVector SoundOrigin{1,2,3}; <- some custom vector for later implementation tests 
    
    int DelayFrames = 32; 
    
    
    while (IsRunning)
    {
    
        SampleAmount = 0;
        size_t ReadAmount =  InputRingBuffer->Read(TempArray,BufferSize);
        for (size_t i=0;i<ReadAmount;i++)
        {
            float currentSample = TempArray[i];
            HistoryBuffer[writeHead] = currentSample;
        
            float leftSample = currentSample;
            size_t readPos = (writeHead + BufferSize - DelayFrames) % BufferSize;
            float rightSample = HistoryBuffer[readPos];
        
            StereoOutputArray[i*2 + 0] = leftSample;
            StereoOutputArray[i*2 + 1] = rightSample;
            SampleAmount += 2; // 
            writeHead = (writeHead + 1) % BufferSize;
        }
        OutputRingBuffer->Write(StereoOutputArray,SampleAmount);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(5)); //After testing and getting used to, change to event based instead timer
    }
    
}

void AudioProcessor::Process()
{
    if (IsRunning.load()||InputRingBuffer == nullptr|| OutputRingBuffer == nullptr) return;
    ProcessorThread = std::thread(&AudioProcessor::ProcessThread,this);
}


AudioProcessor::AudioProcessor(RingBuffer<float>* Input, RingBuffer<float>* Output)
{
    InputRingBuffer = Input;
    OutputRingBuffer = Output;
    IsRunning.store(false);
}
