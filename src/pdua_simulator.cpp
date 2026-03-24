#include "pdua_simulator.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <stdexcept>

PDUASimulator::PDUASimulator()
    : acc_(0),
      a_(0),
      dptr_(0),
      pc_(0),
      zero_flag_(false),
      negative_flag_(false),
      carry_flag_(false),
      memory_(kMemorySize, 0),
      call_stack_(),
      loaded_program_size_(0),
      stop_reason_(StopReason::None),
      last_opcode_(0)
{
}

void PDUASimulator::reset()
{
    acc_ = 0;
    a_ = 0;
    dptr_ = 0;
    pc_ = 0;
    zero_flag_ = false;
    negative_flag_ = false;
    carry_flag_ = false;
    stop_reason_ = StopReason::None;
    last_opcode_ = 0;

    std::fill(memory_.begin(), memory_.end(), static_cast<std::uint8_t>(0));
    call_stack_.clear();
    loaded_program_size_ = 0;
}

void PDUASimulator::loadProgram(const std::vector<std::uint8_t>& program)
{
    reset();

    if (program.size() > memory_.size())
    {
        throw std::runtime_error("Program too large for 256-byte PDUA memory.");
    }

    for (std::size_t i = 0; i < program.size(); ++i)
    {
        memory_[i] = program[i];
    }

    loaded_program_size_ = program.size();
    pc_ = 0;
}

void PDUASimulator::updateZNFlags(std::uint8_t value)
{
    zero_flag_ = (value == 0U);
    negative_flag_ = (value & 0x80U) != 0U;
}

void PDUASimulator::setFlags(std::uint8_t value, bool carry) noexcept
{
    updateZNFlags(value);
    carry_flag_ = carry;
}

bool PDUASimulator::isKnownOpcode(std::uint8_t opcode) const noexcept
{
    switch (opcode)
    {
    case 0x08:
    case 0x10:
    case 0x18:
    case 0x20:
    case 0x28:
    case 0x30:
    case 0x38:
    case 0x40:
    case 0x48:
    case 0x50:
    case 0x58:
    case 0x60:
    case 0x68:
    case 0x70:
    case 0x78:
    case 0x80: // XOR ACC, A
    case 0x88: // SLL ACC, A
    case 0x90: // SRL ACC, A
    case 0x98: // OR ACC, A
    case 0xA0: // SUB ACC, A
        return true;
    default:
        return false;
    }
}

void PDUASimulator::run()
{
    std::size_t steps = 0;

    while (true)
    {
        if (steps >= kMaxSteps)
        {
            stop_reason_ = StopReason::StepLimitReached;
            return;
        }

        if (pc_ >= memory_.size())
        {
            stop_reason_ = StopReason::ProgramCounterOutOfRange;
            return;
        }

        const std::uint8_t opcode = memory_[pc_++];
        last_opcode_ = opcode;

        if (!isKnownOpcode(opcode))
        {
            stop_reason_ = StopReason::UnknownOpcode;
            return;
        }

        ++steps;

        switch (opcode)
        {
        case 0x08: // MOV ACC, A
            acc_ = a_;
            setFlags(acc_, false);
            break;

        case 0x10: // MOV A, ACC
            a_ = acc_;
            setFlags(a_, false);
            break;

        case 0x18: // MOV ACC, CTE
            if (pc_ >= memory_.size())
            {
                stop_reason_ = StopReason::ProgramCounterOutOfRange;
                return;
            }
            acc_ = memory_[pc_++];
            break;

        case 0x20: // MOV ACC, [DPTR]
            acc_ = memory_[dptr_];
            break;

        case 0x28: // MOV DPTR, ACC
            dptr_ = acc_;
            setFlags(dptr_, false);
            break;

        case 0x30: // MOV [DPTR], ACC
            memory_[dptr_] = acc_;
            setFlags(acc_, false);
            break;

        case 0x38: // INV ACC
            acc_ = static_cast<std::uint8_t>(~acc_);
            setFlags(acc_, false);
            break;

        case 0x40: // AND ACC, A
            acc_ = static_cast<std::uint8_t>(acc_ & a_);
            setFlags(acc_, false);
            break;

        case 0x48: // ADD ACC, A
        {
            const std::uint16_t sum =
                static_cast<std::uint16_t>(acc_) +
                static_cast<std::uint16_t>(a_);

            acc_ = static_cast<std::uint8_t>(sum & 0xFFU);
            setFlags(acc_, sum > 0xFFU);
            break;
        }

        case 0x50: // JMP CTE
            if (pc_ >= memory_.size())
            {
                stop_reason_ = StopReason::ProgramCounterOutOfRange;
                return;
            }
            pc_ = memory_[pc_];
            break;

        case 0x58: // JZ CTE
        {
            if (pc_ >= memory_.size())
            {
                stop_reason_ = StopReason::ProgramCounterOutOfRange;
                return;
            }
            const std::uint8_t addr = memory_[pc_++];
            if (zero_flag_)
            {
                pc_ = addr;
            }
            break;
        }

        case 0x60: // JN CTE
        {
            if (pc_ >= memory_.size())
            {
                stop_reason_ = StopReason::ProgramCounterOutOfRange;
                return;
            }
            const std::uint8_t addr = memory_[pc_++];
            if (negative_flag_)
            {
                pc_ = addr;
            }
            break;
        }

        case 0x68: // JC CTE
        {
            if (pc_ >= memory_.size())
            {
                stop_reason_ = StopReason::ProgramCounterOutOfRange;
                return;
            }
            const std::uint8_t addr = memory_[pc_++];
            if (carry_flag_)
            {
                pc_ = addr;
            }
            break;
        }

        case 0x70: // CALL DIR/CTE
        {
            if (pc_ >= memory_.size())
            {
                stop_reason_ = StopReason::ProgramCounterOutOfRange;
                return;
            }
            const std::uint8_t addr = memory_[pc_++];
            call_stack_.push_back(pc_);
            pc_ = addr;
            break;
        }

        case 0x78: // RET
            if (call_stack_.empty())
            {
                stop_reason_ = StopReason::RetInstruction;
                return;
            }
            pc_ = call_stack_.back();
            call_stack_.pop_back();
            break;

        case 0x80: // XOR ACC, A
            acc_ = static_cast<std::uint8_t>(acc_ ^ a_);
            setFlags(acc_, false);
            break;

        case 0x88: // SLL ACC, A
        {
            const std::uint8_t shift_amount = static_cast<std::uint8_t>(a_ & 0x07U);
            bool carry = false;

            if (shift_amount != 0U)
            {
                carry = ((acc_ >> (8U - shift_amount)) & 0x01U) != 0U;
                acc_ = static_cast<std::uint8_t>(acc_ << shift_amount);
            }

            setFlags(acc_, carry);
            break;
        }

        case 0x90: // SRL ACC, A
        {
            const std::uint8_t shift_amount = static_cast<std::uint8_t>(a_ & 0x07U);
            bool carry = false;

            if (shift_amount != 0U)
            {
                carry = ((acc_ >> (shift_amount - 1U)) & 0x01U) != 0U;
                acc_ = static_cast<std::uint8_t>(acc_ >> shift_amount);
            }

            setFlags(acc_, carry);
            break;
        }

        case 0x98: // OR ACC, A
            acc_ = static_cast<std::uint8_t>(acc_ | a_);
            setFlags(acc_, false);
            break;

        case 0xA0: // SUB ACC, A
        {
            const bool borrow = acc_ < a_;
            acc_ = static_cast<std::uint8_t>(acc_ - a_);
            setFlags(acc_, borrow);
            break;
        }

        default:
            stop_reason_ = StopReason::UnknownOpcode;
            return;
        }
    }
}

PDUASimulator::StopReason PDUASimulator::stopReason() const noexcept
{
    return stop_reason_;
}

std::string PDUASimulator::stopReasonString() const
{
    switch (stop_reason_)
    {
    case StopReason::None:
        return "No stop reason recorded.";
    case StopReason::RetInstruction:
        return "RET";
    case StopReason::UnknownOpcode:
        return "Unknown opcode (likely data section)";
    case StopReason::ProgramCounterOutOfRange:
        return "PC out of memory range";
    case StopReason::StepLimitReached:
        return "Step limit reached";
    case StopReason::StackUnderflow:
        return "Stack underflow";
    default:
        return "Unknown";
    }
}

std::uint8_t PDUASimulator::acc() const noexcept {
    return acc_;
}
std::uint8_t PDUASimulator::a() const noexcept {
    return a_;
}
std::uint8_t PDUASimulator::dptr() const noexcept {
    return dptr_;
}
std::uint8_t PDUASimulator::pc() const noexcept {
    return pc_;
}

bool PDUASimulator::zeroFlag() const noexcept {
    return zero_flag_;
}
bool PDUASimulator::negativeFlag() const noexcept {
    return negative_flag_;
}
bool PDUASimulator::carryFlag() const noexcept {
    return carry_flag_;
}

std::uint8_t PDUASimulator::readMemory(std::uint8_t address) const noexcept
{
    return memory_[address];
}

std::size_t PDUASimulator::loadedProgramSize() const noexcept
{
    return loaded_program_size_;
}

void PDUASimulator::dumpState() const
{
    std::cout << "========== PDUA EXECUTION REPORT ==========\n";
    std::cout << "Stop reason : " << stopReasonString() << '\n';
    std::cout << "Last opcode : 0x"
              << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
              << static_cast<int>(last_opcode_) << '\n';
    std::cout << "Program size: " << std::dec << loaded_program_size_ << " byte(s)\n\n";

    std::cout << "Final registers\n";
    std::cout << "+------+------+\n";
    std::cout << "| ACC  | 0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(acc_) << " |\n";
    std::cout << "| A    | 0x" << std::setw(2) << static_cast<int>(a_) << " |\n";
    std::cout << "| DPTR | 0x" << std::setw(2) << static_cast<int>(dptr_) << " |\n";
    std::cout << "| PC   | 0x" << std::setw(2) << static_cast<int>(pc_) << " |\n";
    std::cout << "+------+------+\n\n";

    std::cout << "Flags\n";
    std::cout << "+---+---+---+\n";
    std::cout << "| Z | N | C |\n";
    std::cout << "+---+---+---+\n";
    std::cout << "| " << std::dec << zero_flag_
              << " | " << negative_flag_
              << " | " << carry_flag_ << " |\n";
    std::cout << "+---+---+---+\n";
}

void PDUASimulator::dumpMemory(std::uint8_t start, std::uint8_t end) const
{
    if (start > end)
    {
        std::swap(start, end);
    }

    const std::uint16_t aligned_start = static_cast<std::uint16_t>(start) & 0xF0U;
    const std::uint16_t aligned_end = static_cast<std::uint16_t>(end) | 0x0FU;

    std::cout << "\nMemory map\n";
    std::cout << "       ";
    for (int col = 0; col < 16; ++col)
    {
        std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << col << ' ';
    }
    std::cout << '\n';

    for (std::uint16_t row = aligned_start; row <= aligned_end && row < memory_.size(); row += 16)
    {
        std::cout << "0x"
                  << std::hex << std::uppercase
                  << std::setw(2) << std::setfill('0')
                  << row << " : ";

        for (std::uint16_t col = 0; col < 16; ++col)
        {
            const std::uint16_t addr = row + col;
            if (addr < memory_.size())
            {
                std::cout << std::setw(2)
                          << static_cast<int>(memory_[addr]) << ' ';
            }
        }
        std::cout << '\n';
    }
}

void PDUASimulator::dumpDataSection(std::uint8_t start, std::uint8_t count) const
{
    std::cout << "\nData section summary\n";
    std::cout << "+---------+----------+---------+\n";
    std::cout << "| Address | Hex      | Decimal |\n";
    std::cout << "+---------+----------+---------+\n";

    for (std::uint16_t i = 0; i < count; ++i)
    {
        const std::uint16_t addr = static_cast<std::uint16_t>(start) + i;
        if (addr >= memory_.size())
        {
            break;
        }

        const auto value = memory_[addr];

        std::cout << "| 0x"
                  << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
                  << addr
                  << "    | 0x"
                  << std::setw(2) << static_cast<int>(value)
                  << "     | "
                  << std::dec << std::setw(3) << static_cast<int>(value)
                  << "     |\n";
    }

    std::cout << "+---------+----------+---------+\n";
}
