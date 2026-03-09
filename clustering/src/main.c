/*
 * main.c - Full STC simulation with explicit graph generation
 *
 * Generates backbone graphs using configuration model, applies triadic
 * closure, and measures properties averaged over multiple realizations.
 *
 * Usage:
 *   ./stc_full -g {ER|RRN|PL} [params] -f <prob> -n <nodes> 
 *              -M_back <backbones> -M_stc <stc_per_backbone> [options]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "rng.h"
#include "graph.h"
#include "config_model.h"
#include "triadic_closure.h"
#include "measurements.h"

/* ============================================================================
 * Configuration
 * ============================================================================ */

typedef enum {
    GRAPH_ER = 0,
    GRAPH_RRN,
    GRAPH_PL
} GraphType;

typedef struct {
    /* Graph type and parameters */
    GraphType graph_type;
    int n;              /* Number of nodes */
    double c;           /* Mean degree (ER) or exact degree (RRN) */
    int kmin;           /* Min degree (PL) */
    double gamma;       /* Exponent (PL) */
    int kmax;           /* Max degree (PL) */
    
    /* STC parameters */
    double f;           /* Triadic closure probability */
    
    /* Simulation parameters */
    int M_back;         /* Number of backbone realizations */
    int M_stc;          /* Number of STC realizations per backbone */
    uint64_t seed;      /* Random seed */
    
    /* Output */
    char prefix[256];   /* Output file prefix */
    int verbose;        /* Verbosity level */
} Config;

static void config_init(Config *cfg) {
    cfg->graph_type = GRAPH_ER;
    cfg->n = 0;
    cfg->c = 0.0;
    cfg->kmin = 0;
    cfg->gamma = 0.0;
    cfg->kmax = 0;
    cfg->f = -1.0;
    cfg->M_back = 1;
    cfg->M_stc = 1;
    cfg->seed = 0;
    strcpy(cfg->prefix, "stc");
    cfg->verbose = 1;
}

/* ============================================================================
 * Command-line parsing
 * ============================================================================ */

static void print_usage(const char *prog) {
    printf("Usage: %s [options]\n\n", prog);
    printf("Required options:\n");
    printf("  -g TYPE          Graph type: ER, RRN, or PL\n");
    printf("  -n VALUE         Number of nodes\n");
    printf("  -f VALUE         Triadic closure probability (0 to 1)\n\n");
    printf("Graph parameters:\n");
    printf("  -c VALUE         Mean degree (for ER) or degree (for RRN)\n");
    printf("  -kmin VALUE      Minimum degree (for PL)\n");
    printf("  -gamma VALUE     Power-law exponent (for PL, must be > 2)\n");
    printf("  -kmax VALUE      Maximum degree (for PL)\n\n");
    printf("Simulation parameters:\n");
    printf("  -M_back VALUE    Number of backbone realizations (default: 1)\n");
    printf("  -M_stc VALUE     Number of STC realizations per backbone (default: 1)\n");
    printf("  -seed VALUE      Random seed (default: from time)\n\n");
    printf("Output:\n");
    printf("  -prefix STRING   Output file prefix (default: 'stc')\n");
    printf("  -q               Quiet mode\n\n");
    printf("Examples:\n");
    printf("  %s -g ER -c 4.0 -n 10000 -f 0.5 -M_back 10 -M_stc 100\n", prog);
    printf("  %s -g RRN -c 3 -n 10000 -f 1.0 -M_back 10 -M_stc 100\n", prog);
    printf("  %s -g PL -kmin 3 -gamma 2.5 -kmax 100 -n 10000 -f 0.5 -M_back 5 -M_stc 50\n", prog);
}

static int parse_args(int argc, char **argv, Config *cfg) {
    int graph_set = 0;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-g") == 0 && i + 1 < argc) {
            i++;
            if (strcmp(argv[i], "ER") == 0) {
                cfg->graph_type = GRAPH_ER;
                graph_set = 1;
            } else if (strcmp(argv[i], "RRN") == 0) {
                cfg->graph_type = GRAPH_RRN;
                graph_set = 1;
            } else if (strcmp(argv[i], "PL") == 0) {
                cfg->graph_type = GRAPH_PL;
                graph_set = 1;
            } else {
                fprintf(stderr, "Error: unknown graph type '%s'\n", argv[i]);
                return -1;
            }
        } else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            cfg->n = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            cfg->c = atof(argv[++i]);
        } else if (strcmp(argv[i], "-kmin") == 0 && i + 1 < argc) {
            cfg->kmin = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-gamma") == 0 && i + 1 < argc) {
            cfg->gamma = atof(argv[++i]);
        } else if (strcmp(argv[i], "-kmax") == 0 && i + 1 < argc) {
            cfg->kmax = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            cfg->f = atof(argv[++i]);
        } else if (strcmp(argv[i], "-M_back") == 0 && i + 1 < argc) {
            cfg->M_back = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-M_stc") == 0 && i + 1 < argc) {
            cfg->M_stc = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-seed") == 0 && i + 1 < argc) {
            cfg->seed = strtoull(argv[++i], NULL, 10);
        } else if (strcmp(argv[i], "-prefix") == 0 && i + 1 < argc) {
            strncpy(cfg->prefix, argv[++i], sizeof(cfg->prefix) - 1);
        } else if (strcmp(argv[i], "-q") == 0) {
            cfg->verbose = 0;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            exit(0);
        } else {
            fprintf(stderr, "Error: unknown option '%s'\n", argv[i]);
            return -1;
        }
    }
    
    /* Validate */
    if (!graph_set) {
        fprintf(stderr, "Error: -g is required\n");
        return -1;
    }
    
    if (cfg->n <= 0) {
        fprintf(stderr, "Error: -n must be positive\n");
        return -1;
    }
    
    if (cfg->f < 0.0 || cfg->f > 1.0) {
        fprintf(stderr, "Error: -f must be between 0 and 1\n");
        return -1;
    }
    
    if ((cfg->graph_type == GRAPH_ER || cfg->graph_type == GRAPH_RRN) && cfg->c <= 0) {
        fprintf(stderr, "Error: -c is required for ER and RRN\n");
        return -1;
    }
    
    if (cfg->graph_type == GRAPH_PL) {
        if (cfg->kmin <= 0) {
            fprintf(stderr, "Error: -kmin must be positive for PL\n");
            return -1;
        }
        if (cfg->gamma <= 2.0) {
            fprintf(stderr, "Error: -gamma must be > 2 for PL\n");
            return -1;
        }
        if (cfg->kmax <= cfg->kmin) {
            fprintf(stderr, "Error: -kmax must be > kmin for PL\n");
            return -1;
        }
    }
    
    return 0;
}

/* ============================================================================
 * Main simulation
 * ============================================================================ */

static int *generate_degree_sequence(const Config *cfg) {
    switch (cfg->graph_type) {
        case GRAPH_ER:
            return degree_seq_poisson(cfg->n, cfg->c);
        case GRAPH_RRN:
            return degree_seq_regular(cfg->n, (int)cfg->c);
        case GRAPH_PL:
            return degree_seq_powerlaw(cfg->n, cfg->kmin, cfg->gamma, cfg->kmax);
        default:
            return NULL;
    }
}

static void run_simulation(const Config *cfg) {
    if (cfg->verbose) {
        printf("=== STC Full Simulation ===\n\n");
        printf("Graph type: %s\n", 
               cfg->graph_type == GRAPH_ER ? "ER" :
               cfg->graph_type == GRAPH_RRN ? "RRN" : "PL");
        printf("Nodes: %d\n", cfg->n);
        if (cfg->graph_type == GRAPH_PL) {
            printf("kmin=%d, gamma=%.2f, kmax=%d\n", cfg->kmin, cfg->gamma, cfg->kmax);
        } else {
            printf("c=%.2f\n", cfg->c);
        }
        printf("f=%.4f\n", cfg->f);
        printf("M_back=%d, M_stc=%d\n", cfg->M_back, cfg->M_stc);
        printf("Total samples: %d\n", cfg->M_back * cfg->M_stc);
        printf("Seed: %lu\n\n", cfg->seed);
    }
    
    /* Estimate max degree for accumulators */
    int est_kmax;
    if (cfg->graph_type == GRAPH_PL) {
        est_kmax = cfg->kmax * cfg->kmax;
    } else {
        double mean_K = cfg->c * (1 + cfg->f * cfg->c);
        est_kmax = (int)(mean_K + 10 * sqrt(mean_K)) + 100;
    }
    
    /* Allocate accumulators */
    DegreeHist *hist_PK = deghist_alloc(est_kmax);
    BinnedAccum *acc_CK = accum_alloc(est_kmax);
    BinnedAccum *acc_knn = accum_alloc(est_kmax);
    
    /* For Pearson coefficient */
    double total_mean_k = 0, total_var_k = 0, total_cov = 0;
    long total_edge_samples = 0;
    
    /* Allocate reusable graph for STC output */
    Graph *gf = graph_alloc(cfg->n, cfg->n * 10);
    
    /* Progress tracking */
    int total_iterations = cfg->M_back * cfg->M_stc;
    int progress_step = total_iterations / 10;
    if (progress_step == 0) progress_step = 1;
    int iteration = 0;
    
    /* Main simulation loop */
    for (int b = 0; b < cfg->M_back; b++) {
        /* Generate degree sequence and backbone */
        int *degrees = generate_degree_sequence(cfg);
        if (!degrees) {
            fprintf(stderr, "Error generating degree sequence\n");
            continue;
        }
        
        Graph *backbone = config_model_generate(cfg->n, degrees, 100);
        free(degrees);
        
        if (!backbone) {
            fprintf(stderr, "Error generating backbone graph\n");
            continue;
        }
        
        if (cfg->verbose && b == 0) {
            printf("Backbone sample:\n");
            graph_print_stats(backbone);
            printf("\n");
        }
        
        /* Multiple STC realizations on this backbone */
        for (int s = 0; s < cfg->M_stc; s++) {
            /* Apply triadic closure */
            if (triadic_closure_inplace(backbone, cfg->f, gf) < 0) {
                fprintf(stderr, "Error in triadic closure\n");
                continue;
            }
            
            /* Measure properties */
            measure_degree_distribution(gf, hist_PK);
            measure_clustering(gf, acc_CK);
            measure_knn(gf, acc_knn);
            
            /* Degree correlations */
            double mean_k, var_k, cov_k1k2;
            measure_degree_correlations(gf, &mean_k, &var_k, &cov_k1k2);
            total_mean_k += mean_k * gf->m;
            total_var_k += var_k * gf->m;
            total_cov += cov_k1k2 * gf->m;
            total_edge_samples += gf->m;
            
            iteration++;
            if (cfg->verbose && iteration % progress_step == 0) {
                printf("Progress: %d%%\n", iteration * 100 / total_iterations);
            }
        }
        
        graph_free(backbone);
    }
    
    /* Compute final Pearson coefficient */
    double final_mean_k = total_mean_k / total_edge_samples;
    double final_var_k = total_var_k / total_edge_samples;
    double final_cov = total_cov / total_edge_samples;
    double pearson_r = (final_var_k > 0) ? final_cov / final_var_k : 0;
    
    /* Print results */
    if (cfg->verbose) {
        printf("\n=== RESULTS ===\n");
        printf("Pearson r = %.6f\n", pearson_r);
        printf("Mean edge endpoint degree = %.4f\n", final_mean_k);
    }
    
    /* Write output files */
    char filename[512];
    char type_str[64];
    
    if (cfg->graph_type == GRAPH_ER) {
        snprintf(type_str, sizeof(type_str), "ER_c%.2f", cfg->c);
    } else if (cfg->graph_type == GRAPH_RRN) {
        snprintf(type_str, sizeof(type_str), "RRN_c%d", (int)cfg->c);
    } else {
        snprintf(type_str, sizeof(type_str), "PL_kmin%d_g%.2f_kmax%d", 
                 cfg->kmin, cfg->gamma, cfg->kmax);
    }
    
    snprintf(filename, sizeof(filename), "%s_%s_n%d_f%.2f_PK.dat",
             cfg->prefix, type_str, cfg->n, cfg->f);
    deghist_write(hist_PK, filename, "# K P(K) CDF(K) count");
    if (cfg->verbose) printf("Wrote %s\n", filename);
    
    snprintf(filename, sizeof(filename), "%s_%s_n%d_f%.2f_CK.dat",
             cfg->prefix, type_str, cfg->n, cfg->f);
    accum_write(acc_CK, filename, "# K C(K) stderr count");
    if (cfg->verbose) printf("Wrote %s\n", filename);
    
    snprintf(filename, sizeof(filename), "%s_%s_n%d_f%.2f_knn.dat",
             cfg->prefix, type_str, cfg->n, cfg->f);
    accum_write(acc_knn, filename, "# K knn(K) stderr count");
    if (cfg->verbose) printf("Wrote %s\n", filename);
    
    /* Write summary */
    snprintf(filename, sizeof(filename), "%s_%s_n%d_f%.2f_summary.dat",
             cfg->prefix, type_str, cfg->n, cfg->f);
    FILE *fp = fopen(filename, "w");
    if (fp) {
        fprintf(fp, "# STC Full Simulation Summary\n");
        fprintf(fp, "graph_type = %s\n", type_str);
        fprintf(fp, "n = %d\n", cfg->n);
        fprintf(fp, "f = %.6f\n", cfg->f);
        fprintf(fp, "M_back = %d\n", cfg->M_back);
        fprintf(fp, "M_stc = %d\n", cfg->M_stc);
        fprintf(fp, "seed = %lu\n", cfg->seed);
        fprintf(fp, "pearson_r = %.10e\n", pearson_r);
        fprintf(fp, "mean_edge_degree = %.10e\n", final_mean_k);
        fclose(fp);
        if (cfg->verbose) printf("Wrote %s\n", filename);
    }
    
    /* Cleanup */
    deghist_free(hist_PK);
    accum_free(acc_CK);
    accum_free(acc_knn);
    graph_free(gf);
}

/* ============================================================================
 * Main
 * ============================================================================ */

int main(int argc, char **argv) {
    Config cfg;
    config_init(&cfg);
    
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    if (parse_args(argc, argv, &cfg) < 0) {
        print_usage(argv[0]);
        return 1;
    }
    
    if (cfg.seed == 0) {
        cfg.seed = (uint64_t)time(NULL);
    }
    rng_seed(cfg.seed);
    
    run_simulation(&cfg);
    
    return 0;
}