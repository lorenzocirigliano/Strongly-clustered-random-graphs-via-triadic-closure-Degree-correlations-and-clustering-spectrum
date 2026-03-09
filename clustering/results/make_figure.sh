#!/bin/bash
# Bash script to generate multiplot figures and clean up temporary files

echo "=== Generating plots with gnuplot ==="
gnuplot script1.gp
gnuplot script2.gp


echo ""
echo "=== Compiling LaTeX documents to PDF ==="

echo "Compiling clustering multiplot..."
pdflatex clustering_spectra_RR_ER.tex
pdflatex clustering_spectra_PL.tex

pdfcrop clustering_spectra_RR_ER.pdf clustering_spectra_RR_ER.pdf
pdfcrop clustering_spectra_PL.pdf clustering_spectra_PL.pdf

echo ""
echo "=== Cleaning up temporary files ==="
rm -f *.tex *-inc*.pdf *converted*.pdf *.log *.aux *.eps

echo ""
echo "=== Done! ==="
echo "Generated files:"
ls -lh clustering_spectra_RR_ER.pdf clustering_spectra_PL.pdf 2>/dev/null || echo "Check for any errors above."
