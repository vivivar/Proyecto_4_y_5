#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAX_VARS 20
#define MAX_REST 10
#define M 1000000.0  // Valor grande para M

typedef struct {
    double coef[MAX_VARS];
    double b;
    int tipo;  // 1: <=, 2: >=, 3: =
    int var_holgura;
    int var_exceso;
    int var_artificial;
} Restriccion;

typedef struct {
    double tabla[MAX_REST + 1][MAX_VARS];
    int num_vars;
    int num_rest;
    int num_holgura;
    int num_exceso;
    int num_artificial;
    int vars_base[MAX_REST];
    char nombres_vars[MAX_VARS][10];
    int es_artificial[MAX_VARS]; // 1 si la variable es artificial
} TablaSimplex;

void inicializarTabla(TablaSimplex *tabla, int num_vars_original, int num_restricciones, 
                     Restriccion restricciones[], int es_maximizacion) {
    int i, j;
    
    // Contar variables adicionales
    tabla->num_holgura = 0;
    tabla->num_exceso = 0;
    tabla->num_artificial = 0;
    
    for (i = 0; i < num_restricciones; i++) {
        if (restricciones[i + 1].tipo == 1) { // <=
            tabla->num_holgura++;
        } else if (restricciones[i + 1].tipo == 2) { // >=
            tabla->num_exceso++;
            tabla->num_artificial++;
        } else { // =
            tabla->num_artificial++;
        }
    }
    
    tabla->num_vars = num_vars_original + tabla->num_holgura + tabla->num_exceso + tabla->num_artificial;
    tabla->num_rest = num_restricciones;
    
    // Inicializar nombres de variables y marcar artificiales
    for (i = 0; i < tabla->num_vars; i++) {
        tabla->es_artificial[i] = 0;
    }
    
    int idx = num_vars_original;
    for (i = 0; i < tabla->num_holgura; i++) {
        sprintf(tabla->nombres_vars[idx], "s%d", i + 1);
        idx++;
    }
    for (i = 0; i < tabla->num_exceso; i++) {
        sprintf(tabla->nombres_vars[idx], "e%d", i + 1);
        idx++;
    }
    for (i = 0; i < tabla->num_artificial; i++) {
        sprintf(tabla->nombres_vars[idx], "a%d", i + 1);
        tabla->es_artificial[idx] = 1; // Marcar como artificial
        idx++;
    }
    
    // Inicializar tabla con ceros
    for (i = 0; i <= tabla->num_rest; i++) {
        for (j = 0; j <= tabla->num_vars; j++) {
            tabla->tabla[i][j] = 0.0;
        }
    }
    
    // Configurar función objetivo original (sin M)
    for (i = 0; i < num_vars_original; i++) {
        tabla->tabla[0][i] = (es_maximizacion ? -restricciones[0].coef[i] : restricciones[0].coef[i]);
    }
    
    // Configurar restricciones
    int cont_holgura = 0, cont_exceso = 0, cont_artificial = 0;
    for (i = 0; i < num_restricciones; i++) {
        // Coeficientes de variables originales
        for (j = 0; j < num_vars_original; j++) {
            tabla->tabla[i + 1][j] = restricciones[i + 1].coef[j];
        }
        
        // Variables de holgura, exceso y artificiales
        if (restricciones[i + 1].tipo == 1) { // <=
            tabla->tabla[i + 1][num_vars_original + cont_holgura] = 1;
            tabla->vars_base[i] = num_vars_original + cont_holgura;
            cont_holgura++;
        } else if (restricciones[i + 1].tipo == 2) { // >=
            tabla->tabla[i + 1][num_vars_original + tabla->num_holgura + cont_exceso] = -1;
            tabla->tabla[i + 1][num_vars_original + tabla->num_holgura + tabla->num_exceso + cont_artificial] = 1;
            tabla->vars_base[i] = num_vars_original + tabla->num_holgura + tabla->num_exceso + cont_artificial;
            cont_exceso++;
            cont_artificial++;
        } else { // =
            tabla->tabla[i + 1][num_vars_original + tabla->num_holgura + tabla->num_exceso + cont_artificial] = 1;
            tabla->vars_base[i] = num_vars_original + tabla->num_holgura + tabla->num_exceso + cont_artificial;
            cont_artificial++;
        }
        
        // Lado derecho
        tabla->tabla[i + 1][tabla->num_vars] = restricciones[i + 1].b;
    }
    
    // Ajustar función objetivo para variables artificiales (agregar términos con M)
    for (i = 0; i < tabla->num_rest; i++) {
        if (tabla->vars_base[i] >= num_vars_original + tabla->num_holgura + tabla->num_exceso) {
            // Es variable artificial - agregar -M * (fila de la restricción) a la función objetivo
            double signo = (es_maximizacion ? -M : M);
            for (j = 0; j <= tabla->num_vars; j++) {
                tabla->tabla[0][j] += signo * tabla->tabla[i + 1][j];
            }
        }
    }
}

void imprimirTabla(TablaSimplex *tabla) {
    int i, j;
    
    printf("\n%8s", "Base");
    for (j = 0; j < tabla->num_vars; j++) {
        printf("%8s", tabla->nombres_vars[j]);
    }
    printf("%8s\n", "b");
    
    printf("%8s", "Z");
    for (j = 0; j < tabla->num_vars; j++) {
        // Mostrar términos con M de forma especial
        if (fabs(tabla->tabla[0][j]) > M/10) {
            // Es un término con M
            double coeficiente = tabla->tabla[0][j] / M;
            if (fabs(coeficiente - round(coeficiente)) < 1e-6) {
                printf("%7.0fM", coeficiente);
            } else {
                printf("%7.1fM", coeficiente);
            }
        } else if (fabs(tabla->tabla[0][j]) > 1e-10) {
            printf("%8.1f", tabla->tabla[0][j]);
        } else {
            printf("%8.0f", 0.0);
        }
    }
    
    // Mostrar valor de Z
    if (fabs(tabla->tabla[0][tabla->num_vars]) > M/10) {
        double coeficiente = tabla->tabla[0][tabla->num_vars] / M;
        if (fabs(coeficiente - round(coeficiente)) < 1e-6) {
            printf("%7.0fM", coeficiente);
        } else {
            printf("%7.1fM", coeficiente);
        }
    } else if (fabs(tabla->tabla[0][tabla->num_vars]) > 1e-10) {
        printf("%8.1f", tabla->tabla[0][tabla->num_vars]);
    } else {
        printf("%8.0f", 0.0);
    }
    printf("\n");
    
    for (i = 1; i <= tabla->num_rest; i++) {
        printf("%8s", tabla->nombres_vars[tabla->vars_base[i - 1]]);
        for (j = 0; j <= tabla->num_vars; j++) {
            if (fabs(tabla->tabla[i][j]) > 1e-10) {
                printf("%8.1f", tabla->tabla[i][j]);
            } else {
                printf("%8.0f", 0.0);
            }
        }
        printf("\n");
    }
    printf("\n");
}

int encontrarColumnaPivote(TablaSimplex *tabla) {
    int j, col_pivote = -1;
    double min_val = 0;
    
    for (j = 0; j < tabla->num_vars; j++) {
        // NO considerar variables artificiales para entrar a la base
        if (tabla->es_artificial[j]) {
            continue;
        }
        
        if (tabla->tabla[0][j] < min_val - 1e-10) {
            min_val = tabla->tabla[0][j];
            col_pivote = j;
        }
    }
    
    return col_pivote;
}

int encontrarFilaPivote(TablaSimplex *tabla, int col_pivote) {
    int i, fila_pivote = -1;
    double min_ratio = 1e15;
    
    for (i = 1; i <= tabla->num_rest; i++) {
        if (tabla->tabla[i][col_pivote] > 1e-10) {
            double ratio = tabla->tabla[i][tabla->num_vars] / tabla->tabla[i][col_pivote];
            if (ratio >= 0 && ratio < min_ratio - 1e-10) {
                min_ratio = ratio;
                fila_pivote = i;
            }
        }
    }
    
    return fila_pivote;
}

void realizarPivote(TablaSimplex *tabla, int fila_pivote, int col_pivote) {
    int i, j;
    double pivote = tabla->tabla[fila_pivote][col_pivote];
    
    // Actualizar variable básica
    tabla->vars_base[fila_pivote - 1] = col_pivote;
    
    // Normalizar fila pivote
    for (j = 0; j <= tabla->num_vars; j++) {
        tabla->tabla[fila_pivote][j] /= pivote;
    }
    
    // Actualizar otras filas
    for (i = 0; i <= tabla->num_rest; i++) {
        if (i != fila_pivote) {
            double factor = tabla->tabla[i][col_pivote];
            for (j = 0; j <= tabla->num_vars; j++) {
                tabla->tabla[i][j] -= factor * tabla->tabla[fila_pivote][j];
            }
        }
    }
}

int verificarOptimalidad(TablaSimplex *tabla) {
    int j;
    for (j = 0; j < tabla->num_vars; j++) {
        // Solo verificar variables no artificiales
        if (!tabla->es_artificial[j] && tabla->tabla[0][j] < -1e-10) {
            return 0; // No óptimo
        }
    }
    return 1; // Óptimo
}

int verificarFactibilidad(TablaSimplex *tabla, int num_vars_original) {
    int i;
    for (i = 0; i < tabla->num_rest; i++) {
        // Si hay variable artificial en la base con valor positivo
        if (tabla->vars_base[i] >= num_vars_original + tabla->num_holgura + tabla->num_exceso) {
            if (fabs(tabla->tabla[i + 1][tabla->num_vars]) > 1e-10) {
                return 0; // No factible
            }
        }
    }
    return 1; // Factible
}

void resolverSimplex(TablaSimplex *tabla, int num_vars_original, int es_maximizacion) {
    int iteracion = 0;
    int max_iteraciones = 100;
    
    printf("=== INICIO DEL METODO SIMPLEX CON GRAN M ===\n");
    printf("M = %.0f\n", M);
    
    while (iteracion < max_iteraciones) {
        printf("\n--- Iteracion %d ---", iteracion + 1);
        imprimirTabla(tabla);
        
        if (verificarOptimalidad(tabla)) {
            printf("SOLUCION OPTIMA ENCONTRADA\n");
            
            if (!verificarFactibilidad(tabla, num_vars_original)) {
                printf("PROBLEMA NO FACTIBLE (variables artificiales en la base con valor positivo)\n");
                return;
            }
            
            // Mostrar solución
            printf("\nSOLUCION OPTIMA:\n");
            double z_valor = tabla->tabla[0][tabla->num_vars];
            // Ajustar Z si hay términos con M residuales
            if (fabs(z_valor) > M/10) {
                printf("Z = %.2f (contiene terminos con M)\n", z_valor);
            } else {
                printf("Z = %.2f\n", es_maximizacion ? z_valor : -z_valor);
            }
            
            for (int i = 0; i < num_vars_original; i++) {
                double valor = 0;
                for (int j = 0; j < tabla->num_rest; j++) {
                    if (tabla->vars_base[j] == i) {
                        valor = tabla->tabla[j + 1][tabla->num_vars];
                        break;
                    }
                }
                printf("%s = %.2f\n", tabla->nombres_vars[i], valor);
            }
            return;
        }
        
        int col_pivote = encontrarColumnaPivote(tabla);
        if (col_pivote == -1) {
            printf("PROBLEMA NO ACOTADO\n");
            return;
        }
        
        int fila_pivote = encontrarFilaPivote(tabla, col_pivote);
        if (fila_pivote == -1) {
            printf("PROBLEMA NO ACOTADO\n");
            return;
        }
        
        printf("Variable entrante: %s\n", tabla->nombres_vars[col_pivote]);
        printf("Variable saliente: %s\n", tabla->nombres_vars[tabla->vars_base[fila_pivote - 1]]);
        
        realizarPivote(tabla, fila_pivote, col_pivote);
        iteracion++;
    }
    
    printf("MAXIMO DE ITERACIONES ALCANZADO\n");
}

int main() {
    TablaSimplex tabla;
    
    printf("METODO DE LA GRAN M - SIMPLEX\n");
    printf("==============================\n");
    
    // Ejemplo del problema proporcionado
    printf("Ejemplo: Maximizar Z = 10000x1 + 3000x2\n");
    printf("Sujeto a:\n");
    printf("x1 + x2 <= 7\n");
    printf("10x1 + 4x2 <= 40\n");
    printf("x2 >= 3\n\n");
    
    // Configurar el problema
    int num_vars_original = 2;
    int num_restricciones = 3;
    int es_maximizacion = 1; // 1 para maximizar, 0 para minimizar
    
    Restriccion restricciones[MAX_REST];
    
    // Función objetivo (índice 0)
    restricciones[0].coef[0] = 10000;
    restricciones[0].coef[1] = 3000;
    
    // Restricción 1: x1 + x2 <= 7
    restricciones[1].coef[0] = 1;
    restricciones[1].coef[1] = 1;
    restricciones[1].b = 7;
    restricciones[1].tipo = 1; // <=
    
    // Restricción 2: 10x1 + 4x2 <= 40
    restricciones[2].coef[0] = 10;
    restricciones[2].coef[1] = 4;
    restricciones[2].b = 40;
    restricciones[2].tipo = 1; // <=
    
    // Restricción 3: x2 >= 3
    restricciones[3].coef[0] = 0;
    restricciones[3].coef[1] = 1;
    restricciones[3].b = 3;
    restricciones[3].tipo = 2; // >=
    
    // Inicializar y resolver
    inicializarTabla(&tabla, num_vars_original, num_restricciones, restricciones, es_maximizacion);
    resolverSimplex(&tabla, num_vars_original, es_maximizacion);
    
    return 0;
}