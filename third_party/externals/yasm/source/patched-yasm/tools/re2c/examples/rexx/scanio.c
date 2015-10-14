uchar *ScanFill(uchar *cursor){
    unsigned cnt = s->tok - s->bot;
    s->pos += cursor - s->mrk;
    if(cnt){
        if(s->eot){
            unsigned len = s->eot - s->tok;
            memcpy(s->bot, s->tok, len);
            s->eot = &s->bot[len];
            if((len = s->lim - cursor) != 0)
                memcpy(s->eot, cursor, len);
            cursor = s->eot;
            s->lim = &cursor[len];
        } else {
            memcpy(s->bot, s->tok, s->lim - s->tok);
            cursor -= cnt;
            s->lim -= cnt;
        }
        s->tok = s->bot;
        s->ptr -= cnt;
    }
    if((s->top - s->lim) < 512){
        uchar *buf = (uchar*) malloc(((s->lim - s->bot) + 512)*sizeof(uchar));
        memcpy(buf, s->bot, s->lim - s->bot);
        s->tok = buf;
        s->ptr = &buf[s->ptr - s->bot];
        if(s->eot)
            s->eot = &buf[s->eot - s->bot];
        cursor = &buf[cursor - s->bot];
        s->lim = &buf[s->lim - s->bot];
        s->top = &s->lim[512];
        free(s->bot);
        s->bot = buf;
    }
    s->mrk = cursor;
    if(ScanCBIO.file){
        if((cnt = read(ScanCBIO.u.f.fd, (char*) s->lim, 512)) != 512)
            memset(&s->lim[cnt], 0, 512 - cnt);
        s->lim += 512;
    }
    return cursor;
}
