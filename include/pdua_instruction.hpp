#pragma once

#include <cstddef>
#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

enum class OperandKind : std::uint8_t
{
    None,
    Immediate8,
    Address8
};

class PDUAInstruction
{
public:
    PDUAInstruction() = default;

    PDUAInstruction(std::byte opcode_base,
                    std::byte opcode_mask,
                    std::string signature,
                    std::uint8_t size_bytes,
                    bool affects_flags,
                    OperandKind operand_kind);

    [[nodiscard]] std::byte opcodeBase() const noexcept;
    [[nodiscard]] std::byte opcodeMask() const noexcept;
    [[nodiscard]] const std::string& signature() const noexcept;
    [[nodiscard]] std::uint8_t sizeBytes() const noexcept;
    [[nodiscard]] bool affectsFlags() const noexcept;
    [[nodiscard]] OperandKind operandKind() const noexcept;

    [[nodiscard]] bool isExactSignature() const noexcept;
    [[nodiscard]] bool matchesNormalized(const std::string& normalized_line) const;
    [[nodiscard]] std::string operandPrefix() const;

    friend std::ostream& operator<<(std::ostream& os, const PDUAInstruction& instruction);

private:
    std::byte opcode_base_{};
    std::byte opcode_mask_{};
    std::string signature_{};
    std::uint8_t size_bytes_{0};
    bool affects_flags_{false};
    OperandKind operand_kind_{OperandKind::None};
};

[[nodiscard]] std::string normalizeAssemblyText(const std::string& text);
[[nodiscard]] const std::vector<PDUAInstruction>& basicInstructionSet();