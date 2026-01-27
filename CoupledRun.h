#pragma once

struct CoupledRunI32
{
	

public:
	int s0;
	int m0;

	// pitch = pitchN / pitchD steps per unit (eg 2/3 instead of 0.6666
	// pitch in µ/rev, same as leadscrewPitchUM

	int pitchN;
	int pitchD;

	int mRemain = 0; // for accumulating fractional steps

	CoupledRunI32(int spndlEnc, int motorCount, int pN, int pD)
	{
		s0 = spndlEnc;
		m0 = motorCount;

		pitchN = pN * motorStepsPerRev;
		pitchD = pD * leadscrewPitchUM * spindlePulsesPerRev;
	}

	int getTargetMotorCount(int spndlEnc)
	{
						
		int sd = (spndlEnc - s0);
		int motorSteps = m0 + (sd * pitchN / pitchD);
		
		// accumulate remainder (this is all int maths)
		mRemain += sd * pitchN % pitchD;

		return motorSteps;
	}
};



struct CoupledRunF32
{
public:
	int s0, sLast;
	int m0;
		
	float K; // coupling ratio: motor steps per spindle encoder pulse

	float eAvg = 0.999f;
	float vAvg = 0.0f;

	CoupledRunF32(int spndlEnc, int motorCount, int pN, int pD)
	{
		s0 = spndlEnc;
		sLast = spndlEnc;
		

		m0 = motorCount;
		K = ((float)pN * motorStepsPerRev) / ((float)pD * leadscrewPitchUM * spindlePulsesPerRev);
	}


	int getTargetMotorCount(int spndlEnc, ulong dtMicros)
	{
		int sd = (spndlEnc - s0);
		int motorSteps = m0 + (int)(sd * K);
		
		return motorSteps;
	}

	float updStepperSpeed(int spndlEnc, ulong dtMicros)
	{
		// velocity diff
		float vel = (float)((spndlEnc - sLast) * K) / dtMicros; 
		sLast = spndlEnc;

		
		// EMA velocity diff
		vAvg += eAvg * (vel - vAvg);

		return vAvg;
	}
};
