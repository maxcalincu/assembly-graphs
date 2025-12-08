#pragma once

#include <core/interfaces.h>
#include <core/sag_with_endpoints.h>
#include <optional>
#include <stack>

template<SimpleAssemblyGraphImpl SAG, typename... Options>
class SAGGenerator;

template<>
class SAGGenerator<SAGWithEndpoints> {
using Vertex = SAGWithEndpoints::Vertex;
using Edge = SAGWithEndpoints::Edge;
using ECyc = SAGWithEndpoints::ECyc;

public:

SAGGenerator(size_t graph_id, size_t n);
std::optional<SAGWithEndpoints> Yield();

private:

std::stack<Vertex> inserted_vertices;
SAGWithEndpoints graph;

};