#if ! CHOWDSP_NO_XSIMD

#pragma once

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsign-conversion")

namespace chowdsp
{
/**
 * FIR Filter processor that is a bit more optimized
 * than the JUCE one.
 *
 * Still, for filters with an order 512 and higher,
 * juce::dsp::Convolution is probably going to be faster.
 */
template <typename FloatType>
class FIRFilter
{
public:
    /** Default constructor */
    FIRFilter();

    /** Constructs a filter with a given order */
    explicit FIRFilter (int filterOrder);

    FIRFilter (FIRFilter&&) noexcept = default;
    FIRFilter& operator= (FIRFilter&&) noexcept = default;

    /**
     * Changes the order of this filter.
     *
     * Note that this will clear any coefficients which
     * had previously been loaded.
     */
    void setOrder (int newOrder);

    /** Returns the current filter order */
    [[nodiscard]] int getOrder() const noexcept { return order; }

    /** Prepares the filter for processing a new number of channels */
    void prepare (int numChannels);

    /** Reset filter state */
    void reset() noexcept;

    /**
     * Copies a new set of coefficients to use for the filter.
     * The length of data pointed to by the incoming data,
     * must be exactly the same as the filter order.
     */
    void setCoefficients (const FloatType* coeffsData);

    /** Process a single sample */
    inline FloatType processSample (FloatType x, int channel = 0) noexcept
    {
        return processSampleInternal (x, state[channel].data(), coefficients.data(), zPtr[channel], order, paddedOrder);
    }

    /** Process block of samples */
    void processBlock (FloatType* block, const int numSamples, const int channel = 0) noexcept
    {
        auto* z = state[channel].data();
        const auto* h = coefficients.data();
        chowdsp::ScopedValue zPtrLocal { zPtr[channel] };

        for (int n = 0; n < numSamples; ++n)
            block[n] = processSampleInternal (block[n], z, h, zPtrLocal.get(), order, paddedOrder);
    }

    /** Process block of samples */
    void processBlock (const chowdsp::BufferView<FloatType>& block) noexcept
    {
        const auto numChannels = (int) block.getNumChannels();
        const auto numSamples = (int) block.getNumSamples();
        for (int channel = 0; channel < numChannels; ++channel)
            processBlock (block.getWritePointer (channel), numSamples, channel);
    }

    /**
     * Pushes a block of samples into the filter state without processing.
     * This can be useful for "bypassing" the filter.
     */
    void processBlockBypassed (const FloatType* block, const int numSamples, const int channel = 0) noexcept
    {
        auto* z = state[channel].data();
        chowdsp::ScopedValue zPtrLocal { zPtr[channel] };

        for (int n = 0; n < numSamples; ++n)
            processSampleInternalBypassed (block[n], z, zPtrLocal.get(), order);
    }

    /**
     * Pushes a block of samples into the filter state without processing.
     * This can be useful for "bypassing" the filter.
     */
    void processBlockBypassed (const chowdsp::BufferView<FloatType>& block) noexcept
    {
        const auto numChannels = (int) block.getNumChannels();
        const auto numSamples = (int) block.getNumSamples();
        for (int channel = 0; channel < numChannels; ++channel)
            processBlockBypassed (block.getReadPointer (channel), numSamples, channel);
    }

private:
    static FloatType processSampleInternal (FloatType x, FloatType* z, const FloatType* h, int& zPtr, int order, int paddedOrder) noexcept;
    static FloatType simdInnerProduct (const FloatType* z, const FloatType* h, int N);
    static void processSampleInternalBypassed (FloatType x, FloatType* z, int& zPtr, int order) noexcept;

    int order = 0;
    int paddedOrder = 0;
    std::vector<int> zPtr;

    std::vector<FloatType, xsimd::default_allocator<FloatType>> coefficients;
    std::vector<std::vector<FloatType>> state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FIRFilter)
};
} // namespace chowdsp

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#endif // ! CHOWDSP_NO_XSIMD
