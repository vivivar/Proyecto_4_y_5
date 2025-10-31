#include "simplex.h"
#include <string.h>

TablaSimplex* crear_tabla_simplex(int num_vars, int num_rest, TipoProblema tipo) {
    TablaSimplex *tabla = g_new0(TablaSimplex, 1);
    tabla->num_vars = num_vars;
    tabla->num_rest = num_rest;
    tabla->tipo = tipo;
    
    // +1 para la fila Z, +1 para la columna del lado derecho
    tabla->tabla = g_new0(double*, num_rest + 1);
    for (int i = 0; i <= num_rest; i++) {
        tabla->tabla[i] = g_new0(double, num_vars + num_rest + 1);
    }
    
    tabla->variables_basicas = g_new0(int, num_rest);
    
    // Inicializar variables básicas como variables de holgura
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
    g_print("=== ESTABLECIENDO FUNCIÓN OBJETIVO ===\n");
    g_print("Tipo de problema: %s\n", tabla->tipo == MAXIMIZACION ? "MAXIMIZACION" : "MINIMIZACION");
    g_print("Coeficientes recibidos: ");
    for (int j = 0; j < tabla->num_vars; j++) {
        g_print("X%d=%.2f ", j+1, coeficientes[j]);
    }
    g_print("\n");
    
    if (tabla->tipo == MAXIMIZACION) {
        for (int j = 0; j < tabla->num_vars; j++) {
            tabla->tabla[0][j] = coeficientes[j];
            g_print("MAX: Tabla[0][%d] = %.2f\n", j, tabla->tabla[0][j]);
        }
    } else {
        for (int j = 0; j < tabla->num_vars; j++) {
            tabla->tabla[0][j] = -coeficientes[j];
            g_print("MIN: Tabla[0][%d] = -%.2f = %.2f\n", j, coeficientes[j], tabla->tabla[0][j]);
        }
    }
    
    // El valor de Z inicial es 0
    tabla->tabla[0][tabla->num_vars + tabla->num_rest] = 0.0;
    
    g_print("Fila Z completa: ");
    for (int j = 0; j <= tabla->num_vars + tabla->num_rest; j++) {
        g_print("%.2f ", tabla->tabla[0][j]);
    }
    g_print("\n");
}

void agregar_restriccion_simplex(TablaSimplex *tabla, int indice, double coeficientes[], double lado_derecho) {
    if (indice < 0 || indice >= tabla->num_rest) return;
    
    int fila = indice + 1;  
    g_print("Restricción %d: ", indice + 1);
    for (int j = 0; j < tabla->num_vars; j++) {
        g_print("%.2f*X%d ", coeficientes[j], j+1);
    }
    g_print("<= %.2f\n", lado_derecho);
    
    // Copiar coeficientes de las variables de decisión
    for (int j = 0; j < tabla->num_vars; j++) {
        tabla->tabla[fila][j] = coeficientes[j];
    }
    tabla->tabla[fila][tabla->num_vars + indice] = 1.0;
    
    // Establecer lado derecho
    tabla->tabla[fila][tabla->num_vars + tabla->num_rest] = lado_derecho;
}

bool es_optima(TablaSimplex *tabla) {
    g_print("Verificando optimalidad - Coeficientes en fila Z: ");
    for (int j = 0; j < tabla->num_vars + tabla->num_rest; j++) {
        g_print("%.4f ", tabla->tabla[0][j]);
    }
    g_print("\n");
    
    // Solo verificar las variables de decisión (primeras num_vars columnas)
    for (int j = 0; j < tabla->num_vars; j++) {
        if (tabla->tabla[0][j] > EPSILON) {
            g_print("No óptima - coeficiente positivo en variable X%d: %.4f\n", j+1, tabla->tabla[0][j]);
            return false;
        }
    }
    
    g_print("Tabla es óptima - todos los coeficientes de variables de decisión son <= 0\n");
    return true;
}

int encontrar_columna_pivote(TablaSimplex *tabla) {
    int col_pivote = -1;
    double max_valor = -1e9;
    
    g_print("Buscando columna pivote...\n");
    
    // Buscar el coeficiente más positivo en la fila 0 SOLO en variables de decisión
    for (int j = 0; j < tabla->num_vars; j++) {
        g_print("Variable X%d: coeficiente = %.4f\n", j+1, tabla->tabla[0][j]);
        
        if (tabla->tabla[0][j] > max_valor + EPSILON) {
            max_valor = tabla->tabla[0][j];
            col_pivote = j;
            g_print("Nueva columna pivote candidata: %d, valor: %.4f\n", col_pivote, max_valor);
        }
    }
    
    g_print("Columna pivote final: %d, valor máximo: %.4f\n", col_pivote, max_valor);
    
    // Si no encontramos valores positivos, la solución es óptima
    if (max_valor <= EPSILON) {
        g_print("No hay coeficientes positivos en variables de decisión - solución óptima\n");
        return -1;
    }
    
    return col_pivote;
}

int encontrar_fila_pivote(TablaSimplex *tabla, int col_pivote) {
    if (col_pivote == -1) return -1;
    
    int fila_pivote = -1;
    double min_ratio = 1e9;
    
    g_print("Buscando fila pivote para columna %d\n", col_pivote);
    
    for (int i = 1; i <= tabla->num_rest; i++) {
        if (tabla->tabla[i][col_pivote] > EPSILON) {
            double ratio = tabla->tabla[i][tabla->num_vars + tabla->num_rest] / 
                          tabla->tabla[i][col_pivote];
            
            g_print("Fila %d: LADO_DERECHO=%.2f / COEF=%.2f = RATIO=%.2f\n", 
                   i, tabla->tabla[i][tabla->num_vars + tabla->num_rest],
                   tabla->tabla[i][col_pivote], ratio);
            
            if (ratio < min_ratio - EPSILON) {
                min_ratio = ratio;
                fila_pivote = i - 1;  // Convertir a índice base 0 para variables_basicas
                g_print("Nueva fila pivote: %d (ratio=%.2f)\n", fila_pivote, min_ratio);
            }
        } else {
            g_print("Fila %d: coeficiente %.2f <= 0, se salta\n", i, tabla->tabla[i][col_pivote]);
        }
    }
    
    g_print("Fila pivote final: %d\n", fila_pivote);
    return fila_pivote;
}

void pivotear(TablaSimplex *tabla, int fila_pivote, int col_pivote) {
    if (fila_pivote == -1 || col_pivote == -1) return;
    
    int fila_real = fila_pivote + 1;  // Convertir a índice real de la tabla
    double elemento_pivote = tabla->tabla[fila_real][col_pivote];
    
    g_print("=== PIVOTEO ===\n");
    g_print("Pivoteando en (%d, %d) = %.4f\n", fila_real, col_pivote, elemento_pivote);
    g_print("Variable básica anterior: %d\n", tabla->variables_basicas[fila_pivote]);
    
    // 1. Normalizar la fila pivote
    for (int j = 0; j <= tabla->num_vars + tabla->num_rest; j++) {
        tabla->tabla[fila_real][j] /= elemento_pivote;
    }
    
    // 2. Actualizar todas las demás filas
    for (int i = 0; i <= tabla->num_rest; i++) {
        if (i == fila_real) continue;
        
        double factor = tabla->tabla[i][col_pivote];
        if (fabs(factor) > EPSILON) {
            for (int j = 0; j <= tabla->num_vars + tabla->num_rest; j++) {
                tabla->tabla[i][j] -= factor * tabla->tabla[fila_real][j];
            }
        }
    }
    
    // 3. Actualizar la variable básica
    tabla->variables_basicas[fila_pivote] = col_pivote;
    g_print("Nueva variable básica: %d\n", col_pivote);
    
    g_print("Tabla después del pivoteo:\n");
    imprimir_tabla_consola(tabla);
}

bool es_no_acotado(TablaSimplex *tabla, int col_pivote) {
    if (col_pivote == -1) return false;
    
    // Verificar si todas las entradas en la columna pivote son <= 0
    for (int i = 1; i <= tabla->num_rest; i++) {
        if (tabla->tabla[i][col_pivote] > EPSILON) {
            return false;
        }
    }
    return true;
}

bool es_solucion_multiple(TablaSimplex *tabla) {
    // Una solución es múltiple si hay alguna variable no básica con coeficiente 0 en la fila Z
    for (int j = 0; j < tabla->num_vars; j++) {
        bool es_basica = false;
        
        // Verificar si esta variable es básica
        for (int i = 0; i < tabla->num_rest; i++) {
            if (tabla->variables_basicas[i] == j) {
                es_basica = true;
                break;
            }
        }
        
        // Si no es básica y tiene coeficiente 0 (o casi 0), hay solución múltiple
        if (!es_basica && fabs(tabla->tabla[0][j]) < EPSILON) {
            return true;
        }
    }
    return false;
}

bool es_degenerado(TablaSimplex *tabla) {
    // Un problema es degenerado si alguna variable básica tiene valor 0
    for (int i = 0; i < tabla->num_rest; i++) {
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
    
    // Las variables básicas toman el valor del lado derecho
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
    
    // Copiar la tabla completa
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

// Función auxiliar para debug
void imprimir_tabla_consola(TablaSimplex *tabla) {
    g_print("\n=== TABLA SIMPLEX ===\n");
    g_print("Z: ");
    for (int j = 0; j <= tabla->num_vars + tabla->num_rest; j++) {
        g_print("%8.3f ", tabla->tabla[0][j]);
    }
    g_print("\n");
    
    for (int i = 1; i <= tabla->num_rest; i++) {
        if (tabla->variables_basicas[i-1] < tabla->num_vars) {
            g_print("X%d: ", tabla->variables_basicas[i-1] + 1);
        } else {
            g_print("S%d: ", tabla->variables_basicas[i-1] - tabla->num_vars + 1);
        }
        for (int j = 0; j <= tabla->num_vars + tabla->num_rest; j++) {
            g_print("%8.3f ", tabla->tabla[i][j]);
        }
        g_print("\n");
    }
    g_print("====================\n\n");
}

ResultadoSimplex* ejecutar_simplex_completo(TablaSimplex *tabla, gboolean mostrar_tablas) {
    ResultadoSimplex *resultado = g_new0(ResultadoSimplex, 1);
    resultado->proceso = g_string_new("");
    resultado->solucion = g_new0(double, tabla->num_vars);
    resultado->tipo_solucion = SOLUCION_UNICA;
    resultado->mensaje = NULL;
    resultado->tablas_intermedias = NULL;
    resultado->iteraciones = 0;
    
    int iteracion = 0;
    const int MAX_ITERACIONES = 100;
    
    g_string_append_printf(resultado->proceso, "--- INICIANDO ALGORITMO SIMPLEX ---\n");
    g_string_append_printf(resultado->proceso, "Tipo de problema: %s\n", 
                          tabla->tipo == MAXIMIZACION ? "MAXIMIZACION" : "MINIMIZACION");
    
    // Trabajar con una copia para no modificar la original
    TablaSimplex *tabla_trabajo = copiar_tabla_simplex(tabla);
    
    g_print("=== INICIANDO ALGORITMO SIMPLEX ===\n");
    g_print("Problema: %s\n", tabla->tipo == MAXIMIZACION ? "MAXIMIZACION" : "MINIMIZACION");
    g_print("Variables: %d, Restricciones: %d\n", tabla->num_vars, tabla->num_rest);
    
    // Guardar tabla inicial si se solicitan tablas intermedias
    if (mostrar_tablas) {
        resultado->tablas_intermedias = g_list_append(resultado->tablas_intermedias, copiar_tabla_simplex(tabla_trabajo));
    }
    
    // Mostrar tabla inicial
    g_print("=== TABLA INICIAL ===\n");
    imprimir_tabla_consola(tabla_trabajo);
    
    while (iteracion < MAX_ITERACIONES) {
        g_print("\n--- Iteración %d ---\n", iteracion);
        g_string_append_printf(resultado->proceso, "\n--- Iteración %d ---\n", iteracion);
        
        if (mostrar_tablas) {
            imprimir_tabla_simplex(tabla_trabajo, resultado->proceso);
        }
        
        // Verificar si ya es óptima
        if (es_optima(tabla_trabajo)) {
            g_print("Solución óptima alcanzada en iteración %d\n", iteracion);
            g_string_append_printf(resultado->proceso, "Solución óptima alcanzada\n");
            break;
        }
        
        // Encontrar columna pivote
        int col_pivote = encontrar_columna_pivote(tabla_trabajo);
        g_string_append_printf(resultado->proceso, "Columna pivote: %d\n", col_pivote);
        
        if (col_pivote == -1) {
            g_print("No se encontró columna pivote - solución óptima\n");
            g_string_append_printf(resultado->proceso, "No se encontró columna pivote - solución óptima\n");
            break;
        }
        
        // Verificar si es no acotado
        if (es_no_acotado(tabla_trabajo, col_pivote)) {
            g_print("PROBLEMA NO ACOTADO\n");
            g_string_append_printf(resultado->proceso, "PROBLEMA NO ACOTADO\n");
            resultado->tipo_solucion = NO_ACOTADO;
            resultado->mensaje = g_strdup("El problema es no acotado");
            break;
        }
        
        // Encontrar fila pivote
        int fila_pivote = encontrar_fila_pivote(tabla_trabajo, col_pivote);
        g_string_append_printf(resultado->proceso, "Fila pivote: %d\n", fila_pivote);
        
        if (fila_pivote == -1) {
            g_print("No se encontró fila pivote válida\n");
            g_string_append_printf(resultado->proceso, "No se encontró fila pivote válida\n");
            resultado->tipo_solucion = NO_ACOTADO;
            resultado->mensaje = g_strdup("No se encontró fila pivote válida");
            break;
        }
        
        // Verificar degeneración
        if (es_degenerado(tabla_trabajo)) {
            g_print("PROBLEMA DEGENERADO detectado\n");
            g_string_append_printf(resultado->proceso, "PROBLEMA DEGENERADO detectado\n");
            resultado->tipo_solucion = DEGENERADO;
            if (!resultado->mensaje) {
                resultado->mensaje = g_strdup("Problema degenerado detectado");
            }
        }
        
        // Realizar pivoteo
        g_print("Pivoteando en fila %d, columna %d\n", fila_pivote + 1, col_pivote);
        g_string_append_printf(resultado->proceso, "Pivoteando en (%d, %d)\n", fila_pivote + 1, col_pivote);
        pivotear(tabla_trabajo, fila_pivote, col_pivote);
        
        // Guardar tabla intermedia si se solicitan
        if (mostrar_tablas) {
            resultado->tablas_intermedias = g_list_append(resultado->tablas_intermedias, copiar_tabla_simplex(tabla_trabajo));
        }
        
        iteracion++;
    }
    
    resultado->iteraciones = iteracion;
    
    if (iteracion >= MAX_ITERACIONES) {
        g_print("ADVERTENCIA: Máximo de iteraciones alcanzado\n");
        g_string_append_printf(resultado->proceso, "ADVERTENCIA: Máximo de iteraciones alcanzado\n");
        if (!resultado->mensaje) {
            resultado->mensaje = g_strdup("Se alcanzó el máximo número de iteraciones");
        }
    }
    
    // Extraer la solución final
    extraer_solucion(tabla_trabajo, resultado->solucion);
    
    // Calcular valor óptimo 
    resultado->valor_optimo = -tabla_trabajo->tabla[0][tabla_trabajo->num_vars + tabla_trabajo->num_rest];
    
    g_print("Valor óptimo calculado: %.4f\n", resultado->valor_optimo);
    g_print("Solución: ");
    for (int i = 0; i < tabla_trabajo->num_vars; i++) {
        g_print("X%d=%.2f ", i+1, resultado->solucion[i]);
    }
    g_print("\n");
    
    // Verificar solución múltiple
    if (resultado->tipo_solucion == SOLUCION_UNICA && es_solucion_multiple(tabla_trabajo)) {
        g_print("SOLUCION MULTIPLE detectada\n");
        g_string_append_printf(resultado->proceso, "SOLUCION MULTIPLE detectada\n");
        resultado->tipo_solucion = SOLUCION_MULTIPLE;
        resultado->mensaje = g_strdup("Se detectaron soluciones múltiples");
    }
    
    // Guardar tabla final 
    TablaSimplex *copia_final = copiar_tabla_simplex(tabla_trabajo);
    resultado->tabla_final = *copia_final;
    g_string_append_printf(resultado->proceso, "\n--- FINALIZANDO ALGORITMO SIMPLEX ---\n");
    g_string_append_printf(resultado->proceso, "Iteraciones totales: %d\n", iteracion);
    g_string_append_printf(resultado->proceso, "Valor óptimo: %.4f\n", resultado->valor_optimo);
    
    // Liberar tabla de trabajo
    liberar_tabla_simplex(tabla_trabajo);
    
    g_print("=== ALGORITMO SIMPLEX COMPLETADO ===\n");
    g_print("Iteraciones: %d, Valor óptimo: %.4f\n", iteracion, resultado->valor_optimo);
    
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
    
    if (resultado->solucion) g_free(resultado->solucion);
    if (resultado->proceso) g_string_free(resultado->proceso, TRUE);
    if (resultado->mensaje) g_free(resultado->mensaje);
    
    // Liberar tablas intermedias
    if (resultado->tablas_intermedias) {
        GList *iter = resultado->tablas_intermedias;
        while (iter) {
            liberar_tabla_simplex((TablaSimplex*)iter->data);
            iter = g_list_next(iter);
        }
        g_list_free(resultado->tablas_intermedias);
    }
    
    g_free(resultado);
}