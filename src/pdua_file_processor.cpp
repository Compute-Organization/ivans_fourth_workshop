#include "pdua_file_processor.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

bool PDUAFileProcessor::hasValidSourceExtension(const std::string& file_path)
{
    const std::string ext = getExtension(file_path);
    return ext == ".pdua" || ext == ".asm";
}

std::string PDUAFileProcessor::readSourceFile(const std::string& file_path)
{
    if (!hasValidSourceExtension(file_path))
    {
        throw std::runtime_error(
            "Invalid source file extension. Expected .pdua or .asm: " + file_path
        );
    }

    std::ifstream input(file_path);
    if (!input)
    {
        throw std::runtime_error("Could not open source file: " + file_path);
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();

    if (!input.good() && !input.eof())
    {
        throw std::runtime_error("Error while reading source file: " + file_path);
    }

    return buffer.str();
}

void PDUAFileProcessor::writeBinaryFile(const std::string& file_path,
                                        const std::vector<std::uint8_t>& bytes)
{
    std::ofstream output(file_path, std::ios::binary);
    if (!output)
    {
        throw std::runtime_error("Could not open output file: " + file_path);
    }

    output.write(reinterpret_cast<const char*>(bytes.data()),
                 static_cast<std::streamsize>(bytes.size()));

    if (!output)
    {
        throw std::runtime_error("Error while writing output file: " + file_path);
    }
}

std::string PDUAFileProcessor::replaceExtension(const std::string& file_path,
                                                const std::string& new_extension)
{
    std::filesystem::path path(file_path);
    path.replace_extension(new_extension);
    return path.string();
}

std::string PDUAFileProcessor::getExtension(const std::string& file_path)
{
    const std::filesystem::path path(file_path);
    return path.extension().string();
}