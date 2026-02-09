#include <core/generators.h>

TEdge<SAGWithEndpoints> SAGGenerator::RemoveFirstVertex(SAGWithEndpoints& graph) const {
    if (graph.GetSize() == 0) {
        throw std::runtime_error("RemoveFirstVertex");
    }
    auto [x, y] = graph.RemoveVertex(graph.GetKthEdge(0).GetHead());
    x.OrientTowards(graph.GetOrientation());
    y.OrientTowards(graph.GetOrientation());
    return (x == graph.GetKthEdge(0) ? y : x);
}

SAGWithEndpoints SAGGenerator::GetLexicographicallySmallest(size_t graph_id, size_t n) const {
    SAGWithEndpoints graph(graph_id);
    while (graph.GetSize() < n) {
        graph.InsertVertex(graph.GetStartEdge(), graph.GetStartEdge());
    }
    return graph;
}

bool SAGGenerator::Advance(SAGWithEndpoints& graph) const {
    size_t n = graph.GetSize();
    while (graph.GetSize() > 0) {
        auto previous_edge = RemoveFirstVertex(graph);
        if (previous_edge == graph.GetLastEdge()) {
            continue;
        }
        auto next_edge = graph.TransversalAdvance(previous_edge);
        graph.InsertVertex(graph.GetKthEdge(0), next_edge);
        while (graph.GetSize() < n) {
            graph.InsertVertex(graph.GetKthEdge(0), graph.GetKthEdge(0));
        }
        break;
    }
    if (graph.GetSize() != 0) {
        return true;
    }
    while (graph.GetSize() < n) {
        graph.InsertVertex(graph.GetStartEdge(), graph.GetLastEdge());
    }
    return false;
}

Details::PatternMatcher::PatternMatcher(size_t n, const std::string& pattern)  {
    size_t cur = 0;
    for (size_t i = 0; i < pattern.size(); ++i) {
        switch (pattern[i]) {
            case OPENING_SYMBOL:
                opening_positions.insert(cur++);
                break;
            case CLOSING_SYMBOL:
                closing_positions.insert(cur++);
                break;
            case UNKNOWN_SYMBOL:
                ++cur;
                break;
            case SEPARATOR_SYMBOL:
                break;
            default:
                throw std::runtime_error("FilteredSAGGenerator: pattern symbol wasn't recognised");
        }
    }
    if (cur != 2 * n && pattern.size() > 0) {
        throw std::runtime_error("FilteredSAGGenerator: invalid pattern size");
    }
};

void Details::FenwickTree::update(size_t pos, int delta) {
    for (++pos; pos < data.size(); pos += lsb(pos)) {
        data.at(pos) += delta;
    }
}

int Details::FenwickTree::query(size_t pos) const {
    int result = 0;
    for (++pos; pos > 0; pos -= lsb(pos)) {
        result += data.at(pos);
    }
    return result;
}

void Details::PersistentStack::pop() {
    if (empty()) {
        throw std::runtime_error("PersistentStack: nothing to pop");
    }
    records.emplace(Change::Pop, vertices.top());
    vertices.pop();
}

void Details::PersistentStack::push(size_t vertex) {
    records.emplace(Change::Push, vertex);
    vertices.push(vertex);
}

size_t Details::PersistentStack::top() const {
    if (empty()) {
        throw std::runtime_error("PersistentStack: no top element");
    }
    return vertices.top();
}

void Details::PersistentStack::restore() {
    if (records.empty()) {
        throw std::runtime_error("PersistentStack: nothing to restore");
    }
    auto record = records.top();
    record.operation == Change::Pop ? vertices.push(record.vertex) : vertices.pop();;
    records.pop();
}

void Details::PersistentStack::restore_pop_sequence() {
    while (!records.empty() && records.top().operation == Change::Pop) {
        restore();
    }
}

FilteredSAGGenerator::FilteredSAGGenerator(size_t n, size_t min_sub_word, const std::string& pattern): 
    n(n), min_sub_word(min_sub_word),
    two_word(), left_bound(n + 1),
    appearances(n + 1),

    pattern_matcher(n, pattern), 
    prefix_openings(2 * n) {
        two_word.reserve(2 * n);
    }

bool FilteredSAGGenerator::Yield(SAGWithEndpoints& graph) {
    if (exhausted) {
        return false;
    }

    do {
        auto prev_option = two_word.empty() 
            ? std::optional<size_t>() 
            : two_word.back();
        PopAppearance();
        if (DFS(prev_option)) {
            break;
        }
    } while (!two_word.empty());

    if (two_word.empty()) {
        exhausted = true;
        return false;
    }

    graph = SAGWithEndpoints{graph.GetGraphId(), two_word};
    return true;
}

void FilteredSAGGenerator::PopAppearance() {
    if (two_word.empty()) {
        return;
    }
    auto vertex = two_word.back();
    two_word.pop_back();

    if (unpaired_vertices.contains(vertex)) {
        prefix_openings.update(appearances[vertex].first, -1);
        unpaired_vertices.erase(vertex);
    } else {
        prefix_openings.update(appearances[vertex].first, 1);
        unpaired_vertices.insert(vertex);
        PopRecord();
    }
}

void FilteredSAGGenerator::PushAppearance(size_t vertex) {
    const size_t position = two_word.size();

    if (unpaired_vertices.contains(vertex)) {
        appearances[vertex].second = position;
        unpaired_vertices.erase(vertex);
        prefix_openings.update(appearances[vertex].first, -1);
        PushRecord(vertex);
    } else {
        appearances[vertex].first = position;
        unpaired_vertices.insert(vertex);
        prefix_openings.update(appearances[vertex].first, 1);
    }
    two_word.push_back(vertex);
}

bool FilteredSAGGenerator::DFS(std::optional<size_t> prev_option) {
    const auto position = two_word.size(); 
    if (position == 2 * n) {
        return true;
    }

    if (pattern_matcher.is_closing(position)) {
        auto start = prev_option.has_value() 
            ? unpaired_vertices.upper_bound(prev_option.value())
            : unpaired_vertices.begin();
        std::vector<size_t> candidates(start, unpaired_vertices.end());
        for (auto next_option : candidates) {
            PushAppearance(next_option);
            if (CheckMinString(next_option) && DFS(std::nullopt)) {
                return true;
            }
            PopAppearance();
        }
    }

    if (pattern_matcher.is_opening(position)) {
        auto new_vertex = 1 + (two_word.size() + unpaired_vertices.size())/2;
        if ((prev_option.has_value() && prev_option.value() == new_vertex) || new_vertex > n) {
            return false;
        }
        PushAppearance(new_vertex);
        if (DFS(std::nullopt)) {
            return true;
        }
        PopAppearance();
    }
    return false;
}

size_t FilteredSAGGenerator::GetMinSubword() const {
    return min_sub_word;
}

size_t FilteredSAGGenerator::GetSize() const {
    return n;
}

void FilteredSAGGenerator::PushRecord(size_t vertex) {
    left_bound[vertex] = vertex;
    while (!min_records.empty()) {
        auto prev_vertex = min_records.top(); 
        if (appearances[prev_vertex].second < appearances[vertex].first) {
            break;
        }
        if (appearances[left_bound[prev_vertex]].first < appearances[left_bound[vertex]].first) {
            left_bound[vertex] = left_bound[prev_vertex];
        }
        min_records.pop();
    }
    min_records.push(vertex);
}

void FilteredSAGGenerator::PopRecord() {
    min_records.restore();
    min_records.restore_pop_sequence();
}

bool FilteredSAGGenerator::CheckMinString(size_t final_vertex) const {
    bool is_final = prefix_openings.query(appearances[left_bound[final_vertex]].first) 
                ==  prefix_openings.query(appearances[left_bound[final_vertex]].second);
    auto segment_l = appearances[left_bound[final_vertex]].first;
    auto segment_r = appearances[final_vertex].second;
    return !is_final ||  segment_r - segment_l + 1 >= 2 * min_sub_word;
}
