#include "pdua_file_processor.hpp"
#include "pdua_instruction.hpp"
#include "pdua_simulator.hpp"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
std::map<std::string, std::uint8_t> readSymbolFile(const std::string& path)
{
    std::ifstream input(path);
    if (!input)
    {
        return {};
    }

    std::map<std::string, std::uint8_t> symbols;
    std::string name;
    std::string address_token;

    while (input >> name >> address_token)
    {
        std::size_t idx = 0U;
        unsigned long value = 0UL;

        if (address_token.size() > 2U &&
            address_token[0] == '0' &&
            (address_token[1] == 'x' || address_token[1] == 'X'))
        {
            value = std::stoul(address_token, &idx, 16);
        }
        else
        {
            value = std::stoul(address_token, &idx, 10);
        }

        if (idx == address_token.size() && value <= 0xFFUL)
        {
            symbols[name] = static_cast<std::uint8_t>(value);
        }
    }

    return symbols;
}

std::vector<std::string> readDataLabelsFromSource(const std::string& source_path)
{
    std::ifstream input(source_path);
    if (!input)
    {
        return {};
    }

    static const std::regex data_definition_pattern(
        R"(^\s*([A-Za-z_][A-Za-z0-9_]*)\s*:\s*(0x[0-9A-Fa-f]+)\s*$)"
    );

    std::vector<std::string> labels;
    std::string line;

    while (std::getline(input, line))
    {
        const auto hash_pos = line.find('#');
        const auto semicolon_pos = line.find(';');
        const auto at_pos = line.find('@');

        std::size_t cut_pos = std::string::npos;

        if (hash_pos != std::string::npos)
        {
            cut_pos = hash_pos;
        }
        if (semicolon_pos != std::string::npos)
        {
            cut_pos = (cut_pos == std::string::npos)
                        ? semicolon_pos
                        : std::min(cut_pos, semicolon_pos);
        }
        if (at_pos != std::string::npos)
        {
            cut_pos = (cut_pos == std::string::npos)
                        ? at_pos
                        : std::min(cut_pos, at_pos);
        }

        if (cut_pos != std::string::npos)
        {
            line = line.substr(0, cut_pos);
        }

        std::smatch match;
        if (std::regex_match(line, match, data_definition_pattern))
        {
            labels.push_back(normalizeAssemblyText(match[1].str()));
        }
    }

    return labels;
}

void printUsefulDataValues(const PDUASimulator& simulator,
                           const std::map<std::string, std::uint8_t>& symbols,
                           const std::vector<std::string>& data_labels)
{
    if (symbols.empty() || data_labels.empty())
    {
        return;
    }

    std::vector<std::pair<std::string, std::uint8_t>> resolved;
    resolved.reserve(data_labels.size());

    for (const auto& label : data_labels)
    {
        const auto it = symbols.find(label);
        if (it != symbols.end())
        {
            resolved.push_back(*it);
        }
    }

    if (resolved.empty())
    {
        return;
    }

    std::sort(resolved.begin(), resolved.end(),
              [](const auto& lhs, const auto& rhs) {
                  if (lhs.second != rhs.second)
                  {
                      return lhs.second < rhs.second;
                  }
                  return lhs.first < rhs.first;
              });

    std::cout << "\nUseful data values\n";

    for (const auto& [name, address] : resolved)
    {
        const auto value = simulator.readMemory(address);

        std::cout << std::left << std::setw(8) << name
                  << "= 0x"
                  << std::hex << std::uppercase
                  << std::setw(2) << std::setfill('0')
                  << static_cast<int>(value)
                  << std::setfill(' ')
                  << " (" << std::dec << static_cast<int>(value) << ")"
                  << " at address 0x"
                  << std::hex << std::uppercase
                  << std::setw(2) << static_cast<int>(address)
                  << std::dec << '\n';
    }

    std::cout << std::right << std::setfill(' ');
}
} // namespace

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: " << argv[0] << " <program.bin>\n";
            return 1;
        }

        const std::string input_path = argv[1];
        const std::string symbol_path =
            PDUAFileProcessor::replaceExtension(input_path, ".sym");
        const std::string source_path =
            PDUAFileProcessor::replaceExtension(input_path, ".pdua");

        std::ifstream input(input_path, std::ios::binary);
        if (!input)
        {
            throw std::runtime_error("Could not open binary file: " + input_path);
        }

        std::vector<std::uint8_t> program(
            (std::istreambuf_iterator<char>(input)),
            std::istreambuf_iterator<char>());

        const auto symbols = readSymbolFile(symbol_path);
        const auto data_labels = readDataLabelsFromSource(source_path);

        PDUASimulator simulator;
        simulator.loadProgram(program);
        simulator.run();
        simulator.dumpState();

        std::cout << "\nResult interpretation\n";
        std::cout << "Final A register   = 0x"
                  << std::hex << std::uppercase
                  << static_cast<int>(simulator.a())
                  << " (" << std::dec << static_cast<int>(simulator.a()) << ")\n";

        std::cout << "Final ACC register = 0x"
                  << std::hex << std::uppercase
                  << static_cast<int>(simulator.acc())
                  << " (" << std::dec << static_cast<int>(simulator.acc()) << ")\n";

        printUsefulDataValues(simulator, symbols, data_labels);
        simulator.dumpMemory(0x00, 0xF0);

        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
}