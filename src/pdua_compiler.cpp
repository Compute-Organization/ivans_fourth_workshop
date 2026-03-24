#include "pdua_compiler.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <unordered_set>
#include <vector>

std::string PDUACompiler::trim(const std::string& text)
{
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
    {
        return {};
    }

    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1U);
}

std::vector<std::uint8_t> PDUACompiler::assemble(const std::string& source) const
{
    return assembleDetailed(source).bytes;
}

PDUACompiler::AssemblyResult PDUACompiler::assembleDetailed(const std::string& source) const
{
    SymbolTable symbols;
    const auto parsed_lines = firstPass(source, symbols);

    AssemblyResult result;
    result.bytes = secondPass(parsed_lines, symbols);
    result.symbols = std::move(symbols);

    return result;
}

std::string PDUACompiler::formatBytes(const std::vector<std::uint8_t>& bytes)
{
    std::ostringstream oss;

    for (std::size_t i = 0; i < bytes.size(); ++i)
    {
        if (i != 0U)
        {
            oss << ' ';
        }

        oss << "0x"
            << std::uppercase
            << std::hex
            << std::setw(2)
            << std::setfill('0')
            << static_cast<int>(bytes[i]);
    }

    return oss.str();
}

void PDUACompiler::writeSymbolFile(const std::string& file_path, const SymbolTable& symbols)
{
    std::vector<std::pair<std::string, std::uint8_t>> ordered(symbols.begin(), symbols.end());

    std::sort(ordered.begin(), ordered.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.second != rhs.second)
        {
            return lhs.second < rhs.second;
        }
        return lhs.first < rhs.first;
    });

    std::ofstream output(file_path);
    if (!output)
    {
        throw std::runtime_error("Could not open symbol file for writing: " + file_path);
    }

    for (const auto& [name, address] : ordered)
    {
        output << name << " 0x"
               << std::hex
               << std::uppercase
               << std::setw(2)
               << std::setfill('0')
               << static_cast<int>(address)
               << '\n';
    }

    if (!output)
    {
        throw std::runtime_error("Could not write symbol file: " + file_path);
    }
}

std::string PDUACompiler::stripComment(const std::string& line)
{
    std::size_t cut_pos = std::string::npos;

    for (const char marker : {
                '#', ';', '@'
            })
    {
        const auto pos = line.find(marker);
        if (pos != std::string::npos)
        {
            cut_pos = (cut_pos == std::string::npos) ? pos : std::min(cut_pos, pos);
        }
    }

    if (cut_pos == std::string::npos)
    {
        return line;
    }

    return line.substr(0, cut_pos);
}

std::string PDUACompiler::extractOperandToken(const std::string& text)
{
    std::string cleaned = trim(text);

    const auto paren_pos = cleaned.find('(');
    if (paren_pos != std::string::npos)
    {
        cleaned = trim(cleaned.substr(0, paren_pos));
    }

    std::istringstream iss(cleaned);
    std::string token;
    iss >> token;
    return token;
}

std::uint8_t PDUACompiler::parseByteValue(const std::string& text,
        const SymbolTable& symbols,
        std::size_t line_number)
{
    const std::string token = normalizeAssemblyText(extractOperandToken(text));

    if (token.empty())
    {
        throw std::runtime_error(
            "Line " + std::to_string(line_number) + ": missing operand value."
        );
    }

    const auto symbol_it = symbols.find(token);
    if (symbol_it != symbols.end())
    {
        return symbol_it->second;
    }

    try
    {
        std::size_t idx = 0U;
        unsigned long value = 0UL;

        if (token.size() > 2U && token[0] == '0' && token[1] == 'X')
        {
            value = std::stoul(token, &idx, 16);
        }
        else if (token.size() > 2U && token[0] == '0' && token[1] == 'B')
        {
            value = std::stoul(token.substr(2), &idx, 2);
            idx += 2U;
        }
        else
        {
            value = std::stoul(token, &idx, 10);
        }

        if (idx != token.size())
        {
            throw std::runtime_error("Invalid numeric token.");
        }

        if (value > 0xFFUL)
        {
            throw std::runtime_error("Value out of 8-bit range.");
        }

        return static_cast<std::uint8_t>(value);
    }
    catch (const std::exception&)
    {
        throw std::runtime_error(
            "Line " + std::to_string(line_number) +
            ": invalid 8-bit value '" + text + "'."
        );
    }
}

bool PDUACompiler::isDataDefinition(const std::string& text)
{
    const std::string normalized = normalizeAssemblyText(text);
    if (normalized.empty())
    {
        return false;
    }

    return findInstruction(normalized) == nullptr;
}

const PDUAInstruction* PDUACompiler::findInstruction(const std::string& normalized_line)
{
    const auto& instructions = basicInstructionSet();

    for (const auto& instruction : instructions)
    {
        if (instruction.operandKind() == OperandKind::None &&
                instruction.matchesNormalized(normalized_line))
        {
            return &instruction;
        }
    }

    for (const auto& instruction : instructions)
    {
        if (instruction.operandKind() != OperandKind::None &&
                instruction.matchesNormalized(normalized_line))
        {
            return &instruction;
        }
    }

    return nullptr;
}

std::vector<PDUACompiler::ParsedLine> PDUACompiler::firstPass(const std::string& source,
        SymbolTable& symbols) const
{
    std::vector<ParsedLine> code_lines;
    std::vector<ParsedLine> data_lines;
    std::unordered_set<std::string> all_labels;

    std::istringstream input(source);
    std::string raw_line;
    std::size_t line_number = 0U;
    std::uint16_t current_code_address = 0U;

    bool waiting_operand = false;
    ParsedLine pending_instruction{};

    auto register_label = [&](const std::string& label, std::uint8_t address, std::size_t line) {
        if (label.empty())
        {
            return;
        }

        if (!all_labels.insert(label).second)
        {
            throw std::runtime_error(
                "Line " + std::to_string(line) + ": duplicated label '" + label + "'."
            );
        }

        symbols[label] = address;
    };

    while (std::getline(input, raw_line))
    {
        ++line_number;

        const std::string without_comment = trim(stripComment(raw_line));
        if (without_comment.empty())
        {
            continue;
        }

        if (waiting_operand)
        {
            const std::string operand_line = trim(without_comment);
            if (operand_line.find(':') != std::string::npos)
            {
                throw std::runtime_error(
                    "Line " + std::to_string(line_number) +
                    ": an operand line cannot define labels or variables."
                );
            }

            pending_instruction.operand_text = operand_line;
            code_lines.push_back(pending_instruction);
            current_code_address += pending_instruction.instruction->sizeBytes();

            if (current_code_address > 0x100U)
            {
                throw std::runtime_error(
                    "Program exceeds PDUA 8-bit address space at line " +
                    std::to_string(line_number) + "."
                );
            }

            waiting_operand = false;
            pending_instruction = {};
            continue;
        }

        std::string working = without_comment;
        std::string label;

        const auto colon_pos = working.find(':');
        if (colon_pos != std::string::npos)
        {
            label = normalizeAssemblyText(trim(working.substr(0, colon_pos)));
            if (label.empty())
            {
                throw std::runtime_error(
                    "Line " + std::to_string(line_number) + ": empty label."
                );
            }

            working = trim(working.substr(colon_pos + 1U));
        }

        if (working.empty())
        {
            register_label(label, static_cast<std::uint8_t>(current_code_address), line_number);
            continue;
        }

        const std::string normalized = normalizeAssemblyText(working);
        const PDUAInstruction* instruction = findInstruction(normalized);

        if (instruction != nullptr)
        {
            register_label(label, static_cast<std::uint8_t>(current_code_address), line_number);

            ParsedLine parsed;
            parsed.kind = LineKind::Instruction;
            parsed.line_number = line_number;
            parsed.original_text = raw_line;
            parsed.normalized_instruction = normalized;
            parsed.label = label;
            parsed.instruction = instruction;

            if (instruction->operandKind() == OperandKind::None)
            {
                code_lines.push_back(parsed);
                current_code_address += instruction->sizeBytes();
            }
            else
            {
                waiting_operand = true;
                pending_instruction = parsed;
            }

            if (current_code_address > 0x100U)
            {
                throw std::runtime_error(
                    "Program exceeds PDUA 8-bit address space at line " +
                    std::to_string(line_number) + "."
                );
            }

            continue;
        }

        if (!label.empty() && isDataDefinition(working))
        {
            if (!all_labels.insert(label).second)
            {
                throw std::runtime_error(
                    "Line " + std::to_string(line_number) + ": duplicated label '" + label + "'."
                );
            }

            ParsedLine parsed;
            parsed.kind = LineKind::Data;
            parsed.line_number = line_number;
            parsed.original_text = raw_line;
            parsed.label = label;
            parsed.operand_text = working;
            data_lines.push_back(parsed);
            continue;
        }

        throw std::runtime_error(
            "Line " + std::to_string(line_number) +
            ": unknown statement '" + without_comment + "'."
        );
    }

    if (waiting_operand)
    {
        throw std::runtime_error(
            "Unexpected end of file: missing operand for instruction '" +
            pending_instruction.normalized_instruction + "'."
        );
    }

    std::uint16_t current_data_address = current_code_address;
    for (auto& data_line : data_lines)
    {
        if (current_data_address > 0xFFU)
        {
            throw std::runtime_error("Program exceeds PDUA 8-bit address space in data section.");
        }

        symbols[data_line.label] = static_cast<std::uint8_t>(current_data_address++);
    }

    for (auto& data_line : data_lines)
    {
        data_line.data_value = parseByteValue(data_line.operand_text, symbols, data_line.line_number);
    }

    std::vector<ParsedLine> ordered_lines;
    ordered_lines.reserve(code_lines.size() + data_lines.size());
    ordered_lines.insert(ordered_lines.end(), code_lines.begin(), code_lines.end());
    ordered_lines.insert(ordered_lines.end(), data_lines.begin(), data_lines.end());
    return ordered_lines;
}

std::vector<std::uint8_t> PDUACompiler::secondPass(const std::vector<ParsedLine>& lines,
        const SymbolTable& symbols) const
{
    std::vector<std::uint8_t> output;
    output.reserve(lines.size() * 2U);

    for (const auto& line : lines)
    {
        if (line.kind == LineKind::Data)
        {
            output.push_back(line.data_value);
            continue;
        }

        if (line.instruction == nullptr)
        {
            continue;
        }

        output.push_back(
            static_cast<std::uint8_t>(
                std::to_integer<unsigned int>(line.instruction->opcodeBase())
            )
        );

        if (line.instruction->operandKind() != OperandKind::None)
        {
            output.push_back(
                parseByteValue(line.operand_text, symbols, line.line_number)
            );
        }
    }

    return output;
}
