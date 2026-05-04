#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>
#include <math.h>

/* updated 3 May 2026
compile with : gcc -shared -fPIC -Wshadow -o GCDM.so GCDM.c

=====================================================================
=                          GCDM parameters                          =
=====================================================================
Dendauw et al. (2024). The gated cascade diffusion model: An integrated theory of decision making,
motor preparation, and motor execution. *Psychological Review*, 131(4), 825–857.
https://doi.org/10.1037/rev0000464

MODEL PARAMETERS ====================================================
x_0    : starting point of the decision variable (in e)
v      : drift rate (in e/s)
g      : gating inhibition (in e)
r      : response bound (in e)
xi     : std dev of the white Gaussian noise at the corruption stage (in e/sqrt(s))
lambda : leakage (in 1/s)
Te     : mean duration of sensory encoding and corticomuscular delay (in s)
Tr     : mean duration of residual motor components related to force production (in s)
sv     : between-trial variability in drift rate (in e/s; sampleV ~ N(v,sv^2))

OUTPUT VECTORS ======================================================
resp        : predicted accuracy for each trial (1 if right response, 2 if left response)
RT          : predicted Response Time for each trial (in s; includes Te and Tr)
PMT         : predicted Pre Motor Time for each trial (in s; includes Te)
MT          : predicted Motor Time for each trial (in s; includes Tr)
firstHit    : predicted time at which the motor preparation variable reached a gating inhibition
              threshold for the first time for each trial (in s; includes Te)
firstHitLoc : predicted location of the firstHit for each trial (1 if right side, 2 if left side)

OTHER COMPUTATIONAL VARIABLES =======================================
s       : diffusion coefficient (in e/sqrt(s))
dt      : step size dt (in s)
n       : number of simulated trials
maxiter : maximum number of iterations for the sample path to hit a response bound

NOTES ===============================================================
- If no response threshold is reached within the allotted time, resp, RT, and MT retain their
  initial value of -1. However, depending on the trial path, firstHit, firstHitLoc, and PMT may
  take non-negative values. Non-response trials should therefore be identified and handled in
  Python using RT == -1.
- The variables rangeLow, rangeHigh, and randomTable are used to simulate a random draw from a
  standard normal distribution (see Evans (2019). *Behavior Research Methods*).
*/


// HELPER ===========================================================
static inline double sample_normal(int rangeLow, int rangeHigh, double *tbl) {
    // returns a random number from the standard normal distribution
    double r = rand() / (1.0 + RAND_MAX);
    int idx = (int)(r * (rangeHigh - rangeLow + 1)) + rangeLow;
    return tbl[idx];
}


// MODEL SIMULATION FUNCTION ========================================
void GCDM(double x_0, double v, double g, double r, double xi, double lambda,
          double Te, double Tr, double sv,
          double *resp, double *RT, double *PMT, double *MT,
          double *firstHit, double *firstHitLoc,
          double s, double dt, int n, int maxiter,
          int rangeLow, int rangeHigh, double *randomTable) {

    // randomize seed of random number generator
    struct timeval t1;
    gettimeofday(&t1, NULL);
    srand(t1.tv_usec * t1.tv_sec);

    // precomputed (to improve performance), dt is constant across trials
    double sqrt_dt = sqrt(dt);

    // simulate n trials
    for (int i=0; i<n; i++) {

        // prepare trial
        resp[i] = -1.0;
        RT[i]   = -1.0;
        PMT[i]  = -1.0;
        MT[i]   = -1.0;
        firstHit[i]    = -1.0;
        firstHitLoc[i] = -1.0;

        // setting the starting point of :
        double x       = x_0; // - the decision variable,
        double tilde_x = x;   // - the corrupted decision variable,
        double y       = x;   // - the motor prep variable,
        double zU      = 0;   // - the neural drive of correct
        double zL      = 0;   // - and incorrect effector.

        // compute the drift rate of the trial (including between-trial variability or not).
        double sampleV;
        if (sv < 0.00001) {  // floating point epsilon guard: treat sv as zero
            sampleV = v;
        } else {
            double randNormNum = sample_normal(rangeLow, rangeHigh, randomTable);
            sampleV = v + (sv*randNormNum);
        }

        // utils variables.
        bool outOfGate = false;

        // time loop - compute sample paths dv, x, and y.
        // simulate trial until max number of dt steps allowed (defined by maxiter).
        // starts at 1 because the index 0 is the starting point.
        for (int iter = 1; iter <= maxiter; iter++) {
            double t = iter * dt;

            // decision variable
            double randNormNum = sample_normal(rangeLow, rangeHigh, randomTable);
            x = x + (sampleV*dt) + (sqrt_dt*s*randNormNum);

            // corrupted decision variable
            randNormNum = sample_normal(rangeLow, rangeHigh, randomTable);
            tilde_x = x + (xi*randNormNum);

            // motor preparation variable
            y = y + lambda*(tilde_x - y)*dt;

            // neural drives for correct (upper bound)
            // and incorrect effectors (lower bound)
            zU =  y - g;
            zL = -y - g;

            if (((zU > 0) || (zL > 0)) && (outOfGate == false)) {
                if (firstHit[i] < 0) {  // this is the first time the gate is overcome
                    firstHit[i] = (t - (dt/2.0)) + Te; // latency of this first gate overcoming
                    if (zU > 0) { firstHitLoc[i] = 1.0; }
                    if (zL > 0) { firstHitLoc[i] = 2.0; } // location
                }
                // the motor preparation variable is now located in the region between
                // an EMG bound and the corresponding response bound : out of gate.
                outOfGate = true;
                PMT[i] = (t - (dt/2.0)) + Te;
            }

            if ((outOfGate) && ((zU <= 0) && (zL <= 0))) { outOfGate = false; }

            // correct response
            if (zU >= r) {
                resp[i] = 1.0;
                RT[i] = t - (dt/2.0) + Te + Tr;
                MT[i] = RT[i] - PMT[i];
                break;
            }

            // incorrect response
            if (zL >= r) {
                resp[i] = 2.0;
                RT[i] = t - (dt/2.0) + Te + Tr;
                MT[i] = RT[i] - PMT[i];
                break;
            }
        }
    }
}

