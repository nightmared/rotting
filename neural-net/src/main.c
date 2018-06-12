#include "common.h"
#include "morpion.h"
#include <time.h> // for seeding the randomness
#include <gtk/gtk.h>

static struct morpion* morpion;
static struct brain* brain;
GtkWidget* buttons_morpion_grid[9];

static void update_grid() {
	for (int i = 0; i < 9; i++) {
		switch (morpion->grid[i]) {
			case 'o': 
				gtk_button_set_label((GtkButton*)buttons_morpion_grid[i], "o");
				gtk_widget_set_sensitive(buttons_morpion_grid[i], FALSE);
				break;
			case 'x':
				gtk_button_set_label((GtkButton*)buttons_morpion_grid[i], "x");
				gtk_widget_set_sensitive(buttons_morpion_grid[i], FALSE);
				break;
			case '_':
				gtk_button_set_label((GtkButton*)buttons_morpion_grid[i], "");
				gtk_widget_set_sensitive(buttons_morpion_grid[i], TRUE);
		}
	}
}

static void change_data(GtkWidget *widget, gpointer data) {
	morpion->grid[(size_t)data] = 'x';
	morpion_AIplay(morpion, brain);
	update_grid();
	printf("grid: %s\n", morpion->grid);
}

static void load_from_file(GtkWidget *widget, gpointer data) {
	brain_free(brain);
	free(brain);
	brain = brain_load("data/brain-copie.txt");
}

static void learn_from_file(GtkWidget *widget, gpointer data) {
	brain_learn(brain, "data/parties_gagnantes_rond_1.txt");
}

static void save_to_file(GtkWidget *widget, gpointer data) {
	brain_save(brain, "data/brain.txt");
}

static void reset_morpion(GtkWidget *widget, gpointer data) {
	morpion_reset(morpion);
	update_grid();
}

static void mainWin(GtkApplication* app, gpointer user_data) {
	GtkWidget *window;

	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "Morpion");
	gtk_window_set_default_size (GTK_WINDOW(window), 250, 250);

	GtkGrid* container = (GtkGrid*)gtk_grid_new();
	gtk_container_add(GTK_CONTAINER(window), (GtkWidget*)container);

	for (int i = 0; i < 9; i++) {
		buttons_morpion_grid[i] = gtk_button_new();
		g_signal_connect(buttons_morpion_grid[i], "clicked", G_CALLBACK(change_data), (void*)(size_t)i);
		gtk_grid_attach(container, buttons_morpion_grid[i], i%3, 1+i-i%3, 1, 1);
	}

	GtkBox* commands = (GtkBox*)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_grid_attach(container, (GtkWidget*)commands, 0, 0, 3, 1);

	GtkWidget* load = gtk_button_new_with_label("load");
	g_signal_connect(load, "clicked", G_CALLBACK(load_from_file), NULL);
	gtk_box_pack_start(commands, load, TRUE, TRUE, 0);

	GtkWidget* learn = gtk_button_new_with_label("learn");
	g_signal_connect(learn, "clicked", G_CALLBACK(learn_from_file), NULL);
	gtk_box_pack_start(commands, learn, TRUE, TRUE, 0);

	GtkWidget* save = gtk_button_new_with_label("save");
	g_signal_connect(save, "clicked", G_CALLBACK(save_to_file), NULL);
	gtk_box_pack_start(commands, save, TRUE, TRUE, 0);

	GtkWidget* reset = gtk_button_new_with_label("reset");
	g_signal_connect(reset, "clicked", G_CALLBACK(reset_morpion), NULL);
	gtk_box_pack_start(commands, reset, TRUE, TRUE, 0);

	gtk_widget_show_all (window);
}

int main(int argc, char **argv) {
	// seed randomness
	srand(time(NULL));
	morpion = morpion_new(0);
	brain = brain_new(9, 9, 9, 4);
	brain_loadData(brain, "data/parties_gagnantes_rond_1.txt");
	brain_run(brain, brain->inData[0]);

	GtkApplication *app = gtk_application_new ("fr.tipe.morpion", G_APPLICATION_FLAGS_NONE);
	g_signal_connect (app, "activate", G_CALLBACK (mainWin), NULL);
	g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref(app);

	brain_free(brain);
	free(brain);
	free(morpion);
	return 0;
}
