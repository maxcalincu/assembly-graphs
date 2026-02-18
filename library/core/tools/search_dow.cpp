#include <core/set_builders.h>
#include <core/generators.h>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char *argv[]) {


    size_t n, k, min_sub_word;
    std::string k_string, pattern;
    char comparison_symbol;

    std::map<char, std::function<bool(size_t, size_t)>> compare_functions{{
        {'=', [](size_t x, size_t y) { return x == y; }},
        {'g', [](size_t x, size_t y) { return x > y;  }},
        {'l', [](size_t x, size_t y) { return x < y;  }},
    }};

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Show this help message\n")

        ("size,n", 
            po::value<size_t>(&n)
                ->required(),
                "desired d.o. word size\n")
        ("assembly-number,k", 
            po::value<std::string>(&k_string)
                ->required(),
                "comparison operator followed by a number\n"
                "allowed operators: ['g', 'l', '=']\n"
                "'g' stands for greater and 'l' for less\n"
        )
        ("pattern,p", 
            po::value<std::string>(&pattern)
                ->default_value(""),
                "the pattern the d.o. word has to match\n"
                "   1: a first appearance of a letter\n"
                "   2: a second appearance of a letter\n"
                "   ?: either option\n"
        )
        ("min-sub-word,m", 
            po::value<size_t>(&min_sub_word)
                ->default_value(1),
                "the minimal size of a sub-word that is also a d.o. word");
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
              std::cout << "Examples:\n";
              std::cout << argv[0] << " 7 =3\n";
              std::cout << argv[0] << " -n 7 -k g1 --pattern 1122?1122?1122 --min-sub-word 2\n";
              std::cout << argv[0] << " -n 7 -k l2 --pattern 11?11?12222??? --min-sub-word 7\n";
              std::cout << "\
Search for all double-occurrence words in ascending order that satisfy several constraints: word size, assembly-number, pattern, size of min. sub-word that is also a d. o. word\n";
              return 0;
        }
        po::notify(vm);
        
        comparison_symbol = k_string.at(0);
        if (!compare_functions.contains(comparison_symbol)) {
            throw std::runtime_error("unknown comparison operator");
        }

        k = std::stoul(k_string.substr(1));
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    FilteredSAGGenerator generator(n, min_sub_word, pattern);
    auto compare = compare_functions.at(comparison_symbol); 

    int words_found = 0;
    SAGWithEndpoints graph(0, {});

    while (generator.Yield(graph)) {
        if (compare(An(graph), k)) {
            std::cout << graph.ConvertToString(",") << "\n";
            ++words_found;
        }
    }

    std::cout << "words found: " << words_found << "\n"; 
    return 0;
}
