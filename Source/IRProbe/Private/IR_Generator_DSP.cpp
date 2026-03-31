#include "IR_Generator_DSP.h"
#include "SampleBuffer.h"
#include "DSP/Noise.h"
#include "AtmosphericFilterCutoffSolver.h"


Audio::TSampleBuffer<> IR_Generator_DSP::RunDSP(TArray<Impulse>& InImpulses)
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
		TArray<float> RenderedImpulse = PlayingImpulse->RenderImpulse();
		for (uint64 Clock = 0; Clock < LengthSamples; ++Clock)
		{
			Buffer[Clock + PlayingImpulse->StartSample] += RenderedImpulse[Clock];
		}
	}

	Audio::TSampleBuffer<> IntBuffer = Audio::TSampleBuffer(Buffer.GetData(), Buffer.Num(), 1, SampleRate);

	return IntBuffer;
}

void Filters::ApplyFiltersToAudio(TArray<float>& Input, const MaterialCoefficients& DesiredGainValues, double Distance)
{
	Filter0.Init(SampleRate, 1, Audio::EBiquadFilter::LowShelf, 125);
	Filter1.Init(SampleRate, 1, Audio::EBiquadFilter::HighShelf, 125);
	Filter2.Init(SampleRate, 1, Audio::EBiquadFilter::HighShelf, 250);
	Filter3.Init(SampleRate, 1, Audio::EBiquadFilter::HighShelf, 500);
	Filter4.Init(SampleRate, 1, Audio::EBiquadFilter::HighShelf, 1000);
	Filter5.Init(SampleRate, 1, Audio::EBiquadFilter::HighShelf, 2000);
	Filter6.Init(SampleRate, 1, Audio::EBiquadFilter::HighShelf, 4000);
	
	
	MaterialCoefficients AdjustedGainValues = GetAdjustedGainValues(DesiredGainValues);
	
	// UE_LOG(LogTemp, Warning, TEXT("125 gain db %f"), AdjustedGainValues[0]);
	
	// Filter0.SetGainDB(AdjustedGainValues[0]);
	// Filter1.SetGainDB(AdjustedGainValues[0]);
	// Filter2.SetGainDB(AdjustedGainValues[1]);
	// Filter3.SetGainDB(AdjustedGainValues[2]);
	// Filter4.SetGainDB(AdjustedGainValues[3]);
	// Filter5.SetGainDB(AdjustedGainValues[4]);
	// Filter6.SetGainDB(AdjustedGainValues[5]);
	//
	// Filter0.SetEnabled(true);
	// Filter1.SetEnabled(true);
	// Filter2.SetEnabled(true);
	// Filter3.SetEnabled(true);
	// Filter4.SetEnabled(true);
	// Filter5.SetEnabled(true);
	// Filter6.SetEnabled(true);
	//
	// Filter0.ProcessAudio(Input.GetData(), Input.Num(), Input.GetData());
	// Filter1.ProcessAudio(Input.GetData(), Input.Num(), Input.GetData());
	// Filter2.ProcessAudio(Input.GetData(), Input.Num(), Input.GetData());
	// Filter3.ProcessAudio(Input.GetData(), Input.Num(), Input.GetData());
	// Filter4.ProcessAudio(Input.GetData(), Input.Num(), Input.GetData());
	// Filter5.ProcessAudio(Input.GetData(), Input.Num(), Input.GetData());
	// Filter6.ProcessAudio(Input.GetData(), Input.Num(), Input.GetData());
	
	
	AtmosphericFilterCutoffSolver solver = AtmosphericFilterCutoffSolver(HumidityPercent, TemperatureFahrenheit);
	auto lowPassFrequency = solver.Solve(Distance);
	
	AtmosphericLowpass.Init(SampleRate, 1, Audio::EBiquadFilter::Lowpass, lowPassFrequency);
	AtmosphericLowpass.SetEnabled(true);
	
	AtmosphericLowpass.ProcessAudio(Input.GetData(), Input.Num(), Input.GetData());
}

TArray<float> PlayingImpulse::RenderImpulse()
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

	EQs.ApplyFiltersToAudio(Buffer, DesiredGainValues, DistanceTraveledMeters);

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
