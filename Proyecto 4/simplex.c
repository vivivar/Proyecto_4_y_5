#include "simplex.h"
#include <string.h>

TablaSimplex* crear_tabla_simplex(int num_vars, int num_rest, TipoProblema tipo) {
    TablaSimplex *tabla = g_new0(TablaSimplex, 1);
    tabla->num_vars = num_vars;
    tabla->num_rest = num_rest;
    tabla->tipo = tipo;
    
    tabla->tabla = g_new0(double*, num_rest + 1);
    for (int i = 0; i <= num_rest; i++) {
        tabla->tabla[i] = g_new0(double, num_vars + num_rest + 1);
    }
    
    tabla->variables_basicas = g_new0(int, num_rest);
    for (int i = 0; i < num_rest; i++) {
        tabla->variables_basicas[i] = num_vars + i;
    }
    
    return tabla;
}

void liberar_tabla_simplex(TablaSimplex *tabla) {
    if (!tabla) return;
    
    if (tabla->tabla) {
        for (int i = 0; i <= tabla->num_rest; i++) {
            g_free(tabla->tabla[i]);
        }
        g_free(tabla->tabla);
    }
    
    if (tabla->variables_basicas) {
        g_free(tabla->variables_basicas);
    }
    
    g_free(tabla);
}

void establecer_funcion_objetivo_simplex(TablaSimplex *tabla, double coeficientes[]) {
    for (int j = 0; j < tabla->num_vars; j++) {
        tabla->tabla[0][j] = -coeficientes[j];
    }
    tabla->tabla[0][tabla->num_vars + tabla->num_rest] = 0.0;
}

void agregar_restriccion_simplex(TablaSimplex *tabla, int indice, double coeficientes[], double lado_derecho) {
    if (indice < 0 || indice >= tabla->num_rest) return;
    int fila = indice + 1;
    
    for (int j = 0; j < tabla->num_vars; j++) {
        tabla->tabla[fila][j] = coeficientes[j];
    }
    
    tabla->tabla[fila][tabla->num_vars + indice] = 1.0;
    tabla->tabla[fila][tabla->num_vars + tabla->num_rest] = lado_derecho;
}

bool es_optima(TablaSimplex *tabla) {
    if (tabla->tipo == MAXIMIZACION) {
        for (int j = 0; j < tabla->num_vars; j++) {
            if (tabla->tabla[0][j] < -EPSILON) {
                return false;
            }
        }
    } else {
        for (int j = 0; j < tabla->num_vars; j++) {
            if (tabla->tabla[0][j] > EPSILON) {
                return false;
            }
        }
    }
    return true;
}

int encontrar_columna_pivote(TablaSimplex *tabla) {
    int col_pivote = -1;
    
    if (tabla->tipo == MAXIMIZACION) {
        double min_valor = 0.0;
        for (int j = 0; j < tabla->num_vars; j++) {
            if (tabla->tabla[0][j] < min_valor - EPSILON) {
                min_valor = tabla->tabla[0][j];
                col_pivote = j;
            }
        }
    } else {
        double max_valor = 0.0;
        for (int j = 0; j < tabla->num_vars; j++) {
            if (tabla->tabla[0][j] > max_valor + EPSILON) {
                max_valor = tabla->tabla[0][j];
                col_pivote = j;
            }
        }
    }
    
    return col_pivote;
}

int encontrar_fila_pivote(TablaSimplex *tabla, int col_pivote) {
    if (col_pivote == -1) return -1;
    
    int fila_pivote = -1;
    double min_ratio = 1e9;
    
    for (int i = 1; i <= tabla->num_rest; i++) {
        if (tabla->tabla[i][col_pivote] > EPSILON) {
            double ratio = tabla->tabla[i][tabla->num_vars + tabla->num_rest] / 
                          tabla->tabla[i][col_pivote];
            if (ratio >= 0 && (ratio < min_ratio - EPSILON || fila_pivote == -1)) {
                min_ratio = ratio;
                fila_pivote = i - 1;
            }
        }
    }
    
    g_print("Fila pivote seleccionada: %d\n", fila_pivote);
    return fila_pivote;
}
void pivotear(TablaSimplex *tabla, int fila_pivote, int col_pivote) {
    if (fila_pivote == -1 || col_pivote == -1) return;
    
    int fila_real = fila_pivote + 1;
    double elemento_pivote = tabla->tabla[fila_real][col_pivote];
    
    g_print("Pivoteando en fila %d, columna %d, elemento pivote: %f\n", 
           fila_real, col_pivote, elemento_pivote);
    for (int j = 0; j <= tabla->num_vars + tabla->num_rest; j++) {
        tabla->tabla[fila_real][j] /= elemento_pivote;
    }
    for (int i = 0; i <= tabla->num_rest; i++) {
        if (i == fila_real) continue;
        
        double factor = tabla->tabla[i][col_pivote];
        if (fabs(factor) > EPSILON) {
            for (int j = 0; j <= tabla->num_vars + tabla->num_rest; j++) {
                tabla->tabla[i][j] -= factor * tabla->tabla[fila_real][j];
            }
        }
    }
    tabla->variables_basicas[fila_pivote] = col_pivote;
}

bool es_no_acotado(TablaSimplex *tabla, int col_pivote) {
    if (col_pivote == -1) return false;
    
    for (int i = 1; i <= tabla->num_rest; i++) {
        if (tabla->tabla[i][col_pivote] > EPSILON) {
            return false;
        }
    }
    return true;
}

bool es_solucion_multiple(TablaSimplex *tabla) {
    for (int j = 0; j < tabla->num_vars; j++) {
        bool es_basica = false;
        for (int i = 0; i < tabla->num_rest; i++) {
            if (tabla->variables_basicas[i] == j) {
                es_basica = true;
                break;
            }
        }
        if (!es_basica && fabs(tabla->tabla[0][j]) < EPSILON) {
            return true;
        }
    }
    return false;
}

bool es_degenerado(TablaSimplex *tabla) {
    for (int i = 0; i < tabla->num_rest; i++) {
        double valor = tabla->tabla[i + 1][tabla->num_vars + tabla->num_rest];
        if (fabs(valor) < EPSILON) {
            return true;
        }
    }
    return false;
}

void extraer_solucion(TablaSimplex *tabla, double solucion[]) {
    for (int j = 0; j < tabla->num_vars; j++) {
        solucion[j] = 0.0;
    }
    
    for (int i = 0; i < tabla->num_rest; i++) {
        int var_index = tabla->variables_basicas[i];
        if (var_index < tabla->num_vars) {
            solucion[var_index] = tabla->tabla[i + 1][tabla->num_vars + tabla->num_rest];
        }
    }
}

TablaSimplex* copiar_tabla_simplex(const TablaSimplex *original) {
    if (!original) return NULL;
    TablaSimplex *copia = crear_tabla_simplex(original->num_vars, original->num_rest, original->tipo);
    for (int i = 0; i <= original->num_rest; i++) {
        for (int j = 0; j <= original->num_vars + original->num_rest; j++) {
            copia->tabla[i][j] = original->tabla[i][j];
        }
    }
    for (int i = 0; i < original->num_rest; i++) {
        copia->variables_basicas[i] = original->variables_basicas[i];
    }
    return copia;
}

ResultadoSimplex* ejecutar_simplex_completo(TablaSimplex *tabla, gboolean mostrar_tablas) {
    ResultadoSimplex *resultado = g_new0(ResultadoSimplex, 1);
    resultado->proceso = g_string_new("");
    resultado->solucion = g_new0(double, tabla->num_vars);
    resultado->tipo_solucion = SOLUCION_UNICA;
    resultado->mensaje = NULL;
    resultado->tablas_intermedias = NULL;
    resultado->tabla_final = NULL;  
    resultado->segunda_tabla = NULL; 
    resultado->iteraciones = 0;
    
    int iteracion = 0;
    const int MAX_ITERACIONES = 100;
    
    g_string_append_printf(resultado->proceso, "--- INICIANDO ALGORITMO SIMPLEX ---\n");
    g_string_append_printf(resultado->proceso, "Tipo de problema: %s\n", 
                          tabla->tipo == MAXIMIZACION ? "MAXIMIZACION" : "MINIMIZACION");
    
    TablaSimplex *tabla_trabajo = copiar_tabla_simplex(tabla);
    
    if (mostrar_tablas) {
        resultado->tablas_intermedias = g_list_append(resultado->tablas_intermedias, copiar_tabla_simplex(tabla_trabajo));
    }
    
    while (iteracion < MAX_ITERACIONES) {
        g_string_append_printf(resultado->proceso, "\n--- Iteración %d ---\n", iteracion);
        
        if (mostrar_tablas) {
            imprimir_tabla_simplex(tabla_trabajo, resultado->proceso);
        }
        
        if (es_optima(tabla_trabajo)) {
            g_string_append_printf(resultado->proceso, "Solución óptima alcanzada\n");
            break;
        }
        
        int col_pivote = encontrar_columna_pivote(tabla_trabajo);
        g_string_append_printf(resultado->proceso, "Columna pivote: %d\n", col_pivote);
        
        if (col_pivote == -1) {
            g_string_append_printf(resultado->proceso, "No se encontró columna pivote - solución óptima\n");
            break;
        }
        
        if (es_no_acotado(tabla_trabajo, col_pivote)) {
            g_string_append_printf(resultado->proceso, "PROBLEMA NO ACOTADO\n");
            resultado->tipo_solucion = NO_ACOTADO;
            resultado->mensaje = g_strdup("El problema es no acotado");
            break;
        }
        
        int fila_pivote = encontrar_fila_pivote(tabla_trabajo, col_pivote);
        g_string_append_printf(resultado->proceso, "Fila pivote: %d\n", fila_pivote);
        
        if (fila_pivote == -1) {
            g_string_append_printf(resultado->proceso, "No se encontró fila pivote válida\n");
            resultado->tipo_solucion = NO_ACOTADO;
            resultado->mensaje = g_strdup("No se encontró fila pivote válida");
            break;
        }
        
        g_string_append_printf(resultado->proceso, "Pivoteando en (%d, %d)\n", fila_pivote + 1, col_pivote);
        pivotear(tabla_trabajo, fila_pivote, col_pivote);
        
        if (mostrar_tablas) {
            resultado->tablas_intermedias = g_list_append(resultado->tablas_intermedias, copiar_tabla_simplex(tabla_trabajo));
        }
        
        iteracion++;
    }
    
    resultado->iteraciones = iteracion;
    
    if (iteracion >= MAX_ITERACIONES) {
        g_string_append_printf(resultado->proceso, "ADVERTENCIA: Máximo de iteraciones alcanzado\n");
        if (!resultado->mensaje) {
            resultado->mensaje = g_strdup("Se alcanzó el máximo número de iteraciones");
        }
    }
    
    extraer_solucion(tabla_trabajo, resultado->solucion);
    
    if (tabla_trabajo->tipo == MAXIMIZACION) {
        resultado->valor_optimo = tabla_trabajo->tabla[0][tabla_trabajo->num_vars + tabla_trabajo->num_rest];
    } else {
        resultado->valor_optimo = -tabla_trabajo->tabla[0][tabla_trabajo->num_vars + tabla_trabajo->num_rest];
    }
    
    // Verificar soluciones múltiples
    if (resultado->tipo_solucion == SOLUCION_UNICA && es_solucion_multiple(tabla_trabajo)) {
        g_string_append_printf(resultado->proceso, "SOLUCION MULTIPLE detectada\n");
        resultado->tipo_solucion = SOLUCION_MULTIPLE;
        resultado->mensaje = g_strdup("Se detectaron soluciones múltiples");
        resultado->tabla_final = copiar_tabla_simplex(tabla_trabajo);
        resultado->segunda_tabla = copiar_tabla_simplex(tabla_trabajo);
    } else {
        resultado->tabla_final = copiar_tabla_simplex(tabla_trabajo);
    }
    
    g_string_append_printf(resultado->proceso, "\n--- FINALIZANDO ALGORITMO SIMPLEX ---\n");
    g_string_append_printf(resultado->proceso, "Iteraciones totales: %d\n", iteracion);
    g_string_append_printf(resultado->proceso, "Valor óptimo: %.4f\n", resultado->valor_optimo);
    
    liberar_tabla_simplex(tabla_trabajo);
    return resultado;
}


void imprimir_tabla_simplex(TablaSimplex *tabla, GString *output) {
    if (!output) return;
    g_string_append_printf(output, "\nTabla Simplex:\n");
    g_string_append_printf(output, "Z: ");
    for (int j = 0; j <= tabla->num_vars + tabla->num_rest; j++) {
        g_string_append_printf(output, "%8.3f ", tabla->tabla[0][j]);
    }
    g_string_append_printf(output, "\n");
    for (int i = 1; i <= tabla->num_rest; i++) {
        if (tabla->variables_basicas[i-1] < tabla->num_vars) {
            g_string_append_printf(output, "X%d: ", tabla->variables_basicas[i-1] + 1);
        } else {
            g_string_append_printf(output, "S%d: ", tabla->variables_basicas[i-1] - tabla->num_vars + 1);
        }
        for (int j = 0; j <= tabla->num_vars + tabla->num_rest; j++) {
            g_string_append_printf(output, "%8.3f ", tabla->tabla[i][j]);
        }
        g_string_append_printf(output, "\n");
    }
    g_string_append_printf(output, "\n");
}

void liberar_resultado(ResultadoSimplex *resultado) {
    if (!resultado) return;
    
    if (resultado->solucion) {
        g_free(resultado->solucion);
        resultado->solucion = NULL;
    }
    
    if (resultado->proceso) {
        g_string_free(resultado->proceso, TRUE);
        resultado->proceso = NULL;
    }
    
    if (resultado->mensaje) {
        g_free(resultado->mensaje);
        resultado->mensaje = NULL;
    }
    
    if (resultado->tabla_final) {
        liberar_tabla_simplex(resultado->tabla_final);
        resultado->tabla_final = NULL;
    }
    
    if (resultado->segunda_tabla) {
        liberar_tabla_simplex(resultado->segunda_tabla);
        resultado->segunda_tabla = NULL;
    }
    
    if (resultado->tablas_intermedias) {
        GList *iter = resultado->tablas_intermedias;
        while (iter) {
            if (iter->data) {
                liberar_tabla_simplex((TablaSimplex*)iter->data);
            }
            iter = g_list_next(iter);
        }
        g_list_free(resultado->tablas_intermedias);
        resultado->tablas_intermedias = NULL;
    }
    
    g_free(resultado);
}
