/* 
   DIESES BEISPIEL WURDE MIT FOLGENDER HARDWARE GETESTET:
   AZ_TOUCH

Installiere die "lvgl"-Bibliothek Version 9.X von kisvegabor, um das TFT-Display anzusteuern - https://lvgl.io/
   *** WICHTIG: Die verfügbare lv_conf.h wird wahrscheinlich NICHT mit dem Beispiel funktionieren ***
   *** SIE MÜSSEN DIE lv_conf.h DATEI VERWENDEN, DIE IM Repo ZUR VERFÜGUNG GESTELLT WIRD, UM Das BEISPIEL ZU VERWENDEN ***
 */
#include <lvgl.h>

/*
Installiere die "TFT_eSPI"-Bibliothek von Bodmer, um das TFT-Display anzusteuern - https://github.com/Bodmer/TFT_eSPI
   *** WICHTIG: Die verfügbare User_Setup.h wird wahrscheinlich NICHT mit den Beispiel funktionieren ***
   *** SIE MÜSSEN DIE User_Setup.h DATEI VERWENDEN, DIE IM Repo ZUR VERFÜGUNG GESTELLT WIRD, UM Das BEISPIEL ZU VERWENDEN ***
*/
#include <TFT_eSPI.h>

// Installiere die "XPT2046_Touchscreen"-Bibliothek von Paul Stoffregen, um den Touchscreen zu verwenden - https://github.com/PaulStoffregen/XPT2046_Touchscreen
#include <XPT2046_Touchscreen.h>

// Touchscreen-Pins

#define XPT2046_MOSI 23  //gleich wie TFT
#define XPT2046_MISO 19  //gleich wie TFT
#define XPT2046_CLK 18   //gleich wie TFT

#define XPT2046_IRQ 27  //T_IRQ
#define XPT2046_CS 14   //T_CS

// Setze die Drehung des Touchscreens im Querformat
// Hinweis: Bei einigen Displays könnte der Touchscreen umgekehrt sein, daher muss die Drehung möglicherweise auf 3 gesetzt werden
#define TOUCH_ROTATION 1

SPIClass touchscreenSPI = SPIClass(HSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Touchscreen-Koordinaten: (x, y) und Druck (z)
int x, y, z;

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

// Wenn das Logging aktiviert ist, wird der Benutzer über die Vorgänge in der Bibliothek informiert
void log_print(lv_log_level_t level, const char* buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

// Touchscreen-Daten abrufen
void touchscreen_read(lv_indev_t* indev, lv_indev_data_t* data) {
  // Überprüft, ob der Touchscreen berührt wurde und gibt X, Y und Druck (Z) aus
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    // Touchscreen-Punkte abrufen
    TS_Point p = touchscreen.getPoint();
    // Kalibriert die Touchscreen-Punkte mit der map-Funktion auf die richtige Breite und Höhe
    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;

    data->state = LV_INDEV_STATE_PRESSED;

    // Setzt die Koordinaten
    data->point.x = x;
    data->point.y = y;

    // Gibt die Touchscreen-Informationen über X, Y und Druck (Z) im Seriellen Monitor aus
    /* 
    Serial.print("X = ");
    Serial.print(x);
    Serial.print(" | Y = ");
    Serial.print(y);
    Serial.print(" | Druck = ");
    Serial.print(z);
    Serial.println();
    */
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

int btn1_count = 0;
// Callback, der ausgelöst wird, wenn btn1 geklickt wird
static void event_handler_btn1(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    btn1_count++;
    LV_LOG_USER("Button geklickt %d", (int)btn1_count);
  }
}

// Callback, der ausgelöst wird, wenn btn2 geklickt/umgeschaltet wird
static void event_handler_btn2(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t* obj = (lv_obj_t*)lv_event_get_target(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    LV_UNUSED(obj);
    LV_LOG_USER("Umschalten %s", lv_obj_has_state(obj, LV_STATE_CHECKED) ? "ein" : "aus");
  }
}

static lv_obj_t* slider_label;
// Callback, der den aktuellen Schiebereglerwert auf dem TFT-Display und im Seriellen Monitor zur Debugging-Zwecken ausgibt
static void slider_event_callback(lv_event_t* e) {
  lv_obj_t* slider = (lv_obj_t*)lv_event_get_target(e);
  char buf[8];
  lv_snprintf(buf, sizeof(buf), "%d%%", (int)lv_slider_get_value(slider));
  lv_label_set_text(slider_label, buf);
  lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  LV_LOG_USER("Schieberegler geändert auf %d%%", (int)lv_slider_get_value(slider));
}

void lv_create_main_gui(void) {
  // Erstelle ein Text-Label zentriert oben ("Hello, world!")
  lv_obj_t* text_label = lv_label_create(lv_screen_active());
  lv_label_set_long_mode(text_label, LV_LABEL_LONG_WRAP);  // Bricht die langen Zeilen um
  lv_label_set_text(text_label, "Hello, world!");
  lv_obj_set_width(text_label, 150);  // Setze kleinere Breite, um die Zeilen umzubrechen
  lv_obj_set_style_text_align(text_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(text_label, LV_ALIGN_CENTER, 0, -90);

  lv_obj_t* btn_label;
  // Erstelle einen Button (btn1)
  lv_obj_t* btn1 = lv_button_create(lv_screen_active());
  lv_obj_add_event_cb(btn1, event_handler_btn1, LV_EVENT_ALL, NULL);
  lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -50);
  lv_obj_remove_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);

  btn_label = lv_label_create(btn1);
  lv_label_set_text(btn_label, "Button");
  lv_obj_center(btn_label);

  // Erstelle einen Umschalt-Button (btn2)
  lv_obj_t* btn2 = lv_button_create(lv_screen_active());
  lv_obj_add_event_cb(btn2, event_handler_btn2, LV_EVENT_ALL, NULL);
  lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 10);
  lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_set_height(btn2, LV_SIZE_CONTENT);

  btn_label = lv_label_create(btn2);
  lv_label_set_text(btn_label, "Umschalten");
  lv_obj_center(btn_label);

  // Erstelle einen Schieberegler, der zentriert am unteren Rand des TFT-Displays ausgerichtet ist
  lv_obj_t* slider = lv_slider_create(lv_screen_active());
  lv_obj_align(slider, LV_ALIGN_CENTER, 0, 60);
  lv_obj_add_event_cb(slider, slider_event_callback, LV_EVENT_VALUE_CHANGED, NULL);
  lv_slider_set_range(slider, 0, 100);
  lv_obj_set_style_anim_duration(slider, 2000, 0);

  // Erstelle ein Label unter dem Schieberegler, um den aktuellen Schiebereglerwert anzuzeigen
  slider_label = lv_label_create(lv_screen_active());
  lv_label_set_text(slider_label, "0%");
  lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}

void setup() {
  String LVGL_Arduino = String("LVGL-Bibliotheksversion: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
  Serial.println(LVGL_Arduino);

  // Starte LVGL
  lv_init();
  // Registriere die Druckfunktion für Debugging
  lv_log_register_print_cb(log_print);

  // Starte SPI für den Touchscreen und initialisiere den Touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);

  touchscreen.setRotation(TOUCH_ROTATION);

  // Erstelle ein Display-Objekt
  lv_display_t* disp;
  // Initialisiere das TFT-Display mit der TFT_eSPI-Bibliothek
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));

  // Initialisiere ein LVGL-Eingabegerät (Touchscreen)
  lv_indev_t* indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  // Setze die Callback-Funktion zum Lesen der Touchscreen-Eingabe
  lv_indev_set_read_cb(indev, touchscreen_read);

  // Funktion zum Zeichnen der GUI (Text, Buttons und Schieberegler)
  lv_create_main_gui();
}

void loop() {
  lv_task_handler();  // lasse die GUI ihre Arbeit machen
  lv_tick_inc(5);     // sage LVGL, wie viel Zeit vergangen ist
  delay(5);           // lasse diese Zeit verstreichen
}
