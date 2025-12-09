#include <catch2/catch_test_macros.hpp>
#include <core/sag_with_endpoints.h>

using Vertex = TVertex<SAGWithEndpoints>;
using Edge = TEdge<SAGWithEndpoints>;
using ECyc = TECyc<SAGWithEndpoints>;

auto check_polarity_invariant = [](const SAGWithEndpoints& graph) {
    for (auto edge = graph.GetKthEdge(0); edge != graph.GetLastEdge(); edge = graph.TransversalAdvance(edge)) {        
        REQUIRE(edge.GetOrientation() == graph.GetLastEdge().GetOrientation());
    }
};

TEST_CASE("Constructors") {
    REQUIRE_NOTHROW(SAGWithEndpoints(1, {}));
    REQUIRE_NOTHROW(SAGWithEndpoints(0));
    REQUIRE(SAGWithEndpoints(0) == SAGWithEndpoints(1, {}));
    SAGWithEndpoints example(0, {1, 1, 2, 3, 3, 2});
    REQUIRE(example == example);
    REQUIRE(example.IsIsomorphic(example));
    REQUIRE(example == SAGWithEndpoints(1, {4, 5, 5, 4, 0, 0}));
    REQUIRE_FALSE(example.IsIsomorphic(SAGWithEndpoints(1, {4, 5, 5, 4, 0, 0})));
    REQUIRE(example.IsIsomorphic(SAGWithEndpoints(1, {4, 4, 6, 7, 7, 6})));

    REQUIRE(SAGWithEndpoints(1, {67, 67}).ConvertToVector() == std::vector<size_t>{1, 1});
    REQUIRE(SAGWithEndpoints(1, {4, 5, 5, 4, 0, 0}).ConvertToVector() == std::vector<size_t>{1, 2, 2, 1, 3, 3});
    
    REQUIRE_THROWS(SAGWithEndpoints(1, {1, 2, 2, 1, 3}));
    REQUIRE_THROWS(SAGWithEndpoints(42, {1}));
    REQUIRE_THROWS(SAGWithEndpoints(67, {1, 2, 2, 1, 3, 2, 3, 3}));
    REQUIRE_THROWS(SAGWithEndpoints(67, {4, 4, 4, 4}));

}

TEST_CASE("TransversalAdvance") {
    SAGWithEndpoints example(0, {1, 1, 2, 3, 3, 2});
    REQUIRE(example.GetSize() == 3);
    REQUIRE(SAGWithEndpoints(0).GetSize() == 0);
    REQUIRE(SAGWithEndpoints(6, {1, 1}).GetSize() == 1);
    REQUIRE(SAGWithEndpoints(6, {2, 1, 2, 1}).GetSize() == 2);
    
    check_polarity_invariant(example);
    
    auto edge = example.GetKthEdge(0);
    
    std::vector<Edge> edges;
    edges.push_back(edge);
    for (size_t i = 0; i < 6; ++i) {
        edge = example.TransversalAdvance(edge);
        edges.push_back(edge);
    }
    REQUIRE(edges[1].IsLoop());
    REQUIRE(edges[4].IsLoop());
    REQUIRE(edges[6] == example.GetLastEdge());
    REQUIRE(edges[1].GetHead() == edges[2].GetTail());
    REQUIRE(edges[2].GetHead() == edges[6].GetTail());

    REQUIRE(example.IsEndpoint(edges[0].GetTail()));
    REQUIRE(example.IsEndpoint(edges[6].GetHead()));
    REQUIRE_FALSE(example.IsEndpoint(edges[4].GetHead()));
    
    Vertex impostor(42, 42);
    REQUIRE(example.HasVertex(edges[4].GetTail()));
    REQUIRE_FALSE(example.HasVertex(impostor));
    
    auto two = edges[2].GetHead();
    ECyc ecyc = example.GetECyc(two);
    REQUIRE(ecyc.GetCentralVertex() == two);
    auto transversal = ecyc.GetTransversal(edges[2]);
    transversal.SwitchOrientation();
    REQUIRE(transversal == edges[3]);
    REQUIRE_THROWS(example.GetECyc(edges[0].GetTail()));
    REQUIRE_THROWS(example.GetECyc(edges[6].GetHead()));
    REQUIRE_THROWS(example.GetECyc(impostor));

    edge = example.GetLastEdge();
    edge.SwitchOrientation();
    for (size_t i = 0; i < 6; ++i) {
        edge = example.TransversalAdvance(edge);
        edges[5 - i].SwitchOrientation();
        REQUIRE(edges[5 - i] == edge);
    }
}

TEST_CASE("Modification") {
    SAGWithEndpoints example(0, {1, 1, 2, 3, 3, 2});

    REQUIRE(example.GetStartEdge().GetHead() != example.GetLastEdge().GetHead());

    auto example_copy = example;

    example_copy.Reverse();
    REQUIRE(example.GetStartEdge().GetHead() == example_copy.GetLastEdge().GetHead());
    REQUIRE(example == example_copy);
    REQUIRE_FALSE(example.IsIsomorphic(example_copy));

    example_copy.Reverse();
    REQUIRE(example.IsIsomorphic(example_copy));

    SAGWithEndpoints graph(3, {});
    graph.InsertVertex(graph.GetStartEdge(), graph.GetLastEdge());
    REQUIRE(graph == SAGWithEndpoints{1, {1, 1}});
    graph = SAGWithEndpoints(0, {});
    Vertex vertex = graph.InsertVertex(graph.GetStartEdge(), graph.GetStartEdge());
    REQUIRE(graph == SAGWithEndpoints{1, {1, 1}});
    REQUIRE(graph.GetStartEdge().GetTail() == vertex);
    REQUIRE(graph.GetLastEdge().GetTail() == vertex);

    graph.InsertVertex(graph.GetKthEdge(0), graph.GetKthEdge(0));
    REQUIRE(graph == SAGWithEndpoints{1, {2, 2, 1, 1}});
    REQUIRE_NOTHROW(graph.GetKthEdge(1));
    REQUIRE_NOTHROW(graph.GetKthEdge(3));
    graph.InsertVertex(graph.GetKthEdge(1), graph.GetKthEdge(3));
    REQUIRE(graph == SAGWithEndpoints{1, {2, 3, 2, 1, 3, 1}});

    check_polarity_invariant(graph);

    graph.InsertVertex(graph.GetKthEdge(3), graph.GetKthEdge(3));
    REQUIRE(graph == SAGWithEndpoints{1, {2, 3, 2, 4, 4, 1, 3, 1}});
    graph.InsertVertex(graph.GetKthEdge(0), graph.GetKthEdge(6));

    check_polarity_invariant(graph);

    REQUIRE(graph == SAGWithEndpoints{1, {5, 2, 3, 2, 4, 4, 1, 5, 3, 1}});
    graph.InsertVertex(graph.GetKthEdge(5), graph.GetKthEdge(5));
    REQUIRE(graph == SAGWithEndpoints{1, {5, 2, 3, 2, 4, 6, 6, 4, 1, 5, 3, 1}});

    graph.RemoveVertex(graph.GetKthEdge(0).GetHead());
    REQUIRE(graph == SAGWithEndpoints{1, {2, 3, 2, 4, 6, 6, 4, 1, 3, 1}});
    graph.RemoveVertex(graph.GetKthEdge(4).GetHead());
    REQUIRE(graph == SAGWithEndpoints{3, {2, 3, 2, 4, 4, 1, 3, 1}});
    graph.RemoveVertex(graph.GetKthEdge(1).GetHead());

    check_polarity_invariant(graph);

    REQUIRE(graph == SAGWithEndpoints{1, {2, 2, 4, 4, 1, 1}});
    graph.RemoveVertex(graph.GetKthEdge(4).GetHead());
    REQUIRE(graph == SAGWithEndpoints{1, {2, 2, 4, 4}});

    check_polarity_invariant(graph);

    graph.RemoveVertex(graph.GetKthEdge(3).GetHead());
    REQUIRE(graph == SAGWithEndpoints{1, {2, 2}});

    graph = SAGWithEndpoints(0, {1, 2, 2, 1});
    graph.RemoveVertex(graph.GetKthEdge(1).GetHead());
    REQUIRE(graph == SAGWithEndpoints(0, {1, 1}));

    graph = SAGWithEndpoints(0, {1, 2, 1, 2});
    graph.RemoveVertex(graph.GetKthEdge(0).GetHead());
    auto vector = graph.ConvertToVector();
    REQUIRE(graph == SAGWithEndpoints{1, {1, 1}});

    graph = SAGWithEndpoints(0, {1, 2, 1, 2});
    graph.RemoveVertex(graph.GetKthEdge(1).GetHead());
    vector = graph.ConvertToVector();
    REQUIRE(graph == SAGWithEndpoints{1, {1, 1}});

    graph.RemoveVertex(graph.GetKthEdge(0).GetHead());
    REQUIRE(graph == SAGWithEndpoints{1, {}});

    check_polarity_invariant(graph);

    graph = SAGWithEndpoints{1, {3, 3}};
    graph.InsertGraph(graph.GetKthEdge(1), SAGWithEndpoints(4, {3, 3}));
    REQUIRE(graph == SAGWithEndpoints{42, {1, 2, 2, 1}});

    check_polarity_invariant(graph);

    graph = SAGWithEndpoints{1, {2, 3, 2, 4, 6, 6, 4, 1, 3, 1}};
    graph.InsertGraph(graph.GetKthEdge(3), SAGWithEndpoints(4, {3, 2, 2, 3}));
    REQUIRE(graph == SAGWithEndpoints{42, {2, 3, 2, 12, 13, 13, 12, 4, 6, 6, 4, 1, 3, 1}});
    graph.InsertGraph(graph.GetKthEdge(7), SAGWithEndpoints(4, {1, 2, 1, 2}));

    check_polarity_invariant(graph);

    REQUIRE(graph == SAGWithEndpoints{42, {2, 3, 2, 12, 13, 13, 12, 21, 22, 21, 22, 4, 6, 6, 4, 1, 3, 1}});
    graph.InsertGraph(graph.GetKthEdge(0), SAGWithEndpoints(4, {1, 1}));
    REQUIRE(graph == SAGWithEndpoints{42, {100, 100, 2, 3, 2, 12, 13, 13, 12, 21, 22, 21, 22, 4, 6, 6, 4, 1, 3, 1}});
}