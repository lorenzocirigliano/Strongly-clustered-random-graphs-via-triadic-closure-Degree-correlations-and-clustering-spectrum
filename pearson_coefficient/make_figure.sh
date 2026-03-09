#!/bin/bash
# Bash script to generate multiplot figures and clean up temporary files

echo "=== Generating plots with gnuplot ==="
gnuplot RR_ER_Pearson.gp
gnuplot PL_Pearson.gp

echo ""
echo "=== Compiling LaTeX documents to PDF ==="

echo "Compiling clustering multiplot..."
pdflatex RR_ER_Pearson.tex
pdflatex PL_Pearson.tex

pdfcrop RR_ER_Pearson.pdf RR_ER_Pearson.pdf
pdfcrop PL_Pearson.pdf PL_Pearson.pdf

echo ""
echo "=== Cleaning up temporary files ==="
rm -f *.tex *-inc*.pdf *converted*.pdf *.log *.aux *.eps

echo ""
echo "=== Done! ==="
echo "Generated files:"
ls -lh RR_ER_Pearson.pdf PL_Pearson.pdf 2>/dev/null || echo "Check for any errors above."
