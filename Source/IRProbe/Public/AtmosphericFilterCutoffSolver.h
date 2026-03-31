#pragma once

constexpr double kPressureSeaLevelPascals = 101325.0;

// The solver for the atmospheric absorption cutoff solver is based on 
// Nic Taylor's implementation featured in Guy Somberg's Game Audio Programming 3 
class AtmosphericFilterCutoffSolver
{
public:
	AtmosphericFilterCutoffSolver(
		const double humidity_percent,
		const double temperature_fahrenheit,
		const double pressure_pascals = kPressureSeaLevelPascals);

	double Solve(
		const double distance,
		const double cutoff_gain = 3.0
	) const;

private:
	double nitrogen_relax_freq;
	double oxygen_relax_freq;

	// Pre-computed coefficients independent of the absorption
	// coefficient.
	double a1, a2, a3;
};
