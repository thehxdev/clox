char *io_file_read(sth_arena_t *arena, const char *path, size_t *out_size) {
    char *buffer = NULL;
    size_t file_size, nread;

    FILE *fp = fopen(path, "r");
    if (!fp)
        goto ret;

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    buffer = sth_arena_alloc(arena, file_size);
    nread = fread(buffer, sizeof(char), file_size, fp);
    assert(nread == file_size);

    fclose(fp);
    if (out_size)
        *out_size = file_size;

ret:
    return buffer;
}
