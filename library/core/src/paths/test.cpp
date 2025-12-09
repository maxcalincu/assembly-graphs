#include "core/sag_with_endpoints.h"
#include <catch2/catch_test_macros.hpp>
#include <core/paths.h>

using Vertex = TVertex<SAGWithEndpoints>;
using Edge = TEdge<SAGWithEndpoints>;
using ECyc = TECyc<SAGWithEndpoints>;
using Set = SetOfSimplePaths<SAGWithEndpoints>;

TEST_CASE("Simple") {
    SAGWithEndpoints graph(0, {1, 2, 1, 2});
    Set set(graph);
    REQUIRE_FALSE(set.Contains(graph.GetKthEdge(0)));
    REQUIRE_FALSE(set.IsInsertionValid(graph.GetKthEdge(0)));
    REQUIRE(set.IsInsertionValid(graph.GetKthEdge(1)));
    REQUIRE(set.IsInsertionValid(graph.GetKthEdge(2)));
    REQUIRE(set.IsInsertionValid(graph.GetKthEdge(3)));
    REQUIRE_FALSE(set.IsInsertionValid(graph.GetKthEdge(4)));

    graph = SAGWithEndpoints(0, {1, 1});
    set = Set(graph);
    REQUIRE_FALSE(set.IsInsertionValid(graph.GetKthEdge(0)));
    REQUIRE_FALSE(set.IsInsertionValid(graph.GetKthEdge(1)));
    REQUIRE_FALSE(set.IsInsertionValid(graph.GetKthEdge(2)));

    graph = SAGWithEndpoints(0, {1, 2, 3, 3, 1, 2});
    set = Set(graph);
    REQUIRE(set.IsInsertionValid(graph.GetKthEdge(2)));
    REQUIRE_FALSE(set.IsHamiltonian());
    set.InsertEdge(graph.GetKthEdge(1));
    REQUIRE(set.GetNumberOfVertices() == 2);
    REQUIRE(set.GetNumberOfEdges() == 1);
    REQUIRE(set.GetNumberOfPaths() == 1);
    REQUIRE(set.IsEndpoint(graph.GetKthEdge(0).GetHead()));
    REQUIRE(set.IsEndpoint(graph.GetKthEdge(1).GetHead()));

    REQUIRE(set.IsInsertionValid(graph.GetKthEdge(4)));
    REQUIRE_FALSE(set.IsInsertionValid(graph.GetKthEdge(5)));
    REQUIRE_FALSE(set.IsHamiltonian());
    set.InsertEdge(graph.GetKthEdge(4));
    REQUIRE(set.GetNumberOfVertices() == 3);
    REQUIRE(set.GetNumberOfEdges() == 2);
    REQUIRE(set.GetNumberOfPaths() == 1);
    REQUIRE(set.IsEndpoint(graph.GetKthEdge(2).GetHead()));
    REQUIRE(set.IsEndpoint(graph.GetKthEdge(1).GetHead()));
    REQUIRE_FALSE(set.IsEndpoint(graph.GetKthEdge(0).GetHead()));
    REQUIRE(set.GetOtherEndpoint(graph.GetKthEdge(2).GetHead()) == graph.GetKthEdge(1).GetHead());
    REQUIRE(set.GetOtherEndpoint(graph.GetKthEdge(1).GetHead()) == graph.GetKthEdge(2).GetHead());
    REQUIRE(set.Contains(graph.GetKthEdge(1)));
    REQUIRE(set.Contains(graph.GetKthEdge(4)));
    REQUIRE_FALSE(set.Contains(graph.GetKthEdge(0)));
    REQUIRE_FALSE(set.Contains(graph.GetKthEdge(2)));
    REQUIRE_FALSE(set.Contains(graph.GetKthEdge(3)));
    REQUIRE_FALSE(set.Contains(graph.GetKthEdge(5)));
    REQUIRE(set.GetNextPathEdge(graph.GetKthEdge(4)) == graph.GetKthEdge(1));
    REQUIRE(set.IsHamiltonian());

    graph = SAGWithEndpoints(0, {});
    set = Set(graph);
    REQUIRE(set.IsHamiltonian());
}

TEST_CASE("Example") {
    auto graph = SAGWithEndpoints(0, {1, 2, 3, 2, 4, 4, 3, 1});
    auto set = Set(graph);
    REQUIRE_NOTHROW(set.InsertEdge(graph.GetKthEdge(1)));
    REQUIRE_NOTHROW(set.InsertEdge(graph.GetKthEdge(4)));
    REQUIRE_NOTHROW(set.InsertEdge(graph.GetKthEdge(7)));
    REQUIRE(set.GetNumberOfPaths() == 1);
    REQUIRE(set.IsHamiltonian());
    REQUIRE(set.IsEndpoint(graph.GetKthEdge(2).GetHead()));
    REQUIRE(set.IsEndpoint(graph.GetKthEdge(4).GetHead()));
    auto edge = graph.GetKthEdge(7);
    REQUIRE_NOTHROW(edge = set.GetNextPathEdge(edge));
    REQUIRE_NOTHROW(edge = set.GetNextPathEdge(edge));
    REQUIRE(edge == graph.GetKthEdge(4));
    REQUIRE(set.GetOtherEndpoint(graph.GetKthEdge(2).GetHead()) == graph.GetKthEdge(4).GetHead());
    REQUIRE(set.GetOtherEndpoint(graph.GetKthEdge(4).GetHead()) == graph.GetKthEdge(2).GetHead());
}
