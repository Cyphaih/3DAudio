#pragma once

#include <mmdeviceapi.h>
#include <string>
#include <wrl/client.h>



//This handles all the COM stuff, will also include calls to device specific classes necessary, 
//Like Input Devices and Output Devices

struct IMMDeviceEnumerator;
class AudioSetup
{
    
private:
    
    Microsoft::WRL::ComPtr<IMMDeviceEnumerator> DeviceEnumerator;
    
    Microsoft::WRL::ComPtr<IMMDeviceEnumerator> CreateOrGetDeviceEnumerator();
    void ReleaseDeviceEnumerator();
    
public:
    AudioSetup();
    
    ~AudioSetup();
    
    void Cleanup();
    
    AudioSetup(const AudioSetup&) = delete;
    AudioSetup& operator=(const AudioSetup&) = delete;

    AudioSetup(AudioSetup&&) = delete;
    AudioSetup& operator=(AudioSetup&&) = delete;
    
    bool IsSetup(){ if (!DeviceEnumerator) return false; return true;}
    
    HRESULT InitResult;
    HRESULT InstanceResult;
    
    Microsoft::WRL::ComPtr<IMMDevice> GetDeviceByID(const std::wstring& deviceId);
    Microsoft::WRL::ComPtr<IMMDevice> GetStandardInputDevice();
    Microsoft::WRL::ComPtr<IMMDevice> GetStandardOutputDevice();
};


