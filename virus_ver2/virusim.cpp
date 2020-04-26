//
// Simulação de propagação de vírus.
// Adaptada de um código proposto por David Joiner (Kean University).
//
// Uso: virusim <tamanho-da-populacao> <nro. experimentos> <probab. maxima>

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "Population.h"
#include "Random.h"

#define NUM_THREADS 4

void checkCommandLine(int argc, char **argv, int &size, int &trials,
                      int &probs) {
    if (argc > 1) {
        size = atoi(argv[1]);
    }
    if (argc > 2) {
        trials = atoi(argv[2]);
    }
    if (argc > 3) {
        probs = atoi(argv[3]);
    }
}

int main(int argc, char *argv[]) {

    // parâmetros dos experimentos
    int population_size = 30;
    int n_trials = 5000;
    int n_probs = 101;

    double *percent_infected; // percentuais de infectados (saída)
    double *prob_spread;      // probabilidades a serem testadas (entrada)
    double prob_min = 0.0;
    double prob_max = 1.0;
    double prob_step;
    int base_seed = 100;

    omp_set_num_threads(NUM_THREADS);
    omp_set_schedule(omp_sched_auto, -1);

    checkCommandLine(argc, argv, population_size, n_trials, n_probs);
    
    try {
        Random rand;
        Population **population = new Population *[NUM_THREADS];
        
        int i;
        for (i = 0; i < NUM_THREADS; i++){
            population[i] = new Population(population_size);
        }

        prob_spread = new double[n_probs];
        percent_infected = new double[n_probs];

        prob_step = (prob_max - prob_min) / (double)(n_probs - 1);

        printf("Probabilidade, Percentual Infectado\n");

        // para cada probabilidade, calcula o percentual de pessoas infectadas
        #pragma omp parallel for schedule(runtime)
        for (int ip = 0; ip < n_probs; ip++) {

            prob_spread[ip] = prob_min + (double)ip * prob_step;
            percent_infected[ip] = 0.0;
            rand.setSeed(base_seed + ip); // nova seqüência de números aleatórios

                // executa vários experimentos para esta probabilidade
                    for (int it = 0; it < n_trials; it++) {
                        int thread_num = omp_get_thread_num();
                        // queima floresta até o fogo apagar
                        population[thread_num]->propagateUntilOut(population[thread_num]->centralPerson(), prob_spread[ip], rand);
                        percent_infected[ip] += population[thread_num]->getPercentInfected();
                    }

            // calcula média dos percentuais de árvores queimadas
            percent_infected[ip] /= n_trials;

            // mostra resultado para esta probabilidade
            printf("%lf, %lf\n", prob_spread[ip], percent_infected[ip]);
        }

        delete[] prob_spread;
        delete[] percent_infected;
    } 
    catch (std::bad_alloc) {
        std::cerr << "Erro: alocacao de memoria" << std::endl;
        return 1;
    }

    return 0;
}