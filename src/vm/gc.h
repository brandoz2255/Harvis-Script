#ifndef HARVIS_VM_GC_H
#define HARVIS_VM_GC_H

#include "object.h"
#include <cstddef>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <algorithm>

namespace hs {

// Mark-and-sweep garbage collector with per-object allocation tracking.
// Each RuntimeObject is allocated with new and freed with delete during sweep.
// Thread-unsafe for now -- locks added later.
class GC {
public:
    static constexpr int DEFAULT_THRESHOLD = 1000;

    GC();
    ~GC();

    // --- Allocation tracking ------------------------------------------------
    // Register an allocation so the GC can track and free it later.
    void track(RuntimeObject* obj, size_t size);

    // --- Mark phase -------------------------------------------------------
    void markObject(RuntimeObject* obj);

    // Mark all roots: stack, globals, upvalues, modules, genericCache
    void markRoots(
        const std::vector<Value>& stack,
        const std::unordered_map<std::string, Value>& globals,
        const std::vector<std::shared_ptr<Upvalue>>& upvalues,
        const std::unordered_map<std::string, Value>& modules,
        const std::unordered_map<std::string, Value>& genericCache);

    // --- Sweep phase ------------------------------------------------------
    // Delete all untracked/unmarked objects and clear the allocation list.
    void sweep();

    // --- Full collection cycle --------------------------------------------
    void collect(
        const std::vector<Value>& stack,
        const std::unordered_map<std::string, Value>& globals,
        const std::vector<std::shared_ptr<Upvalue>>& upvalues,
        const std::unordered_map<std::string, Value>& modules,
        const std::unordered_map<std::string, Value>& genericCache);

    // --- Allocation bookkeeping -------------------------------------------
    bool countAllocation();
    void resetAllocationCount();

    int  getAllocationCount() const { return allocationCount; }
    void setThreshold(int t)        { threshold = t; }
    int  getThreshold() const       { return threshold; }

private:
    void markValue(const Value& val);
    void markReferences(RuntimeObject* obj);

    // --- Allocation list: tracks every GC-managed object -----------------
    struct AllocEntry {
        RuntimeObject* ptr;
        size_t size;
        bool  marked;
        AllocEntry(RuntimeObject* p, size_t s) : ptr(p), size(s), marked(false) {}
    };
    std::vector<AllocEntry> allocations;

    // --- Mark bit side table (RuntimeObject* -> bool) ---------------------
    std::unordered_set<RuntimeObject*> markedObjects;

    // --- Allocation counter -----------------------------------------------
    int allocationCount = 0;
    int threshold = DEFAULT_THRESHOLD;
};

} // namespace hs

#endif // HARVIS_VM_GC_H
