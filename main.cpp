#include <iostream>

#include "Util/LoggerUtil.h"
#include "Util/GameUtil.h"
#include "Util/Terminal.h"

const std::string SIGNATURE = "74 ? F3 0F 5D 35";

int main()
{
    Terminal::setup();
    if (!GameUtil::close()) {
        LoggerUtil::log("(!) Starting Minecraft.Windows.exe", std::chrono::milliseconds(25));
        system("start minecraft://");
    }

    LoggerUtil::warning("(?) Waiting for the game...", std::chrono::milliseconds(10));
    while (GameUtil::handle == nullptr || GameUtil::module == nullptr)
    {
        GameUtil::handle = GameUtil::GetProcessByName("Minecraft.Windows.exe");
        GameUtil::module = GameUtil::GetModule(GameUtil::handle);
        if (GameUtil::handle != nullptr && GameUtil::module != nullptr) LoggerUtil::success("(+) Game found!", std::chrono::milliseconds(10));
    }
    Terminal::clear();

    GameUtil::baseAddress = reinterpret_cast<uintptr_t>(GameUtil::module);
    LoggerUtil::warning("(?) Scan in progress...", std::chrono::milliseconds(20));

    void* sig = GameUtil::scanSig(SIGNATURE);
    if (sig == nullptr) {
        LoggerUtil::error("(-) Not found Reach sig.", std::chrono::milliseconds(10));
        std::this_thread::sleep_for(std::chrono::seconds(5));
        return 0;
    }

    float reach;
    LoggerUtil::log2("(!) Enter the desired reach value (default: 3.0): ", std::chrono::milliseconds(10));
    std::cin >> reach;

    if (std::cin.fail() || reach <= 0) {
        LoggerUtil::error("(-) Invalid reach value. (ex: 3.1)", std::chrono::milliseconds(10));
        std::this_thread::sleep_for(std::chrono::seconds(5));
        return 1;
    }

    uintptr_t addrNOP = reinterpret_cast<uintptr_t>(sig);
    uintptr_t addrBytesCREATIVE = addrNOP + 0x2;

    constexpr size_t LEN = 8;
    uint8_t ins[LEN]{};

    GameUtil::readMemory(reinterpret_cast<void*>(addrBytesCREATIVE), ins, sizeof(ins));

    auto disp = static_cast<int32_t>((ins[4]) | (ins[5] << 8) | (ins[6] << 16) | (ins[7] << 24));
    auto addrReach = static_cast<uintptr_t>(static_cast<int64_t>(addrBytesCREATIVE) + LEN + disp);

    GameUtil::nopBytes(reinterpret_cast<void*>(addrNOP), 2);
    GameUtil::patchBytes(reinterpret_cast<void*>(addrReach), &reach, sizeof(float));

    LoggerUtil::success("(!) Reach successfully injected!", std::chrono::milliseconds(25));
    std::this_thread::sleep_for(std::chrono::seconds(5));
    return 0;
}
