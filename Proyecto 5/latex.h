#ifndef LATEX_H
#define LATEX_H

#include <glib.h>
#include "simplex.h"

// Prototipos de funciones LaTeX
void formatear_numero(double valor, char *buffer, size_t buffer_size);
void formatear_nombre_variable_latex(const char *nombre_original, char *buffer, size_t buffer_size);
void generar_portada_latex(GString *latex, const char *nombre_problema);
void generar_algoritmo_simplex_latex(GString *latex);
void generar_problema_original_latex(GString *latex, ProblemaInfo *info);
void generar_tabla_inicial_latex(GString *latex, TablaSimplex *tabla, ProblemaInfo *info);
void generar_tablas_intermedias_latex(GString *latex, GList *tablas, ProblemaInfo *info, ResultadoSimplex *resultado);
void generar_tabla_final_latex(GString *latex, ResultadoSimplex *resultado, ProblemaInfo *info);
void generar_explicacion_soluciones_multiples_latex(GString *latex, ResultadoSimplex *resultado, ProblemaInfo *info);
void generar_explicacion_no_acotado_latex(GString *latex);
void generar_explicacion_degenerado_latex(GString *latex, ResultadoSimplex *resultado);
void generar_soluciones_adicionales_latex(GString *latex, ResultadoSimplex *resultado, ProblemaInfo *info);
void generar_solucion_latex(GString *latex, ResultadoSimplex *resultado, ProblemaInfo *info);
void generar_documento_latex(ResultadoSimplex *resultado, ProblemaInfo *info, 
                            const char *nombre_archivo, gboolean mostrar_tablas);
void compilar_y_mostrar_pdf(const char *nombre_archivo_tex, const char *nombre_archivo_pdf);
void encontrar_interseccion(double a1, double b1, double c1, 
                           double a2, double b2, double c2,
                           double *x, double *y, gboolean *existe);
#endif