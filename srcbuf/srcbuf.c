#include "srcbuf.h"

struct src_buffer
src_buf_load(const char * path)
{
    struct src_buffer sb = {0};
    FILE * f = fopen(path, "r");
    if (!f) return sb;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    sb.text = malloc(size + 1);
    if (!sb.text) { fclose(f); return sb; }
    fread(sb.text, 1, size, f);
    sb.text[size] = '\0';
    fclose(f);

    // count lines
    int cap = 64, cnt = 0;
    sb.lines = malloc(cap * sizeof(char *));
    sb.lines[cnt++] = sb.text;
    for (char * p = sb.text; *p; p++) {
        if (*p == '\n') {
            *p = '\0';          // split in place
            if (cnt == cap) {
                cap *= 2;
                sb.lines = realloc(sb.lines, cap * sizeof(char *));
            }
            sb.lines[cnt++] = p + 1;
        }
    }
    sb.line_count = cnt;
    return sb;
}

void
src_buf_free(struct src_buffer * sb)
{
    free(sb->text);
    free(sb->lines);
    *sb = (struct src_buffer){0};
}

void
src_buf_print_loc(const struct src_buffer * sb,
                  struct ast_src_loc loc)
{
    int line_idx = loc.first_line - 1;
    if (line_idx < 0 || line_idx >= sb->line_count) return;
    const char * line = sb->lines[line_idx];
    fprintf(stderr, "%s\n", line);
    int start = loc.first_column - 1;
    int end   = (loc.first_line == loc.last_line)
                    ? loc.last_column
                    : (int)strlen(line);
    for (int i = 0; i < start; i++) fprintf(stderr, " ");
    for (int i = start; i < end; i++) fprintf(stderr, "^");
    fprintf(stderr, "\n");
}
