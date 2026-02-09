#include <catch2/catch_test_macros.hpp>

#include <core/generators.h>
#include <core/set_builders.h>

using Vertex = TVertex<SAGWithEndpoints>;
using Edge = TEdge<SAGWithEndpoints>;
using ECyc = TECyc<SAGWithEndpoints>;

auto generator = SAGGenerator();


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

TEST_CASE("PatternMatcher tests", "[PatternMatcher]") {
    SECTION("Empty pattern") {
        Details::PatternMatcher matcher(5, "");
        REQUIRE(matcher.is_opening(0) == true);
        REQUIRE(matcher.is_closing(0) == true);
    }
    
    SECTION("Pattern with mixed symbols") {
        Details::PatternMatcher matcher(3, "1?2?12");
        
        // Position 0: '1' (opening)
        REQUIRE(matcher.is_opening(0) == true);
        REQUIRE(matcher.is_closing(0) == false);
        
        // Position 1: '?' (unknown)
        REQUIRE(matcher.is_opening(1) == true);
        REQUIRE(matcher.is_closing(1) == true);
        
        // Position 2: '2' (closing)
        REQUIRE(matcher.is_opening(2) == false);
        REQUIRE(matcher.is_closing(2) == true);
        
        // Position 3: '?' (unknown)
        REQUIRE(matcher.is_opening(3) == true);
        REQUIRE(matcher.is_closing(3) == true);
        
        // Position 4: '1' (opening)
        REQUIRE(matcher.is_opening(4) == true);
        REQUIRE(matcher.is_closing(4) == false);
        
        // Position 5: '2' (closing)
        REQUIRE(matcher.is_opening(5) == false);
        REQUIRE(matcher.is_closing(5) == true);
    }
    
    SECTION("Invalid pattern throws exception") {
        REQUIRE_THROWS(Details::PatternMatcher(1, "13"));
        REQUIRE_THROWS(Details::PatternMatcher(2, "a1b2"));
        REQUIRE_THROWS(Details::PatternMatcher(4, "invalid1"));
        REQUIRE_THROWS(Details::PatternMatcher(100, "1212"));
        
        REQUIRE_NOTHROW(Details::PatternMatcher(0, "-"));
        REQUIRE_NOTHROW(Details::PatternMatcher(2, "12-12"));
        REQUIRE_NOTHROW(Details::PatternMatcher(2, "1-2-1-2"));
        REQUIRE_NOTHROW(Details::PatternMatcher(2, "11-?--2"));
    }
    
    SECTION("Out of bounds position") {
        Details::PatternMatcher matcher(1, "11");
        REQUIRE(matcher.is_opening(100) == true);
        REQUIRE(matcher.is_closing(100) == true);
    }
}

TEST_CASE("FenwickTree tests", "[FenwickTree]") {
    
    SECTION("Single element tree") {
        Details::FenwickTree tree(1);
        
        REQUIRE(tree.query(0) == 0);
        
        tree.update(0, 5);
        REQUIRE(tree.query(0) == 5);
        
        tree.update(0, -3);
        REQUIRE(tree.query(0) == 2);

        REQUIRE_THROWS(tree.query(1));
        REQUIRE_THROWS(tree.query(5));
    }
    
    SECTION("Multiple updates and queries") {
        Details::FenwickTree tree(10);
        
        for (size_t i = 0; i < 10; ++i) {
            REQUIRE(tree.query(i) == 0);
        }
        
        tree.update(3, 10);
        REQUIRE(tree.query(3) == 10);
        REQUIRE(tree.query(2) == 0);
        REQUIRE(tree.query(4) == 10);
        
        tree.update(7, 5);
        REQUIRE(tree.query(6) == 10);
        REQUIRE(tree.query(7) == 15);
        REQUIRE(tree.query(9) == 15);
        
        tree.update(4, -4);
        REQUIRE(tree.query(3) == 10);
        REQUIRE(tree.query(4) == 6);
        REQUIRE(tree.query(7) == 11);
        
        tree.update(0, 1);
        tree.update(1, 2);
        tree.update(2, 3);
        
        REQUIRE(tree.query(0) == 1);
        REQUIRE(tree.query(1) == 3);
        REQUIRE(tree.query(2) == 6);
        REQUIRE(tree.query(3) == 16);
    }
}

TEST_CASE("PersistentStack tests", "[PersistentStack]") {
    SECTION("Empty stack") {
        Details::PersistentStack stack;
        REQUIRE(stack.empty());
        REQUIRE_THROWS_AS(stack.top(), std::runtime_error);
        REQUIRE_THROWS_AS(stack.pop(), std::runtime_error);
        REQUIRE_THROWS_AS(stack.restore(), std::runtime_error);
    }
    
    SECTION("Basic push and pop operations") {
        Details::PersistentStack stack;
        
        stack.push(1);
        REQUIRE_FALSE(stack.empty());
        REQUIRE(stack.top() == 1);
        
        stack.push(2);
        REQUIRE(stack.top() == 2);
        
        stack.push(3);
        REQUIRE(stack.top() == 3);
        
        stack.pop();
        REQUIRE(stack.top() == 2);
        
        stack.pop();
        REQUIRE(stack.top() == 1);
        
        stack.pop();
        REQUIRE(stack.empty());
    }
    
    SECTION("Restore operations") {
        Details::PersistentStack stack;
        
        stack.push(10);
        stack.push(20);
        stack.push(30);
        
        REQUIRE(stack.top() == 30);
        
        stack.pop();
        REQUIRE(stack.top() == 20);
        
        stack.restore();
        REQUIRE(stack.top() == 30);
        
        stack.push(40);
        REQUIRE(stack.top() == 40);
        
        stack.restore();
        REQUIRE(stack.top() == 30);
        
        stack.pop();
        stack.push(50);
        stack.push(60);
        
        REQUIRE(stack.top() == 60);
        
        stack.restore();
        REQUIRE(stack.top() == 50);
        
        stack.restore();
        REQUIRE(stack.top() == 20);
        
        stack.restore();
        REQUIRE(stack.top() == 30);
    }
    
    SECTION("restore_pop_sequence") {
        Details::PersistentStack stack;
        
        stack.push(1);
        stack.push(2);
        stack.pop();
        stack.push(3);
        stack.pop();
        stack.pop();
        REQUIRE(stack.empty());

        // restore_pop_sequence should restore consecutive pop operations
        // until it encounters a push or runs out of records
        stack.restore_pop_sequence();
        REQUIRE(stack.top() == 3);

        stack.restore_pop_sequence();
        REQUIRE(stack.top() == 3);
        
        stack.restore();
        stack.restore_pop_sequence();

        REQUIRE(stack.top() == 2);
        stack.restore();
        stack.restore();
        REQUIRE(stack.empty());
        REQUIRE_NOTHROW(stack.restore_pop_sequence());
        REQUIRE(stack.empty());
    }
}

TEST_CASE("FilteredSAGGenerator") {
    SECTION("Simple") {
        FilteredSAGGenerator generator(2, 1);
        SAGWithEndpoints graph(0, {});

        REQUIRE(generator.Yield(graph));
        REQUIRE(graph == SAGWithEndpoints(0, {1, 1, 2, 2}));
        REQUIRE(generator.Yield(graph));
        REQUIRE(graph == SAGWithEndpoints(0, {1, 2, 1, 2}));
        REQUIRE(generator.Yield(graph));
        REQUIRE(graph == SAGWithEndpoints(0, {1, 2, 2, 1}));
        REQUIRE_FALSE(generator.Yield(graph));
    
        generator = FilteredSAGGenerator(2, 2);
        REQUIRE(generator.Yield(graph));
        REQUIRE(graph == SAGWithEndpoints(0, {1, 2, 1, 2}));
        REQUIRE_FALSE(generator.Yield(graph));

        generator = FilteredSAGGenerator(2, 1, "??1?");
        REQUIRE(generator.Yield(graph));
        REQUIRE(graph == SAGWithEndpoints(0, {1, 1, 2, 2}));
        REQUIRE_FALSE(generator.Yield(graph));

        generator = FilteredSAGGenerator(2, 1, "??2?");
        REQUIRE(generator.Yield(graph));
        REQUIRE(graph == SAGWithEndpoints(0, {1, 2, 1, 2}));
        REQUIRE(generator.Yield(graph));
        REQUIRE(graph == SAGWithEndpoints(0, {1, 2, 2, 1}));
        REQUIRE_FALSE(generator.Yield(graph));

        generator = FilteredSAGGenerator(2, 1, "???1");
        REQUIRE_FALSE(generator.Yield(graph));

        REQUIRE_THROWS(FilteredSAGGenerator(2, 1, "?"));
    }

    SECTION("Complex") {
        auto count_instances = [&] (FilteredSAGGenerator generator, size_t assembly_number) {
            auto result = 0;
            SAGWithEndpoints graph(0, {});
            size_t actual_min = generator.GetSize();
            while(generator.Yield(graph)) {
                actual_min = std::min(actual_min, graph.MinSubword());
                if (assembly_number == 0 || assembly_number == An(graph)) {
                    ++result;
                }
            }
            REQUIRE(actual_min >= generator.GetMinSubword());
            return result;
        };
        CHECK(  count_instances(FilteredSAGGenerator(5 * 2 - 3, 2), 2) == 
                count_instances(FilteredSAGGenerator(2 - 1, 1), 0));
        CHECK(count_instances(FilteredSAGGenerator(5 * 2 - 2, 2), 2) == 49);
        CHECK(count_instances(FilteredSAGGenerator(6, 1, "111211221222"), 0) == 216); //3 4 3 3 2 1
        CHECK(count_instances(FilteredSAGGenerator(6, 1, "111?11221222"), 0) == 216);
        CHECK(count_instances(FilteredSAGGenerator(6, 2, "111211221222"), 0) == 72); //2 3 3 2 2 1
        CHECK(count_instances(FilteredSAGGenerator(6, 3, "111211221222"), 0) == 64); //2 ((3 3) - 1) 2 2 1
    }
}
