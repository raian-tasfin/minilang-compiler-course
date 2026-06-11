#include "../ast/ast.h"

#ifndef SRCBUF_H
#define SRCBUF_H 1

struct src_buffer {
    char *  text;
    char ** lines;
    int     line_count;
};

struct src_buffer src_buf_load(const char * path);
void src_buf_free(struct src_buffer * buf);
void src_buf_print_loc(const struct src_buffer * sb, struct ast_src_loc loc);


#endif
// SRCBUF_H
