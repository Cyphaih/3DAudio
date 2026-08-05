
#include <iostream>
#include "AudioInputDevice.h"
#include "AudioRenderer.h"
#include "AudioSetup.h"
#include "HelperFunctions.h"
#include "RingBuffer.h"


int main()
{
    AudioSetup AudioSetup;
    AudioInputDevice InputDevice;
    RingBuffer<float> InputRingBuffer(4800);
    AudioRenderer AudioRenderer;
    
    InputDevice.SetupInput(GetDeviceIDSafely(AudioSetup.GetStandardInputDevice())); 
    
    AudioRenderer.SetupRendering(GetDeviceIDSafely(AudioSetup.GetStandardOutputDevice()), &InputDevice.wFormatInfo);
    
    InputDevice.StartCapture(InputRingBuffer);
    AudioRenderer.StartRendering(InputRingBuffer);
    
    
    std::cin.get();
    
    
    return 0;
}