#include "AudioSetup.h"

#include <iostream>
#include <ostream>
#include <mmdeviceapi.h>

AudioSetup::AudioSetup()
{
    InitResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(InitResult))
    {
        std::cout << "CoInitializeEx failed with error : " << InitResult << '\n';
    }
    CreateOrGetDeviceEnumerator();
}

AudioSetup::~AudioSetup()
{
    Cleanup();
   
}

void AudioSetup::Cleanup()
{
    ReleaseDeviceEnumerator();
    
    if (SUCCEEDED(InitResult))
    {
        CoUninitialize();
    }
}

Microsoft::WRL::ComPtr<IMMDevice> AudioSetup::GetDeviceByID(const std::wstring& deviceId)
{
    if (FAILED(InstanceResult)) return nullptr;
    
    Microsoft::WRL::ComPtr<IMMDevice> device;
    HRESULT hr = DeviceEnumerator->GetDevice(deviceId.c_str(), &device);
    if (FAILED(hr)) return nullptr;
    return device;
}

Microsoft::WRL::ComPtr<IMMDevice> AudioSetup::GetStandardInputDevice()
{
    Microsoft::WRL::ComPtr<IMMDevice> Device;
    HRESULT hr = DeviceEnumerator->GetDefaultAudioEndpoint(eCapture,eConsole,&Device);
    if (SUCCEEDED(hr))
    {
        return Device;
    }
    return nullptr;
}

Microsoft::WRL::ComPtr<IMMDevice> AudioSetup::GetStandardOutputDevice()
{
    Microsoft::WRL::ComPtr<IMMDevice> Device;
    HRESULT hr = DeviceEnumerator->GetDefaultAudioEndpoint(eRender,eConsole,&Device);
    if (SUCCEEDED(hr))
    {
        return Device;
    }
    return nullptr;
}

Microsoft::WRL::ComPtr<IMMDeviceEnumerator> AudioSetup::CreateOrGetDeviceEnumerator()
{
    if (FAILED(InstanceResult) || !DeviceEnumerator)
    {
        ReleaseDeviceEnumerator();
        
        InstanceResult = CoCreateInstance(
            __uuidof(MMDeviceEnumerator), 
            nullptr, 
            CLSCTX_INPROC_SERVER, 
            IID_PPV_ARGS(&DeviceEnumerator) 
        );
    }
    
    return DeviceEnumerator;
}

void AudioSetup::ReleaseDeviceEnumerator()
{
    
    DeviceEnumerator.Reset();
    InstanceResult = E_POINTER;
    
}
