#ifndef SEVENZIP_COMPAT_H
#define SEVENZIP_COMPAT_H

#if MY_VER_MAJOR < 23
#define GETPROCADDRESS(_l_,_n_) (_l_.GetProc(_n_))
#else
#define GETPROCADDRESS(_l_,_n_) (GetProcAddress(_l_.Get_HMODULE(),_n_))
#endif

#if MY_VER_MAJOR < 23
extern "C" {
    typedef HRESULT (WINAPI *Func_GetModuleProp)(PROPID propID, PROPVARIANT *value);
}
namespace NModulePropID {
    enum EEnum {kInterfaceType, kVersion};
}
namespace NModuleInterfaceType {
    const UInt32 k_IUnknown_VirtDestructor_No  = 0;
    const UInt32 k_IUnknown_VirtDestructor_Yes = 1;
    const UInt32 k_IUnknown_VirtDestructor_ThisModule =
    #if !defined(_WIN32)
        k_IUnknown_VirtDestructor_Yes;
    #else
        k_IUnknown_VirtDestructor_No;
    #endif
}
#endif

#if MY_VER_MAJOR < 23
#define Z7_COM_UNKNOWN_IMP_1(i) MY_UNKNOWN_IMP1(i)
#define Z7_COM_UNKNOWN_IMP_2(i1, i2) MY_UNKNOWN_IMP2(i1, i2)
#define Z7_COM_UNKNOWN_IMP_3(i1, i2, i3) MY_UNKNOWN_IMP3(i1, i2, i3)
#define Z7_COM_UNKNOWN_IMP_4(i1, i2, i3, i4) MY_UNKNOWN_IMP4(i1, i2, i3, i4)
#define Z7_override override
#define Z7_final final
#endif

#endif
