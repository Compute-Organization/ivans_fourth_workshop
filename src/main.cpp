#include "pdua_compiler.hpp"
#include "pdua_file_processor.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{
void printUsage(const char* program_name)
{
    std::cout
        << "Usage:\n"
        << "  " << program_name << " <source.pdua>\n"
        << "  " << program_name << " <source.pdua> <output.bin>\n\n"
        << "Accepted source extensions:\n"
        << "  .pdua\n"
        << "  .asm\n";
}
} // namespace

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2 && argc != 3)
        {
            printUsage(argv[0]);
            return 1;
        }

        const std::string input_path = argv[1];

        if (!PDUAFileProcessor::hasValidSourceExtension(input_path))
        {
            throw std::runtime_error(
                "Invalid source file extension. Expected .pdua or .asm: " + input_path
            );
        }

        const std::string source = PDUAFileProcessor::readSourceFile(input_path);

        PDUACompiler compiler;
        const auto result = compiler.assembleDetailed(source);

        std::cout << "Assembled " << result.bytes.size() << " byte(s):\n";
        std::cout << PDUACompiler::formatBytes(result.bytes) << '\n';

        std::string output_path;
        if (argc == 3)
        {
            output_path = argv[2];
        }
        else
        {
            output_path = PDUAFileProcessor::replaceExtension(input_path, ".bin");
        }

        PDUAFileProcessor::writeBinaryFile(output_path, result.bytes);

        const std::string symbol_path =
            PDUAFileProcessor::replaceExtension(output_path, ".sym");
        PDUACompiler::writeSymbolFile(symbol_path, result.symbols);

        std::cout << "Binary written to: " << output_path << '\n';
        std::cout << "Symbols written to: " << symbol_path << '\n';

        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
}