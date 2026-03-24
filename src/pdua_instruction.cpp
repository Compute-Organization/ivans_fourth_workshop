#include "pdua_instruction.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace
{
std::string trim(const std::string& text)
{
    const auto first = std::find_if_not(text.begin(), text.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });

    if (first == text.end())
    {
        return {};
    }

    const auto last = std::find_if_not(text.rbegin(), text.rend(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    }).base();

    return std::string(first, last);
}
} // namespace

PDUAInstruction::PDUAInstruction(std::byte opcode_base,
                                 std::byte opcode_mask,
                                 std::string signature,
                                 std::uint8_t size_bytes,
                                 bool affects_flags,
                                 OperandKind operand_kind)
    : opcode_base_(opcode_base),
      opcode_mask_(opcode_mask),
      signature_(normalizeAssemblyText(signature)),
      size_bytes_(size_bytes),
      affects_flags_(affects_flags),
      operand_kind_(operand_kind)
{
}

std::byte PDUAInstruction::opcodeBase() const noexcept
{
    return opcode_base_;
}

std::byte PDUAInstruction::opcodeMask() const noexcept
{
    return opcode_mask_;
}

const std::string& PDUAInstruction::signature() const noexcept
{
    return signature_;
}

std::uint8_t PDUAInstruction::sizeBytes() const noexcept
{
    return size_bytes_;
}

bool PDUAInstruction::affectsFlags() const noexcept
{
    return affects_flags_;
}

OperandKind PDUAInstruction::operandKind() const noexcept
{
    return operand_kind_;
}

bool PDUAInstruction::isExactSignature() const noexcept
{
    return operand_kind_ == OperandKind::None;
}

std::string PDUAInstruction::operandPrefix() const
{
    if (operand_kind_ == OperandKind::None)
    {
        return signature_;
    }

    const auto pos_cte = signature_.find("CTE");
    const auto pos_dir = signature_.find("DIR");

    std::size_t pos = std::string::npos;
    if (pos_cte != std::string::npos)
    {
        pos = pos_cte;
    }
    else if (pos_dir != std::string::npos)
    {
        pos = pos_dir;
    }

    if (pos == std::string::npos)
    {
        throw std::runtime_error("Instruction signature does not contain operand placeholder.");
    }

    return trim(signature_.substr(0, pos));
}

bool PDUAInstruction::matchesNormalized(const std::string& normalized_line) const
{
    if (operand_kind_ == OperandKind::None)
    {
        return normalized_line == signature_;
    }

    const std::string prefix = operandPrefix();

    if (normalized_line.size() <= prefix.size())
    {
        return false;
    }

    if (normalized_line.compare(0, prefix.size(), prefix) != 0)
    {
        return false;
    }

    if (normalized_line[prefix.size()] != ' ')
    {
        return false;
    }

    return true;
}

std::ostream& operator<<(std::ostream& os, const PDUAInstruction& instruction)
{
    os << "{opcode_base="
       << std::to_integer<int>(instruction.opcodeBase())
       << ", mask="
       << std::to_integer<int>(instruction.opcodeMask())
       << ", signature=\"" << instruction.signature()
       << "\", bytes=" << static_cast<int>(instruction.sizeBytes())
       << ", flags=" << instruction.affectsFlags()
       << "}";

    return os;
}

std::string normalizeAssemblyText(const std::string& text)
{
    std::string out;
    out.reserve(text.size());

    bool previous_was_space = false;

    for (char ch : text)
    {
        const auto uch = static_cast<unsigned char>(ch);

        if (std::isspace(uch) != 0)
        {
            if (!previous_was_space)
            {
                out.push_back(' ');
                previous_was_space = true;
            }
            continue;
        }

        if (ch == ',')
        {
            while (!out.empty() && out.back() == ' ')
            {
                out.pop_back();
            }
            out.push_back(',');
            out.push_back(' ');
            previous_was_space = true;
            continue;
        }

        out.push_back(static_cast<char>(std::toupper(uch)));
        previous_was_space = false;
    }

    return trim(out);
}

const std::vector<PDUAInstruction>& basicInstructionSet()
{
    static const std::vector<PDUAInstruction> instructions = {
        PDUAInstruction{std::byte{0x08}, std::byte{0xF8}, "MOV ACC, A",      1, true,  OperandKind::None},
        PDUAInstruction{std::byte{0x10}, std::byte{0xF8}, "MOV A, ACC",      1, true,  OperandKind::None},
        PDUAInstruction{std::byte{0x18}, std::byte{0xF8}, "MOV ACC, CTE",    2, false, OperandKind::Immediate8},
        PDUAInstruction{std::byte{0x20}, std::byte{0xF8}, "MOV ACC, [DPTR]", 1, false, OperandKind::None},
        PDUAInstruction{std::byte{0x28}, std::byte{0xF8}, "MOV DPTR, ACC",   1, true,  OperandKind::None},
        PDUAInstruction{std::byte{0x30}, std::byte{0xF8}, "MOV [DPTR], ACC", 1, true,  OperandKind::None},
        PDUAInstruction{std::byte{0x38}, std::byte{0xF8}, "INV ACC",         1, true,  OperandKind::None},
        PDUAInstruction{std::byte{0x40}, std::byte{0xF8}, "AND ACC, A",      1, true,  OperandKind::None},
        PDUAInstruction{std::byte{0x48}, std::byte{0xF8}, "ADD ACC, A",      1, true,  OperandKind::None},
        PDUAInstruction{std::byte{0x50}, std::byte{0xF8}, "JMP CTE",         2, false, OperandKind::Address8},
        PDUAInstruction{std::byte{0x58}, std::byte{0xF8}, "JZ CTE",          2, false, OperandKind::Address8},
        PDUAInstruction{std::byte{0x60}, std::byte{0xF8}, "JN CTE",          2, false, OperandKind::Address8},
        PDUAInstruction{std::byte{0x68}, std::byte{0xF8}, "JC CTE",          2, false, OperandKind::Address8},
        PDUAInstruction{std::byte{0x70}, std::byte{0xF8}, "CALL DIR",        2, false, OperandKind::Address8},
        PDUAInstruction{std::byte{0x70}, std::byte{0xF8}, "CALL CTE",        2, false, OperandKind::Address8},
        PDUAInstruction{std::byte{0x78}, std::byte{0xF8}, "RET",             1, false, OperandKind::None},

        // Instrucciones pedagógicas agregadas en el rango disponible 1xxxx---
        PDUAInstruction{std::byte{0x80}, std::byte{0xF8}, "XOR ACC, A",      1, true,  OperandKind::None},
        PDUAInstruction{std::byte{0x88}, std::byte{0xF8}, "SLL ACC, A",      1, true,  OperandKind::None},
        PDUAInstruction{std::byte{0x90}, std::byte{0xF8}, "SRL ACC, A",      1, true,  OperandKind::None},
        PDUAInstruction{std::byte{0x90}, std::byte{0xF8}, "SLR ACC, A",      1, true,  OperandKind::None},
        PDUAInstruction{std::byte{0x98}, std::byte{0xF8}, "OR ACC, A",       1, true,  OperandKind::None},
        PDUAInstruction{std::byte{0xA0}, std::byte{0xF8}, "SUB ACC, A",      1, true,  OperandKind::None},
    };

    return instructions;
}
