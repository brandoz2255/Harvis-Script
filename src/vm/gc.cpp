#include "gc.h"
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace hs {

// ---------------------------------------------------------------------------
// Construction / Destruction
// ---------------------------------------------------------------------------

GC::GC() : allocationCount(0), threshold(DEFAULT_THRESHOLD) {}

GC::~GC() {
    // Free any remaining tracked allocations
    for (auto& entry : allocations) {
        delete entry.ptr;
    }
    allocations.clear();
    markedObjects.clear();
}

// ---------------------------------------------------------------------------
// Allocation tracking
// ---------------------------------------------------------------------------

void GC::track(RuntimeObject* obj, size_t size) {
    if (!obj) return;
    allocations.emplace_back(obj, size);
}

// ---------------------------------------------------------------------------
// Allocation bookkeeping
// ---------------------------------------------------------------------------

bool GC::countAllocation() {
    allocationCount++;
    return allocationCount >= threshold;
}

void GC::resetAllocationCount() {
    allocationCount = 0;
}

// ---------------------------------------------------------------------------
// Mark phase
// ---------------------------------------------------------------------------

void GC::markObject(RuntimeObject* obj) {
    if (!obj) return;

    // Already marked?
    if (markedObjects.count(obj)) return;

    // Mark in side table
    markedObjects.insert(obj);

    // Mark in allocation list entry
    for (auto& entry : allocations) {
        if (entry.ptr == obj) {
            entry.marked = true;
            break;
        }
    }

    // Mark references
    markReferences(obj);
}

void GC::markValue(const Value& val) {
    RuntimeObject* obj = val.asObject();
    if (obj) {
        markObject(obj);
    }
}

void GC::markReferences(RuntimeObject* obj) {
    switch (obj->type) {
        case ObjectType::Closure: {
            auto* closure = static_cast<Closure*>(obj);
            if (closure->function) markObject(closure->function);
            for (auto& uv : closure->upvalues) {
                if (uv->closed) markValue(*uv->closed);
            }
            break;
        }
        case ObjectType::Class: {
            auto* klass = static_cast<ClassObj*>(obj);
            if (klass->superClass) markObject(klass->superClass);
            for (auto& kv : klass->methods) {
                markObject(kv.second);
            }
            break;
        }
        case ObjectType::Instance: {
            auto* inst = static_cast<Instance*>(obj);
            if (inst->klass) markObject(inst->klass);
            for (auto& kv : inst->fields) {
                markValue(kv.second);
            }
            break;
        }
        case ObjectType::BoundMethod: {
            auto* bm = static_cast<BoundMethod*>(obj);
            if (bm->instance) markObject(bm->instance);
            if (bm->method) markObject(bm->method);
            break;
        }
        case ObjectType::Array: {
            auto* arr = static_cast<ArrayObj*>(obj);
            for (auto& elem : arr->elements) {
                markValue(elem);
            }
            break;
        }
        case ObjectType::Map: {
            auto* map = static_cast<MapObj*>(obj);
            for (auto& kv : map->entries) {
                markValue(kv.second);
            }
            break;
        }
        case ObjectType::ArrayMethod: {
            auto* am = static_cast<ArrayMethodObj*>(obj);
            if (am->array) markObject(am->array);
            break;
        }
        // Function, Native, String, Channel, Mutex, WaitGroup -- no GC refs
        default:
            break;
    }
}

void GC::markRoots(
    const std::vector<Value>& stack,
    const std::unordered_map<std::string, Value>& globals,
    const std::vector<std::shared_ptr<Upvalue>>& upvalues,
    const std::unordered_map<std::string, Value>& modules,
    const std::unordered_map<std::string, Value>& genericCache)
{
    for (auto& val : stack) {
        markValue(val);
    }

    for (auto& kv : globals) {
        markValue(kv.second);
    }

    for (auto& uv : upvalues) {
        if (uv->closed) markValue(*uv->closed);
    }

    for (auto& kv : modules) {
        markValue(kv.second);
    }

    for (auto& kv : genericCache) {
        markValue(kv.second);
    }
}

// ---------------------------------------------------------------------------
// Sweep phase
// ---------------------------------------------------------------------------

void GC::sweep() {
    // Delete every unmarked allocation
    std::vector<AllocEntry> survivors;
    survivors.reserve(allocations.size());

    for (auto& entry : allocations) {
        if (entry.marked) {
            survivors.push_back(entry);
            entry.marked = false;  // reset mark for next cycle
        } else {
            delete entry.ptr;
        }
    }

    allocations = std::move(survivors);
    markedObjects.clear();
}

// ---------------------------------------------------------------------------
// Full collection cycle
// ---------------------------------------------------------------------------

void GC::collect(
    const std::vector<Value>& stack,
    const std::unordered_map<std::string, Value>& globals,
    const std::vector<std::shared_ptr<Upvalue>>& upvalues,
    const std::unordered_map<std::string, Value>& modules,
    const std::unordered_map<std::string, Value>& genericCache)
{
    markRoots(stack, globals, upvalues, modules, genericCache);
    sweep();
    resetAllocationCount();
}

} // namespace hs
