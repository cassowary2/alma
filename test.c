#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include "alma.h"
#include "ast.h"
#include "ustrings.h"
#include "eval.h"
#include "scope.h"
#include "lib.h"
#include "parse.h"
#include "compile.h"
#include "registry.h"
#include "grammar.tab.h"

#define ALMATESTINTRO(filename) \
    FILE *in = fopen(filename, "r"); \
    ADeclSeqNode *program = NULL; \
    ASymbolTable symtab = NULL; \
    AStack *stack = stack_new(20); \
    AScope *lib_scope = scope_new(NULL); \
    AScope *scope = scope_new(lib_scope); \
    AFuncRegistry *reg = registry_new(20); \
    parse_file(in, &program, &symtab); \
    lib_init(symtab, lib_scope); \
    ABindInfo bi = {0,0};

#define ALMATESTCLEAN() \
    free_stack(stack); \
    free_registry(reg); \
    free_decl_seq(program); \
    free_scope(scope); \
    free_lib_scope(lib_scope); \
    free_symbol_table(&symtab)

int ustr_check(AUstr *ustr, const char *test) {
    AUstr *tester = parse_string(test, strlen(test));
    int result = ustr_eq(ustr, tester);
    free_ustring(tester);
    return result;
}

START_TEST(test_stack_push) {
    ALMATESTINTRO("tests/simplepush.alma");

    ck_assert(program->first != NULL);

    ACompileStatus stat = compile(scope, reg, program, bi);
    ck_assert_int_eq(stat, compile_success);

    AFunc *mainfunc = scope_find_func(scope, symtab, "main");
    ck_assert(mainfunc != NULL);
    eval_word(stack, NULL, mainfunc);

    ck_assert_int_eq(stack->size, 5);
    ck_assert(stack_peek(stack, 0)->type == str_val);
    ck_assert(ustr_check(stack_peek(stack, 0)->data.str, "hello world"));
    ck_assert(stack_peek(stack, 1)->type == int_val);
    ck_assert(stack_peek(stack, 1)->data.i == 1);
    ck_assert(stack_peek(stack, 2)->type == int_val);
    ck_assert(stack_peek(stack, 2)->data.i == 2);
    ck_assert(stack_peek(stack, 3)->type == int_val);
    ck_assert(stack_peek(stack, 3)->data.i == 3);
    ck_assert(stack_peek(stack, 4)->type == int_val);
    ck_assert(stack_peek(stack, 4)->data.i == 4);

    ALMATESTCLEAN();
} END_TEST

START_TEST(test_stack_pop_print) {
    ALMATESTINTRO("tests/simplepop.alma");

    ACompileStatus stat = compile(scope, reg, program, bi);
    ck_assert_int_eq(stat, compile_success);

    printf("\nThe next thing printed should be ‘hi4’.\n");

    AFunc *mainfunc = scope_find_func(scope, symtab, "main");
    ck_assert(mainfunc != NULL);
    eval_word(stack, NULL, mainfunc);

    ck_assert_int_eq(stack->size, 0);

    ALMATESTCLEAN();
} END_TEST

START_TEST(test_addition) {
    ALMATESTINTRO("tests/basicmath.alma");

    ACompileStatus stat = compile(scope, reg, program, bi);
    ck_assert_int_eq(stat, compile_success);

    AFunc *mainfunc = scope_find_func(scope, symtab, "main");
    ck_assert(mainfunc != NULL);
    eval_word(stack, NULL, mainfunc);

    ck_assert_int_eq(stack->size, 1);
    ck_assert_int_eq(stack_peek(stack, 0)->data.i, 9);

    ALMATESTCLEAN();
} END_TEST

START_TEST(test_apply) {
    ALMATESTINTRO("tests/applyfunc.alma");

    ACompileStatus stat = compile(scope, reg, program, bi);
    ck_assert_int_eq(stat, compile_success);

    AFunc *mainfunc = scope_find_func(scope, symtab, "main");
    ck_assert(mainfunc != NULL);
    eval_word(stack, NULL, mainfunc);

    ck_assert_int_eq(stack->size, 1);
    ck_assert_int_eq(stack_peek(stack, 0)->data.i, 9);

    ALMATESTCLEAN();
} END_TEST

START_TEST(test_duplicate_func_error) {
    ALMATESTINTRO("tests/dupfunc.alma");

    printf("\nThe next thing printed should be an error message.\n");

    ACompileStatus stat = compile(scope, reg, program, bi);
    /* Compilation should fail due to duplicate function name. */
    ck_assert_int_eq(stat, compile_fail);

    ALMATESTCLEAN();
} END_TEST

START_TEST(test_unknown_func_error) {
    ALMATESTINTRO("tests/unknownfunc.alma");

    printf("\nThe next thing printed should be an error message.\n");

    ACompileStatus stat = compile(scope, reg, program, bi);
    /* Compilation should fail due to unknown function name. */
    ck_assert_int_eq(stat, compile_fail);

    ALMATESTCLEAN();
} END_TEST

START_TEST(test_definition) {
    ALMATESTINTRO("tests/definition.alma");

    ACompileStatus stat = compile(scope, reg, program, bi);
    ck_assert_int_eq(stat, compile_success);

    eval_sequence(stack, NULL, program->first->node);

    ck_assert_int_eq(stack->size, 1);
    ck_assert_int_eq(stack_peek(stack, 0)->data.i, 24);

    ALMATESTCLEAN();
} END_TEST

START_TEST(test_let) {
    ALMATESTINTRO("tests/let.alma");

    ACompileStatus stat = compile(scope, reg, program, bi);
    ck_assert_int_eq(stat, compile_success);

    eval_sequence(stack, NULL, program->first->node);

    ck_assert_int_eq(stack->size, 1);
    ck_assert_int_eq(stack_peek(stack, 0)->data.i, 12);

    ALMATESTCLEAN();
} END_TEST

START_TEST(test_2let) {
    ALMATESTINTRO("tests/doublelet.alma");

    ACompileStatus stat = compile(scope, reg, program, bi);
    ck_assert_int_eq(stat, compile_success);

    eval_sequence(stack, NULL, program->first->node);

    ck_assert_int_eq(stack->size, 1);
    ck_assert_int_eq(stack_peek(stack, 0)->data.i, 18);

    ALMATESTCLEAN();
} END_TEST

START_TEST(test_bind) {
    ALMATESTINTRO("tests/bind.alma");

    ACompileStatus stat = compile(scope, reg, program, bi);
    ck_assert_int_eq(stat, compile_success);

    AFunc *mainfunc = scope_find_func(scope, symtab, "main");

    ck_assert(mainfunc != NULL);

    eval_word(stack, NULL, mainfunc);

    ck_assert_int_eq(stack->size, 1);
    ck_assert_int_eq(stack_peek(stack, 0)->data.i, 12);

    ALMATESTCLEAN();
} END_TEST

START_TEST(test_blockparam) {
    ALMATESTINTRO("tests/bindnode.alma");

    ACompileStatus stat = compile(scope, reg, program, bi);
    ck_assert_int_eq(stat, compile_success);

    AFunc *mainfunc = scope_find_func(scope, symtab, "main");

    ck_assert(mainfunc != NULL);

    eval_word(stack, NULL, mainfunc);

    ck_assert_int_eq(stack->size, 1);
    ck_assert_int_eq(stack_peek(stack, 0)->data.i, 9);

    ALMATESTCLEAN();
} END_TEST

START_TEST(test_2bind) {
    ALMATESTINTRO("tests/doublebind.alma");

    ACompileStatus stat = compile(scope, reg, program, bi);
    ck_assert_int_eq(stat, compile_success);

    AFunc *mainfunc = scope_find_func(scope, symtab, "main");

    ck_assert(mainfunc != NULL);

    eval_word(stack, NULL, mainfunc);

    ck_assert_int_eq(stack->size, 2);
    ck_assert_int_eq(stack_peek(stack, 0)->data.i, 20);
    ck_assert_int_eq(stack_peek(stack, 1)->data.i, 24);

    ALMATESTCLEAN();
} END_TEST

START_TEST(test_funcargs) {
    ALMATESTINTRO("tests/funcargs.alma");

    ACompileStatus stat = compile(scope, reg, program, bi);
    ck_assert_int_eq(stat, compile_success);

    AFunc *mainfunc = scope_find_func(scope, symtab, "main");

    ck_assert(mainfunc != NULL);

    eval_word(stack, NULL, mainfunc);

    ck_assert_int_eq(stack->size, 2);
    ck_assert_int_eq(stack_peek(stack, 0)->data.i, 30);
    ck_assert_int_eq(stack_peek(stack, 1)->data.i, 9);

    ALMATESTCLEAN();
} END_TEST

START_TEST(test_closure) {
    ALMATESTINTRO("tests/closure.alma");

    ACompileStatus stat = compile(scope, reg, program, bi);
    ck_assert_int_eq(stat, compile_success);

    AFunc *mainfunc = scope_find_func(scope, symtab, "main");

    ck_assert(mainfunc != NULL);

    eval_word(stack, NULL, mainfunc);

    ck_assert_int_eq(stack->size, 4);
    ck_assert_int_eq(stack_peek(stack, 0)->data.i, 50);
    ck_assert_int_eq(stack_peek(stack, 1)->data.i, 42);
    ck_assert_int_eq(stack_peek(stack, 2)->data.i, 15);
    ck_assert_int_eq(stack_peek(stack, 3)->data.i, 30);

    ALMATESTCLEAN();
} END_TEST

START_TEST(test_multibuf) {
    ALMATESTINTRO("tests/multibuf.alma");

    ACompileStatus stat = compile(scope, reg, program, bi);
    ck_assert_int_eq(stat, compile_success);

    AFunc *mainfunc = scope_find_func(scope, symtab, "main");

    ck_assert(mainfunc != NULL);

    eval_word(stack, NULL, mainfunc);

    ck_assert_int_eq(stack->size, 1);
    ck_assert_int_eq(stack_peek(stack, 0)->data.i, 12);

    ALMATESTCLEAN();
} END_TEST

START_TEST(test_namedclosure) {
    ALMATESTINTRO("tests/namedclosure.alma");

    ACompileStatus stat = compile(scope, reg, program, bi);
    ck_assert_int_eq(stat, compile_success);

    AFunc *mainfunc = scope_find_func(scope, symtab, "main");

    ck_assert(mainfunc != NULL);

    eval_word(stack, NULL, mainfunc);

    ck_assert_int_eq(stack->size, 1);
    ck_assert_int_eq(stack_peek(stack, 0)->data.i, 5);

    ALMATESTCLEAN();
} END_TEST

Suite *simple_suite(void) {
    Suite *s;
    TCase *tc_core, *tc_comp, *tc_bind;

    s = suite_create("Basics");

    /* core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_stack_push);
    tcase_add_test(tc_core, test_stack_pop_print);
    tcase_add_test(tc_core, test_addition);
    tcase_add_test(tc_core, test_apply);
    suite_add_tcase(s, tc_core);

    /* test compilation */
    tc_comp = tcase_create("Compilation");

    tcase_add_test(tc_comp, test_duplicate_func_error);
    tcase_add_test(tc_comp, test_unknown_func_error);
    tcase_add_test(tc_comp, test_definition);
    tcase_add_test(tc_comp, test_let);
    tcase_add_test(tc_comp, test_2let);
    suite_add_tcase(s, tc_comp);

    /* test name binding (closures, etc) */
    tc_bind = tcase_create("Name binding");

    tcase_add_test(tc_bind, test_bind);
    tcase_add_test(tc_bind, test_2bind);
    tcase_add_test(tc_bind, test_funcargs);
    tcase_add_test(tc_bind, test_blockparam);
    tcase_add_test(tc_bind, test_closure);
    tcase_add_test(tc_bind, test_multibuf);
    tcase_add_test(tc_bind, test_namedclosure);
    suite_add_tcase(s, tc_bind);

    return s;
}

int main(void) {
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = simple_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
