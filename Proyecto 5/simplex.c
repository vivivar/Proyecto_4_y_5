#include "simplex.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define M_GRANDE 1.0e6
#define EPSILON 1.0e-10
#define MAX_ITERACIONES 1000

// Función auxiliar para copiar una tabla
static TablaSimplex* copiar_tabla(TablaSimplex *original) {
    if (!original) return NULL;
    
    TablaSimplex *copia = g_new0(TablaSimplex, 1);
    
    copia->filas = original->filas;
    copia->columnas = original->columnas;
    copia->num_vars_decision = original->num_vars_decision;
    copia->num_restricciones = original->num_restricciones;
    copia->num_vars_holgura = original->num_vars_holgura;
    copia->num_vars_exceso = original->num_vars_exceso;
    copia->num_vars_artificiales = original->num_vars_artificiales;
    copia->tipo = original->tipo;
    
    if (original->tabla) {
        copia->tabla = g_new0(double*, copia->filas);
        for (int i = 0; i < copia->filas; i++) {
            copia->tabla[i] = g_new0(double, copia->columnas);
            for (int j = 0; j < copia->columnas; j++) {
                copia->tabla[i][j] = original->tabla[i][j];
            }
        }
    }
    
    if (original->variables_base) {
        copia->variables_base = g_new0(int, copia->num_restricciones);
        for (int i = 0; i < copia->num_restricciones; i++) {
            copia->variables_base[i] = original->variables_base[i];
        }
    }
    
    if (original->nombres_vars) {
        int total_vars = copia->num_vars_decision + copia->num_vars_holgura + 
                        copia->num_vars_exceso + copia->num_vars_artificiales;
        copia->nombres_vars = g_new0(char*, total_vars);
        copia->es_artificial = g_new0(int, total_vars);
        
        for (int i = 0; i < total_vars; i++) {
            if (original->nombres_vars[i]) {
                copia->nombres_vars[i] = g_strdup(original->nombres_vars[i]);
            }
            copia->es_artificial[i] = original->es_artificial[i];
        }
    }
    
    return copia;
}

// Función para crear la tabla simplex
TablaSimplex* crear_tabla_simplex(int num_vars, int num_rest, TipoProblema tipo) {
    TablaSimplex *tabla = g_new0(TablaSimplex, 1);
    tabla->num_vars_decision = num_vars;
    tabla->num_restricciones = num_rest;
    tabla->tipo = tipo;
    
    tabla->tipos_restricciones = g_new0(TipoRestriccion, num_rest);
    tabla->lados_derechos = g_new0(double, num_rest);
    
    return tabla;
}

void establecer_funcion_objetivo(TablaSimplex *tabla, double *coeficientes) {
    tabla->c = g_new0(double, tabla->num_vars_decision);
    for (int i = 0; i < tabla->num_vars_decision; i++) {
        tabla->c[i] = coeficientes[i];
    }
}

void agregar_restriccion(TablaSimplex *tabla, int indice_rest, double *coeficientes, 
                        double lado_derecho, TipoRestriccion tipo) {
    if (!tabla->A) {
        tabla->A = g_new0(double*, tabla->num_restricciones);
    }
    
    tabla->A[indice_rest] = g_new0(double, tabla->num_vars_decision);
    for (int i = 0; i < tabla->num_vars_decision; i++) {
        tabla->A[indice_rest][i] = coeficientes[i];
    }
    
    tabla->lados_derechos[indice_rest] = lado_derecho;
    tabla->tipos_restricciones[indice_rest] = tipo;
}

static void preparar_tabla_simplex(TablaSimplex *tabla) {
    tabla->num_vars_holgura = 0;
    tabla->num_vars_exceso = 0;
    tabla->num_vars_artificiales = 0;
    
    for (int i = 0; i < tabla->num_restricciones; i++) {
        switch (tabla->tipos_restricciones[i]) {
            case RESTRICCION_LE:
                tabla->num_vars_holgura++;
                break;
            case RESTRICCION_GE:
                tabla->num_vars_exceso++;
                tabla->num_vars_artificiales++;
                break;
            case RESTRICCION_EQ:
                tabla->num_vars_artificiales++;
                break;
        }
    }
    
    int total_vars = tabla->num_vars_decision + tabla->num_vars_holgura + 
                    tabla->num_vars_exceso + tabla->num_vars_artificiales;
    
    tabla->filas = tabla->num_restricciones + 1;
    tabla->columnas = total_vars + 1; 
    
    tabla->tabla = g_new0(double*, tabla->filas);
    for (int i = 0; i < tabla->filas; i++) {
        tabla->tabla[i] = g_new0(double, tabla->columnas);
    }
    
    tabla->nombres_vars = g_new0(char*, total_vars);
    tabla->es_artificial = g_new0(int, total_vars);
    
    for (int i = 0; i < tabla->num_vars_decision; i++) {
        tabla->nombres_vars[i] = g_strdup_printf("x%d", i + 1);
        tabla->es_artificial[i] = 0;
    }
    
    int idx = tabla->num_vars_decision;
    
    for (int i = 0; i < tabla->num_vars_holgura; i++) {
        tabla->nombres_vars[idx] = g_strdup_printf("s%d", i + 1);
        tabla->es_artificial[idx] = 0;
        idx++;
    }
    
    for (int i = 0; i < tabla->num_vars_exceso; i++) {
        tabla->nombres_vars[idx] = g_strdup_printf("e%d", i + 1);
        tabla->es_artificial[idx] = 0;
        idx++;
    }
    
    for (int i = 0; i < tabla->num_vars_artificiales; i++) {
        tabla->nombres_vars[idx] = g_strdup_printf("a%d", i + 1);
        tabla->es_artificial[idx] = 1;
        idx++;
    }
    
    for (int j = 0; j < tabla->num_vars_decision; j++) {
        tabla->tabla[0][j] = -tabla->c[j];
    }
    
    for (int j = tabla->num_vars_decision + tabla->num_vars_holgura + tabla->num_vars_exceso; 
         j < total_vars; j++) {
        tabla->tabla[0][j] = (tabla->tipo == MAXIMIZACION) ? -M_GRANDE : M_GRANDE;
    }
    
    tabla->variables_base = g_new0(int, tabla->num_restricciones);
    int cont_holgura = 0, cont_exceso = 0, cont_artificial = 0;
    
    for (int i = 0; i < tabla->num_restricciones; i++) {
        int fila = i + 1;
        
        for (int j = 0; j < tabla->num_vars_decision; j++) {
            tabla->tabla[fila][j] = tabla->A[i][j];
        }
        
        TipoRestriccion tipo = tabla->tipos_restricciones[i];
        
        if (tipo == RESTRICCION_LE) {
            int pos_holgura = tabla->num_vars_decision + cont_holgura;
            tabla->tabla[fila][pos_holgura] = 1.0;
            tabla->variables_base[i] = pos_holgura;
            cont_holgura++;
            
        } else if (tipo == RESTRICCION_GE) {
            int pos_exceso = tabla->num_vars_decision + tabla->num_vars_holgura + cont_exceso;
            int pos_artificial = tabla->num_vars_decision + tabla->num_vars_holgura + 
                                tabla->num_vars_exceso + cont_artificial;
            
            tabla->tabla[fila][pos_exceso] = -1.0;
            tabla->tabla[fila][pos_artificial] = 1.0;
            tabla->variables_base[i] = pos_artificial;
            
            cont_exceso++;
            cont_artificial++;
            
        } else { 
            int pos_artificial = tabla->num_vars_decision + tabla->num_vars_holgura + 
                                tabla->num_vars_exceso + cont_artificial;
            
            tabla->tabla[fila][pos_artificial] = 1.0;
            tabla->variables_base[i] = pos_artificial;
            cont_artificial++;
        }
        
        tabla->tabla[fila][total_vars] = tabla->lados_derechos[i];
    }
    
    for (int i = 0; i < tabla->num_restricciones; i++) {
        if (tabla->es_artificial[tabla->variables_base[i]]) {
            int var_base = tabla->variables_base[i];
            double factor = (tabla->tipo == MAXIMIZACION) ? M_GRANDE : -M_GRANDE;
            
            for (int j = 0; j <= total_vars; j++) {
                tabla->tabla[0][j] -= factor * tabla->tabla[i + 1][j];
            }
        }
    }
}

// Encontrar columna pivote según el tipo de problema
static int encontrar_columna_pivote(TablaSimplex *tabla) {
    if (tabla->tipo == MAXIMIZACION) {
        double min_val = 0.0;
        int col_pivote = -1;
        
        for (int j = 0; j < tabla->columnas - 1; j++) {
            if (tabla->es_artificial[j]) {
                continue;
            }
            
            if (tabla->tabla[0][j] < min_val - EPSILON) {
                min_val = tabla->tabla[0][j];
                col_pivote = j;
            }
        }
        
        return col_pivote;
    } else {
        double max_val = 0.0;
        int col_pivote = -1;
        
        for (int j = 0; j < tabla->columnas - 1; j++) {
            if (tabla->es_artificial[j]) {
                continue;
            }
            
            if (tabla->tabla[0][j] > max_val + EPSILON) {
                max_val = tabla->tabla[0][j];
                col_pivote = j;
            }
        }
        
        if (max_val <= EPSILON) {
            return -1;
        }
        
        return col_pivote;
    }
}

// Encontrar fila pivote con detección de degeneración
static int encontrar_fila_pivote(TablaSimplex *tabla, int col_pivote, gboolean *es_degenerado) {
    double min_ratio = 1e15;
    int fila_pivote = -1;
    int cont_ceros = 0;
    
    for (int i = 1; i < tabla->filas; i++) {
        if (tabla->tabla[i][col_pivote] > EPSILON) {
            double ratio = tabla->tabla[i][tabla->columnas - 1] / tabla->tabla[i][col_pivote];
            
            if (fabs(ratio) < EPSILON) {
                cont_ceros++;
            }
            
            if (ratio >= -EPSILON && ratio < min_ratio - EPSILON) {
                min_ratio = ratio;
                fila_pivote = i;
            }
        }
    }
    
    if (es_degenerado) {
        *es_degenerado = (cont_ceros > 0);
        for (int i = 0; i < tabla->num_restricciones; i++) {
            double valor_base = tabla->tabla[i + 1][tabla->columnas - 1];
            if (fabs(valor_base) < EPSILON) {
                *es_degenerado = TRUE;
                break;
            }
        }
    }
    
    return fila_pivote;
}

// Realizar operación de pivote
static void realizar_pivote(TablaSimplex *tabla, int fila_pivote, int col_pivote) {
    double pivote = tabla->tabla[fila_pivote][col_pivote];
    for (int j = 0; j < tabla->columnas; j++) {
        tabla->tabla[fila_pivote][j] /= pivote;
    }
    
    for (int i = 0; i < tabla->filas; i++) {
        if (i != fila_pivote) {
            double factor = tabla->tabla[i][col_pivote];
            for (int j = 0; j < tabla->columnas; j++) {
                tabla->tabla[i][j] -= factor * tabla->tabla[fila_pivote][j];
            }
        }
    }
    tabla->variables_base[fila_pivote - 1] = col_pivote;
}

// Verificar optimalidad según tipo de problema
static int verificar_optimalidad(TablaSimplex *tabla) {
    if (tabla->tipo == MAXIMIZACION) {
        for (int j = 0; j < tabla->columnas - 1; j++) {
            if (!tabla->es_artificial[j] && tabla->tabla[0][j] < -EPSILON) {
                return 0; 
            }
        }
    } else {
        for (int j = 0; j < tabla->columnas - 1; j++) {
            if (!tabla->es_artificial[j] && tabla->tabla[0][j] > EPSILON) {
                return 0; 
            }
        }
    }
    return 1; 
}

// Verificar no acotamiento
static int verificar_no_acotamiento(TablaSimplex *tabla, int col_pivote) {
    if (col_pivote == -1) return 0;
    int tiene_positivo = 0;
    for (int i = 1; i < tabla->filas; i++) {
        if (tabla->tabla[i][col_pivote] > EPSILON) {
            tiene_positivo = 1;
            break;
        }
    }
    
    return !tiene_positivo;
}

// Verificar factibilidad
static int verificar_factibilidad(TablaSimplex *tabla) {
    for (int i = 0; i < tabla->num_restricciones; i++) {
        if (tabla->es_artificial[tabla->variables_base[i]]) {
            double valor = tabla->tabla[i + 1][tabla->columnas - 1];
            if (fabs(valor) > EPSILON) {
                return 0; 
            }
        }
    }
    return 1; 
}

// Verificar si hay solución múltiple
static int verificar_solucion_multiple(TablaSimplex *tabla) {
    int total_vars = tabla->num_vars_decision + tabla->num_vars_holgura + 
                    tabla->num_vars_exceso + tabla->num_vars_artificiales;
    
    for (int j = 0; j < total_vars; j++) {
        int es_variable_base = 0;
        for (int i = 0; i < tabla->num_restricciones; i++) {
            if (tabla->variables_base[i] == j) {
                es_variable_base = 1;
                break;
            }
        }
        
        if (!es_variable_base && !tabla->es_artificial[j]) {
            if (fabs(tabla->tabla[0][j]) < EPSILON) {
                int puede_entrar = 0;
                for (int i = 1; i < tabla->filas; i++) {
                    if (tabla->tabla[i][j] > EPSILON) {
                        puede_entrar = 1;
                        break;
                    }
                }
                
                if (puede_entrar) {
                    return 1; 
                }
            }
        }
    }
    return 0;
}
// Función para encontrar solución alternativa para múltiples soluciones
static TablaSimplex* encontrar_solucion_alternativa(TablaSimplex *tabla_original) {
    TablaSimplex *tabla = copiar_tabla(tabla_original);
    int total_vars = tabla->num_vars_decision + tabla->num_vars_holgura + tabla->num_vars_exceso + tabla->num_vars_artificiales;
    
    for (int j = 0; j < total_vars; j++) {
        if (!tabla->es_artificial[j] && fabs(tabla->tabla[0][j]) < EPSILON) {
            int fila_pivote = -1;
            double min_ratio = 1e15;
            
            for (int i = 1; i < tabla->filas; i++) {
                if (tabla->tabla[i][j] > EPSILON) {
                    double ratio = tabla->tabla[i][total_vars] / tabla->tabla[i][j];
                    if (ratio >= -EPSILON && ratio < min_ratio - EPSILON) {
                        min_ratio = ratio;
                        fila_pivote = i;
                    }
                }
            }
            
            if (fila_pivote != -1) {
                realizar_pivote(tabla, fila_pivote, j);
                return tabla;
            }
        }
    }
    
    liberar_tabla_simplex(tabla);
    return NULL;
}

// Extraer solución de la tabla
void extraer_solucion(TablaSimplex *tabla, double *solucion) {
    for (int i = 0; i < tabla->num_vars_decision; i++) {
        solucion[i] = 0.0;
    }
    
    for (int i = 0; i < tabla->num_restricciones; i++) {
        int var_base = tabla->variables_base[i];
        if (var_base < tabla->num_vars_decision) {
            solucion[var_base] = tabla->tabla[i + 1][tabla->columnas - 1];
        }
    }
}

// Función principal para resolver el simplex con todas las mejoras
ResultadoSimplex* resolver_simplex(TablaSimplex *tabla, gboolean mostrar_tablas) {
    ResultadoSimplex *resultado = g_new0(ResultadoSimplex, 1);
    resultado->tablas_intermedias = g_new0(TablaSimplex*, MAX_ITERACIONES + 2);
    resultado->num_tablas = 0;
    resultado->es_degenerado = FALSE;
    preparar_tabla_simplex(tabla);
    resultado->tablas_intermedias[resultado->num_tablas++] = copiar_tabla(tabla);
    int iteracion = 0;
    gboolean problema_no_acotado = FALSE;
    gboolean problema_degenerado = FALSE;
    
    while (iteracion < MAX_ITERACIONES) {
        if (verificar_optimalidad(tabla)) {
            if (!verificar_factibilidad(tabla)) {
                resultado->tipo_solucion = SOLUCION_NO_FACTIBLE;
                resultado->mensaje = g_strdup("El problema no tiene solución factible (variables artificiales en la base con valor positivo)");
                break;
            }
            
            resultado->tipo_solucion = SOLUCION_OPTIMA;
            resultado->valor_z = tabla->tabla[0][tabla->columnas - 1];
            resultado->solucion = g_new0(double, tabla->num_vars_decision);
            extraer_solucion(tabla, resultado->solucion);
            
            if (verificar_solucion_multiple(tabla)) {
                resultado->tipo_solucion = SOLUCION_MULTIPLE;
                resultado->mensaje = g_strdup("Solución óptima múltiple encontrada");
                
                TablaSimplex *tabla_alternativa = encontrar_solucion_alternativa(tabla);
                if (tabla_alternativa) {
                    resultado->segunda_tabla = tabla_alternativa;
                    
                    resultado->num_soluciones_adicionales = 1;
                    resultado->soluciones_adicionales = g_new0(double*, 1);
                    resultado->soluciones_adicionales[0] = g_new0(double, tabla->num_vars_decision);
                    extraer_solucion(tabla_alternativa, resultado->soluciones_adicionales[0]);
                }
            } else {
                resultado->mensaje = g_strdup("Solución óptima única encontrada");
            }
            
            if (problema_degenerado) {
                resultado->es_degenerado = TRUE;
                resultado->mensaje = g_strdup_printf("%s (problema degenerado)", resultado->mensaje);
            }
            
            break;
        }
        
        int col_pivote = encontrar_columna_pivote(tabla);
        
        if (verificar_no_acotamiento(tabla, col_pivote)) {
            resultado->tipo_solucion = SOLUCION_NO_ACOTADA;
            resultado->mensaje = g_strdup("El problema es no acotado");
            problema_no_acotado = TRUE;
            break;
        }
        
        if (col_pivote == -1) {
            resultado->tipo_solucion = SOLUCION_NO_ACOTADA;
            resultado->mensaje = g_strdup("El problema es no acotado");
            break;
        }
        
        gboolean es_degenerado_iteracion = FALSE;
        int fila_pivote = encontrar_fila_pivote(tabla, col_pivote, &es_degenerado_iteracion);
        
        if (fila_pivote == -1) {
            resultado->tipo_solucion = SOLUCION_NO_ACOTADA;
            resultado->mensaje = g_strdup("El problema es no acotado");
            break;
        }
        
        if (es_degenerado_iteracion) {
            problema_degenerado = TRUE;
        }
        
        if (mostrar_tablas) {
            resultado->tablas_intermedias[resultado->num_tablas++] = copiar_tabla(tabla);
        }
        
        realizar_pivote(tabla, fila_pivote, col_pivote);
        iteracion++;
    }
    
    if (iteracion >= MAX_ITERACIONES && !problema_no_acotado) {
        resultado->tipo_solucion = SOLUCION_NO_FACTIBLE;
        resultado->mensaje = g_strdup("Número máximo de iteraciones alcanzado");
    }
    
    // Guardar tabla final
    resultado->tablas_intermedias[resultado->num_tablas++] = copiar_tabla(tabla);
    
    return resultado;
}

ResultadoSimplex* ejecutar_simplex_completo(TablaSimplex *tabla, gboolean mostrar_tablas) {
    return resolver_simplex(tabla, mostrar_tablas);
}

void liberar_tabla_simplex(TablaSimplex *tabla) {
    if (!tabla) return;
    
    if (tabla->tabla) {
        for (int i = 0; i < tabla->filas; i++) {
            g_free(tabla->tabla[i]);
        }
        g_free(tabla->tabla);
    }
    
    if (tabla->A) {
        for (int i = 0; i < tabla->num_restricciones; i++) {
            g_free(tabla->A[i]);
        }
        g_free(tabla->A);
    }
    
    if (tabla->c) g_free(tabla->c);
    if (tabla->variables_base) g_free(tabla->variables_base);
    if (tabla->es_artificial) g_free(tabla->es_artificial);
    if (tabla->lados_derechos) g_free(tabla->lados_derechos);
    if (tabla->tipos_restricciones) g_free(tabla->tipos_restricciones);
    
    if (tabla->nombres_vars) {
        int total_vars = tabla->num_vars_decision + tabla->num_vars_holgura + 
                        tabla->num_vars_exceso + tabla->num_vars_artificiales;
        for (int i = 0; i < total_vars; i++) {
            g_free(tabla->nombres_vars[i]);
        }
        g_free(tabla->nombres_vars);
    }
    
    g_free(tabla);
}

void liberar_resultado(ResultadoSimplex *resultado) {
    if (!resultado) return;
    
    if (resultado->solucion) g_free(resultado->solucion);
    if (resultado->mensaje) g_free(resultado->mensaje);
    
    if (resultado->tablas_intermedias) {
        for (int i = 0; i < resultado->num_tablas; i++) {
            liberar_tabla_simplex(resultado->tablas_intermedias[i]);
        }
        g_free(resultado->tablas_intermedias);
    }
    
    if (resultado->soluciones_adicionales) {
        for (int i = 0; i < resultado->num_soluciones_adicionales; i++) {
            g_free(resultado->soluciones_adicionales[i]);
        }
        g_free(resultado->soluciones_adicionales);
    }
    
    if (resultado->segunda_tabla) {
        liberar_tabla_simplex(resultado->segunda_tabla);
    }
    
    g_free(resultado);
}

const char* obtener_nombre_variable(TablaSimplex *tabla, int indice) {
    if (indice < 0 || indice >= (tabla->num_vars_decision + tabla->num_vars_holgura + 
                                tabla->num_vars_exceso + tabla->num_vars_artificiales)) {
        return "?";
    }
    return tabla->nombres_vars[indice];
}