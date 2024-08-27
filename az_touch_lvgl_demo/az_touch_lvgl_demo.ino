#include <lvgl.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#define XPT2046_MOSI 23  // gleich wie TFT
#define XPT2046_MISO 19  // gleich wie TFT
#define XPT2046_CLK 18   // gleich wie TFT
#define XPT2046_IRQ 27   // T_IRQ
#define XPT2046_CS 14    // T_CS
#define TOUCH_ROTATION 1

SPIClass touchscreenSPI = SPIClass(HSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

// Debounce Zeit für Touch-Events (in Millisekunden)
#define TOUCH_DEBOUNCE_MS 50
unsigned long last_touch_time = 0;

// Callback für serielle Logausgaben
void log_print(lv_log_level_t level, const char* buf) {
    LV_UNUSED(level);
    Serial.println(buf);
    Serial.flush();
}

// Funktion zum Lesen des Touchscreen-Eingangs
void touchscreen_read(lv_indev_t* indev, lv_indev_data_t* data) {
    unsigned long current_time = millis();
    if ((current_time - last_touch_time) >= TOUCH_DEBOUNCE_MS) {
        last_touch_time = current_time;

        if (touchscreen.tirqTouched() && touchscreen.touched()) {
            TS_Point p = touchscreen.getPoint();
            int x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
            int y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
            data->state = LV_INDEV_STATE_PRESSED;
            data->point.x = x;
            data->point.y = y;
            Serial.print("Touch detected at: ");
            Serial.print(x);
            Serial.print(", ");
            Serial.println(y);
        } else {
            data->state = LV_INDEV_STATE_RELEASED;
        }
    }
}

// Tab 1: Demonstration von Buttons und Labels
void create_tab1(lv_obj_t* parent) {
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, "Dies ist ein Label");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t* btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Klick mich");

    // Button-Event-Callback
    lv_obj_add_event_cb(btn, [](lv_event_t* e) {
        Serial.println("Button in Tab 1 clicked");
    }, LV_EVENT_CLICKED, NULL);
}

// Slider-Wert-Anzeige aktualisieren
static void slider_event_cb(lv_event_t* e) {
    // Expliziter Cast von void* zu lv_obj_t*
    lv_obj_t* slider = (lv_obj_t*)lv_event_get_target(e);
    lv_obj_t* label = (lv_obj_t*)lv_event_get_user_data(e);

    // Slider-Wert abrufen und anzeigen
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d", lv_slider_get_value(slider));
    lv_label_set_text(label, buf);
    lv_obj_align_to(label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    Serial.print("Slider value: ");
    Serial.println(buf);
}

// Tab 2: Demonstration von Slidern und Switches
void create_tab2(lv_obj_t* parent) {
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, "Dies ist ein Slider");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t* slider = lv_slider_create(parent);
    lv_obj_align(slider, LV_ALIGN_CENTER, 0, -20);
    lv_obj_set_width(slider, 200);

    // Slider-Wert-Label
    lv_obj_t* slider_label = lv_label_create(parent);
    lv_label_set_text(slider_label, "0");
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    // Event-Callback für den Slider zur Wertanzeige
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, slider_label);

    lv_obj_t* switch_label = lv_label_create(parent);
    lv_label_set_text(switch_label, "Dies ist ein Switch");
    lv_obj_align(switch_label, LV_ALIGN_CENTER, 0, 40);

    lv_obj_t* sw = lv_switch_create(parent);
    lv_obj_align(sw, LV_ALIGN_CENTER, 0, 80);

    // Switch-Event-Callback
    lv_obj_add_event_cb(sw, [](lv_event_t* e) {
        // Expliziter Cast von void* zu lv_obj_t*
        lv_obj_t* sw = (lv_obj_t*)lv_event_get_target(e);
        if (lv_obj_has_state(sw, LV_STATE_CHECKED)) {
            Serial.println("Switch turned ON");
        } else {
            Serial.println("Switch turned OFF");
        }
    }, LV_EVENT_VALUE_CHANGED, NULL);
}

// Tab 3: Demonstration von Dropdown-Menüs und Checkboxes
void create_tab3(lv_obj_t* parent) {
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, "Dropdown-Menu");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t* dropdown = lv_dropdown_create(parent);
    lv_dropdown_set_options(dropdown, "Option 1\nOption 2\nOption 3");
    lv_obj_align(dropdown, LV_ALIGN_CENTER, 0, -30);

    // Dropdown-Event-Callback
    lv_obj_add_event_cb(dropdown, [](lv_event_t* e) {
        Serial.println("Dropdown option selected");
    }, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t* cb1 = lv_checkbox_create(parent);
    lv_checkbox_set_text(cb1, "Checkbox 1");
    lv_obj_align(cb1, LV_ALIGN_CENTER, 0, 10);

    lv_obj_t* cb2 = lv_checkbox_create(parent);
    lv_checkbox_set_text(cb2, "Checkbox 2");
    lv_obj_align(cb2, LV_ALIGN_CENTER, 0, 40);

    // Checkbox-Event-Callback
    lv_obj_add_event_cb(cb1, [](lv_event_t* e) {
        Serial.println("Checkbox 1 toggled");
    }, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_add_event_cb(cb2, [](lv_event_t* e) {
        Serial.println("Checkbox 2 toggled");
    }, LV_EVENT_VALUE_CHANGED, NULL);
}

// Tab 4: Demonstration von Fortschrittsanzeigen
void create_tab4(lv_obj_t* parent) {
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, "Fortschrittsanzeige");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t* prog = lv_bar_create(parent);
    lv_obj_set_size(prog, 200, 20);
    lv_obj_align(prog, LV_ALIGN_CENTER, 0, -30);
    lv_bar_set_value(prog, 50, LV_ANIM_OFF);  // Setze den Fortschrittswert

    // Fortschrittsanzeige aktualisieren
    lv_obj_add_event_cb(prog, [](lv_event_t* e) {
        static int value = 0;
        value += 10;
        if (value > 100) value = 0;
        lv_bar_set_value((lv_obj_t*)lv_event_get_target(e), value, LV_ANIM_OFF);
        Serial.print("Progress value: ");
        Serial.println(value);
    }, LV_EVENT_CLICKED, NULL);
}

// Tab 5: Demonstration von Tabellen
void create_tab5(lv_obj_t* parent) {
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, "Tabelle");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t* table = lv_table_create(parent);
    lv_obj_set_size(table, 200, 150);
    lv_obj_align(table, LV_ALIGN_CENTER, 0, -20);
    lv_table_set_col_cnt(table, 3);
    lv_table_set_row_cnt(table, 4);

    // Tabellenkopfzeilen hinzufügen
    lv_table_set_cell_value(table, 0, 0, "Header 1");
    lv_table_set_cell_value(table, 0, 1, "Header 2");
    lv_table_set_cell_value(table, 0, 2, "Header 3");

    // Tabelleninhalte hinzufügen
    for (int row = 1; row < 4; row++) {
        for (int col = 0; col < 3; col++) {
            char buf[32];
            snprintf(buf, sizeof(buf), "Row %d, Col %d", row, col);
            lv_table_set_cell_value(table, row, col, buf);
        }
    }
}

// Tab 6: Demonstration von Textfeldern
void create_tab6(lv_obj_t* parent) {
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, "Textfeld");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t* ta = lv_textarea_create(parent);
    lv_obj_set_size(ta, 200, 100);
    lv_obj_align(ta, LV_ALIGN_CENTER, 0, 0);
    lv_textarea_set_text(ta, "Hier ist ein Textfeld. Du kannst hier Text eingeben oder anzeigen.");
}

// Tab 7: Demonstration von Fortschrittsbalken
void create_tab7(lv_obj_t* parent) {
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, "Fortschrittsbalken");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t* bar = lv_bar_create(parent);
    lv_obj_set_size(bar, 200, 20);
    lv_obj_align(bar, LV_ALIGN_CENTER, 0, 0);
    lv_bar_set_value(bar, 50, LV_ANIM_OFF);  // Setze den Fortschrittswert auf 50%

    // Fortschrittsbalken aktualisieren
    lv_obj_add_event_cb(bar, [](lv_event_t* e) {
        static int value = 0;
        value += 10;
        if (value > 100) value = 0;
        lv_bar_set_value((lv_obj_t*)lv_event_get_target(e), value, LV_ANIM_OFF);
        Serial.print("Progress value: ");
        Serial.println(value);
    }, LV_EVENT_CLICKED, NULL);
}


// Haupt-GUI erstellen
void lv_create_main_gui(void) {
    lv_obj_t* tabview = lv_tabview_create(lv_scr_act());

    lv_obj_t* tab1 = lv_tabview_add_tab(tabview, "Tab 1");
    lv_obj_t* tab2 = lv_tabview_add_tab(tabview, "Tab 2");
    lv_obj_t* tab3 = lv_tabview_add_tab(tabview, "Tab 3");
    lv_obj_t* tab4 = lv_tabview_add_tab(tabview, "Tab 4");
    lv_obj_t* tab5 = lv_tabview_add_tab(tabview, "Tab 5");
    lv_obj_t* tab6 = lv_tabview_add_tab(tabview, "Tab 6");    
    lv_obj_t* tab7 = lv_tabview_add_tab(tabview, "Tab 7");     

    create_tab1(tab1);
    create_tab2(tab2);
    create_tab3(tab3);
    create_tab4(tab4);
    create_tab5(tab5);
    create_tab6(tab6);    
    create_tab7(tab7);      
}

void setup() {
    Serial.begin(115200);
    Serial.println("Setup started");

    lv_init();
    lv_log_register_print_cb(log_print);

    touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    touchscreen.begin(touchscreenSPI);
    touchscreen.setRotation(TOUCH_ROTATION);

    lv_disp_t* disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
    Serial.println("Display initialized");

    lv_indev_t* indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touchscreen_read);
    Serial.println("Touchscreen initialized");

    lv_create_main_gui();
    Serial.println("GUI created");
}

void loop() {
    lv_task_handler();
    lv_tick_inc(5);
    delay(5);
}