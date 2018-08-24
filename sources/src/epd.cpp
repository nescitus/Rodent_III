#include <stdio.h>
#include <math.h>
#include "rodent.h"

#ifdef USE_TUNING

#include "epd_white_won.h"
#include "epd_black_won.h"
#include "epd_draw.h"

int startTune = a1Bish;
int endTune = h8Bish+1;

bool cEngine::TuneOne(POS *p, int *pv, int par) {

    bool tuned = false;
	double curr_tune;

	// reinitialize evaluation components that might be changed

    Par.Recalculate();

	if (Par.values[par] < Par.max_val[par]) {
		Par.values[par] += 1;
        Par.Recalculate();
		curr_tune = TexelFit(p, pv);
		if (curr_tune < best_tune) {
			best_tune = curr_tune;
			return true;
		}
	}

	if (Par.values[par] - 1 > Par.min_val[par]) {
		Par.values[par] -= 2;
        Par.Recalculate();
		curr_tune = TexelFit(p, pv);
		if (curr_tune < best_tune) {
			best_tune = curr_tune;
			return true;
		}
	}

	Par.values[par] += 1;
    Par.Recalculate();
	return false;
}

void cEngine::TuneMe(POS *p, int *pv, int iterations) {

	best_tune = TexelFit(p, pv);
	srand(GetMS());
	int test = 0;

	for (int i = 0; i < N_OF_VAL; i++)
		Par.wait[i] = 0;

	for (;;) {
		test++;
		if (test > iterations) break;
		for (int par = startTune; par < endTune; ++par) {
			if (Par.wait[par] == 0 && Par.tunable[par]) {
				printf("Iteration %4d, testing %14s\r", test, paramNames[par]);
				if (TuneOne(p, pv, par)) Par.PrintValues(startTune, endTune);
				else Par.wait[par] = 2;
			}
			else {
				Par.wait[par] -= 1;
				if (Par.wait[par] < 0) Par.wait[par] = 0;
			}
		}
	}
}

double cEngine::TexelFit(POS *p, int *pv) {

  int score = 0;
  double sigmoid = 0.0;
  double sum = 0.0;
  double k_const = 1.600;
  int iter = 0;
  int div = 2000000;
  Trans.Clear();
  ClearAll();

  double result = 1;

  for (int i = 0; epd_10[i]; ++i) {
    iter++;
    if (iter % div == 0) printf("%d\n", iter);
    p->SetPosition(epd_10[i]);
	Par.InitAsymmetric(p);
	score = Quiesce(p, 0, -INF, INF, pv);
    if (p->mSide == BC) score = -score;
    sigmoid = TexelSigmoid(score, k_const);
    sum += ((result - sigmoid)*(result - sigmoid));
  }

  result = 0;

  for (int i = 0; epd_01[i]; ++i) {
    iter++;
    if (iter % div == 0) printf("%d\n", iter);
    p->SetPosition(epd_01[i]);
	Par.InitAsymmetric(p);
	score = Quiesce(p, 0, -INF, INF, pv);
    if (p->mSide == BC) score = -score;
    sigmoid = TexelSigmoid(score, k_const);
    sum += ((result - sigmoid)*(result - sigmoid));
  }

  result = 0.5;

  for (int i = 0; epd_05[i]; ++i) {
    iter++;
    if (iter % div == 0) printf("%d\n", iter);
    p->SetPosition(epd_05[i]);
    Par.InitAsymmetric(p);
	score = Quiesce(p, 0, -INF, INF, pv);
    if (p->mSide == BC) score = -score;
    sigmoid = TexelSigmoid(score, k_const);
    sum += ((result - sigmoid)*(result - sigmoid));
  }

  //printf("%d positions processed\n", iter);
  return ((1.0/iter) * sum);
}

double TexelSigmoid(int score, double k) {
  double exp;

  exp = -(k*((double)score) / 400.0);
  return 1.0 / (1.0 + pow(10.0, exp));
}

#endif