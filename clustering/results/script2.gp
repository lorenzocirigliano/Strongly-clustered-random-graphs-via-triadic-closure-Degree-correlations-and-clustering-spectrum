#!/usr/bin/gnuplot
# figure_paper.gp — 6-panel P(K) with K_max insets (no do-for loop)
#
# Compile:  gnuplot figure_paper.gp && pdflatex figure_paper.tex

reset

set terminal epslatex standalone colortext 14 size 18cm,20cm 
set output "clustering_spectra_PL.tex"

# --- Colors ---
c1 = "#2C6349";  c2 = "#7b2cbf"; c3 = "#ff2e00"
c4 = "#6494aa";  c5= "#0b0500"; 



# --- Grid layout via screen margins ---
L = 0.11;  R = 0.98
B = 0.08;  T = 0.97
hgap = 0.12    # 1 horizontal gap  (between 2 columns)
vgap = 0.08    # 2 vertical gaps   (between 3 rows)

pw = (R - L - 1*hgap) / 2.0    # width  of each panel
ph = (T - B - 2*vgap) / 3.0    # height of each panel

pl(c) = L + c * (pw + hgap)
pr(c) = pl(c) + pw
pt(r) = T - r * (ph + vgap)
pb(r) = pt(r) - ph

# ==============================================================
#  Common settings macros
# ==============================================================
theta(x) = (x >= 0 ? 1 : 0)
# Main panel defaults
set_main = "unset grid; set border 15 lw 0.5; \
            set tics scale 0.6 front; \
            set key off; unset label; unset object"

set_ins = "set logscale xy; set border 15 lw 0.3; set key off; \
           unset label; set format x ''; set format y ''; \
           set xlabel ''; set ylabel ''; \
           set xtics ('' 100, '' 1000, '' 10000) scale 0.3; \
           set ytics ('' 100, '' 10000, '' 1000000) scale 0.3; \
           set mxtics 1; set mytics 1; \
           set xrange [7000:5e6]; set yrange [*:*]"

set multiplot


# ==============================================================
#  (a) PL gamma=2.25
# ==============================================================
@set_main
set border linewidth 4
set lmargin at screen pl(0)
set rmargin at screen pr(0)
set tmargin at screen pt(0)
set bmargin at screen pb(0)



set format x '$10^{%T}$'
set format y '$10^{%T}$'
set xlabel '$K$' offset 0,0.5 font ",14"
set ylabel '$C(K)$' offset 1,0 font ",14"

set label 1 '\textbf{(a)}' at graph 0.05, graph 0.9 left front

set logscale
set xrange [3:3000000]
set yrange[1.E-2:1.1]
set samples 1000
set key samplen 0.1 spacing 1.0 width -2 bottom right 

set samples 1000

plot 'PL/gamma_2.25/stc_PL_kmin3_g2.25_kmax100_n10000_f0.10_CK.dat' u 1:2 every 2 w p pt 4 ps 1.2 lw 4 lc rgb c1 t '$N\!=\!10^4$', \
    'PL/gamma_2.25/stc_PL_kmin3_g2.25_kmax316_n100000_f0.10_CK.dat' u 1:2 every 2 w p pt 6 ps 1.2 lw 4 lc rgb c2 t '$N\!=\!10^5$', \
    'PL/gamma_2.25/stc_PL_kmin3_g2.25_kmax1000_n1000000_f0.10_CK.dat' u 1:2 every 2 w p pt 8 ps 1.2 lw 4 lc rgb c3 t '$N\!=\!10^6$', \
    'PL/gamma_2.25/CK_PL_kmin3_gamma2.25_kmax100000000_f0.10.dat' u 1:6 w l lw 5 dt (2,2) lc rgb c5 notitle, \
    'PL/gamma_2.25/stc_PL_kmin3_g2.25_kmax100_n10000_f1.00_CK.dat' u 1:2 every 2 w p pt 5 ps 1.2 lw 4 lc rgb c1 notitle, \
    'PL/gamma_2.25/stc_PL_kmin3_g2.25_kmax316_n100000_f1.00_CK.dat' u 1:2 every 2 w p pt 7 ps 1.2 lw 4 lc rgb c2 notitle, \
    'PL/gamma_2.25/stc_PL_kmin3_g2.25_kmax1000_n1000000_f1.00_CK.dat' u 1:2 every 2 w p pt 9 ps 1.2 lw 4 lc rgb c3 notitle, \
    'PL/gamma_2.25/CK_PL_kmin3_gamma2.25_kmax100000000_f1.00.dat' u 1:6 w l lw 5 lc rgb c5 notitle


unset label 1
#unset label 2
#unset label 3
#unset label 4

# ==============================================================
#  (b) PL gamma=2.75
# ==============================================================
@set_main
set border linewidth 4
set lmargin at screen pl(1)
set rmargin at screen pr(1)
set tmargin at screen pt(0)
set bmargin at screen pb(0)



set format x '$10^{%T}$'
set format y '$10^{%T}$'
set xlabel '$K$' offset 0,0.5 font ",14"
set ylabel '$C(K)$' offset 1,0 font ",14"

set label 1 '\textbf{(b)}' at graph 0.05, graph 0.9 left front

set logscale
set xrange [3:3000000]
set yrange[1.E-2:1.1]
set samples 1000
set key samplen 0.1 spacing 1.0 width -2 bottom right 

set samples 1000


plot 'PL/gamma_2.75/stc_PL_kmin3_g2.75_kmax100_n10000_f0.10_CK.dat' u 1:2 every 2 w p pt 4 ps 1.2 lw 4 lc rgb c1 t '$N\!=\!10^4$', \
    'PL/gamma_2.75/stc_PL_kmin3_g2.75_kmax316_n100000_f0.10_CK.dat' u 1:2 every 2 w p pt 6 ps 1.2 lw 4 lc rgb c2 t '$N\!=\!10^5$', \
    'PL/gamma_2.75/stc_PL_kmin3_g2.75_kmax1000_n1000000_f0.10_CK.dat' u 1:2 every 2 w p pt 8 ps 1.2 lw 4 lc rgb c3 t '$N\!=\!10^6$', \
    'PL/gamma_2.75/CK_PL_kmin3_gamma2.75_kmax100000000_f0.10.dat' u 1:6 w l lw 5 dt (2,2) lc rgb c5 notitle, \
    'PL/gamma_2.75/stc_PL_kmin3_g2.75_kmax100_n10000_f1.00_CK.dat' u 1:2 every 2 w p pt 5 ps 1.2 lw 4 lc rgb c1 notitle, \
    'PL/gamma_2.75/stc_PL_kmin3_g2.75_kmax316_n100000_f1.00_CK.dat' u 1:2 every 2 w p pt 7 ps 1.2 lw 4 lc rgb c2 notitle, \
    'PL/gamma_2.75/stc_PL_kmin3_g2.75_kmax1000_n1000000_f1.00_CK.dat' u 1:2 every 2 w p pt 9 ps 1.2 lw 4 lc rgb c3 notitle, \
    'PL/gamma_2.75/CK_PL_kmin3_gamma2.75_kmax100000000_f1.00.dat' u 1:6 w l lw 5 lc rgb c5 notitle


unset label 1
#unset label 2
#unset label 3
#unset label 4


# ==============================================================
#  (c) PL gamma=3.5
# ==============================================================
@set_main
set border linewidth 4
set lmargin at screen pl(0)
set rmargin at screen pr(0)
set tmargin at screen pt(1)
set bmargin at screen pb(1)



set format x '$10^{%T}$'
set format y '$10^{%T}$'
set xlabel '$K$' offset 0,0.5 font ",14"
set ylabel '$C(K)$' offset 1,0 font ",14"

set label 1 '\textbf{(c)}' at graph 0.05, graph 0.9 left front

set logscale
set xrange [3:30000]
set yrange[1.E-2:1.1]
set samples 1000
set key samplen 0.1 spacing 1.0 width -2 bottom right 

set samples 1000

plot 'PL/gamma_3.50/stc_PL_kmin3_g3.50_kmax40_n10000_f0.10_CK.dat' u 1:2 every 2 w p pt 4 ps 1.2 lw 4 lc rgb c1 t '$N\!=\!10^4$', \
    'PL/gamma_3.50/stc_PL_kmin3_g3.50_kmax100_n100000_f0.10_CK.dat' u 1:2 every 2 w p pt 6 ps 1.2 lw 4 lc rgb c2 t '$N\!=\!10^5$', \
    'PL/gamma_3.50/stc_PL_kmin3_g3.50_kmax251_n1000000_f0.10_CK.dat' u 1:2 every 2 w p pt 8 ps 1.2 lw 4 lc rgb c3 t '$N\!=\!10^6$', \
    'PL/gamma_3.50/CK_PL_kmin3_gamma3.50_kmax100000000_f0.10.dat' u 1:6 w l lw 5 dt (2,2) lc rgb c5 notitle, \
    'PL/gamma_3.50/stc_PL_kmin3_g3.50_kmax40_n10000_f1.00_CK.dat' u 1:2 every 2 w p pt 5 ps 1.2 lw 4 lc rgb c1 notitle, \
    'PL/gamma_3.50/stc_PL_kmin3_g3.50_kmax100_n100000_f1.00_CK.dat' u 1:2 every 2 w p pt 7 ps 1.2 lw 4 lc rgb c2 notitle, \
    'PL/gamma_3.50/stc_PL_kmin3_g3.50_kmax251_n1000000_f1.00_CK.dat' u 1:2 every 2 w p pt 9 ps 1.2 lw 4 lc rgb c3 notitle, \
    'PL/gamma_3.50/CK_PL_kmin3_gamma3.50_kmax100000000_f1.00.dat' u 1:6 w l lw 5 lc rgb c5 notitle


unset label 1
#unset label 2
#unset label 3
#unset label 4

# ==============================================================
#  (d) PL gamma=4.5
# ==============================================================
@set_main
set border linewidth 4
set lmargin at screen pl(1)
set rmargin at screen pr(1)
set tmargin at screen pt(1)
set bmargin at screen pb(1)



set format x '$10^{%T}$'
set format y '$10^{%T}$'
set xlabel '$K$' offset 0,0.5 font ",14"
set ylabel '$C(K)$' offset 1,0 font ",14"

set label 1 '\textbf{(d)}' at graph 0.05, graph 0.9 left front

set logscale 
set xrange [3:1000]
set yrange[1.E-2:1.1]
set samples 1000
set key samplen 0.1 spacing 1.0 width -2 bottom right 

set samples 1000

plot 'PL/gamma_4.50/stc_PL_kmin3_g4.50_kmax14_n10000_f0.10_CK.dat' u 1:2 every 2 w p pt 4 ps 1.2 lw 4 lc rgb c1 t '$N\!=\!10^4$', \
    'PL/gamma_4.50/stc_PL_kmin3_g4.50_kmax27_n100000_f0.10_CK.dat' u 1:2 every 2 w p pt 6 ps 1.2 lw 4 lc rgb c2 t '$N\!=\!10^5$', \
    'PL/gamma_4.50/stc_PL_kmin3_g4.50_kmax52_n1000000_f0.10_CK.dat' u 1:2 every 2 w p pt 8 ps 1.2 lw 4 lc rgb c3 t '$N\!=\!10^6$', \
    'PL/gamma_4.50/CK_PL_kmin3_gamma4.50_kmax100000000_f0.10.dat' u 1:6 w l lw 5 dt (2,2) lc rgb c5 notitle, \
    'PL/gamma_4.50/stc_PL_kmin3_g4.50_kmax14_n10000_f1.00_CK.dat' u 1:2 every 2 w p pt 5 ps 1.2 lw 4 lc rgb c1 notitle, \
    'PL/gamma_4.50/stc_PL_kmin3_g4.50_kmax27_n100000_f1.00_CK.dat' u 1:2 every 2 w p pt 7 ps 1.2 lw 4 lc rgb c2 notitle, \
    'PL/gamma_4.50/stc_PL_kmin3_g4.50_kmax52_n1000000_f1.00_CK.dat' u 1:2 every 2 w p pt 9 ps 1.2 lw 4 lc rgb c3 notitle, \
    'PL/gamma_4.50/CK_PL_kmin3_gamma4.50_kmax100000000_f1.00.dat' u 1:6 w l lw 5 lc rgb c5 notitle


unset label 1
#unset label 2
#unset label 3
#unset label 4



unset multiplot
set output
