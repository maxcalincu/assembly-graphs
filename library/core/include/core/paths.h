#pragma once

#include <core/sag_with_endpoints.h>
#include <set>

template<SimpleAssemblyGraphImpl SAG>
class SetOfSimplePaths;

template<>
class SetOfSimplePaths<SAGWithEndpoints> {
    using Vertex = SAGWithEndpoints::Vertex;
    using Edge = SAGWithEndpoints::Edge;
    using ECyc = SAGWithEndpoints::ECyc;
public:
    friend std::ostream& operator<<(std::ostream& os, const SetOfSimplePaths<SAGWithEndpoints>& set) {
        size_t counter = 0;
        std::map<Vertex, size_t> bijection;
        auto& graph = set.graph;
        
        os << "SetOfSimplePaths(";
        for (auto edge = graph.GetKthEdge(0); edge != graph.GetLastEdge(); edge = set.graph.TransversalAdvance(edge)) {
            if (!bijection.contains(edge.GetHead())) {
                bijection[edge.GetHead()] = ++counter;
            }
            if (edge != graph.GetKthEdge(0)) {
                os << (set.Contains(edge) ? " - " : " ");
            }
            os << bijection[edge.GetHead()];
        }
        os << ")";
        return os;
    }
    SetOfSimplePaths(const SAGWithEndpoints& graph);
    void InsertEdge(Edge edge);
    void RemoveEdge(Edge edge);
    void ReverseGraph();
    bool IsInsertionValid(Edge edge) const;
    size_t GetNumberOfPaths() const;
    size_t GetNumberOfVertices() const;
    size_t GetNumberOfEdges() const;
    size_t GetNumberOfDots() const;

    Edge GetEndpointEdge(const Vertex& vertex) const;
    Vertex GetOtherEndpoint(const Vertex& vertex) const;
    Edge GetNextPathEdge(const Edge& edge) const;

    bool IsPathEndpoint(const Vertex& vertex) const;
    bool IsCovered(const Vertex& vertex) const;
    bool Contains(const Edge& edge) const;

private:
    SAGWithEndpoints graph;
    std::map<Vertex, Edge> endpoint_edges;
    std::map<Vertex, Vertex> other_endpoint;
    std::map<Edge, Edge> next_edge;
    std::set<Vertex> covered_vertices;
};
