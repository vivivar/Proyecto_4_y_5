#include "simplex.h"

TablaSimplex* crear_tabla_simplex(int num_vars, int num_rest, TipoProblema tipo) {
    TablaSimplex *tabla = g_new0(TablaSimplex, 1);
    tabla->num_vars = num_vars;
    tabla->num_rest = num_rest;
    tabla->tipo = tipo;
    
    // Asignar memoria para la tabla
    tabla->tabla = g_new0(double*, num_rest + 1);
    for (int i = 0; i <= num_rest; i++) {
        tabla->tabla[i] = g_new0(double, num_vars + num_rest + 1);
    }
    
    // Asignar memoria para variables básicas
    tabla->variables_basicas = g_new0(int, num_rest);
    
    // Inicializar variables básicas (holguras)
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
    
    for (int j = 0; j < tabla->num_vars; j++) {
        tabla->tabla[indice + 1][j] = coeficientes[j];
    }
    
    tabla->tabla[indice + 1][tabla->num_vars + indice] = 1.0;
    tabla->tabla[indice + 1][tabla->num_vars + tabla->num_rest] = lado_derecho;
}

bool es_optima(TablaSimplex *tabla) {
    for (int j = 0; j < tabla->num_vars + tabla->num_rest; j++) {
        if (tabla->tabla[0][j] > EPSILON) {
            return false;
        }
    }
    return true;
}

int encontrar_columna_pivote(TablaSimplex *tabla) {
    int col_pivote = -1;
    double max_valor = 0.0;
    
    // Para ambos casos (max y min) buscamos el coeficiente más positivo
    for (int j = 0; j < tabla->num_vars + tabla->num_rest; j++) {
        if (tabla->tabla[0][j] > max_valor + EPSILON) {
            max_valor = tabla->tabla[0][j];
            col_pivote = j;
        }
    }
    
    return col_pivote;
}

int encontrar_fila_pivote(TablaSimplex *tabla, int col_pivote) {
    if (col_pivote == -1) return -1;
    
    int fila_pivote = -1;
    double min_ratio = -1.0;
    
    for (int i = 0; i < tabla->num_rest; i++) {
        if (tabla->tabla[i + 1][col_pivote] > EPSILON) {
            double ratio = tabla->tabla[i + 1][tabla->num_vars + tabla->num_rest] / 
                          tabla->tabla[i + 1][col_pivote];
            
            if (fila_pivote == -1 || ratio < min_ratio - EPSILON) {
                min_ratio = ratio;
                fila_pivote = i;
            }
        }
    }
    
    return fila_pivote;
}

void resolver_empates(double ratios[], bool disponibles[], int size, int *fila_ganadora) {
    double min_ratio = ratios[*fila_ganadora];
    int primera_fila = -1;
    
    for (int i = 0; i < size; i++) {
        if (disponibles[i] && fabs(ratios[i] - min_ratio) < EPSILON) {
            if (primera_fila == -1) {
                primera_fila = i;
            }
        }
    }
    
    if (primera_fila != -1) {
        *fila_ganadora = primera_fila;
    }
}

void pivotear(TablaSimplex *tabla, int fila_pivote, int col_pivote) {
    if (fila_pivote == -1 || col_pivote == -1) return;
    
    int fila_real = fila_pivote + 1;
    double elemento_pivote = tabla->tabla[fila_real][col_pivote];
    
    // Dividir la fila pivote por el elemento pivote
    for (int j = 0; j <= tabla->num_vars + tabla->num_rest; j++) {
        tabla->tabla[fila_real][j] /= elemento_pivote;
    }
    
    // Actualizar las demás filas
    for (int i = 0; i <= tabla->num_rest; i++) {
        if (i == fila_real) continue;
        
        double factor = tabla->tabla[i][col_pivote];
        for (int j = 0; j <= tabla->num_vars + tabla->num_rest; j++) {
            tabla->tabla[i][j] -= factor * tabla->tabla[fila_real][j];
        }
    }
    
    // Actualizar variables básicas
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
    // Hay solución múltiple si alguna variable no básica tiene coeficiente 0 en la fila objetivo
    for (int j = 0; j < tabla->num_vars; j++) {
        bool es_basica = false;
        
        // Verificar si la variable j es básica
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
        int var_basica = tabla->variables_basicas[i];
        double valor = tabla->tabla[i + 1][tabla->num_vars + tabla->num_rest];
        
        if (fabs(valor) < EPSILON) {
            return true;
        }
    }
    return false;
}

void extraer_solucion(TablaSimplex *tabla, double solucion[]) {
    // Inicializar todas las variables a 0
    for (int j = 0; j < tabla->num_vars; j++) {
        solucion[j] = 0.0;
    }
    
    // Asignar valores a las variables básicas
    for (int i = 0; i < tabla->num_rest; i++) {
        int var_index = tabla->variables_basicas[i];
        if (var_index < tabla->num_vars) {
            solucion[var_index] = tabla->tabla[i + 1][tabla->num_vars + tabla->num_rest];
        }
    }
}

// Función para copiar una tabla simplex
TablaSimplex* copiar_tabla_simplex(const TablaSimplex *original) {
    if (!original) return NULL;
    
    TablaSimplex *copia = crear_tabla_simplex(original->num_vars, original->num_rest, original->tipo);
    
    // Copiar datos de la tabla
    for (int i = 0; i <= original->num_rest; i++) {
        for (int j = 0; j <= original->num_vars + original->num_rest; j++) {
            copia->tabla[i][j] = original->tabla[i][j];
        }
    }
    
    // Copiar variables básicas
    for (int i = 0; i < original->num_rest; i++) {
        copia->variables_basicas[i] = original->variables_basicas[i];
    }
    
    return copia;
}

// Función principal adaptada 
ResultadoSimplex* ejecutar_simplex_completo(TablaSimplex *tabla, gboolean mostrar_tablas) {
    ResultadoSimplex *resultado = g_new0(ResultadoSimplex, 1);
    resultado->proceso = g_string_new("");
    resultado->solucion = g_new0(double, tabla->num_vars);
    resultado->tipo_solucion = SOLUCION_UNICA;
    resultado->mensaje = NULL;
    
    int iteracion = 0;
    const int MAX_ITERACIONES = 100;
    
    g_string_append_printf(resultado->proceso, "--- INICIANDO ALGORITMO SIMPLEX ---\n");
    g_string_append_printf(resultado->proceso, "Tipo de problema: %s\n", 
                          tabla->tipo == MAXIMIZACION ? "MAXIMIZACION" : "MINIMIZACION");
    
    // Crear una copia de trabajo de la tabla
    TablaSimplex *tabla_trabajo = copiar_tabla_simplex(tabla);
    
    while (iteracion < MAX_ITERACIONES && !es_optima(tabla_trabajo)) {
        g_string_append_printf(resultado->proceso, "\n--- Iteración %d ---\n", iteracion + 1);
        
        if (mostrar_tablas) {
            imprimir_tabla_simplex(tabla_trabajo, resultado->proceso);
        }
        
        int col_pivote = encontrar_columna_pivote(tabla_trabajo);
        g_string_append_printf(resultado->proceso, "Columna pivote: %d\n", col_pivote);
        
        if (col_pivote == -1) {
            g_string_append_printf(resultado->proceso, "No se encontró columna pivote - solución óptima alcanzada\n");
            break;
        }
        
        if (es_no_acotado(tabla_trabajo, col_pivote)) {
            g_string_append_printf(resultado->proceso, "PROBLEMA NO ACOTADO - Columna %d no tiene elementos positivos\n", col_pivote);
            resultado->tipo_solucion = NO_ACOTADO;
            resultado->mensaje = g_strdup("El problema es no acotado");
            break;
        }
        
        int fila_pivote = encontrar_fila_pivote(tabla_trabajo, col_pivote);
        g_string_append_printf(resultado->proceso, "Fila pivote: %d\n", fila_pivote);
        
        if (fila_pivote == -1) {
            g_string_append_printf(resultado->proceso, "PROBLEMA NO ACOTADO - No se encontró fila pivote válida\n");
            resultado->tipo_solucion = NO_ACOTADO;
            resultado->mensaje = g_strdup("El problema es no acotado");
            break;
        }
        
        if (es_degenerado(tabla_trabajo)) {
            g_string_append_printf(resultado->proceso, "PROBLEMA DEGENERADO detectado\n");
            resultado->tipo_solucion = DEGENERADO;
            if (!resultado->mensaje) {
                resultado->mensaje = g_strdup("Problema degenerado detectado");
            }
        }
        
        g_string_append_printf(resultado->proceso, "Pivoteando en (%d, %d)\n", fila_pivote + 1, col_pivote + 1);
        pivotear(tabla_trabajo, fila_pivote, col_pivote);
        iteracion++;
    }
    
    if (iteracion >= MAX_ITERACIONES) {
        g_string_append_printf(resultado->proceso, "ADVERTENCIA: Se alcanzó el máximo número de iteraciones\n");
        resultado->mensaje = g_strdup("Se alcanzó el máximo número de iteraciones");
    }
    
    // Verificar solución múltiple
    if (resultado->tipo_solucion == SOLUCION_UNICA && es_solucion_multiple(tabla_trabajo)) {
        g_string_append_printf(resultado->proceso, "SOLUCION MULTIPLE detectada\n");
        resultado->tipo_solucion = SOLUCION_MULTIPLE;
        resultado->mensaje = g_strdup("Se detectaron soluciones múltiples");
        
        // Guardar la primera tabla óptima
        resultado->tabla_final = *copiar_tabla_simplex(tabla_trabajo);
        
        // Encontrar una variable no básica con coeficiente 0 para pivotear
        for (int j = 0; j < tabla_trabajo->num_vars; j++) {
            bool es_basica = false;
            for (int i = 0; i < tabla_trabajo->num_rest; i++) {
                if (tabla_trabajo->variables_basicas[i] == j) {
                    es_basica = true;
                    break;
                }
            }
            
            if (!es_basica && fabs(tabla_trabajo->tabla[0][j]) < EPSILON) {
                g_string_append_printf(resultado->proceso, "Pivoteando en variable no básica x%d para obtener segunda solución\n", j + 1);
                int nueva_col_pivote = j;
                int nueva_fila_pivote = encontrar_fila_pivote(tabla_trabajo, nueva_col_pivote);
                
                if (nueva_fila_pivote != -1) {
                    // Crear una copia para la segunda solución
                    TablaSimplex *tabla_segunda = copiar_tabla_simplex(tabla_trabajo);
                    pivotear(tabla_segunda, nueva_fila_pivote, nueva_col_pivote);
                    resultado->segunda_tabla = *tabla_segunda;
                    liberar_tabla_simplex(tabla_segunda);
                    break;
                }
            }
        }
    } else {
        // Guardar tabla final para solución única
        resultado->tabla_final = *copiar_tabla_simplex(tabla_trabajo);
    }

    // Extraer solución y valor óptimo
    extraer_solucion(tabla_trabajo, resultado->solucion);
    
    // El valor óptimo está en la esquina inferior derecha
    resultado->valor_optimo = tabla_trabajo->tabla[0][tabla_trabajo->num_vars + tabla_trabajo->num_rest];
    
    // Para minimización, ajustamos el signo
    if (tabla_trabajo->tipo == MINIMIZACION) {
        resultado->valor_optimo = -resultado->valor_optimo;
    }
    
    g_string_append_printf(resultado->proceso, "\n--- FINALIZANDO ALGORITMO SIMPLEX ---\n");
    g_string_append_printf(resultado->proceso, "Iteraciones totales: %d\n", iteracion);
    g_string_append_printf(resultado->proceso, "Tipo de solución: %d\n", resultado->tipo_solucion);
    
    // Liberar tabla de trabajo
    liberar_tabla_simplex(tabla_trabajo);
    
    return resultado;
}

void imprimir_tabla_simplex(TablaSimplex *tabla, GString *output) {
    if (!output) return;
    
    g_string_append_printf(output, "\nTabla Simplex (%d vars, %d rest):\n", tabla->num_vars, tabla->num_rest);
    g_string_append_printf(output, "F0: ");
    for (int j = 0; j <= tabla->num_vars + tabla->num_rest; j++) {
        g_string_append_printf(output, "%8.2f ", tabla->tabla[0][j]);
    }
    g_string_append_printf(output, "\n");
    
    for (int i = 1; i <= tabla->num_rest; i++) {
        g_string_append_printf(output, "F%d: ", i);
        for (int j = 0; j <= tabla->num_vars + tabla->num_rest; j++) {
            g_string_append_printf(output, "%8.2f ", tabla->tabla[i][j]);
        }
        g_string_append_printf(output, "\n");
    }
    
    g_string_append_printf(output, "Variables básicas: ");
    for (int i = 0; i < tabla->num_rest; i++) {
        g_string_append_printf(output, "x%d ", tabla->variables_basicas[i] + 1);
    }
    g_string_append_printf(output, "\n");
}

void liberar_resultado(ResultadoSimplex *resultado) {
    if (!resultado) return;
    
    if (resultado->solucion) g_free(resultado->solucion);
    if (resultado->proceso) g_string_free(resultado->proceso, TRUE);
    if (resultado->mensaje) g_free(resultado->mensaje);
    
    g_free(resultado);
}