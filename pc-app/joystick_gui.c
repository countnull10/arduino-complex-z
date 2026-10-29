#include <gtk/gtk.h>
#include <math.h>

#include "serial.h"

int serial_fd;
int x, y;
unsigned char deadzone;

// Structure to store application state
typedef struct {
    GtkWidget *drawing_area;
    GtkWidget *coord_label;
    double angle;       // Angle to make the internal point move
    double cx, cy;      // Center of the circle
    double radius;      // Circle radius
    double px, py;      // Internal point coordinates
    double qx, qy;      // Perimeter point coordinates
} AppState;

/* Callback to handle key presses */
static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    if (event->keyval == GDK_KEY_F11) {
        GtkWindow *window = GTK_WINDOW(widget);
        GdkWindow *gdk_win = gtk_widget_get_window(widget);

        if (gdk_win) {
            // Check current state via GDK (compatible with all GTK3 versions)
            if (gdk_window_get_state(gdk_win) & GDK_WINDOW_STATE_FULLSCREEN) {
                gtk_window_unfullscreen(window);
            } else {
                gtk_window_fullscreen(window);
            }
        }

        return TRUE; // Event handled, stop propagation
    }

    return FALSE; // Let other keys be handled normally
}

// Drawing function (Cairo)
static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    AppState *state = (AppState *)data;
    guint width = gtk_widget_get_allocated_width(widget);
    guint height = gtk_widget_get_allocated_height(widget);

    // White background
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    // Calculate center and radius based on canvas dimensions
    state->cx = width / 2.0;
    state->cy = height / 2.0;
    state->radius = fmin(width, height) / 2.0 - 30.0; // Safety margin

    // 1. Draw main circle (Black, thickness 1 pt)
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_set_line_width(cr, 1.0);
    cairo_arc(cr, state->cx, state->cy, state->radius, 0, 2 * G_PI);
    cairo_stroke(cr);

    // 2. Calculate internal point position (moves along an internal circular trajectory)
    double r_internal = state->radius * 0.6; // Internal radius smaller than the main radius
    //state->px = state->cx + r_internal * cos(state->angle);
    //state->py = state->cy + r_internal * sin(state->angle);

    state->px = state->cx + ((r_internal/512.0) * x);
    state->py = state->cy + ((r_internal/512.0) * y);

    // 3. Calculate the intersection of the ray (passing from center through the point) with the perimeter
    double dx = state->px - state->cx;
    double dy = state->py - state->cy;
    double dist = sqrt(dx * dx + dy * dy);

    if (dist > 0) {
        state->qx = state->cx + state->radius * (dx / dist);
        state->qy = state->cy + state->radius * (dy / dist);
    } else {
        state->qx = state->cx + state->radius;
        state->qy = state->cy;
    }

    // 3.9 CHECK DEADZONE
    deadzone = (x > 15 || x < -15 || y > 15 || y < -15) ? 0 : 1;

    if (deadzone == 0) {
        // 4. Draw the ray from center to perimeter (Black, thickness 5 pt)
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_set_line_width(cr, 1.0);
        cairo_move_to(cr, state->cx, state->cy);
        cairo_line_to(cr, state->qx, state->qy);
        cairo_stroke(cr);

        // Perimeter point
        cairo_arc(cr, state->qx, state->qy, 3.0, 0, 2 * G_PI);
        cairo_fill(cr);
    }

    // Internal point
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    cairo_arc(cr, state->px, state->py, 3.0, 0, 2 * G_PI);
    cairo_fill(cr);

    return FALSE;
}

// Function called periodically by the timer
static gboolean on_timer(gpointer data) {
    AppState *state = (AppState *)data;

    int result = get_data(serial_fd, &x, &y);

    // Update angle to move the point

    // Calculate coordinates relative to center (inverting Y axis for Cartesian consistency)

    double rel_px = 0.0;
    double rel_py = 0.0;
    double rel_qx = 0.0;
    double rel_qy = 0.0;

    if (deadzone == 0) {
        rel_px = state->px - state->cx;
        rel_py = -(state->py - state->cy);
        rel_qx = state->qx - state->cx;
        rel_qy = -(state->qy - state->cy);
    }

    // Update label text with coordinates
    gchar *text = g_strdup_printf(
        "<b>Internal Point:</b>\n"
        "X: %.1f\n"
        "Y: %.1f\n\n"
        "<b>Perimeter Point:</b>\n"
        "X: %.1f\n"
        "Y: %.1f\n\n"
        "<b>Developed by</b>\n"
        "<b>Count NULL</b>\n\n"
        "<b>Links:</b>\n"
        "<b>Youtube:</b> https://www.youtube.com/@CountNull10\n"
        "<b>GitHub:</b> https://github.com/countnull10",
        rel_px, rel_py, rel_qx, rel_qy
    );
    gtk_label_set_markup(GTK_LABEL(state->coord_label), text);
    g_free(text);

    // Force canvas redraw
    gtk_widget_queue_draw(state->drawing_area);

    return G_SOURCE_CONTINUE; // Keep timer active
}

int main(int argc, char *argv[]) {

    char *portname = "/dev/ttyACM0";

    // 1. Serial initialization (done once at startup)
    serial_fd = init_serial(portname);
    if (serial_fd < 0) {
        fprintf(stderr, "Failed to start serial port.\n");
        return 1;
    } else {
        printf("Listening on port %s...\n", portname);
    }
    gtk_init(&argc, &argv);

    // Initialize state
    AppState state;
    state.angle = 0.0;

    // Main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Arduino Complex Z - PC GUI");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 450);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Connect signal for key capture
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), NULL);

    // Main horizontal layout (Box)
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 15);
    gtk_container_add(GTK_CONTAINER(window), hbox);

    // Logos side panel
    GtkWidget *vbox_left = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(hbox), vbox_left, FALSE, FALSE, 0);

    // 2. Logos
    GtkWidget *logo_fc = gtk_image_new_from_file("./assets/");
    GtkWidget *logo_ld = gtk_image_new_from_file("./assets/");

    // gtk_box_pack_end places the element at the bottom of the vertical box
    gtk_box_pack_end(GTK_BOX(vbox_left), logo_fc, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(vbox_left), logo_ld, FALSE, FALSE, 0);

    // Canvas (Drawing Area)
    state.drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(state.drawing_area, 400, 400);
    g_signal_connect(G_OBJECT(state.drawing_area), "draw", G_CALLBACK(on_draw), &state);
    gtk_box_pack_start(GTK_BOX(hbox), state.drawing_area, TRUE, TRUE, 0);

    // Coordinates side panel
    GtkWidget *vbox_right = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(hbox), vbox_right, FALSE, FALSE, 0);

    // 1. Coordinates label (goes at the top of the vertical box)
    state.coord_label = gtk_label_new(NULL);
    gtk_label_set_xalign(GTK_LABEL(state.coord_label), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox_right), state.coord_label, FALSE, FALSE, 0);

    // Start update timer (0.1 second = 100 ms)
    g_timeout_add(100, on_timer, &state);

    // Show all
    gtk_widget_show_all(window);
    gtk_main();

    // 3. Final close (if the program terminates)
    close(serial_fd);

    return 0;
}
