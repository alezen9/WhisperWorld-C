#include "list.h"
#include <check.h>
#include <string.h>
#include <time.h>

START_TEST(test_list_append) {
    struct List list = {NULL, NULL};

    struct Message msg1, msg2;
    strcpy(msg1.user_name, "User 1");
    strcpy(msg1.content, "Message 1");
    msg1.timestamp = (time_t)111111;

    strcpy(msg2.user_name, "User 2");
    strcpy(msg2.content, "Message 2");
    msg2.timestamp = (time_t)222222;

    list_append(&msg1, &list);

    ck_assert_ptr_nonnull(list.head);
    ck_assert_ptr_eq(list.head, list.tail);

    list_append(&msg2, &list);

    ck_assert_ptr_nonnull(list.head);
    ck_assert_ptr_ne(list.head, list.tail);
    ck_assert_str_eq(list.head->user_name, "User 1");
    ck_assert_str_eq(list.head->content, "Message 1");
    ck_assert_int_eq(list.head->timestamp, 111111);

    ck_assert_str_eq(list.tail->user_name, "User 2");
    ck_assert_str_eq(list.tail->content, "Message 2");
    ck_assert_int_eq(list.tail->timestamp, 222222);
}
END_TEST

Suite *list_suite(void) {
    Suite *s = suite_create("List");
    TCase *tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_list_append);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void) {
    int number_failed;
    Suite *s = list_suite();
    SRunner *sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? 0 : 1;
}
