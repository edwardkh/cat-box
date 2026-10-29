// settings merge/getter parity (persistent_state/state.py merge_state +
// get_state_of defaults) and the alert counter quirk (`>` comparison).
#include <unity.h>

#include <string.h>

#include "../src/core/settings_core.h"
#include "../src/alert_stub.h"
#include "../src/settings.h"

static SettingsStore store;
static char g_saved[4096];
static bool g_has_saved;
static char g_tmp[64];
static int g_persist_calls;

static int fake_load(char* buf, size_t buflen) {
    if (!g_has_saved) return SETTINGS_LOAD_ABSENT;
    strncpy(buf, g_saved, buflen - 1);
    buf[buflen - 1] = '\0';
    return SETTINGS_LOAD_OK;
}
static bool fake_persist(const char* json) {
    strncpy(g_saved, json, sizeof(g_saved) - 1);
    g_saved[sizeof(g_saved) - 1] = '\0';
    g_persist_calls++;
    return true;
}

static void fresh_store(const char* initial) {
    settings_init(&store, fake_load, fake_persist);
    settings_load(store, initial);
    g_has_saved = false;
    g_saved[0] = '\0';
    g_persist_calls = 0;
}

static void test_missing_file_behaves_as_empty_object(void) {
    fresh_store("");
    char buf[256];
    settings_dump(store, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("{}", buf);
    TEST_ASSERT_EQUAL_INT(7, settings_get_i("absent", 7));
}

static void test_get_state_of_default_fallback(void) {
    fresh_store("{\"load_sensor_threshold\": 6000}");
    TEST_ASSERT_EQUAL_INT(6000, settings_get_i("load_sensor_threshold", 1234));
    TEST_ASSERT_EQUAL_INT(24, settings_get_i("timed_cycle_delay_hours", 24));  // default path
    TEST_ASSERT_EQUAL_STRING("catbox",
                             settings_get_s("wifi_hostname", "catbox", g_tmp, sizeof(g_tmp)));
}

static void test_shallow_merge_unknown_keys_preserved(void) {
    fresh_store("{\"a\":1,\"keep\":\"me\"}");
    TEST_ASSERT_TRUE(settings_apply_patch(store, "{\"a\":2,\"b\":3}"));
    TEST_ASSERT_EQUAL_INT(2, settings_get_i("a", 0));
    TEST_ASSERT_EQUAL_INT(3, settings_get_i("b", 0));
    TEST_ASSERT_EQUAL_STRING("me", settings_get_s("keep", "?", g_tmp, sizeof(g_tmp)));
    char buf[256];
    settings_dump(store, buf, sizeof(buf));
    // compact (ujson parity: no spaces), base order first then appended keys
    TEST_ASSERT_EQUAL_STRING("{\"a\":2,\"keep\":\"me\",\"b\":3}", buf);
}

static void test_invalid_patch_leaves_state(void) {
    fresh_store("{\"a\":1}");
    TEST_ASSERT_FALSE(settings_apply_patch(store, "{not json"));
    TEST_ASSERT_EQUAL_INT(1, settings_get_i("a", 0));
    TEST_ASSERT_FALSE(settings_apply_patch(store, ""));
    TEST_ASSERT_FALSE(settings_apply_patch(store, "[1,2]"));  // non-object
    TEST_ASSERT_EQUAL_INT(1, settings_get_i("a", 0));
}

static void test_merge_and_save_persists(void) {
    fresh_store("{\"a\":1}");
    g_has_saved = true;
    strcpy(g_saved, "{\"a\":1}");
    TEST_ASSERT_TRUE(settings_merge_and_save("{\"a\":9}"));
    TEST_ASSERT_EQUAL_STRING("{\"a\":9}", g_saved);
    // invalid patch: nothing written
    char before[2048];
    strcpy(before, g_saved);
    TEST_ASSERT_FALSE(settings_merge_and_save("garbage{"));
    TEST_ASSERT_EQUAL_STRING(before, g_saved);
}

static void test_increment_persists(void) {
    fresh_store("{}");
    g_has_saved = true;
    TEST_ASSERT_EQUAL_INT(1, settings_increment_and_save("cycles"));
    TEST_ASSERT_EQUAL_INT(2, settings_increment_and_save("cycles"));
    TEST_ASSERT_EQUAL_STRING("{\"cycles\":2}", g_saved);
}

static void test_alert_threshold_comparison_strictly_greater(void) {
    // Python: if cycles > get_state_of("alert_bin_full_cycles", get_bin_full_cycles())
    fresh_store("{\"bin_full_cycles\":2}");
    g_has_saved = true;
    alert_reset();
    TEST_ASSERT_FALSE(alert_react_to_cycle());  // 1 > 2 false
    TEST_ASSERT_FALSE(alert_react_to_cycle());  // 2 > 2 false (NOT >=)
    TEST_ASSERT_TRUE(alert_react_to_cycle());   // 3 > 2 true
    TEST_ASSERT_EQUAL_INT(3, alert_current_cycles());
    alert_reset();
    TEST_ASSERT_EQUAL_INT(0, alert_current_cycles());
}

static void test_alert_override_key_wins(void) {
    fresh_store("{\"bin_full_cycles\":5,\"alert_bin_full_cycles\":1}");
    g_has_saved = true;
    alert_reset();
    TEST_ASSERT_FALSE(alert_react_to_cycle());  // 1 > 1 false
    TEST_ASSERT_TRUE(alert_react_to_cycle());   // 2 > 1 true (override key)
}

static void test_alert_message_text(void) {
    char m[256];
    alert_build_message(m, sizeof(m), "http://192.168.1.5", 21);
    TEST_ASSERT_EQUAL_STRING(
        "Empty the cat box. It has cycled 21 times. Reset with http://192.168.1.5/reset", m);
}

static void test_float_and_bool_getters(void) {
    fresh_store("{\"loop_sleep\":0.5,\"rotate_direction_reversed\":true}");
    TEST_ASSERT_EQUAL_DOUBLE(0.5, settings_get_f("loop_sleep", 0.25));
    TEST_ASSERT_TRUE(settings_get_b("rotate_direction_reversed", false));
    TEST_ASSERT_FALSE(settings_get_b("absent_bool", false));
    TEST_ASSERT_EQUAL_DOUBLE(0.25, settings_get_f("absent", 0.25));
    // int-typed value readable as float (ujson made no distinction)
    fresh_store("{\"cycle_eating_time\":7}");
    TEST_ASSERT_EQUAL_DOUBLE(7.0, settings_get_f("cycle_eating_time", 0));
}

// ---- settings_dump fit contract (truncation-safety fix) ----

static void test_dump_reports_overflow_and_refuses_partial(void) {
    fresh_store("{}");
    char big[300];
    memset(big, 'x', sizeof(big) - 1);
    big[sizeof(big) - 1] = '\0';
    char patch[400];
    snprintf(patch, sizeof(patch), "{\"pad\":\"%s\"}", big);
    TEST_ASSERT_TRUE(settings_apply_patch(store, patch));
    char small[64];
    // On overflow the return is false; `small` may contain partial garbage
    // (serializeJson streams into the buffer) and MUST be discarded — the
    // persist wrapper is the layer that refuses it (see below).
    TEST_ASSERT_FALSE(settings_dump(store, small, sizeof(small)));
    (void)small;
    char roomy[512];
    TEST_ASSERT_TRUE(settings_dump(store, roomy, sizeof(roomy)));
    TEST_ASSERT_EQUAL_STRING_LEN("{\"pad\":\"xxx", roomy, 10);
}

static void test_load_distinguishes_absent_and_corrupt(void) {
    fresh_store("{}");
    TEST_ASSERT_EQUAL_INT(SETTINGS_LOAD_ABSENT, settings_load(store, ""));
    TEST_ASSERT_EQUAL_INT(SETTINGS_LOAD_CORRUPT, settings_load(store, "{truncated"));
    TEST_ASSERT_EQUAL_INT(SETTINGS_LOAD_CORRUPT, settings_load(store, "[1,2]"));  // non-object
    TEST_ASSERT_EQUAL_INT(SETTINGS_LOAD_OK, settings_load(store, "{\"a\":1}"));
    // corrupt still behaves as {} in RAM (Python parity for reads)
    settings_load(store, "{truncated");
    TEST_ASSERT_EQUAL_INT(7, settings_get_i("anything", 7));
}

// A corrupt-but-present file must never be auto-overwritten: the wrapper
// preserves it for user recovery (data-loss fix).
static bool counting_persist(const char* json) {
    (void)json;
    g_persist_calls++;
    return true;
}
static int big_file_load(char* buf, size_t buflen) {
    // simulates fs_load on a file larger than the read buffer
    (void)buf;
    (void)buflen;
    return SETTINGS_LOAD_CORRUPT;
}
static int ok_file_load(char* buf, size_t buflen) {
    strncpy(buf, "{\"keep\":1}", buflen - 1);
    buf[buflen - 1] = '\0';
    return SETTINGS_LOAD_OK;
}

static void test_corrupt_file_is_preserved_not_overwritten(void) {
    settings_init(&store, big_file_load, counting_persist);
    settings_begin();
    TEST_ASSERT_TRUE(settings_file_broken());
    // run-from-defaults in RAM
    char g2[32];
    TEST_ASSERT_EQUAL_STRING("catbox",
                             settings_get_s("wifi_hostname", "catbox", g2, sizeof(g2)));
    // every persistence path must be refused while broken
    TEST_ASSERT_EQUAL_INT(1, settings_increment_and_save("cycles"));  // in-RAM only
    TEST_ASSERT_TRUE(settings_merge_and_save("{\"a\":1}"));
    settings_set_i_and_save("cycles", 0);
    TEST_ASSERT_EQUAL_INT(0, g_persist_calls);  // ZERO writes to the broken file
}

static void test_valid_file_clears_broken_flag(void) {
    settings_init(&store, ok_file_load, counting_persist);
    settings_begin();
    TEST_ASSERT_FALSE(settings_file_broken());
    TEST_ASSERT_EQUAL_INT(1, settings_get_i("keep", 0));
    settings_increment_and_save("cycles");
    TEST_ASSERT_EQUAL_INT(1, g_persist_calls);  // writes allowed again
}


int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_missing_file_behaves_as_empty_object);
    RUN_TEST(test_get_state_of_default_fallback);
    RUN_TEST(test_shallow_merge_unknown_keys_preserved);
    RUN_TEST(test_invalid_patch_leaves_state);
    RUN_TEST(test_merge_and_save_persists);
    RUN_TEST(test_increment_persists);
    RUN_TEST(test_alert_threshold_comparison_strictly_greater);
    RUN_TEST(test_alert_override_key_wins);
    RUN_TEST(test_alert_message_text);
    RUN_TEST(test_float_and_bool_getters);
    RUN_TEST(test_dump_reports_overflow_and_refuses_partial);
    RUN_TEST(test_load_distinguishes_absent_and_corrupt);
    RUN_TEST(test_corrupt_file_is_preserved_not_overwritten);
    RUN_TEST(test_valid_file_clears_broken_flag);
    return UNITY_END();
}
