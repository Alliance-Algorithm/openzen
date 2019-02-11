#ifndef ZEN_IO_SYSTEMS_PCANBASICSYSTEM_H_
#define ZEN_IO_SYSTEMS_PCANBASICSYSTEM_H_

#include "io/IIoSystem.h"
#include "io/can/PcanBasicChannel.h"

#include <set>

namespace zen
{
    struct PcanFnTable
    {
        using InitializeFn = TPCANStatus (__stdcall *)(TPCANHandle, TPCANBaudrate, TPCANType, DWORD, WORD);
        using GetStatusFn = TPCANStatus (__stdcall *)(TPCANHandle);
        using ReadFn = TPCANStatus (__stdcall *)(TPCANHandle, TPCANMsg*, TPCANTimestamp*);
        using ResetFn = TPCANStatus (__stdcall *)(TPCANHandle);
        using SetValueFn = TPCANStatus (__stdcall *)(TPCANHandle, TPCANParameter, void*, DWORD);
        using UninitializeFn = TPCANStatus (__stdcall *)(TPCANHandle);
        using WriteFn = TPCANStatus (__stdcall *)(TPCANHandle, TPCANMsg*);

        InitializeFn initialize;
        GetStatusFn getStatus;
        ReadFn read;
        ResetFn reset;
        SetValueFn setValue;
        UninitializeFn uninitialize;
        WriteFn write;
    };

    class PcanBasicSystem : public IIoSystem
    {
        constexpr static unsigned int DEFAULT_BAUDRATE = PCAN_BAUD_125K;

    public:
        constexpr static const char KEY[] = "PCANBasic";

        ~PcanBasicSystem();

        bool available() override;

        ZenError listDevices(std::vector<ZenSensorDesc>& outDevices) override;

        std::unique_ptr<BaseIoInterface> obtain(const ZenSensorDesc& desc, ZenSensorInitError& outError) override;

        static PcanFnTable fnTable;

    private:
        ZenError openChannel(TPCANHandle handle);

        std::set<std::unique_ptr<PcanBasicChannel>> m_channels;
        void* m_handle;
    };
}

#endif
