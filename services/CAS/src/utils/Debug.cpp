#include "Debug.h"
#include "Config.h"

std::string Debug::padRight(const std::string &s, size_t width) {
    if (s.size() >= width) return s.substr(0, width);
    return s + std::string(width - s.size(), ' ');
};

bool Debug::executeSteps(std::unique_ptr<ASTNode>& node, bool debug, std::vector<Table::Step>& steps, std::string name) {
    int iterations = 0;
    bool changed = false;

    std::vector<Table::Col> cols = {
        {"Step", 40},
        {"Changed", 7},
        {"Node changed", 50},
    };

    do {
        if (iterations > Config::MAX_ITERATIONS_CONVERGE_SOLVE){
            dbg(node->toString());
            throw std::runtime_error(name + " did not converge after maximum iterations.");
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
            std::string nameField = Debug::padRight(s.name, cols[0].width);

            std::string changedRaw = s.result ? "true" : "false";
            std::string changedPadded = Debug::padRight(changedRaw, cols[1].width);
            std::string changedColored = s.result
                ? std::string(Color::GREEN) + changedPadded + Color::RESET
                : std::string(Color::RED)   + changedPadded + Color::RESET;

            std::string nodeField = Debug::padRight(s.nodeStrAfter, cols[2].width);

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

    return iterations > 1;
}
