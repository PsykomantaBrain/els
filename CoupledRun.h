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

	void beginRun(int spndlEnc, int motorCount, int pN, int pD)
	{
		s0 = spndlEnc;
		m0 = motorCount;

		pitchN = pN * motorStepsPerRev;
		pitchD = pD * leadscrewPitchUM * spindlePulsesPerRev;
	}

	int getTargetMotorCount(int spndlEnc)
	{
		if (pitchN == 0)
			return m0;
						
		int sd = (spndlEnc - s0);
		int motorSteps = m0 + (sd * pitchN / pitchD);
		
		// accumulate remainder (this is all int maths)
		mRemain += sd * pitchN % pitchD;

		return motorSteps;
	}

	void endRun()
	{
		pitchN = 0;
		pitchD = 1;
		mRemain = 0;
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

	bool running = false;

	bool isRunning()
	{
		return running;
	}

	void beginRun(int spndlEnc, int motorCount, float pitch)
	{
		s0 = spndlEnc;
		sLast = spndlEnc;


		m0 = motorCount;
		K = pitch;

		running = true;
		//Serial.println((String)"CoupledRunF32 begin: K=" + K + " s0=" + s0 + " m0=" + m0);
	}

	int getTargetMotorCount(int spndlEnc)
	{
		if (K == 0.0f)
			return m0;

		float sd = (spndlEnc - s0);
		float sRevs = sd / (float)spindlePulsesPerRev;
		float ldsRevs = sRevs * (K / (float)leadscrewPitchUM);

		int motorSteps = m0 + (int)(ldsRevs * motorStepsPerRev);

		return motorSteps;
	}

	float updStepperSpeed(int spndlEnc, ulong dtMicros)
	{
		if (K == 0.0f || dtMicros == 0)
			return 0.0f;

		// velocity diff
		float vel = (float)((spndlEnc - sLast) * K) / dtMicros;
		sLast = spndlEnc;


		// EMA velocity diff
		vAvg += eAvg * (vel - vAvg);

		return vAvg;
	}

	void endRun()
	{
		K = 0.0f;
		vAvg = 0.0f;
		running = false;
	}
	
};
