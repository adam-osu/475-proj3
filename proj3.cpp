#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include <stdio.h>
unsigned int seed = 0;


int NowYear;  // 2022 - 2027
int NowMonth; // 0 - 11

float NowPrecip; // inches of rain per month
float NowTemp;   // temperature this month
float NowHeight; // grain height in inches
int NowNumDeer;  // number of deer in the current population

const float GRAIN_GROWS_PER_MONTH = 9.0;
const float ONE_DEER_EATS_PER_MONTH = 1.0;

const float AVG_PRECIP_PER_MONTH = 7.0; // average
const float AMP_PRECIP_PER_MONTH = 6.0; // plus or minus
const float RANDOM_PRECIP = 2.0;        // plus or minus noise

const float AVG_TEMP = 60.0;    // average
const float AMP_TEMP = 20.0;    // plus or minus
const float RANDOM_TEMP = 10.0; // plus or minus noise

const float MIDTEMP = 40.0;
const float MIDPRECIP = 10.0;

float SQR(float x)
{
  return x * x;
}

float Ranf(unsigned int *seedp, float low, float high)
{
  float r = (float)rand_r(seedp); // 0 - RAND_MAX

  return (low + r * (high - low) / (float)RAND_MAX);
}

int Ranf(unsigned int *seedp, int ilow, int ihigh)
{
  float low = (float)ilow;
  float high = (float)ihigh + 0.9999f;

  return (int)(Ranf(seedp, low, high));
}

void Deer()
{
  int currDeerCount;

  while (NowYear < 2028)
  {
    currDeerCount = NowNumDeer;

    // if fewer dears than height, add a deer
    if (currDeerCount < NowHeight)
    {
      NowNumDeer++;
    }
    // if more dears than height, remove a deer
    else if (currDeerCount > NowHeight)
    {
      NowNumDeer--;
    }


// DoneComputing barrier
#pragma omp barrier
    NowNumDeer = currDeerCount;

// DoneAssigning barrier
#pragma omp barrier

// DonePrinting barrier
#pragma omp barrier
  }
}

void Grain()
{
  float currHeight;
  while (NowYear < 2028)
  {
    currHeight = NowHeight;

    float tempFactor = exp(-SQR((NowTemp - MIDTEMP) / 10.));
    float precipFactor = exp(-SQR((NowPrecip - MIDPRECIP) / 10.));

    currHeight += GRAIN_GROWS_PER_MONTH * tempFactor * precipFactor;
    currHeight -= ONE_DEER_EATS_PER_MONTH * NowNumDeer;

    if (currHeight < 0)
      currHeight = 0;

// DoneComputing barrier:
#pragma omp barrier
    NowHeight = currHeight;

// DoneAssigning barrier:
#pragma omp barrier

// DonePrinting barrier:
#pragma omp barrier
  }
}

void Watcher()
{
  int currMonth;
  int currYear;
  float currPrecip;
  float currTemp;

  while (NowYear < 2028)
  {
// DoneComputing barrier:
#pragma omp barrier

// DoneAssigning barrier:
#pragma omp barrier

    currMonth = NowMonth + 1;
    currYear = NowYear;
    currPrecip = NowPrecip;
    currTemp = NowTemp;

    printf("%d %d %f %f\n", currMonth, currYear, currPrecip, currTemp);

    // increment currYear if month is 12
    if (currMonth == 12)
    {
      currMonth = 0;
      currYear++;
    }
    else
    {
      currMonth++;
    }

    // update global month and year
    NowMonth = currMonth;
    NowYear = currYear;

    // update temp
    float ang = (30. * (float)NowMonth + 15.) * (M_PI / 180.);
    float temp = AVG_TEMP - AMP_TEMP * cos(ang);
    NowTemp = temp + Ranf(&seed, -RANDOM_TEMP, RANDOM_TEMP);

    float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin(ang);
    NowPrecip = precip + Ranf(&seed, -RANDOM_PRECIP, RANDOM_PRECIP);
    if (NowPrecip < 0.)
      NowPrecip = 0.;

// DonePrinting barrier
#pragma omp barrier
  }
}

// float x = Ranf(&seed, -1.f, 1.f);

int main()
{
  // starting date and time:
  NowMonth = 0;
  NowYear = 2022;

  // starting state (feel free to change this if you want):
  NowNumDeer = 1;
  NowHeight = 1.;

  omp_set_num_threads(3); // same as # of sections
#pragma omp parallel sections
  {
#pragma omp section
    {
      Deer();
    }

#pragma omp section
    {
      Grain();
    }

#pragma omp section
    {
      Watcher();
    }
  } // implied barrier -- all functions must return in order
  // to allow any of them to get past here

  return 0;
}