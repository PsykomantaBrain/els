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

	void beginRun(int enc, int motorCount, int pN, int pD)
	{
		s0 = enc;
		m0 = motorCount;

		pitchN = pN * motorStepsPerRev;
		pitchD = pD * leadscrewPitchUM * spindlePulsesPerRev;
	}

	int getTargetMotorCount(int enc)
	{
		if (pitchN == 0)
			return m0;
						
		int sd = (enc - s0);
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
	int s0;
	int sLast;
	int m0;
		
	float K; // coupling ratio: motor steps per encoder pulse

	float eAvg = 0.9999f;
	float vAvg = 0.0f;

	float accAvg = 0.0f;

	bool running = false;

	long microsLast = 0;

	bool isRunning()
	{
		return running;
	}

	void beginRun(int enc, int motorCount, float pitch)
	{
		s0 = enc;
		sLast = enc;

		m0 = motorCount;
		K = pitch / (float)leadscrewPitchUM;

		running = true;
		microsLast = micros();

		Serial.println((String)"CoupledRunF32 begin: K=" + K + " s0=" + s0 + " m0=" + m0);
	}

	int getTargetMotorCount(int enc)
	{
		if (K == 0.0f)
			return m0;

		float sd = (enc - s0);
		float sRevs = sd / (float)spindlePulsesPerRev;
		float ldsRevs = sRevs * K;

		int motorSteps = m0 + (int)(ldsRevs * motorStepsPerRev);

		return motorSteps;
	}

	float updStepperSpeed(int enc, ulong micros)
	{		
		float dt = (float)(micros - microsLast) / 1000000.0f;
		if (K == 0.0f || dt == 0.0f)
			return 0.0f;

		// spindle diff
		int dSpndl = enc - sLast;

		// derive vel
		float vel = (float)dSpndl / dt;		

		microsLast = micros;
		sLast = enc;

		// EMA velocity diff
		vAvg += eAvg * (vel - vAvg);

		//Serial.println((String)"CoupledRunF32 velocity update: enc=" + enc+ " dt=" + dt + " vel=" + vel + " vAvg=" + vAvg);

		return vAvg;
	}


	void endRun()
	{
		K = 0.0f;
		vAvg = 0.0f;
		running = false;
	}
	
};
