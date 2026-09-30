# M2 Technical & Design Understanding

This file is an assessed technical-understanding artifact, not ordinary project documentation.
Answer all four questions using your own submitted implementation. Concise answers are acceptable when they are technically correct and specific.

Generic descriptions of C++ concepts or restatements of the assignment that do not identify and explain corresponding parts of your code will receive limited credit.

## 1. Polymorphism and dynamic dispatch - 1.5 points

Identify one place in your M2 implementation where runtime polymorphism occurs. Name the relevant base interface, derived implementation, and `ProcessingCore` function involved. Trace the call from `ProcessingCore` to the selected strategy implementation and explain why the derived implementation is invoked.

Then explain what would change if the relevant operation were not declared `virtual`.

Runtime polymorphism occurs in ProcessingCore::search(). The base interface is RetrievalStrategy. The default derived implementation is RetrievalEngine, and my runtime_substitution_test() uses LastOnlyRetrieval as another derived implementation. ProcessingCore::search() calls impl_->retrieval->search(...). Since search() is virtual, C++ uses the actual object stored in impl_->retrieval to decide which function to call. With the default constructor, RetrievalEngine::search() runs. With my custom test configuration, LastOnlyRetrieval::search() runs instead. If search() were not virtual, a call through the RetrievalStrategy pointer would not dynamically select the derived implementation, so runtime substitution would not work correctly.

## 2. Ownership and lifetime - 1.5 points

Identify where one of the strategy objects is created, where ownership is transferred, and which object ultimately owns it. Explain how `std::unique_ptr` represents that ownership relationship and when the strategy object is destroyed.

Also explain why `ProcessingCore` is move-only and why the strategy base classes require virtual destructors.

In runtime_substitution_test(), a LastOnlyRetrieval object is created using std::make_unique<LastOnlyRetrieval>() and passed to the configurable ProcessingCore constructor. Inside that constructor, std::move(retrieval) transfers ownership into impl_->retrieval. The ProcessingCore::Impl object then owns the retrieval strategy through a std::unique_ptr<RetrievalStrategy>. std::unique_ptr represents exclusive ownership, so only one owner is responsible for that strategy object. When ProcessingCore and its Impl are destroyed, the unique_ptr is destroyed and automatically destroys the strategy object. ProcessingCore is move only because its strategy objects have unique ownership. Moving transfers that ownership, while copying is disabled. The strategy base classes need virtual destructors so that a derived object such as RetrievalEngine or LastOnlyRetrieval is completely destroyed when it is owned through a base class pointer.

## 3. Architecture, extensibility, and M1 compatibility - 1.5 points

Explain one specific architectural decision in your M2 implementation that makes the processing system extensible while preserving M1 behavior.

Identify the classes or interfaces involved and explain both:
- how the default configuration preserves M1 behavior; and
- how a different implementation can be substituted without changing the normal `ProcessingCore` API.

Include one plausible design alternative and explain why the M2 design is preferable for this milestone. The alternative does not need to be something you actually implemented.

A main architectural decision in M2 is that ProcessingCore::Impl stores the three processing roles through ChunkingStrategy, RetrievalStrategy, and ContextStrategy instead of depending only on concrete processing classes. The default ProcessingCore constructor still creates Chunker, RetrievalEngine, and ContextBuilder, so the normal M1 processing behavior is preserved. The configurable constructor can instead receive different implementations of the same strategy interfaces. ProcessingCore can still use the same rebuild(), search(), and build_context() API because those functions call the strategy interfaces. One alternative would be to keep only the concrete classes and add conditionals inside ProcessingCore to select different behaviors. That would require ProcessingCore to be changed whenever a new processing implementation was added. The strategy design is preferable because each processing role can be substituted independently while keeping the rest of ProcessingCore unchanged.

## 4. Testing and defect reasoning - 1.5 points

Select one meaningful test from your `tests/student_tests.cpp`.

Explain:
- what M2 requirement the test validates;
- what specific implementation defect the test could detect; and
- why your test provides useful evidence beyond simply rerunning the supplied public tests.

If your test uses a custom strategy, explain how its observable behavior demonstrates that `ProcessingCore` is actually using runtime substitution.

My runtime_substitution_test() validates that ProcessingCore actually uses a caller supplied strategy at runtime. The test creates a custom LastOnlyRetrieval strategy that deliberately returns the last chunk instead of using the normal retrieval behavior. The workspace contains d1 with "alpha only" and d2 with "beta only". The test searches for "alpha", but LastOnlyRetrieval intentionally returns d2 with a score of 50.0. The test checks for that result and also checks that build_context() produces context from d2. This test could detect a defect where ProcessingCore accepts a custom RetrievalStrategy but still uses a hard coded RetrievalEngine. In that case, the result would follow the normal retrieval behavior instead of returning d2, and the test would fail. This provides useful evidence beyond only rerunning the supplied public tests because it uses my own derived strategy with intentionally different behavior. The observable result shows that ProcessingCore is actually dispatching through the RetrievalStrategy interface and using the runtime object that was supplied.
