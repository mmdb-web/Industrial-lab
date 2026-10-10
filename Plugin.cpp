#include <JuceHeader.h>

class IndustrialLabAudioProcessor final : public juce::AudioProcessor
{
public:
    IndustrialLabAudioProcessor();
    ~IndustrialLabAudioProcessor() override = default;
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Industrial Lab"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    juce::AudioProcessorValueTreeState parameters;
private:
    static juce::AudioProcessorValueTreeState::ParameterLayout makeParameters();
    double rate = 44100.0, phase = 0.0, age = 0.0;
    float velocity = 1.0f;
    bool playing = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IndustrialLabAudioProcessor)
};

#include <cmath>

IndustrialLabAudioProcessor::IndustrialLabAudioProcessor()
: juce::AudioProcessor(juce::AudioProcessor::BusesProperties()
      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
  parameters(*this, nullptr, "PARAMETERS", makeParameters()) {}

juce::AudioProcessorValueTreeState::ParameterLayout IndustrialLabAudioProcessor::makeParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"pitch",1}, "Final Frequency (Hz)",
        juce::NormalisableRange<float>(35.0f, 100.0f, 0.1f), 52.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"sweep",1}, "Pitch Sweep (Hz)",
        juce::NormalisableRange<float>(0.0f, 700.0f, 1.0f), 180.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"sweeptime",1}, "Sweep Time (ms)",
        juce::NormalisableRange<float>(5.0f, 160.0f, 0.5f), 35.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"decay",1}, "Decay (ms)",
        juce::NormalisableRange<float>(40.0f, 1300.0f, 1.0f), 350.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"drive",1}, "Drive (dB)",
        juce::NormalisableRange<float>(0.0f, 30.0f, 0.1f), 10.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"output",1}, "Output (dB)",
        juce::NormalisableRange<float>(-30.0f, 0.0f, 0.1f), -9.0f));
    return layout;
}

void IndustrialLabAudioProcessor::prepareToPlay(double sampleRate, int)
{
    rate = sampleRate > 0.0 ? sampleRate : 44100.0;
    phase = age = 0.0;
    playing = false;
}

void IndustrialLabAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    const float base = parameters.getRawParameterValue("pitch")->load();
    const float sweep = parameters.getRawParameterValue("sweep")->load();
    const double sweepTime = parameters.getRawParameterValue("sweeptime")->load() * 0.001;
    const double decay = parameters.getRawParameterValue("decay")->load() * 0.001;
    const float inputGain = juce::Decibels::decibelsToGain(parameters.getRawParameterValue("drive")->load());
    const float outGain = juce::Decibels::decibelsToGain(parameters.getRawParameterValue("output")->load());
    auto iterator = midi.cbegin();
    const auto end = midi.cend();
    const int channels = buffer.getNumChannels();
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        while (iterator != end && (*iterator).samplePosition <= i)
        {
            const auto msg = (*iterator).getMessage();
            if (msg.isNoteOn())
            {
                age = 0.0; phase = 0.0;
                velocity = msg.getFloatVelocity();
                playing = true;
            }
            ++iterator;
        }
        float sample = 0.0f;
        if (playing)
        {
            const double freq = base + sweep * std::exp(-age / juce::jmax(0.001, sweepTime));
            phase += juce::MathConstants<double>::twoPi * freq / rate;
            if (phase >= juce::MathConstants<double>::twoPi)
                phase = std::fmod(phase, juce::MathConstants<double>::twoPi);
            const double env = (1.0 - std::exp(-age / 0.0015)) * std::exp(-age / juce::jmax(0.01, decay));
            const float raw = static_cast<float>(std::sin(phase) * env * velocity);
            const float normalized = std::tanh(inputGain * raw) / juce::jmax(1.0f, std::tanh(inputGain));
            sample = normalized * outGain;
            age += 1.0 / rate;
            if (age > juce::jmax(0.2, decay * 9.0)) playing = false;
        }
        for (int ch = 0; ch < channels; ++ch) buffer.setSample(ch, i, sample);
    }
    midi.clear();
}

juce::AudioProcessorEditor* IndustrialLabAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}
void IndustrialLabAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto xml = parameters.copyState().createXml();
    if (xml != nullptr) copyXmlToBinary(*xml, dest);
}
void IndustrialLabAudioProcessor::setStateInformation(const void* data, int size)
{
    auto xml = getXmlFromBinary(data, size);
    if (xml != nullptr && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new IndustrialLabAudioProcessor(); }
