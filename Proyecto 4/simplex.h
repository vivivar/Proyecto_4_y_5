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
    double *solucion;
    double valor_optimo;
    TipoSolucion tipo_solucion;
    TablaSimplex tabla_final;
    TablaSimplex segunda_tabla;
    char *mensaje;
    GString *proceso;
    GList *tablas_intermedias; // Lista de tablas intermedias para LaTeX
    int iteraciones;
} ResultadoSimplex;

// Prototipos de funciones del algoritmo
TablaSimplex* crear_tabla_simplex(int num_vars, int num_rest, TipoProblema tipo);
void liberar_tabla_simplex(TablaSimplex *tabla);
void establecer_funcion_objetivo_simplex(TablaSimplex *tabla, double coeficientes[]);
void agregar_restriccion_simplex(TablaSimplex *tabla, int indice, double coeficientes[], double lado_derecho);
ResultadoSimplex* ejecutar_simplex_completo(TablaSimplex *tabla, gboolean mostrar_tablas);
void imprimir_tabla_simplex(TablaSimplex *tabla, GString *output);
void liberar_resultado(ResultadoSimplex *resultado);
TablaSimplex* copiar_tabla_simplex(const TablaSimplex *original);

#endif