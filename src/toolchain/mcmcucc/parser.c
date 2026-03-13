#include "parser.h"

#include "context.h"
#include "ast.h"

// compile-unit	:= extern-decl | compile-unit
// extern-decl	:= func-def | decl
// func-def		:= decl-spec declarator decl-list? comp-stmt
// decl-list	:= decl | decl-list decl

void parse(context_t *ctx, ast_t *ast) {

}
