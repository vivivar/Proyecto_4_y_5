#include "latex.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <glib.h>
#ifndef M_GRANDE
#define M_GRANDE 1.0e6
#endif

#ifndef EPSILON
#define EPSILON 1.0e-10
#endif

// Función auxiliar para escapar caracteres especiales de LaTeX
static char* escape_latex(const char *texto) {
    if (!texto) return g_strdup("");
    
    GString *resultado = g_string_new("");
    for (const char *p = texto; *p; p++) {
        switch (*p) {
            case '&': g_string_append(resultado, "\\&"); break;
            case '%': g_string_append(resultado, "\\%"); break;
            case '$': g_string_append(resultado, "\\$"); break;
            case '#': g_string_append(resultado, "\\#"); break;
            case '_': g_string_append(resultado, "\\_"); break;
            case '{': g_string_append(resultado, "\\{"); break;
            case '}': g_string_append(resultado, "\\}"); break;
            case '~': g_string_append(resultado, "\\textasciitilde{}"); break;
            case '^': g_string_append(resultado, "\\textasciicircum{}"); break;
            case '\\': g_string_append(resultado, "\\textbackslash{}"); break;
            default: g_string_append_c(resultado, *p); break;
        }
    }
    return g_string_free(resultado, FALSE);
}

// Funciones auxiliares para formateo de números y variables
static void formatear_numero(double valor, char *buffer, size_t buffer_size) {
    if (fabs(valor - round(valor)) < 0.0001) {
        snprintf(buffer, buffer_size, "%.0f", valor);
    } else {
        snprintf(buffer, buffer_size, "%.2f", valor);
        for (char *p = buffer; *p; p++) {
            if (*p == ',') *p = '.';
        }
    }
}

static void formatear_nombre_variable_latex(const char *nombre_original, char *buffer, size_t buffer_size) {
    if (!nombre_original) {
        snprintf(buffer, buffer_size, "?");
        return;
    }
    
    if (nombre_original[0] == 'X' || nombre_original[0] == 'x') {
        int num_var = atoi(nombre_original + 1);
        if (num_var > 0) {
            snprintf(buffer, buffer_size, "x_{%d}", num_var);
        } else {
            snprintf(buffer, buffer_size, "\\text{%s}", nombre_original);
        }
    } else if (nombre_original[0] == 'S' || nombre_original[0] == 's') {
        int num_var = atoi(nombre_original + 1);
        if (num_var > 0) {
            snprintf(buffer, buffer_size, "s_{%d}", num_var);
        } else {
            snprintf(buffer, buffer_size, "\\text{%s}", nombre_original);
        }
    } else if (nombre_original[0] == 'E' || nombre_original[0] == 'e') {
        int num_var = atoi(nombre_original + 1);
        if (num_var > 0) {
            snprintf(buffer, buffer_size, "e_{%d}", num_var);
        } else {
            snprintf(buffer, buffer_size, "\\text{%s}", nombre_original);
        }
    } else if (nombre_original[0] == 'A' || nombre_original[0] == 'a') {
        int num_var = atoi(nombre_original + 1);
        if (num_var > 0) {
            snprintf(buffer, buffer_size, "a_{%d}", num_var);
        } else {
            snprintf(buffer, buffer_size, "\\text{%s}", nombre_original);
        }
    } else {
        snprintf(buffer, buffer_size, "\\text{%s}", nombre_original);
    }
}

static void formatear_nombre_variable_tabla(const char *nombre_original, char *buffer, size_t buffer_size) {
    if (!nombre_original) {
        snprintf(buffer, buffer_size, "?");
        return;
    }
    
    char var_latex[64];
    formatear_nombre_variable_latex(nombre_original, var_latex, sizeof(var_latex));
    snprintf(buffer, buffer_size, "$%s$", var_latex);
}

// Función para generar la portada y encabezado del documento
static void generar_portada_latex(GString *latex, const char *nombre_problema) {
    char *nombre_escape = escape_latex(nombre_problema);
    
    g_string_append_printf(latex,
        "\\documentclass[12pt]{article}\n"
        "\\usepackage[utf8]{inputenc}\n"
        "\\usepackage[spanish]{babel}\n"
        "\\usepackage{amsmath,amssymb}\n"
        "\\usepackage{booktabs}\n"
        "\\usepackage{xcolor}\n"
        "\\usepackage{graphicx}\n"
        "\\usepackage{geometry}\n"
        "\\geometry{margin=2.5cm}\n"
        "\\usepackage{fancyhdr}\n"
        "\\setlength{\\headheight}{14.5pt}\n"
        "\\pagestyle{fancy}\n"
        "\\fancyhf{}\n"
        "\\rhead{Investigación de Operaciones}\n"
        "\\lhead{Método Simplex}\n"
        "\n"
        "\\definecolor{basecolor}{RGB}{0,128,0}\n"
        "\\definecolor{entracolor}{RGB}{255,0,0}\n"
        "\\definecolor{empatecolor}{RGB}{0,0,200}\n"
        "\\definecolor{pivotecolor}{RGB}{200,0,200}\n"
        "\n"
        "\\title{Resultados del Método Simplex\\\\\n"
        "\\large Problema: \\textbf{%s}}\n"
        "\\author{\n"
        "Emily Sánchez\\\\\n"
        "Viviana Vargas\\\\\n"
        "\\\\\n"
        "Curso: Investigación de Operaciones\\\\\n"
        "Semestre II: 2025\n"
        "}\n"
        "\\date{\\today}\n"
        "\n"
        "\\begin{document}\n"
        "\n"
        "\\maketitle\n"
        "\\thispagestyle{empty}\n"
        "\n"
        "\\vfill\n"
        "\\begin{center}\n"
        "\\textbf{George Dantzig (1914-2005)}\n"
        "\\\\\n"
        "Creador del Método Simplex\n"
        "\\end{center}\n"
        "\\vfill\n"
        "\n"
        "\\newpage\n"
        "\\tableofcontents\n"
        "\\newpage\n",
        nombre_escape);
    
    g_free(nombre_escape);
}

// Función para generar la explicación del algoritmo simplex
static void generar_algoritmo_simplex_latex(GString *latex) {
    g_string_append_printf(latex,
        "\\section{El Algoritmo Simplex}\n"
        "\n"
        "\\subsection{Historia}\n"
        "El método Simplex fue desarrollado por George Dantzig en 1947 mientras trabajaba para la Fuerza Aérea de los Estados Unidos. \\\\\n"
        "\n"
        "Es uno de los algoritmos más importantes en la historia de la optimización matemática y ha sido fundamental en el desarrollo de la programación lineal. Usa operaciones sobre matrices hasta encontrar la solución óptima o determinar que el problema no tiene solución. Parte de un vértice de la región factible y \"salta\" a vértices adyacentes que mejoren lo encontrado hasta encontrar la condición de salida.\n"
        "\n"
        "\\subsection{Método de la Gran M}\n"
        "El método de la Gran M se utiliza cuando el problema tiene restricciones de tipo $\\geq$ o $=$ que requieren variables artificiales. Se asigna un coeficiente $M$ muy grande en la función objetivo para las variables artificiales, donde:\n"
        "\\begin{itemize}\n"
        "\\item Para \\textbf{maximización}: $M$ es negativo grande ($-M$)\n"
        "\\item Para \\textbf{minimización}: $M$ es positivo grande ($+M$)\n"
        "\\item El valor de $M$ utilizado es: $%.0f$\n"
        "\\end{itemize}\n"
        "Esto fuerza a las variables artificiales a salir de la base en la solución óptima.\n"
        "\n"
        "\\subsection{Propiedades Fundamentales}\n"
        "\\begin{itemize}\n"
        "\\item \\textbf{Convergencia:} El algoritmo converge a la solución óptima en un número finito de pasos\n"
        "\\item \\textbf{Optimalidad:} Garantiza encontrar la solución óptima global para problemas convexos\n"
        "\\item \\textbf{Factibilidad:} Mantiene la factibilidad en cada iteración\n"
        "\\end{itemize}\n\n",
        M_GRANDE);
}

// Función para generar el problema original en LaTeX
void generar_problema_original_latex(GString *latex, ProblemaInfo *info) {
    g_string_append(latex, "\\section{Formulación del Problema}\n\n");
    char *nombre_escape = escape_latex(info->nombre_problema);
    g_string_append_printf(latex, "\\textbf{Problema:} %s\\\\\n", nombre_escape);
    g_free(nombre_escape);
    g_string_append_printf(latex, "\\textbf{Tipo:} %s\\\\\n", (strcmp(info->tipo_problema, "MAX") == 0) ? "Maximización" : "Minimización");
    g_string_append_printf(latex, "\\textbf{Número de variables:} %d\\\\\n", info->num_vars);
    g_string_append_printf(latex, "\\textbf{Número de restricciones:} %d\n\n", info->num_rest);
    g_string_append(latex, "\\subsection{Función Objetivo}\n");
    g_string_append(latex, "\\[\n");
    g_string_append_printf(latex, "%s Z = ", 
                          (strcmp(info->tipo_problema, "MAX") == 0) ? "Maximizar" : "Minimizar");
    
    int primer_coef = 1;
    for (int i = 0; i < info->num_vars; i++) {
        if (fabs(info->coef_obj[i]) > EPSILON) {
            char num_buffer[32];
            formatear_numero(info->coef_obj[i], num_buffer, sizeof(num_buffer));
            
            if (!primer_coef && info->coef_obj[i] >= 0) {
                g_string_append(latex, " + ");
            } else if (!primer_coef && info->coef_obj[i] < 0) {
                g_string_append(latex, " - ");
            } else if (primer_coef && info->coef_obj[i] < 0) {
                g_string_append(latex, "-");
            }
            
            char var_latex[64];
            formatear_nombre_variable_latex(info->nombres_vars[i], var_latex, sizeof(var_latex));
            
            double coef_abs = fabs(info->coef_obj[i]);
            if (fabs(coef_abs - 1.0) > EPSILON) {
                g_string_append_printf(latex, "%s%s", num_buffer, var_latex);
            } else {
                g_string_append_printf(latex, "%s", var_latex);
            }
            primer_coef = 0;
        }
    }
    g_string_append(latex, "\n\\]\n\n");
    g_string_append(latex, "\\subsection{Restricciones}\n");
    g_string_append(latex, "\\[\n");
    g_string_append(latex, "\\begin{cases}\n");

    for (int r = 0; r < info->num_rest; r++) {
        primer_coef = 1;
        for (int i = 0; i < info->num_vars; i++) {
            if (fabs(info->coef_rest[r][i]) > EPSILON) {
                char num_buffer[32];
                formatear_numero(info->coef_rest[r][i], num_buffer, sizeof(num_buffer));
                
                if (!primer_coef && info->coef_rest[r][i] >= 0) {
                    g_string_append(latex, " + ");
                } else if (!primer_coef && info->coef_rest[r][i] < 0) {
                    g_string_append(latex, " - ");
                } else if (primer_coef && info->coef_rest[r][i] < 0) {
                    g_string_append(latex, "-");
                }
                
                char var_latex[64];
                formatear_nombre_variable_latex(info->nombres_vars[i], var_latex, sizeof(var_latex));
                
                double coef_abs = fabs(info->coef_rest[r][i]);
                if (fabs(coef_abs - 1.0) > EPSILON) {
                    g_string_append_printf(latex, "%s%s", num_buffer, var_latex);
                } else {
                    g_string_append_printf(latex, "%s", var_latex);
                }
                primer_coef = 0;
            }
        }
        
        if (primer_coef) {
            g_string_append(latex, "0");
        }
        
        char rhs_buffer[32];
        formatear_numero(info->lados_derechos[r], rhs_buffer, sizeof(rhs_buffer));
        
        // CORRECCIÓN: Usar el símbolo correcto según el tipo de restricción
        const char* simbolo;
        switch (info->tipos_restricciones[r]) {
            case RESTRICCION_LE:
                simbolo = "\\leq";
                break;
            case RESTRICCION_GE:
                simbolo = "\\geq";
                break;
            case RESTRICCION_EQ:
                simbolo = "=";
                break;
            default:
                simbolo = "\\leq"; // Por defecto
                break;
        }
        
        g_string_append_printf(latex, " %s %s", simbolo, rhs_buffer);
        
        if (r < info->num_rest - 1) {
            g_string_append(latex, " \\\\\n");
        }
    }

    g_string_append(latex, "\n\\end{cases}\n");
    g_string_append(latex, "\\]\n\n");
    g_string_append(latex, "\\subsection{Restricciones de No Negatividad}\n");
    g_string_append(latex, "\\[\n");
    for (int i = 0; i < info->num_vars; i++) {
        char var_latex[64];
        formatear_nombre_variable_latex(info->nombres_vars[i], var_latex, sizeof(var_latex));
        g_string_append_printf(latex, "%s \\geq 0", var_latex);
        if (i < info->num_vars - 1) {
            g_string_append(latex, ", \\quad ");
        }
    }
    g_string_append(latex, "\n\\]\n\n");
}

// Función para generar una tabla simplex en LaTeX
void generar_tabla_latex(GString *latex, TablaSimplex *tabla, const char *titulo, int iteracion, gboolean es_final) {
    if (!tabla || !tabla->tabla) return;
    g_string_append_printf(latex, "\\subsection{%s}\n\n", titulo);
    
    if (iteracion >= 0) {
        g_string_append_printf(latex, "\\textbf{Iteración:} %d\\\\\n", iteracion);
    }
    g_string_append(latex, "\\textbf{Variables básicas:} ");
    for (int i = 0; i < tabla->num_restricciones; i++) {
        int var_base = tabla->variables_base[i];
        char var_latex[64];
        formatear_nombre_variable_latex(tabla->nombres_vars[var_base], var_latex, sizeof(var_latex));
        g_string_append_printf(latex, "$%s$", var_latex);
        if (i < tabla->num_restricciones - 1) {
            g_string_append(latex, ", ");
        }
    }
    g_string_append(latex, "\\\\\n\n");
    g_string_append(latex, "\\begin{center}\n");
    g_string_append(latex, "\\small\n");
    g_string_append(latex, "\\begin{tabular}{|c|");
    
    for (int j = 0; j < tabla->columnas - 1; j++) {
        g_string_append(latex, "c|");
    }
    g_string_append(latex, "c|}\n");
    g_string_append(latex, "\\hline\n");
    g_string_append(latex, "\\textbf{Base} & ");
    for (int j = 0; j < tabla->columnas - 1; j++) {
        char var_escape[64];
        formatear_nombre_variable_tabla(tabla->nombres_vars[j], var_escape, sizeof(var_escape));
        g_string_append_printf(latex, "\\textbf{%s}", var_escape);
        
        if (j < tabla->columnas - 2) {
            g_string_append(latex, " & ");
        }
    }
    g_string_append(latex, " & \\textbf{b} \\\\\n");
    g_string_append(latex, "\\hline\n");
    g_string_append(latex, "Z & ");
    for (int j = 0; j < tabla->columnas; j++) {
        double valor = tabla->tabla[0][j];
        char num_buffer[32];
        
        if (fabs(valor) > M_GRANDE/10) {
            double coef_m = valor / M_GRANDE;
            if (fabs(coef_m - round(coef_m)) < EPSILON) {
                if (fabs(coef_m - 1.0) < EPSILON) {
                    g_string_append_printf(latex, "M");
                } else if (fabs(coef_m + 1.0) < EPSILON) {
                    g_string_append_printf(latex, "-M");
                } else {
                    g_string_append_printf(latex, "%.0fM", coef_m);
                }
            } else {
                g_string_append_printf(latex, "%.1fM", coef_m);
            }
        } else if (fabs(valor) < EPSILON) {
            g_string_append(latex, "0");
        } else {
            formatear_numero(valor, num_buffer, sizeof(num_buffer));
            g_string_append_printf(latex, "%s", num_buffer);
        }
        
        if (j < tabla->columnas - 1) {
            g_string_append(latex, " & ");
        }
    }
    g_string_append(latex, " \\\\\n");
    g_string_append(latex, "\\hline\n");
    
    for (int i = 1; i < tabla->filas; i++) {
        int var_base = tabla->variables_base[i - 1];
        char var_base_escape[64];
        formatear_nombre_variable_tabla(tabla->nombres_vars[var_base], var_base_escape, sizeof(var_base_escape));
        g_string_append_printf(latex, "%s & ", var_base_escape);
        
        for (int j = 0; j < tabla->columnas; j++) {
            double valor = tabla->tabla[i][j];
            char num_buffer[32];
            
            if (fabs(valor) < EPSILON) {
                g_string_append(latex, "0");
            } else {
                formatear_numero(valor, num_buffer, sizeof(num_buffer));
                g_string_append_printf(latex, "%s", num_buffer);
            }
            
            if (j < tabla->columnas - 1) {
                g_string_append(latex, " & ");
            }
        }
        g_string_append(latex, " \\\\\n");
        g_string_append(latex, "\\hline\n");
    }
    
    g_string_append(latex, "\\end{tabular}\n");
    g_string_append(latex, "\\end{center}\n\n");
    
    if (es_final) {
        double *solucion = g_new0(double, tabla->num_vars_decision);
        extraer_solucion(tabla, solucion);
        g_string_append(latex, "\\textbf{Solución:}\n");
        g_string_append(latex, "\\begin{align*}\n");
        for (int i = 0; i < tabla->num_vars_decision; i++) {
            char var_latex[64];
            formatear_nombre_variable_latex(tabla->nombres_vars[i], var_latex, sizeof(var_latex));
            char num_buffer[32];
            formatear_numero(solucion[i], num_buffer, sizeof(num_buffer));
            g_string_append_printf(latex, "%s &= %s", var_latex, num_buffer);
            if (i < tabla->num_vars_decision - 1) {
                g_string_append(latex, " \\\\\n");
            }
        }
        g_string_append(latex, "\n\\end{align*}\n\n");
        
        g_free(solucion);
    }
}

// Función para generar la tabla inicial
void generar_tabla_inicial_latex(GString *latex, TablaSimplex *tabla, ProblemaInfo *info) {
    generar_tabla_latex(latex, tabla, "Tabla Inicial del Método Simplex", -1, FALSE);
    if (tabla->num_vars_artificiales > 0) {
        g_string_append(latex, "\\textbf{Nota:} Se utilizó el método de la Gran M con ");
        g_string_append_printf(latex, "$M = %.0f$\\\\\n", M_GRANDE);
        g_string_append(latex, "\\textbf{Variables artificiales:} ");
        
        int primera = 1;
        for (int i = tabla->num_vars_decision + tabla->num_vars_holgura + tabla->num_vars_exceso;
             i < tabla->num_vars_decision + tabla->num_vars_holgura + 
                 tabla->num_vars_exceso + tabla->num_vars_artificiales; i++) {
            if (!primera) g_string_append(latex, ", ");
            char var_latex[64];
            formatear_nombre_variable_latex(tabla->nombres_vars[i], var_latex, sizeof(var_latex));
            g_string_append_printf(latex, "$%s$", var_latex);
            primera = 0;
        }
        g_string_append(latex, "\\\\\n\n");
    }
}

// Función para generar tablas intermedias
void generar_tablas_intermedias_latex(GString *latex, TablaSimplex **tablas, int num_tablas, ProblemaInfo *info, ResultadoSimplex *resultado) {
    if (!tablas || num_tablas <= 1) return;
    g_string_append(latex, "\\section{Iteraciones del Método Simplex}\n\n");
    for (int i = 1; i < num_tablas - 1; i++) {
        generar_tabla_latex(latex, tablas[i], "Tabla Intermedia", i, FALSE);
    }
}

// Función para generar la tabla final
void generar_tabla_final_latex(GString *latex, ResultadoSimplex *resultado, ProblemaInfo *info) {
    if (!resultado || !resultado->tablas_intermedias || resultado->num_tablas == 0) return;
    TablaSimplex *tabla_final = resultado->tablas_intermedias[resultado->num_tablas - 1];
    generar_tabla_latex(latex, tabla_final, "Tabla Final - Solución Óptima", -1, TRUE);
    switch (resultado->tipo_solucion) {
        case SOLUCION_OPTIMA:
            if (resultado->es_degenerado) {
                g_string_append(latex, "\\textbf{Problema Degenerado:} Al menos una variable básica tiene valor cero.\\\\\n");
            }
            break;
            
        case SOLUCION_MULTIPLE:
            g_string_append(latex, "\\textbf{Solución Múltiple:} Existen infinitas soluciones óptimas.\\\\\n");
            break;
            
        case SOLUCION_NO_ACOTADA:
            g_string_append(latex, "\\textbf{Problema No Acotado:} La función objetivo puede mejorar indefinidamente.\\\\\n");
            break;
            
        case SOLUCION_NO_FACTIBLE:
            g_string_append(latex, "\\textbf{Problema No Factible:} No existe solución que satisfaga todas las restricciones.\\\\\n");
            break;
    }
    
    g_string_append(latex, "\n");
}

// Función para generar solución múltiple
void generar_solucion_multiple_latex(GString *latex, ResultadoSimplex *resultado, ProblemaInfo *info) {
    if (resultado->tipo_solucion != SOLUCION_MULTIPLE) return;
    
    g_string_append(latex, "\\section{Solución Múltiple}\n\n");
    g_string_append(latex, "El problema tiene infinitas soluciones óptimas.\\\\\n");
    
    if (resultado->segunda_tabla) {
        g_string_append(latex, "\\subsection{Segunda Tabla Óptima}\n\n");
        generar_tabla_latex(latex, resultado->segunda_tabla, "Segunda Solución Básica Óptima", -1, TRUE);
    }
    
    g_string_append(latex, "\\subsection{Explicación de Soluciones Múltiples}\n\n");
    g_string_append(latex, "Cuando un problema de programación lineal tiene soluciones múltiples, significa que existe más de una combinación de valores para las variables de decisión que produce el mismo valor óptimo de la función objetivo.\n\n");
    g_string_append(latex, "\\textbf{Condición para soluciones múltiples:}\n");
    g_string_append(latex, "\\begin{itemize}\n");
    g_string_append(latex, "\\item Al menos una variable no básica tiene coeficiente cero en la fila Z de la tabla óptima\n");
    g_string_append(latex, "\\item Esto indica que podemos introducir esa variable en la base sin cambiar el valor de Z\n");
    g_string_append(latex, "\\item El conjunto de soluciones óptimas forma un segmento de recta (en 2D) o un hiperplano (en nD)\n");
    g_string_append(latex, "\\end{itemize}\n\n");
    
    if (resultado->soluciones_adicionales && resultado->num_soluciones_adicionales > 0) {
        g_string_append(latex, "\\subsection{Soluciones Adicionales}\n\n");
        g_string_append(latex, "A continuación se presentan soluciones adicionales obtenidas como combinaciones convexas:\n\n");
        
        for (int k = 0; k < resultado->num_soluciones_adicionales; k++) {
            g_string_append_printf(latex, "\\subsubsection{Solución %d}\n", k + 1);
            g_string_append(latex, "\\begin{align*}\n");
            
            for (int i = 0; i < info->num_vars; i++) {
                char sol_buffer[32];
                formatear_numero(resultado->soluciones_adicionales[k][i], sol_buffer, sizeof(sol_buffer));
                char var_latex[64];
                formatear_nombre_variable_latex(info->nombres_vars[i], var_latex, sizeof(var_latex));
                
                g_string_append_printf(latex, "%s &= %s", var_latex, sol_buffer);
                if (i < info->num_vars - 1) {
                    g_string_append(latex, " \\\\\n");
                }
            }
            g_string_append(latex, "\n\\end{align*}\n\n");
            double z_adicional = 0.0;
            for (int i = 0; i < info->num_vars; i++) {
                z_adicional += info->coef_obj[i] * resultado->soluciones_adicionales[k][i];
            }
            char z_buffer[32];
            formatear_numero(z_adicional, z_buffer, sizeof(z_buffer));
            g_string_append_printf(latex, "\\textbf{Valor de Z:} $%s$ (mismo valor óptimo)\\\\\n\n", z_buffer);
        }
    }
}


// Función para generar explicación de problemas especiales
static void generar_explicacion_problemas_especiales_latex(GString *latex, ResultadoSimplex *resultado) {
    switch (resultado->tipo_solucion) {
        case SOLUCION_NO_ACOTADA:
            g_string_append(latex, "\\subsection{Explicación del Problema No Acotado}\n\n");
            g_string_append(latex, "Un problema de programación lineal se considera \\textbf{no acotado} cuando la función objetivo puede mejorar indefinidamente sin violar ninguna restricción.\n\n");
            g_string_append(latex, "\\textbf{Condiciones para no acotamiento:}\n");
            g_string_append(latex, "\\begin{itemize}\n");
            g_string_append(latex, "\\item Existe al menos una variable que puede aumentar indefinidamente\n");
            g_string_append(latex, "\\item Todos los coeficientes en la columna pivote son negativos o cero\n");
            g_string_append(latex, "\\item No hay restricciones que limiten el crecimiento de la variable\n");
            g_string_append(latex, "\\end{itemize}\n\n");
            break;
            
        case SOLUCION_NO_FACTIBLE:
            g_string_append(latex, "\\subsection{Explicación del Problema No Factible}\n\n");
            g_string_append(latex, "Un problema de programación lineal se considera \\textbf{no factible} cuando no existe ninguna solución que satisfaga todas las restricciones simultáneamente.\n\n");
            g_string_append(latex, "\\textbf{Causas comunes:}\n");
            g_string_append(latex, "\\begin{itemize}\n");
            g_string_append(latex, "\\item Restricciones contradictorias\n");
            g_string_append(latex, "\\item Región factible vacía\n");
            g_string_append(latex, "\\item Variables artificiales permanecen en la base con valor positivo\n");
            g_string_append(latex, "\\end{itemize}\n\n");
            break;
            
        default:
            break;
    }
}

// Función para generar conclusión
void generar_conclusion_latex(GString *latex, ResultadoSimplex *resultado, ProblemaInfo *info) {
    g_string_append(latex, "\\section{Conclusión}\n\n");
    
    switch (resultado->tipo_solucion) {
        case SOLUCION_OPTIMA:
            g_string_append(latex, "El problema tiene una \\textbf{solución óptima única}.\\\\\n");
            g_string_append_printf(latex, "El valor óptimo de la función objetivo es: \\textbf{Z = %.2f}\\\\\n", 
                                  resultado->valor_z);
            break;
            
        case SOLUCION_MULTIPLE:
            g_string_append(latex, "El problema tiene \\textbf{múltiples soluciones óptimas}.\\\\\n");
            g_string_append_printf(latex, "El valor óptimo de la función objetivo es: \\textbf{Z = %.2f}\\\\\n", 
                                  resultado->valor_z);
            g_string_append(latex, "Existen infinitos puntos que alcanzan este valor óptimo.\\\\\n");
            break;
            
        case SOLUCION_NO_ACOTADA:
            g_string_append(latex, "El problema es \\textbf{no acotado}.\\\\\n");
            g_string_append(latex, "La función objetivo puede mejorar indefinidamente sin violar las restricciones.\\\\\n");
            break;
            
        case SOLUCION_NO_FACTIBLE:
            g_string_append(latex, "El problema es \\textbf{no factible}.\\\\\n");
            g_string_append(latex, "No existe ninguna solución que satisfaga todas las restricciones simultáneamente.\\\\\n");
            break;
    }
    
    if (resultado->mensaje) {
        char *mensaje_escape = escape_latex(resultado->mensaje);
        g_string_append_printf(latex, "\\textbf{Mensaje:} %s\\\\\n", mensaje_escape);
        g_free(mensaje_escape);
    }
    
    g_string_append(latex, "\n\\textbf{Recomendaciones:}\n");
    g_string_append(latex, "\\begin{itemize}\n");
    
    switch (resultado->tipo_solucion) {
        case SOLUCION_OPTIMA:
            g_string_append(latex, "\\item La solución encontrada es óptima y puede implementarse directamente.\n");
            if (resultado->es_degenerado) {
                g_string_append(latex, "\\item Aunque el problema es degenerado, la solución sigue siendo válida.\n");
            }
            break;
        case SOLUCION_MULTIPLE:
            g_string_append(latex, "\\item Se pueden elegir diferentes soluciones según criterios adicionales.\n");
            g_string_append(latex, "\\item Considere factores externos para seleccionar la solución más apropiada.\n");
            break;
        case SOLUCION_NO_ACOTADA:
            g_string_append(latex, "\\item Revise la formulación del problema.\n");
            g_string_append(latex, "\\item Posiblemente falten restricciones importantes.\n");
            break;
        case SOLUCION_NO_FACTIBLE:
            g_string_append(latex, "\\item Revise las restricciones del problema.\n");
            g_string_append(latex, "\\item Puede haber conflictos entre las restricciones.\n");
            break;
    }
    
    g_string_append(latex, "\\end{itemize}\n\n");
}

// Función principal para generar el documento LaTeX completo
void generar_documento_latex(ResultadoSimplex *resultado, ProblemaInfo *info, const char *nombre_archivo, gboolean mostrar_tablas) {
    FILE *archivo = fopen(nombre_archivo, "w");
    if (!archivo) {
        g_printerr("Error: No se pudo abrir el archivo %s para escritura\n", nombre_archivo);
        return;
    }
    
    GString *latex = g_string_new("");
    generar_portada_latex(latex, info->nombre_problema);
    generar_algoritmo_simplex_latex(latex);
    generar_problema_original_latex(latex, info);
    g_string_append(latex, "\\section{Método de Solución}\n\n");
    gboolean uso_gran_m = FALSE;
    if (resultado->tablas_intermedias && resultado->num_tablas > 0) {
        TablaSimplex *primera_tabla = resultado->tablas_intermedias[0];
        if (primera_tabla->num_vars_artificiales > 0) {
            uso_gran_m = TRUE;
        }
    }
    
    if (uso_gran_m) {
        g_string_append(latex, "Se utilizó el \\textbf{método de la Gran M} debido a la presencia ");
        g_string_append(latex, "de restricciones de tipo $\\geq$ o $=$.\n\n");
        
        g_string_append(latex, "\\begin{itemize}\n");
        g_string_append_printf(latex, "\\item Valor de M utilizado: $\\mathbf{%.0f}$\n", M_GRANDE);
        g_string_append(latex, "\\item Se introdujeron variables artificiales para las restricciones relevantes\n");
        g_string_append(latex, "\\item El método garantiza encontrar una solución factible si existe\n");
        g_string_append(latex, "\\end{itemize}\n\n");
    } else {
        g_string_append(latex, "Se utilizó el \\textbf{método simplex estándar}.\n\n");
        
        g_string_append(latex, "\\begin{itemize}\n");
        g_string_append(latex, "\\item Todas las restricciones son del tipo $\\leq$\n");
        g_string_append(latex, "\\item Se introdujeron variables de holgura\n");
        g_string_append(latex, "\\item No fue necesario utilizar el método de la Gran M\n");
        g_string_append(latex, "\\end{itemize}\n\n");
    }
    
    if (resultado->tablas_intermedias && resultado->num_tablas > 0) {
        generar_tabla_inicial_latex(latex, resultado->tablas_intermedias[0], info);
    }
    
    if (mostrar_tablas && resultado->tablas_intermedias && resultado->num_tablas > 1) {
        generar_tablas_intermedias_latex(latex, resultado->tablas_intermedias, resultado->num_tablas, info, resultado);
    }
    
    if (resultado->tablas_intermedias && resultado->num_tablas > 0) {
        generar_tabla_final_latex(latex, resultado, info);
    }
    
    g_string_append(latex, "\\section{Resultados}\n\n");
    g_string_append(latex, "\\subsection{Solución Encontrada}\n\n");
    if (resultado->tipo_solucion == SOLUCION_OPTIMA || resultado->tipo_solucion == SOLUCION_MULTIPLE) {
        g_string_append_printf(latex, "\\textbf{Valor óptimo de Z:} $\\mathbf{%.2f}$\\\\\n\n", resultado->valor_z);
        
        g_string_append(latex, "\\textbf{Valores de las variables de decisión:}\\\\\n");
        g_string_append(latex, "\\begin{align*}\n");
        for (int i = 0; i < info->num_vars; i++) {
            char var_latex[64];
            formatear_nombre_variable_latex(info->nombres_vars[i], var_latex, sizeof(var_latex));
            char num_buffer[32];
            formatear_numero(resultado->solucion[i], num_buffer, sizeof(num_buffer));
            g_string_append_printf(latex, "%s &= %s", var_latex, num_buffer);
            if (i < info->num_vars - 1) {
                g_string_append(latex, " \\\\\n");
            }
        }
        g_string_append(latex, "\n\\end{align*}\n\n");
    }
    
    if (resultado->tipo_solucion == SOLUCION_MULTIPLE) {
        generar_solucion_multiple_latex(latex, resultado, info);
    }
    
    generar_explicacion_problemas_especiales_latex(latex, resultado);
    generar_conclusion_latex(latex, resultado, info);
    g_string_append(latex, "\\end{document}\n");
    fwrite(latex->str, 1, latex->len, archivo);
    fclose(archivo);
    
    g_string_free(latex, TRUE);
    
    g_print("Archivo LaTeX generado: %s\n", nombre_archivo);
}

// Función para compilar y mostrar el PDF
void compilar_y_mostrar_pdf(const char *nombre_archivo_tex, const char *nombre_archivo_pdf) {
    char comando[2048];
    system("mkdir -p ProblemasSimplex");

    char comando_mover[512];
    snprintf(comando_mover, sizeof(comando_mover),
             "mv %s ProblemasSimplex/ 2>/dev/null", nombre_archivo_tex);
    system(comando_mover);

    snprintf(comando, sizeof(comando),
             "cd ProblemasSimplex && pdflatex -interaction=nonstopmode %s > /dev/null 2>&1",
             nombre_archivo_tex);

    int result1 = system(comando);
    int result2 = system(comando);

    if (result1 != 0 || result2 != 0) {
        g_printerr("Advertencia: Puede haber errores en la compilación LaTeX\n");
    }

    char nombre_base[256];
    size_t len = strlen(nombre_archivo_tex);

    if (len > 4) {
        size_t base_len = len - 4;
        if (base_len >= sizeof(nombre_base))
            base_len = sizeof(nombre_base) - 1;

        strncpy(nombre_base, nombre_archivo_tex, base_len);
        nombre_base[base_len] = '\0';

        snprintf(comando, sizeof(comando),
                 "cd ProblemasSimplex && rm -f %s.{aux,log,out,toc}",
                 nombre_base);

        system(comando);
    }

    snprintf(comando, sizeof(comando),
             "cd ProblemasSimplex && if [ -f %s ]; then evince --presentation %s > /dev/null 2>&1 & fi",
             nombre_archivo_pdf, nombre_archivo_pdf);
    system(comando);

    g_print("PDF generado y abierto: ProblemasSimplex/%s\n", nombre_archivo_pdf);
}
