#include "ac_wrapper.h"

AcWrapper* AcWrapper::instance = nullptr;

AcWrapper::AcWrapper() {
    instance = this;
}

AcWrapper::~AcWrapper() {
    instance = nullptr;
}

int AcWrapper::init(std::function<void()> callback) {
    this->acWaveDetectedCallback = callback;
    return init_ac_wave_detector(cStyleCallback);
}

void AcWrapper::enable() {
    enable_ac_wave_detector();
}

void AcWrapper::cStyleCallback() {
    if (instance != nullptr && instance->acWaveDetectedCallback) {
        instance->acWaveDetectedCallback();
    }
}
