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
    SetOfSimplePaths(const SAGWithEndpoints& graph);
    void InsertEdge(Edge edge);
    void RemoveEdge(Edge edge);
    bool IsInsertionValid(Edge edge) const;
    size_t GetNumberOfPaths() const;
    size_t GetNumberOfVertices() const;
    size_t GetNumberOfEdges() const;

    Edge GetEndpointEdge(const Vertex& vertex) const;
    Vertex GetOtherEndpoint(const Vertex& vertex) const;
    Edge GetNextPathEdge(const Edge& edge) const;

    bool IsHamiltonian() const;
    bool IsEndpoint(const Vertex& vertex) const;
    bool IsCovered(const Vertex& vertex) const;
    bool Contains(const Edge& edge) const;

private:
    SAGWithEndpoints graph;
    std::map<Vertex, Edge> endpoint_edges;
    std::map<Vertex, Vertex> other_endpoint;
    std::map<Edge, Edge> next_edge;
    std::set<Vertex> covered_vertices;
};
