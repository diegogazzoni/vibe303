#pragma once
#include <cmath>

//==============================================================================
// AcidEnvelope
// AD (Attack-Decay) envelope faithful to the TB-303.
// The 303 has a virtually instantaneous attack; only Decay is user-controllable.
// Used for both the filter envelope and the amplitude envelope.
//==============================================================================
class AcidEnvelope
{
public:
    enum class State { Idle, Attack, Decay, Sustain };

    AcidEnvelope() = default;

    void prepare (double sampleRate_)
    {
        sampleRate = sampleRate_;
        currentLevel = 0.0f;
        state = State::Idle;
    }

    // decaySeconds: the user-controlled Decay knob value (0.002 – 2.0 s)
    void setDecay (float decaySeconds)
    {
        // Time constant for exponential decay: coeff = e^(-1 / (time * sr))
        decayCoeff = std::exp (-1.0f / (decaySeconds * (float) sampleRate));
    }

    // attackSeconds: near-instantaneous for amp env, slightly longer for filter env
    void setAttack (float attackSeconds)
    {
        if (attackSeconds <= 0.0001f)
            attackCoeff = 0.0f; // instant
        else
            attackCoeff = std::exp (-1.0f / (attackSeconds * (float) sampleRate));
    }

    void noteOn()
    {
        state = State::Attack;
        currentLevel = 0.0f;
    }

    void noteOff()
    {
        // The 303 envelope is AD — the decay runs to zero regardless of note off.
        // Note off just stops the attack and triggers decay if we were still attacking.
        if (state == State::Attack || state == State::Sustain)
            state = State::Decay;
    }

    float process()
    {
        switch (state)
        {
            case State::Idle:
                currentLevel = 0.0f;
                break;

            case State::Attack:
                // Instant attack: jump to 1.0 in one sample (attackCoeff == 0)
                if (attackCoeff == 0.0f)
                {
                    currentLevel = 1.0f;
                    state = State::Decay;
                }
                else
                {
                    currentLevel = 1.0f - attackCoeff * (1.0f - currentLevel);
                    if (currentLevel >= 0.999f)
                    {
                        currentLevel = 1.0f;
                        state = State::Decay;
                    }
                }
                break;

            case State::Decay:
                currentLevel *= decayCoeff;
                if (currentLevel < 0.0001f)
                {
                    currentLevel = 0.0f;
                    state = State::Idle;
                }
                break;

            case State::Sustain:
                // Used when gate is held — stays at peak until noteOff
                currentLevel = 1.0f;
                break;
        }

        return currentLevel;
    }

    float getLevel() const { return currentLevel; }
    bool  isActive() const { return state != State::Idle; }

private:
    double sampleRate    = 44100.0;
    float  currentLevel  = 0.0f;
    float  decayCoeff    = 0.999f;
    float  attackCoeff   = 0.0f;   // default: instantaneous attack
    State  state         = State::Idle;
};
