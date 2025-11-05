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
            snprintf(buffer, buffer_size, "%s", nombre_original);
        }
    } else {
        snprintf(buffer, buffer_size, "%s", nombre_original);
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
        "\\section{Tabla Inicial}\n"
        "\n"
        "La tabla inicial del método Simplex incluye las variables de holgura para convertir las desigualdades en igualdades.\n\n"
        "\\begin{center}\n"
        "\\small\n"
        "\\begin{tabular}{|c|");
    
    // Encabezados de columnas
    for (int j = 0; j < tabla->num_vars; j++) {
        g_string_append(latex, "c|");
    }
    for (int j = 0; j < tabla->num_rest; j++) {
        g_string_append(latex, "c|");
    }
    g_string_append(latex, "c|}\n\\hline\n");
    
    // Nombres de columnas
    g_string_append(latex, " & ");
    for (int j = 0; j < tabla->num_vars; j++) {
        g_string_append_printf(latex, "\\textbf{%s} & ", info->nombres_vars[j]);
    }
    for (int j = 0; j < tabla->num_rest; j++) {
        g_string_append_printf(latex, "\\textbf{$s_%d$} & ", j + 1);
    }
    g_string_append(latex, "\\textbf{L.D.} \\\\\n\\hline\n");
    
    // Fila Z
    g_string_append(latex, "\\textbf{Z} & ");
    for (int j = 0; j < tabla->num_vars + tabla->num_rest; j++) {
        char num_buffer[32];
        formatear_numero(tabla->tabla[0][j], num_buffer, sizeof(num_buffer));
        g_string_append_printf(latex, "%s & ", num_buffer);
    }
    
    char ld_buffer[32];
    formatear_numero(tabla->tabla[0][tabla->num_vars + tabla->num_rest], ld_buffer, sizeof(ld_buffer));
    g_string_append_printf(latex, "%s \\\\\n\\hline\n", ld_buffer);
    
    // Filas de restricciones
    for (int i = 1; i <= tabla->num_rest; i++) {
        g_string_append_printf(latex, "\\textbf{$s_%d$} & ", i);
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

void generar_tablas_intermedias_latex(GString *latex, GList *tablas, ProblemaInfo *info) {
    if (!tablas || g_list_length(tablas) <= 1) return;
    
    g_string_append_printf(latex,
        "\\section{Tablas Intermedias}\n"
        "\n"
        "A continuación se presentan las tablas intermedias generadas durante la ejecución del algoritmo Simplex.\n\n");
    
    int iteracion = 0;
    GList *iter = tablas;
    
    while (iter) {
        TablaSimplex *tabla = (TablaSimplex*)iter->data;
        
        g_string_append_printf(latex,
            "\\subsection{Iteración %d}\n"
            "\\begin{center}\n"
            "\\small\n"
            "\\begin{tabular}{|c|", iteracion);
        
        // Encabezados
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
        
        // Datos de la tabla
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
            g_string_append_printf(latex, "\\textbf{$s_%d$} & ", i);
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

void generar_solucion_latex(GString *latex, ResultadoSimplex *resultado, ProblemaInfo *info) {
    g_string_append_printf(latex,
        "\\section{Solución Óptima}\n"
        "\n");
    
    char valor_optimo_buffer[32];
    formatear_numero(resultado->valor_optimo, valor_optimo_buffer, sizeof(valor_optimo_buffer));
    
    g_string_append_printf(latex, "\\textbf{Valor óptimo: } $Z = %s$\n\n", valor_optimo_buffer);
    
    g_string_append(latex, "\\textbf{Solución óptima:}\n\\begin{align*}\n");
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
    g_string_append(latex, "\n\\end{align*}\n\n");
    
    switch (resultado->tipo_solucion) {
        case SOLUCION_UNICA:
            g_string_append(latex, "\\textbf{Tipo:} Solución Única\n\n");
            g_string_append(latex, "El problema tiene una única solución óptima en el punto encontrado.\n\n");
            break;
            
        case SOLUCION_MULTIPLE:
            g_string_append(latex, "\\textbf{Tipo:} Soluciones Múltiples\n\n");
            g_string_append(latex, "El problema tiene infinitas soluciones óptimas. Las dos soluciones básicas mostradas representan vértices del conjunto de soluciones óptimas.\n\n");
            g_string_append(latex, "Cualquier combinación convexa de estas soluciones también es óptima.\n\n");
            break;
            
        case NO_ACOTADO:
            g_string_append(latex, "\\textbf{Tipo:} Problema No Acotado\n\n");
            g_string_append(latex, "El problema no tiene solución óptima finita. La función objetivo puede mejorar indefinidamente.\n\n");
            break;
            
        case DEGENERADO:
            g_string_append(latex, "\\textbf{Tipo:} Problema Degenerado\n\n");
            g_string_append(latex, "Se detectó degeneración durante la ejecución del algoritmo. Esto ocurre cuando alguna variable básica toma valor cero.\n\n");
            break;
            
        case NO_FACTIBLE:
            g_string_append(latex, "\\textbf{Tipo:} Problema No Factible\n\n");
            g_string_append(latex, "El problema no tiene solución factible. El conjunto de restricciones es incompatible.\n\n");
            break;
    }
    
    g_string_append_printf(latex, "\\textbf{Iteraciones realizadas:} %d\n\n", resultado->iteraciones);
    
    if (resultado->mensaje) {
        g_string_append_printf(latex, "\\textbf{Observaciones:} %s\n\n", resultado->mensaje);
    }
}

void generar_documento_latex(ResultadoSimplex *resultado, ProblemaInfo *info, 
                            const char *nombre_archivo, gboolean mostrar_tablas) {
    GString *latex = g_string_new("");
    
    // Generar todas las secciones
    generar_portada_latex(latex, info->nombre_problema);
    generar_algoritmo_simplex_latex(latex);
    generar_problema_original_latex(latex, info);
    
    // Tabla inicial 
    if (resultado->tablas_intermedias) {
        TablaSimplex *tabla_inicial = (TablaSimplex*)g_list_first(resultado->tablas_intermedias)->data;
        generar_tabla_inicial_latex(latex, tabla_inicial, info);
    }
    
    // Tablas intermedias 
    if (mostrar_tablas && resultado->tablas_intermedias) {
        generar_tablas_intermedias_latex(latex, resultado->tablas_intermedias, info);
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