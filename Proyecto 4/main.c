/*
MINI PROYECTO IO - Simplex
Estudiantes:
Emily Sánchez
Viviana Vargas
*/

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

GtkWidget *funcionLabel;
GtkWidget *ZLabel;
GtkWidget *variablesScroll1;

GtkWidget *restQtyLabel;
GtkWidget *maxminSelectionLabel;
GtkWidget *sujetoLabel;
GtkWidget *spinRestrictions;
GtkWidget *restrictionsScroll;
GtkWidget *restQtyLabel1;

GtkWidget *continueButton;
GtkWidget *loadFileButton;
GtkWidget *cargarLabel;
GtkWidget *showTablesCheck;
GtkWidget *solveButton;
GtkWidget *exitButton;

GtkWidget *dantzigImage;

GtkBuilder *builder;
GtkCssProvider *cssProvider;


/* 
Para ejecutar:
Abrir la terminal en el folder principal de Mini Proyecto.
Usar el comando: gcc main.c $(pkg-config --cflags --libs gtk+-3.0) -o main -export-dynamic
(Esto para que pueda correr con libgtk-3.0)
Ejecutar el main con el comando en terminal: ./main
También se puede hacer click en el archivo ejecutable 'Main' en la carpeta principal.
*/

//Función para que se utilice el archivo .css como proveedor de estilos.
void set_css (GtkCssProvider *cssProvider, GtkWidget *widget){
	GtkStyleContext *styleContext = gtk_widget_get_style_context(widget);
	gtk_style_context_add_provider(styleContext,GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
}



//Función de acción para el botón de 'Exit' que cierra todo el programa.
void on_exitButton_clicked (GtkButton *exitButton, gpointer data){
	gtk_main_quit();
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
    varnameLabel = GTK_WIDGET(gtk_builder_get_object(builder, "varQtyLabel1"));
    funcionLabel = GTK_WIDGET(gtk_builder_get_object(builder, "funcionLabel"));
    ZLabel = GTK_WIDGET(gtk_builder_get_object(builder, "ZLabel"));
    variablesScroll1 = GTK_WIDGET(gtk_builder_get_object(builder, "variablesScroll1"));
    restQtyLabel = GTK_WIDGET(gtk_builder_get_object(builder, "restQtyLabel"));
    maxminSelectionLabel = GTK_WIDGET(gtk_builder_get_object(builder, "maxminSelectionLabel"));
    sujetoLabel = GTK_WIDGET(gtk_builder_get_object(builder, "sujetoLabel"));
    spinRestrictions = GTK_WIDGET(gtk_builder_get_object(builder, "spinRestrictions"));
    restrictionsScroll = GTK_WIDGET(gtk_builder_get_object(builder, "restrictionsScroll"));
    restQtyLabel1 = GTK_WIDGET(gtk_builder_get_object(builder, "restQtyLabel1"));
    continueButton = GTK_WIDGET(gtk_builder_get_object(builder, "continueButton"));
    loadFileButton = GTK_WIDGET(gtk_builder_get_object(builder, "loadFileButton"));
    cargarLabel = GTK_WIDGET(gtk_builder_get_object(builder, "cargarLabel"));
    showTablesCheck = GTK_WIDGET(gtk_builder_get_object(builder, "showTablesCheck"));
    solveButton = GTK_WIDGET(gtk_builder_get_object(builder, "solveButton"));
    exitButton = GTK_WIDGET(gtk_builder_get_object(builder, "exitButton"));
    dantzigImage = GTK_WIDGET(gtk_builder_get_object(builder, "dantzigImage"));


	cssProvider = gtk_css_provider_new();
	gtk_css_provider_load_from_path(cssProvider, "theme.css", NULL);

	set_css(cssProvider, window);
	set_css(cssProvider, continueButton);
	set_css(cssProvider, loadFileButton);
	set_css(cssProvider, solveButton);
	set_css(cssProvider, exitButton);

	g_signal_connect(exitButton, "clicked", G_CALLBACK(on_exitButton_clicked), NULL);

	
	gtk_widget_show(window);
	
	gtk_main();

	return EXIT_SUCCESS;
	}


