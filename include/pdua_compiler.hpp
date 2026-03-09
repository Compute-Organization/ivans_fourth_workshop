#pragma once

#include "pdua_instruction.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class PDUACompiler
{
public:
    using SymbolTable = std::unordered_map<std::string, std::uint8_t>;

    struct AssemblyResult
    {
        std::vector<std::uint8_t> bytes;
        SymbolTable symbols;
    };

    PDUACompiler() = default;

    [[nodiscard]] std::vector<std::uint8_t> assemble(const std::string& source) const;
    [[nodiscard]] AssemblyResult assembleDetailed(const std::string& source) const;

    [[nodiscard]] static std::string formatBytes(const std::vector<std::uint8_t>& bytes);
    static void writeSymbolFile(const std::string& file_path, const SymbolTable& symbols);

private:
    enum class LineKind : std::uint8_t
    {
        Instruction,
        Data
    };

    struct ParsedLine
    {
        LineKind kind{LineKind::Instruction};
        std::size_t line_number{};
        std::string original_text{};
        std::string normalized_instruction{};
        std::string label{};
        const PDUAInstruction* instruction{nullptr};
        std::string operand_text{};
        std::uint8_t data_value{0};
        std::uint8_t address{0};
    };

    [[nodiscard]] std::vector<ParsedLine> firstPass(const std::string& source,
                                                    SymbolTable& symbols) const;

    [[nodiscard]] std::vector<std::uint8_t> secondPass(const std::vector<ParsedLine>& lines,
                                                       const SymbolTable& symbols) const;

    [[nodiscard]] static std::string stripComment(const std::string& line);
    [[nodiscard]] static std::string trim(const std::string& text);
    [[nodiscard]] static std::string extractOperandToken(const std::string& text);

    [[nodiscard]] static std::uint8_t parseByteValue(const std::string& text,
                                                     const SymbolTable& symbols,
                                                     std::size_t line_number);

    [[nodiscard]] static bool isDataDefinition(const std::string& text);
    [[nodiscard]] static const PDUAInstruction* findInstruction(const std::string& normalized_line);
};