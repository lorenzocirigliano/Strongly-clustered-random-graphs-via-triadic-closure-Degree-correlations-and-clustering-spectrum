# =============================================================================
# Gnuplot script to reproduce RR_ER_Pearson figure
# Output: epslatex (produces .eps + .tex files for LaTeX integration)
# =============================================================================

# -----------------------------------------------------------------------------
# Terminal and output settings
# -----------------------------------------------------------------------------
set terminal epslatex standalone color colortext size 27cm,6cm font ",14"
set output "RR_ER_Pearson.tex"

# --- Colors ---
c1 = "#2C6349";  c2 = "#7b2cbf"; c3 = "#ff2e00"
c4 = "#ECA400";  c5= "#0b0500"; 
# -----------------------------------------------------------------------------
# Multiplot layout: two panels side by side
# -----------------------------------------------------------------------------
set multiplot layout 1,2

# -----------------------------------------------------------------------------
# Common style settings
# -----------------------------------------------------------------------------
set border linewidth 1
set tics scale 0.5
set key spacing 1.2
set key samplen 2

# Define line colors matching the original figure
# Panel (a): c=2 (red), c=3 (blue), c=5 (dark green), c=10 (maroon)
# Panel (b): f=0.2 (red), f=0.4 (dark green), f=1.0 (blue)
set style line 1 linecolor rgb c1 linewidth 4 dashtype 1  # red
set style line 2 linecolor rgb c2 linewidth 4 dashtype 1  # blue
set style line 3 linecolor rgb c3 linewidth 4 dashtype 1  # dark green
set style line 4 linecolor rgb c4 linewidth 4 dashtype 1  # maroon/dark red

# =============================================================================
# Panel (a) - Left panel: RR_Pearson (r vs f)
# =============================================================================
set lmargin at screen 0.07
set rmargin at screen 0.30
set bmargin at screen 0.2
set tmargin at screen 0.95

set border linewidth 3


# Axis configuration
set xrange [0:1]
set yrange [0:0.4]
set xtics 0, 0.2, 1.0 format "%.1f"
set ytics 0, 0.1, 0.4 format "%.1f"
set mxtics 2
set mytics 2

# Axis labels (LaTeX math mode)
set xlabel "$f$" offset 0,0.5
set ylabel '$r_{\footnotesize \textrm{RR}}$' offset 1.1,0

# Legend/Key position (inside, top right area)
set key at graph 0.95, graph 0.95 right top
#set key box opaque

# Panel label
set label 1 '\textbf{(a)}' at graph 0.05, graph 0.9 font ",14"

# Plot commands - replace with your actual data files
# Each line represents a different value of f
plot \
    NaN title '$c \! = \! 2$' with lines linestyle 1, \
    NaN title '$c \! = \! 3$' with lines linestyle 2, \
    NaN title '$c \! = \! 5$'with lines linestyle 3, \
    NaN title '$c \! = \! 10$' with lines linestyle 4
    
# Example with data files (uncomment and modify as needed):
 plot \
     'Random_Regular/theory_c2' using 1:2 notitle with lines linestyle 1, \
     'Random_Regular/simulation_c=2' using 1:2 title '$c \! = \! 2$' with points pt 4 ps 1.5 lc rgb c1 lw 4, \
     'Random_Regular/theory_c3' using 1:2 notitle with lines linestyle 2, \
     'Random_Regular/simulation_c=3' using 1:2 title '$c \! = \! 3$'  with points pt 6 ps 1.5 lc rgb c2 lw 4, \
     'Random_Regular/theory_c5' using 1:2 notitle with lines linestyle 3, \
     'Random_Regular/simulation_c=5' using 1:2 title '$c \! = \! 5$'  with points pt 8 ps 1.5 lc rgb c3 lw 4, \
     'Random_Regular/theory_c10' using 1:2 notitle with lines linestyle 4, \
     'Random_Regular/simulation_c=10' using 1:2 title '$c \! = \! 10$' with points pt 10 ps 1.5 lc rgb c4 lw 4


# =============================================================================
# Panel (b) - Right panel: ER_Pearson (r vs c)
# =============================================================================
set lmargin at screen 0.38
set rmargin at screen 0.61
set bmargin at screen 0.2
set tmargin at screen 0.95


set border linewidth 3


# Axis configuration
set xrange [0:1]
set yrange [0:1]
set xtics 0, 0.2, 1 format "%g"
set ytics 0, 0.2, 1 format "%.1f"
set mxtics 2
set mytics 2

# Axis labels (LaTeX math mode)
set xlabel "$f$" offset 0,0.5
set ylabel '$r_{\footnotesize \textrm{ER}}$' offset 1.1,0

# Legend/Key position (inside, top right area)
set key at graph 0.95, graph 0.95 right top
#set key box opaque

# Panel label
set label 1 '\textbf{(b)}' at graph 0.05, graph 0.9 font ",14"

# Redefine line styles for panel (b) color scheme
# f=0.2 (red), f=0.4 (dark green), f=1.0 (blue)
set style line 11 linecolor rgb c1 linewidth 4 dashtype 1  # red
set style line 12 linecolor rgb c2 linewidth 4 dashtype 1  # dark green
set style line 13 linecolor rgb c3 linewidth 4 dashtype 1  # blue
set style line 14 linecolor rgb c4 linewidth 4 dashtype 1  # blue
set style line 15 linecolor rgb c5 linewidth 4 dashtype 1  # blue

set key samplen 2 spacing 0.9 width -2 bottom right at graph 0.92,0.5 

# --- Colors ---
#c1 = "#2C6349";  c2 = "#7b2cbf"; c3 = "#ff2e00"
#c4 = "#6494aa";  c5= "#0b0500"; 

# Plot commands - replace with your actual data files
# Each line represents a different value of f
plot \
    NaN title '$c \! = \! 1$' with lines linestyle 11, \
    NaN title '$c \! = \! 2$' with lines linestyle 12, \
    NaN title '$c \! = \! 3$' with lines linestyle 13, \
    NaN title '$c \! = \! 4$' with lines linestyle 14, \
    NaN title '$c \! = \! 10$' with lines linestyle 15

# Example with data files (uncomment and modify as needed):
 plot \
     'Erdos_Renyi/ER_Pearson_f_dependence/theory_c=1' using 1:2 notitle with lines linestyle 11, \
     'Erdos_Renyi/ER_Pearson_f_dependence/simulation_c=1' using 1:2 title '$c \! = \! 1$' with points pt 4 ps 1.5 lc rgb c1 lw 4, \
     'Erdos_Renyi/ER_Pearson_f_dependence/theory_c=2' using 1:2 notitle with lines linestyle 12, \
     'Erdos_Renyi/ER_Pearson_f_dependence/simulation_c=2' using 1:2 title '$c \! = \! 2$' with points pt 6 ps 1.5 lc rgb c2 lw 4, \
     'Erdos_Renyi/ER_Pearson_f_dependence/theory_c=3' using 1:2 notitle with lines linestyle 13, \
     'Erdos_Renyi/ER_Pearson_f_dependence/simulation_c=3' using 1:2 title '$c \! = \! 3$' with points pt 8 ps 1.5 lc rgb c3 lw 4, \
     'Erdos_Renyi/ER_Pearson_f_dependence/theory_c=4' using 1:2 notitle with lines linestyle 14, \
     'Erdos_Renyi/ER_Pearson_f_dependence/simulation_c=4' using 1:2 title '$c \! = \! 4$' with points pt 8 ps 1.5 lc rgb c4 lw 4, \
     'Erdos_Renyi/ER_Pearson_f_dependence/theory_c=10' using 1:2 notitle with lines linestyle 15,\
     'Erdos_Renyi/ER_Pearson_f_dependence/simulation_c=10' using 1:2 title '$c \! = \! 10$' with points pt 10 ps 1.5 lc rgb c5 lw 4




# =============================================================================
# Panel (c) - Right panel: ER_Pearson (r vs c)
# =============================================================================
set lmargin at screen 0.673
set rmargin at screen 0.903
set bmargin at screen 0.2
set tmargin at screen 0.95

set border linewidth 3


# Axis configuration
set xrange [0:10]
set yrange [0:1]
set xtics 0, 2, 10 format "%g"
set ytics 0, 0.2, 1 format "%.1f"
set mxtics 2
set mytics 2

# Axis labels (LaTeX math mode)
set xlabel "$c$" offset 0,0.5
set ylabel '$r_{ \footnotesize \textrm{ER}}$' offset 1.1,0

# Legend/Key position (inside, top right area)
set key at graph 0.95, graph 0.95 right top
#set key box opaque

# Panel label
set label 1 '\textbf{(c)}' at graph 0.05, graph 0.9 font ",14"

# Redefine line styles for panel (b) color scheme
# f=0.2 (red), f=0.4 (dark green), f=1.0 (blue)
set style line 11 linecolor rgb c1 linewidth 4 dashtype 1  # red
set style line 12 linecolor rgb c2 linewidth 4 dashtype 1  # dark green
set style line 13 linecolor rgb c3 linewidth 4 dashtype 1  # blue

# Plot commands - replace with your actual data files
# Each line represents a different value of f
plot \
    NaN title '$f \! = \!  0.2$' with lines linestyle 11, \
    NaN title '$f \! = \!  0.4$' with lines linestyle 12, \
    NaN title '$f \! = \! 1.0$'  with lines linestyle 13

# Example with data files (uncomment and modify as needed):
 plot \
     'Erdos_Renyi/theory_f=0.2' using 1:2 notitle with lines linestyle 11, \
     'Erdos_Renyi/simulation_f=0.2' using 1:2 title '$f \! = \!  0.2$' with points pt 4 ps 1.5 lc rgb c1 lw 4, \
     'Erdos_Renyi/theory_f=0.4' using 1:2 notitle with lines linestyle 12, \
     'Erdos_Renyi/simulation_f=0.4' using 1:2 title '$f \! = \!  0.4$' with points pt 6 ps 1.5 lc rgb c2 lw 4, \
     'Erdos_Renyi/theory_f=1.0' using 1:2 notitle with lines linestyle 13, \
     'Erdos_Renyi/simulation_f=1.0' using 1:2 title '$f \! = \!  1.0$' with points pt 8 ps 1.5 lc rgb c3 lw 4

# =============================================================================
# End multiplot
# =============================================================================
unset multiplot
set output  # Close output file

# =============================================================================
# Usage notes:
# -----------------------------------------------------------------------------
# 1. Run this script:    gnuplot RR_ER_Pearson.gp
# 2. Compile the output: pdflatex RR_ER_Pearson.tex
#    This produces RR_ER_Pearson.pdf
#
# Alternatively, include the .tex file in your LaTeX document:
#    \input{RR_ER_Pearson}
# (requires the accompanying .eps file in the same directory)
# =============================================================================
