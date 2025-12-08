#pragma once

#include <core/interfaces.h>
#include <core/sag_with_endpoints.h>

class SetOfSimplePaths {
    using Vertex = SAGWithEndpoints::Vertex;
    using Edge = SAGWithEndpoints::Edge;
    using ECyc = SAGWithEndpoints::ECyc;
public:
    SetOfSimplePaths(const SAGWithEndpoints& Graph);
    void AddEdge(Edge);
    void RemoveEdge(Edge);
    size_t getNumberOfPaths() const;
    size_t getNumberOfVertices() const;
private:
    // TBA
};