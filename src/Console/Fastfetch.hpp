#include <iostream>
#include <vector>
#include <string>
#include <iomanip>

void fastfetch(const std::vector<std::string>& info) {
    const std::string RESET    = "\033[0m";
    const std::string MAGENTA  = "\033[38;5;201m";
    const std::string PURPLE   = "\033[38;5;93m"; 
    const std::string WHITE    = "\033[38;5;15m"; 

    std::vector<std::string> logo = {
        "          .-````-.",
        "       .-'          '-.",
        "     .'                '.",
        "    /          *         \\",
        "   |         * * *        |",
        "   |       *   *   *      |",
        "   |         * * *        |",
        "    \\          *         /",
        "     '.                .'",
        "      |'-.          .-'",
        "      |   '-.____.-'",
        "      |  /",
        "      | / ",
        "      |/  "
    };

    size_t logo_width = 30;
    size_t info_width = 32;
    size_t total_lines = std::max(logo.size(), info.size());

    for (size_t i = 0; i < total_lines; ++i) {
        std::string logo_line = (i < logo.size()) ? logo[i] : "";
        std::string info_line = (i < info.size()) ? info[i] : "";

        std::cout << MAGENTA << std::setw(logo_width) << std::left << logo_line;
        std::cout << "  ";
        std::cout << WHITE << std::setw(info_width) << std::left << info_line;
        std::cout << RESET << std::endl;
    }

    std::cout << std::setw(logo_width + 2 + info_width) << ' ' << RESET << std::endl;
}