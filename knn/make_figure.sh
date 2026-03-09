#!/bin/bash
# Bash script to generate multiplot figures and clean up temporary files

echo "=== Generating plots with gnuplot ==="
gnuplot script2.gp


echo ""
echo "=== Compiling LaTeX documents to PDF ==="

echo "Compiling clustering multiplot..."
pdflatex Knn_PL.tex

pdfcrop Knn_PL.pdf Knn_PL.pdf

echo ""
echo "=== Cleaning up temporary files ==="
rm -f Knn_PL.tex *-inc*.pdf *converted*.pdf *.log *.aux Knn_PL.eps

echo ""
echo "=== Done! ==="
echo "Generated files:"
ls -lh Knn_PL.pdf 1>/dev/null || echo "Check for any errors above."
