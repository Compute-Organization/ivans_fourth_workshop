#pragma once

#include <cstdint>
#include <string>
#include <vector>

class PDUAFileProcessor
{
public:
    [[nodiscard]] static bool hasValidSourceExtension(const std::string& file_path);
    [[nodiscard]] static std::string readSourceFile(const std::string& file_path);
    [[nodiscard]] static std::string replaceExtension(const std::string& file_path,
                                                      const std::string& new_extension);

    static void writeBinaryFile(const std::string& file_path,
                                const std::vector<std::uint8_t>& bytes);

private:
    [[nodiscard]] static std::string getExtension(const std::string& file_path);
};