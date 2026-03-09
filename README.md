# Strongly-clustered-random-graphs-via-triadic-closure-Degree-correlations-and-clustering-spectrum
This repositories contains the C codes used to perform the numerical simulations in the paper "Strongly clustered random graphs via triadic closure: Degree correlations and clustering spectrum". The raw data generated to produce the figures are also provided.

The subfolder /degree_distributions contains the C code used to sample the degree distribution P(K). The raw data generated are given in the subfolders. The figure (Figure 2 in the paper) can be created running ./make_figure.sh in this subfolder. 

The subfolder /clustering contains the C code to create synthetic STC networks and compute various observables analyzed in the paper, such as the clustering spectrum C(K) and the average nearest-neighbor degree Knn(K). It also contains in /clustering/theory a code to evaluate C(K) numerically using the exact expression derived in the paper. In /clustering/results, the raw data created and used to produce the figures in the paper are given (in a compressed format). Figures (Figure 7 and 8 in the paper) can be created running the command ./make_figure.sh in this subfolder. Gnuplot must be installed on your system.

The subfolder "knn" serves to produce the figure (Figure 5 in the paper) on Knn, running ./make_figure.sh in this subfolder. The raw data here are the same as in /clustering/results. 

The subfolder /pearson_coefficient contains the raw data created to produce the figures (Figure 3 and 4 in the paper) on the Pearson coefficient. Figures can be created running ./make_figure.sh in this subfolder.
