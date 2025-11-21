#ifndef SIMPLEX_H
#define SIMPLEX_H

#include <glib.h>

typedef enum {
    MAXIMIZACION,
    MINIMIZACION
} TipoProblema;

typedef enum {
    RESTRICCION_LE,  // <=
    RESTRICCION_GE,  // >=
    RESTRICCION_EQ   // =
} TipoRestriccion;

typedef enum {
    SOLUCION_OPTIMA,
    SOLUCION_MULTIPLE,
    SOLUCION_NO_ACOTADA,
    SOLUCION_NO_FACTIBLE
} TipoSolucion;

typedef struct {
    int filas;
    int columnas;
    int num_vars_decision;
    int num_restricciones;
    int num_vars_holgura;
    int num_vars_exceso;
    int num_vars_artificiales;
    TipoProblema tipo;
    
    double **tabla;
    double **A;
    double *c;
    double *lados_derechos;
    int *variables_base;
    char **nombres_vars;
    int *es_artificial;
    TipoRestriccion *tipos_restricciones;
} TablaSimplex;

typedef struct {
    TipoSolucion tipo_solucion;
    double valor_z;
    double *solucion;
    char *mensaje;
    TablaSimplex **tablas_intermedias;
    int num_tablas;
    gboolean es_degenerado;
    
    // Para soluciones múltiples
    double **soluciones_adicionales;
    int num_soluciones_adicionales;
    TablaSimplex *segunda_tabla;
} ResultadoSimplex;

// Estructura para información del problema 
typedef struct {
    const char *nombre_problema;
    const char *tipo_problema;
    int num_vars;
    int num_rest;
    const char **nombres_vars;
    double *coef_obj;
    double **coef_rest;
    double *lados_derechos;
    TipoRestriccion *tipos_restricciones;  
} ProblemaInfo;


// Prototipos de funciones
TablaSimplex* crear_tabla_simplex(int num_vars, int num_rest, TipoProblema tipo);
void establecer_funcion_objetivo(TablaSimplex *tabla, double *coeficientes);
void agregar_restriccion(TablaSimplex *tabla, int indice_rest, double *coeficientes, 
                        double lado_derecho, TipoRestriccion tipo);
ResultadoSimplex* resolver_simplex(TablaSimplex *tabla, gboolean mostrar_tablas);
ResultadoSimplex* ejecutar_simplex_completo(TablaSimplex *tabla, gboolean mostrar_tablas);
void liberar_tabla_simplex(TablaSimplex *tabla);
void liberar_resultado(ResultadoSimplex *resultado);
const char* obtener_nombre_variable(TablaSimplex *tabla, int indice);
void extraer_solucion(TablaSimplex *tabla, double *solucion);
void preparar_tabla_simplex(TablaSimplex *tabla);

#endif