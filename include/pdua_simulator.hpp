#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class PDUASimulator
{
public:
    enum class StopReason : std::uint8_t
    {
        None,
        RetInstruction,
        UnknownOpcode,
        ProgramCounterOutOfRange,
        StepLimitReached,
        StackUnderflow
    };

    PDUASimulator();

    void reset();
    void loadProgram(const std::vector<std::uint8_t>& program);
    void run();

    [[nodiscard]] StopReason stopReason() const noexcept;
    [[nodiscard]] std::string stopReasonString() const;

    [[nodiscard]] std::uint8_t acc() const noexcept;
    [[nodiscard]] std::uint8_t a() const noexcept;
    [[nodiscard]] std::uint8_t dptr() const noexcept;
    [[nodiscard]] std::uint8_t pc() const noexcept;

    [[nodiscard]] bool zeroFlag() const noexcept;
    [[nodiscard]] bool negativeFlag() const noexcept;
    [[nodiscard]] bool carryFlag() const noexcept;

    [[nodiscard]] std::uint8_t readMemory(std::uint8_t address) const noexcept;
    [[nodiscard]] std::size_t loadedProgramSize() const noexcept;

    void dumpState() const;
    void dumpMemory(std::uint8_t start, std::uint8_t end) const;
    void dumpDataSection(std::uint8_t start, std::uint8_t count) const;

private:
    static constexpr std::size_t kMemorySize = 256;
    static constexpr std::size_t kMaxSteps = 100000;

    std::uint8_t acc_;
    std::uint8_t a_;
    std::uint8_t dptr_;
    std::uint8_t pc_;
    bool zero_flag_;
    bool negative_flag_;
    bool carry_flag_;

    std::vector<std::uint8_t> memory_;
    std::vector<std::uint8_t> call_stack_;
    std::size_t loaded_program_size_;
    StopReason stop_reason_;
    std::uint8_t last_opcode_;

    void updateZNFlags(std::uint8_t value);
    void setFlags(std::uint8_t value, bool carry) noexcept;
    [[nodiscard]] bool isKnownOpcode(std::uint8_t opcode) const noexcept;
};
