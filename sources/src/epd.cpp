#include <stdio.h>
#include <math.h>
#include "rodent.h"

void cEngine::LoadEpd() {

    cnt01 = 0;
    cnt10 = 0;
    cnt05 = 0;

    FILE *epdFile = NULL;
    epdFile = fopen("epd.epd", "r");
    char line[256];
    char *pos;
    std::string posString;
    int readCnt = 0;

    if (epdFile == NULL) {
        printf("Epd file not found!");
        return;
    }

    while (fgets(line, sizeof(line), epdFile)) {    // read positions line by line

        while ((pos = strpbrk(line, "\r\n"))) *pos = '\0'; // cleanup
        posString = line;
        readCnt++;
        if (readCnt % 100000 == 0)
            printf("%d positions loaded\n", readCnt);

        if (posString.find("1/2-1/2") != std::string::npos) {
            epd05[cnt05] = posString;
            cnt05++;
        }
        else if (posString.find("1-0") != std::string::npos) {
            epd10[cnt10] = posString;
            cnt10++;
        }
        else if (posString.find("0-1") != std::string::npos) {
            epd01[cnt01] = posString;
            cnt01++;
        }
    }

    fclose(epdFile);
    printf("%d Total positions loaded\n", readCnt);
}

int startTune = NTR_MG;
int endTune = QTR_EG + 1;

bool cEngine::TuneOne(POS *p, int *pv, int par) {

    bool tuned = false;
    double curr_tune;

    // reinitialize evaluation components that might be changed

    Par.Recalculate();

    if (Par.values[par] < Par.max_val[par]) {
        Par.values[par] += step;
        Par.Recalculate();
        curr_tune = TexelFit(p, pv);
        if (curr_tune < best_tune) {
            best_tune = curr_tune;
            return true;
        }
    }

    if (Par.values[par] - 1 > Par.min_val[par]) {
        Par.values[par] -= 2 * step;
        Par.Recalculate();
        curr_tune = TexelFit(p, pv);
        if (curr_tune < best_tune) {
            best_tune = curr_tune;
            return true;
        }
    }

    Par.values[par] += step;
    Par.Recalculate();
    return false;
}

void cEngine::TuneMe(POS *p, int *pv, int iterations) {

    best_tune = TexelFit(p, pv);
    int test = 0;

    for (int i = 0; i < N_OF_VAL; i++)
        Par.wait[i] = 0;

    for (;;) {
        test++;
        if (test > iterations) break;
        for (int par = startTune; par < endTune; ++par) {
            if (Par.wait[par] == 0 && Par.tunable[par]) {
                printf("Iteration %4d, testing %14s\r", test, paramNames[par]);
                if (TuneOne(p, pv, par))
                    Par.PrintValues(startTune, endTune);
                else Par.wait[par] = 2;
            }
            else {
                Par.wait[par] -= 1;
                if (Par.wait[par] < 0) Par.wait[par] = 0;
            }
        }
        if (step > 1) step--;
        printf("Step is %d \n", step);
    }
}

double cEngine::TexelFit(POS *p, int *pv) {

    int score = 0;
    double sigmoid = 0.0;
    double sum = 0.0;
    double k_const = 1.250;
    int iteration = 0;
    Trans.Clear();
    ClearAll();

    double result = 1;

    for (int i = 0; i < cnt10; ++i) {
        iteration++;
        char *cstr = new char[epd10[i].length() + 1];
        strcpy(cstr, epd10[i].c_str());
        p->SetPosition(cstr);
        delete[] cstr;
        Par.InitAsymmetric(p);
        score = Quiesce(p, 0, -INF, INF, pv);
        if (p->mSide == BC) score = -score;
        sigmoid = TexelSigmoid(score, k_const);
        sum += ((result - sigmoid)*(result - sigmoid));
    }

    result = 0;

    for (int i = 0; i < cnt01; ++i) {
        iteration++;
        char *cstr = new char[epd01[i].length() + 1];
        strcpy(cstr, epd01[i].c_str());
        p->SetPosition(cstr);
        delete[] cstr;
        Par.InitAsymmetric(p);
        score = Quiesce(p, 0, -INF, INF, pv);
        if (p->mSide == BC) score = -score;
        sigmoid = TexelSigmoid(score, k_const);
        sum += ((result - sigmoid)*(result - sigmoid));
    }

    result = 0.5;

    for (int i = 0; i < cnt05; ++i) {
        iteration++;
        char *cstr = new char[epd05[i].length() + 1];
        strcpy(cstr, epd05[i].c_str());
        p->SetPosition(cstr);
        delete[] cstr;
        Par.InitAsymmetric(p);
        score = Quiesce(p, 0, -INF, INF, pv);
        if (p->mSide == BC) score = -score;
        sigmoid = TexelSigmoid(score, k_const);
        sum += ((result - sigmoid)*(result - sigmoid));
    }

    return ((1.0 / iteration) * sum);
}

double TexelSigmoid(int score, double k) {

    double exp = -(k*((double)score) / 400.0);
    return 1.0 / (1.0 + pow(10.0, exp));
}