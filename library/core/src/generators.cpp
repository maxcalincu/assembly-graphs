#include <core/generators.h>

TEdge<SAGWithEndpoints> SAGGenerator<SAGWithEndpoints>::RemoveFirstVertex(SAGWithEndpoints& graph) {
    if (graph.GetSize() == 0) {
        throw std::runtime_error("RemoveFirstVertex");
    }
    auto [x, y] = graph.RemoveVertex(graph.GetKthEdge(0).GetHead());
    x.OrientTowards(graph.GetOrientation());
    y.OrientTowards(graph.GetOrientation());
    return (x == graph.GetKthEdge(0) ? y : x);
}

SAGWithEndpoints SAGGenerator<SAGWithEndpoints>::GetLexicographicallySmallest(size_t graph_id, size_t n) {
    SAGWithEndpoints graph(graph_id);
    while (graph.GetSize() < n) {
        graph.InsertVertex(graph.GetStartEdge(), graph.GetStartEdge());
    }
    return graph;
}

bool SAGGenerator<SAGWithEndpoints>::Advance(SAGWithEndpoints& graph) {
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
