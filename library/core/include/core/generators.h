#pragma once

#include <core/sag_with_endpoints.h>
#include <stack>
#include <optional>

class SAGGenerator {
    using Vertex = SAGWithEndpoints::Vertex;
    using Edge = SAGWithEndpoints::Edge;
    using ECyc = SAGWithEndpoints::ECyc;

    public:

    bool Advance(SAGWithEndpoints& graph) const;
    SAGWithEndpoints GetLexicographicallySmallest (size_t graph_id, size_t n) const;

    private:
    Edge RemoveFirstVertex(SAGWithEndpoints& graph) const;

};

namespace Details {

    class PatternMatcher {
        std::set<size_t> opening_positions, closing_positions;
        static constexpr char OPENING_SYMBOL = '1';
        static constexpr char CLOSING_SYMBOL = '2';
        static constexpr char UNKNOWN_SYMBOL = '?';
        static constexpr char SEPARATOR_SYMBOL = '-';
    public:
        PatternMatcher(size_t n, const std::string& pattern = "");
        bool is_opening(size_t position) const { return !closing_positions.contains(position); }
        bool is_closing(size_t position) const { return !opening_positions.contains(position); }
    };

    class FenwickTree {
        std::vector<int> data;
        size_t lsb(size_t x) const { return x - (x & (x - 1)); }
    public:
        FenwickTree(size_t n): data(n + 1, 0) {}
        void update(size_t pos, int delta);
        int query(size_t pos) const;
    };

    class PersistentStack {
        struct Change {
            enum Operation{Pop, Push};
            Operation operation;
            size_t vertex;
        };
        std::stack<Change> records;
        std::stack<size_t> vertices;
    public:
        bool empty() const { return vertices.empty(); }
        void pop();
        void push(size_t vertex);
        size_t top() const;
        void restore();
        void restore_pop_sequence();
    };

} // namespace Details

class FilteredSAGGenerator {
    using Vertex = SAGWithEndpoints::Vertex;
    using Edge = SAGWithEndpoints::Edge;
    using ECyc = SAGWithEndpoints::ECyc;

    bool exhausted = false;
    size_t n, min_sub_word;

    std::vector<size_t> two_word, left_bound;
    std::vector<std::pair<size_t, size_t>> appearances;
    std::set<size_t> unpaired_vertices;

    Details::PatternMatcher pattern_matcher;
    Details::PersistentStack min_records;
    Details::FenwickTree prefix_openings;

    void PopAppearance();
    void PushAppearance(size_t vertex);
    bool DFS(std::optional<size_t> prev_option);

    void PushRecord(size_t vertex);
    void PopRecord();
    bool CheckMinString(size_t final_vertex) const;
    public:

    size_t GetMinSubword() const;
    size_t GetSize() const;

    FilteredSAGGenerator(size_t n, size_t min_sub_word, const std::string& pattern = "");
    bool Yield(SAGWithEndpoints& graph);
};
