#!/usr/bin/gnuplot
# figure_paper.gp — 6-panel P(K) with K_max insets (no do-for loop)
#
# Compile:  gnuplot figure_paper.gp && pdflatex figure_paper.tex

reset

set terminal epslatex standalone colortext 14 size 28cm,14cm
set output "degree_distribution.tex"

# --- Colors ---
cf01 = "#40916c";  cf1 = "#7b2cbf"
cGM1 = "#0b0500";  cGAM = "#0b0500";  cGR = "#888888"
cREF = "#333333"


# --- Grid layout via screen margins ---
L = 0.08;  R = 0.98
B = 0.08;  T = 0.97
hgap = 0.08    # was 0.04 — now fits y-label + tics between columns
vgap = 0.12    # was 0.08 — now fits x-label + tics between rows

pw = (R - L - 2*hgap) / 3.0    # ≈ 0.233  (was 0.273)
ph = (T - B - vgap)   / 2.0    # ≈ 0.385  (was 0.405)

pl(c) = L + c * (pw + hgap)
pr(c) = pl(c) + pw
pt(r) = T - r * (ph + vgap)
pb(r) = pt(r) - ph

# Inset margins (fraction of panel width/height, offset from panel edges)
ins_lm(c) = pr(c) - 0.50 * pw
ins_rm(c) = pr(c) - 0.02
ins_bm(r) = pb(r) + 0.06 * ph
ins_tm(r) = pb(r) + 0.48 * ph
# Bottom row: lift insets to clear x-labels
ins_bm1(r) = pb(r) + 0.16 * ph
ins_tm1(r) = pb(r) + 0.58 * ph

N0 = 3e6

# ==============================================================
#  Common settings macros
# ==============================================================

# Main panel defaults
set_main = "set logscale xy; unset grid; set border 15 lw 0.5; \
            set tics scale 0.6 front; set mxtics 10; set mytics 10; \
            set xtics autofreq; set ytics autofreq; \
            set key off; unset label; unset object"

# Inset defaults
set_ins = "set logscale xy; set border 15 lw 0.3; set key off; \
           unset label; set format x ''; set format y ''; \
           set xlabel ''; set ylabel ''; \
           set xtics ('' 100, '' 1000, '' 10000) scale 0.3; \
           set ytics ('' 100, '' 10000, '' 1000000) scale 0.3; \
           set mxtics 1; set mytics 1; \
           set xrange [7000:5e6]; set yrange [*:*]"

set multiplot

# ==============================================================
#  (a) gamma = 2.25
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
set ylabel '$P(K)$' offset 2,0 font ",14"

set label 1 '\textbf{(a)}' at graph 0.85, graph 0.9 left front

x0 = 2.E4
y0 = 3.E-3
x1 = 1.E3
y1 = 3.E-4
x2 = 1.E4
y2 = 3.E-5
set label 50 '$K^{*}$' at x0, y0 center offset 1, 0.5 front font ",12"
set arrow 1 from x0, y0 to x1, y1 head lw 2 lc rgb "#000000" front
set arrow 2 from x0, y0 to x2, y2 head lw 2 lc rgb "#000000" front
#set label 10 '{\small $\circ$\;$f = 0.1$}'                                at graph 0.56, graph 0.82 left front
#set label 11 '{\small $\bullet$\;$f = 1$}'                                at graph 0.56, graph 0.72 left front
set label 12 '{\small$\sim K^{-(\gamma-1)}$}' at graph 0.83, graph 0.63 left front
set label 13 '{\small$ K^{-\gamma} \sim $}'  at graph 0.73, graph 0.1 left front

set xrange [1:1e7]
set yrange [1e-12:1.e-1]

plot \
    'f_0.1/pk_gamma2.25_f0.10.dat' u 1:2 w p pt 6 ps 1.3 lw 2 lc rgb cf01 notitle, \
    'f_1/pk_gamma2.25_f1.00.dat' u 1:2 w p pt 7 ps 1.3 lw 2 lc rgb cf1  notitle, \
    theta(1.E7-x) * theta(x-5.E1) * 4 * x**(-1.25) w l dt (10,5) lw 5 lc rgb cGM1 notitle, \
    theta(x-1.E3) * 150. * x**(-2.25) w l dt (1,2) lw 8 lc rgb cGAM notitle

unset label 50
unset arrow 1
unset arrow 2

# Inset (a)
@set_ins
set border linewidth 4
set lmargin at screen 0.11
set rmargin at screen 0.195
set tmargin at screen 0.8
set bmargin at screen 0.65

set format x '$10^{%T}$'
set format y '$10^{%T}$'
set xtics ("$10^2$" 100, "$10^7$" 10000000) font ",2"
#set ytics ("$10^2$" 100, "$10^4$" 10000, "$10^6$" 1000000) font ",8"
#set tics scale 0.5

set xlabel '$fk_{\mathrm{max}}^{4-\gamma}$' font ",10" offset 0, 1.7
set ylabel '$K_{\mathrm{max}}$' font ",10" offset 1, 0

set xrange [30:30000000]
set yrange [30:30000000]

plot \
    "< awk '$1==2.25' f_0.1/kmax_scaling_f0.10.dat" u (0.1*$3**(4-2.25)):4:5 w yerrorbars pt 6 ps 1.5 lw 3 lc rgb cf01 notitle, \
    "< awk '$1==2.25' f_1/kmax_scaling_f1.0.dat"    u (1*$3**(4-2.25)):4:5 w yerrorbars pt 7 ps 1.5 lw 3 lc rgb cf1  notitle, \
    x**(1) w l dt (7,3) lw 4 lc rgb cREF notitle


# ==============================================================
#  (b) gamma = 2.50
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
set ylabel '$P(K)$' offset 2,0 font ",14"

set label 1 '\textbf{(b)}' at graph 0.85, graph 0.9 left front

x0 = 2.E4
y0 = 3.E-3
x1 = 1.E3
y1 = 1.E-4
x2 = 1.E4
y2 = 1.E-5
set label 50 '$K^{*}$' at x0, y0 center offset 1, 0.5 front font ",12"
set arrow 1 from x0, y0 to x1, y1 head lw 2 lc rgb "#000000" front
set arrow 2 from x0, y0 to x2, y2 head lw 2 lc rgb "#000000" front

#set label 10 '{\small $\circ$\;$f = 0.1$}'                                at graph 0.56, graph 0.82 left front
#set label 11 '{\small $\bullet$\;$f = 1$}'                                at graph 0.56, graph 0.72 left front
set label 12 '{\small$\sim K^{-(\gamma-1)}$}' at graph 0.85, graph 0.63 left front
set label 13 '{\small$ K^{-\gamma} \sim $}'  at graph 0.71, graph 0.1 left front
set xrange [2:2e6]
set yrange [1e-12:3.e-1]

plot \
    'f_0.1/pk_gamma2.50_f0.10.dat' u 1:2 w p pt 6 ps 1.3 lw 2 lc rgb cf01 notitle, \
    'f_1/pk_gamma2.50_f1.00.dat' u 1:2 w p pt 7 ps 1.3 lw 2 lc rgb cf1  notitle, \
    theta(x-1.E2) * theta(1.E6-x) * 10.05 * x**(-1.50) w l dt (10,5) lw 5 lc rgb cGM1 notitle, \
    theta(x-1.E3) * 171. * x**(-2.50) w l dt (1,2) lw 8 lc rgb cGAM notitle
unset label 50
unset arrow 1
unset arrow 2

# Inset (b)
@set_ins
set border linewidth 4
set lmargin at screen 0.435
set rmargin at screen 0.52
set tmargin at screen 0.8
set bmargin at screen 0.65

set format x '$10^{%T}$'
set format y '$10^{%T}$'
set xtics ("$10^2$" 100, "$10^7$" 10000000) font ",2"
#set xtics ("$10^2$" 100, "$10^4$" 10000) font ",8"
#set ytics ("$10^2$" 100, "$10^4$" 10000, "$10^6$" 1000000) font ",8"
#set tics scale 0.5

set xlabel '$fk_{\mathrm{max}}^{4-\gamma}$' font ",10" offset 0, 1.7
set ylabel '$K_{\mathrm{max}}$' font ",10" offset 1, 0

set xrange [30:30000000]
set yrange [30:30000000]


plot \
    "< awk '$1==2.50' f_0.1/kmax_scaling_f0.10.dat" u (0.1*$3**(4-2.5)):4:5 w yerrorbars pt 6 ps 1.5 lw 3 lc rgb cf01 notitle, \
    "< awk '$1==2.50' f_1/kmax_scaling_f1.0.dat"    u (1*$3**(4-2.5)):4:5 w yerrorbars pt 7 ps 1.5 lw 3 lc rgb cf1  notitle, \
    x**(1) w l dt (7,3) lw 4 lc rgb cREF notitle



# ==============================================================
#  (c) gamma = 2.75
# ==============================================================
@set_main
set border linewidth 4
set lmargin at screen pl(2)
set rmargin at screen pr(2)
set tmargin at screen pt(0)
set bmargin at screen pb(0)

set format x '$10^{%T}$'
set format y '$10^{%T}$'
set xlabel '$K$' offset 0,0.5 font ",14"
set ylabel '$P(K)$' offset 2,0 font ",14"

set label 1 '\textbf{(c)}' at graph 0.85, graph 0.9 left front

x0 = 2.E4
y0 = 3.E-3
x1 = 1.E3
y1 = 1.E-5
x2 = 1.E4
y2 = 1.E-6
set label 50 '$K^{*}$' at x0, y0 center offset 1, 0.5 front font ",12"
set arrow 1 from x0, y0 to x1, y1 head lw 2 lc rgb "#000000" front
set arrow 2 from x0, y0 to x2, y2 head lw 2 lc rgb "#000000" front


#set label 10 '{\small $\circ$\;$f = 0.1$}'                                at graph 0.56, graph 0.82 left front
#set label 11 '{\small $\bullet$\;$f = 1$}'                                at graph 0.56, graph 0.72 left front
set label 12 '{\small$\sim K^{-(\gamma-1)}$}' at graph 0.85, graph 0.52 left front
set label 13 '{\small$ K^{-\gamma} \sim $}'  at graph 0.68, graph 0.1 left front
set xrange [2:1e6]
set yrange [1e-12:5.e-1]

plot \
    'f_0.1/pk_gamma2.75_f0.10.dat' u 1:2 w p pt 6 ps 1.3 lw 2 lc rgb cf01 notitle, \
    'f_1/pk_gamma2.75_f1.00.dat' u 1:2 w p pt 7 ps 1.3 lw 2 lc rgb cf1  notitle, \
    theta(x-1.E2) * theta(1.E6-x) * 20.05 * x**(-1.75) w l dt (10,5) lw 5 lc rgb cGM1 notitle, \
    theta(x-1.E3) * 101. * x**(-2.75) w l dt (1,2) lw 8 lc rgb cGAM notitle

unset label 50
unset arrow 1
unset arrow 2

# Inset (c)
@set_ins
set border linewidth 4
set lmargin at screen 0.762
set rmargin at screen 0.847
set tmargin at screen 0.8
set bmargin at screen 0.65

set format x '$10^{%T}$'
set format y '$10^{%T}$'
set xtics ("$10^1$" 10, "$10^6$" 1000000) font ",2"
#set xtics ("$10^2$" 100, "$10^4$" 10000) font ",8"
#set ytics ("$10^2$" 100, "$10^4$" 10000, "$10^6$" 1000000) font ",8"
#set tics scale 0.5

set xlabel '$fk_{\mathrm{max}}^{4-\gamma}$' font ",10" offset 0, 1.7
set ylabel '$K_{\mathrm{max}}$' font ",10" offset 1, 0

set xrange [8:1000000]
set yrange [8:1000000]


plot \
    "< awk '$1==2.75' f_0.1/kmax_scaling_f0.10.dat" u (0.1*$3**(4-2.75)):4:5 w yerrorbars pt 6 ps 1.5 lw 3 lc rgb cf01 notitle, \
    "< awk '$1==2.75' f_1/kmax_scaling_f1.0.dat"    u (1*$3**(4-2.75)):4:5 w yerrorbars pt 7 ps 1.5 lw 3 lc rgb cf1  notitle, \
    x**(1) w l dt (7,3) lw 4 lc rgb cREF notitle

# ==============================================================
#  (d) gamma = 3.00
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
set ylabel '$P(K)$' offset 2,0 font ",14"

set label 1 '\textbf{(d)}' at graph 0.85, graph 0.9 left front

x0 = 5.E3
y0 = 3.E-3
x1 = 1.E3
y1 = 3.E-6
x2 = 1.E4
y2 = 3.E-7
set label 50 '$K^{*}$' at x0, y0 center offset 0.5, 0.5 front font ",12"
set arrow 1 from x0, y0 to x1, y1 head lw 2 lc rgb "#000000" front
set arrow 2 from x0, y0 to x2, y2 head lw 2 lc rgb "#000000" front

#set label 10 '{\small $\circ$\;$f = 0.1$}'                                at graph 0.56, graph 0.82 left front
#set label 11 '{\small $\bullet$\;$f = 1$}'                                at graph 0.56, graph 0.72 left front
set label 12 '{\small$\sim K^{-(\gamma-1)}$}' at graph 0.86, graph 0.55 left front
set label 13 '{\small$ K^{-\gamma} \sim $}'  at graph 0.68, graph 0.1 left front
set xrange [2:3e5]
set yrange [1e-13:1.e0]

plot \
    'f_0.1/pk_gamma3.00_f0.10.dat' u 1:2 w p pt 6 ps 1.3 lw 2 lc rgb cf01 notitle, \
    'f_1/pk_gamma3.00_f1.00.dat' u 1:2 w p pt 7 ps 1.3 lw 2 lc rgb cf1  notitle, \
    theta(x-1.E2) * theta(5.E5-x) * 40.05 * x**(-2) w l dt (10,5) lw 5 lc rgb cGM1 notitle, \
    theta(x-5.E2) * 40. * x**(-3) w l dt (1,2) lw 8 lc rgb cGAM notitle

unset label 50
unset arrow 1
unset arrow 2

@set_ins
set border linewidth 4
set lmargin at screen 0.11
set rmargin at screen 0.195
set tmargin at screen 0.29
set bmargin at screen 0.14

set format x '$10^{%T}$'
set format y '$10^{%T}$'
set xtics ("$10^1$" 10, "$10^5$" 100000) font ",2"
#set ytics ("$10^2$" 100, "$10^4$" 10000, "$10^6$" 1000000) font ",8"
#set tics scale 0.5

set xlabel '$fk_{\mathrm{max}}^{4-\gamma}$' font ",10" offset 0, 1.7
set ylabel '$K_{\mathrm{max}}$' font ",10" offset 1, 0

set xrange [5:300000]
set yrange [5:300000]

plot \
    "< awk '$1==3.00' f_0.1/kmax_scaling_f0.10.dat" u (0.1*$3**(4-3)):4:5 w yerrorbars pt 6 ps 1.5 lw 3 lc rgb cf01 notitle, \
    "< awk '$1==3.00' f_1/kmax_scaling_f1.0.dat"    u (1*$3**(4-3)):4:5 w yerrorbars pt 7 ps 1.5 lw 3 lc rgb cf1  notitle, \
    x**(1) w l dt (7,3) lw 4 lc rgb cREF notitle



# ==============================================================
#  (e) gamma = 3.50
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
set ylabel '$P(K)$' offset 2,0 font ",14"

set label 1 '\textbf{(e)}' at graph 0.85, graph 0.9 left front

x0 = 3.E2
y0 = 3.E-3
x1 = 1.5E2
y1 = 5.E-6
x2 = 1.5E3
y2 = 5.E-7
set label 50 '$K^{*}$' at x0, y0 center offset 0.5, 0.5 front font ",12"
set arrow 1 from x0, y0 to x1, y1 head lw 2 lc rgb "#000000" front
set arrow 2 from x0, y0 to x2, y2 head lw 2 lc rgb "#000000" front

#set label 10 '{\small $\circ$\;$f = 0.1$}'                                at graph 0.56, graph 0.82 left front
#set label 11 '{\small $\bullet$\;$f = 1$}'                                at graph 0.56, graph 0.72 left front
set label 12 '{\small$\sim K^{-(\gamma-1)}$}' at graph 0.85, graph 0.55 left front
set label 13 '{\small$ K^{-\gamma} \sim $}'  at graph 0.68, graph 0.1 left front
set xrange [2:2e4]
set yrange [1e-12:1.e-0]

plot \
    'f_0.1/pk_gamma3.50_f0.10.dat' u 1:2 w p pt 6 ps 1.3 lw 2 lc rgb cf01 notitle, \
    'f_1/pk_gamma3.50_f1.00.dat' u 1:2 w p pt 7 ps 1.3 lw 2 lc rgb cf1  notitle, \
    theta(x-1.E2) * theta(5.E4-x) * 100.05 * x**(-2.5) w l dt (10,5) lw 5 lc rgb cGM1 notitle, \
    theta(x-1.E2) * 40. * x**(-3.5) w l dt (1,2) lw 8 lc rgb cGAM notitle

unset label 50
unset arrow 1
unset arrow 2

# Inset (e)
@set_ins
set border linewidth 4
set lmargin at screen 0.435
set rmargin at screen 0.52
set tmargin at screen 0.29
set bmargin at screen 0.14

set format x '$10^{%T}$'
set format y '$10^{%T}$'
set xtics ("$10^1$" 10, "$10^4$" 10000) font ",2"
#set ytics ("$10^2$" 100, "$10^4$" 10000, "$10^6$" 1000000) font ",8"
#set tics scale 0.5

set xlabel '$\theta k_{\mathrm{max}}$' font ",10" offset 0, 1.7
set ylabel '$K_{\mathrm{max}}$' font ",10" offset 1, 0

set xrange [9:20000]
set yrange [9:20000]

plot \
    "< awk '$1==3.50' f_0.1/kmax_scaling_f0.10.dat" u ((1+0.1*6.67)*$3):4:5 w yerrorbars pt 6 ps 1.5 lw 3 lc rgb cf01 notitle, \
    "< awk '$1==3.50' f_1/kmax_scaling_f1.0.dat"    u ((1+1*6.66)*$3):4:5 w yerrorbars pt 7 ps 1.5 lw 3 lc rgb cf1  notitle, \
    x**(1) w l dt (7,3) lw 4 lc rgb cREF notitle


# ==============================================================
#  (f) gamma = 4.50
# ==============================================================
@set_main
set border linewidth 4
set lmargin at screen pl(2)
set rmargin at screen pr(2)
set tmargin at screen pt(1)
set bmargin at screen pb(1)

set format x '$10^{%T}$'
set format y '$10^{%T}$'
set xlabel '$K$' offset 0,0.5 font ",14"
set ylabel '$P(K)$' offset 2,0 font ",14"

set label 1 '\textbf{(f)}' at graph 0.85, graph 0.9 left front

x0 = 1.E2
y0 = 1.E-2
x1 = 1.3E1
y1 = 5.E-3
x2 = 1.8E2
y2 = 4.E-6
set label 50 '$K^{*}$' at x0, y0 center offset 0.5, 0.5 front font ",12"
set arrow 1 from x0, y0 to x1, y1 head lw 2 lc rgb "#000000" front
set arrow 2 from x0, y0 to x2, y2 head lw 2 lc rgb "#000000" front

#set label 10 '{\small $\circ$\;$f = 0.1$}'                                at graph 0.56, graph 0.82 left front
#set label 11 '{\small $\bullet$\;$f = 1$}'                                at graph 0.56, graph 0.72 left front
set label 12 '{\small$\sim K^{-(\gamma-1)}$}' at graph 0.85, graph 0.6 left front
set label 13 '{\small$ K^{-\gamma} \sim $}'  at graph 0.74, graph 0.1 left front
set xrange [2:1e3]
set yrange [5e-12:1.e0]

plot \
    'f_0.1/pk_gamma4.50_f0.10.dat' u 1:2 w p pt 6 ps 1.3 lw 2 lc rgb cf01 notitle, \
    'f_1/pk_gamma4.50_f1.00.dat' u 1:2 w p pt 7 ps 1.3 lw 2 lc rgb cf1  notitle, \
    theta(x-3.E1) * theta(1.E3-x) * 1000.05 * x**(-3.5) w l dt (10,5) lw 5 lc rgb cGM1 notitle, \
    theta(x-1.E1) * 40. * x**(-4.5) w l dt (1,2) lw 8 lc rgb cGAM notitle

unset label 50
unset arrow 1
unset arrow 2

# Inset (f)
@set_ins
set border linewidth 4
set lmargin at screen 0.762
set rmargin at screen 0.847
set tmargin at screen 0.29
set bmargin at screen 0.14

set format x '$10^{%T}$'
set format y '$10^{%T}$'
set xtics ("$10^1$" 10, "$10^3$" 1000) font ",2"
#set ytics ("$10^2$" 100, "$10^4$" 10000, "$10^6$" 1000000) font ",8"
#set tics scale 0.5

set xlabel '$\theta k_{\mathrm{max}}$' font ",10" offset 0, 1.7
set ylabel '$K_{\mathrm{max}}$' font ",10" offset 1, 0

set xrange [9:2000]
set yrange [9:2000]

plot \
    "< awk '$1==4.50' f_0.1/kmax_scaling_f0.10.dat" u ((1+0.1*3.29)*$3):4:5 w yerrorbars pt 6 ps 1.5 lw 3 lc rgb cf01 notitle, \
    "< awk '$1==4.50' f_1/kmax_scaling_f1.0.dat"    u ((1+1*3.29)*$3):4:5 w yerrorbars pt 7 ps 1.5 lw 3 lc rgb cf1  notitle, \
    x**(1) w l dt (7,3) lw 4 lc rgb cREF notitle

unset multiplot
set output
