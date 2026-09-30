#include "aiws/chunker.hpp"
#include "aiws/context_builder.hpp"
#include "aiws/processing_core.hpp"
#include "aiws/retrieval_strategy.hpp"

#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// Student-written M2 tests
//
// Add your own tests to this file. Your tests are part of the submitted work
// and are evaluated under Student-Written Testing & Validation.
//
// Do not modify tests/public_tests.cpp.
//
// Your tests should exercise important M2 behavior beyond the supplied public
// tests. Consider default compatibility, custom strategies, runtime dispatch,
// invalid configuration, ownership/move behavior, and component interactions.

class LastOnlyRetrieval final : public aiws::RetrievalStrategy {
public:
    std::vector<aiws::SearchResult> search(
        const std::string&,
        int k,
        const std::vector<aiws::Chunk>& chunks,
        const aiws::CorpusIndex&) const override {

        if (k <= 0 || chunks.empty()) {
            return {};
        }

        const aiws::Chunk& chunk =
            chunks.back();

        return {
            aiws::SearchResult{
                chunk.id,
                chunk.document_id,
                chunk.sequence,
                chunk.text,
                50.0,
                1
            }
        };
    }
};

void default_behavior_test() {
    using namespace aiws;

    Workspace workspace;

    workspace.add_document(
        Document(
            "d1",
            "One",
            "alpha alpha beta"));

    workspace.add_document(
        Document(
            "d2",
            "Two",
            "gamma alpha"));

    ProcessingCore core;
    core.rebuild(workspace);

    assert(core.chunk_count() == 2);

    assert(
        core.document_frequency(
            "ALPHA!") == 2);

    assert(
        core.term_frequency(
            "alpha",
            "d1#0") == 2);

    const std::vector<SearchResult> results =
        core.search(
            "alpha beta",
            10);

    assert(!results.empty());

    assert(
        results[0].document_id ==
        "d1");
}

void runtime_substitution_test() {
    using namespace aiws;

    Workspace workspace;

    workspace.add_document(
        Document(
            "d1",
            "One",
            "alpha only"));

    workspace.add_document(
        Document(
            "d2",
            "Two",
            "beta only"));

    ProcessingCore core(
        std::make_unique<Chunker>(),
        std::make_unique<LastOnlyRetrieval>(),
        std::make_unique<ContextBuilder>());

    core.rebuild(workspace);

    const std::vector<SearchResult> results =
        core.search(
            "alpha",
            1);

    assert(results.size() == 1);

    assert(
        results[0].document_id ==
        "d2");

    assert(
        results[0].score ==
        50.0);

    const std::vector<ContextItem> context =
        core.build_context(
            "alpha",
            1,
            2);

    assert(context.size() == 1);

    assert(
        context[0].document_id ==
        "d2");

    assert(
        context[0].text ==
        "beta only");
}

void invalid_configuration_test() {
    using namespace aiws;

    bool threw = false;

    try {
        ProcessingCore core(
            nullptr,
            std::make_unique<LastOnlyRetrieval>(),
            std::make_unique<ContextBuilder>());
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    threw = false;

    try {
        ProcessingCore core(
            std::make_unique<Chunker>(),
            nullptr,
            std::make_unique<ContextBuilder>());
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    threw = false;

    try {
        ProcessingCore core(
            std::make_unique<Chunker>(),
            std::make_unique<LastOnlyRetrieval>(),
            nullptr);
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);
}

void move_test() {
    using namespace aiws;

    Workspace workspace;

    workspace.add_document(
        Document(
            "d1",
            "One",
            "alpha"));

    workspace.add_document(
        Document(
            "d2",
            "Two",
            "beta"));

    ProcessingCore core(
        std::make_unique<Chunker>(),
        std::make_unique<LastOnlyRetrieval>(),
        std::make_unique<ContextBuilder>());

    core.rebuild(workspace);

    ProcessingCore moved(
        std::move(core));

    assert(
        moved.search(
            "alpha",
            1)[0].document_id ==
        "d2");

    ProcessingCore assigned;

    assigned =
        std::move(moved);

    assert(
        assigned.search(
            "alpha",
            1)[0].document_id ==
        "d2");
}

void failed_rebuild_test() {
    using namespace aiws;

    Workspace valid;

    valid.add_document(
        Document(
            "d1",
            "One",
            "alpha"));

    ProcessingCore core;
    core.rebuild(valid);

    Workspace duplicate;

    duplicate.add_document(
        Document(
            "same",
            "One",
            "beta"));

    duplicate.add_document(
        Document(
            "same",
            "Two",
            "gamma"));

    bool threw = false;

    try {
        core.rebuild(duplicate);
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    assert(
        core.document_frequency(
            "alpha") == 1);

    assert(
        core.chunk_count() == 1);
}

int main() {
    default_behavior_test();
    runtime_substitution_test();
    invalid_configuration_test();
    move_test();
    failed_rebuild_test();

    std::cout <<
        "Student tests passed\n";
    return 0;
}
