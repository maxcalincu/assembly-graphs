#include <core/set_builders.h>

#include <functional>

SetPointer GreedySetBuilder::Build(const SAGWithEndpoints& graph) const {
    SetPointer set = std::make_shared<SetOfPolygonalPaths<SAGWithEndpoints>>(graph);
    if (graph.GetSize() == 0) {
        return set;
    }
    for (auto edge = graph.GetKthEdge(1); edge != graph.GetLastEdge(); edge = graph.TransversalAdvance(edge)) {
        if (set->IsInsertionValid(edge)) {
            set->InsertEdge(edge);
        }
    }
    return set;
}

SetPointer SymGreedySetBuilder::Build(const SAGWithEndpoints& graph) const {
    GreedySetBuilder sb;
    auto copy = graph; copy.Reverse();

    auto forwards = sb.Build(graph);
    auto backwards = sb.Build(copy);
    backwards->ReverseGraph();
    return forwards->GetNumberOfPaths() < backwards->GetNumberOfPaths() ? forwards : backwards;
}

SetPointer ExactSetBuilder::Build(const SAGWithEndpoints& graph) const {
    SetPointer set = std::make_shared<SetOfPolygonalPaths<SAGWithEndpoints>>(graph);
    SetPointer best_result = std::make_shared<SetOfPolygonalPaths<SAGWithEndpoints>>(graph); 
    std::function<void(const TEdge<SAGWithEndpoints>&, size_t)> recursion = [&](const TEdge<SAGWithEndpoints>& edge, size_t edges_left) {
        if (edges_left == 0) {
            if (best_result->GetNumberOfPaths() > set->GetNumberOfPaths()) {
                *best_result = *set;
            }
            return;
        }
        if (set->GetNumberOfPaths() >= (edges_left + 1)/2 + best_result->GetNumberOfPaths()) {
            return;
        }
        auto next_edge = graph.TransversalAdvance(edge);
        if (set->IsInsertionValid(edge)) {
            set->InsertEdge(edge);
            recursion(next_edge, edges_left - 1);
            set->RemoveEdge(edge);
        }
        recursion(next_edge, edges_left - 1);
    };
    recursion(graph.GetKthEdge(1), 2 * graph.GetSize() - 1);
    return best_result;
}

size_t GetNumberOfPaths(const SAGWithEndpoints& graph, const ISetBuilder& sb) {
    return sb.Build(graph)->GetNumberOfPaths();
}

size_t An(const SAGWithEndpoints& graph) {
    return GetNumberOfPaths(graph, ExactSetBuilder());
}
