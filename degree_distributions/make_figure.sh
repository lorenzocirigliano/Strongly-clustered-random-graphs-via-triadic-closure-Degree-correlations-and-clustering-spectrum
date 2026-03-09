#!/bin/bash
# Bash script to generate multiplot figures and clean up temporary files

echo "=== Generating plots with gnuplot ==="
gnuplot script.gp


echo ""
echo "=== Compiling LaTeX documents to PDF ==="

echo "Compiling Bernoulli multiplot..."
pdflatex degree_distribution.tex
pdfcrop degree_distribution.pdf degree_distribution.pdf

echo ""
echo "=== Cleaning up temporary files ==="
rm -f *.tex *-inc*.pdf *converted*.pdf *.log *.aux *.eps

echo ""
echo "=== Done! ==="
echo "Generated files:"
ls -lh degree_distribution.pdf 1>/dev/null || echo "Check for any errors above."
