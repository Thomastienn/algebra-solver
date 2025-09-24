#include "Debug.h"
#include "Config.h"

std::string Debug::padRight(const std::string &s, size_t width) {
    if (s.size() >= width) return s.substr(0, width);
    return s + std::string(width - s.size(), ' ');
};

void Debug::executeSteps(std::unique_ptr<ASTNode>& node, bool debug, std::vector<Table::Step>& steps, std::vector<Table::Col> cols) {
    int iterations = 0;
    bool changed = false;
    do {
        if (iterations > Config::MAX_ITERATIONS){
            throw std::runtime_error("Simplification did not converge after maximum iterations.");
        }
        changed = false;

        // The order matter performance (or even correctness)
        // So be careful when changing the order

        // aggregate overall change
        for (auto &s : steps) {
            s.result = s.func();
            changed |= s.result;
            s.nodeStrAfter = node->toString();
        }

        const size_t nameWidth    = 40;
        const size_t changedWidth = 7;
        const size_t nodeStrWidth = 50;

        // build separator like: +-----+------+------+
        std::string separator = "+";
        for (auto &c : cols) {
            separator += std::string(c.width + 2, '-') + "+";
        }
        separator += "\n";

        // build header row: | Title ... |
        std::string header = "|";
        for (auto &c : cols) {
            header += " " + Debug::padRight(c.title, c.width) + " |";
        }
        header += "\n";

        std::ostringstream out;
        out << "Iteration " << iterations << ":\n";
        out << separator;
        out << header;
        out << separator;

        // rows
        for (auto &s : steps) {
            std::string nameField = Debug::padRight(s.name, nameWidth);

            std::string changedRaw = s.result ? "true" : "false";
            std::string changedPadded = Debug::padRight(changedRaw, changedWidth);
            std::string changedColored = s.result
                ? std::string(Color::GREEN) + changedPadded + Color::RESET
                : std::string(Color::RED)   + changedPadded + Color::RESET;

            std::string nodeField = Debug::padRight(s.nodeStrAfter, nodeStrWidth);

            out << "| " << nameField
                << " | " << changedColored
                << " | " << nodeField << " |\n";
        }

        out << separator;
        out << "Result: " << node->toString() << "\n";

        std::string table = out.str();


        if (debug) dbg(table);  // send the table to your debug macro
        iterations++;

    } while (changed);
}
