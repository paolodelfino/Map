# What?
*PastDSE* is a **Driver Sign Enforcement** "bypass" using a leaked EV code signing certificate.
It is actually not a real bypass since it does only change the date to 01-01-2014 before signing the driver and restores it afterwards.
The Kernel driver loader will accept all driver images as long as the code was signed by a *extended validation code signing certificate* which was not revoked.

The DSE "bypass" works **only** on Windows 10 x64: 1803, 1809, 1903. You **won't** be able to load *PastDSE* signed drivers on other Windows versions.
But it is still possible to use *PastDSE* with other DSE disabling techniques e.g. enabling testsigning or using [EfiGuard](https://github.com/Mattiwatti/EfiGuard).
It works, because *PastDSE* is basically a manual driver mapper, nothing more.


# Build Dependencies
- Visual Studio 2019 Community Edition (Visual Studio 2017 is still supported, see VS-2017 branch)
- Windows 10 x64 1803, 1809 and 1903 (may work on older versions, not verified)
- Windows 10 SDK 10.0.17763.0
- Windows Driver Kit
- Windows Universal CRT SDK
- C++/CLI support
- VC++ 2017 tools

The recommended way to install all dependencies is through [vs_community.exe](https://visualstudio.microsoft.com/).


# HowTo
If you do not want to build it from source, you can skip the text below and download the build artifacts from Github.

Assuming a successful (Debug) build, you have to do the automatic sign procedure by running `driver-sign.bat` as Administrator.  
If the console window outputs something like `Number of files successfully Verified: 1` then the procedure was probably succesful.  
It should now be possible to load the (Debug) target driver by running `driver-start.bat` as Administrator.  
You can now use **PastDSECtrl** to manual map your (unsigned) driver.  


# Insights
Your driver requires an exported  
`NTSTATUS DriverEntry(_In_  struct _DRIVER_OBJECT *DriverObject, _In_  PUNICODE_STRING RegistryPath)`  
symbol just as usual.  
  
**But**: `DriverObject` will *always* be a `NULL` pointer whereas `RegistryPath` points to the mapped driver base address.  
Since this is a manual mapped driver you can not use all kernel functions without getting either into trouble with *PatchGuard*  
or they just won't  work (usual returning an *Access denied*).  
Example:
- *PatchGuard* will complain if you use functions like `PsSetLoadImageNotifyRoutine`, `PsSetCreateProcessNotifyRoutine` and `PsSetCreateThreadNotifyRoutine`
- `ObRegisterCallbacks` returns *Access denied*
- there may be other functions e.g. `FltRegisterFilter`


# Contributors
Some slightly modified code from [BlackBone](https://github.com/DarthTon/Blackbone) for the driver mapping and relocation.
