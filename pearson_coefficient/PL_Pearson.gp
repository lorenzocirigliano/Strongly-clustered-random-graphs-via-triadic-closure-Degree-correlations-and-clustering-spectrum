# =============================================================================
# Gnuplot script to reproduce SF_Pearson figure (4-panel layout)
# Output: epslatex (produces .eps + .tex files for LaTeX integration)
# =============================================================================

# -----------------------------------------------------------------------------
# Terminal and output settings
# -----------------------------------------------------------------------------
set terminal epslatex standalone color colortext size 18cm,12cm font ",14"
set output "PL_Pearson.tex"

# -----------------------------------------------------------------------------
# Multiplot layout: 2x2 grid
# -----------------------------------------------------------------------------
set multiplot layout 2,2

# -----------------------------------------------------------------------------
# Common style settings
# -----------------------------------------------------------------------------
set border linewidth 3
set tics scale 0.5
set key spacing 1.2
set key samplen 2

theta(x) = (x >= 0 ? 1 : 0)

# --- Colors ---
a1 = "#C6E6D8";  a2 = "#9CD3BB"; a3 = "#64B994"
a4 = "#469B76";  a5= "#3F8D69"; a6= "#2C6349"; 

b1 = "#D8BCF0";  b2 = "#C49BE8"; b3 = "#B17AE1"
b4 = "#9D59D9";  b5= "#8A37D2"; b6= "#7b2cbf"; 

c1 = "#FFCDC2";  c2 = "#FFAC99"; c3 = "#FF8A70"
c4 = "#FF6947";  c5= "#FF481F"; c6= "#ff2e00"; 

d1 = "#2C6349";  d2 = "#7b2cbf"; d3 = "#ECA400"
d4 = "#ff2e00";  d5= "#0b0500"; 

# Define line colors (typical scientific plot colors)
set style line 1 linecolor rgb c1 linewidth 2 dashtype 1  # black (single curves)
set style line 2 linecolor rgb c2 linewidth 2 dashtype 1  # red
set style line 3 linecolor rgb c3 linewidth 2 dashtype 1  # blue
set style line 4 linecolor rgb c4 linewidth 2 dashtype 1  # dark green
set style line 5 linecolor rgb c5 linewidth 2 dashtype 1  # magenta

# Common Y-axis settings (same for all panels)
set yrange [0:1]
set ytics 0, 0.2, 1.0 format "%.1f"
set mytics 2
set ylabel '$r_{ \footnotesize \textrm{PL}}$' offset 1.,0

# =============================================================================
# Panel (a) - Top left: f = 0.1, gamma range 2-8
# =============================================================================
set lmargin at screen 0.10
set rmargin at screen 0.46
set bmargin at screen 0.59
set tmargin at screen 0.96

# X-axis configuration
set xrange [2:8]
set xtics 2, 1, 8 format "%g"
set mxtics 2
set xlabel "$\\gamma$" offset 0,0.5

# Legend
set key at graph 0.95, graph 0.95 right top
#set key box opaque

# Panel label
set label 1 '\textbf{(a)}' at graph 0.84, graph 0.9 font ",11"
set label 2 "$f = 0.1$" at graph 0.95, graph 0.75 right font ",11"

# Plot command - replace with your actual data file
plot \
    NaN notitle with lines linestyle 1

set samples 1000 
# Example with data file:
plot 'Scale_Free/f=0.1_gamma=2_to_8/kmax=1e2' using 1:2 notitle with lines dt 2 lc rgb a1 lw 4,\
    'Scale_Free/f=0.1_gamma=2_to_8/kmax=1e3' using 1:2 notitle with lines dt 2 lc rgb a2 lw 4,\
    'Scale_Free/f=0.1_gamma=2_to_8/kmax=1e4' using 1:2 notitle with lines dt 2 lc rgb a3 lw 4,\
    'Scale_Free/f=0.1_gamma=2_to_8/kmax=1e5' using 1:2 notitle with lines dt 2 lc rgb a4 lw 4,\
    'Scale_Free/f=0.1_gamma=2_to_8/kmax=1e6' using 1:2 notitle with lines dt 2 lc rgb a5 lw 4,\
    'Scale_Free/kmax=inf_gamma=2_to_40/f=0.1' using 1:2 notitle with lines dt 1 lc rgb a6 lw 4

# =============================================================================
# Panel (b) - Top right: f = 0.4, gamma range 2-8
# =============================================================================
unset label 1
unset label 2

set lmargin at screen 0.58
set rmargin at screen 0.94
set bmargin at screen 0.59
set tmargin at screen 0.96

# X-axis configuration (same as panel a)
set xrange [2:8]
set xtics 2, 1, 8 format "%g"
set mxtics 2
set xlabel "$\\gamma$" offset 0,0.5

# Panel label
set label 1 '\textbf{(b)}' at graph 0.84, graph 0.9 font ",9"
set label 2 "$f = 0.4$" at graph 0.95, graph 0.75 right font ",9"

# Plot command - replace with your actual data file
plot \
    NaN notitle with lines linestyle 1

# Example with data file:
plot 'Scale_Free/f=0.4_gamma=2_to_8/kmax=1e2' using 1:2 notitle with lines dt 2 lc rgb b1 lw 4,\
    'Scale_Free/f=0.4_gamma=2_to_8/kmax=1e3' using 1:2 notitle with lines dt 2 lc rgb b2 lw 4,\
    'Scale_Free/f=0.4_gamma=2_to_8/kmax=1e4' using 1:2 notitle with lines dt 2 lc rgb b3 lw 4,\
    'Scale_Free/f=0.4_gamma=2_to_8/kmax=1e5' using 1:2 notitle with lines dt 2 lc rgb b4 lw 4,\
    'Scale_Free/f=0.4_gamma=2_to_8/kmax=1e6' using 1:2 notitle with lines dt 2 lc rgb b5 lw 4,\
    'Scale_Free/kmax=inf_gamma=2_to_40/f=0.4' using 1:2 notitle with lines dt 1 lc rgb b6 lw 4

# =============================================================================
# Panel (c) - Bottom left: f = 1.0, gamma range 2-8
# =============================================================================
unset label 1
unset label 2

set lmargin at screen 0.10
set rmargin at screen 0.46
set bmargin at screen 0.10
set tmargin at screen 0.47

# X-axis configuration (same as panels a,b)
set xrange [2:8]
set xtics 2, 1, 8 format "%g"
set mxtics 2
set xlabel "$\\gamma$" offset 0,0.5

# Panel label
set label 1 '\textbf{(c)}' at graph 0.84, graph 0.9 font ",9"
set label 2 "$f = 1$" at graph 0.95, graph 0.75 right font ",9"

# Plot command - replace with your actual data file
plot \
    NaN notitle with lines linestyle 1

# Example with data file:
plot 'Scale_Free/f=1.0_gamma=2_to_8/kmax=1e2' using 1:2 notitle with lines dt 2 lc rgb c1 lw 4,\
    'Scale_Free/f=1.0_gamma=2_to_8/kmax=1e3' using 1:2 notitle with lines dt 2 lc rgb c2 lw 4,\
    'Scale_Free/f=1.0_gamma=2_to_8/kmax=1e4' using 1:2 notitle with lines dt 2 lc rgb c3 lw 4,\
    'Scale_Free/f=1.0_gamma=2_to_8/kmax=1e5' using 1:2 notitle with lines dt 2 lc rgb c4 lw 4,\
    'Scale_Free/f=1.0_gamma=2_to_8/kmax=1e6' using 1:2 notitle with lines dt 2 lc rgb c5 lw 4,\
    'Scale_Free/kmax=inf_gamma=2_to_40/f=1.0' using 1:2 notitle with lines dt 1 lc rgb c6 lw 4

# =============================================================================
# Panel (d) - Bottom right: multiple f values, gamma range 0-40
# =============================================================================
unset label 1
unset label 2

set lmargin at screen 0.58
set rmargin at screen 0.94
set bmargin at screen 0.10
set tmargin at screen 0.47

# X-axis configuration (different range for this panel)
set xrange [2:40]
set xtics ("$2$" 2,  "$10$" 10,  "$20$" 20, "$30$" 30, "$40$" 40)
#set mxtics 2
set xlabel "$\\gamma$" offset 0,0.5

# Legend for multiple curves
set key samplen 2 spacing 1.0 width -2.5 bottom right at graph 0.85, graph 0.95 right top Left
#set key box opaque

# Panel label
set label 1 '\textbf{(d)}' at graph 0.84, graph 0.9 font ",9"

# Plot commands - replace with your actual data files
# Four curves for f = 0.1, 0.4, 0.99, 1.0

# Example with data files:
plot 'Scale_Free/kmax=inf_gamma=2_to_40/f=0.1' using 1:2 title "$f = 0.1$" with lines dt 1 lc rgb d1 lw 4,\
    0.17647 notitle with lines dt 2 lc rgb "#204634" lw 3,\
    'Scale_Free/kmax=inf_gamma=2_to_40/f=0.4' using 1:2 title "$f = 0.4$" with lines dt 1 lc rgb d2 lw 4,\
    0.0625 notitle with lines dt 2 lc rgb "#602296" lw 3,\
    'Scale_Free/kmax=inf_gamma=2_to_40/f=0.99' using 1:2 title "$f = 0.99$" with lines dt 1 lc rgb d3 lw 4,\
    0.000041718 notitle with lines dt 2 lc rgb "#3A5A69" lw 3,\
    'Scale_Free/kmax=inf_gamma=2_to_40/f=1.0' using 1:2 title "$f = 1$" with lines dt 1 lc rgb d4 lw 4,\
    0.24324 notitle with lines dt 3 lc rgb "#B82200" lw 3

# =============================================================================
# End multiplot
# =============================================================================
unset multiplot
set output  # Close output file

# =============================================================================
# Usage notes:
# -----------------------------------------------------------------------------
# 1. Run this script:    gnuplot SF_Pearson.gp
# 2. Compile the output: pdflatex SF_Pearson.tex
#    This produces SF_Pearson.pdf
#
# Alternatively, include the .tex file in your LaTeX document:
#    \input{SF_Pearson}
# (requires the accompanying .eps file in the same directory)
#
# Note: The Greek letter gamma is written as \\gamma (double backslash)
# because Gnuplot requires escaping for LaTeX commands.
# =============================================================================
