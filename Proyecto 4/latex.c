#include "latex.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 


void formatear_numero(double valor, char *buffer, size_t buffer_size) {
    if (fabs(valor - round(valor)) < 0.0001) {
        snprintf(buffer, buffer_size, "%.0f", valor);
    } else {
        snprintf(buffer, buffer_size, "%.2f", valor);
        for (char *p = buffer; *p; p++) {
            if (*p == ',') *p = '.';
        }
    }
}

void formatear_nombre_variable_latex(const char *nombre_original, char *buffer, size_t buffer_size) {
    if (nombre_original && (nombre_original[0] == 'X' || nombre_original[0] == 'x')) {
        int num_var = atoi(nombre_original + 1);
        if (num_var > 0) {
            snprintf(buffer, buffer_size, "x_{%d}", num_var);
        } else {
            snprintf(buffer, buffer_size, "\\text{%s}", nombre_original);
        }
    } else {
        snprintf(buffer, buffer_size, "\\text{%s}", nombre_original);
    }
}

void formatear_nombre_variable_tabla(const char *nombre_original, char *buffer, size_t buffer_size) {
    if (nombre_original && (nombre_original[0] == 'X' || nombre_original[0] == 'x')) {
        int num_var = atoi(nombre_original + 1);
        if (num_var > 0) {
            snprintf(buffer, buffer_size, "$x_{%d}$", num_var);
        } else {
            snprintf(buffer, buffer_size, "\\textbf{%s}", nombre_original);
        }
    } else {
        snprintf(buffer, buffer_size, "\\textbf{%s}", nombre_original);
    }
}

void generar_portada_latex(GString *latex, const char *nombre_problema) {
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
        nombre_problema ? nombre_problema : "Optimización");
}

void generar_algoritmo_simplex_latex(GString *latex) {
    g_string_append_printf(latex,
        "\\section{El Algoritmo Simplex}\n"
        "\n"
        "\\subsection{Historia}\n"
        "El método Simplex fue desarrollado por George Dantzig en 1947 mientras trabajaba para la Fuerza Aérea de los Estados Unidos. \\\\\n"
        "\n"
        "Es uno de los algoritmos más importantes en la historia de la optimización matemática y ha sido fundamental en el desarrollo de la programación lineal.\n"
        "\n"
        "\\subsection{Propiedades Fundamentales}\n"
        "\\begin{itemize}\n"
        "\\item \\textbf{Convergencia:} El algoritmo converge a la solución óptima en un número finito de pasos (en la mayoría de los casos prácticos)\n"
        "\\item \\textbf{Complejidad:} En el peor caso tiene complejidad exponencial, pero en la práctica es muy eficiente\n"
        "\\item \\textbf{Optimalidad:} Garantiza encontrar la solución óptima global para problemas convexos\n"
        "\\item \\textbf{Factibilidad:} Mantiene la factibilidad en cada iteración\n"
        "\\end{itemize}\n"
        "\n"
        "\\subsection{Descripción del Método}\n"
        "El método Simplex opera moviéndose entre vértices adyacentes del poliedro factible, mejorando el valor de la función objetivo en cada paso hasta alcanzar el óptimo.\n"
        "\n");
}

void generar_problema_original_latex(GString *latex, ProblemaInfo *info) {
    g_string_append_printf(latex,
        "\\section{Problema Original}\n"
        "\n"
        "\\subsection{Formulación Matemática}\n");
    
    if (info->tipo_problema && strcmp(info->tipo_problema, "MAX") == 0) {
        g_string_append(latex, "\\textbf{Problema de Maximización}\n\n");
        g_string_append(latex, "\\[ \\text{Maximizar } Z = ");
    } else {
        g_string_append(latex, "\\textbf{Problema de Minimización}\n\n");
        g_string_append(latex, "\\[ \\text{Minimizar } Z = ");
    }
    
    // Función objetivo
    for (int i = 0; i < info->num_vars; i++) {
        char num_buffer[32];
        formatear_numero(info->coef_obj[i], num_buffer, sizeof(num_buffer));
        
        if (i > 0 && info->coef_obj[i] >= 0) {
            g_string_append(latex, "+ ");
        }
        const char *nombre_var = info->nombres_vars[i];
        if (nombre_var && (nombre_var[0] == 'X' || nombre_var[0] == 'x')) {
            int num_var = atoi(nombre_var + 1);
            if (num_var > 0) {
                g_string_append_printf(latex, "%sx_{%d} ", num_buffer, num_var);
            } else {
                g_string_append_printf(latex, "%s%s ", num_buffer, nombre_var);
            }
        } else {
            g_string_append_printf(latex, "%s%s ", num_buffer, nombre_var);
        }
    }
    g_string_append(latex, "\\]\n\n");
    
    // Restricciones
    g_string_append(latex, "\\textbf{Sujeto a:}\n\\begin{align*}\n");
    for (int i = 0; i < info->num_rest; i++) {
        for (int j = 0; j < info->num_vars; j++) {
            char num_buffer[32];
            formatear_numero(info->coef_rest[i][j], num_buffer, sizeof(num_buffer));
            
            if (j > 0 && info->coef_rest[i][j] >= 0) {
                g_string_append(latex, "+ ");
            }
            const char *nombre_var = info->nombres_vars[j];
            if (nombre_var && (nombre_var[0] == 'X' || nombre_var[0] == 'x')) {
                int num_var = atoi(nombre_var + 1);
                if (num_var > 0) {
                    g_string_append_printf(latex, "%sx_{%d} ", num_buffer, num_var);
                } else {
                    g_string_append_printf(latex, "%s%s ", num_buffer, nombre_var);
                }
            } else {
                g_string_append_printf(latex, "%s%s ", num_buffer, nombre_var);
            }
        }
        
        char rhs_buffer[32];
        formatear_numero(info->lados_derechos[i], rhs_buffer, sizeof(rhs_buffer));
        g_string_append_printf(latex, "&\\leq %s", rhs_buffer);
        
        if (i < info->num_rest - 1) {
            g_string_append(latex, " \\\\\n");
        }
    }
    g_string_append(latex, "\n\\end{align*}\n\n");
    g_string_append(latex, "\\textbf{Con:}\n\\[ ");
    for (int i = 0; i < info->num_vars; i++) {
        const char *nombre_var = info->nombres_vars[i];
        if (nombre_var && (nombre_var[0] == 'X' || nombre_var[0] == 'x')) {
            int num_var = atoi(nombre_var + 1);
            if (num_var > 0) {
                g_string_append_printf(latex, "x_{%d} \\geq 0", num_var);
            } else {
                g_string_append_printf(latex, "%s \\geq 0", nombre_var);
            }
        } else {
            g_string_append_printf(latex, "%s \\geq 0", nombre_var);
        }
        
        if (i < info->num_vars - 1) {
            g_string_append(latex, ", \\quad ");
        }
    }
    g_string_append(latex, " \\]\n\n");
}

void generar_tabla_inicial_latex(GString *latex, TablaSimplex *tabla, ProblemaInfo *info) {
    g_string_append_printf(latex,
        "\\section{Tabla Inicial del Método Simplex}\n"
        "\n"
        "La tabla inicial del método Simplex se construye agregando variables de holgura para convertir las desigualdades en igualdades.\n\n"
        "\\begin{center}\n"
        "\\small\n"
        "\\begin{tabular}{|c|");
    
    // Encabezados de columnas
    g_string_append(latex, "c|"); // Z
    for (int j = 0; j < tabla->num_vars; j++) {
        g_string_append(latex, "c|");
    }
    for (int j = 0; j < tabla->num_rest; j++) {
        g_string_append(latex, "c|");
    }
    g_string_append(latex, "c|}\n\\hline\n");
    
    // Nombres de columnas
    g_string_append(latex, "\\textbf{Variable} & \\textbf{Z} & ");
    for (int j = 0; j < tabla->num_vars; j++) {
        const char *nombre_var = info->nombres_vars[j];
        char var_latex[32];
        formatear_nombre_variable_tabla(nombre_var, var_latex, sizeof(var_latex));
        g_string_append_printf(latex, "\\textbf{%s} & ", var_latex);
    }
    for (int j = 0; j < tabla->num_rest; j++) {
        g_string_append_printf(latex, "\\textbf{$S_%d$} & ", j + 1);
    }
    g_string_append(latex, "\\textbf{b} \\\\\n\\hline\n");
    g_string_append(latex, "\\textbf{Z} & ");
    g_string_append(latex, "1 & "); // Coeficiente de Z 
    
    for (int j = 0; j < tabla->num_vars; j++) {
        char num_buffer[32];
        double valor = tabla->tabla[0][j];
        if (fabs(valor) < 0.0001) valor = 0.0;
        formatear_numero(valor, num_buffer, sizeof(num_buffer));
        g_string_append_printf(latex, "%s & ", num_buffer);
    }
    
    for (int j = 0; j < tabla->num_rest; j++) {
        char num_buffer[32];
        double valor = tabla->tabla[0][tabla->num_vars + j];
        if (fabs(valor) < 0.0001) valor = 0.0;
        formatear_numero(valor, num_buffer, sizeof(num_buffer));
        g_string_append_printf(latex, "%s & ", num_buffer);
    }
    
    char ld_buffer[32];
    formatear_numero(tabla->tabla[0][tabla->num_vars + tabla->num_rest], ld_buffer, sizeof(ld_buffer));
    g_string_append_printf(latex, "%s \\\\\n\\hline\n", ld_buffer);
    
    // Filas de restricciones
    for (int i = 1; i <= tabla->num_rest; i++) {
        g_string_append_printf(latex, "\\textbf{$S_%d$} & 0 & ", i);
        
        for (int j = 0; j < tabla->num_vars; j++) {
            char num_buffer[32];
            formatear_numero(tabla->tabla[i][j], num_buffer, sizeof(num_buffer));
            g_string_append_printf(latex, "%s & ", num_buffer);
        }
        
        for (int j = 0; j < tabla->num_rest; j++) {
            if (j == i - 1) {
                g_string_append(latex, "1 & "); // Variable de holgura correspondiente
            } else {
                g_string_append(latex, "0 & ");
            }
        }
        
        char ld_buffer[32];
        formatear_numero(tabla->tabla[i][tabla->num_vars + tabla->num_rest], ld_buffer, sizeof(ld_buffer));
        g_string_append_printf(latex, "%s \\\\\n\\hline\n", ld_buffer);
    }
    
    g_string_append(latex, "\\end{tabular}\n");
    g_string_append(latex, "\\end{center}\n\n");
    
    g_string_append_printf(latex,
        "\\textbf{Explicación de la tabla inicial:}\n\n"
        "\\begin{itemize}\n"
        "\\item \\textbf{Variable:} Indica las variables básicas actuales\n"
        "\\item \\textbf{Z:} Coeficiente de la función objetivo (siempre 1)\n");
    
    for (int j = 0; j < tabla->num_vars; j++) {
        const char *nombre_var = info->nombres_vars[j];
        char var_latex[32];
        formatear_nombre_variable_latex(nombre_var, var_latex, sizeof(var_latex));
        g_string_append_printf(latex, "\\item \\textbf{%s:} Coeficientes de las variables de decisi\\'on\n", var_latex);
    }
    
    for (int j = 0; j < tabla->num_rest; j++) {
        g_string_append_printf(latex, "\\item \\textbf{$S_%d$:} Coeficientes de las variables de holgura\n", j + 1);
    }
    
    g_string_append_printf(latex,
        "\\item \\textbf{b:} T\\'erminos independientes (lado derecho)\n"
        "\\item \\textbf{Base inicial:} Variables de holgura $S_1, S_2, \\ldots, S_%d$\n"
        "\\end{itemize}\n\n",
        tabla->num_rest);
}

void generar_tablas_intermedias_latex(GString *latex, GList *tablas, ProblemaInfo *info, ResultadoSimplex *resultado) {
    if (!tablas || g_list_length(tablas) <= 1) return;
    
    g_string_append_printf(latex,
        "\\section{Proceso Iterativo del Método Simplex}\n"
        "\n"
        "A continuación se detalla el proceso iterativo del algoritmo Simplex, mostrando cada tabla y las operaciones de pivoteo realizadas.\n\n");
    
    int iteracion = 0;
    GList *iter = tablas;
    
    while (iter) {
        TablaSimplex *tabla = (TablaSimplex*)iter->data;
        
        if (iteracion == 0) {
            iter = g_list_next(iter);
            iteracion++;
            continue;
        }
        
        g_string_append_printf(latex, "\\subsection{Iteración %d}\n", iteracion);
        if (resultado->operaciones_pivoteo && iteracion <= g_list_length(resultado->operaciones_pivoteo)) {
            GList *op_iter = g_list_nth(resultado->operaciones_pivoteo, iteracion - 1);
            if (op_iter) {
                OperacionPivoteo *op = (OperacionPivoteo*)op_iter->data;
                g_string_append_printf(latex, "\\textbf{Operación de pivoteo:}\n");
                g_string_append_printf(latex, "\\begin{itemize}\n");
                
                if (op->variable_entra < tabla->num_vars) {
                    const char *nombre_var = info->nombres_vars[op->variable_entra];
                    char var_latex[32];
                    formatear_nombre_variable_latex(nombre_var, var_latex, sizeof(var_latex));
                    g_string_append_printf(latex, "\\item \\textbf{Variable que entra:} $%s$\n", var_latex);
                } else {
                    g_string_append_printf(latex, "\\item \\textbf{Variable que entra:} $S_%d$\n", 
                                         op->variable_entra - tabla->num_vars + 1);
                }
                
                if (op->variable_sale < tabla->num_vars) {
                    const char *nombre_var = info->nombres_vars[op->variable_sale];
                    char var_latex[32];
                    formatear_nombre_variable_latex(nombre_var, var_latex, sizeof(var_latex));
                    g_string_append_printf(latex, "\\item \\textbf{Variable que sale:} $%s$\n", var_latex);
                } else {
                    g_string_append_printf(latex, "\\item \\textbf{Variable que sale:} $S_%d$\n", 
                                         op->variable_sale - tabla->num_vars + 1);
                }
                
                g_string_append_printf(latex, "\\item \\textbf{Elemento pivote:} %.4f (fila %d, columna %d)\n", 
                                     op->elemento_pivote, op->fila_pivote + 1, op->columna_pivote + 1);
                g_string_append_printf(latex, "\\end{itemize}\n\n");
            }
        }
        
        g_string_append_printf(latex,
            "\\begin{center}\n"
            "\\small\n"
            "\\begin{tabular}{|c|c|");
        
        // Encabezados
        for (int j = 0; j < tabla->num_vars; j++) {
            g_string_append(latex, "c|");
        }
        for (int j = 0; j < tabla->num_rest; j++) {
            g_string_append(latex, "c|");
        }
        g_string_append(latex, "c|}\n\\hline\n");
        
        g_string_append(latex, "\\textbf{Variable} & \\textbf{Z} & ");
        for (int j = 0; j < tabla->num_vars; j++) {
            const char *nombre_var = info->nombres_vars[j];
            char var_latex[32];
            formatear_nombre_variable_tabla(nombre_var, var_latex, sizeof(var_latex));
            g_string_append_printf(latex, "\\textbf{%s} & ", var_latex);
        }
        for (int j = 0; j < tabla->num_rest; j++) {
            g_string_append_printf(latex, "\\textbf{$S_%d$} & ", j + 1);
        }
        g_string_append(latex, "\\textbf{b} \\\\\n\\hline\n");
        
        // Fila Z
        g_string_append(latex, "\\textbf{Z} & 1 & ");
        for (int j = 0; j < tabla->num_vars; j++) {
            char num_buffer[32];
            double valor = tabla->tabla[0][j];
            if (fabs(valor) < 0.0001) valor = 0.0;
            formatear_numero(valor, num_buffer, sizeof(num_buffer));
            
            if ((tabla->tipo == MAXIMIZACION && tabla->tabla[0][j] < -EPSILON) ||
                (tabla->tipo == MINIMIZACION && tabla->tabla[0][j] > EPSILON)) {
                g_string_append_printf(latex, "\\textcolor{red}{%s} & ", num_buffer);
            } else {
                g_string_append_printf(latex, "%s & ", num_buffer);
            }
        }
        
        for (int j = 0; j < tabla->num_rest; j++) {
            char num_buffer[32];
            formatear_numero(tabla->tabla[0][tabla->num_vars + j], num_buffer, sizeof(num_buffer));
            g_string_append_printf(latex, "%s & ", num_buffer);
        }
        
        char ld_buffer[32];
        formatear_numero(tabla->tabla[0][tabla->num_vars + tabla->num_rest], ld_buffer, sizeof(ld_buffer));
        g_string_append_printf(latex, "%s \\\\\n\\hline\n", ld_buffer);
        
        // Filas de restricciones
        for (int i = 1; i <= tabla->num_rest; i++) {
            const char *var_name = "S";
            int var_index = tabla->variables_basicas[i-1];
            if (var_index < tabla->num_vars) {
                var_name = info->nombres_vars[var_index];
                char var_latex[32];
                formatear_nombre_variable_latex(var_name, var_latex, sizeof(var_latex));
                g_string_append_printf(latex, "\\textbf{\\textcolor{green}{$%s$}} & 0 & ", var_latex);
            } else {
                g_string_append_printf(latex, "\\textbf{\\textcolor{green}{$S_%d$}} & 0 & ", var_index - tabla->num_vars + 1);
            }
            
            for (int j = 0; j < tabla->num_vars + tabla->num_rest; j++) {
                char num_buffer[32];
                formatear_numero(tabla->tabla[i][j], num_buffer, sizeof(num_buffer));
                
                if (resultado->operaciones_pivoteo && iteracion <= g_list_length(resultado->operaciones_pivoteo)) {
                    GList *op_iter = g_list_nth(resultado->operaciones_pivoteo, iteracion - 1);
                    if (op_iter) {
                        OperacionPivoteo *op = (OperacionPivoteo*)op_iter->data;
                        if (i-1 == op->fila_pivote && j == op->columna_pivote) {
                            g_string_append_printf(latex, "\\mathbf{[%s]} & ", num_buffer);
                            continue;
                        }
                    }
                }
                g_string_append_printf(latex, "%s & ", num_buffer);
            }
            
            char ld_buffer[32];
            formatear_numero(tabla->tabla[i][tabla->num_vars + tabla->num_rest], ld_buffer, sizeof(ld_buffer));
            g_string_append_printf(latex, "%s \\\\\n\\hline\n", ld_buffer);
        }
        
        g_string_append(latex, "\\end{tabular}\n");
        g_string_append(latex, "\\end{center}\n\n");
        g_string_append(latex, "\\textbf{Variables en la base:} ");
        for (int i = 0; i < tabla->num_rest; i++) {
            int var_index = tabla->variables_basicas[i];
            if (var_index < tabla->num_vars) {
                const char *nombre_var = info->nombres_vars[var_index];
                char var_latex[32];
                formatear_nombre_variable_latex(nombre_var, var_latex, sizeof(var_latex));
                g_string_append_printf(latex, "\\textcolor{green}{$%s$}", var_latex);
            } else {
                g_string_append_printf(latex, "\\textcolor{green}{$S_%d$}", var_index - tabla->num_vars + 1);
            }
            if (i < tabla->num_rest - 1) {
                g_string_append(latex, ", ");
            }
        }
        g_string_append(latex, "\\\\\n\n");
        iter = g_list_next(iter);
        iteracion++;
    }
}

void generar_tabla_final_latex(GString *latex, ResultadoSimplex *resultado, ProblemaInfo *info) {
    g_string_append_printf(latex,
        "\\section{Tabla Final}\n"
        "\n");
    if (!resultado->tabla_final) {
        g_string_append(latex, "\\textbf{Error:} No se pudo generar la tabla final.\n\n");
        return;
    }
    
    if (resultado->tipo_solucion == SOLUCION_MULTIPLE) {
        g_string_append(latex, "\\textbf{Soluciones Múltiples:} Se encontraron dos tablas finales que representan diferentes soluciones óptimas.\n\n");
        
        g_string_append(latex, "\\subsection{Primera Solución Óptima}\n");
    } else {
        g_string_append(latex, "La siguiente tabla representa la solución óptima del problema:\n\n");
    }
    
    // Primera tabla final
    TablaSimplex *tabla = resultado->tabla_final;
    
    g_string_append(latex, "\\begin{center}\n");
    g_string_append(latex, "\\small\n");
    g_string_append(latex, "\\begin{tabular}{|c|");
    
    for (int j = 0; j < tabla->num_vars; j++) {
        g_string_append(latex, "c|");
    }
    for (int j = 0; j < tabla->num_rest; j++) {
        g_string_append(latex, "c|");
    }
    g_string_append(latex, "c|}\n\\hline\n");
    
    g_string_append(latex, " & ");
    for (int j = 0; j < tabla->num_vars; j++) {
        const char *nombre_var = info->nombres_vars[j];
        char var_latex[32];
        formatear_nombre_variable_tabla(nombre_var, var_latex, sizeof(var_latex));
        g_string_append_printf(latex, "\\textbf{%s} & ", var_latex);
    }
    for (int j = 0; j < tabla->num_rest; j++) {
        g_string_append_printf(latex, "\\textbf{$s_%d$} & ", j + 1);
    }
    g_string_append(latex, "\\textbf{L.D.} \\\\\n\\hline\n");
    
    g_string_append(latex, "\\textbf{Z} & ");
    for (int j = 0; j < tabla->num_vars + tabla->num_rest; j++) {
        char num_buffer[32];
        formatear_numero(tabla->tabla[0][j], num_buffer, sizeof(num_buffer));
        g_string_append_printf(latex, "%s & ", num_buffer);
    }
    
    char ld_buffer[32];
    formatear_numero(tabla->tabla[0][tabla->num_vars + tabla->num_rest], ld_buffer, sizeof(ld_buffer));
    g_string_append_printf(latex, "%s \\\\\n\\hline\n", ld_buffer);
    
    for (int i = 1; i <= tabla->num_rest; i++) {
        const char *var_name = "s";
        int var_index = tabla->variables_basicas[i-1];
        if (var_index < tabla->num_vars) {
            var_name = info->nombres_vars[var_index];
            char var_latex[32];
            formatear_nombre_variable_latex(var_name, var_latex, sizeof(var_latex));
            g_string_append_printf(latex, "\\textbf{$%s$} & ", var_latex);
        } else {
            g_string_append_printf(latex, "\\textbf{$s_%d$} & ", var_index - tabla->num_vars + 1);
        }
        
        for (int j = 0; j < tabla->num_vars + tabla->num_rest; j++) {
            char num_buffer[32];
            formatear_numero(tabla->tabla[i][j], num_buffer, sizeof(num_buffer));
            g_string_append_printf(latex, "%s & ", num_buffer);
        }
        
        char ld_buffer[32];
        formatear_numero(tabla->tabla[i][tabla->num_vars + tabla->num_rest], ld_buffer, sizeof(ld_buffer));
        g_string_append_printf(latex, "%s \\\\\n\\hline\n", ld_buffer);
    }
    
    g_string_append(latex, "\\end{tabular}\n");
    g_string_append(latex, "\\end{center}\n\n");
    if (resultado->tipo_solucion == SOLUCION_MULTIPLE && resultado->segunda_tabla) {
        g_string_append(latex, "\\subsection{Segunda Solución Óptima}\n");
        
        tabla = resultado->segunda_tabla;
        
        g_string_append(latex, "\\begin{center}\n");
        g_string_append(latex, "\\small\n");
        g_string_append(latex, "\\begin{tabular}{|c|");
        
        for (int j = 0; j < tabla->num_vars; j++) {
            g_string_append(latex, "c|");
        }
        for (int j = 0; j < tabla->num_rest; j++) {
            g_string_append(latex, "c|");
        }
        g_string_append(latex, "c|}\n\\hline\n");
        
        g_string_append(latex, " & ");
        for (int j = 0; j < tabla->num_vars; j++) {
            g_string_append_printf(latex, "\\textbf{%s} & ", info->nombres_vars[j]);
        }
        for (int j = 0; j < tabla->num_rest; j++) {
            g_string_append_printf(latex, "\\textbf{$s_%d$} & ", j + 1);
        }
        g_string_append(latex, "\\textbf{L.D.} \\\\\n\\hline\n");
        
        g_string_append(latex, "\\textbf{Z} & ");
        for (int j = 0; j < tabla->num_vars + tabla->num_rest; j++) {
            char num_buffer[32];
            formatear_numero(tabla->tabla[0][j], num_buffer, sizeof(num_buffer));
            g_string_append_printf(latex, "%s & ", num_buffer);
        }
        
        char ld_buffer[32];
        formatear_numero(tabla->tabla[0][tabla->num_vars + tabla->num_rest], ld_buffer, sizeof(ld_buffer));
        g_string_append_printf(latex, "%s \\\\\n\\hline\n", ld_buffer);
        
        for (int i = 1; i <= tabla->num_rest; i++) {
            const char *var_name = "s";
            int var_index = tabla->variables_basicas[i-1];
            if (var_index < tabla->num_vars) {
                var_name = info->nombres_vars[var_index];
            } else {
                var_name = "s";
            }
            
            g_string_append_printf(latex, "\\textbf{$%s$} & ", var_name);
            for (int j = 0; j < tabla->num_vars + tabla->num_rest; j++) {
                char num_buffer[32];
                formatear_numero(tabla->tabla[i][j], num_buffer, sizeof(num_buffer));
                g_string_append_printf(latex, "%s & ", num_buffer);
            }
            
            char ld_buffer[32];
            formatear_numero(tabla->tabla[i][tabla->num_vars + tabla->num_rest], ld_buffer, sizeof(ld_buffer));
            g_string_append_printf(latex, "%s \\\\\n\\hline\n", ld_buffer);
        }
        
        g_string_append(latex, "\\end{tabular}\n");
        g_string_append(latex, "\\end{center}\n\n");
    }
}

void generar_explicacion_soluciones_multiples_latex(GString *latex, ResultadoSimplex *resultado, ProblemaInfo *info) {
    if (resultado->tipo_solucion != SOLUCION_MULTIPLE) return;
    
    g_string_append_printf(latex,
        "\\subsection{Explicación de Soluciones Múltiples}\n"
        "\n"
        "Cuando un problema de programación lineal tiene soluciones múltiples, significa que existe más de una combinación de valores para las variables de decisión que produce el mismo valor óptimo de la función objetivo.\n\n"
        "\\textbf{Condición para soluciones múltiples:}\n"
        "\\begin{itemize}\n"
        "\\item Al menos una variable no básica tiene coeficiente cero en la fila Z de la tabla óptima\n"
        "\\item Esto indica que podemos introducir esa variable en la base sin cambiar el valor de Z\n"
        "\\item El conjunto de soluciones óptimas forma un segmento de recta (en 2D) o un hiperplano (en nD)\n"
        "\\end{itemize}\n\n"
        "\\textbf{Ecuación para generar todas las soluciones óptimas:}\n"
        "\\[\n"
        "X = \\lambda X_1 + (1-\\lambda) X_2, \\quad 0 \\leq \\lambda \\leq 1\n"
        "\\]\n"
        "Donde $X_1$ y $X_2$ son las dos soluciones básicas encontradas y $\\lambda$ es un parámetro entre 0 y 1.\n\n");
}

void generar_explicacion_no_acotado_latex(GString *latex) {
    g_string_append_printf(latex,
        "\\subsection{Explicación del Problema No Acotado}\n"
        "\n"
        "Un problema de programación lineal se considera \\textbf{no acotado} cuando la función objetivo puede mejorar indefinidamente sin violar ninguna restricción.\n\n"
        "\\textbf{Condiciones para no acotamiento:}\n"
        "\\begin{itemize}\n"
        "\\item Existe al menos una variable que puede aumentar indefinidamente\n"
        "\\item Todos los coeficientes en la columna pivote son negativos o cero\n"
        "\\item No hay restricciones que limiten el crecimiento de la variable\n"
        "\\end{itemize}\n\n"
        "\\textbf{Interpretación práctica:}\n"
        "\\begin{itemize}\n"
        "\\item En problemas de maximización: La ganancia puede ser infinita\n"
        "\\item En problemas de minimización: El costo puede disminuir indefinidamente\n"
        "\\item Suele indicar un error en la formulación del problema\n"
        "\\item Puede significar que faltan restricciones importantes\n"
        "\\end{itemize}\n\n"
        "\\textbf{Solución:} Revisar la formulación del problema y verificar que todas las restricciones necesarias estén incluidas.\n\n");
}

void generar_explicacion_degenerado_latex(GString *latex, ResultadoSimplex *resultado) {
    g_string_append_printf(latex,
        "\\subsection{Explicación del Problema Degenerado}\n"
        "\n"
        "Un problema de programación lineal se considera \\textbf{degenerado} cuando al menos una variable básica toma el valor cero en la solución óptima.\n\n"
        "\\textbf{Características de la degeneración:}\n"
        "\\begin{itemize}\n"
        "\\item Al menos una variable básica tiene valor cero\n"
        "\\item Puede ocurrir cuando hay restricciones redundantes\n"
        "\\item Puede llevar a ciclado en el algoritmo Simplex (aunque es raro en la práctica)\n"
        "\\item La solución óptima puede no ser única\n"
        "\\item En problemas prácticos, la degeneración es común pero generalmente no afecta la calidad de la solución\n"
        "\\end{itemize}\n\n"
        "\\textbf{Causas comunes:}\n"
        "\\begin{itemize}\n"
        "\\item Restricciones redundantes en el modelo\n"
        "\\item Múltiples restricciones que se intersectan en el mismo punto\n"
        "\\item Problemas con estructura especial que genera empates en la selección de variables\n"
        "\\end{itemize}\n\n"
        "\\textbf{Manejo en el algoritmo Simplex:}\n"
        "\\begin{itemize}\n"
        "\\item Se utiliza una tolerancia numérica ($\\\\epsilon = 10^{-10}$) para detectar valores cero\n"
        "\\item Cuando hay empates en la regla del ratio mínimo, se elige arbitrariamente\n"
        "\\item La elección arbitraria evita el ciclado en la mayoría de los casos prácticos\n"
        "\\item En este problema se realizaron %d iteraciones sin ciclado\n"
        "\\end{itemize}\n\n"
        "\\textbf{Implicaciones prácticas:}\n"
        "\\begin{itemize}\n"
        "\\item La solución encontrada es válida y óptima\n"
        "\\item Puede existir más de una solución óptima (soluciones alternativas)\n"
        "\\item En aplicaciones prácticas, la degeneración generalmente no es problemática\n"
        "\\item Si es necesario, se pueden usar técnicas anti-ciclado (regla de Bland)\n"
        "\\end{itemize}\n\n",
        resultado->iteraciones);
}

void generar_explicacion_minimizacion_latex(GString *latex) {
    g_string_append_printf(latex,
        "\\subsection{Nota sobre Problemas de Minimización}\n"
        "\n"
        "Para problemas de \\textbf{minimización}, el valor óptimo de Z se toma directamente de la tabla Simplex final:\n"
        "\\begin{itemize}\n"
        "\\item No se aplica cambio de signo al valor de Z\n"
        "\\item El valor reportado es el valor real de la función objetivo\n"
        "\\item Si el valor es negativo, indica un costo mínimo negativo\n"
        "\\item Esto es matemáticamente correcto y coherente con la formulación\n"
        "\\end{itemize}\n\n"
        "\\textbf{Forma estándar para minimización:}\n"
        "\\[\n"
        "\\text{Minimizar } Z = c^Tx \\quad \\text{sujeto a } Ax \\leq b, x \\geq 0\n"
        "\\]\n"
        "El valor óptimo Z* se lee directamente de la tabla sin modificaciones.\n\n");
}

void generar_soluciones_adicionales_latex(GString *latex, ResultadoSimplex *resultado, ProblemaInfo *info) {
    if (resultado->tipo_solucion != SOLUCION_MULTIPLE || !resultado->soluciones_adicionales) return;
    
    g_string_append_printf(latex,
        "\\subsection{Soluciones Adicionales}\n"
        "\n"
        "A continuación se presentan 3 soluciones adicionales obtenidas como combinaciones convexas de las dos soluciones básicas:\n\n");
    
    for (int k = 0; k < resultado->num_soluciones_adicionales; k++) {
        double lambda = (k + 1) * 0.25;
        
        g_string_append_printf(latex,
            "\\subsubsection{Solución con $\\lambda = %.2f$}\n"
            "\\begin{align*}\n", lambda);
        
        for (int i = 0; i < info->num_vars; i++) {
            char sol_buffer[32];
            formatear_numero(resultado->soluciones_adicionales[k][i], sol_buffer, sizeof(sol_buffer));
            const char *nombre_var = info->nombres_vars[i];
            char var_latex[32];
            formatear_nombre_variable_latex(nombre_var, var_latex, sizeof(var_latex));
            
            g_string_append_printf(latex, "%s &= %s", var_latex, sol_buffer);
            if (i < info->num_vars - 1) {
                g_string_append(latex, " \\\\\n");
            }
        }
        g_string_append(latex, "\n\\end{align*}\n\n");
        
        double z_calculado = 0.0;
        for (int i = 0; i < info->num_vars; i++) {
            z_calculado += info->coef_obj[i] * resultado->soluciones_adicionales[k][i];
        }
        
        char z_buffer[32];
        formatear_numero(z_calculado, z_buffer, sizeof(z_buffer));
        g_string_append_printf(latex, "\\textbf{Verificación: } $Z = %s$ (mismo valor óptimo)\\par\\smallskip\n\n", z_buffer);
    }
}

void generar_solucion_latex(GString *latex, ResultadoSimplex *resultado, ProblemaInfo *info) {
    g_string_append_printf(latex,
        "\\section{Solución Óptima}\n"
        "\n");
    
    char valor_optimo_buffer[32];
    formatear_numero(resultado->valor_optimo, valor_optimo_buffer, sizeof(valor_optimo_buffer));
    g_string_append_printf(latex, "\\textbf{Valor óptimo de la función objetivo: } $Z = %s$\n\n", valor_optimo_buffer);
    g_string_append(latex, "\\textbf{Valores de las variables:}\n\\begin{align*}\n");
    
    for (int i = 0; i < info->num_vars; i++) {
        char sol_buffer[32];
        formatear_numero(resultado->solucion[i], sol_buffer, sizeof(sol_buffer));
        const char *nombre_var = info->nombres_vars[i];
        char var_latex[32];
        formatear_nombre_variable_latex(nombre_var, var_latex, sizeof(var_latex));
        
        g_string_append_printf(latex, "%s &= %s", var_latex, sol_buffer);
        if (i < info->num_vars - 1) {
            g_string_append(latex, " \\\\\n");
        }
    }
    
    if (resultado->tabla_final) {
        TablaSimplex *tabla_final = resultado->tabla_final;
        gboolean *holgura_mostrada = g_new0(gboolean, tabla_final->num_rest);
        
        for (int i = 0; i < tabla_final->num_rest; i++) {
            int var_index = tabla_final->variables_basicas[i];
            if (var_index >= tabla_final->num_vars) {
                int indice_holgura = var_index - tabla_final->num_vars;
                double valor_holgura = tabla_final->tabla[i + 1][tabla_final->num_vars + tabla_final->num_rest];
                
                char sol_buffer[32];
                formatear_numero(valor_holgura, sol_buffer, sizeof(sol_buffer));
                if (fabs(valor_holgura) < EPSILON) {
                    g_string_append_printf(latex, " \\\\\n\\textcolor{red}{S_%d} &= \\textcolor{red}{%s} \\quad \\text{(degenerada)}", 
                                         indice_holgura + 1, sol_buffer);
                } else {
                    g_string_append_printf(latex, " \\\\\nS_%d &= %s", indice_holgura + 1, sol_buffer);
                }
                holgura_mostrada[indice_holgura] = TRUE;
            }
        }
        
        for (int i = 0; i < tabla_final->num_rest; i++) {
            if (!holgura_mostrada[i]) {
                g_string_append_printf(latex, " \\\\\nS_%d &= 0", i + 1);
            }
        }
        
        g_free(holgura_mostrada);
    }
    
    g_string_append(latex, "\n\\end{align*}\n\n");
    
    switch (resultado->tipo_solucion) {
        case SOLUCION_UNICA:
            g_string_append(latex, "\\textbf{Tipo:} Solución Única\n\n");
            g_string_append(latex, "El problema tiene una única solución óptima en el punto encontrado.\n\n");
            break;
            
        case SOLUCION_MULTIPLE:
            g_string_append(latex, "\\textbf{Tipo:} Soluciones Múltiples\n\n");
            generar_explicacion_soluciones_multiples_latex(latex, resultado, info);
            generar_soluciones_adicionales_latex(latex, resultado, info);
            break;
            
        case NO_ACOTADO:
            g_string_append(latex, "\\textbf{Tipo:} Problema No Acotado\n\n");
            generar_explicacion_no_acotado_latex(latex);
            break;
            
        case DEGENERADO:
            g_string_append(latex, "\\textbf{Tipo:} Problema Degenerado\n\n");
            generar_explicacion_degenerado_latex(latex, resultado);
            
            if (resultado->tabla_final) {
                g_string_append(latex, "\\textbf{Variables básicas con valor cero (degeneradas):}\n\\begin{itemize}\n");
                TablaSimplex *tabla = resultado->tabla_final;
                for (int i = 0; i < tabla->num_rest; i++) {
                    double valor = tabla->tabla[i + 1][tabla->num_vars + tabla->num_rest];
                    if (fabs(valor) < EPSILON) {
                        int var_index = tabla->variables_basicas[i];
                        if (var_index < tabla->num_vars) {
                            const char *nombre_var = info->nombres_vars[var_index];
                            char var_latex[32];
                            formatear_nombre_variable_latex(nombre_var, var_latex, sizeof(var_latex));
                            g_string_append_printf(latex, "\\item $%s = 0$\n", var_latex);
                        } else {
                            g_string_append_printf(latex, "\\item $S_%d = 0$\n", var_index - tabla->num_vars + 1);
                        }
                    }
                }
                g_string_append(latex, "\\end{itemize}\n\n");
            }
            break;
            
        case NO_FACTIBLE:
            g_string_append(latex, "\\textbf{Tipo:} Problema No Factible\n\n");
            g_string_append(latex, "El problema no tiene solución factible. El conjunto de restricciones es incompatible.\n\n");
            break;
    }
    
    if (info->tipo_problema && strcmp(info->tipo_problema, "MIN") == 0) {
        generar_explicacion_minimizacion_latex(latex);
    }
    
    g_string_append_printf(latex, "\\textbf{Iteraciones realizadas:} %d\n\n", resultado->iteraciones);
    
    if (resultado->mensaje) {
        g_string_append_printf(latex, "\\textbf{Observaciones:} %s\n\n", resultado->mensaje);
    }
}

void generar_documento_latex(ResultadoSimplex *resultado, ProblemaInfo *info, 
                            const char *nombre_archivo, gboolean mostrar_tablas) {
    GString *latex = g_string_new("");
    
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
        "\\newpage\n"
        "\\tableofcontents\n"
        "\\newpage\n",
        info->nombre_problema ? info->nombre_problema : "Optimización");
    
    // Generar todas las secciones
    generar_algoritmo_simplex_latex(latex);
    generar_problema_original_latex(latex, info);
    
    // Tabla inicial 
    if (resultado->tablas_intermedias) {
        TablaSimplex *tabla_inicial = (TablaSimplex*)g_list_first(resultado->tablas_intermedias)->data;
        generar_tabla_inicial_latex(latex, tabla_inicial, info);
    }
    
    // Tablas intermedias 
    if (mostrar_tablas && resultado->tablas_intermedias) {
        generar_tablas_intermedias_latex(latex, resultado->tablas_intermedias, info, resultado);
    }
    
    // Tabla final
    generar_tabla_final_latex(latex, resultado, info);
    
    // Solución
    generar_solucion_latex(latex, resultado, info);
    
    // Finalizar documento
    g_string_append(latex, "\\end{document}\n");
    
    // Guardar archivo
    FILE *archivo = fopen(nombre_archivo, "w");
    if (archivo) {
        fprintf(archivo, "%s", latex->str);
        fclose(archivo);
        g_print("Archivo LaTeX guardado: %s\n", nombre_archivo);
    } else {
        g_printerr("Error al guardar archivo LaTeX: %s\n", nombre_archivo);
    }
    
    g_string_free(latex, TRUE);
}

void compilar_y_mostrar_pdf(const char *nombre_archivo_tex, const char *nombre_archivo_pdf) {
    char comando1[512];
    char comando2[512];
    char comando3[512];
    system("mkdir -p ProblemasSimplex");
    snprintf(comando3, sizeof(comando3), "mv %s ProblemasSimplex/ 2>/dev/null", nombre_archivo_tex);
    system(comando3);
    snprintf(comando1, sizeof(comando1), "cd ProblemasSimplex && pdflatex -interaction=nonstopmode %s", nombre_archivo_tex);
    int result1 = system(comando1);
    int result2 = system(comando1); 
    
    if (result1 != 0 || result2 != 0) {
        g_printerr("Error al compilar el archivo LaTeX\n");
    }
    
    snprintf(comando2, sizeof(comando2), "cd ProblemasSimplex && if [ -f %s ]; then evince --presentation %s & fi", nombre_archivo_pdf, nombre_archivo_pdf);
    system(comando2);
}