#pragma once

#include "SampleBuffer.h"
#include "DSP/Filter.h"

constexpr int32 ImpulseSampleRate = 48000;

// Size and shape of impulses
static float LengthSeconds = 0.05;
static float FadeInTimeSeconds = 0.002;
static float FadeOutTimeSeconds = 0.002;

constexpr uint NumFrequencyBands = 6;

using MaterialCoefficients = TStaticArray<float, NumFrequencyBands>;

inline MaterialCoefficients GetAdjustedGainValues(
	const MaterialCoefficients& DesiredGainValues)
{
	MaterialCoefficients ReturnValue;
	ReturnValue[0] = Audio::ConvertToDecibels(DesiredGainValues[0]);
	ReturnValue[0] = FMath::Clamp(ReturnValue[0], -32.0f, 32.0f);
	float AccumulatedGain = ReturnValue[0];
	for (int i = 1; i < NumFrequencyBands; i++)
	{
		ReturnValue[i] = 2.0f * (Audio::ConvertToDecibels(DesiredGainValues[i]) - AccumulatedGain);
		ReturnValue[i] = FMath::Clamp(ReturnValue[i], -32.0f, 32.0f);
		AccumulatedGain += ReturnValue[i];
	}
	return ReturnValue;
}

constexpr uint64 GetSamplesFromSeconds(const float Seconds)
{
	return Seconds * ImpulseSampleRate;
}

inline uint64 LengthSamples = GetSamplesFromSeconds(LengthSeconds);
inline uint64 FadeInTimeSamples = GetSamplesFromSeconds(FadeInTimeSeconds);
inline uint64 FadeOutTimeSamples = GetSamplesFromSeconds(FadeOutTimeSeconds);
inline uint64 FadeOutStart = LengthSamples - FadeOutTimeSamples;

class Filters
{
public:
	void ApplyFiltersToAudio(TArray<float>& Input, const MaterialCoefficients& DesiredGainValues, double DistanceMeters, bool ApplyAcousticAbsorption, bool
	                         ApplyAtmosphericAbsorption);

private:
	Audio::FBiquadFilter Filter0;
	Audio::FBiquadFilter Filter1;
	Audio::FBiquadFilter Filter2;
	Audio::FBiquadFilter Filter3;
	Audio::FBiquadFilter Filter4;
	Audio::FBiquadFilter Filter5;
	Audio::FBiquadFilter Filter6;
	Audio::FBiquadFilter AtmosphericLowpass;

public:
	Filters()
	{
	}
};

struct Impulse
{
	uint64 StartSample = 0;
	MaterialCoefficients DesiredGainValues;
	float DistanceTraveled = 0.0f;

	auto operator<=>(const Impulse& Other) const
	{
		return DistanceTraveled <=> Other.DistanceTraveled;
	}

	void MergeWith(const Impulse& Other);
	float GetDelaySeconds() const;
	bool Invalid = false;

	Impulse()
	{
		for (int i = 0; i < NumFrequencyBands; i++)
		{
			DesiredGainValues[i] = 1.0f;
		}
	}
};

constexpr float GetDistanceAttenuationDB(float DistanceMeters)
{
	constexpr float MinDistance = 100.0f;
	if (DistanceMeters < MinDistance)
		return 0.0f;
	return log2f(DistanceMeters / MinDistance) * -6.0f;
}

struct PlayingImpulse
{
	TArray<float> RenderImpulse(bool ApplyAcousticAbsorption, bool ApplyAtmosphericAbsorption);



	PlayingImpulse(const Impulse& Impulse) :
		StartSample(Impulse.StartSample),
		DesiredGainValues(Impulse.DesiredGainValues),
		DistanceTraveledMeters(Impulse.DistanceTraveled / 100.0f)
	{
		DistanceBasedGain = Audio::ConvertToLinear(GetDistanceAttenuationDB(Impulse.DistanceTraveled));
	}


	float CalculateFadeGain(uint64 CurrentSample)
	{
		if (CurrentSample < FadeInTimeSamples)
		{
			return static_cast<float>(CurrentSample) / static_cast<float>(FadeInTimeSamples);
		}
		if (CurrentSample >= FadeInTimeSamples && CurrentSample < FadeOutStart)
		{
			return 1.0f;
		}
		if (CurrentSample >= FadeOutStart && CurrentSample < LengthSamples)
		{
			return
				static_cast<float>(LengthSamples - CurrentSample)
				/ static_cast<float>(FadeOutTimeSamples);
		}

		return 0.0f;
	}

	uint64 StartSample;

private:
	Filters EQs;

	float DistanceBasedGain;

	TArray<float> RenderedNoise;

	MaterialCoefficients DesiredGainValues;

	float DistanceTraveledMeters;
};

class IR_Generator_DSP
{
public:
	static Audio::TSampleBuffer<> RunDSP(TArray<Impulse>& InImpulses, bool ApplyAcousticAbsorption, bool ApplyAtmosphericAbsorption);
};
