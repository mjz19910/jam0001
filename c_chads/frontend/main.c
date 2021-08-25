#include <stdio.h>
#include <stdlib.h>
#include "../lang/parser/parser.h"
#include "../lang/parser/checker.h"
#include "../lang/interpreter/interpreter.h"
#include "../aid/sfio/sfio.h"

void print_type(struct Parser_Type *type) {
    for (usize i = 0; i < type->depths.size; i += 1) {
        usize v = *(usize*)vec_get(&type->depths, i);
        if (v != 0) printf("%zu", v);
        printf("*");
    }
    printf("%.*s", (int)type->name.size, type->name.view);
}

void print_ast(struct Parser_Node *node, usize depth) {
    if (node->kind == PN_DECL) {
        struct Vec *annots = &node->data.decl.annotations;
        for (usize i = 0; i < annots->size; i += 1) {
            for (usize i = 0; i < depth; i += 1) 
                printf("    ");
            strview_t *view = vec_get(annots, i);
            printf("#%.*s\n", (int)view->size, view->view);
        }
    }
    for (usize i = 0; i < depth; i += 1) 
        printf("    ");

    switch (node->addressing) {
        case PA_UNARY:
            printf("~~");
            break;
        case PA_TERNARY:
            printf("??");
            break;
        case PA_LISTING:
            printf("[]");
            break;
        case PA_BINARY:
            printf("++");
            break;
        case PA_ENDPOINT:
            printf("->");
            break;
    }
    switch (node->kind) {
        case PN_INVAL:
            break;
        case PN_ACCESS:
            printf("Access\n");
            break;
        case PN_LIST:
            printf("List\n");
            break;
        case PN_MACRO:
            printf("Macro\n");
            break;
        case PN_COMMAND:
            printf("Command\n");
            break;
        case PN_NEW:
            printf("New(");
            print_type(&node->data.newinst.type);
            printf(")\n");
            break;
        case PN_STRUCT:
            printf("Struct\n");
            break;
        case PN_TOPLEVEL:
            printf("Toplevel\n");
            break;
        case PN_UNARY:
            printf("Unary(%.*s)\n", (int)node->data.unary.op.size, node->data.unary.op.view);
            break;
        case PN_FIELD:
            printf("Field(%.*s)\n", (int)node->data.field.name.size, node->data.field.name.view);
            break;
        case PN_IF:
            printf("If\n");
            break;
        case PN_WHILE:
            printf("While\n");
            break;
        case PN_TYPELIST:
            printf("TypeList\n");
            break;
        case PN_PARAMS:
            printf("Params\n");
            break;
        case PN_BODY:
            printf("Body\n");
            break;
        case PN_INIT:
            printf("Init(name = ");
            printf("%.*s)\n", (int)node->data.init.name.size, node->data.init.name.view);
            break;
        case PN_DECL:
            printf("Decl(type = ");
            print_type(&node->data.decl.type);
            printf(", name = %.*s)\n", (int)node->data.decl.name.size, node->data.decl.name.view);
            break;
        case PN_OPERATOR:
            printf("Op(%.*s)\n", (int)node->data.op.op.size, node->data.op.op.view);
            break;
        case PN_ASSIGN:
            printf("Assignment\n");
            break;
        case PN_RETURN:
            printf("Return\n");
            break;
        case PN_PROC:
            printf("Proc(return = ");
            print_type(&node->data.proc.return_type);
            printf(")\n");
            break;
        case PN_CALL:
            printf("Call\n");
            break;
        case PN_STRING:
            printf("String(%.*s)\n", (int)node->data.string.val.size, node->data.string.val.view);
            break;
        case PN_NUMBER:
            printf("Number@%d(%.*s)\n", node->data.number.kind, (int)node->data.number.val.size, node->data.number.val.view);
            break;
        case PN_IDENT:
            printf("Ident(%.*s)\n", (int)node->data.ident.val.size, node->data.ident.val.view);
            break;
    }
    if (node->kind != PN_INVAL)
        for (usize i = 0; i < node->children.size; i += 1) {
            print_ast(vec_get(&node->children, i), depth + 1);
        }

#if 0
    if (node->kind != PN_INVAL)
        vec_drop(&node->children);
#endif
}

int main(int argc, char *argv[]) {
    if (argc < 2)  {
        fprintf(stderr, "No input files provided, usage: %s <file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    const string src = sfio_read_text(argv[1]);
    if (src == NULL) {
        fprintf(stderr, "Can't access file %s\n", argv[1]);
        return EXIT_FAILURE;
    } 
    eh_set_file(argv[1]);
    eh_init();
    parser_init(src);
    //printf("------\n%s\n------\n", parser_get_state()->lexer.src);
    struct Parser_Node result = parser_parse_toplevel();
    checker_run(&result);
    //print_ast(&result, 0);
    printf("------Running------\n");

    intrp_init();

    intrp_run(&result, NULL);
    intrp_main();

    intrp_deinit();


    vec_drop(&result.children);
    parser_deinit();
    free(src);
    eh_deinit();
    return 0;
}
