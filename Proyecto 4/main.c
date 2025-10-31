/*
MINI PROYECTO IO - Simplex
Estudiantes:
Emily Sánchez
Viviana Vargas
*/

#include "simplex.h"
#include "latex.h"
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <math.h>
#include <ctype.h>
#include <pthread.h>

GtkWidget *window;
GtkWidget *fixed;

GtkWidget *titleSimplex;
GtkWidget *instructions;
GtkWidget *nameLabel;
GtkWidget *nameEntry;

GtkWidget *rbMaximizar;
GtkWidget *rbMinimizar;

GtkWidget *varQtyLabel;
GtkWidget *spinVariables;
GtkWidget *variablesScroll;
GtkWidget *varnameLabel;
GtkWidget *gridVariables; 

GtkWidget *funcionLabel;
GtkWidget *ZLabel;
GtkWidget *ZScroll;
GtkWidget *ZGrid;

GtkWidget *restQtyLabel;
GtkWidget *maxminSelectionLabel;
GtkWidget *sujetoLabel;
GtkWidget *spinRestrictions;
GtkWidget *restrictionsScroll;
GtkWidget *xiLabel;
GtkWidget *gridRestrictions;

GtkWidget *continueButton;
GtkWidget *loadFileButton;
GtkWidget *cargarLabel;
GtkWidget *showTablesCheck;
GtkWidget *solveButton;
GtkWidget *exitButton;

GtkWidget *dantzigImage;
GtkWidget *saveButton;
GtkWidget *loadButton;

GtkBuilder *builder;
GtkCssProvider *cssProvider;

// ---- Variables Globales ----

const char *type = "MAX";
gboolean showTables = FALSE;

typedef struct {
    double **A;
    int rows;
    int cols;
    int n_vars;
    int n_slack;
    int n_exceso;
    int n_artificial;
} Tabla;

// -------------------------------------------
// ----------------- Helpers -----------------
// -------------------------------------------

static void validateEntryNumber(GtkEditable *editable,const gchar *text, gint length, gint *position, gpointer user_data) {
    const gchar *current = gtk_entry_get_text(GTK_ENTRY(editable));
    gboolean isReal = (strchr(current, '.') != NULL);
    gboolean isNegative = (current[0] == '-');

    gint sel_start = 0, sel_end = 0;
    gboolean has_sel = gtk_editable_get_selection_bounds(editable, &sel_start, &sel_end);

    GString *filtered = g_string_sized_new(length);

    for (int i = 0; i < length; i++) {
        char c = text[i];

        if (g_ascii_isdigit(c)) {
            g_string_append_c(filtered, c);
            continue;
        }

        if (c == '.') {
            if (!isReal || (has_sel && strchr(current + sel_start, '.') < current + sel_end)) {
                g_string_append_c(filtered, c);
                isReal = TRUE;
            }
            continue;
        }

        if (c == '-') {
            gboolean possibleNegative = FALSE;

            int pos = *position + filtered->len;

            if (!isNegative) {
                if (pos == 0) {
                    possibleNegative = TRUE;
                } else if (has_sel && sel_start == 0) {
                    possibleNegative = TRUE;
                }
            }

            if (possibleNegative) {
                g_string_append_c(filtered, c);
                isNegative = TRUE;
            }
            continue;
        }
    }

    if (filtered->len != length) {
        g_signal_stop_emission_by_name(editable, "insert-text");
        gtk_editable_insert_text(editable, filtered->str, filtered->len, position);
    }
    g_string_free(filtered, TRUE);
}

static gboolean normalizeValue(GtkWidget *entry, GdkEvent *event, gpointer user_data) {
    const char *t = gtk_entry_get_text(GTK_ENTRY(entry));
    if (t == NULL || *t == '\0' || (strcmp(t, "-") == 0) || (strcmp(t, ".") == 0) || (strcmp(t, "-.") == 0)) {
        gtk_entry_set_text(GTK_ENTRY(entry), "0");
    }
    return FALSE; 
}

static GtkWidget* grid_at(GtkWidget *grid, int col, int row) {
    return gtk_grid_get_child_at(GTK_GRID(grid), col, row);
}

static double entry_to_double(GtkWidget *w) {
    if (!w) return 0.0;

    if (GTK_IS_ENTRY(w)) {
        const char *t = gtk_entry_get_text(GTK_ENTRY(w));
        if (!t) return 0.0;
        char *endptr = NULL;
        return g_strtod(t, &endptr);
    }

    if (GTK_IS_SPIN_BUTTON(w)) {
        return gtk_spin_button_get_value(GTK_SPIN_BUTTON(w));
    }

    g_warning("Se esperaba GtkEntry/GtkSpinButton pero se encontró otro widget.");
    return 0.0;
}

static int combo_active_index(GtkWidget *maybe_combo) {
    if (!maybe_combo) return 0;
    return gtk_combo_box_get_active(GTK_COMBO_BOX(maybe_combo));
}

static double **alloc_matrix(int rows, int cols) {
    double **M = g_new0(double*, rows);
    for (int i = 0; i < rows; ++i) {
        M[i] = g_new0(double, cols);
    }
    return M;
}

static void free_tabla(Tabla *tab) {
    if (!tab) return;
    if (tab->A) {
        for (int i = 0; i < tab->rows; ++i) g_free(tab->A[i]);
        g_free(tab->A);
    }
    g_free(tab);
}

// -------------------------------------------
// ---------------- Funciones ----------------
// -------------------------------------------

static void clearContainer(GtkWidget *container) {
    GList *children = gtk_container_get_children(GTK_CONTAINER(container));
    for (GList *it = children; it != NULL; it = g_list_next(it)) {
        gtk_widget_destroy(GTK_WIDGET(it->data));
    }
    g_list_free(children);
}

void createVariables(GtkSpinButton *spin, gpointer user_data) {
    int cant = gtk_spin_button_get_value_as_int(spin);

    GList *children = gtk_container_get_children(GTK_CONTAINER(gridVariables));
    for (GList *i = children; i != NULL; i = g_list_next(i)) {
        gtk_widget_destroy(GTK_WIDGET(i->data));
    }
    g_list_free(children);

    for (int i = 0; i < cant; i++) {
        gchar defaultName[16];
        g_snprintf(defaultName, sizeof(defaultName), "X%d", i + 1);

        GtkWidget *entry = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(entry), defaultName);
        gtk_entry_set_alignment(GTK_ENTRY(entry), 0.0); 

        gtk_grid_attach(GTK_GRID(gridVariables), entry, 0, i, 1, 1);
    }

    gtk_widget_show_all(gridVariables);
}

static void createZ (void) {
    int n = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinVariables));
    if (n <= 0) {
        clearContainer(ZGrid);
        gtk_widget_show_all(ZGrid);
        return;
    }

    clearContainer(ZGrid);

    int col = 0; 

    for (int i = 0; i < n; i++) {
        GtkWidget *c = gtk_entry_new();
        gtk_entry_set_width_chars(GTK_ENTRY(c), 6);    
        gtk_entry_set_alignment(GTK_ENTRY(c), 1.0);
        g_signal_connect(c, "insert-text", G_CALLBACK(validateEntryNumber), NULL);
        g_signal_connect(c, "focus-out-event", G_CALLBACK(normalizeValue), NULL);    
        gtk_grid_attach(GTK_GRID(ZGrid), c, col++, 0, 1, 1);

        GtkWidget *nombreEntry = gtk_grid_get_child_at(GTK_GRID(gridVariables), 0, i);
        const char *nombreVar = nombreEntry ? gtk_entry_get_text(GTK_ENTRY(nombreEntry)) : "X?";
        GtkWidget *lblVar = gtk_label_new(nombreVar);
        gtk_label_set_xalign(GTK_LABEL(lblVar), 0.0);
        gtk_grid_attach(GTK_GRID(ZGrid), lblVar, col++, 0, 1, 1);

        if (i < n - 1) {
            GtkWidget *mas = gtk_label_new("+");
            gtk_grid_attach(GTK_GRID(ZGrid), mas, col++, 0, 1, 1);
        }
    }

    gtk_widget_show_all(ZGrid);
}

static void createRestrictions (void) {
    int m = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinRestrictions));
    int n = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinVariables));   

    clearContainer(gridRestrictions);

    if (m <= 0 || n <= 0) {
        gtk_widget_show_all(gridRestrictions);
        return;
    }

    for (int r = 0; r < m; r++) {
        int col = 0;

        for (int i = 0; i < n; i++) {
            GtkWidget *c = gtk_entry_new();
            gtk_entry_set_width_chars(GTK_ENTRY(c), 6);
            gtk_entry_set_alignment(GTK_ENTRY(c), 1.0);
            gtk_entry_set_text(GTK_ENTRY(c), "0");
            g_signal_connect(c, "insert-text", G_CALLBACK(validateEntryNumber), NULL);
            g_signal_connect(c, "focus-out-event", G_CALLBACK(normalizeValue), NULL);
            gtk_grid_attach(GTK_GRID(gridRestrictions), c, col++, r, 1, 1);

            GtkWidget *nombreEntry = gtk_grid_get_child_at(GTK_GRID(gridVariables), 0, i);
            const char *nombreVar = nombreEntry ? gtk_entry_get_text(GTK_ENTRY(nombreEntry)) : "X?";
            GtkWidget *lblVar = gtk_label_new(nombreVar);
            gtk_label_set_xalign(GTK_LABEL(lblVar), 0.0);
            gtk_grid_attach(GTK_GRID(gridRestrictions), lblVar, col++, r, 1, 1);

            if (i < n - 1) {
                GtkWidget *mas = gtk_label_new("+");
                gtk_grid_attach(GTK_GRID(gridRestrictions), mas, col++, r, 1, 1);
            }
        }

        GtkWidget *rel = gtk_combo_box_text_new();
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rel), "≤");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rel), "≥");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rel), "=");
        gtk_combo_box_set_active(GTK_COMBO_BOX(rel), 0);
        gtk_grid_attach(GTK_GRID(gridRestrictions), rel, col++, r, 1, 1);

        GtkWidget *rhs = gtk_entry_new();
        gtk_entry_set_width_chars(GTK_ENTRY(rhs), 8);
        gtk_entry_set_alignment(GTK_ENTRY(rhs), 1.0);
        gtk_entry_set_text(GTK_ENTRY(rhs), "0");
        g_signal_connect(rhs, "insert-text", G_CALLBACK(validateEntryNumber), NULL);
        g_signal_connect(rhs, "focus-out-event", G_CALLBACK(normalizeValue), NULL);
        gtk_grid_attach(GTK_GRID(gridRestrictions), rhs, col++, r, 1, 1);
    }

    gtk_widget_show_all(gridRestrictions);
}

// Función para mostrar resultados en una nueva ventana
void mostrar_resultados(ResultadoSimplex *resultado, int num_vars, const char **nombres_vars) {
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *scroll;
    GtkWidget *label;
    GString *texto = g_string_new("");
    
    dialog = gtk_dialog_new_with_buttons("Resultados del Simplex",
                                        GTK_WINDOW(window),
                                        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                        "Cerrar",
                                        GTK_RESPONSE_CLOSE,
                                        NULL);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
    
    label = gtk_label_new("");
    gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
    gtk_label_set_selectable(GTK_LABEL(label), TRUE);
    gtk_container_add(GTK_CONTAINER(scroll), label);
    gtk_container_add(GTK_CONTAINER(content_area), scroll);
    
    g_string_append_printf(texto, "=== RESULTADOS DEL ALGORITMO SIMPLEX ===\n\n");
    g_string_append_printf(texto, "Valor óptimo: %.4f\n", resultado->valor_optimo);
    g_string_append_printf(texto, "\nSolución óptima:\n");
    
    for (int i = 0; i < num_vars; i++) {
        const char *nombre = nombres_vars[i] ? nombres_vars[i] : "X?";
        g_string_append_printf(texto, "  %s = %.4f\n", nombre, resultado->solucion[i]);
    }
    
    switch (resultado->tipo_solucion) {
        case SOLUCION_UNICA:
            g_string_append_printf(texto, "\nTipo: SOLUCIÓN ÚNICA\n");
            break;
        case SOLUCION_MULTIPLE:
            g_string_append_printf(texto, "\nTipo: SOLUCIONES MÚLTIPLES\n");
            break;
        case NO_ACOTADO:
            g_string_append_printf(texto, "\nTipo: PROBLEMA NO ACOTADO\n");
            break;
        case DEGENERADO:
            g_string_append_printf(texto, "\nTipo: PROBLEMA DEGENERADO\n");
            break;
        case NO_FACTIBLE:
            g_string_append_printf(texto, "\nTipo: PROBLEMA NO FACTIBLE\n");
            break;
    }
    
    if (resultado->mensaje) {
        g_string_append_printf(texto, "\n%s\n", resultado->mensaje);
    }
    
    gtk_label_set_text(GTK_LABEL(label), texto->str);
    g_string_free(texto, TRUE);
    
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Función para construir el problema simplex desde la interfaz
ResultadoSimplex* construir_y_resolver_simplex(void) {
    int n = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinVariables));
    int m = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinRestrictions));
    
    if (n <= 0 || m < 0) {
        return NULL;
    }
    
    TipoProblema tipo = (strcmp(type, "MAX") == 0) ? MAXIMIZACION : MINIMIZACION;
    TablaSimplex *tabla = crear_tabla_simplex(n, m, tipo);
    
    double *coef_obj = g_new0(double, n);
    int z_base = 0;
    GtkWidget *c00 = grid_at(ZGrid, 0, 0);
    if (c00 && GTK_IS_LABEL(c00)) z_base = 1;
    
    for (int i = 0; i < n; i++) {
        GtkWidget *coef_entry = grid_at(ZGrid, z_base + 3*i, 0);
        coef_obj[i] = entry_to_double(coef_entry);
    }
    establecer_funcion_objetivo_simplex(tabla, coef_obj);
    g_free(coef_obj);
    
    for (int r = 0; r < m; r++) {
        double *coef_rest = g_new0(double, n);
        double lado_derecho = 0.0;
        
        for (int i = 0; i < n; i++) {
            GtkWidget *coef_entry = grid_at(gridRestrictions, 0 + 3*i, r);
            coef_rest[i] = entry_to_double(coef_entry);
        }
        
        GtkWidget *rhs_entry = grid_at(gridRestrictions, 3*n, r);
        lado_derecho = entry_to_double(rhs_entry);
        
        GtkWidget *rel = grid_at(gridRestrictions, 3*n - 1, r);
        int tipo_rest = combo_active_index(rel);
        
        if (tipo_rest == 0) {
            agregar_restriccion_simplex(tabla, r, coef_rest, lado_derecho);
        } else {
            g_printerr("Solo se permiten restricciones de tipo '≤'\n");
            g_free(coef_rest);
            liberar_tabla_simplex(tabla);
            return NULL;
        }
        
        g_free(coef_rest);
    }
    
    ResultadoSimplex *resultado = ejecutar_simplex_completo(tabla, showTables);
    liberar_tabla_simplex(tabla);
    
    return resultado;
}

// -------------------------------------------
// ----------------- BOTONES -----------------
// -------------------------------------------

void on_rbMaximizar_toggled(GtkRadioButton *rbMaximizar, gpointer user_data) {
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rbMaximizar))) {
        type = "MAX";
    }
}

void on_rbMinimizar_toggled(GtkRadioButton *rbMinimizar, gpointer user_data) {
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rbMinimizar))) {
        type = "MIN";
    }
}

void on_continueButton_clicked(GtkWidget *widget, gpointer data) {
    createZ();
    createRestrictions();

    if (type == "MAX"){
        gtk_label_set_text(GTK_LABEL(maxminSelectionLabel), "Maximizar");
    }

    if (type == "MIN"){
        gtk_label_set_text(GTK_LABEL(maxminSelectionLabel), "Minimizar");
    }

    gtk_widget_set_sensitive(solveButton, TRUE);
}

static void on_showTablesCheck_button_toggled(GtkToggleButton *button, gpointer user_data) {
    if (gtk_toggle_button_get_active(button)) {
        showTables = TRUE;
    } else {
        showTables = FALSE;
    }
}

void on_loadFileButton_clicked(GtkWidget *widget, gpointer data) {
    g_print("Cargar archivo presionado\n");
}

void on_solveButton_clicked(GtkWidget *widget, gpointer data) {
    int n = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinVariables));
    int m = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinRestrictions));
    
    if (n <= 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Debe ingresar al menos una variable");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    if (m <= 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Debe ingresar al menos una restricción");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    gboolean todas_menor_igual = TRUE;
    for (int r = 0; r < m; r++) {
        GtkWidget *rel = grid_at(gridRestrictions, 3*n - 1, r);
        int tipo_rest = combo_active_index(rel);
        if (tipo_rest != 0) {
            todas_menor_igual = FALSE;
            break;
        }
    }
    
    if (!todas_menor_igual) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_WARNING,
                                                  GTK_BUTTONS_OK,
                                                  "Solo se permiten restricciones de tipo '≤' según los requisitos del proyecto");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    // En on_solveButton_clicked, antes de construir_y_resolver_simplex
    g_print("=== VERIFICACIÓN DE DATOS ===\n");
    g_print("FunciÓn objetivo: Z = ");
    for (int i = 0; i < n; i++) {
        GtkWidget *coef_entry = grid_at(ZGrid, 3*i, 0); // Ajusta según tu estructura
        double coef = entry_to_double(coef_entry);
        g_print("%.2fX%d ", coef, i+1);
        if (i < n-1) g_print("+ ");
    }
    g_print("\n");

    g_print("Restricciones:\n");
    for (int r = 0; r < m; r++) {
        for (int i = 0; i < n; i++) {
            GtkWidget *coef_entry = grid_at(gridRestrictions, 3*i, r);
            double coef = entry_to_double(coef_entry);
            g_print("%.2fX%d ", coef, i+1);
            if (i < n-1) g_print("+ ");
        }
        GtkWidget *rhs_entry = grid_at(gridRestrictions, 3*n, r);
        double rhs = entry_to_double(rhs_entry);
        g_print("<= %.2f\n", rhs);
    }
    
    ResultadoSimplex *resultado = construir_y_resolver_simplex();
    
    if (resultado) {
        const char **nombres_vars = g_new0(const char*, n);
        for (int i = 0; i < n; i++) {
            GtkWidget *entry = grid_at(gridVariables, 0, i);
            if (entry && GTK_IS_ENTRY(entry)) {
                nombres_vars[i] = gtk_entry_get_text(GTK_ENTRY(entry));
            } else {
                nombres_vars[i] = "X?";
            }
        }
        
        mostrar_resultados(resultado, n, nombres_vars);
        
        ProblemaInfo info;
        info.nombres_vars = nombres_vars;
        info.num_vars = n;
        info.num_rest = m;
        
        const char *nombre_problema = gtk_entry_get_text(GTK_ENTRY(nameEntry));
        info.nombre_problema = (nombre_problema && strlen(nombre_problema) > 0) ? 
                              nombre_problema : "Problema de Optimización";
        info.tipo_problema = type;
        
        info.coef_obj = g_new0(double, n);
        int z_base = 0;
        GtkWidget *c00 = grid_at(ZGrid, 0, 0);
        if (c00 && GTK_IS_LABEL(c00)) z_base = 1;
        
        for (int i = 0; i < n; i++) {
            GtkWidget *coef_entry = grid_at(ZGrid, z_base + 3*i, 0);
            info.coef_obj[i] = entry_to_double(coef_entry);
        }
        
        info.coef_rest = g_new0(double*, m);
        info.lados_derechos = g_new0(double, m);
        
        for (int r = 0; r < m; r++) {
            info.coef_rest[r] = g_new0(double, n);
            for (int i = 0; i < n; i++) {
                GtkWidget *coef_entry = grid_at(gridRestrictions, 0 + 3*i, r);
                info.coef_rest[r][i] = entry_to_double(coef_entry);
            }
            
            GtkWidget *rhs_entry = grid_at(gridRestrictions, 3*n, r);
            info.lados_derechos[r] = entry_to_double(rhs_entry);
        }
        
        char nombre_archivo_tex[256];
        GString *nombre_seguro = g_string_new("");
        for (const char *p = info.nombre_problema; *p; p++) {
            if (g_ascii_isalnum(*p)) {
                g_string_append_c(nombre_seguro, *p);
            } else if (*p == ' ') {
                g_string_append_c(nombre_seguro, '_');
            }
        }
        snprintf(nombre_archivo_tex, sizeof(nombre_archivo_tex), 
                "simplex_%s.tex", nombre_seguro->str);
        
        generar_documento_latex(resultado, &info, nombre_archivo_tex, showTables);
        
        GtkWidget *progress_dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_NONE,
            "Generando documento PDF...\n\nPor favor espere.");
        gtk_window_set_title(GTK_WINDOW(progress_dialog), "Generando PDF");
        gtk_dialog_run(GTK_DIALOG(progress_dialog));
        
        compilar_y_mostrar_pdf(nombre_archivo_tex);
        gtk_widget_destroy(progress_dialog);
        
        char nombre_pdf[256];
        snprintf(nombre_pdf, sizeof(nombre_pdf), "simplex_%s.pdf", nombre_seguro->str);
        
        GtkWidget *success_dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "Proceso completado exitosamente.\n\n"
            "• Archivo LaTeX generado: %s\n"
            "• Archivo PDF generado: %s\n\n"
            "El PDF se ha abierto automáticamente en el visor.",
            nombre_archivo_tex,
            nombre_pdf);
        
        gtk_window_set_title(GTK_WINDOW(success_dialog), "Proceso Completado");
        gtk_dialog_run(GTK_DIALOG(success_dialog));
        gtk_widget_destroy(success_dialog);
        
        g_string_free(nombre_seguro, TRUE);
        g_free(nombres_vars);
        g_free(info.coef_obj);
        for (int r = 0; r < m; r++) {
            g_free(info.coef_rest[r]);
        }
        g_free(info.coef_rest);
        g_free(info.lados_derechos);
        
        liberar_resultado(resultado);
        
    } else {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Error al resolver el problema simplex");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

void on_saveButton_clicked(GtkWidget *widget, gpointer data) {
    // Por implementar
}

void on_loadButton_clicked(GtkWidget *widget, gpointer data) {
    gtk_widget_set_sensitive(solveButton, TRUE);
}

void on_exitButton_clicked (GtkButton *exitButton, gpointer data){
    gtk_main_quit();
}

//Función para que se utilice el archivo .css como proveedor de estilos.
void set_css (GtkCssProvider *cssProvider, GtkWidget *widget){
    GtkStyleContext *styleContext = gtk_widget_get_style_context(widget);
    gtk_style_context_add_provider(styleContext,GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
}

//Main
int main (int argc, char *argv[]){
    gtk_init(&argc, &argv);
    
    builder =  gtk_builder_new_from_file ("Simplex.glade");
    
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    gtk_builder_connect_signals(builder, NULL);
    
    fixed  = GTK_WIDGET(gtk_builder_get_object(builder, "fixed"));

    titleSimplex = GTK_WIDGET(gtk_builder_get_object(builder, "titleSimplex"));
    instructions = GTK_WIDGET(gtk_builder_get_object(builder, "instructions"));
    nameLabel = GTK_WIDGET(gtk_builder_get_object(builder, "nameLabel"));
    nameEntry = GTK_WIDGET(gtk_builder_get_object(builder, "nameEntry"));
    rbMaximizar = GTK_WIDGET(gtk_builder_get_object(builder, "rbMaximizar"));
    rbMinimizar = GTK_WIDGET(gtk_builder_get_object(builder, "rbMinimizar"));
    varQtyLabel = GTK_WIDGET(gtk_builder_get_object(builder, "varQtyLabel"));
    spinVariables = GTK_WIDGET(gtk_builder_get_object(builder, "spinVariables"));
    variablesScroll = GTK_WIDGET(gtk_builder_get_object(builder, "variablesScroll"));
    varnameLabel = GTK_WIDGET(gtk_builder_get_object(builder, "varnameLabel"));
    funcionLabel = GTK_WIDGET(gtk_builder_get_object(builder, "funcionLabel"));
    ZLabel = GTK_WIDGET(gtk_builder_get_object(builder, "ZLabel"));
    ZScroll = GTK_WIDGET(gtk_builder_get_object(builder, "ZScroll"));
    restQtyLabel = GTK_WIDGET(gtk_builder_get_object(builder, "restQtyLabel"));
    maxminSelectionLabel = GTK_WIDGET(gtk_builder_get_object(builder, "maxminSelectionLabel"));
    sujetoLabel = GTK_WIDGET(gtk_builder_get_object(builder, "sujetoLabel"));
    spinRestrictions = GTK_WIDGET(gtk_builder_get_object(builder, "spinRestrictions"));
    restrictionsScroll = GTK_WIDGET(gtk_builder_get_object(builder, "restrictionsScroll"));
    xiLabel = GTK_WIDGET(gtk_builder_get_object(builder, "xiLabel"));
    continueButton = GTK_WIDGET(gtk_builder_get_object(builder, "continueButton"));
    loadFileButton = GTK_WIDGET(gtk_builder_get_object(builder, "loadFileButton"));
    cargarLabel = GTK_WIDGET(gtk_builder_get_object(builder, "cargarLabel"));
    showTablesCheck = GTK_WIDGET(gtk_builder_get_object(builder, "showTablesCheck"));
    solveButton = GTK_WIDGET(gtk_builder_get_object(builder, "solveButton"));
    exitButton = GTK_WIDGET(gtk_builder_get_object(builder, "exitButton"));
    dantzigImage = GTK_WIDGET(gtk_builder_get_object(builder, "dantzigImage"));
    saveButton = GTK_WIDGET(gtk_builder_get_object(builder, "saveButton"));
    loadButton = GTK_WIDGET(gtk_builder_get_object(builder, "loadButton"));

    cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(cssProvider, "theme.css", NULL);

    set_css(cssProvider, window);
    set_css(cssProvider, continueButton);
    set_css(cssProvider, loadFileButton);
    set_css(cssProvider, solveButton);
    set_css(cssProvider, exitButton);

    gtk_widget_set_sensitive(solveButton, FALSE);

    g_signal_connect(exitButton, "clicked", G_CALLBACK(on_exitButton_clicked), NULL);
    g_signal_connect(continueButton, "clicked", G_CALLBACK(on_continueButton_clicked), NULL);
    g_signal_connect(loadFileButton, "clicked", G_CALLBACK(on_loadFileButton_clicked), NULL);
    g_signal_connect(solveButton, "clicked", G_CALLBACK(on_solveButton_clicked), NULL);
    g_signal_connect(exitButton, "clicked", G_CALLBACK(on_exitButton_clicked), NULL);
    g_signal_connect(saveButton, "clicked", G_CALLBACK(on_saveButton_clicked), NULL);
    g_signal_connect(loadButton, "clicked", G_CALLBACK(on_loadButton_clicked), NULL);

    gridVariables = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(gridVariables), 6);
    gtk_grid_set_column_spacing(GTK_GRID(gridVariables), 6);

    ZGrid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(ZGrid), 8);
    gtk_grid_set_row_spacing(GTK_GRID(ZGrid), 6);
    gtk_container_add(GTK_CONTAINER(ZScroll), ZGrid);

    gridRestrictions = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(gridRestrictions), 8);
    gtk_grid_set_row_spacing(GTK_GRID(gridRestrictions), 6);
    gtk_container_add(GTK_CONTAINER(restrictionsScroll), gridRestrictions);

    gtk_container_add(GTK_CONTAINER(variablesScroll), gridVariables);
        
    g_signal_connect(spinVariables, "value-changed",G_CALLBACK(createVariables), NULL);
    
    g_signal_connect(rbMaximizar, "toggled", G_CALLBACK(on_rbMaximizar_toggled), NULL);
    g_signal_connect(rbMinimizar, "toggled", G_CALLBACK(on_rbMinimizar_toggled), NULL);
    g_signal_connect(showTablesCheck, "toggled", G_CALLBACK(on_showTablesCheck_button_toggled), NULL);

    gtk_widget_show(window);
    
    gtk_main();

    return EXIT_SUCCESS;
}