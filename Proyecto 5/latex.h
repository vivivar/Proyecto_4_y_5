#ifndef LATEX_H
#define LATEX_H

#include "simplex.h"
#include <glib.h>

// Prototipos de funciones principales
void generar_problema_original_latex(GString *latex, ProblemaInfo *info);
void generar_tabla_latex(GString *latex, TablaSimplex *tabla, const char *titulo, int iteracion, gboolean es_final);
void generar_tabla_inicial_latex(GString *latex, TablaSimplex *tabla, ProblemaInfo *info);
void generar_tablas_intermedias_latex(GString *latex, TablaSimplex **tablas, int num_tablas, ProblemaInfo *info, ResultadoSimplex *resultado);
void generar_tabla_final_latex(GString *latex, ResultadoSimplex *resultado, ProblemaInfo *info);
void generar_solucion_multiple_latex(GString *latex, ResultadoSimplex *resultado, ProblemaInfo *info);
void generar_analisis_sensibilidad_latex(GString *latex, ResultadoSimplex *resultado, ProblemaInfo *info);
void generar_conclusion_latex(GString *latex, ResultadoSimplex *resultado, ProblemaInfo *info);
void generar_documento_latex(ResultadoSimplex *resultado, ProblemaInfo *info, const char *nombre_archivo, gboolean mostrar_tablas);
void compilar_y_mostrar_pdf(const char *nombre_archivo_tex, const char *nombre_archivo_pdf);

#endif