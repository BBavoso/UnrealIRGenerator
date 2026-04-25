#include "IR_Generator_DSP.h"
#include "SampleBuffer.h"
#include "DSP/Noise.h"
#include "AtmosphericFilterCutoffSolver.h"


Audio::TSampleBuffer<> IR_Generator_DSP::RunDSP(
	TArray<Impulse>& InImpulses,
	bool ApplyAcousticAbsorption,
	bool ApplyAtmosphericAbsorption)
{
	uint64 TotalLengthSamples = InImpulses.Last().StartSample + LengthSamples + 1;
	UE_LOG(LogTemp, Warning, TEXT("Should be %i samples"), TotalLengthSamples);

	TArray<TUniquePtr<PlayingImpulse>> PlayingImpulses;

	for (Impulse& Impulse : InImpulses)
	{
		PlayingImpulses.Emplace(MakeUnique<PlayingImpulse>(Impulse));
	}

	TArray<float> Buffer;
	Buffer.Init(0, TotalLengthSamples);

	for (TUniquePtr<PlayingImpulse>& PlayingImpulse : PlayingImpulses)
	{
		TArray<float> RenderedImpulse = PlayingImpulse->RenderImpulse(ApplyAcousticAbsorption,
		                                                              ApplyAtmosphericAbsorption);
		for (uint64 Clock = 0; Clock < LengthSamples; ++Clock)
		{
			Buffer[Clock + PlayingImpulse->StartSample] += RenderedImpulse[Clock];
		}
	}

	Audio::TSampleBuffer<> IntBuffer = Audio::TSampleBuffer(Buffer.GetData(), Buffer.Num(), 1, ImpulseSampleRate);

	return IntBuffer;
}

void Filters::ApplyFiltersToAudio(
	TArray<float>& Input,
	const MaterialCoefficients& DesiredGainValues,
	double DistanceMeters,
	bool ApplyAcousticAbsorption,
	bool ApplyAtmosphericAbsorption)
{
	if (ApplyAcousticAbsorption)
	{
		MaterialCoefficients AdjustedGainValues = GetAdjustedGainValues(DesiredGainValues);

		UE_LOG(LogTemp, Warning, TEXT("gain lin %f %f %f %f %f %f at time %f"), DesiredGainValues[0],DesiredGainValues[1] , DesiredGainValues[2], DesiredGainValues[3], DesiredGainValues[4], DesiredGainValues[5], static_cast<float>(DistanceMeters) / 343.0f);
		UE_LOG(LogTemp, Warning, TEXT("gain db %f %f %f %f %f %f at time %f"), AdjustedGainValues[0],AdjustedGainValues[1] , AdjustedGainValues[2], AdjustedGainValues[3], AdjustedGainValues[4], AdjustedGainValues[5], static_cast<float>(DistanceMeters) / 343.0f);

		Filter0.Init(ImpulseSampleRate, 1, Audio::EBiquadFilter::LowShelf, 125, 1, AdjustedGainValues[0]);
		Filter1.Init(ImpulseSampleRate, 1, Audio::EBiquadFilter::HighShelf, 125, 1, AdjustedGainValues[0]);
		Filter2.Init(ImpulseSampleRate, 1, Audio::EBiquadFilter::HighShelf, 250, 1, AdjustedGainValues[1]);
		Filter3.Init(ImpulseSampleRate, 1, Audio::EBiquadFilter::HighShelf, 500, 1, AdjustedGainValues[2]);
		Filter4.Init(ImpulseSampleRate, 1, Audio::EBiquadFilter::HighShelf, 1000, 1, AdjustedGainValues[3]);
		Filter5.Init(ImpulseSampleRate, 1, Audio::EBiquadFilter::HighShelf, 2000, 1, AdjustedGainValues[4]);
		Filter6.Init(ImpulseSampleRate, 1, Audio::EBiquadFilter::HighShelf, 4000, 1, AdjustedGainValues[5]);

		Filter0.SetEnabled(true);
		Filter1.SetEnabled(true);
		Filter2.SetEnabled(true);
		Filter3.SetEnabled(true);
		Filter4.SetEnabled(true);
		Filter5.SetEnabled(true);
		Filter6.SetEnabled(true);

		Filter0.ProcessAudio(Input.GetData(), Input.Num(), Input.GetData());
		Filter1.ProcessAudio(Input.GetData(), Input.Num(), Input.GetData());
		Filter2.ProcessAudio(Input.GetData(), Input.Num(), Input.GetData());
		Filter3.ProcessAudio(Input.GetData(), Input.Num(), Input.GetData());
		Filter4.ProcessAudio(Input.GetData(), Input.Num(), Input.GetData());
		Filter5.ProcessAudio(Input.GetData(), Input.Num(), Input.GetData());
		Filter6.ProcessAudio(Input.GetData(), Input.Num(), Input.GetData());
	}

	if (ApplyAtmosphericAbsorption)
	{
		AtmosphericFilterCutoffSolver solver = AtmosphericFilterCutoffSolver(HumidityPercent, TemperatureFahrenheit);
		auto lowPassFrequency = solver.Solve(DistanceMeters);

		AtmosphericLowpass.Init(ImpulseSampleRate, 1, Audio::EBiquadFilter::Lowpass, lowPassFrequency);
		AtmosphericLowpass.SetEnabled(true);

		AtmosphericLowpass.ProcessAudio(Input.GetData(), Input.Num(), Input.GetData());
	}
}

TArray<float> PlayingImpulse::RenderImpulse(bool ApplyAcousticAbsorption, bool ApplyAtmosphericAbsorption)
{
	Audio::FWhiteNoise Noise{};

	TArray<float> Buffer;
	Buffer.Init(0, LengthSamples);

	for (uint64 Clock = 0; Clock < LengthSamples; ++Clock)
	{
		Buffer[Clock] = Noise.Generate();
		Buffer[Clock] *= DistanceBasedGain;
		Buffer[Clock] *= CalculateFadeGain(Clock);
	}

	EQs.ApplyFiltersToAudio(
		Buffer,
		DesiredGainValues,
		DistanceTraveledMeters,
		ApplyAcousticAbsorption,
		ApplyAtmosphericAbsorption
	);

	return Buffer;
}

void Impulse::MergeWith(const Impulse& Other)
{
	DistanceTraveled = FMath::Min(DistanceTraveled, Other.DistanceTraveled);

	for (size_t i = 0; i < NumFrequencyBands; i++)
	{
		DesiredGainValues[i] = FMath::Max(DesiredGainValues[i], Other.DesiredGainValues[i]);
	}
}
