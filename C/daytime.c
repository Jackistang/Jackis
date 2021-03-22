
#define DAYTIME_LEN 10
/**
 * @brief Get the format time Hour:Min:Sec from time_t.
 * @param time A pointer to timer_t.
 * @return Hour:Min:Sec format time string.
 */
char *daytime(time_t *time)
{
    static char buf[DAYTIME_LEN];
    int blank = 0, i = 0;
    char *t = ctime(time);
    while (*t != '\0' && i < DAYTIME_LEN) {
        if (*t == ' ')
            blank++;
        if (blank == 3) {
            buf[i++] = *t;
        }
        t++;
    }

    return buf;
}