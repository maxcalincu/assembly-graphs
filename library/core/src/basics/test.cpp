#include <catch2/catch_test_macros.hpp>
#include <core/basics.h>

class DummySAG;
using Vertex = TVertex<DummySAG>;
using Edge = TEdge<DummySAG>;
using ECyc = TECyc<DummySAG>;


TEST_CASE("Vertex") {
    Vertex a(6, 6), b(6, 9);
    Vertex x(2, 9), y(2, 9);
    REQUIRE(a == a);
    REQUIRE_FALSE(a == b);
    REQUIRE_FALSE(b == y);
    REQUIRE(x == y);
    REQUIRE(y.GetGraphId() == 2);
    REQUIRE(a.GetVertexId() == 6);
    REQUIRE(a < b);
    REQUIRE_FALSE(a < x);
}

TEST_CASE("Polarity & Tier") {
    Polarity p = Positive, n = Negative;
    REQUIRE(p == p);
    REQUIRE((!p) == n);
    REQUIRE((!n) == p);
    REQUIRE_FALSE(p == n);

    Tier tier = Tier::A;
    REQUIRE(tier == tier);
    REQUIRE(++tier == Tier::B);
    REQUIRE(++tier == Tier::C);
    REQUIRE(++tier == Tier::D);
    REQUIRE(++tier == Tier::X);
    REQUIRE(++tier == Tier::X);
}

TEST_CASE("Edge") {
    Vertex a(5, 6), b(5, 7), impostor(6, 9);
    REQUIRE_THROWS(Edge(a, impostor, Tier::A, Positive));
    Edge edge(a, b, Tier::C, Negative);
    Edge different_edge(a, b, Tier::A, Negative);
    Edge loop_p(b, b, Tier::A, Positive);
    Edge loop_n(b, b, Tier::A, Negative);
    
    Edge opposite_edge = edge;
    opposite_edge.SwitchOrientation();
    REQUIRE(edge != opposite_edge);
    REQUIRE(edge == edge);

    REQUIRE(edge.GetPositive() == a);
    REQUIRE(edge.GetNegative() == b);
    REQUIRE(edge.GetHead() == b);
    REQUIRE(edge.GetTail() == a);
    REQUIRE(edge.GetTier() == Tier::C);
    REQUIRE(edge.GetOrientation() == Negative);

    REQUIRE(edge.GetOther(b) == a);
    REQUIRE_THROWS(edge.GetOther(impostor));

    REQUIRE_FALSE(edge.HasEndpoint(impostor));
    REQUIRE(edge.HasEndpoint(a));
    
    REQUIRE_FALSE(edge.IsLoop());
    REQUIRE(loop_p.IsLoop());
    REQUIRE_FALSE(loop_p == loop_n);
    loop_n.SwitchOrientation();
    REQUIRE(loop_p == loop_n);
    REQUIRE(loop_n.GetOrientation() == Positive);
    REQUIRE_FALSE(edge == different_edge);
    
    REQUIRE_THROWS(loop_p.OrientTowards(b));
    edge.OrientTowards(b);
    REQUIRE(edge.GetHead() == b);
    edge.OrientTowards(a);
    REQUIRE(edge.GetHead() == a);
}

TEST_CASE("ECyc") {
    Vertex  o(0, 0), 
            a(0, 1),
            b(0, 2),
            x(0, 3),
            y(0, 4),
            impostor_vertex(67, 67);
    Edge    ao(a, o, Tier::A, Positive),
            ob(o, b, Tier::A, Positive),
            xo(x, o, Tier::A, Positive),
            oy(o, y, Tier::A, Positive),
            loop_p(o, o, Tier::A, Positive),
            loop_n(o, o, Tier::A, Negative),
            ao_2(a, o, Tier::B, Positive),
            ao_3(a, o, Tier::C, Positive),
            impostor(impostor_vertex, impostor_vertex, Tier::A, Positive);

    ao.OrientTowards(o);
    ob.OrientTowards(o);
    xo.OrientTowards(o);
    oy.OrientTowards(o);
    ao_2.OrientTowards(o);
    ao_3.OrientTowards(o);

    ECyc ecyc(o, {xo, oy}, {ao, ob});
    ECyc ecyc_copy = ecyc;
    REQUIRE_THROWS(ecyc.ReplaceEdge(impostor, ao_2));
    REQUIRE(ecyc == ecyc_copy);
    ecyc.ReplaceEdge(ao, ao_2);
    REQUIRE(ecyc == ECyc(o, {xo, oy}, {ao_2, ob}));
    REQUIRE_THROWS(ecyc.ReplaceEdge(ob, ao_3));
    ecyc.ReplaceEdge(ob, ao);
    REQUIRE(ecyc == ECyc(o, {xo, oy}, {ao_2, ao}));
    ecyc.ReplaceEdge(ao, ao_3);
    REQUIRE(ecyc == ECyc(o, {xo, oy}, {ao_2, ao_3}));

    ao.OrientTowards(a);
    REQUIRE_THROWS(ECyc(o, {xo, oy}, {ao, ob}));
    ao.OrientTowards(o);

    REQUIRE_THROWS(ECyc(o, {ao, loop_p}, {loop_p, oy}));
    REQUIRE_THROWS(ECyc(o, {xo, oy}, {ao, oy}));


    auto test_ecyc = [&](const Vertex& o, const Edge& xo, const Edge& oy, const Edge& ao, const Edge& ob, const std::map<Vertex, Tier>& expected_tiers){
        ECyc ecyc(o, std::make_pair(xo, oy), std::make_pair(ao, ob));

        REQUIRE(ecyc.HasEdge(xo));
        REQUIRE(ecyc.HasEdge(oy));
        REQUIRE(ecyc.HasEdge(ao));
        REQUIRE(ecyc.HasEdge(ob));

        REQUIRE(ecyc == ECyc(o, std::make_pair(oy, xo), std::make_pair(ao, ob)));
        REQUIRE(ecyc == ECyc(o, std::make_pair(ao, ob), std::make_pair(oy, xo)));
        REQUIRE(ecyc == ECyc(o, std::make_pair(ob, ao), std::make_pair(oy, xo)));
        REQUIRE(ecyc == ECyc(o, {xo, ao, oy, ob}));
        REQUIRE_FALSE(ecyc == ECyc(o, {xo, ao, ob, oy}));

        REQUIRE(ecyc.HasEdge(oy));
        REQUIRE_FALSE(ecyc.HasEdge(impostor));

        REQUIRE(ecyc.GetTransversal(xo) == oy);
        REQUIRE(ecyc.GetTransversal(oy) == xo);
        REQUIRE(ecyc.GetTransversal(ao) == ob);
        REQUIRE(ecyc.GetTransversal(ob) == ao);

        REQUIRE(ecyc.HasEdge(ecyc.GetAnyEdge()));
        REQUIRE(ecyc.GetCentralVertex() == o);

        auto [n_1, n_2] = ecyc.GetNeighbours(ob);
        REQUIRE(((n_1 == xo && n_2 == oy) || (n_2 == xo && n_1 == oy)));

        REQUIRE(ecyc.MEXTier(xo.GetOther(o)) == expected_tiers.at(xo.GetOther(o)));
        REQUIRE(ecyc.MEXTier(oy.GetOther(o)) == expected_tiers.at(oy.GetOther(o)));
        REQUIRE(ecyc.MEXTier(ao.GetOther(o)) == expected_tiers.at(ao.GetOther(o)));
        REQUIRE(ecyc.MEXTier(ob.GetOther(o)) == expected_tiers.at(ob.GetOther(o)));
    };
    test_ecyc(o, xo, oy, ao, ob, {
        {x, Tier::B},
        {y, Tier::B},
        {a, Tier::B},
        {b, Tier::B},
    });
    test_ecyc(o, xo, loop_n, loop_p, ob, {
        {x, Tier::B},
        {b, Tier::B},
        {o, Tier::B},
    });
    test_ecyc(o, xo, ao_2, ao, ao_3, {
        {x, Tier::B},
        {a, Tier::D},
    });
}

TEST_CASE("Create Copy") {
    Vertex  o(0, 0), 
            a(0, 1),
            b(0, 2),
            x = a,
            y(0, 4),

            O(1, 10), 
            A(1, 11),
            B(1, 12),
            X = A,
            Y(1, 14);
    Edge    ao(a, o, Tier::A, Positive),
            ob(o, b, Tier::A, Positive),
            xo(x, o, Tier::B, Positive),
            oy(o, y, Tier::A, Positive),
            
            AO(A, O, Tier::A, Positive),
            OB(O, B, Tier::A, Positive),
            XO(X, O, Tier::B, Positive),
            OY(O, Y, Tier::A, Positive);

            ao.OrientTowards(o);
            ob.OrientTowards(o);
            xo.OrientTowards(o);
            oy.OrientTowards(o);

            AO.OrientTowards(O);
            OB.OrientTowards(O);
            XO.OrientTowards(O);
            OY.OrientTowards(O);
    std::map<Vertex, Vertex> bijection = {
        {a, A},
        {b, B},
        {x, X},
        {y, Y},
        {o, O},
    }, BIJECTION = {
        {A, a},
        {B, b},
        {X, x},
        {Y, y},
        {O, o},
    }, id = {
        {a, a},
        {b, b},
        {x, x},
        {y, y},
        {o, o},

    };
    ECyc ecyc(o, std::make_pair(ao, ob), std::make_pair(xo, oy));
    ECyc ECYC(O, std::make_pair(XO, OY), std::make_pair(OB, AO));
    REQUIRE(ecyc.CreateCopy(bijection) == ECYC);
    REQUIRE(ECYC.CreateCopy(BIJECTION) == ecyc);
    REQUIRE(ecyc.CreateCopy(id) == ecyc);
}
