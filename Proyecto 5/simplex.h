#ifndef SIMPLEX_H
#define SIMPLEX_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <glib.h>

#define MAX_VARS 50
#define MAX_REST 50
#define EPSILON 1e-10

typedef enum {
    MAXIMIZACION,
    MINIMIZACION
} TipoProblema;

typedef enum {
    SOLUCION_UNICA,
    SOLUCION_MULTIPLE,
    NO_ACOTADO,
    DEGENERADO,
    NO_FACTIBLE
} TipoSolucion;

typedef struct {
    double **tabla;
    int num_vars;
    int num_rest;
    int *variables_basicas;
    TipoProblema tipo;
} TablaSimplex;

typedef struct {
    int iteracion;
    int columna_pivote;
    int fila_pivote;
    int variable_entra;
    int variable_sale;
    double elemento_pivote;
    gboolean hubo_empate;  
    int num_empates;       
    int *filas_empatadas;  
} OperacionPivoteo;

typedef struct {
    double *solucion;
    double valor_optimo;
    GString *proceso;
    char *mensaje;
    GList *tablas_intermedias;
    TablaSimplex *tabla_final;      
    TablaSimplex *segunda_tabla;    
    TipoSolucion tipo_solucion;
    int iteraciones;
    double **soluciones_adicionales;
    int num_soluciones_adicionales;
    GList *operaciones_pivoteo; 
    int *variables_entran;      
    int *variables_salen;      
    int *filas_pivote;          
    int *columnas_pivote;       
} ResultadoSimplex;

typedef struct {
    const char **nombres_vars;
    int num_vars;
    int num_rest;
    const char *nombre_problema;
    const char *tipo_problema;
    double *coef_obj;
    double **coef_rest;
    double *lados_derechos;
} ProblemaInfo;

// Prototipos de funciones del algoritmo
TablaSimplex* crear_tabla_simplex(int num_vars, int num_rest, TipoProblema tipo);
void liberar_tabla_simplex(TablaSimplex *tabla);
void establecer_funcion_objetivo_simplex(TablaSimplex *tabla, double coeficientes[]);
void agregar_restriccion_simplex(TablaSimplex *tabla, int indice, double coeficientes[], double lado_derecho);
ResultadoSimplex* ejecutar_simplex_completo(TablaSimplex *tabla, gboolean mostrar_tablas);
void imprimir_tabla_simplex(TablaSimplex *tabla, GString *output);
void liberar_resultado(ResultadoSimplex *resultado);
TablaSimplex* copiar_tabla_simplex(const TablaSimplex *original);

// Agregar estos prototipos que faltan
void extraer_solucion(TablaSimplex *tabla, double solucion[]);
bool es_optima(TablaSimplex *tabla);
int encontrar_columna_pivote(TablaSimplex *tabla);
int encontrar_fila_pivote(TablaSimplex *tabla, int col_pivote, int **filas_empatadas, int *num_empates);
void pivotear(TablaSimplex *tabla, int fila_pivote, int col_pivote);
bool es_no_acotado(TablaSimplex *tabla, int col_pivote);
bool es_solucion_multiple(TablaSimplex *tabla);
bool es_degenerado(TablaSimplex *tabla);
int encontrar_variable_no_basica_con_cero(TablaSimplex *tabla);
bool pivotear_para_segunda_solucion(TablaSimplex *tabla, int variable_cero);

#endif