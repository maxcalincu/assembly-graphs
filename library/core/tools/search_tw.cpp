#include "core/sag_with_endpoints.h"
#include <core/set_builders.h>
#include <core/generators.h>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char *argv[]) {


    size_t n, k;
    bool exclude_loops;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Show this help message\n")

        ("size,n", 
            po::value<size_t>(&n)
                ->required(),
                "desired double-occurence word size\n")
        ("assembly-number,k", 
            po::value<size_t>(&k)
                ->required(),
                "desired assembly number\n")
        ("exclude-loops", po::bool_switch(&exclude_loops)
            ->default_value(false), 
            "exclude words which contain loops");
    try {
        po::positional_options_description p;
        p.add("size", 1);
        p.add("assembly-number", 1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv)
            .options(desc)
            .positional(p)
            .run(), vm);

        if (vm.count("help")) {
              std::cout << "Usage: " << argv[0] << " [options] <size> <assembly-number> \n";
              std::cout << desc << "\n";
              std::cout << "Example: " << argv[0] << " --exclude-loops 8 2\n";
              std::cout << "\
Search for all normalized double-occurence words of given size and assembly number\n";
              return 0;
        }
        po::notify(vm);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    const SAGGenerator<SAGWithEndpoints> generator;
    int words_found = 0;
    std::cout << "SIZE = " << n << " An = " << k << "\nEXCLUDE LOOPS: " << (exclude_loops ? "true" : "false") << "\n";
    auto graph = generator.GetLexicographicallySmallest(0, n);
    do {
        if ((exclude_loops && graph.HasLoop()) || GetNumberOfPaths(graph, SymGreedySetBuilder()) < k) {
            continue;
        }
        if (An(graph) == k) {
            std::cout << graph.ConvertToString(",") << "\n";
            ++words_found;
        }
    } while (generator.Advance(graph));

    std::cout << "words found: " << words_found << "\n"; 
    return 0;
}
