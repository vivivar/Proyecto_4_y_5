#ifndef LATEX_H
#define LATEX_H

#include "simplex.h"
#include <glib.h>

// Estructura para información del problema
typedef struct {
    const char **nombres_vars;
    int num_vars;
    int num_rest;
    double *coef_obj;
    double **coef_rest;
    double *lados_derechos;
    const char *nombre_problema;
    const char *tipo_problema;
} ProblemaInfo;

// Prototipos de funciones LaTeX
void generar_documento_latex(ResultadoSimplex *resultado, ProblemaInfo *info, 
                            const char *nombre_archivo, gboolean mostrar_tablas);
void generar_portada_latex(GString *latex, const char *nombre_problema);
void generar_algoritmo_simplex_latex(GString *latex);
void generar_problema_original_latex(GString *latex, ProblemaInfo *info);
void generar_tabla_inicial_latex(GString *latex, TablaSimplex *tabla, ProblemaInfo *info);
void generar_tablas_intermedias_latex(GString *latex, GList *tablas, ProblemaInfo *info);
void generar_tabla_final_latex(GString *latex, ResultadoSimplex *resultado, ProblemaInfo *info);
void generar_solucion_latex(GString *latex, ResultadoSimplex *resultado, ProblemaInfo *info);
void compilar_y_mostrar_pdf(const char *nombre_archivo_tex);

#endif