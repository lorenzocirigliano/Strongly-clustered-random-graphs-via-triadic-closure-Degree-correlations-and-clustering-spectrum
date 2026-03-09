#!/usr/bin/gnuplot
# figure_paper.gp — 6-panel P(K) with K_max insets (no do-for loop)
#
# Compile:  gnuplot figure_paper.gp && pdflatex figure_paper.tex

reset

set terminal epslatex standalone colortext 14 size 18cm,20cm 
set output "clustering_spectra_RR_ER.tex"

# --- Colors ---
cf01 = "#40916c";  cf05 = "#7b2cbf"; cf1 = "#bf3d2cff"
cGM1 = "#0b0500";  cGAM = "#0b0500";  cGR = "#888888"
cREF = "#333333"

c1 = "#2C6349";  c2 = "#7b2cbf"; c3 = "#ff2e00"
c4 = "#6494aa";  c5= "#0b0500"; 

# --- Grid layout via screen margins ---
L = 0.1;  R = 0.98
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

theta(x) = (x >= 0) ? 1 : 1/0

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
#  (a) RRN
# ==============================================================
@set_main
set border linewidth 4
set lmargin at screen pl(0)
set rmargin at screen pr(0)
set tmargin at screen pt(0)
set bmargin at screen pb(0)

#set format x '$10^{%T}$'
#set format y '$10^{%T}$'
set xlabel '$K$' offset 0,0.5 font ",14"
set ylabel '$C(K)$' offset 1,0 font ",14"


set label 1 '\textbf{(a)}' at graph 0.85, graph 0.9 left front
set label 2 '{\scriptsize $c=3,\!f = 0.5$}' at 5.2,0.5 left front
set label 3 '{\scriptsize $c=3,\!f = 0.1$}' at 5.2,0.12 left front
set label 4 '{\scriptsize $c=10,\!f = 0.5$}' at 19,0.45 left front
set label 5 '{\scriptsize $c=10,\!f = 0.1$}' at 16.5,0.035 left front



set xtics ("$3$" 3, "$10$" 10, "$30$" 30, "$100$" 100)
set ytics 0.1
set logscale x
set xrange [2.5:100]
set samples 1000
#set key top right 
C(x,f,c) = theta(x-(c-0.1))*theta(c**2+0.2-x)*(f*c*(c-1)+2*(x-c)+f*((x-c)*(x-c-1)*(c-2))/(c*(c-1)-1))/(x*(x-1))

plot 'RRN/stc_RRN_c3_n1000000_f0.10_CK.dat' u (theta($1-3)*$1):2 w p pt 4 ps 1.5 lw 4 lc rgb c1 title '$c=3, f=0.1$', \
    C(x,0.1,3) lc rgb c1 lw 4 notitle, \
    'RRN/stc_RRN_c3_n1000000_f0.50_CK.dat' u (theta($1-3)*$1):2 w p pt 5 ps 1.5 lw 2 lc rgb c1 title '$c=3, f=0.5$', \
    C(x,0.5,3) lc rgb c1 lw 4 notitle, \
    'RRN/stc_RRN_c10_n100000_f0.10_CK.dat' u (theta($1-10)*$1):2 w p pt 6 ps 1.5 lw 4 lc rgb c2 title '$c=10, f=0.1$', \
    C(x,0.1,10) lc rgb c2 lw 4 notitle, \
    'RRN/stc_RRN_c10_n100000_f0.50_CK.dat' u (theta($1-10)*$1):2 w p pt 7 ps 1.5 lw 2 lc rgb c2 title '$c=10, f=0.5$', \
    C(x,0.5,10) lc rgb c2 lw 4 notitle

unset label 1
unset label 2
unset label 3
unset label 4
unset label 5



# ==============================================================
#  (b) ER
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

set label 1 '\textbf{(b)}' at graph 0.85, graph 0.9 left front

set xtics ("$10^0$" 1,  "$10^1$" 10,  "$10^2$" 100, "$10^3$" 1000) font ",2"
set ytics autofreq
set logscale
set xrange [2:500]
set yrange[1.E-3:1.1]
set samples 1000
unset key
set key samplen 0.5 spacing 0.8 width -25 at graph 1.02, graph 0.02 right bottom Left 


plot 'ER/stc_ER_c3.00_n100000_f0.10_CK.dat' u (theta($1-2.9)*$1):2:3 with errorbars pt 4 ps 1.2 lw 4 lc rgb c1 t '\footnotesize$c \! = \! 3, \! f \! = \! 0.1$', \
    'ER/CK_ER_c3_f0.10_Kmax100000000.dat' u 1:3 w l lw 4 lc rgb c1 notitle,\
    'ER/stc_ER_c3.00_n100000_f1.00_CK.dat' u (theta($1-2.9)*$1):2:3 with errorbars pt 5 ps 1.2 lw 2 lc rgb c2 t '\footnotesize$c \!= \! 3, \! f \! = \! 1$', \
    'ER/CK_ER_c3_f1.00_Kmax100000000.dat' u 1:3 w l lw 4 lc rgb c2 notitle,\
    'ER/stc_ER_c10.00_n100000_f0.10_CK.dat' u (theta($1-9.9)*$1):2:3 with errorbars pt 6 ps 1.2 lw 4 lc rgb c3 t '\footnotesize$c \! = \! 10, \! f \! = \! 0.1$', \
    'ER/CK_ER_c10_f0.10_Kmax100000000.dat' u 1:3 w l lw 4 lc rgb c3 notitle,\
    'ER/stc_ER_c10.00_n100000_f1.00_CK.dat' u (theta($1-9.9)*$1):2:3 with errorbars pt 7 ps 1.2 lw 2 lc rgb c4 t '\footnotesize$c \! = \! 10, \! f \! = \! 1$', \
    'ER/CK_ER_c10_f1.00_Kmax100000000.dat' u 1:3 w l lw 4 lc rgb c4 notitle

unset label 1
#unset label 2
#unset label 3
#unset label 4

# Inset (b)
@set_ins
set border linewidth 4
set lmargin at screen 0.67
set rmargin at screen 0.8
set tmargin at screen 0.85
set bmargin at screen 0.77

set format x '$10^{%T}$'
set format y '$10^{%T}$'
set xtics ("$10^2$" 100,  "$10^4$" 10000,  "$10^6$" 1000000, "$10^8$" 100000000) font ",2"
set ytics ("$10^{-3}$" 1.E-3, "$10^{-2}$" 1.E-2) font ",2"
#set ytics ("$10^2$" 100, "$10^4$" 10000, "$10^6$" 1000000) font ",8"
#set tics scale 0.5

#set xlabel '$K$' font ",10" offset 0, 1.7
#set ylabel '$K_{\mathrm{max}}$' font ",10" offset 1, 0

set xrange [9.E1:1.E8]
set yrange [5.E-4:2.E-2]
A(x,f,c) = 1 + f*c + log(f) + log(x)
C(x,f,c) = f/(A(x,f,c)-2*log(A(x,f,c)))**2



plot 'ER/CK_ER_c10_f0.10_Kmax100000000.dat' u 1:3 w l lw 5 lc rgb c3,\
    C(x,0.1,10) w l lw 5 dt 2 lc rgb c3,\
    'ER/CK_ER_c10_f1.00_Kmax100000000.dat' u 1:3 w l lw 5 lc rgb c4, \
    C(x,1,10) w l lw 5 dt 2 lc rgb c4
unset logscale 
unset ytics



unset multiplot
set output
