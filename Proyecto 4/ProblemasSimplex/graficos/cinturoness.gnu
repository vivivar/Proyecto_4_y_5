set terminal pngcairo size 800,600 enhanced font 'Arial,10'
set output 'ProblemasSimplex/graficos/cinturoness.png'
set xlabel 'X1'
set ylabel 'X2'
set xrange [0:73,00]
set yrange [0:49,00]
set grid
set key outside right top
set title 'Región Factible - Método Simplex'
plot 'ProblemasSimplex/graficos/cinturoness.dat' index 0 with lines lw 2 lt 1 lc rgb 'red' title "1,0X1 + 1,0X2 ≤ 40,0", 'ProblemasSimplex/graficos/cinturoness.dat' index 1 with lines lw 2 lt 1 lc rgb 'blue' title "1,0X1 + 2,0X2 ≤ 60,0", x with filledcurves above y=0 lc rgb 'green' lt 1 lw 0.5 title "Región Factible", 20,0000, 20,0000 with points pointtype 7 pointsize 2 lc rgb 'black' title "Solución Óptima (20,00, 20,00)"
