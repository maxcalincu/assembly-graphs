#include "core/basics.h"
#include "core/sag_with_endpoints.h"
#include <catch2/catch_test_macros.hpp>
#include <core/generators.h>

using Vertex = TVertex<SAGWithEndpoints>;
using Edge = TEdge<SAGWithEndpoints>;
using ECyc = TECyc<SAGWithEndpoints>;

auto generator = SAGGenerator<SAGWithEndpoints>();


TEST_CASE("Lexicographically Smallest") {
    auto graph = generator.GetLexicographicallySmallest(2, 0);
    REQUIRE(graph == SAGWithEndpoints(0, {}));
    graph = generator.GetLexicographicallySmallest(4, 1);
    REQUIRE(graph == SAGWithEndpoints(0, {1, 1}));
    graph = generator.GetLexicographicallySmallest(3, 2);
    REQUIRE(graph == SAGWithEndpoints(0, {1, 1, 2, 2}));
    graph = generator.GetLexicographicallySmallest(0, 5);
    REQUIRE(graph == SAGWithEndpoints(0, {1, 1, 2, 2, 3, 3, 4, 4, 5, 5}));
}

TEST_CASE("Advance") {
    auto max_graph_check = [&](const SAGWithEndpoints& graph) {
        auto graph_copy = graph;
        REQUIRE_FALSE(generator.Advance(graph_copy));
        REQUIRE(graph == graph_copy);
    };
    max_graph_check(SAGWithEndpoints(0, {}));
    max_graph_check(SAGWithEndpoints(5, {1, 1}));
    max_graph_check(SAGWithEndpoints(8, {3, 1, 1, 3}));
    max_graph_check(SAGWithEndpoints(8, {6, 3, 1, 1, 3, 6}));
    max_graph_check(SAGWithEndpoints(8, {9, 6, 3, 1, 1, 3, 6, 9}));

    SAGWithEndpoints graph = generator.GetLexicographicallySmallest(4, 2);
    REQUIRE(generator.Advance(graph));
    REQUIRE(graph == SAGWithEndpoints(0, {1, 2, 1, 2}));
    REQUIRE(generator.Advance(graph));
    REQUIRE(graph == SAGWithEndpoints(0, {1, 2, 2, 1}));
    REQUIRE_FALSE(generator.Advance(graph));

    graph = SAGWithEndpoints(6, {1, 2, 3, 5, 5, 6, 6, 3, 2, 1});
    REQUIRE(generator.Advance(graph));
    REQUIRE(graph == SAGWithEndpoints(0, {1, 1, 2, 2, 3, 3, 5, 6, 5, 6}));
    size_t counter = 0;
    while (generator.Advance(graph)) {
        ++counter;
    }
    REQUIRE(counter == (9 * 7 * 5 * 2 - 1));
}