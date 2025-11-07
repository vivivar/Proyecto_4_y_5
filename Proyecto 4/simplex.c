#include "simplex.h"
#include <string.h>
#include <stdlib.h> 
#include <time.h>   

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
    int empates[tabla->num_rest]; 
    int num_empates = 0;
    bool hubo_empate = false;
    
    for (int i = 1; i <= tabla->num_rest; i++) {
        if (tabla->tabla[i][col_pivote] > EPSILON) {
            double ratio = tabla->tabla[i][tabla->num_vars + tabla->num_rest] / 
                          tabla->tabla[i][col_pivote];
            
            if (ratio >= 0) {
                if (fabs(ratio - min_ratio) < EPSILON) {
                    empates[num_empates++] = i - 1;
                    hubo_empate = true;
                } else if (ratio < min_ratio - EPSILON || fila_pivote == -1) {
                    min_ratio = ratio;
                    fila_pivote = i - 1;
                    num_empates = 0; 
                    empates[num_empates++] = i - 1;
                    hubo_empate = false;
                }
            }
        }
    }
    
    if (num_empates > 1) {
        int indice_aleatorio = rand() % num_empates;
        fila_pivote = empates[indice_aleatorio];
        g_print("EMPATE DETECTADO: Se eligió arbitrariamente la fila %d de %d opciones\n", 
               fila_pivote + 1, num_empates);
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
    bool degenerado = false;
    int variables_cero = 0;
    
    for (int i = 0; i < tabla->num_rest; i++) {
        double valor = tabla->tabla[i + 1][tabla->num_vars + tabla->num_rest];
        g_print("Verificando variable básica en fila %d: valor = %.15f\n", i+1, valor);
        
        if (fabs(valor) < EPSILON) {
            g_print("DEGENERACIÓN DETECTADA: Variable básica en fila %d tiene valor cero: %.15f\n", i+1, valor);
            variables_cero++;
            degenerado = true;
        }
    }
    
    if (degenerado) {
        g_print("PROBLEMA DEGENERADO CONFIRMADO: %d variable(s) básica(s) con valor cero\n", variables_cero);
    } else {
        g_print("No se detectó degeneración - todas las variables básicas tienen valores distintos de cero\n");
    }
    
    return degenerado;
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

int encontrar_variable_no_basica_con_cero(TablaSimplex *tabla) {
    for (int j = 0; j < tabla->num_vars; j++) {
        bool es_basica = false;
        for (int i = 0; i < tabla->num_rest; i++) {
            if (tabla->variables_basicas[i] == j) {
                es_basica = true;
                break;
            }
        }
        
        if (!es_basica && fabs(tabla->tabla[0][j]) < EPSILON) {
            return j;
        }
    }
    return -1;
}

bool pivotear_para_segunda_solucion(TablaSimplex *tabla, int variable_cero) {
    if (variable_cero == -1) return false;
    
    g_print("Encontrada variable no básica con coeficiente cero: X%d\n", variable_cero + 1);
    
    int fila_pivote = -1;
    double min_ratio = 1e9;
    
    for (int i = 1; i <= tabla->num_rest; i++) {
        if (tabla->tabla[i][variable_cero] > EPSILON) {
            double ratio = tabla->tabla[i][tabla->num_vars + tabla->num_rest] / 
                          tabla->tabla[i][variable_cero];
            if (ratio >= 0 && (ratio < min_ratio - EPSILON || fila_pivote == -1)) {
                min_ratio = ratio;
                fila_pivote = i - 1;
            }
        }
    }
    
    if (fila_pivote == -1) {
        g_print("No se encontró fila pivote válida para variable X%d\n", variable_cero + 1);
        return false;
    }
    
    g_print("Pivoteando en fila %d, columna %d para segunda solución\n", 
           fila_pivote + 1, variable_cero);
    
    pivotear(tabla, fila_pivote, variable_cero);
    return true;
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
    resultado->soluciones_adicionales = NULL;
    resultado->num_soluciones_adicionales = 0;
    resultado->operaciones_pivoteo = NULL;
    
    int iteracion = 0;
    const int MAX_ITERACIONES = 50;
    
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

        OperacionPivoteo *op = g_new0(OperacionPivoteo, 1);
        op->iteracion = iteracion;
        op->columna_pivote = col_pivote;
        op->fila_pivote = fila_pivote;
        op->variable_entra = col_pivote;
        if (fila_pivote >= 0 && fila_pivote < tabla_trabajo->num_rest) {
            op->variable_sale = tabla_trabajo->variables_basicas[fila_pivote];
        } else {
            op->variable_sale = -1;
        }
        if (fila_pivote >= 0 && col_pivote >= 0) {
            op->elemento_pivote = tabla_trabajo->tabla[fila_pivote + 1][col_pivote];
        } else {
            op->elemento_pivote = 0.0;
        }

        resultado->operaciones_pivoteo = g_list_append(resultado->operaciones_pivoteo, op);
        
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
    
    resultado->valor_optimo = tabla_trabajo->tabla[0][tabla_trabajo->num_vars + tabla_trabajo->num_rest];
    
    bool problema_degenerado = es_degenerado(tabla_trabajo);
    if (problema_degenerado) {
        g_string_append_printf(resultado->proceso, "PROBLEMA DEGENERADO detectado\n");
        g_string_append_printf(resultado->proceso, "Al menos una variable básica tiene valor cero\n");
        resultado->tipo_solucion = DEGENERADO;
        
        if (!resultado->mensaje) {
            resultado->mensaje = g_strdup("Se detectó degeneración en el problema");
        }
    }
    
    if (resultado->tipo_solucion != DEGENERADO && es_solucion_multiple(tabla_trabajo)) {
        g_string_append_printf(resultado->proceso, "SOLUCION MULTIPLE detectada\n");
        resultado->tipo_solucion = SOLUCION_MULTIPLE;
        resultado->mensaje = g_strdup("Se detectaron soluciones múltiples");
        resultado->tabla_final = copiar_tabla_simplex(tabla_trabajo);
        
        int variable_cero = encontrar_variable_no_basica_con_cero(tabla_trabajo);
        
        if (variable_cero != -1) {
            g_string_append_printf(resultado->proceso, "Variable no básica con coeficiente cero: X%d\n", variable_cero + 1);
            
            resultado->segunda_tabla = copiar_tabla_simplex(tabla_trabajo);
            
            if (pivotear_para_segunda_solucion(resultado->segunda_tabla, variable_cero)) {
                g_string_append_printf(resultado->proceso, "Segunda solución obtenida exitosamente\n");
                
                if (mostrar_tablas) {
                    resultado->tablas_intermedias = g_list_append(resultado->tablas_intermedias, 
                        copiar_tabla_simplex(resultado->segunda_tabla));
                }
            } else {
                g_string_append_printf(resultado->proceso, "No se pudo obtener segunda solución\n");
                liberar_tabla_simplex(resultado->segunda_tabla);
                resultado->segunda_tabla = NULL;
            }
        } else {
            g_string_append_printf(resultado->proceso, "No se encontró variable no básica con coeficiente cero\n");
            resultado->segunda_tabla = NULL;
        }
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
    
    if (resultado->soluciones_adicionales) {
        for (int i = 0; i < resultado->num_soluciones_adicionales; i++) {
            if (resultado->soluciones_adicionales[i]) {
                g_free(resultado->soluciones_adicionales[i]);
            }
        }
        g_free(resultado->soluciones_adicionales);
        resultado->soluciones_adicionales = NULL;
    }
    resultado->num_soluciones_adicionales = 0;
    
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
    
    if (resultado->operaciones_pivoteo) {
        GList *iter = resultado->operaciones_pivoteo;
        while (iter) {
            if (iter->data) {
                g_free(iter->data);
            }
            iter = g_list_next(iter);
        }
        g_list_free(resultado->operaciones_pivoteo);
        resultado->operaciones_pivoteo = NULL;
    }
    
    if (resultado->variables_entran) {
        g_free(resultado->variables_entran);
        resultado->variables_entran = NULL;
    }
    
    if (resultado->variables_salen) {
        g_free(resultado->variables_salen);
        resultado->variables_salen = NULL;
    }
    
    if (resultado->filas_pivote) {
        g_free(resultado->filas_pivote);
        resultado->filas_pivote = NULL;
    }
    
    if (resultado->columnas_pivote) {
        g_free(resultado->columnas_pivote);
        resultado->columnas_pivote = NULL;
    }
    
    g_free(resultado);
}
