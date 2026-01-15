#include <catch2/catch_test_macros.hpp>
#include <core/set_builders.h>

using Vertex = TVertex<SAGWithEndpoints>;
using Edge = TEdge<SAGWithEndpoints>;
using ECyc = TECyc<SAGWithEndpoints>;
using Set = SetOfPolygonalPaths<SAGWithEndpoints>;

const SAGWithEndpoints loop(0, {1, 1});
const SAGWithEndpoints pretzel(1, {1, 2, 1, 2});

TEST_CASE("Greedy") {
    GreedySetBuilder greedy;
    SymGreedySetBuilder sym_greedy;
    CHECK(GetNumberOfPaths(SAGWithEndpoints(3, {}), greedy) == 0);
    CHECK(GetNumberOfPaths(SAGWithEndpoints(4, {1, 1}).InteriorSaturate(loop), greedy) == 1);
    CHECK(GetNumberOfPaths(SAGWithEndpoints(4, {1, 1}).Saturate(loop), greedy) == 2);
    CHECK(GetNumberOfPaths(SAGWithEndpoints(5, {1, 1, 2, 2, 3, 3}).Saturate(loop), greedy) == 4);
    CHECK(GetNumberOfPaths(SAGWithEndpoints(5, {1, 2, 3, 2, 4, 4, 3, 1}).Saturate(loop), greedy) == 5);
    CHECK(GetNumberOfPaths(SAGWithEndpoints(5, {1, 1}).Saturate(pretzel), greedy) == 2);
    CHECK(GetNumberOfPaths(SAGWithEndpoints(5, {1, 2, 3, 2, 4, 4, 3, 1}).Saturate(pretzel), greedy) == 5);
    CHECK(GetNumberOfPaths(3 * SAGWithEndpoints(5, {1, 2, 3, 4, 3, 4, 5, 1, 2, 5}), greedy) == 3);
    CHECK(GetNumberOfPaths(3 * SAGWithEndpoints(5, {1, 2, 3, 4, 3, 4, 5, 1, 2, 5}), sym_greedy) == 1);
}

TEST_CASE("An") {
    CHECK(An(loop) == 1);
    CHECK(An(pretzel) == 1);
    CHECK(An(SAGWithEndpoints(4, {})) == 0);
    CHECK(An(SAGWithEndpoints(4, {1, 1}).InteriorSaturate(loop)) == 1);
    CHECK(An(SAGWithEndpoints(4, {1, 1}).Saturate(loop)) == 2);
    CHECK(An(SAGWithEndpoints(5, {1, 2, 3, 2, 4, 4, 3, 1})) == 1);
    CHECK(An(SAGWithEndpoints(5, {1, 1, 2, 2, 3, 3}).Saturate(loop)) == 4);
    auto graph = SAGWithEndpoints(5, {1, 1, 2, 2, 3, 3}).Saturate(loop);
    graph.RemoveVertex(graph.GetKthEdge(3).GetHead());
    CHECK(An(graph) == 3);
    graph.RemoveVertex(graph.GetKthEdge(0).GetHead());
    CHECK(An(graph) == 3);
    graph.RemoveVertex(graph.GetKthEdge(0).GetHead());
    CHECK(An(graph) == 3);
    graph.RemoveVertex(graph.GetKthEdge(0).GetHead());
    CHECK(An(graph) == 2);
}