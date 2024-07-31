#ifndef AC_WRAPPER_H
#define AC_WRAPPER_H

#include <functional>

extern "C" {
    #include "ac_wave_detector.h"
}

class AcWrapper {
public:
    AcWrapper();
    ~AcWrapper();

    int init(std::function<void()> acWaveDetectedCallback);
    void enable();

private:
    static void cStyleCallback();

    std::function<void()> acWaveDetectedCallback;
    static AcWrapper* instance;
};

#endif // AC_WRAPPER_H
