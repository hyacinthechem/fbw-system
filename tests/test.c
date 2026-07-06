#include <string.h>
#include <stdio.h>
#include "XPLMDefs.h"
#include "XPLMPlugin.h"
#include "XPLMDataAccess.h"
#include "XPLMProcessing.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"

// ─────────────────────────────────────────────
// STATE MACHINE
// Tracks where we are in the landing cycle
// ─────────────────────────────────────────────
typedef enum {
    STATE_AIRBORNE,     // aircraft is flying, watching for touchdown
    STATE_TOUCHDOWN,    // just landed, showing result on screen
    STATE_ON_GROUND     // taxiing, waiting for next takeoff
} LandingState;

static LandingState current_state = STATE_AIRBORNE;

// ─────────────────────────────────────────────
// DATAREF HANDLES
// These are references to X-Plane's internal variables
// Found via: developer.x-plane.com/datarefs
// We find them ONCE at startup and reuse the handle every frame
// ─────────────────────────────────────────────

// Vertical speed in feet per minute (negative = descending)
// Source: developer.x-plane.com/datarefs — sim/flightmodel/position/vh_ind_fpm
// Type: float — use XPLMGetDataf()
static XPLMDataRef vertical_speed_ref = NULL;

// 1 if any landing gear is touching the ground, 0 if airborne
// Source: developer.x-plane.com/datarefs — sim/flightmodel/failures/onground_any
// Type: int — use XPLMGetDatai()
static XPLMDataRef on_ground_ref = NULL;

// Indicated airspeed in knots
// Source: developer.x-plane.com/datarefs — sim/flightmodel/position/indicated_airspeed
// Type: float — use XPLMGetDataf()
static XPLMDataRef airspeed_knots_ref = NULL;

// G-force on the normal (vertical) axis — 1.0 is normal gravity
// Source: developer.x-plane.com/datarefs — sim/flightmodel/forces/g_nrml
// Type: float — use XPLMGetDataf()
static XPLMDataRef gforce_ref = NULL;

// ─────────────────────────────────────────────
// LANDING DATA
// Values captured at the moment of touchdown
// ─────────────────────────────────────────────

// Vertical speed captured at the exact moment of touchdown (feet per minute)
static float touchdown_vertical_speed = 0.0f;

// G-force captured at the exact moment of touchdown
static float touchdown_gforce = 0.0f;

// Counts down from 10 to 0 — how long to keep the result on screen
static float display_countdown_seconds = 0.0f;

// How many seconds to show the result after landing
#define RESULT_DISPLAY_DURATION 10.0f

// ─────────────────────────────────────────────
// WINDOW
// The floating window that shows the landing result
// Created in XPluginStart, shown/hidden as needed
// ─────────────────────────────────────────────
static XPLMWindowID result_window = NULL;

// ─────────────────────────────────────────────
// RATING HELPERS
// Takes the vertical speed at touchdown and returns
// a text rating and a colour
// ─────────────────────────────────────────────

// Returns a text rating based on touchdown vertical speed
// Thresholds based on real-world airline landing standards
static const char *get_landing_rating(float fpm)
{
    if (fpm > -60)   return "GREASER!";       // perfect — barely felt it
    if (fpm > -180)  return "EXCELLENT";      // smooth airline landing
    if (fpm > -300)  return "GOOD";           // acceptable
    if (fpm > -500)  return "HARD LANDING";   // passengers are complaining
    return "CRASH LANDING!";                  // structural damage territory
}

// Returns an RGB colour matching the rating
// red = bad, green = good, purple = perfect
static void get_rating_colour(float fpm,
                               float *red, float *green, float *blue)
{
    if (fpm > -60)  { *red=0.8f; *green=0.4f; *blue=1.0f; return; } // purple  — greaser
    if (fpm > -180) { *red=0.0f; *green=1.0f; *blue=0.4f; return; } // green   — excellent
    if (fpm > -300) { *red=1.0f; *green=1.0f; *blue=0.0f; return; } // yellow  — good
    if (fpm > -500) { *red=1.0f; *green=0.5f; *blue=0.0f; return; } // orange  — hard
                    { *red=1.0f; *green=0.1f; *blue=0.1f; return; } // red     — crash
}

// ─────────────────────────────────────────────
// FORWARD DECLARATIONS
// Needed because flight_loop and draw_window are
// defined after XPluginStart which references them
// ─────────────────────────────────────────────
static float flight_loop_callback(float, float, int, void *);
static void  draw_result_window(XPLMWindowID, void *);

// ─────────────────────────────────────────────
// XPLUGINSTART
// Called once by X-Plane when the plugin loads
// This is where we find datarefs and create the window
// Source: SDK/CHeaders/XPLM/XPLMPlugin.h
// ─────────────────────────────────────────────
PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc)
{
    // These three strings identify your plugin in X-Plane's plugin manager
    strcpy(outName, "Landing Rate Monitor");
    strcpy(outSig,  "com.yourname.landingrate");  // must be unique
    strcpy(outDesc, "Shows your landing rate at touchdown");

    // Find all datarefs by their string name
    // XPLMFindDataRef returns a handle — NOT the value
    // We call this once here and store the handle for later use
    // Source: SDK/CHeaders/XPLM/XPLMDataAccess.h — XPLMFindDataRef()
    vertical_speed_ref = XPLMFindDataRef("sim/flightmodel/position/vh_ind_fpm");
    on_ground_ref      = XPLMFindDataRef("sim/flightmodel/failures/onground_any");
    airspeed_knots_ref = XPLMFindDataRef("sim/flightmodel/position/indicated_airspeed");
    gforce_ref         = XPLMFindDataRef("sim/flightmodel/forces/g_nrml");

    // If any dataref wasn't found, bail out early
    // This would happen if the dataref name is wrong or X-Plane version is too old
    if (!vertical_speed_ref || !on_ground_ref ||
        !airspeed_knots_ref || !gforce_ref) {
        // XPLMDebugString writes to X-Plane 12/Log.txt
        // Source: SDK/CHeaders/XPLM/XPLMPlugin.h — XPLMDebugString()
        XPLMDebugString("LandingRate: ERROR — one or more datarefs not found\n");
        return 0; // 0 = failed to load, X-Plane will disable the plugin
    }

    // Create the floating result window
    // It starts hidden — we show it only when a landing is detected
    // Source: SDK/CHeaders/XPLM/XPLMDisplay.h — XPLMCreateWindowEx()
    XPLMCreateWindow_t window_params;
    memset(&window_params, 0, sizeof(window_params));
    window_params.structSize               = sizeof(window_params);
    window_params.left                     = 100;   // pixels from left
    window_params.top                      = 600;   // pixels from bottom
    window_params.right                    = 400;   // pixels from left
    window_params.bottom                   = 450;   // pixels from bottom
    window_params.visible                  = 0;     // hidden at startup
    window_params.drawWindowFunc           = draw_result_window;
    window_params.handleMouseClickFunc     = NULL;
    window_params.handleKeyFunc            = NULL;
    window_params.handleCursorFunc         = NULL;
    window_params.handleMouseWheelFunc     = NULL;
    window_params.refcon                   = NULL;
    window_params.layer                    = xplm_WindowLayerFloatingWindows;
    window_params.decorateAsFloatingWindow = xplm_WindowDecorationRoundRectangle;

    result_window = XPLMCreateWindowEx(&window_params);
    XPLMSetWindowPositioningMode(result_window, xplm_WindowPositionFree, -1);
    XPLMSetWindowTitle(result_window, "Landing Rate");

    XPLMDebugString("LandingRate: loaded successfully\n");
    return 1; // 1 = loaded successfully
}

// ─────────────────────────────────────────────
// XPLUGINENABLE
// Called when the plugin is enabled
// This is where we register our flight loop
// Source: SDK/CHeaders/XPLM/XPLMPlugin.h
// ─────────────────────────────────────────────
PLUGIN_API int XPluginEnable(void)
{
    // Register our flight loop to run every 0.1 seconds (100ms)
    // Source: SDK/CHeaders/XPLM/XPLMProcessing.h — XPLMRegisterFlightLoopCallback()
    XPLMRegisterFlightLoopCallback(flight_loop_callback, 0.1f, NULL);
    return 1;
}

// ─────────────────────────────────────────────
// XPLUGINDISABLE / XPLUGINSTOP
// Called when plugin is disabled or X-Plane quits
// Always clean up what you created
// ─────────────────────────────────────────────
PLUGIN_API void XPluginDisable(void)
{
    // Unregister the flight loop so it stops running
    XPLMUnregisterFlightLoopCallback(flight_loop_callback, NULL);
}

PLUGIN_API void XPluginStop(void)
{
    // Destroy the window to free memory
    if (result_window) {
        XPLMDestroyWindow(result_window);
        result_window = NULL;
    }
}

// Required by the SDK but we don't need it for this plugin
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID from_plugin,
                                       int message, void *param) {}

// ─────────────────────────────────────────────
// FLIGHT LOOP CALLBACK
// X-Plane calls this every 100ms
// This is where we read datarefs and detect touchdown
// Source: SDK/CHeaders/XPLM/XPLMProcessing.h
// ─────────────────────────────────────────────
static float flight_loop_callback(float seconds_since_last_call,
                                   float seconds_since_last_loop,
                                   int counter, void *refcon)
{
    // Read current values from X-Plane's sim state
    // We use the handles found in XPluginStart — NOT XPLMFindDataRef again
    // Source: SDK/CHeaders/XPLM/XPLMDataAccess.h

    // vertical speed in feet per minute (negative = descending)
    float current_vertical_speed = XPLMGetDataf(vertical_speed_ref);

    // 1 if any gear is on the ground, 0 if airborne
    int gear_is_on_ground = XPLMGetDatai(on_ground_ref);

    // current airspeed in knots
    float current_airspeed = XPLMGetDataf(airspeed_knots_ref);

    // current g-force (1.0 = normal, higher = harder impact)
    float current_gforce = XPLMGetDataf(gforce_ref);

    switch (current_state) {

        case STATE_AIRBORNE:
            // Touchdown = gear just hit the ground AND we were moving fast enough
            // The airspeed > 40 check prevents triggering while taxiing
            if (gear_is_on_ground && current_airspeed > 40.0f) {

                // Capture the values at this exact moment
                touchdown_vertical_speed  = current_vertical_speed;
                touchdown_gforce          = current_gforce;
                display_countdown_seconds = RESULT_DISPLAY_DURATION;
                current_state             = STATE_TOUCHDOWN;

                // Show the result window
                // Source: SDK/CHeaders/XPLM/XPLMDisplay.h — XPLMSetWindowIsVisible()
                XPLMSetWindowIsVisible(result_window, 1);

                // Log the result to X-Plane 12/Log.txt for debugging
                char log_message[128];
                snprintf(log_message, sizeof(log_message),
                         "LandingRate: %.0f fpm | G-Force: %.2f | %s\n",
                         touchdown_vertical_speed,
                         touchdown_gforce,
                         get_landing_rating(touchdown_vertical_speed));
                XPLMDebugString(log_message);
            }
            break;

        case STATE_TOUCHDOWN:
            // Count down the display timer each frame
            display_countdown_seconds -= seconds_since_last_call;

            // When timer runs out hide the window and move to on-ground state
            if (display_countdown_seconds <= 0.0f) {
                XPLMSetWindowIsVisible(result_window, 0);
                current_state = STATE_ON_GROUND;
            }
            break;

        case STATE_ON_GROUND:
            // Wait until we're airborne again before resetting
            // airspeed > 60 confirms we actually took off, not just taxiing fast
            if (!gear_is_on_ground && current_airspeed > 60.0f)
                current_state = STATE_AIRBORNE;
            break;
    }

    return 0.1f; // tell X-Plane to call us again in 100ms
}

// ─────────────────────────────────────────────
// DRAW RESULT WINDOW
// X-Plane calls this every frame while the window is visible
// This is where we draw the landing result on screen
// Source: SDK/CHeaders/XPLM/XPLMDisplay.h
//         SDK/CHeaders/XPLM/XPLMGraphics.h
// ─────────────────────────────────────────────
static void draw_result_window(XPLMWindowID window_id, void *refcon)
{
    // Get the window's current position on screen
    // Source: SDK/CHeaders/XPLM/XPLMDisplay.h — XPLMGetWindowGeometry()
    int left, top, right, bottom;
    XPLMGetWindowGeometry(window_id, &left, &top, &right, &bottom);

    // Get the colour for this landing rating
    float red, green, blue;
    get_rating_colour(touchdown_vertical_speed, &red, &green, &blue);

    // White for secondary info lines
    float white[3]        = {1.0f, 1.0f, 1.0f};
    float rating_color[3] = {red, green, blue};

    // Line 1 — vertical speed at touchdown
    char fpm_line[64];
    snprintf(fpm_line, sizeof(fpm_line),
             "%.0f ft/min", touchdown_vertical_speed);

    // Source: SDK/CHeaders/XPLM/XPLMGraphics.h — XPLMDrawString()
    XPLMDrawString(rating_color,
                   left + 15, top - 30,
                   fpm_line, NULL, xplmFont_Proportional);

    // Line 2 — rating text
    char rating_line[32];
    snprintf(rating_line, sizeof(rating_line),
             "%s", get_landing_rating(touchdown_vertical_speed));
    XPLMDrawString(rating_color,
                   left + 15, top - 55,
                   rating_line, NULL, xplmFont_Proportional);

    // Line 3 — g-force at touchdown
    char gforce_line[64];
    snprintf(gforce_line, sizeof(gforce_line),
             "G-Force: %.2f", touchdown_gforce);
    XPLMDrawString(white,
                   left + 15, top - 80,
                   gforce_line, NULL, xplmFont_Proportional);

    // Line 4 — countdown until window closes
    char countdown_line[32];
    snprintf(countdown_line, sizeof(countdown_line),
             "Closing in %.0fs", display_countdown_seconds);
    XPLMDrawString(white,
                   left + 15, top - 105,
                   countdown_line, NULL, xplmFont_Proportional);
}
