// See the file "COPYING" in the main distribution directory for copyright.

#pragma once

#include <prometheus/counter.h>
#include <prometheus/family.h>
#include <cstdint>
#include <initializer_list>
#include <memory>

#include "zeek/Span.h"
#include "zeek/telemetry/MetricFamily.h"
#include "zeek/telemetry/Utils.h"
#include "zeek/telemetry/telemetry.bif.h"

namespace zeek::telemetry {

/**
 * A handle to a metric that can only go up.
 */
class Counter {
public:
    static inline const char* OpaqueName = "CounterMetricVal";

    using Handle = prometheus::Counter;
    using FamilyType = prometheus::Family<Handle>;

    explicit Counter(FamilyType* family, const prometheus::Labels& labels,
                     prometheus::CollectCallbackPtr callback = nullptr) noexcept;

    /**
     * Increments the value by 1.
     */
    void Inc() noexcept { Inc(1); }

    /**
     * Increments the value by @p amount.
     * @pre `amount >= 0`
     */
    void Inc(double amount) noexcept { handle.Increment(amount); }

    /**
     * Increments the value by 1.
     * @return The new value.
     */
    double operator++() noexcept {
        Inc(1);
        return Value();
    }

    double Value() const noexcept;

    bool operator==(const Counter& rhs) const noexcept { return &handle == &rhs.handle; }
    bool operator!=(const Counter& rhs) const noexcept { return &handle != &rhs.handle; }

    bool CompareLabels(const prometheus::Labels& lbls) const { return labels == lbls; }

private:
    Handle& handle;
    prometheus::Labels labels;
    bool has_callback = false;
};

using CounterPtr = std::shared_ptr<Counter>;

class CounterFamily : public MetricFamily, public std::enable_shared_from_this<CounterFamily> {
public:
    static inline const char* OpaqueName = "CounterMetricFamilyVal";

    CounterFamily(prometheus::Family<prometheus::Counter>* family, Span<const std::string_view> labels)
        : MetricFamily(labels), family(family) {}

    /**
     * Returns the metrics handle for given labels, creating a new instance
     * lazily if necessary.
     */
    CounterPtr GetOrAdd(Span<const LabelView> labels, prometheus::CollectCallbackPtr callback = nullptr);

    /**
     * @copydoc GetOrAdd
     */
    CounterPtr GetOrAdd(std::initializer_list<LabelView> labels, prometheus::CollectCallbackPtr callback = nullptr);

    zeek_int_t MetricType() const noexcept override { return BifEnum::Telemetry::MetricType::COUNTER; }

private:
    prometheus::Family<prometheus::Counter>* family;
    std::vector<CounterPtr> counters;
};

using CounterFamilyPtr = std::shared_ptr<CounterFamily>;

} // namespace zeek::telemetry
