#include <core/set_builders.h>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char *argv[]) {

    std::string algorithm, two_word_string;
    std::vector<size_t> two_word;

    const std::vector<std::string> allowed_algorithms = {"exact", "greedy", "sym"};


    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Show this help message\n")

        ("algorithm,a", 
            po::value<std::string>(&algorithm)
                ->default_value("exact"), 
                "algorithm type: greedy, sym, exact\n")
        ("two-word", po::value<std::string>(&two_word_string)->required(), 
            "double-occurrence word represented by a comma-separated list of integers");
    try {
        po::positional_options_description p;
        p.add("two-word", 1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv)
            .options(desc)
            .positional(p)
            .run(), vm);

        if (vm.count("help")) {
              std::cout << "Usage: " << argv[0] << " [options] <double-occurrence word> \n";
              std::cout << desc << "\n";
              std::cout << "Example: " << argv[0] << " --algorithm exact 1,2,1,2,3,3\n";
              std::cout << "\
Construct a Hamiltonian set of polygonal paths for a given double-occurrence word\n";
              return 0;
        }
        po::notify(vm);

        if (std::find(allowed_algorithms.begin(), allowed_algorithms.end(), algorithm) 
            == allowed_algorithms.end()) {
            std::cerr << "Error: Invalid algorithm '" << algorithm << "'.\n";
            return 1;
        }

        std::string token;
        std::istringstream token_stream(two_word_string);
        
        while (std::getline(token_stream, token, ',')) {
            try {
                two_word.push_back(std::stoul(token));
            } catch (const std::invalid_argument&) {
                std::cerr << "Error: Invalid integer in list: '" << token << "'\n";
                return 1;
            } catch (const std::out_of_range&) {
                std::cerr << "Error: Integer out of range: '" << token << "'\n";
                return 1;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
  
    SAGWithEndpoints graph(0, two_word);
    std::unique_ptr<ISetBuilder> sb = [](const std::string& algorithm)-> std::unique_ptr<ISetBuilder> {
        if (algorithm == "greedy") {
            return std::make_unique<GreedySetBuilder>();
        }
        if (algorithm == "sym") {
            return std::make_unique<SymGreedySetBuilder>();
        }
        return std::make_unique<ExactSetBuilder>();
    }(algorithm);
    
    auto set = sb->Build(graph);
    std::cout << *set << "\n";
    std::cout << "number of paths: " << set->GetNumberOfPaths() << "\n";
    std::cout << "number of edges: " << set->GetNumberOfEdges() << "\n";
    std::cout << "number of dots:  " << set->GetNumberOfDots() << "\n";

    return 0;
}
