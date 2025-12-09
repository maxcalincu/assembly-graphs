#pragma once

#include <core/sag_with_endpoints.h>

template<SimpleAssemblyGraphImpl SAG, typename... Options>
class SAGGenerator;

template<>
class SAGGenerator<SAGWithEndpoints> {
    using Vertex = SAGWithEndpoints::Vertex;
    using Edge = SAGWithEndpoints::Edge;
    using ECyc = SAGWithEndpoints::ECyc;

    public:

    bool Advance(SAGWithEndpoints& graph);
    SAGWithEndpoints GetLexicographicallySmallest (size_t graph_id, size_t n);

    private:
    Edge RemoveFirstVertex(SAGWithEndpoints& graph);

};
